#include <unordered_map>
#include <vector>
#include <string>
#include <map>
#include <utility>
class DataManager
{
private:
    std::map<int, std::map<std::string, int>> sites;
public:
  void setSite(int siteIdx, std::string variable, int value);
  void printDM();
};