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
    //the variables that are not commited after recovering
    std::set<int> unclean_data_on_site;



public:
    DataManager(int _site_id);
    void initializeDataManager();
    void setData(int variable, int value, int commit_time);
    void printDM();
    OperationResult read( int variable, Transaction txn);
    int readRO( int variable, Transaction txn);
    bool checkIfDataRecoved(int variable);
    int getLastCommittedTime(int variable);
};

#endif