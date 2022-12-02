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
    // Method that takes adjList and returns nodes present in a cycle, if no cycle is present - returns empty vector
    std::vector<int> deadlockDetector(std::map<int, std::vector<int>> adjList);
};

#endif