#ifndef DATAMANAGER
#define DATAMANAGER

#include <string>
#include <map>
#include <unordered_map>
#include "lock_manager.hpp"


enum D_STATUS
{
    d_down = 2,
    d_up = 4,
    d_recovering= 8,
};


class DataManager
{
private:
    // data info about each site site_id -> map [ variable -> value, commitTime] 
    std::map<int, std::map<int, std::pair<int,int>>> sites;
    // status info about each site site_id -> map [ site_status -> last time status changed] 
    std::map<int, std::map<int, D_STATUS>> sites_status;
    // Lock Manager instance that holds locks for each site
    std::map<int, LockManager*> site_lock_map;

public:
    DataManager();
    void initializeDataManager();
    void setSite(int siteIdx, int variable, int value, int commit_time);
    void printDM();
};

#endif