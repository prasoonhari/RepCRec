
#include "deadlock_manager.hpp"
#include <stack>

using namespace std;


DeadLockManager::DeadLockManager() {


}

vector<int> dfs(int vertex, map<int, vector<int> > &adjList, map<int, int > &visited, stack<int> &tracker) {
    visited[vertex] = -1;

    for (auto next : adjList[vertex]) {
        if (visited[next] == -1) {
            // cycle
            vector<int> buffer;
            while(!tracker.empty()) {
                buffer.push_back(tracker.top());
                tracker.pop();
            }
            return buffer;
        } else if (visited[next] == 0) {
            tracker.push(next);
            vector<int> res = dfs(next, adjList, visited, tracker);
            if (res.size() > 0) return res;
        }
    }

    tracker.pop();
    visited[vertex] = 1;
    return vector<int>();
}

vector<int> DeadLockManager::deadlockDetector(map<int, vector<int> > adjList) {
    map<int, int> visited;
    for (auto item : adjList) {
        visited[item.first] = 0;
    }
    
    for (auto item : visited) {
        if (item.second == 0) {
            stack<int> tracker;
            tracker.push(item.first);
            vector<int> res = dfs(item.first, adjList, visited, tracker);
            if (res.size() > 0) return res;
        }
    }

    return vector<int>();
}

