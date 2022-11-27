
#include <iostream>
#include "data_manager.hpp"
using namespace std;

DataManager::DataManager()
{
    sites = {};
}

void DataManager::setSite(int siteIdx, string variable, int value)
{
    sites[siteIdx][variable] = value;
}

void DataManager::printDM()
{
    for (auto it = sites.begin(); it != sites.end(); it++)
    {
        cout << "Site " << it->first << " - ";
        for (auto itt = it->second.begin(); itt != it->second.end(); itt++){
            cout<< itt->first << ": " << itt->second << ", ";
        }
        cout << "\n";
    }
}