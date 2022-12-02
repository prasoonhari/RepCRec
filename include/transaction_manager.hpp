#ifndef TRANSACTIONMANAGER
#define TRANSACTIONMANAGER

#include <string>
#include <set>
#include <unordered_map>
#include <map>
#include <deque>
#include "common.hpp"
#include "data_manager.hpp"
#include "deadlock_manager.hpp"


struct SiteDetail {
    DataManager *dm;
    SITE_STATUS status;
    int lastStatusChangeTime;
    std::vector<int> failedHistory;
};


class TransactionManager {
private:
    // all the transaction details - All Txn that we receive is stored
    std::map<int, Transaction> transactions;
    // Has the site detail to all the 10 sites (site -> their Detail) (the detail has instance of dataManager used to manage the site)
    std::map<int, SiteDetail> siteMap;
    // variable and the sites they are present -Initialized at the beginning Used to check if a data is replicated or not
    std::map<int, std::vector<int>> variableMap;
    // variable -> txns, for a given variable list of Txn that are waiting (because can't acquire lock) (must be in order of the time transaction arrived)
    std::map<int, std::vector<int>> transactionWaitingForData;
    // sites -> txns
    // TODO: not required i think
    std::map<int, std::vector<int>> transactionWaitingForSite;
    // An AdjList, Txn -> list of Txns that it's waiting for (to acquire locks)
    std::map<int, std::vector<int>> transactionDependency;

    DeadLockManager *deadLockManager;



public:
    // Flag used to check if we might need a deadlock check at the beginning of the tick
    bool deadlockMightOccur;

    TransactionManager();

    void initializeDB();

    void printTM();

    std::pair<bool, std::vector<int>> isWritePossible(int variable, Transaction *currentTxn);

    void begin(Operation O, int time);

    void beginRO(Operation O, int time);

    OperationResult read(Transaction *currentTxn, int time);

    OperationResult write(Transaction *currentTxn, int time);

    // void end(Operation O);
    // void fails(Operation O);
    // void recover(Operation O);
    Transaction *getTransactionFromOperation(Operation Op);
    void processBlockingTransaction(Transaction *currentTxn, int variable_id, const std::vector<int>& blockingTxns);

    void printDump();

    void failSite(Operation Op, int time);

    void endTransaction(Operation Op, int time);

    void commitTransaction(Transaction *currentTxn, int commit_time);

    void abortTransaction(Transaction *currentTxn, int commit_time);

    void tryExecutionAgain(const std::vector<int>& txns, int time);

    OperationResult writeOperation(Operation Op, int time);

    OperationResult readOperation(Operation Op, int time);

    void recoverSite(Operation Op, int time);

    bool blockedByWaitlist(int variable_id, int txn_id);

    bool readCondition(int var_site, int variable_id);

    static OperationResult readOnly(Transaction *currentTxn, int variable_id);

    std::map<int, int> CreateSnapshot();

    void resolveDeadlock(int time);

    void ProcessTransactionWaitingForSite(Transaction *currentTxn, int variable_id);
};

#endif