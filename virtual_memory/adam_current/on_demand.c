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
    map = kmalloc(sizeof(struct mem_map), GFP_KERNEL);
 
    map->allocated = 0;
    map->size = PETMEM_REGION_END - PETMEM_REGION_START;
    map->start = PETMEM_REGION_START;

    INIT_LIST_HEAD(&map->node);
    
    return map;
}


void
petmem_deinit_process(struct mem_map * map)
{
    struct mem_map * tmp;

    list_for_each_entry(tmp, &(map->node), node){
	kfree(tmp);
    }

    kfree(map);
}


uintptr_t
petmem_alloc_vspace(struct mem_map * map,
		    u64              num_pages)
{
    struct mem_map * tmp;
    //struct list_head *pos;
    int i;

    printk("Memory allocation\n");


    for(i=1; i<=5; i++){
	tmp = (struct mem_map *)kmalloc(sizeof(struct mem_map), GFP_KERNEL);
        tmp->allocated = i;
	
	list_add_tail(&(tmp->node), &(map->node));
    }

     
/*
    list_for_each(pos, &(map->node)){
	tmp = list_entry(pos, struct mem_map, node);
	if(tmp->allocated == 0 && tmp->size > (num_pages*PAGE_SIZE_4KB)){
		printk("can allocate\n");
	}
	else{
		printk("cannot allocate\n");
        }
    }

*/

/*
    list_for_each_entry(tmp, &(map->node), node){
	printk("tmp->allocated: %d\n", tmp->allocated);
    }
*/

/*
    list_for_each_entry_safe(tmp, tmp2, &(map->node), node){

    }
*/ 
   
    return map->start;
}

void
petmem_dump_vspace(struct mem_map * map)
{   
    struct mem_map * tmp;
   
    printk("\npetmem dump vspace:\n");
    
    printk("allocated: %d\n", map->allocated);
    printk("size:    : %lx\n", map->size);
    printk("start:   : %lx\n\n", map->start);

    list_for_each_entry(map, &(map->node), node){
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
    
    petmem_free_pages(0x800000, 1);
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
    pde64_t * pde;
    pte64_t * pte;
 
    //printk("pml64_index: %ul\n pdpe_index: %ul\n pdp_index: %ul\n pte_index: %ul\n", pml4_index, pdpe_index, pde_index, pte_index);  

    
    printk("Page fault! error_code: %d    fault address: %p\n", error_code, (void*)fault_addr);

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
        //pde_table_page = (unsigned long)kmalloc(sizeof(PAGE_SIZE_4KB), GFP_KERNEL);
        pde_table_page = __get_free_page(GFP_KERNEL);
        pdpe->pd_base_addr = PAGE_TO_BASE_ADDR(__pa(pde_table_page));	
	pdpe->present = 1;
        pdpe->writable = 1;
        pdpe->user_page = 1;
    }

    pde = __va(BASE_TO_PAGE_ADDR(pdpe->pd_base_addr)) + pde_index*sizeof(pde64_t);
    
    if(pde->present == 0){ 
        //pte_table_page = (unsigned long)kmalloc(sizeof(PAGE_SIZE_4KB), GFP_KERNEL);
        pte_table_page = __get_free_page(GFP_KERNEL);
        pde->pt_base_addr = PAGE_TO_BASE_ADDR(__pa(pte_table_page));	
	pde->present = 1;
        pde->writable = 1;
        pde->user_page = 1;
        pde->accessed = 1;
    }

    pte = __va(BASE_TO_PAGE_ADDR(pde->pt_base_addr)) + pte_index*sizeof(pte64_t); 

    if(pte->present == 0){
	//user_page = get_zeroed_page(GFP_KERNEL);
        user_page = petmem_alloc_pages(1);
        pte->page_base_addr = PAGE_TO_BASE_ADDR(__pa(user_page));	
	pte->present = 1;
        invlpg(fault_addr);
        invlpg(__pa(user_page));
        invlpg(user_page);
        pte->user_page = 1;
        pte->writable = 1;
        pte->accessed =1;
        pte->dirty = 1;
    }

/*
    __asm__ __volatile__ ("movq %%cr3, %%rax\n"
                          "movq %%rax, %%cr3\n;"
                           :
			   :
			   );
*/

	//local_flush_tlb_all();

    return 0;
    //return -1;
}
