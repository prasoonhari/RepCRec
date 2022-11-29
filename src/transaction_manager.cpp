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
        cout << "T" << (*itr).second.id << ": StartTime = " << (*itr).second.startTime << " "
             << ": ReadOnly = " << (*itr).second.ReadOnly << " "
             << ": status = " << (*itr).second.status << " ";
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
    transactions[txn.id]= txn;
}

void TransactionManager::beginRO(Operation Op, int time)
{
    Transaction txn;
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = true;
    txn.status = t_running;
    transactions[txn.id]= txn;
}

OperationResult TransactionManager::read(Operation Op, int time)
{
    int txn_id = extractId(Op.vars[0]);
    int variable_id = extractId(Op.vars[1]);
    transactions[txn_id].currentInstruction.type = INST_TYPE::Read;
    transactions[txn_id].currentInstruction.values = {variable_id};

  
}