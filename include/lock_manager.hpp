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
    int lm_id;

public:
    LockManager(int _lm_id);
    void reset();
    void initializeLock(int variable);
    // returns 1 if successful, 2 if in waiting else 0 if fails
    bool acquireReadLock(int variable, int transaction_id);
    int getWriteLock(int variable, int transaction_id);

    int getReadLockStatus(int variable, int transaction_id);
    int getWriteLockStatus(int variable, int transaction_id);

    LockDetail getLockDetail(int variable);
};

#endif