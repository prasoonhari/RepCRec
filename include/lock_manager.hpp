#ifndef LOCKMANAGER
#define LOCKMANAGER

#include <string>
#include <map>
#include <vector>

#include "common.hpp"

/**
 * @brief The Lock Manager helps in managing the instances of locks on sites.
 * 
 */
class LockManager
{
private:

    /**
     * @brief Holds lock details for each variable
     * LockDetails holds the type of lock the variable has and the transactions
     * that hold the variable
     */
    std::map<int, LockDetail> lock_table;
    
    int lm_id;

public:
    explicit LockManager(int _lm_id);

    /**
     * @brief resets the lock table in case of site failure
     * 
     */
    void reset();

    /**
     * @brief initializes the lock for each variable when a site is
     * initialized or recovers
     * 
     * @param variable 
     */
    void initializeLock(int variable);

    /**
     * @brief acquires the read lock for a txn if all condition are satisfied
     * 
     * @param variable 
     * @param transaction_id 
     * @return true if the lock is acquired, false otherwise
     */
 
    bool acquireReadLock(int variable, int transaction_id);

    /**
     * @brief acquires write lock for a txn if all condition are satisfied for the given variable(data)
     * 
     * @param variable 
     * @param transaction_id 
     * @return true if the lock is acquired, false otherwise
     */
    // 
    bool acquireWriteLock(int variable, int transaction_id);


    // IGNORE - Not in use
    int getReadLockStatus(int variable, int transaction_id);
    // IGNORE - Not in use
    int getWriteLockStatus(int variable, int transaction_id);

    // Returns the lock detail of a particular variable
    LockDetail getLockDetail(int variable);

    /**
     * @brief Removes the lock that a particular transaction holds, 
     * if no one holds the lock after removal, then it resets
     * 
     * @param variable 
     * @param transaction_id 
     * @return true if removed successfully, false otherwise
     */
    bool removeLock(int variable, int transaction_id);

    // Print the lock table data
    void printLM();
};

#endif