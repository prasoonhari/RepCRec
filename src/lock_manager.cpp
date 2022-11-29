
#include <iostream>
#include "lock_manager.hpp"
using namespace std;

LockManager::LockManager()
{
    lock_table = {};
}


// void LockManager::InitializeLockManager()
// {
//     for (int var = 1; var <= 20; var++)
//     {
//         if (var & 1)
//         {
//             readLocks[var] = {{1 + var % 10, {}}};
//             writeLocks[var] = {{1 + var % 10, {}}};
//         }
//         else
//         {
//             std::map<int, std::list<int>> readInitialMap;
//             std::map<int, std::list<int>> writeInitialMap;
//             for (int i = 1; i <= 10; i++)
//             {
//                 readInitialMap[i] = {};
//                 writeInitialMap[i] = {};
//             }
//             readLocks[var] = readInitialMap;
//             writeLocks[var] = writeInitialMap;
//         }
//     }

// }


void LockManager::initializeLock(int variable){
    lock_table[variable] = {};
}


int LockManager::getReadLock(int variable, int transaction_id){
    return 0;
}


int LockManager::getWriteLock(int variable, int transaction_id){
    return 0;
}