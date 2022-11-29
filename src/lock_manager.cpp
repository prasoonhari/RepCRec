
#include <iostream>
#include "lock_manager.hpp"
using namespace std;

LockManager::LockManager()
{
    lock_table = {};
}

void LockManager::initializeLock(int variable){
    lock_table[variable] = {};
}

void LockManager::reset(){
    lock_table.clear();
}


int LockManager::getReadLock(int variable, int transaction_id){
    return 0;
}


int LockManager::getWriteLock(int variable, int transaction_id){
    return 0;
}