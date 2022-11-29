
#include <iostream>
#include <iterator>

#include "lock_manager.hpp"
using namespace std;

LockManager::LockManager(int _lm_id)
{
    lm_id = _lm_id;
    lock_table = {};
}

void LockManager::initializeLock(int variable)
{
    lock_table[variable] = {};
}

void LockManager::reset()
{
    lock_table.clear();
}

int LockManager::getReadLock(int variable, int transaction_id)
{
    if ((lock_table[variable].currentHolderQueue.empty() || lock_table[variable].lock_type == LOCK_TYPE::read) && lock_table[variable].waitingQueue.empty())
    {
        lock_table[variable].lock_type = LOCK_TYPE::read;

        // check that already not holding
        if (lock_table[variable].currentHolderMap.find(transaction_id) == lock_table[variable].currentHolderMap.end())
        {
            
            lock_table[variable].currentHolderQueue.push_back(transaction_id);
            lock_table[variable].currentHolderMap[transaction_id] = prev(lock_table[variable].currentHolderQueue.end());
        }
        return 1;
    }
    else if (!lock_table[variable].waitingQueue.empty())
    {
        if (lock_table[variable].waitingMap.find(transaction_id) == lock_table[variable].waitingMap.end())
        {
            lock_table[variable].waitingQueue.push_back(transaction_id);
            lock_table[variable].waitingMap[transaction_id] = prev(lock_table[variable].waitingQueue.end());
        }
        return 2;
    }
    return 0;
}

int LockManager::getWriteLock(int variable, int transaction_id)
{
    if (lock_table[variable].currentHolderQueue.empty() )
    {
        lock_table[variable].lock_type = LOCK_TYPE::write;

        // check that already not holding
        if (lock_table[variable].currentHolderMap.find(transaction_id) == lock_table[variable].currentHolderMap.end())
        {
            lock_table[variable].currentHolderQueue.push_back(transaction_id);
            lock_table[variable].currentHolderMap[transaction_id] = prev(lock_table[variable].currentHolderQueue.end());
        }
        return 1;
    }
    else
    {
        if (lock_table[variable].waitingMap.find(transaction_id) == lock_table[variable].waitingMap.end())
        {
            lock_table[variable].waitingQueue.push_back(transaction_id);
            lock_table[variable].waitingMap[transaction_id] =  prev(lock_table[variable].waitingQueue.end());
        }
        return 2;
    }
    return 0;
}