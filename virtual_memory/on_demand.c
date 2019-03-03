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
    struct mem_map * map;
    struct mem_map * first_entry;
    
    map = kmalloc(sizeof(struct mem_map), GFP_KERNEL);
    map->allocated = -1;

    first_entry = (struct mem_map *)kmalloc(sizeof(struct mem_map), GFP_KERNEL);
    first_entry->allocated = 0;
    first_entry->size = PETMEM_REGION_END - PETMEM_REGION_START;
    first_entry->start = PETMEM_REGION_START;

    INIT_LIST_HEAD(&map->node);
 
    list_add_tail(&(first_entry->node), &(map->node));
        
    return map;
}


void
petmem_deinit_process(struct mem_map * map)
{
    struct mem_map * cur;
    struct mem_map * tmp;

    list_for_each_entry_safe(cur, tmp, &(map->node), node){
	
	/*if(cur->allocated == 1){
		petmem_free_vspace(map, cur->start);
	}
*/
	kfree(cur);
    }

    kfree(map);

}


/*
	returns
		1 - vaddr is in memory map
                0 - otherwise
*/

int
vaddr_in_map(uintptr_t vaddr, struct mem_map * map){
    
    struct mem_map * cur;

    list_for_each_entry(cur, &(map->node), node){
	if(cur->allocated == 1 && vaddr >= cur->start && vaddr <= vaddr + cur->size)
		return 1;
    }

    return 0;
}


uintptr_t
petmem_alloc_vspace(struct mem_map * map,
		    u64              num_pages)
{
    struct mem_map * cur;
    struct mem_map * new;

    unsigned long req_size = num_pages*PAGE_SIZE_4KB;

    printk("Memory allocation\n");

    /*
	look for first node that is not allocated and has size >= to requested size
    */
    list_for_each_entry(cur, &(map->node), node){
	
	if(cur->allocated == 0 && cur->size >= req_size){
	    new = (struct mem_map *)kmalloc(sizeof(struct mem_map), GFP_KERNEL);
	    new->allocated = 0;
            new->size = cur->size - req_size;
            new->start = cur->start + req_size;
	    
            cur->allocated = 1;
            cur->size = num_pages*PAGE_SIZE_4KB;
    		
	    list_add_tail(&(new->node), &(map->node));
	    return cur->start;
	}
    }

    return -1;
}


void
petmem_dump_vspace(struct mem_map * map)
{   
    struct mem_map * tmp;
   
    printk("\npetmem dump vspace:\n"); 

    list_for_each_entry(tmp, &(map->node), node){
    	printk("allocated: %d\n", tmp->allocated);
	printk("size:      %lx\n", tmp->size);
	printk("start:     %lx\n\n", tmp->start);
    }
    
    return;
}



// Only the PML needs to stay, everything else can be freed
void
petmem_free_vspace(struct mem_map * map,
		   uintptr_t        vaddr)
{
    unsigned long int pml4_index = PML4E64_INDEX(vaddr);
    unsigned long int pdpe_index = PDPE64_INDEX(vaddr);
    unsigned long int pde_index = PDE64_INDEX(vaddr);
    unsigned long int pte_index = PTE64_INDEX(vaddr);
    uintptr_t cr3 = get_cr3();

    pml4e64_t * pml;
    pdpe64_t * pdpe;
    pde64_t * pde;
    pte64_t * pte;
    
    unsigned long addr;
    unsigned long page_addr;
    struct mem_map * cur;
    struct mem_map * tmp;
    struct mem_map * next;
    struct mem_map * prev;
    
    printk("Free Memory\n");

    if(vaddr_in_map(vaddr, map) == 0){
	printk("address not in map\n");
	return;
    }

    pml = CR3_TO_PML4E64_VA(cr3)  + pml4_index*sizeof(pml4e64_t);

    pdpe = __va(BASE_TO_PAGE_ADDR(pml->pdp_base_addr)) + pdpe_index*sizeof(pdpe64_t);
    pde = __va(BASE_TO_PAGE_ADDR(pdpe->pd_base_addr)) + pde_index*sizeof(pde64_t);
    pte = __va(BASE_TO_PAGE_ADDR(pde->pt_base_addr)) + pte_index*sizeof(pte64_t);
    pte->present = 0;    
    addr = BASE_TO_PAGE_ADDR(pte->page_base_addr);

    invlpg((unsigned long)__va(addr));

    petmem_free_pages(addr, 1);
    

    page_addr = PAGE_ADDR(vaddr);

    /* 
 	now we remove vaddr from our memory map
    */
    list_for_each_entry_safe(cur, tmp, &(map->node), node){
	if(cur->start == page_addr){
	
                cur->allocated = 0;

	        // check if prev node is empty && combine if so
                prev = list_entry((struct list_head *)&cur->node.prev, struct mem_map, node);
		if(prev->allocated == 0){
			prev->size += cur->size;
			list_del(&cur->node);
	        	cur = prev;
		}

		//  check if next node is empty and combine if so
		next = list_entry((struct list_head *)&cur->node.next, struct mem_map, node);
	        if(next->allocated == 0){
		       	cur->size += next->size;
                        list_del(&cur->node);
			cur = next;
		}
	}
    }

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
    unsigned long int pml4_index = PML4E64_INDEX(fault_addr);
    unsigned long int pdpe_index = PDPE64_INDEX(fault_addr);
    unsigned long int pde_index = PDE64_INDEX(fault_addr);
    unsigned long int pte_index = PTE64_INDEX(fault_addr);
    uintptr_t cr3 = get_cr3();
    
    unsigned long pdpe_table_page;   
    unsigned long pde_table_page;    
    unsigned long pte_table_page;   
    uintptr_t user_page;

    pml4e64_t * pml;
    pdpe64_t * pdpe;
    pde64_t   * pde;
    pte64_t * pte;  

    /*
	first we check if address is in our memory map
    */

    if(vaddr_in_map(fault_addr, map) == 0){
	return -1;
    }
    
    printk("Page fault!\n");

    pml = CR3_TO_PML4E64_VA(cr3)  + pml4_index*sizeof(pml4e64_t);

    if(pml->present == 0){
	pdpe_table_page = __get_free_page(GFP_KERNEL);
	pml->pdp_base_addr = PAGE_TO_BASE_ADDR(__pa(pdpe_table_page));
        pml->present = 1;
        pml->writable = 1;
        pml->user_page = 1;
    }

    pdpe = __va(BASE_TO_PAGE_ADDR(pml->pdp_base_addr)) + pdpe_index*sizeof(pdpe64_t);

    if(pdpe->present == 0){		
        pde_table_page = __get_free_page(GFP_KERNEL);
        pdpe->pd_base_addr = PAGE_TO_BASE_ADDR(__pa(pde_table_page));	
	pdpe->present = 1;
        pdpe->writable = 1;
        pdpe->user_page = 1;
    }
    
    pde = __va(BASE_TO_PAGE_ADDR(pdpe->pd_base_addr)) + pde_index*sizeof(pde64_t);
    
    if(pde->present == 0){ 
        pte_table_page = __get_free_page(GFP_KERNEL);
        pde->pt_base_addr = PAGE_TO_BASE_ADDR(__pa(pte_table_page));	
	pde->present = 1;
        pde->writable = 1;
        pde->user_page = 1;
    }

    pte = __va(BASE_TO_PAGE_ADDR(pde->pt_base_addr)) + pte_index*sizeof(pte64_t); 

    if(pte->present == 0){
        user_page = petmem_alloc_pages(1);
	pte->page_base_addr = PAGE_TO_BASE_ADDR(user_page);
        pte->present = 1;
        pte->user_page = 1;
        pte->writable = 1;
    }


    return 0;
}
