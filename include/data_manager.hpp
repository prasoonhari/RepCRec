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


class DataManager
{
private:
    int site_id;
    // info about each variable {currentValue, last committed value, last committed time} 
    std::map<int, DataDetail> data;
    // Lock Manager instance that holds locks for the site
    LockManager* lm;
    //the variables that are not committed after recovering
    std::set<int> unclean_data_on_site;

    //map of the transaction and the variables it has changed
    // gets updated when any write happens
    // make modification to data if commit else make it same as last committed value if aborting
    // TODO: remove items from it when committed or aborted
    std::map<int, std::vector<int>> txn_changed_variables;



public:
    DataManager(int _site_id);
    void initializeDataManager();
    void setData(int variable, int value, int commit_time);
    void printDM();
    int readRO( int variable, Transaction txn);
    bool checkIfDataRecovered(int variable);
    int getLastCommittedTime(int variable);

    int getReadLockStatus(int variable, Transaction txn);

    int getWriteLockStatus(int variable, Transaction txn);

    TransactionResult write(int variable, Transaction *txn);

    void setDataCommit(int variable, int value, int commit_time);

    void setDataTemp(int variable, int value);

    TransactionResult read(int variable, Transaction *txn);

    TransactionResult writeCheck(int variable, Transaction *txn);
};

#endif