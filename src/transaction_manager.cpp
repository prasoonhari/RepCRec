#include <iostream>
#include "transaction_manager.hpp"
#include "data_manager.hpp"

using namespace std;

//--------------------------------------------Initialize-------------------------------------------------

void TransactionManager::initializeDB() {
    // Creating 10 sites with datamanager
    for (int site = 1; site <= 10; site++) {
        siteMap[site] = {new DataManager(site), SITE_STATUS::up, 0};
    }


    // Setting the initial data value in each site
    for (int var = 1; var <= 20; var++) {
        if (var & 1) {
            siteMap[1 + var % 10].dm->setData(var, 10 * var, 0);
            variableMap[var] = {1 + var % 10};
        } else {
            variableMap[var] = {};
            for (int i = 1; i <= 10; i++) {
                siteMap[i].dm->setData(var, 10 * var, 0);
                variableMap[var].push_back(i);
            }
        }
    }

    // Initializing the 10 datamanager -> will create the lockManager (lock_table) for each site.
    for (int site = 1; site <= 10; site++) {
        siteMap[site].dm->initializeDataManager();
    }
}

//--------------------------------------------Util Methods-------------------------------------------------

void TransactionManager::printTM() {
    for (auto itr = transactions.begin(); itr != transactions.end(); itr++) {
        cout << "T" << (*itr).second.id << ": StartTime = " << (*itr).second.startTime << " "
             << ": ReadOnly = " << (*itr).second.ReadOnly << " "
             << ": status = " << (*itr).second.status << " ";
        cout << endl;
    }
}

int extractId(string opString) {
    string idx = opString.substr(1, opString.size() - 1);
    return stoi(idx);
}

TransactionManager::TransactionManager() {
    siteMap = {};
    transactions = {};
    variableMap = {};
}

Transaction *TransactionManager::getTransactionFromOperation(Operation Op) {
    if (Op.type == CMD_TYPE::R) {
        int txn_id = extractId(Op.vars[0]);
        int variable_id = extractId(Op.vars[1]);
        transactions[txn_id].currentInstruction.type = INST_TYPE::Read;
        transactions[txn_id].currentInstruction.values = {variable_id};
        return &transactions[txn_id];
    } else if (Op.type == CMD_TYPE::W) {
        int txn_id = extractId(Op.vars[0]);
        int variable_id = extractId(Op.vars[1]);
        int variable_value = extractId(Op.vars[2]);
        transactions[txn_id].currentInstruction.type = INST_TYPE::Write;
        transactions[txn_id].currentInstruction.values = {variable_id, variable_value};
        return &transactions[txn_id];
    }
}



//--------------------------------------------Transaction Methods-------------------------------------------------


void TransactionManager::begin(Operation Op, int time) {
    Transaction txn;
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = false;
    txn.status = t_running;
    txn.dirtyData = {};
    transactions[txn.id] = txn;
}

void TransactionManager::beginRO(Operation Op, int time) {
    Transaction txn;
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = true;
    txn.status = t_running;
    txn.dirtyData = {};
    transactions[txn.id] = txn;
}

OperationResult TransactionManager::read(Operation Op, int time) {


    OperationResult OperRes;
    Transaction *currentTxn = getTransactionFromOperation(Op);
    int variable_id = currentTxn->currentInstruction.values[0];
    // If transaction is read-only
    if (currentTxn->ReadOnly) {

    } else {
        // For all the sites that has this variable
        for (auto var_site: variableMap[variable_id]) {
            // If site is up or it is recovering and the data is ok to read
            if (siteMap[var_site].status == SITE_STATUS::up || (siteMap[var_site].status == SITE_STATUS::recovering &&
                                                                (variableMap[variable_id].size() == 1 ||
                                                                 siteMap[var_site].dm->checkIfDataRecovered(
                                                                         variable_id)))) {
                TransactionResult txnRes = siteMap[var_site].dm->read(variable_id, currentTxn);

                if (txnRes.status == RESULT_STATUS::success) {
                    if (currentTxn->dirtyData.find(var_site) == currentTxn->dirtyData.end()) {
                        // empty because it's a read data so won't get dirty
                        currentTxn->dirtyData[var_site] = {};
                    }
                    OperRes.status = RESULT_STATUS::success;
                    return OperRes;
                } else {
                    if (transactionWaiting.find(variable_id) == transactionWaiting.end()) {
                        transactionWaiting[variable_id] = {currentTxn->id};
                    } else {
                        transactionWaiting[variable_id].push_back(currentTxn->id);
                    }
                    if (transaction_wait_table.find(currentTxn->id) == transactionWaiting.end()) {
                        transactionWaiting[currentTxn->id] = {txnRes.transactions[0]};
                    } else {
                        transactionWaiting[currentTxn->id].push_back(txnRes.transactions[0]);
                    }
                    OperRes.status = RESULT_STATUS::failure;
                    OperRes.msg = "Not able to Read Now in waiting";
                    return OperRes;
                }
            }
        }

        // No site found to read from thus txn is blocked now
        currentTxn->status = T_STATUS::t_waiting;
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "No Site Available to Read";
        return OperRes;
    }
}


pair<bool, vector<int>> TransactionManager::isWritePossible(int variable, Transaction *currentTxn){
    vector<int> blockingTransaction;
    for (auto var_site: variableMap[variable]) {
        if (siteMap[var_site].status == SITE_STATUS::up){
            TransactionResult txnRes = siteMap[var_site].dm->writeCheck(variable, currentTxn);
        }
    }
}


OperationResult TransactionManager::write(Operation Op, int time) {

    OperationResult OperRes;
    Transaction *currentTxn = getTransactionFromOperation(Op);
    int variable_id = currentTxn->currentInstruction.values[0];
    // If transaction is read-only
    for (auto var_site: variableMap[variable_id]) {
    }

    // No site found to read from thus txn is blocked now
    currentTxn->status = T_STATUS::t_waiting;
    OperRes.status = RESULT_STATUS::failure;
    OperRes.msg = "No Site Available to Read";
    return OperRes;
}


