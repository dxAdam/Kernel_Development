## sync tools
<pre>
### project:

This project involved writing synchronization primitives for multicore systems.

We wrote inline assembly to implement:
	memory barriers
        atomic addition/subtraction
        spinlocks
        barriers
        reader writer locks
        a lock-free queue. 

The code we wrote is located in locking.c, locking.h, and driver.c. 

See project1.pdf for more information
  
### usage:

compile with  
`make`  
and run the test driver with  
`./driver`  
  
driver.c contains several test that would result in deadlocks or error if
synchronization tools were not used (on multicore systems). The number of
processors used is the value returned by sysconf(_SC_NPROCESSORS_ONLN).
  
### contents:

GCC-Inline-Assembly.pdf   - A guide covering the basics of inline assembly

LockFreeQueues_Valois.pdf - A paper describing the implementation of a lock
                            free queue that ours is based off of

Makefile                  - run `make` to build project

driver.c                  - contains several tests for testing our sync tools

locking.c                 - code implementing our sychronization tools

locking.h                 - locking header file

project1.pdf              - project description and requirements

submitted.tar             - version that was submitted for grading
</pre>
