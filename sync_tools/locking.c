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

	"+" - read initially then write only
	"m" - memory operand is allowed at any machine supported address
	"i" - immediate integer operand
	"r" - keep in register
*/

void
atomic_sub( int * value,
	    int   dec_val)
{
    __asm__ __volatile__(
    	"   lock;        \n"		/* aquire memory bus lock for subl */
        "   subl %1,%0    \n"
        :"+m"(*value)
        :"ir"(dec_val)
	:
	);
}

void
atomic_add( int * value,
	    int   inc_val)
{
    __asm__ __volatile__(
	"   lock;        \n"
        "   addl %1,%0    \n"
        :"+m"(*value)
        :"ir"(inc_val)
	:
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
	"a" - eax register
   */
       
    __asm__ __volatile__(
	"   lock;           \n"
	"   cmpxchg %2, %1  \n"
	:"=a"(init_ptr_val), "+m"(*ptr)
        :"r"(new), "0"(expected)
	:"memory"
	); 			    

    return init_ptr_val;
}


void
spinlock_init(struct spinlock * lock)
{      	
    lock->free = 1;
    mem_barrier(); 
}

void
spinlock_lock(struct spinlock * lock)
{   
    while(compare_and_swap(&lock->free, 1, 0) == 0);
}


void
spinlock_unlock(struct spinlock * lock)
{
    mem_barrier();
    lock->free=1;
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
        "   lock;         \n"
        "   xaddl %0, %1  \n"
        :"=r"(init_val), "=m"(*value)
        :"0"(inc_val)
        :"memory"
        );   
	
    return init_val;
}

/* Exercise 4:
 *     Barrier operations
 */

void
barrier_init(struct barrier * bar,
	     int              count)
{
	bar->iterations = 0;
	bar->cur_count = 0;;
	bar->init_count = count;
}

void
barrier_wait(struct barrier * bar)
{    
 	int my_iteration = bar->iterations;
	
	// get snapshot of current count to see who resets
	int prev = atomic_add_ret_prev(&bar->cur_count, 1);
	if(prev == bar->init_count-1) {
		bar->cur_count = 0;
		bar->iterations++;
		return;
	}
	while(bar->iterations != my_iteration+1);
		
}
/* Exercise 5:
 *     Reader Writer Locks
 */

void
rw_lock_init(struct read_write_lock * lock)
{
    lock->num_readers = 0;
    lock->writer = 0;
    lock->mutex.free = 1;
}


void
rw_read_lock(struct read_write_lock * lock)
{
    spinlock_lock(&lock->mutex);
    while(lock->writer);
    atomic_add(&lock->num_readers, 1);
    spinlock_unlock(&lock->mutex);
}

void
rw_read_unlock(struct read_write_lock * lock)
{
    atomic_sub(&lock->num_readers, 1);
}

void
rw_write_lock(struct read_write_lock * lock)
{
    spinlock_lock(&lock->mutex);
    while(lock->writer || lock->num_readers > 0);
    atomic_add(&lock->writer, 1);
    spinlock_unlock(&lock->mutex);
}


void
rw_write_unlock(struct read_write_lock * lock)
{
    atomic_sub(&lock->writer, 1);
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
		     uintptr_t expected,
		     uintptr_t new)
{
    uintptr_t init_ptr;
   
    __asm__ __volatile__(
        "   lock;          \n"
        "   cmpxchg8b %1   \n"
        :"=a"(init_ptr),
	 "+m"(*ptr)
	:"d"((uint32_t)(expected >> 32)),
         "a"((uint32_t)expected),
       	 "c"((uint32_t)(new >> 32)),
	 "b"((uint32_t)new)	
 	:"cc"
        );
    
    return init_ptr;
}



void
lf_queue_init(struct lf_queue * queue)
{
	queue->head = malloc(sizeof(struct node));
	queue->tail = malloc(sizeof(struct node));
	queue->head->next = NULL;
	queue->tail->next = NULL;
}

void
lf_queue_deinit(struct lf_queue * lf)
{
	lf->head = NULL;
	lf->tail = NULL;
}

void
lf_enqueue(struct lf_queue * queue,
	   int               val)
{
	struct node *q = malloc(sizeof(struct node));
	struct node *p;

	q->value = val;
	q->next = NULL;

	int succ = 0;
	
	while(succ ==0){
	    p = queue->tail;
	   
	    compare_and_swap_ptr(
	                  (uintptr_t *)&p->next,
			  (uintptr_t)NULL,(uintptr_t)q);
            succ = (q==p->next);	   

	    if(succ == 0){
	        compare_and_swap_ptr(
			  (uintptr_t *)&queue->tail, 
	                  (uintptr_t)p, (uintptr_t)(p->next));		
	    
	  }
		
	}
	compare_and_swap_ptr((uintptr_t*)&(queue->tail), (uintptr_t)p, (uintptr_t)q);
}

int
lf_dequeue(struct lf_queue * queue,
	   int             * val)
{
    int succ = 0;
    struct node *p;
    uintptr_t oldhead;

    while(succ ==0) {
        p = queue->head;
	if(p->next == NULL)
		return 0;
  
    oldhead=compare_and_swap_ptr(
		  (uintptr_t *)&queue->head,
		  (uintptr_t)p, (uintptr_t)p->next);
    *val = p->next->value;
    }
    
    return 1;
}



