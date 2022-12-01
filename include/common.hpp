#ifndef COMMON
#define COMMON

#include <unordered_map>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <deque>
#include <utility>
#include <deque>

enum CMD_TYPE {
    begin = 2,
    beginRO = 4,
    W = 8,
    R = 16,
    fail = 32,
    recover = 64,
    dump = 128,
    end = 256
};

enum INST_TYPE {
    Read,
    Write
};

struct Instruction {
    INST_TYPE type;
    std::vector<int> values;
};


static std::unordered_map<std::string, CMD_TYPE> const cmdTable = {
        {"begin",   CMD_TYPE::begin},
        {"beginRO", CMD_TYPE::beginRO},
        {"W",       CMD_TYPE::W},
        {"R",       CMD_TYPE::R},
        {"fail",    CMD_TYPE::fail},
        {"recover", CMD_TYPE::recover},
        {"dump",    CMD_TYPE::dump},
        {"end",     CMD_TYPE::end},
};

struct Operation {
    CMD_TYPE type;
    std::vector<std::string> vars;
};

enum SITE_STATUS {
    down = 4,
    up = 2,
    recovering = 8,
};

enum LOCK_TYPE {
    l_NONE = 1,
    l_read = 2,
    l_write = 4,
};

struct LockDetail {
    LOCK_TYPE lock_type = LOCK_TYPE::l_NONE;
    // transcation and the time it started holding the lock
    std::map<int, std::deque<int>::iterator> currentHolderMap;
    std::deque<int> currentHolderQueue;
//    // waiting tranasactions
//    std::map<int , std::deque<int>::iterator> waitingMap;
//    std::deque<int> waitingQueue;
};


enum RESULT_STATUS {
    success = 2,
    failure = 4
};

struct OperationResult {
    RESULT_STATUS status;
    std::string msg;
};

struct TransactionResult {
    RESULT_STATUS status;
    std::deque<int> blockingTransaction;
};


enum T_STATUS {
    t_committed,
    // Will get aborted - because a site failed or had a failed Read or write (like no site found)
    t_aborting,
    // Blocked by another transaction
    t_blocked,
    // Waiting means waiting for a site to recover
    t_waiting,
    t_running
};


struct Transaction {
    int id;
    int startTime;
    bool ReadOnly;
    T_STATUS status;
    // The current Read or Write Instruction that the Txn has done or is waiting on
    Instruction currentInstruction;
    // site -> data list  - list of all data that was changed or read from a site by this Txn.
    std::map<int, std::vector<int>> dirtyData;
    // First time the site was accessed
    // TODO
    std::map<int, int> siteAccessTime;

};

// DataDetail contain the committed value of variable, the time it was committed and current value
// which is written by a transaction that has not committed it yet, so it's dirty
struct DataDetail {
    int currentValue;
    int committedValue;
    int lastCommitTime;
};


#endif