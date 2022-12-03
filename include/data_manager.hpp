#ifndef DATAMANAGER
#define DATAMANAGER

#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>
#include "lock_manager.hpp"
#include "common.hpp"

/**
 * @brief 
 * If a site fails and recovers, Then Data manager(DM) would normally decide which in-flight transactions to commit by asking the TM about transactions that the DM holds pre-committed but not yet committed. 
 * The DM updates the variable values and manages locks for the particular sites.
 */
class DataManager {
private:
    int site_id;

    /**
     * @brief Info about each variable {currentValue, last committed value, last committed time}.
     * This is the main database that stores value of variables for this site.
     * 
     * DataDetail object contain the committed value of variable, the time it was committed and current value 
     * which is written by a transaction that has not committed it yet, so it's dirty
     * 
     */
    std::map<int, DataDetail> data;

    // Lock Manager instance that holds locks for this site
    LockManager *lm;


public:
   
    /**
     * @brief 
     * 
     * A map of transaction and the variables it has changed gets updated when any lock is 
     * acquired by any transaction make modification to data if commit else 
     * make it same as last committed value if aborting. This is used to remove locks that 
     * the aborted or committed Txn holds
     */
    std::map<int, std::vector<int>> txn_locked_variables;

    /**
     * @brief The variables that are not committed after recovering. 
     * So these can be written by not read in case of replicated data
     * 
     */
    std::set<int> unclean_data_on_site;

    explicit DataManager(int _site_id);

    void initializeLockTable();

    // Set the variable data in the db(the data map) -- Used only at the time of initialization
    void setData(int variable, int value, int commit_time);

    void printDM();

    /**
     * @brief Checks if a write is possible for a transaction on a variable
     * 
     * @param variable 
     * @param txn 
     * @return std::pair<int, std::vector<int>> 
     */
    std::pair<int, std::vector<int>> writeCheck(int variable, Transaction *txn);


    bool checkIfDataRecovered(int variable);

    int getLastCommittedTime(int variable);

//    int getReadLockStatus(int variable, Transaction txn);
//
//    int getWriteLockStatus(int variable, Transaction txn);

    TransactionResult write(int variable, Transaction *txn, int change_time);
   
    /**
     * @brief Commits the data by copying the current value into committed value
     * Done when committing a Txn
     * 
     * @param variable 
     * @param commit_time 
     * @return true /false
     */
    bool setDataCommit(int variable, int commit_time);

    /**
     * @brief Set the current value - Done when a Txn writes (will get committed later if)
     * 
     * @param variable 
     * @param value 
     */
    void setDataTemp(int variable, int value);

    /**
     * @brief Tries to read a variable for a transaction.
     * If successful, reads it, otherwise returns an error response.
     * 
     * @param variable 
     * @param txn 
     * @return TransactionResult 
     */
    TransactionResult read(int variable, Transaction *txn);


    bool checkReadLockCondition(LockDetail varLockDetail, int txn_Id);

    bool checkWriteLockCondition(LockDetail varLockDetail, int txn_Id);

    void printLM();

    void failThisSite();

    /**
     * @brief Releases a lock and returns a list of variables it was holding
     * 
     * @param txn_id 
     * @return std::vector<int> 
     */
    std::vector<int> releaseLock(int txn_id);

    // revert a dirty by copying the committed value into current value - Done when aborting a Txn
    void setDataRevert(int variable);

    void recoverThisSite();

    // at time of recovery all data is dirty as the site was down
    /**
     * @brief At time of recovery, sets all data is dirty as the site was down.
     * 
     */
    void setAllDataDirty();

    /**
     * @brief Tries to read a variable for a read-only transaction.
     * If successful, reads it, otherwise returns an error response.
     * 
     * @param variable 
     * @return TransactionResult 
     */
    TransactionResult readRO(int variable);

    /**
     * @brief Returns the committed value for a variable
     * 
     * @param variable 
     * @return int 
     */
    int readData(int variable);
};

#endif