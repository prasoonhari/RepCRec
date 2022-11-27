#ifndef DATAMANAGER
#define DATAMANAGER

#include <string>
#include <map>
#include <unordered_map>
class DataManager
{
private:
    std::map<int, std::map<int, int>> sites;

public:
    DataManager();
    void setSite(int siteIdx, int variable, int value);
    void initializeSite(int siteIdx);
    void printDM();
};

#endif