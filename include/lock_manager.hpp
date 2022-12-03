#ifndef LOCKMANAGER
#define LOCKMANAGER

#include <string>
#include <map>
#include <vector>

#include "common.hpp"

/**
 * @brief The Lock Manager help in managing the instances of locks on different sites.
 * 
 */
class LockManager
{
private:
    // lock detail on each variable. variable -> lock details
    std::map<int, LockDetail> lock_table;
    int lm_id;

public:
    explicit LockManager(int _lm_id);
    void reset();
    void initializeLock(int variable);
    // acquires the read lock for a txn if all condition are satisfied
    bool acquireReadLock(int variable, int transaction_id);
    // acquires  write lock for a txn if all condition are satisfied for the given variable(data)
    bool acquireWriteLock(int variable, int transaction_id);


    // IGNORE - Not in use
    int getReadLockStatus(int variable, int transaction_id);
    // IGNORE - Not in use
    int getWriteLockStatus(int variable, int transaction_id);

    // Returns the lock detail of a particular variable
    LockDetail getLockDetail(int variable);

    // Removes the lock that a particular transaction holds, if no one holds the lock after removal, then its reset
    bool removeLock(int variable, int transaction_id);

    // Print the lock table data
    void printLM();
};

#endif