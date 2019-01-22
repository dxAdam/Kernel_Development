/*
     Adam Grusky
     January 2019

     x86 Synchronization Tools
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#include "locking.h"




/* Exercise 1:
 *     Basic memory barrier
 */
void mem_barrier() {
	__asm__ __volatile__("" : : : "memory");
}


/* Exercise 2:
 *     Simple atomic operations
 */


/* The two functions below use constraints described here:

	"=" - operand is write-only for this instruction & previous value discarded
	"m" - memory operand is allowed at any machine supported address
	"i" - immediate integer operand
	"r" - keep in register
*/

void
atomic_sub( int * value,
	    int   dec_val)
{
    __asm__ __volatile__(
    	"   lock       ;"		/* aquire memory bus lock for subl */
        "   subl %1,%0 ;"
        :"=m"(*value)
        :"ir"(dec_val), "m"(*value)
        :				/* no clobbered registers 	   */
        );
}

void
atomic_add( int * value,
	    int   inc_val)
{
    __asm__ __volatile__(
	"   lock       ;"
        "   addl %1,%0 ;"
        :"=m"(*value)
        :"ir"(inc_val), "m"(*value)
        );
}


/* Exercise 3:
 *     Spin lock implementation
 */


/* Compare and Swap
 * Returns the value that was stored at *ptr when this function was called
 * Sets '*ptr' to 'new' if '*ptr' == 'expected'
 */
unsigned int
compare_and_swap(unsigned int * ptr,
		 unsigned int   expected,
		 unsigned int   new)
{
    unsigned int init_ptr_val;
   
    /* we will use the CMPXCHG opcode (Intel Vol. 2A 3-181) 

       [lock] cmpxchgl r32, r/m32

       the value in eax is compared to the destination operand

       if equal, src -> dest           else, dst -> eax
   */

   /*
       "a" - use 'a' register
       "+" - read initally then write-only
   */ 

   __asm__ __volatile__(
	"   lock       ;"
	"   cmpxchgl %2, %1"
	:"=a"(init_ptr_val), "+m"(*ptr)
        :"r"(new), "0"(expected)
	:"memory"
	); /* no globbered registers - gcc knows we used eax */			    

    return init_ptr_val;
}


void
spinlock_init(struct spinlock * lock)
{      	
    lock->free = 0;
    mem_barrier(); 
}

void
spinlock_lock(struct spinlock * lock)
{
    // use compare_and_swap to mark spinlock locked (free==1) if unlocked (free==0)   
    while(compare_and_swap(&lock->free, 0, 1) == 1);
    mem_barrier();
}


void
spinlock_unlock(struct spinlock * lock)
{
    mem_barrier();
    lock->free=0;
}


int
atomic_add_ret_prev(int * value,
		    int   inc_val)
{
    int init_val;

    /* we will use the XADD opcode (Intel Vol. 2A 5-581) 

       [lock] xadd r/m32, r32

       dest and src are swapped and then (dest+src) -> dest
   */

   __asm__ __volatile__(
        "   lock       ;"
        "   xaddl %0, %1"
        :"=r"(init_val), "=m"(*value)
        :"0"(inc_val)
        :"memory"
        ); /* no globbered registers - gcc knows we used eax */  
	
    return init_val;
}

/* Exercise 4:
 *     Barrier operations
 */

void
barrier_init(struct barrier * bar,
	     int              count)
{
	//init barrier values
	bar->flag = 0;
	bar->in_counter = 0;
	bar->out_counter = count;
	bar->max_count = count;

	spinlock_init(&bar->lock);
}


void
barrier_wait(struct barrier * bar)
{
    /*
        barrier_wait() must handle incoming threads arriving
	 while threads from a previous call are still exiting
	 
	we can monitor the incoming and outgoing threads with 
	 the counters bar->in_count and bar->out_count

	bar->flag is used to signal the arrival of a full set of
         threads (bar->in_count == bar->max_count)
	 
	bar->out_counter is used to signal incoming threads that
	 threads from a previous call are still exiting
 
        we can make use of our previously defined spinlocks to
	 gain exclusivity to shared barrier variables
   */
       

    // thread needs to lock to check counts and set flag	
    spinlock_lock(&bar->lock);
    if(bar->in_counter == 0){
    	// thread is first of a new set
	if(bar->out_counter != 0){	
	    // threads from a previous set are still exiting

	    // unlock while spinning
	    spinlock_unlock(&bar->lock);
	    while(bar->out_counter != bar->max_count);
	    spinlock_lock(&bar->lock);
        }	
        // previous threads have exited
	bar->flag = 0;
    }

    bar->in_counter++;

    // before unlocking create local variable to save 
    //  thread rank 
    int my_rank = bar->in_counter;
    spinlock_unlock(&bar->lock);//unlock    

    if(my_rank == bar->max_count){
	// thread is last arrival of the set and handles resetting
	bar->in_counter = 0;
	bar->out_counter = 1; // set before flag since no exclusivity here
	bar->flag = 1;
    }
    else{
	// all but last thread of set wait for full set
	while(bar->flag == 0);

	//inlock_lock(&bar->lock);
	//bar->out_counter++;
	//spinlock_unlock(&bar->lock);
		
 		// or

	atomic_add(&bar->out_counter, 1);
    }
}

/* Exercise 5:
 *     Reader Writer Locks
 */

void
rw_lock_init(struct read_write_lock * lock)
{
    /* Implement this */
}


void
rw_read_lock(struct read_write_lock * lock)
{
    /* Implement this */
}

void
rw_read_unlock(struct read_write_lock * lock)
{
    /* Implement this */
}

void
rw_write_lock(struct read_write_lock * lock)
{
    /* Implement this */
}


void
rw_write_unlock(struct read_write_lock * lock)
{
    /* Implement this */
}



/* Exercise 6:
 *      Lock-free queue
 *
 * see: Implementing Lock-Free Queues. John D. Valois.
 *
 * The test function uses multiple enqueue threads and a single dequeue thread.
 *  Would this algorithm work with multiple enqueue and multiple dequeue threads? Why or why not?
 */


/* Compare and Swap 
 * Same as earlier compare and swap, but with pointers 
 * Explain the difference between this and the earlier Compare and Swap function
 */
uintptr_t
compare_and_swap_ptr(uintptr_t * ptr,
		     uintptr_t   expected,
		     uintptr_t   new)
{
    /* Implement this */
}



void
lf_queue_init(struct lf_queue * queue)
{
    /* Implement this */
}

void
lf_queue_deinit(struct lf_queue * lf)
{
    /* Implement this */
}

void
lf_enqueue(struct lf_queue * queue,
	   int               val)
{
    /* Implement this */
}

int
lf_dequeue(struct lf_queue * queue,
	   int             * val)
{
    /* Implement this */
    return 0;
}



