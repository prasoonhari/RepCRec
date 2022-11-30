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

    // blocked transaction queue
    std::deque<int> blockedTransactions;
    // An instance of data manager
    std::map<int, SiteDetail> siteMap;
    // variable and the sites they are present
    std::map<int, std::vector<int>> variableMap;

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
};

#endif