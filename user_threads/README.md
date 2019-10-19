## user threads

### project:

This project involved implementing a user level pthreads-like library. The library contains functions handling the initialization and creation of threads, yielding and joining threads, and the termination of threads.

The code we wrote is located in pet_thread.c.

See project3.pdf for more information
  
### usage:

Clone and enter the repo with  
  
`git clone https://github.com/dxadam/Systems_Software/user_threads`  
`cd user_threads`.  
  
Build the project with  
  
`make`  
  
and make and run the test files with 
  
`cd tests`  
`make`  
`./test.c`.  
   
See project3.pdf for more information.  

### contents:
<pre>
Framework file descriptions can be found in project3.pdf if not listed

tests/Makefile            - builds our test programs

tests/test\*.c            - our test programs

Makefile                  - builds the supplied driver test program

driver.c                  - a simple driver program

pet_hashtable.c/h         - a supplied hashtable to use with our implementation

pet_thread.c              - contains our code implementing the user threads

project3.pdf		  - project description and requirements

pet_thread_hw.S           - assembly code used for saving and restoring registers on context switch

submitted.tar		  - version that was submitted for grading
</pre>
