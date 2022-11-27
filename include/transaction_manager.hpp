#ifndef TRANSACTIONMANAGER
#define TRANSACTIONMANAGER

#include <string>
#include <set>
#include <unordered_map>
#include "common.hpp"
#include "data_manager.hpp"

enum T_STATUS
{
    commited,
    aborted,
    waiting,
    running
};

struct Transaction
{
    int id;
    int startTime;
    bool ReadOnly;
    T_STATUS status;
    Operation currentOperation;
};

inline bool operator<(const Transaction& lhs, const Transaction& rhs)
{
  return lhs.startTime < rhs.startTime;
}

class TransactionManager
{
private:
    std::set<Transaction> transactions;
    DataManager* dataManager;
    
public:
    void initializeDataManager(DataManager& dm);
    TransactionManager();
    void printTM();
    void begin(Operation O, int time);
    void beginRO(Operation O, int time);
    // void read(Operation O);
    // void write(Operation O);
    // void end(Operation O);
    // void fails(Operation O);
    // void recover(Operation O);
};

#endif