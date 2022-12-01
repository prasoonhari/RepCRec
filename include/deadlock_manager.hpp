#ifndef DEADLOCKMANAGER
#define DEADLOCKMANAGER

#include <string>
#include <map>
#include <vector>

#include "common.hpp"

class DeadLockManager {
private:
    // lock detail on each variable. variable_id -> lock details

public:
    DeadLockManager();

    std::vector<int> deadlockDetector(std::map<int, std::vector<int> > adjList);
};

#endif