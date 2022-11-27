#include <iostream>
#include "transaction_manager.hpp"
#include "data_manager.hpp"

using namespace std;



int extractTransactionId(string opString)
{
    string idx = opString.substr(1, opString.size() - 1);
    return stoi(idx);
}

void TransactionManager::initializeDataManager(DataManager &dm)
{
    for (int site = 1; site <= 10; site++)
    {
        dm.initializeSite(site);
    }
    for (int var = 1; var <= 20; var++)
    {
        if (var & 1)
        {
            dm.setSite(1 + var % 10, var, 10 * var);
        }
        else
        {
            for (int i = 1; i <= 10; i++)
            {
                dm.setSite(i, var, 10 * var);
            }
        }
    }
}

TransactionManager::TransactionManager()
{
    dataManager = new DataManager();
    initializeDataManager(*dataManager);
    transactions = {};
}

void TransactionManager::printTM()
{
    for (auto itr = transactions.begin(); itr != transactions.end(); itr++)
    {
        cout << "T" << (*itr).id << ": StartTime = " << (*itr).startTime << " "
             << ": ReadOnly = " << (*itr).ReadOnly << " "
             << ": status = " << (*itr).status << " ";
        cout << endl;
    }
   
}

void TransactionManager::begin(Operation Op, int time)
{
    Transaction txn;
    txn.id = extractTransactionId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = false;
    txn.status = running;
    transactions.insert(txn);
}

void TransactionManager::beginRO(Operation Op, int time)
{
    Transaction txn;
    txn.id = extractTransactionId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = true;
    txn.status = running;
    transactions.insert(txn);
}