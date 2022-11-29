#include <iostream>
#include "transaction_manager.hpp"
#include "data_manager.hpp"

using namespace std;

void TransactionManager::initializeDB()
{
    for (int site = 1; site <= 10; site++)
    {
        siteMap[site] = {new DataManager(site), SITE_STATUS::up, 0};
    }

    for (int var = 1; var <= 20; var++)
    {
        if (var & 1)
        {
            siteMap[1 + var % 10].dm->setData(var, 10 * var, 0);
            variableMap[var] = {1 + var % 10};
        }
        else
        {
            variableMap[var] = {};
            for (int i = 1; i <= 10; i++)
            {
                siteMap[i].dm->setData(var, 10 * var, 0);
                variableMap[var].push_back(i);
            }
        }
    }

    for (int site = 1; site <= 10; site++)
    {
        siteMap[site].dm->initializeDataManager();
    }
}

int extractId(string opString)
{
    string idx = opString.substr(1, opString.size() - 1);
    return stoi(idx);
}

TransactionManager::TransactionManager()
{
    siteMap = {};
    transactions = {};
    variableMap = {};
}



void TransactionManager::begin(Operation Op, int time)
{
    Transaction txn;
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = false;
    txn.status = t_running;
    transactions[txn.id] = txn;
}

void TransactionManager::beginRO(Operation Op, int time)
{
    Transaction txn;
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = true;
    txn.status = t_running;
    transactions[txn.id] = txn;
}

OperationResult TransactionManager::read(Operation Op, int time)
{
    int txn_id = extractId(Op.vars[0]);
    int variable_id = extractId(Op.vars[1]);
    transactions[txn_id].currentInstruction.type = INST_TYPE::Read;
    transactions[txn_id].currentInstruction.values = {variable_id};
    transactions[txn_id];

    // search in all the sites that the variable is present in
    for (auto var_site : variableMap[variable_id])
    {
        if (transactions[txn_id].ReadOnly)
        {
            // If xi is not replicated and the site holding xi is up,
            // then the read-only transaction can read it. Because that is the only site that knows about xi.
            if (variableMap[variable_id].size() == 1 && siteMap[var_site].status == SITE_STATUS::up)
            {
                OperationResult opRes;
                opRes.status = RESULT_STATUS::success;
                opRes.msg = "x" + to_string(variable_id) + ": " + to_string(siteMap[var_site].dm->readRO(variable_id, transactions[txn_id]));
                return opRes;
            }
            // If xi is replicated then RO can read xi from site s if xi was committed at s by some transaction T’ 
            // before RO began and s was up all the time between the time when xi was commited and RO began. 
            // In that case RO can read the version that T’ wrote. If there is no such site then RO can abort

            //TODO: add check for site failure between last commit and Txn start 
            else if (variableMap[variable_id].size() != 1 &&
            transactions[txn_id].startTime > siteMap[var_site].dm->getLastCommittedTime(variable_id)){

            }

        }
        else
        {
            // check if the site was up (that means it has committed data) and if the transaction
            // is not txn.ReadOnly then also check if we can get lock
            if (siteMap[var_site].status == SITE_STATUS::up)
            {
                
                OperationResult OpRn = siteMap[var_site].dm->read(variable_id, transactions[txn_id]);
                if (OpRn.status == RESULT_STATUS::success)
                {
                    return OpRn;
                }
            }
            // if the site is recovering (that means it has uncommitted data) then further check where the data(variable)
            // is unreplicated else check if the replicated data has been commited using unclean_data_on_site that has info about
            // all the data that is currently uncommited in the site.
            else if (siteMap[var_site].status == SITE_STATUS::recovering &&
                     (variableMap[variable_id].size() == 1 || siteMap[var_site].dm->checkIfDataRecoved(variable_id)))
            {
                OperationResult OpRn = siteMap[var_site].dm->read(variable_id, transactions[txn_id]);
                if (OpRn.status == RESULT_STATUS::success)
                {
                    return OpRn;
                }
            }
        }
    }

    // If we haven't found any readable data then return error msg
    
    OperationResult opRes;
    opRes.status = RESULT_STATUS::failure;
    opRes.msg = "Cannot find a suitable site to read";
    return opRes;
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