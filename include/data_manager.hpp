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


class DataManager {
private:
    int site_id;
    // info about each variable {currentValue, last committed value, last committed time}
    // This is the main database that stores value of variables for this site

    std::map<int, DataDetail> data;

    // Lock Manager instance that holds locks for this site
    LockManager *lm;


public:
    // a map of  transaction and the variables it has changed
    // gets updated when any lock is acquired by any transaction
    // make modification to data if commit else make it same as last committed value if aborting
    // DONE: remove items from it when committed or aborted
    // This is used to remove locks that the aborted or committed Txn holds
    std::map<int, std::vector<int>> txn_locked_variables;

    //the variables that are not committed after recovering - So these can be written by not read in case of replicated data
    std::set<int> unclean_data_on_site;

    explicit DataManager(int _site_id);

    void initializeLockTable();

    // Set the variable data in the db(the data map) -- Used only at the time of initialization
    void setData(int variable, int value, int commit_time);

    void printDM();

    std::pair<int, std::vector<int>> writeCheck(int variable, Transaction *txn);


    bool checkIfDataRecovered(int variable);

    int getLastCommittedTime(int variable);

//    int getReadLockStatus(int variable, Transaction txn);
//
//    int getWriteLockStatus(int variable, Transaction txn);

    TransactionResult write(int variable, Transaction *txn, int change_time);
    // commits the data by copying the current value into committed value - Done when committing a Txn
    bool setDataCommit(int variable, int commit_time);
    // Set the current value - Done when a Txn writes (will get committed later if)
    void setDataTemp(int variable, int value);

    TransactionResult read(int variable, Transaction *txn);


    bool checkReadLockCondition(LockDetail varLockDetail, int txn_Id);

    bool checkWriteLockCondition(LockDetail varLockDetail, int txn_Id);

    void printLM();

    void failThisSite();

    std::vector<int> releaseLock(int txn_id);
    // revert a dirty by copying the committed value into current value - Done when aborting a Txn
    void setDataRevert(int variable);

    void recoverThisSite();

    // at time of recovery all data is dirty as the site was down
    void setAllDataDirty();

    TransactionResult readRO(int variable);

    int readData(int variable);
};

#endif