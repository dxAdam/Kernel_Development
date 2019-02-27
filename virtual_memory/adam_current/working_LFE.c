/* On demand Paging Implementation
 * (c) Jack Lange, 2012
 */

#include <linux/slab.h>


#include "petmem.h"
#include "on_demand.h"
#include "pgtables.h"
#include "swap.h"





struct mem_map *
petmem_init_process(void)
{
    struct mem_map * mmap;
    mmap = kmalloc(sizeof(struct mem_map), GFP_KERNEL);

    mmap->allocated = 0;
    mmap->size = PETMEM_REGION_END - PETMEM_REGION_START;
    mmap->start = PETMEM_REGION_START;

    return mmap;
}


void
petmem_deinit_process(struct mem_map * map)
{
    
}


uintptr_t
petmem_alloc_vspace(struct mem_map * map,
		    u64              num_pages)
{
    struct mem_map * tmp;
    struct list_head *pos;
    int i;
    INIT_LIST_HEAD(&map->node);    

    printk("Memory allocation\n");
    printk("map->start: %ld\n", map->start);

  
    for(i=5; i!=0; --i){
	tmp = (struct mem_map *)kmalloc(sizeof(struct mem_map), GFP_KERNEL);
        tmp->allocated = i;
	
	list_add(&(tmp->node), &(map->node));
    }
	
    list_for_each(pos, &(map->node)){

	tmp = list_entry(pos, struct mem_map, node);
	printk("allocated: %d\n", tmp->allocated);
    }


    return map->start;
}

void
petmem_dump_vspace(struct mem_map * map)
{
    return;
}




// Only the PML needs to stay, everything else can be freed
void
petmem_free_vspace(struct mem_map * map,
		   uintptr_t        vaddr)
{
    printk("Free Memory\n");
    return;
}


/* 
   error_code:
       1 == not present
       2 == permissions error
*/

int
petmem_handle_pagefault(struct mem_map * map,
			uintptr_t        fault_addr,
			u32              error_code)
{
    printk("Page fault!\n");

    return -1;
}
