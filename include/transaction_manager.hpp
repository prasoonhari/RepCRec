#ifndef TRANSACTIONMANAGER
#define TRANSACTIONMANAGER

#include <string>
#include <set>
#include <unordered_map>
#include <map>
#include "common.hpp"
#include "data_manager.hpp"



class TransactionManager
{
private:
    // all the transaction details
    std::map<int, Transaction> transactions;
    // An instance of data manager
    DataManager *dataManager;

public:
    TransactionManager();
    void printTM();
    void begin(Operation O, int time);
    void beginRO(Operation O, int time);
    OperationResult read(Operation O, int time);
    // void write(Operation O);
    // void end(Operation O);
    // void fails(Operation O);
    // void recover(Operation O);
};

#endif