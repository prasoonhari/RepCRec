#ifndef TRANSACTIONMANAGER
#define TRANSACTIONMANAGER

#include <string>
#include <set>
#include <unordered_map>
#include <map>
#include <deque>
#include "common.hpp"
#include "data_manager.hpp"
#include "deadlock_manager.hpp"


struct SiteDetail {
    DataManager *dm;
    SITE_STATUS status;
    int lastStatusChangeTime;
    std::vector<int> failedHistory;
};

/**
 * @brief 
 * All operation pertaining to the database is executed by the 
 * Transaction manager (TM). 
 * While performing any operation for transaction T, 
 * the TM uses the Available copies strategy and if it fails to complete the operation 
 * (whether waiting for a lock to be released or a failure to be cleared) 
 * then it must put the corresponding T in a wait queue and check to 
 * resolve it in the next tick. It also checks for deadlocks. 
 * The TM also fulfills the role of a broker which routes requests and 
 * knows the up/down status of each site.
 * 
 */
class TransactionManager {
private: 

    /**
     * @brief all the transaction details - All Txn that we receive is stored
     * A Transaction object encapsulates the status, type of txn, and the current instruction
     * pending/done for this transaction. It also holds the details of affected sites
     * 
     */
    std::map<int, Transaction> transactions;

    /**
     * @brief Has the site detail to all the 10 sites (site -> their Detail) 
     * The detail has instance of dataManager used to manage the site and the status
     *  
     */
    std::map<int, SiteDetail> siteMap;

    /**
     * @brief variable and the sites they are present in
     * Initialized at the beginning Used to check if a data is replicated or not
     * 
     */
    std::map<int, std::vector<int>> variableMap;

    /**
     * @brief variable -> txns, for a given variabl, list of Txn that are waiting because they 
     * can't acquire lock
     * Must be in order of the time transaction arrived
     * 
     */
    std::map<int, std::vector<int>> transactionWaitingForData;

    /**
     * @brief sites -> txns, for a given site, list of Txn that are waiting because the site
     * was down. When a site recovers, we use this to re-execute all the pending instructions
     * Must be in order of the time transaction arrived
     * 
     */
    std::map<int, std::vector<int>> transactionWaitingForSite;

    /**
     * @brief An AdjList, Txn -> list of Txns that it's waiting for another transactions
     * to acquire locks. Used to detect deadlocks
     */
    std::map<int, std::vector<int>> transactionDependency;

    DeadLockManager *deadLockManager;



public:
    // Flag used to check if we might need a deadlock check at the beginning of the tick
    bool deadlockMightOccur;

    TransactionManager();

    void initializeDB();

    void printTM();

    /**
     * @brief Detect if a write is possible or not
     * If possible, returns true, else returns false and the txns because of which the write isn't possible
     * @param variable 
     * @param currentTxn 
     * @return std::pair<bool, std::vector<int>> 
     */
    std::pair<bool, std::vector<int>> isWritePossible(int variable, Transaction *currentTxn);

    /**
     * @brief Begins a Read/Write Operation
     * 
     * @param O operation type and its variables
     * @param time 
     */
    void begin(Operation O, int time);

    /**
     * @brief Begins a Read Only Operation.
     * 
     * @param O operation type and its variables
     * @param time 
     */
    void beginRO(Operation O, int time);


    /**
     * @brief Used for read transactions, if possible. Else, the transaction is put into a 
     * waiting queue
     * 
     * @param currentTxn 
     * @param time 
     * @return OperationResult 
     */
    OperationResult read(Transaction *currentTxn, int time);

    /**
     * @brief Used for write transactions, if possible. Else, the transaction is put into a 
     * waiting queue
     * 
     * @param currentTxn 
     * @param time 
     * @return OperationResult 
     */
    OperationResult write(Transaction *currentTxn, int time);

    Transaction *getTransactionFromOperation(Operation Op);

    void processBlockingTransaction(Transaction *currentTxn, int variable_id, const std::vector<int>& blockingTxns);

    void printDump();

    /**
     * @brief fails a site and resets its locks
     * 
     * @param Op 
     * @param time 
     */
    void failSite(Operation Op, int time);

    /**
     * @brief ends a transaction by either committing or aborting it. Also checks if another
     * transaction can be executed because of this.
     * 
     * @param Op 
     * @param time 
     */
    void endTransaction(Operation Op, int time);

    void commitTransaction(Transaction *currentTxn, int commit_time);

    void abortTransaction(Transaction *currentTxn, int commit_time);

    void tryExecutionAgain(const std::vector<int>& txns, int time);

    OperationResult writeOperation(Operation Op, int time);

    OperationResult readOperation(Operation Op, int time);

    /**
     * @brief recovers a site, initializes its locks, and checks if any transactions
     * can be completed
     * 
     * @param Op 
     * @param time 
     */
    void recoverSite(Operation Op, int time);

    /**
     * @brief checks if there's any transaction waiting for a particular variable
     * 
     * @param variable_id 
     * @param txn_id 
     * @return true/false 
     */
    bool blockedByWaitlist(int variable_id, int txn_id);

    bool readCondition(int var_site, int variable_id);

    static OperationResult readOnly(Transaction *currentTxn, int variable_id);

    std::map<int, int> CreateSnapshot(int time);

    /**
     * @brief resolves deadlocks, aborts the youngest transaction
     * 
     * @param time 
     */
    void resolveDeadlock(int time);

    void ProcessTransactionWaitingForSite(Transaction *currentTxn, int variable_id);

    void removeTransactionFromDependencyList(Transaction *t);

    void omitTransaction(Transaction *txn);
};

#endif