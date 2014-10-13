Multithreaded DNS Lookup
=======================

Authors: Ian Ker-Seymer, Austin Wood, Dawson Botsford

### Lookup DNS info for all names files in input folder (non-threaded):
    ./lookup input/names*.txt results.txt

### Test multi-threaded version of DNS lookups.
##### Currently, the queue should only hold 5 hostnames, so the program should be stuck in a wait condition after 5 URLs have been added. Once the response pool is added to deal with the queue, this will be fixed.
    ./multi_lookup input/names*.txt results.txt

### Check queue for memory leaks:
    valgrind ./queueTest
