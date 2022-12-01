
#include <iostream>
#include <iterator>

#include "lock_manager.hpp"

using namespace std;

LockManager::LockManager(int _lm_id) {
    lm_id = _lm_id;
    lock_table = {};
}

void LockManager::initializeLock(int variable) {
    lock_table[variable] = {};
}

void LockManager::reset() {
    lock_table.clear();
}

LockDetail LockManager::getLockDetail(int variable){
    return lock_table[variable];
}

// get read lock status 1 - means available for read, 2 - means waiting , 0 -means failed
int LockManager::getReadLockStatus(int variable, int transaction_id) {
    // when there is no lock holder or if the current lock status is Read and there is no write waiting
    if (lock_table[variable].currentHolderQueue.empty() || lock_table[variable].lock_type == LOCK_TYPE::l_read) {
        return 1;
    }
        // if the current transaction holds a lock whether read or write
    else if (lock_table[variable].currentHolderMap.find(transaction_id) !=
             lock_table[variable].currentHolderMap.end()) {
        return 1;
    }
    return 0;
}


// get write lock status 1 - means available for write, 2 - means waiting , 0 -means failed for the given txn
int LockManager::getWriteLockStatus(int variable, int transaction_id) {
    // no one currently holds the lock
    if (lock_table[variable].currentHolderQueue.empty()) {
        return 1;
    }
        // only one transaction holds the lock and that is itself, if it holds read lock then that will be promoted while acquiring the lock
    else if (lock_table[variable].currentHolderQueue.size() == 1 &&
             lock_table[variable].currentHolderMap.find(transaction_id) !=
             lock_table[variable].currentHolderMap.end()) {
        return 1;
    }
    return 0;
}

bool LockManager::removeLock(int variable, int transaction_id) {
    // no one currently holds the lock
    if (lock_table[variable].currentHolderMap.find(transaction_id) == lock_table[variable].currentHolderMap.end()){
        cout << "No lock to remove \n";
        return false;
    }
    lock_table[variable].currentHolderQueue.erase(lock_table[variable].currentHolderMap[transaction_id]);
    lock_table[variable].currentHolderMap.erase(transaction_id);
    if (lock_table[variable].currentHolderQueue.empty()){
        lock_table[variable].lock_type = LOCK_TYPE::l_NONE;
    }
    return true;
}

bool LockManager::acquireReadLock(int variable, int transaction_id) {
    if ((lock_table[variable].lock_type == LOCK_TYPE::l_NONE || lock_table[variable].lock_type == LOCK_TYPE::l_read)) {
        lock_table[variable].lock_type = LOCK_TYPE::l_read;

        // check that already not holding
        if (lock_table[variable].currentHolderMap.find(transaction_id) == lock_table[variable].currentHolderMap.end()) {
            lock_table[variable].currentHolderQueue.push_back(transaction_id);
            lock_table[variable].currentHolderMap[transaction_id] = prev(lock_table[variable].currentHolderQueue.end());
        }
        return true;
    }
        // if the current transaction holds a lock whether read or write
    else if (lock_table[variable].currentHolderMap.find(transaction_id) !=
             lock_table[variable].currentHolderMap.end()) {
        return true;
    }
    return false;
}

bool LockManager::acquireWriteLock(int variable, int transaction_id) {
    if (lock_table[variable].lock_type == LOCK_TYPE::l_NONE) {
        lock_table[variable].lock_type = LOCK_TYPE::l_write;
        lock_table[variable].currentHolderQueue.push_back(transaction_id);
        lock_table[variable].currentHolderMap[transaction_id] = prev(lock_table[variable].currentHolderQueue.end());
        return true;

    }
        // only one transaction holds the lock and that is itself, if it holds read lock then that will be promoted while acquiring the lock
    else if (lock_table[variable].currentHolderQueue.size() == 1 &&
             lock_table[variable].currentHolderMap.find(transaction_id) !=
             lock_table[variable].currentHolderMap.end()) {
        // promote to write if not
        lock_table[variable].lock_type = LOCK_TYPE::l_write;
        return true;
    }
    return false;
}

void LockManager::printLM() {
    for (auto itt = lock_table.begin(); itt != lock_table.end(); itt++) {
        cout << "Lock x" << itt->first << ": " << itt->second.lock_type << ", current Holder: ";
        for (auto x : itt->second.currentHolderQueue){
            cout << x << " ";
        }
        cout << "\n";
    }
}