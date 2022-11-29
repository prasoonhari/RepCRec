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
    // data info about each site site_id -> map [ variable -> value, commitTime] 
    std::map<int, std::map<int, std::pair<int,int>>> sites;
    // status info about each site site_id -> pair [ site_status -> last time status changed] 
    std::map<int, std::pair<SITE_STATUS, int>> sites_status;
    // Lock Manager instance that holds locks for each site
    std::map<int, LockManager*> site_lock_map;
    // variable and the sites they are present
    std::map<int, std::vector<int>> variable_map;

    // for each site the variables that are not commited after recovering
    std::map<int, std::set<int>> unclean_data_on_site;

public:
    DataManager();
    void initializeDataManager();
    void setSite(int siteIdx, int variable, int value, int commit_time);
    std::vector<int> getVariableSites(int variable);
    void printDM();
    OperationResult read( int variable, Transaction txn);
};

#endif