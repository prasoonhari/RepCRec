#ifndef COMMON
#define COMMON

#include <unordered_map>
#include <string>
#include <vector>
#include <list>
#include <utility>

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


enum LOCK_TYPE
{
    read = 2,
    write = 4,
};

struct LockDetail
{
    LOCK_TYPE lock_type;
    // transcation and the time it started holding the lock
    std::list<std::pair<int, int>> currentHolder;
    // waiting tranasactions
    std::list<int> waitingQueue;
};

#endif