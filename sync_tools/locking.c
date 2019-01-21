// Adam Grusky -- January 2019

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
   
    /* we will use the x86 cmpxchg opcode (Intel Vol. 2A 3-181) 

       [lock] cmpxchgl reg, reg/m32

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
	);			    

    return init_ptr_val;
}


void
spinlock_init(struct spinlock * lock)
{
    /* Implement this */
}

void
spinlock_lock(struct spinlock * lock)
{
    /* Implement this */
}


void
spinlock_unlock(struct spinlock * lock)
{
    /* Implement this */
}


int
atomic_add_ret_prev(int * value,
		    int   inc_val)
{
    /* Implement this */
    return 0;
}

/* Exercise 4:
 *     Barrier operations
 */

void
barrier_init(struct barrier * bar,
	     int              count)
{
	//init barrier values
	bar->iterations = 0;
	bar->cur_count = 0;
	bar->init_count = count;
}

void
barrier_wait(struct barrier * bar)
{
   // this handles resetting the barrier
   if(bar->cur_count == 0)
   	bar->cur_count = bar->init_count;

   // spin while current count is not zero
   atomic_sub(&bar->cur_count, 1);
   
   while(bar->cur_count != 0);
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



