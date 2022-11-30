
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


TransactionResult DataManager::read(int variable, Transaction *txn) {
    TransactionResult txnRes;
    LockDetail varLockDetail = lm->getLockDetail(variable);
    if (varLockDetail.lock_type == LOCK_TYPE::l_read || (varLockDetail.lock_type == LOCK_TYPE::l_write &&
                                                         varLockDetail.currentHolderMap.find(txn->id) !=
                                                         varLockDetail.currentHolderMap.end())) {
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

TransactionResult DataManager::writeCheck(int variable, Transaction *txn) {
    TransactionResult txnRes;
    LockDetail varLockDetail = lm->getLockDetail(variable);
    if ((varLockDetail.lock_type == LOCK_TYPE::l_read && varLockDetail.currentHolderMap.size() == 1) ||
        (varLockDetail.lock_type == LOCK_TYPE::l_write &&
         varLockDetail.currentHolderMap.find(txn->id) != varLockDetail.currentHolderMap.end())) {

        txnRes.status = RESULT_STATUS::success;

    } else {
        txnRes.status = RESULT_STATUS::failure;
        txnRes.transactions = varLockDetail.currentHolderQueue;
    }
    return txnRes;
}


bool DataManager::checkIfDataRecovered(int variable) {
    return unclean_data_on_site.find(variable) == unclean_data_on_site.end();
}

void DataManager::printDM() {

    for (auto itt = data.begin(); itt != data.end(); itt++) {
        cout << "x" << itt->first << ": " << itt->second.committedValue << ", ";
    }
    cout << "\n";
}