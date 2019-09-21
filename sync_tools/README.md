## sync tools

#### project:

This project involved writing synchronization primitives for multicore systems.

We wrote inline assembly to implement:
	memory barriers
        atomic addition/subtraction
        spinlocks
        barriers
        reader writer locks
        a lock-free queue. 

The code we wrote is located in locking.c and locking.h. 

See project1.pdf for more information

#### usage:

compile with  
`make`  
and run the test driver with  
`./driver`

