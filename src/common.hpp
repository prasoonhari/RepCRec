#include <unordered_map>
#include <string>

enum CMD_TYPE
{
    begin,
    beginRO,
    W,
    R,
    fail,
    recover,
    dump,
    end
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