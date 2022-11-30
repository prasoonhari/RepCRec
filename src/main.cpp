#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include "common.hpp"
#include "transaction_manager.hpp"

using namespace std;

// Imput Commands


// Parse Input commands
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
    std::cout << "ADB projects!" << std::endl;

    if (!fopen(argv[1], "r") || !fopen(argv[2], "w")){
        cout<< "Please provide input and output files";
        exit(1);
    }


    freopen(argv[1], "r", stdin);
    freopen(argv[2], "w", stdout);
    // DataManager dataManager;
    // initializeDataManager(dataManager);

    TransactionManager* tm = new TransactionManager();
    tm->initializeDB();

    string inputCommand;
    int time = 0;
    while (cin >> inputCommand)
    {
        time++;
        OperationResult OptRes;
        Operation operation = parseCommand(inputCommand);
        switch (operation.type)
        {
        case CMD_TYPE::begin:
            tm->begin(operation, time);
            break;
        case CMD_TYPE::beginRO:
            tm->beginRO(operation, time);
            break;
        case CMD_TYPE::W:
            /* code */
            break;
        case CMD_TYPE::R:
            /* code */
            OptRes = tm->read(operation, time);
            cout << OptRes.msg << "\n";
            break;
//        case 32:
//            /* code */
//            break;
//        case 64:
//            /* code */
//            break;
//        case 128:
//            /* code */
//            break;
//        case 256:
//            /* code */
//            break;
        default:
            break;
        }
    }

    tm->printTM();
}