
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
    }
    for (int var = 1; var <= 20; var++)
    {
        if (var & 1)
        {
            sites[1 + var % 10][var] = {10 * var,0};
            site_lock_map[1 + var % 10]->initializeLock(var);
        }
        else
        {
            for (int i = 1; i <= 10; i++)
            {
                sites[i][var] = {10 * var,0};
                site_lock_map[i]->initializeLock(var);
            }
        }
    }
}


void DataManager::setSite(int siteIdx, int variable, int value, int commit_time)
{
    sites[siteIdx][variable] = {value,commit_time};
}

void DataManager::printDM()
{
    for (auto it = sites.begin(); it != sites.end(); it++)
    {
        cout << "Site " << it->first << " - ";
        for (auto itt = it->second.begin(); itt != it->second.end(); itt++)
        {
            cout << "x"<< itt->first << ": " << itt->second.first << ", ";
        }
        cout << "\n";
    }
}