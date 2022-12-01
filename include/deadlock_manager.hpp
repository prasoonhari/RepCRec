#ifndef DEADLOCKMANAGER
#define DEADLOCKMANAGER

#include <string>
#include <map>
#include <vector>

#include "common.hpp"

class DeadLockManager {
private:

public:
    DeadLockManager();

    std::vector<int> deadlockDetector(std::map<int, std::vector<int> > adjList);
};

#endif