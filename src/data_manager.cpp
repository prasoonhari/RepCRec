
#include <iostream>
#include "data_manager.hpp"
#include "lock_manager.hpp"
using namespace std;

DataManager::DataManager(int _site_id)
{
    site_id = _site_id;
    data = {};
    unclean_data_on_site = {};
    lm = new LockManager(_site_id);
}

void DataManager::initializeDataManager()
{
    for (auto x : data)
    {
        lm->initializeLock(x.first);
    }
}

void DataManager::setData(int variable, int value, int commit_time)
{
    data[variable] = {value, value, commit_time};
}

int DataManager::readRO(int variable, Transaction txn)
{
    data[variable].committedValue;
}

int DataManager::getLastCommittedTime(int variable)
{
    data[variable].lastCommitTime;
}

int DataManager::getReadLockStatus(int variable, Transaction txn){
    lm->getReadLockStatus(variable, txn.id);
}

int DataManager::getWriteLockStatus(int variable, Transaction txn){
    lm->getWriteLockStatus(variable, txn.id);
}

OperationResult DataManager::read(int variable, Transaction txn)
{
    OperationResult opRes;
    int lock_status = (lm->getReadLock(variable, txn.id));
    if (lock_status == 1)
    {
        opRes.status = RESULT_STATUS::success;
        opRes.msg = "x" + to_string(variable) + ": " + to_string(data[variable].committedValue);
        return opRes;
    } else if (lock_status == 2){
        opRes.status = RESULT_STATUS::success;
        opRes.msg =  "In waiting Queue";
        return opRes;
    }
    opRes.status = RESULT_STATUS::failure;
    return opRes;
}

OperationResult DataManager::write(int variable, Transaction txn)
{
    OperationResult opRes;
    if ((lm->getWriteLock(variable, txn.id) == 1))
    {
        opRes.status = RESULT_STATUS::success;
        opRes.msg = "x" + to_string(variable) + ": " + to_string(data[variable].committedValue);
        return opRes;
    }
    opRes.status = RESULT_STATUS::failure;
    return opRes;
}

bool DataManager::checkIfDataRecovered(int variable)
{
    return unclean_data_on_site.find(variable) == unclean_data_on_site.end();
}

void DataManager::printDM()
{

    for (auto itt = data.begin(); itt != data.end(); itt++)
    {
        cout << "x" << itt->first << ": " << itt->second.committedValue << ", ";
    }
    cout << "\n";
}