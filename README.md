Multithreaded DNS Lookup
=======================

Authors: Ian Ker-Seymer, Austin Wood, Dawson Botsford

### Lookup DNS info for all names files in input folder:
    ./lookup input/names*.txt results.txt

### Check queue for memory leaks:
    valgrind ./queueTest
