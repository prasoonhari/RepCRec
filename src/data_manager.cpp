
#include <iostream>
#include "data_manager.hpp"
#include "lock_manager.hpp"
#include "common.hpp"

using namespace std;

DataManager::DataManager(int _site_id) {
    site_id = _site_id;
    data = {};
    // TODO: use for checking for all the data that has been not committed after this site was recovered
    unclean_data_on_site = {};
    lm = new LockManager(_site_id);
}

void DataManager::initializeLockTable() {
    for (auto x: data) {
        lm->initializeLock(x.first);
    }
}

void DataManager::setDataCommit(int variable, int commit_time) {
    data[variable].committedValue = data[variable].currentValue;
    data[variable].lastCommitTime = commit_time;
}

void DataManager::setDataRevert(int variable) {
    data[variable].currentValue = data[variable].committedValue;
}

void DataManager::setDataTemp(int variable, int value) {
    data[variable].currentValue = value;
}

void DataManager::setData(int variable, int value, int commit_time) {
    data[variable] = {value, value, commit_time};
}

bool DataManager::checkReadLockCondition(LockDetail varLockDetail, int txn_Id) {
    return varLockDetail.lock_type != LOCK_TYPE::l_write || (varLockDetail.currentHolderMap.find(txn_Id) != varLockDetail.currentHolderMap.end());
}

bool DataManager::checkWriteLockCondition(LockDetail varLockDetail, int txn_Id) {
    return varLockDetail.lock_type == LOCK_TYPE::l_NONE || (varLockDetail.lock_type == LOCK_TYPE::l_read && varLockDetail.currentHolderMap.size() <= 1) ||
           (varLockDetail.lock_type == LOCK_TYPE::l_write &&
            varLockDetail.currentHolderMap.find(txn_Id) != varLockDetail.currentHolderMap.end());
}


TransactionResult DataManager::read(int variable, Transaction *txn) {
    TransactionResult txnRes;
    LockDetail varLockDetail = lm->getLockDetail(variable);
    if (checkReadLockCondition(varLockDetail, txn->id)) {
        txnRes.status = RESULT_STATUS::success;
        bool lockAcquired = lm->acquireReadLock(variable, txn->id);
        if (!lockAcquired) {
            cout << "fail to acquire lock - something went wrong";
        } else {
            // Put in this map to track which transaction holds which variables of this site;
            if (txn_locked_variables.find(txn->id) == txn_locked_variables.end()){
                txn_locked_variables[txn->id] = {variable};
            }else{
                txn_locked_variables[txn->id].push_back(variable);
            }
            cout << "x" + to_string(variable) + ": " + to_string(data[variable].committedValue) << "\n";
        }
    } else {
        txnRes.status = RESULT_STATUS::failure;
        txnRes.transactions = varLockDetail.currentHolderQueue;
    }

    return txnRes;
}

// 0 - write lock is waiting, 1 - read lock waiting, 2 - success no lock waiting
pair<int, vector<int>> DataManager::writeCheck(int variable, Transaction *txn) {
    LockDetail varLockDetail = lm->getLockDetail(variable);
    if (checkWriteLockCondition(varLockDetail, txn->id)) {
        return {2, {}};

    } else if (varLockDetail.lock_type == LOCK_TYPE::l_write && varLockDetail.currentHolderMap.size() == 1) {
        return {0, vector<int>({varLockDetail.currentHolderQueue.begin(), varLockDetail.currentHolderQueue.end()})};
    } else if (varLockDetail.lock_type == LOCK_TYPE::l_read && !varLockDetail.currentHolderMap.empty()) {
        return {1, vector<int>({varLockDetail.currentHolderQueue.begin(), varLockDetail.currentHolderQueue.end()})};
    }
    return {-1, {}};;
}


// Write the data into the database (data map) temporarily
TransactionResult DataManager::write(int variable, Transaction *txn, int change_time) {
    LockDetail varLockDetail = lm->getLockDetail(variable);

    TransactionResult txnRes;
    if (checkWriteLockCondition(varLockDetail, txn->id)) {
        // acquire the right lock
        bool lockAcquired = lm->acquireWriteLock(variable, txn->id);

        if (!lockAcquired) {
            cout << "fail to acquire lock - something went wrong";
        } else {
            // Put in this map to track which transaction holds which variables of this site;
            // INFO: There is no need to delete anything from this other than when a Txn commits or aborts
            if (txn_locked_variables.find(txn->id) == txn_locked_variables.end()){
                txn_locked_variables[txn->id] = {variable};
            }else{
                txn_locked_variables[txn->id].push_back(variable);
            }
//            cout << "T" <<txn->id << " is temporarily writing x" << variable << " in site" << site_id << " \n";
            setDataTemp(variable, txn->currentInstruction.values[1]);
            txnRes.status = RESULT_STATUS::success;
        }
    } else {
        txnRes.status = RESULT_STATUS::failure;
        cout << "Something went wrong while writing the data \n";
        return txnRes;
    }
    return txnRes;
}


bool DataManager::checkIfDataRecovered(int variable) {
    return unclean_data_on_site.find(variable) == unclean_data_on_site.end();
}

void DataManager::printDM() {
    cout << "Data Printing for site" << site_id << " Starts \n";
    for (auto itt = data.begin(); itt != data.end(); itt++) {
        cout << "x" << itt->first << ": " << itt->second.committedValue << ", tempVal = " << itt->second.currentValue
             << ", last commit time = " << itt->second.lastCommitTime << " \n";
    }
    cout << "Data Printing for site " << site_id << " Ends \n";
    cout << "\n";
}


void DataManager::printLM() {
    cout << "lock detail for site" << site_id << " Starts \n";
    lm->printLM();
    cout << "lock detail for site " << site_id << " Ends \n";
    cout << "\n";
}

void DataManager::failThisSite(){
    lm->reset();
    initializeLockTable();
}

vector<int> DataManager::releaseLock(int variable, int txn_id) {

    vector<int> releasedVariable = {};
    if (txn_locked_variables.find(txn_id) == txn_locked_variables.end()){
        // No locks for this txn found here
    }else{
        for (auto var : txn_locked_variables[txn_id]){
            if (lm->removeLock(variable,txn_id)){
                // Add when successfully released
                releasedVariable.push_back(var);
            }
        }
        // remove the txn from its locked variable
        txn_locked_variables.erase(variable);
    }

    return releasedVariable;

}