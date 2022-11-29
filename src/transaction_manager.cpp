#include <iostream>
#include "transaction_manager.hpp"
#include "data_manager.hpp"

using namespace std;



int extractId(string opString)
{
    string idx = opString.substr(1, opString.size() - 1);
    return stoi(idx);
}


TransactionManager::TransactionManager()
{
    dataManager = new DataManager();
    dataManager->initializeDataManager();
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
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = false;
    txn.status = t_running;
    transactions.insert(txn);
}

void TransactionManager::beginRO(Operation Op, int time)
{
    Transaction txn;
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = true;
    txn.status = t_running;
    transactions.insert(txn);
}

void TransactionManager::read(Operation Op, int time)
{
    int txn_id = extractId(Op.vars[0]);
    int variable_id = extractId(Op.vars[1]);
  
}