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
  /*  struct mem_map * tmp;
    struct list_head *pos;
    int i;
    INIT_LIST_HEAD(&map->node);    

    printk("Memory allocation\n");
//    printk("page size: %x\n", PAGE_SIZE_4KB);


//    printk("map->size: %lx\n", map->size);
//    printk("map->start: %lx\n", map->start);
//    printk("end: %lx\n", PETMEM_REGION_END);
*/
 /*
    for(i=0; i<=5; ++i){
	tmp = (struct mem_map *)kmalloc(sizeof(struct mem_map), GFP_KERNEL);
        tmp->allocated = i;
	
	list_add_tail(&(tmp->node), &(map->node));
    }
       

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
    struct mem_map * tmp2;
    list_for_each_entry_safe(tmp, tmp2, &(map->node), node){

    }
*/
    // printk("buddy_return2: %lx\n", buddy_return2);
    //petmem_free_pages(0xffff880000000000 + buddy_return2, 1); 
    //petmem_free_pages(buddy_return, 1);
    
   
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
    
    //unsigned long pdpe_table_page;   
    unsigned long pde_table_page;    
    unsigned long pte_table_page;   
    uintptr_t user_page;

    pml4e64_t * pml;
    pdpe64_t * pdpe;
    pde64_t * pde;
    pte64_t * pte;
 
    //printk("pml64_index: %ul\n pdpe_index: %ul\n pdp_index: %ul\n pte_index: %ul\n", pml4_index, pdpe_index, pde_index, pte_index);  

    
    printk("Page fault! error_code: %d    fault address: %p\n", error_code, (void*)fault_addr);
    
//    printk("CR3_TO_PML4E64_VA(cr3): %p\n", CR3_TO_PML4E64_VA(cr3));

    pml = CR3_TO_PML4E64_VA(cr3)  + pml4_index*sizeof(pml4e64_t);

  //  printk("pml4 present: %d\n", pml->present);

    pdpe = __va(BASE_TO_PAGE_ADDR(pml->pdp_base_addr)) + pdpe_index*sizeof(pdpe64_t);

    if(pdpe->present == 0){		
        //pde_table_page = (unsigned long)kmalloc(sizeof(PAGE_SIZE_4KB), GFP_KERNEL);
        pde_table_page = __get_free_page(GFP_KERNEL);
        //printk("pdpe->present == 0    allocating page for pde table at %p\n", (void*)pde_table_page);
        //printk("PAGE_TO_BASE_ADDR(__pa(pde_table_page)): %lu\n", PAGE_TO_BASE_ADDR(__pa(pde_table_page))); 
        pdpe->pd_base_addr = PAGE_TO_BASE_ADDR(__pa(pde_table_page));	
	pdpe->present = 1;
        pdpe->writable = 1;
        pdpe->user_page = 1;
    }

    pde = __va(BASE_TO_PAGE_ADDR(pdpe->pd_base_addr)) + pde_index*sizeof(pde64_t);
    //printk("pde: %p\n", pde);
    //printk("pde->present: %d\n", pde->present);
    
    if(pde->present == 0){ 
        //pte_table_page = (unsigned long)kmalloc(sizeof(PAGE_SIZE_4KB), GFP_KERNEL);
        pte_table_page = __get_free_page(GFP_KERNEL);
        //printk("pde->present == 0     allocating page for pte table at %p\n", (void*)pte_table_page);
        //printk("PAGE_TO_BASE_ADDR(__pa(pte_table_page)): %lu\n", PAGE_TO_BASE_ADDR(__pa(pte_table_page))); 
        pde->pt_base_addr = PAGE_TO_BASE_ADDR(__pa(pte_table_page));	
	pde->present = 1;
        pde->writable = 1;
        pde->user_page = 1;
    }
    printk("pde->accessed: %d\n", pde->accessed);   

    pte = __va(BASE_TO_PAGE_ADDR(pde->pt_base_addr)) + pte_index*sizeof(pte64_t); 
    //printk("pte: %p\n", pte);
    //printk("pte->present: %d\n", pte->present);

    if(pte->present == 0){
	//user_page = get_zeroed_page(GFP_KERNEL);
        user_page = petmem_alloc_pages(1);
        printk("pte->present == 0    allocating user page at %lu\n", user_page);
        //printk("user_page lx %lx   user_page p %p    user_page  lu %lu\n", user_page, user_page, user_page);
        printk("PAGE_TO_BASE_ADDR(__pa(user_page)): %lu\n", PAGE_TO_BASE_ADDR(__pa(user_page))); 
        pte->page_base_addr = PAGE_TO_BASE_ADDR(__pa(user_page));	
	pte->present = 1;
        //invlpg(__pa(user_page));
        pte->user_page = 1;
        pte->writable = 1;
    }

    
    //printk("pte->accessed: %d\n", pte->accessed); 
    //char *user = (char*)user_page; 

    printk("pte->accessed: %d\n", pte->accessed); 
    //printk("user page byte: %c\n", *user);   
 
    pte->accessed=0;
    pde->accessed=0;

    return 0;
    //return -1;
}
