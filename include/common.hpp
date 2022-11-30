#ifndef COMMON
#define COMMON

#include <unordered_map>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <utility>
#include <deque>

enum CMD_TYPE
{
    begin = 2,
    beginRO = 4,
    W= 8,
    R= 16,
    fail= 32,
    recover= 64,
    dump = 128,
    end = 256
};

enum INST_TYPE
{
    Read,
    Write
};

struct Instruction
{
    INST_TYPE type;
    std::vector<int> values;
};


static std::unordered_map<std::string, CMD_TYPE> const cmdTable = {
    {"begin", CMD_TYPE::begin},
    {"beginRO", CMD_TYPE::beginRO},
    {"W", CMD_TYPE::W},
    {"R", CMD_TYPE::R},
    {"fail", CMD_TYPE::fail},
    {"recover", CMD_TYPE::recover},
    {"dump", CMD_TYPE::dump},
    {"end", CMD_TYPE::end},
};

struct Operation
{
    CMD_TYPE type;
    std::vector<std::string> vars;
};

enum SITE_STATUS
{
    down = 4,
    up = 2,
    recovering= 8,
};

enum LOCK_TYPE
{
    l_read = 2,
    l_write = 4,
};

struct LockDetail
{
    LOCK_TYPE lock_type;
    // transcation and the time it started holding the lock
    std::map<int , std::deque<int>::iterator> currentHolderMap;
    std::deque<int> currentHolderQueue;
//    // waiting tranasactions
//    std::map<int , std::deque<int>::iterator> waitingMap;
//    std::deque<int> waitingQueue;
};


enum RESULT_STATUS
{
    success = 2,
    failure = 4
};

struct OperationResult {
    RESULT_STATUS status;
    std::string msg;
};


enum T_STATUS
{
    t_committed,
    t_aborted,
    t_waiting,
    t_running
};


struct Transaction
{
    int id;
    int startTime;
    bool ReadOnly;
    T_STATUS status;
    Instruction currentInstruction;
    // site -> data list  - list of all data changed or read from a site
    std::map<int, std::vector<std::pair<int, INST_TYPE>>> dirtyData;
    // First time the site was accessed
    std::map<int, int> siteAccessTime;

};

struct DataDetail
{
    int currentValue;
    int committedValue;
    int lastCommitTime;
};




#endif