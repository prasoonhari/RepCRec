
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

void DataManager::setAllDataDirty() {
    for (auto x: data) {
        unclean_data_on_site.insert(x.first);
    }
}

bool DataManager::setDataCommit(int variable, int commit_time) {
    data[variable].committedValue = data[variable].currentValue;
    data[variable].lastCommitTime = commit_time;

    //make this data clean if not
    // INFO: Note that this will be only in case when site is recovering
    if (unclean_data_on_site.find(variable) != unclean_data_on_site.end()){
        unclean_data_on_site.erase(variable);
        // TODO: Now this will make a site available for read, so we can evoke transactions waiting for this site
        return true;
    }

    return false;
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
    return varLockDetail.lock_type != LOCK_TYPE::l_write ||
           (varLockDetail.currentHolderMap.find(txn_Id) != varLockDetail.currentHolderMap.end());
}

bool DataManager::checkWriteLockCondition(LockDetail varLockDetail, int txn_Id) {
    return varLockDetail.lock_type == LOCK_TYPE::l_NONE ||
           (varLockDetail.currentHolderMap.size() == 1 &&
            varLockDetail.currentHolderMap.find(txn_Id) != varLockDetail.currentHolderMap.end());
}

int DataManager::getLastCommittedTime(int variable){
    return data[variable].lastCommitTime;
}

int DataManager::readData(int variable) {
    return data[variable].committedValue;
}

TransactionResult DataManager::readRO(int variable) {
    TransactionResult txnRes;
    txnRes.status = RESULT_STATUS::success;
    cout << "x" + to_string(variable) + ": " + to_string(data[variable].committedValue) << "\n";
    return txnRes;
}

TransactionResult DataManager::read(int variable, Transaction *txn) {
    TransactionResult txnRes;
    LockDetail varLockDetail = lm->getLockDetail(variable);
    if (checkReadLockCondition(varLockDetail, txn->id)) {
        txnRes.status = RESULT_STATUS::success;
        bool lockAcquired = lm->acquireReadLock(variable, txn->id);
        if (!lockAcquired) {
            cout << "fail to acquire read lock - something went wrong \n";
        } else {
            // Put in this map to track which transaction holds which variables of this site;
            if (txn_locked_variables.find(txn->id) == txn_locked_variables.end()) {
                txn_locked_variables[txn->id] = {variable};
            } else {
                if(std::find(txn_locked_variables[txn->id].begin(), txn_locked_variables[txn->id].end(),
                             variable) ==
                        txn_locked_variables[txn->id].end()) {
                    txn_locked_variables[txn->id].push_back(variable);

                }
            }

            bool hasThisTransactionWrittenThisData = false;
            if (txn->dirtyData.find(site_id) != txn->dirtyData.end()){
                for (auto x : txn->dirtyData[site_id]){
                    if (x == variable){
                        hasThisTransactionWrittenThisData = true;
                    }
                }
            }
            if (hasThisTransactionWrittenThisData){
                cout << "Transaction T" << txn->id << " read "<< "x" + to_string(variable) + ": " + to_string(data[variable].currentValue) << "  from site" << site_id << "\n";

            }else{
                cout << "Transaction T" << txn->id << " read "<< "x" + to_string(variable) + ": " + to_string(data[variable].committedValue) << "  from site" << site_id << "\n";
            }
        }
    } else {
        txnRes.status = RESULT_STATUS::failure;
        txnRes.blockingTransaction = varLockDetail.currentHolderQueue;
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
            cout << "fail to acquire write lock - something went wrong \n";
        } else {
            // Put in this map to track which transaction holds which variables of this site;
            // INFO: There is no need to delete anything from this other than when a Txn commits or aborts
            if (txn_locked_variables.find(txn->id) == txn_locked_variables.end()) {
                txn_locked_variables[txn->id] = {variable};
            } else {
                if(std::find(txn_locked_variables[txn->id].begin(), txn_locked_variables[txn->id].end(),
                             variable) ==
                   txn_locked_variables[txn->id].end()) {
                    txn_locked_variables[txn->id].push_back(variable);

                }
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

//void DataManager::printDM() {
//    cout << "Data Printing for site" << site_id << " Starts \n";
//    for (auto itt = data.begin(); itt != data.end(); itt++) {
//        cout << "x" << itt->first << ": " << itt->second.committedValue << ", tempVal = " << itt->second.currentValue
//             << ", last commit time = " << itt->second.lastCommitTime << " \n";
//    }
//    cout << "Data Printing for site " << site_id << " Ends \n";
//    cout << "\n";
//}

void DataManager::printDM() {
    cout << "site " << site_id << "- ";
    int n = data.size();
    int i = 0;
    for (auto & itt : data) {
        cout << "x" << itt.first << ": " << itt.second.committedValue;
        if (i != n-1){
            cout << ", ";
        }
        i++;
    }
    cout << "\n";
}


void DataManager::printLM() {
    cout << "lock detail for site" << site_id << " Starts \n";
    lm->printLM();
    cout << "lock detail for site " << site_id << " Ends \n";
    cout << "\n";
}

void DataManager::failThisSite() {
    lm->reset();
    // TODO : Think about this .. should this be cleared
//    txn_locked_variables.clear();
    unclean_data_on_site.clear();

}

void DataManager::recoverThisSite() {
    initializeLockTable();
    setAllDataDirty();
}

vector<int> DataManager::releaseLock(int txn_id) {

    vector<int> releasedVariable = {};

    for (auto var: txn_locked_variables[txn_id]) {
        if (lm->removeLock(var, txn_id)) {
            // Add when successfully released
            releasedVariable.push_back(var);
        }
    }
    // remove the txn from this site as now all its lock is released
    txn_locked_variables.erase(txn_id);
    return releasedVariable;

}