
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

void DataManager::initializeDataManager() {
    for (auto x: data) {
        lm->initializeLock(x.first);
    }
}

void DataManager::setDataCommit(int variable, int value, int commit_time) {
    data[variable].currentValue = value;
    data[variable].lastCommitTime = commit_time;
}

void DataManager::setDataTemp(int variable, int value) {
    data[variable].currentValue = value;
}

void DataManager::setData(int variable, int value, int commit_time) {
    data[variable] = {value, value, commit_time};
}

bool DataManager::checkReadLockCondition(LockDetail varLockDetail, int txn_Id) {
    return varLockDetail.lock_type == LOCK_TYPE::l_read || (varLockDetail.lock_type == LOCK_TYPE::l_write &&
                                                            varLockDetail.currentHolderMap.find(txn_Id) !=
                                                            varLockDetail.currentHolderMap.end());
}

bool DataManager::checkWriteLockCondition(LockDetail varLockDetail, int txn_Id) {
    return (varLockDetail.lock_type == LOCK_TYPE::l_read && varLockDetail.currentHolderMap.size() <= 1) ||
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
            cout << "x" + to_string(variable) + ": " + to_string(data[variable].committedValue);
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