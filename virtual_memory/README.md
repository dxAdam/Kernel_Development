## virtual memory

### project:

The goal of this project is to implement a four-level page table in the Linux kernel to be used with Ubuntu 18. This involves building the page table data structure, managing a memory map made from a kernel style linked list, translating virtual addresses to physical addresses (and vice versa), correctly freeing pages as they are no longer needed, and more. The supplied project framework takes a block of memory offline and makes it available to our module to allocate from.

The code we wrote is located in on_demand.c and user/test.c.

See project2.pdf for more information
  
### usage:

The best way to work with this project is to install Ubuntu 18 on a virtual machine. To avoid latency I like to SSH into the virtual machine from the hosting desktop and work from there.  

Once the VM is up and running install git and clone the repo with  
  
`sudo apt install git`  
`git clone https://github.com/dxadam/Systems_Software/virtual_memory`  
  
and build the kernel module with  
    
`cd virtual memory`  
`make`  
  

in the main directory. The module is then inserted into a
running kernel with  
  
`sudo insmod petmem.ko`  
  
We take memory offline and give it to our module by first building petmem with  

`cd user`  
`make`  
  
and running petmem with  
  
`sudo ./petmem 128`.  
  
Finally we run the test program with  
  
`./test`.  
  
If changes are made to on_demand.c or any other module file then we will need to remove the current module before inserting our newly compiled one. This is done with  
  
`sudo rmmod petmem.ko`.  
  
This process is automated by running  
  
`./reset.sh`
  
See project2.pdf for more information.  

### contents:
<pre>
Most framework file descriptions can be found in project2.pdf

user/Makefile             - builds petmem

user/petmem               - used to give our module memory from the running kernel

user/test.c               - tests our page table implementation by allocating/freeing memory

Makefile                  - builds the kernel module

on_demand.c               - our code implementing the page table

project2.pdf		  - project description and requirements

reset.sh                  - automates reloading the module and running offlining memory

submitted.tar		  - version that was submitted for grading
</pre>
