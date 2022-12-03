# An Overview of the Inputs Provided

### Test 1
- T2 should abort, T1 should not, because of kill youngest.
- output of dump
    - x1: 101 at site 2
    - x2: 102 at all sites
    - All other variables have their initial values.

### Test 2
- No aborts happens, since RO transactions use multiversion read protocol.
- output of dump
    - x1: 101 at site 2
    - x2: 102 at all sites
    - All other variables have their initial values.

### Test 3
- T1 should not abort because its site did not fail.
- In fact all transactions commit.
- x8 has the value 88 at every site except site 2 where it won't have the correct value right away but must wait for a write to take place.

### Test 4
- T1 aborts, since site 2 died after T1 accessed it.
- T2 Okay.

### Test 5
- T1 fails again here because it wrote to a site that failed.
- T2 Okay.

### Test 6
- T1, T2 Okay.
- T2 reads from a recovering site, but odd variables only at that site.
- At the dump, sites 3 and 4 would have their original values for x8.
- Future reads of x8 to those sites should be refused until a committed write takes place.

### Test 7
- T2 should read the initial version of x3 based on multiversion read consistency.

### Test 8
- T2 still reads the initial value of x3.
- T3 still reads the value of x3 written by T1.

### Test 9
- T1, T2, T3 Okay. 
- T3 waits and then comlete after T2 commits.

### Test 10
- T3 should wait and should not abort.

### Test 11
- All should commit.

### Test 12
- Both Transaction commit.

### Test 13
- T1 and T2 wait but eventually commit.

### Test 14
- They wait in different orders from in the above test, but they all commit.

### Test 15
- T1 will abort because x4 is on site 2 and so site 2 will lose its locks in the fail event.
- So T1 will abort. 
- T2 will be fine as will the others.


### Test 16
- T3 must wait till the commit of T2 before it reads x4 (because of locking), so sees 44.
- T1 reads x2=22 at site1.

### Test 17
- T3 must wait till the commit of T2 before it reads x4 (because of locking), so sees 44.
- T3 must abort though because the lock information is lost on site 4 upon failure.
- T1 reads the initial value of x2 because T3 has aborted.

### Test 18
- A circular deadlock scenario.
- T5 as the youngest will abort, allowing T4 to complete, then T3, T2, and T1.
- Only T5s write will not succeed. All others will succeed.

### Test 19
- An almost circular deadlock scenario with failures.
- T3 fails (T2 and T4 do not fail because the site is up when they execute) because site 4 fails.
- All others succeed.

### Test 20
- T1 aborts.
- T2 can't read x2 from site 1, but doesn't hold a lock on x2 at site 1
- T5 doesn't need to wait because T2 doesn't acquire a lock since site 1 can't respond to the read.

### Test 21
- T2 will try to promote its read lock to a write lock but can't, so there is a deadlock. 
- T2 is younger so will abort.

### Test 22
- T1 should not abort because site 4 did not fail.
- However T1 will write to x4 on every site except site 2.
- Site 2 should not be able to respond to read requests for any replicated variable after it recovers until a write is committed to it.
- T1's write will not go to site 2, so every site except site 2 will have x4 equal to 91.
- x8 will not value 88 because T2 aborts the correct value right away but must wait for a write to take place.
- W(T2,x8,88) will not commit and is lost on failure.
- Even though site 2 recovers before T2, T2 will not retroactively write to the site (in any practical version of available copies).
- T2 aborts because it wrote to x8.

### Test 23
- T1 should not abort because site 4 did not fail.
- In this case, T1 will write to x4 on every site. 
- x8 will not value 88 because T2 aborts.
- The correct value right away but must wait for a write to take place.
- So W(T2,x8,88) will not commit and is lost on failure.
- Even though site 2 recovers before T2, T2 will not retroactively write to the site (in any practical version of available copies).
- T2 aborts because it wrote to x8.

### Test 24
- All Transactions get committed
- T3,T4 get blocked

### Test 25
- T1 gets aborted
- T2 has to wait, but eventually is committed
- T3,T5 are committed

### Test 26
- All Transactions get committed
- T1 waits initially, but gets committed eventually