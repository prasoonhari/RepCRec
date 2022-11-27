#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "common.hpp"
#include "data_manager.hpp"

using namespace std;

struct Operation
{
    CMD_TYPE type;
    vector<string> vars;
};

Operation parseCommand(string inputCommand)
{

    Operation cmd{};
    unsigned parenthesesOpen = inputCommand.find('(');
    unsigned parenthesesClose = inputCommand.find_last_of(')');
    string cmdString = inputCommand.substr(0, parenthesesOpen);
    auto it = cmdTable.find(cmdString);
    if (it != cmdTable.end())
    {
        cmd.type = it->second;
    }
    else
    {
        cout << "Wrong Input Command";
        exit(1);
    }

    string inputVariables = inputCommand.substr(parenthesesOpen + 1, parenthesesClose - parenthesesOpen - 1);
    stringstream ss(inputVariables);
    string temp;
    while (getline(ss, temp, ','))
    {
        cmd.vars.push_back(temp);
    }
    return cmd;
}

int main(int argc, const char *argv[])
{
    std::cout << "ADB project!" << std::endl;

    freopen("../test/input1.txt", "r", stdin);
    freopen("../output/output1.txt", "w", stdout);

    string inputCommand;
    while (cin >> inputCommand)
    {
        Operation operation = parseCommand(inputCommand);
    }
}