
#include <iostream>
#include "data_manager.hpp"
#include "lock_manager.hpp"
using namespace std;

DataManager::DataManager()
{
    sites = {};
}

void DataManager::initializeDataManager()
{
    for (int site = 1; site <= 10; site++)
    {
        sites[site] = {};
        site_lock_map[site] = new LockManager();
        sites_status[site] = {SITE_STATUS::up, 0};
        unclean_data_on_site[site] = {};
    }
    for (int var = 1; var <= 20; var++)
    {
        if (var & 1)
        {

            sites[1 + var % 10][var] = {10 * var, 0};
            site_lock_map[1 + var % 10]->initializeLock(var);
            variable_map[var] = {1 + var % 10};
        }
        else
        {
            variable_map[var] = {};
            for (int i = 1; i <= 10; i++)
            {
                sites[i][var] = {10 * var, 0};
                site_lock_map[i]->initializeLock(var);
                variable_map[var].push_back(i);
            }
        }
    }
}

void DataManager::setSite(int siteIdx, int variable, int value, int commit_time)
{
    sites[siteIdx][variable] = {value, commit_time};
}

vector<int> DataManager::getVariableSites(int variable)
{
    return variable_map[variable];
}


OperationResult DataManager::read(int variable, Transaction txn)
{

    // search in all the sites that the variable is present in
    for (auto var_site : variable_map[variable])
    {
        if (txn.ReadOnly)
        {
            // If xi is not replicated and the site holding xi is up,
            // then the read-only transaction can read it. Because that is the only site that knows about xi.
            if (variable_map[variable].size() == 1 && sites_status[var_site].first == SITE_STATUS::up)
            {
                OperationResult opRes;
                opRes.status = RESULT_STATUS::success;
                opRes.msg = "x" + to_string(variable) + ": " + to_string(sites[var_site][variable].first);
                return opRes;
            }
            // if the site is recovering (that means it has uncommitted data) then further check where the data(variable)
            // is unreplicated else check if the replicated data has been commited using unclean_data_on_site that has info about
            // all the data that is currently uncommited in the site.
            else if (variable_map[variable].size() != 1 &&
             sites[var_site][variable].second &&
            sites_status[var_site].first == SITE_STATUS::up)
            {
                OperationResult opRes;
                opRes.status = RESULT_STATUS::success;
                opRes.msg = "x" + to_string(variable) + ": " + to_string(sites[var_site][variable].first);
                return opRes;
            }
        }
        else
        {
            // check if the site was up (that means it has committed data) and if the transaction
            // is not txn.ReadOnly then also check if we can get lock
            if (sites_status[var_site].first == SITE_STATUS::up && (site_lock_map[var_site]->getReadLock(variable, txn.id) == 1))
            {
                OperationResult opRes;
                opRes.status = RESULT_STATUS::success;
                opRes.msg = "x" + to_string(variable) + ": " + to_string(sites[var_site][variable].first);
                return opRes;
            }
            // if the site is recovering (that means it has uncommitted data) then further check where the data(variable)
            // is unreplicated else check if the replicated data has been commited using unclean_data_on_site that has info about
            // all the data that is currently uncommited in the site.
            else if (sites_status[var_site].first == SITE_STATUS::recovering &&
                     (variable_map[variable].size() == 1 || unclean_data_on_site[var_site].find(variable) == unclean_data_on_site[var_site].end()) &&
                     (site_lock_map[var_site]->getReadLock(variable, txn.id) == 1))
            {
                OperationResult opRes;
                opRes.status = RESULT_STATUS::success;
                opRes.msg = "x" + to_string(variable) + ": " + to_string(sites[var_site][variable].first);
                return opRes;
            }
        }
    }

    // If we haven't found any readable data then return error msg
    OperationResult opRes;
    opRes.status = RESULT_STATUS::failure;
    opRes.msg = "Cannot find a suitable site to read";
    return opRes;
}

void DataManager::printDM()
{
    for (auto it = sites.begin(); it != sites.end(); it++)
    {
        cout << "Site " << it->first << " - ";
        for (auto itt = it->second.begin(); itt != it->second.end(); itt++)
        {
            cout << "x" << itt->first << ": " << itt->second.first << ", ";
        }
        cout << "\n";
    }
}