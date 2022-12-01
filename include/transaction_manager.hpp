#ifndef TRANSACTIONMANAGER
#define TRANSACTIONMANAGER

#include <string>
#include <set>
#include <unordered_map>
#include <map>
#include <deque>
#include "common.hpp"
#include "data_manager.hpp"


struct SiteDetail {
    DataManager *dm;
    SITE_STATUS status;
    int lastStatusChangeTime;
    std::vector<int> failedHistory;
};


class TransactionManager {
private:
    // all the transaction details
    std::map<int, Transaction> transactions;
    // An instance of data manager
    std::map<int, SiteDetail> siteMap;
    // variable and the sites they are present - Used to check if a data is replicated or not
    std::map<int, std::vector<int>> variableMap;
    // variable -> txns
    std::map<int, std::vector<int>> transactionWaitingForData;
    // sites -> txns
    std::map<int, std::vector<int>> transactionWaitingForSite;

    std::map<int, std::vector<int>> transactionDependency;


public:
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

    void isWritePossible(int variable);


    void printDump();

    void failSite(Operation Op, int time);

    void endTransaction(Operation Op, int time);

    void commitTransaction(Transaction *currentTxn, int commit_time);

    void abortTransaction(Transaction *currentTxn, int commit_time);

    void tryExecutionAgain(std::vector<int> txns, int time);

    OperationResult writeOperation(Operation Op, int time);

    OperationResult readOperation(Operation Op, int time);

    void recoverSite(Operation Op, int time);
};

#endif