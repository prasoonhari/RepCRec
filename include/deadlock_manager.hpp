#ifndef DEADLOCKMANAGER
#define DEADLOCKMANAGER

#include <string>
#include <map>
#include <vector>

#include "common.hpp"

/**
 * @brief Helper Class to detect cycle at a site.
 * 
 */
class DeadLockManager {
private:

public:
    DeadLockManager();

    /**
     * @brief Method that takes adjList and returns nodes present in a cycle
     * 
     * @param adjList 
     * @return std::vector<int> , empty if no cycle
     */
    std::vector<int> deadlockDetector(std::map<int, std::vector<int>> adjList);
};

#endif