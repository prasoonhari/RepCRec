#ifndef TRANSACTIONMANAGER
#define TRANSACTIONMANAGER

#include <string>
#include <set>
#include <unordered_map>
#include <map>
#include <deque>
#include "common.hpp"
#include "data_manager.hpp"


struct SiteDetail
{
    DataManager* dm;
    SITE_STATUS status;
    int lastStatusChangeTime;
    int lastFailureTime;
};



class TransactionManager
{
private:
    // all the transaction details
    std::map<int, Transaction> transactions;
    // An instance of data manager
    std::map<int, SiteDetail> siteMap;
    // variable and the sites they are present - Used to check if a data is replicated or not
    std::map<int, std::vector<int>> variableMap;
    // variable -> txns
    std::map<int, std::vector<int>> transactionWaiting;
    // sites -> txns
    std::map<int, std::vector<int>> transactionWaitingForSite;

    std::map<int, std::vector<int>> transaction_wait_table;


public:
    TransactionManager();
    void initializeDB();
    void printTM();
    void begin(Operation O, int time);
    void beginRO(Operation O, int time);
    OperationResult read(Operation O, int time);
    OperationResult write(Operation Op, int time);
    // void end(Operation O);
    // void fails(Operation O);
    // void recover(Operation O);
    Transaction* getTransactionFromOperation(Operation Op);

    void isWritePossible(int variable);

    std::pair<bool, std::vector<int>> isWritePossible(int variable, Transaction *currentTxn);
};

#endif