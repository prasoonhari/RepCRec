#include <iostream>
#include <utility>
#include "transaction_manager.hpp"
#include "data_manager.hpp"

using namespace std;

TransactionManager::TransactionManager() {
    siteMap = {};
    transactions = {};
    variableMap = {};
    deadlockMightOccur = true;
    deadLockManager = new DeadLockManager();
}

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

void TransactionManager::resolveDeadlock(int time) {
    vector<int> transactionInCycle = deadLockManager->deadlockDetector(transactionDependency);
    if (transactionInCycle.empty()){
        return;
    }
    Transaction *youngestTxn;
    int latestTime = -1;
    for (auto txn : transactionInCycle){
        if (transactions[txn].startTime > latestTime){
            youngestTxn = &transactions[txn];
            latestTime = max(latestTime, transactions[txn].startTime);
        }
    }
    transactionDependency.erase(youngestTxn->id);
    youngestTxn->status = T_STATUS::t_aborting;
    abortTransaction(youngestTxn, time);
}

void TransactionManager::printTM() {
    for (auto & transaction : transactions) {
        cout << "T" << transaction.second.id << ": StartTime = " << transaction.second.startTime << " "
             << ": ReadOnly = " << transaction.second.ReadOnly << " "
             << ": status = " << transaction.second.status << " ";
        cout << endl;
    }
}

int extractId(const string& opString) {
    string idx = opString.substr(1, opString.size() - 1);
    return stoi(idx);
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

map<int, int> TransactionManager::CreateSnapshot() {
    map<int, int> snapshot;
    for (const auto& varMap: variableMap) {
        if (snapshot.find(varMap.first) == snapshot.end()) {
            for (auto var_site: varMap.second) {
                if (siteMap[var_site].status == SITE_STATUS::up) {
                    snapshot[varMap.first] = siteMap[var_site].dm->readData(varMap.first);
                }
                break;
            }
        }
    }

    return snapshot;
}

void TransactionManager::beginRO(Operation Op, int time) {
    Transaction txn;
    txn.id = extractId(Op.vars[0]);
    txn.startTime = time;
    txn.ReadOnly = true;
    txn.status = t_running;
    txn.dirtyData = {};
    txn.snapShot = CreateSnapshot();
    transactions[txn.id] = txn;
}

void TransactionManager::printDump() {
    for (auto site: siteMap) {
        site.second.dm->printDM();
        // TODO If you want to see lock manager status as well
//        site.second.dm->printLM();
    }
}

OperationResult TransactionManager::readOperation(Operation Op, int time) {
    Transaction *currentTxn = getTransactionFromOperation(std::move(Op));
    return read(currentTxn, time);
}

bool TransactionManager::blockedByWaitlist(int variable_id, int txn_id) {
    if (transactionWaitingForData.find(variable_id) != transactionWaitingForData.end() &&
        transactionWaitingForData[variable_id][0] != txn_id) {


        if (std::find(transactionWaitingForData[variable_id].begin(), transactionWaitingForData[variable_id].end(),
                      txn_id) ==
            transactionWaitingForData[variable_id].end()) {
            transactionWaitingForData[variable_id].push_back(txn_id);
            if (transactionDependency.find(txn_id) == transactionDependency.end()) {
                transactionDependency[txn_id] = transactionWaitingForData[variable_id];
            } else {
//                transactionDependency[txn_id].insert(transactionDependency[txn_id].end(),
//                                                     transactionWaitingForData[variable_id].begin(),
//                                                     transactionWaitingForData[variable_id].end());
                for (auto x: transactionWaitingForData[variable_id]) {
                    transactionDependency[txn_id].push_back(x);
                }
            }
            deadlockMightOccur = true;
        }


        return true;
    }
    return false;
}

bool TransactionManager::readCondition(int var_site, int variable_id) {
    return siteMap[var_site].status == SITE_STATUS::up || (siteMap[var_site].status == SITE_STATUS::recovering &&
                                                           (variableMap[variable_id].size() == 1 ||
                                                            siteMap[var_site].dm->checkIfDataRecovered(
                                                                    variable_id)));
}

// Returns the value for a RO Txn from the snapshot taken when this Txn beagan
// If we don't find a variable here that means when the Txn began there was no site up after the last txn committed
OperationResult TransactionManager::readOnly(Transaction *currentTxn, int variable_id) {
    OperationResult OperRes;
    if (currentTxn->snapShot.find(variable_id) != currentTxn->snapShot.end()) {
        cout << "T" << currentTxn->id << " reads x" + to_string(variable_id) + ": "
             << currentTxn->snapShot[variable_id] << "\n";
        OperRes.status = RESULT_STATUS::success;
        return OperRes;
    } else {
//                    return readOnly( currentTxn,  var_site,  variable_id);
        currentTxn->status = T_STATUS::t_aborting;
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "No site was up since a transaction commits" + to_string(variable_id) + "\n";
        return OperRes;
    }
}

// Maintains the maps that is used to process the txns that will be waiting
void TransactionManager::processBlockingTransaction(Transaction *currentTxn, int variable_id, const vector<int>& blockingTxns){
    if (transactionWaitingForData.find(variable_id) == transactionWaitingForData.end()) {
        transactionWaitingForData[variable_id] = {currentTxn->id};
    } else {
        transactionWaitingForData[variable_id].push_back(currentTxn->id);
    }
    if (transactionDependency.find(currentTxn->id) == transactionDependency.end()) {
        transactionDependency[currentTxn->id] = {};
        for (auto blkTxn: blockingTxns){
            transactionDependency[currentTxn->id].push_back(blkTxn);
        }
    } else {
        for (auto blkTxn: blockingTxns){
            transactionDependency[currentTxn->id].push_back(blkTxn);
        }
    }

}

void TransactionManager::ProcessTransactionWaitingForSite(Transaction *currentTxn, int variable_id){
    for (auto var_site: variableMap[variable_id]) {
        if (transactionWaitingForSite.find(var_site) == transactionWaitingForSite.end()) {
            transactionWaitingForSite[var_site] = {currentTxn->id};
        }else{
            transactionWaitingForSite[var_site].push_back(currentTxn->id);
        }
    }
}

OperationResult TransactionManager::read(Transaction *currentTxn, int time) {


    OperationResult OperRes;
    int variable_id = currentTxn->currentInstruction.values[0];

    if (currentTxn->status == T_STATUS::t_aborting) {
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "Aborting Transaction Found T" + to_string(currentTxn->id) + " Not doing Read Operation\n";
        return OperRes;
    }

    // If transaction is read-only
    if (currentTxn->ReadOnly) {
        // non replicated data
        if (variableMap[variable_id].size() == 1) {
            int var_site = variableMap[variable_id][0];
//            if (siteMap[var_site].status == SITE_STATUS::up || siteMap[var_site].status == SITE_STATUS::recovering) {
//                return readOnly(currentTxn, variable_id);
//            }
            return readOnly(currentTxn, variable_id);
//            currentTxn->status = T_STATUS::t_aborting;
//            OperRes.status = RESULT_STATUS::failure;
//            OperRes.msg = "No Site Available to Read x" + to_string(variable_id) + "\n";
//            return OperRes;
        }
            // For replicated data
        else {
            bool canReadFromOneSite = false;
            for (auto var_site: variableMap[variable_id]) {
                if (!siteMap[var_site].failedHistory.empty()) {
                    int lastFailIndex = (int)siteMap[var_site].failedHistory.size() - 1;
                    int lastTimeThisSiteFailed = siteMap[var_site].failedHistory[lastFailIndex];
                    int lastTimeThisVariableCommittedAtThisSite = siteMap[var_site].dm->getLastCommittedTime(
                            variable_id);
                    if (lastTimeThisVariableCommittedAtThisSite < currentTxn->startTime &&
                        !(lastTimeThisSiteFailed > lastTimeThisVariableCommittedAtThisSite &&
                          lastTimeThisSiteFailed < currentTxn->startTime)) {
                        canReadFromOneSite = true;
                        break;
                    }
                } else {
                    canReadFromOneSite = true;
                    break;
                }
            }

            if (canReadFromOneSite) {
                return readOnly(currentTxn, variable_id);
            } else {
                currentTxn->status = T_STATUS::t_aborting;
                OperRes.status = RESULT_STATUS::failure;
                OperRes.msg = "No Site was available when this transaction began" + to_string(variable_id) + "\n";
                return OperRes;
            }
        }

    } else {

        // Check if there is a wait list for this variable
        // FIXME: Check if it is required to block in case if a write Txn is waiting on this variable
        if (blockedByWaitlist(variable_id, currentTxn->id)) {
            OperRes.status = RESULT_STATUS::failure;
            currentTxn->status = T_STATUS::t_blocked;
            OperRes.msg = "Blocked by queue - Not able to Read Now in waiting\n";
            return OperRes;
        }

        vector<int> readableSites;
        for (auto var_site: variableMap[variable_id]) {
            if (readCondition(var_site, variable_id)) {
                readableSites.push_back(var_site);
            }
        }
        if (readableSites.empty()) {
            ProcessTransactionWaitingForSite(currentTxn, variable_id);
            // No site found to write thus aborting
            currentTxn->status = T_STATUS::t_waiting;
            OperRes.status = RESULT_STATUS::failure;
            OperRes.msg = "No Site Available to Read";
            return OperRes;
        }
        int noteBlockingTransaction;
        // For all the sites that has this variable
        for (auto var_site: variableMap[variable_id]) {
            // If site is up or it is recovering and the data is ok to read
            if (readCondition(var_site, variable_id)) {
                TransactionResult txnRes = siteMap[var_site].dm->read(variable_id, currentTxn);

                if (txnRes.status == RESULT_STATUS::success) {
                    if (currentTxn->dirtyData.find(var_site) == currentTxn->dirtyData.end()) {
                        // empty because it's a read data so won't get dirty
                        currentTxn->dirtyData[var_site] = {};
                    }
                    OperRes.status = RESULT_STATUS::success;
                    return OperRes;
                } else {
                    noteBlockingTransaction = {txnRes.blockingTransaction[0]};
                }
            }
        }

        OperRes.status = RESULT_STATUS::failure;
        currentTxn->status = T_STATUS::t_blocked;
        OperRes.msg = "Not able to Read Now in waiting\n";
        processBlockingTransaction(currentTxn,  variable_id, {noteBlockingTransaction});

        return OperRes;
    }
}

// Checks if all locks are available to complete a write operation
// If not then it return the Txn's that are blocking a write operation
pair<bool, vector<int>> TransactionManager::isWritePossible(int variable, Transaction *currentTxn) {
    set<int> blockingTransaction;
    for (auto var_site: variableMap[variable]) {
        if (siteMap[var_site].status == SITE_STATUS::up || siteMap[var_site].status == SITE_STATUS::recovering) {
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
        std::vector<int> v(blockingTransaction.begin(), blockingTransaction.end());
        return {false,  v};
    }
}

OperationResult TransactionManager::writeOperation(Operation Op, int time) {
    OperationResult OperRes;
    Transaction *currentTxn = getTransactionFromOperation(std::move(Op));
    return write(currentTxn, time);
}

OperationResult TransactionManager::write(Transaction *currentTxn, int time) {

    OperationResult OperRes;
    int variable_id = currentTxn->currentInstruction.values[0];
    // If transaction is read-only

    if (currentTxn->status == T_STATUS::t_aborting) {
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "Aborting Transaction Found T" + to_string(currentTxn->id) + " Not doing Write Operation\n";
        return OperRes;
    }

    vector<int> UpdatableSites;
    for (auto var_site: variableMap[variable_id]) {
        if (siteMap[var_site].status == SITE_STATUS::up || siteMap[var_site].status == SITE_STATUS::recovering) {
            UpdatableSites.push_back(var_site);
        }
    }
    if (UpdatableSites.empty()) {
        // No site found to write thus aborting
        ProcessTransactionWaitingForSite(currentTxn, variable_id);
        currentTxn->status = T_STATUS::t_waiting;
        OperRes.status = RESULT_STATUS::failure;
        OperRes.msg = "No Site Available to Write \n";
        return OperRes;
    }

    pair<bool, vector<int>> isWritePossibleRes = isWritePossible(variable_id, currentTxn);


    if (isWritePossibleRes.first) {
        for (auto var_site: variableMap[variable_id]) {
            if (siteMap[var_site].status == SITE_STATUS::up || siteMap[var_site].status == SITE_STATUS::recovering) {
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
        OperRes.msg = "Write for x" + to_string(variable_id) + " successful by T" + to_string(currentTxn->id) + "\n";
        return OperRes;
    } else {

        OperRes.status = RESULT_STATUS::failure;
        currentTxn->status = T_STATUS::t_blocked;
        OperRes.msg = "Not able to write Now in waiting\n";
        processBlockingTransaction(currentTxn,  variable_id, isWritePossibleRes.second);
        return OperRes;
    }
}


void TransactionManager::endTransaction(Operation Op, int time) {
    int txnToEnd = extractId(Op.vars[0]);
    Transaction *currentTxn = &transactions[txnToEnd];
    if (currentTxn->status == T_STATUS::t_aborting || currentTxn->status == T_STATUS::t_waiting) {
        // TODO: think if we should add time -- I think no
        abortTransaction(currentTxn, time);
        currentTxn->status == T_STATUS::t_ended;
    } else if(currentTxn->status == T_STATUS::t_running) {
        // TODO: think if we should add time
        commitTransaction(currentTxn, time);
        currentTxn->status == T_STATUS::t_ended;
    } else{
        cout << "Something is wrong : A Txn with wrong status " << currentTxn->status <<" was trying to end\n";
    }
}

void TransactionManager::tryExecutionAgain(const vector<int>& txns, int time) {
    bool somethingChanged = false;
    for (auto txn: txns) {
        if (transactions[txn].status == T_STATUS::t_blocked) {
            transactions[txn].status = T_STATUS::t_running;
            if (transactions[txn].currentInstruction.type == INST_TYPE::Read) {
                OperationResult operRes = read(&transactions[txn], time);
                if(operRes.status == RESULT_STATUS::success){
                    somethingChanged = true;
                }
                cout << operRes.msg;
            } else if (transactions[txn].currentInstruction.type == INST_TYPE::Write) {
                OperationResult operRes = write(&transactions[txn], time);
                if(operRes.status == RESULT_STATUS::success){
                    somethingChanged = true;
                }
                cout << operRes.msg;
            }
        }
    }

    if (!somethingChanged) {
        deadlockMightOccur = true;
    }

}

void TransactionManager::commitTransaction(Transaction *currentTxn, int commit_time) {

    // TODO : check if transaction is ready to be committed
    // Make change by the current Txn Permanent
    set<int> releasedVariable;
    for (const auto& x: currentTxn->dirtyData) {
        // If this is empty that means the txn have just read from data
        // So we nead to just release the locks
        vector<int> allReleaseLock = siteMap[x.first].dm->releaseLock(currentTxn->id);
        std::set<int> s(allReleaseLock.begin(), allReleaseLock.end());
        releasedVariable.insert(s.begin(), s.end());
        for (auto variable: x.second) {
            // Make change by the current Txn Permanent
            siteMap[x.first].dm->setDataCommit(variable, commit_time);
            // release all locks in the site visited by this txn
        }
    }

    cout << "Transaction T" << currentTxn->id << " committed \n";
    // Now find all the txns that were waiting of this variable

//    set<int> transactionSetWaitingForReleasedData = {};
    // Created this set because multiple variable might have been released and the same transaction might be waiting for
    // both - So we avoid duplication
    // TODO: Does order matter?? Yes
    for (auto var: releasedVariable) {
        vector<int> transactionWaitingForReleasedData = {};
        for (auto txn_id: transactionWaitingForData[var]) {
            transactionWaitingForReleasedData.push_back(txn_id);
        }
        transactionWaitingForData.erase(var);
        tryExecutionAgain(transactionWaitingForReleasedData, commit_time);
    }


}

void TransactionManager::abortTransaction(Transaction *currentTxn, int abort_time) {
    // Make change by the current Txn Permanent
    set<int> releasedVariable;
    for (const auto& x: currentTxn->dirtyData) {
        // release all locks in the site visited by this txn if any (will be in case of deadlock breaking)
        vector<int> allReleaseLock = siteMap[x.first].dm->releaseLock(currentTxn->id);
        std::set<int> s(allReleaseLock.begin(), allReleaseLock.end());
        releasedVariable.insert(s.begin(), s.end());
        for (auto variable: x.second) {
            // Revert change by the current Txn back
            siteMap[x.first].dm->setDataRevert(variable);
        }
    }

    cout << "Transaction T" << currentTxn->id << " aborted \n";
    // Now find all the txns that were waiting of this variable


    for (auto var: releasedVariable) {
        vector<int> transactionWaitingForReleasedData = {};
        for (auto txn_id: transactionWaitingForData[var]) {
            transactionWaitingForReleasedData.push_back(txn_id);
        }
        transactionWaitingForData.erase(var);
        tryExecutionAgain(transactionWaitingForReleasedData, abort_time);
    }


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


void TransactionManager::recoverSite(Operation Op, int time) {
    int siteToRecover = stoi(Op.vars[0]);
    // Reset the lock table
    if (siteMap[siteToRecover].status == SITE_STATUS::down) {

        siteMap[siteToRecover].dm->recoverThisSite();
        siteMap[siteToRecover].status = SITE_STATUS::recovering;

        vector<int> transactionWaiting = {};

        for (auto x : transactionWaitingForSite[siteToRecover]){
            transactionWaiting.push_back(x);
        }
        tryExecutionAgain(transactionWaiting, time);
    }


}



