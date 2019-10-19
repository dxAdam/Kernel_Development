## virtual memory

### project:

This project involved writing synchronization primitives for multicore systems. We used inline assembly to implement memory barriers, atomic addition/subtraction, spinlocks, barriers, reader writer locks, and finally a lock-free queue..

The code we wrote is located in locking.c and user/test.c.

See project2.pdf for more information
  
### usage:

First we need to compile and load the kernel module. This is done
by running  
  
	make  
  

in the main directory. The module is then inserted into a
running kernel with  
  
	sudo insmod petmem.ko  
  
We take memory offline and give it to our module by first building petmem with  

	cd user  
	make  
  
and running petmem with  
  
	sudo ./petmem 128  
  
Finally we run the test program with  
  
	./test  
  
If changes are made to on_demand.c or any other module file then we will need to remove the current module before inserting our newly compiled one. This is done with  
  
	sudo rmmod petmem.ko  
  
This process is automated by running  
  
	./reset.sh
  
See project2.pdf for more information.  

### contents:
<pre>
project2.pdf		  - project description and requirements

submitted.tar		  - version that was submitted for grading
</pre>
