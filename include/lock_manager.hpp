#ifndef LOCKMANAGER
#define LOCKMANAGER

#include <string>
#include <map>
#include <vector>

#include "common.hpp"

class LockManager
{
private:
    // lock detail on each variable. variable_id -> lock details
    std::map<int, LockDetail> lock_table;

public:
    LockManager();
    void initializeLock(int variable);
    // returns 1 if successful, 2 if in waiting else 0 if fails
    int getReadLock(int variable, int transaction_id);
    int getWriteLock(int variable, int transaction_id);
};

#endif