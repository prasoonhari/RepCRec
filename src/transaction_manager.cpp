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
        siteMap[site].dm->initializeLockTable();
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
        int variable_value = stoi(Op.vars[2]);
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

void TransactionManager::printDump() {
    for (auto site: siteMap) {
        site.second.dm->printDM();
        site.second.dm->printLM();
    }
}

OperationResult TransactionManager::readOperation(Operation Op, int time) {
    Transaction *currentTxn = getTransactionFromOperation(Op);
    return read(currentTxn, time);
}

OperationResult TransactionManager::read(Transaction *currentTxn, int time) {


    OperationResult OperRes;
    int variable_id = currentTxn->currentInstruction.values[0];

    if (currentTxn->status == T_STATUS::t_aborting) {
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "Aborting Transaction Found";
    }

    // If transaction is read-only
    if (currentTxn->ReadOnly) {

    } else {

//        // Check if there is a wait list for this variable other than himself
//        if (transactionWaitingForData.find(variable_id) != transactionWaitingForData.end() &&
//            std::find(transactionWaitingForData[variable_id].begin(), transactionWaitingForData[variable_id].end(), currentTxn->id) ==
//            transactionWaitingForData[variable_id].end()) {
//
//            OperRes.status = RESULT_STATUS::failure;
//            currentTxn->status = T_STATUS::t_blocked;
//            OperRes.msg = "Not able to Read Now in waiting";
//            transactionWaitingForData[variable_id].push_back(currentTxn->id);
//            if (transactionDependency.find(currentTxn->id) == transactionDependency.end()) {
//                transactionDependency[currentTxn->id] = transactionWaitingForData[variable_id];
//            } else {
//                transactionDependency[currentTxn->id].insert(transactionDependency[currentTxn->id].end(),
//                                                             transactionWaitingForData[variable_id].begin(),
//                                                             transactionWaitingForData[variable_id].end());
//            }
//            // TODO: Check Deadlock Here
//
//            return OperRes;
//        }
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

                    OperRes.status = RESULT_STATUS::failure;
                    currentTxn->status = T_STATUS::t_blocked;
                    OperRes.msg = "Not able to Read Now in waiting";

                    if (transactionWaitingForData.find(variable_id) == transactionWaitingForData.end()) {
                        transactionWaitingForData[variable_id] = {currentTxn->id};
                    } else {
                        transactionWaitingForData[variable_id].push_back(currentTxn->id);
                    }
                    if (transactionDependency.find(currentTxn->id) == transactionDependency.end()) {
                        transactionDependency[currentTxn->id] = {txnRes.transactions[0]};
                    } else {
                        transactionDependency[currentTxn->id].push_back(txnRes.transactions[0]);
                    }

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

// Checks if all locks are available to complete the write
// If not then it return the Txn's that are blocking the write
pair<bool, vector<int>> TransactionManager::isWritePossible(int variable, Transaction *currentTxn) {
    set<int> blockingTransaction;
    for (auto var_site: variableMap[variable]) {
        if (siteMap[var_site].status == SITE_STATUS::up) {
            pair<int, vector<int>> writeCheck = siteMap[var_site].dm->writeCheck(variable, currentTxn);
            if (writeCheck.first == 0) {
                return {false, writeCheck.second};
            } else if (writeCheck.first == 1) {
                set<int> s(writeCheck.second.begin(), writeCheck.second.end());
                blockingTransaction.insert(s.begin(), s.end());
            }
        }
    }
    if (blockingTransaction.empty()) {
        return {true, {}};
    } else {
        return {false, {}};
    }
}

OperationResult TransactionManager::writeOperation(Operation Op, int time) {
    OperationResult OperRes;
    Transaction *currentTxn = getTransactionFromOperation(Op);
    return write(currentTxn, time);
}

OperationResult TransactionManager::write(Transaction *currentTxn, int time) {

    OperationResult OperRes;
    int variable_id = currentTxn->currentInstruction.values[0];
    // If transaction is read-only

    if (currentTxn->status == T_STATUS::t_aborting) {
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "Aborting Transaction Found";
    }

    vector<int> UpdatableSites;
    for (auto var_site: variableMap[variable_id]) {
        if (siteMap[var_site].status == SITE_STATUS::up) {
            UpdatableSites.push_back(var_site);
        }
    }
    if (UpdatableSites.empty()) {
        // No site found to write
        currentTxn->status = T_STATUS::t_waiting;
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "No Site Available to Write";
        return OperRes;
    }

    pair<bool, vector<int>> isWritePossibleRes = isWritePossible(variable_id, currentTxn);


    if (isWritePossibleRes.first) {
        for (auto var_site: variableMap[variable_id]) {
            if (siteMap[var_site].status == SITE_STATUS::up) {
                siteMap[var_site].dm->write(variable_id, currentTxn, time);
                if (currentTxn->dirtyData.find(var_site) == currentTxn->dirtyData.end()) {
                    // empty because it's a read data so won't get dirty
                    currentTxn->dirtyData[var_site] = {variable_id};
                } else {
                    currentTxn->dirtyData[var_site].push_back(variable_id);
                }
            }
        }

        OperRes.status = RESULT_STATUS::success;
        return OperRes;
    } else {

        OperRes.status = RESULT_STATUS::failure;
        currentTxn->status = T_STATUS::t_blocked;
        OperRes.msg = "Not able to Read Now in waiting";

        if (transactionWaitingForData.find(variable_id) == transactionWaitingForData.end()) {
            transactionWaitingForData[variable_id] = {currentTxn->id};
        } else {
            transactionWaitingForData[variable_id].push_back(currentTxn->id);
        }
        if (transactionDependency.find(currentTxn->id) == transactionDependency.end()) {
            transactionDependency[currentTxn->id] = {isWritePossibleRes.second};
        } else {
            transactionDependency[currentTxn->id].insert(transactionWaitingForData[currentTxn->id].end(),
                                                         isWritePossibleRes.second.begin(),
                                                         isWritePossibleRes.second.end());
        }
        return OperRes;
    }
}


void TransactionManager::endTransaction(Operation Op, int time) {
    int txnToEnd = extractId(Op.vars[0]);
    Transaction *currentTxn = &transactions[txnToEnd];
    if (currentTxn->status == T_STATUS::t_aborting) {
        // TODO: Abort transaction
        abortTransaction( currentTxn, time);
        cout << "Transaction T" << currentTxn->id << " aborted \n";
    } else {
        // TODO Commit
        commitTransaction(currentTxn, time);
        cout << "Transaction T" << currentTxn->id << " committed \n";
    }


}

void TransactionManager::tryExecutionAgain(vector<int> txns, int time) {
    bool somethingChanged = false;
    for (auto txn : txns){
        if (transactions[txn].status == T_STATUS::t_blocked){
            transactions[txn].status = T_STATUS::t_running;
            if (transactions[txn].currentInstruction.type == INST_TYPE::Read){
                OperationResult operRes = read(&transactions[txn], time);
                cout << operRes.msg << "\n";
            } else if (transactions[txn].currentInstruction.type == INST_TYPE::Write){
                write(&transactions[txn], time);
            }
        }
    }

    if (!somethingChanged){
        // TODO : Check Deadlock
    }

}

void TransactionManager::commitTransaction(Transaction *currentTxn, int commit_time) {

    // Make change by the current Txn Permanent
    set<int> releasedVariable;
    for (auto x: currentTxn->dirtyData) {
        for (auto variable: x.second) {
            // Make change by the current Txn Permanent
            siteMap[x.first].dm->setDataCommit(variable, commit_time);
            // release all locks in the site visited by this txn
            vector<int> allReleaseLock = siteMap[x.first].dm->releaseLock(variable, currentTxn->id);
            std::set<int> s(allReleaseLock.begin(), allReleaseLock.end());
            releasedVariable.insert(s.begin(), s.end());
        }
    }


    // Now find all the txns that were waiting of this variable
    vector<int> transactionWaitingForReleasedData = {};
    set<int> transactionSetWaitingForReleasedData = {};

    for (auto var : releasedVariable){
        for(auto txn_id : transactionWaitingForData[var]){
            if (transactionSetWaitingForReleasedData.find(txn_id) ==transactionSetWaitingForReleasedData.end() ){
                transactionWaitingForReleasedData.push_back(txn_id);
                transactionSetWaitingForReleasedData.insert(txn_id);
            }
        }
        transactionWaitingForData.erase(var);
    }

    tryExecutionAgain(transactionWaitingForReleasedData, commit_time);
}

void TransactionManager::abortTransaction(Transaction *currentTxn, int abort_time) {
    // Make change by the current Txn Permanent
    set<int> releasedVariable;
    for (auto x: currentTxn->dirtyData) {
        for (auto variable: x.second) {
            // Revert change by the current Txn back
            siteMap[x.first].dm->setDataRevert(variable);
            // release all locks in the site visited by this txn if any (will be in case of deadlock breaking)
            vector<int> allReleaseLock = siteMap[x.first].dm->releaseLock(variable, currentTxn->id);
            std::set<int> s(allReleaseLock.begin(), allReleaseLock.end());
            releasedVariable.insert(s.begin(), s.end());
        }
    }


    // Now find all the txns that were waiting of this variable
    vector<int> transactionWaitingForReleasedData = {};
    set<int> transactionSetWaitingForReleasedData = {};

    for (auto var : releasedVariable){
        for(auto txn_id : transactionWaitingForData[var]){
            if (transactionSetWaitingForReleasedData.find(txn_id) == transactionSetWaitingForReleasedData.end() ){
                transactionWaitingForReleasedData.push_back(txn_id);
                transactionSetWaitingForReleasedData.insert(txn_id);
            }
        }
        transactionWaitingForData.erase(var);
    }

    tryExecutionAgain(transactionWaitingForReleasedData, abort_time);
}


void TransactionManager::failSite(Operation Op, int time) {
    int siteToFail = stoi(Op.vars[0]);
    // Reset the lock table
    siteMap[siteToFail].dm->failThisSite();
    // mark abort all the transactions that used this site;
    for (auto txn: transactions) {
        if (txn.second.dirtyData.find(siteToFail) != txn.second.dirtyData.end()) {
            transactions[txn.first].status = T_STATUS::t_aborting;
        }
    }
    siteMap[siteToFail].failedHistory.push_back(time);
    siteMap[siteToFail].status = SITE_STATUS::down;
}



