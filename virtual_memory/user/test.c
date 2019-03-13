#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "harness.h"


int main(int argc, char ** argv) {
      init_petmem();

/* test 1 - check that we can read/write memory and that freed nodes merge correctly */
/*
    char * buf1 = NULL;
    char * buf2 = NULL;
    char * buf3 = NULL;

    buf1 = pet_malloc(4096);
    buf2 = pet_malloc(4096);
    buf3 = pet_malloc(4096);

    printf("Allocated 1 page at %p\n", buf1);
    fflush(stdout);
  
    pet_dump();

    buf1[50] = 'H';
    buf1[51] = 'e';
    buf1[52] = 'l';
    buf1[53] = 'l';
    buf1[54] = 'o';
    buf1[55] = ' ';

    buf2[60] = 'W';
    buf2[61] = 'o';
    buf2[62] = 'r';
    buf2[63] = 'l';
    buf2[64] = 'd';

    buf3[0] = ' ';
    buf3[1] = '!';
    buf3[2] = '!';
    buf3[3] = '!';
    buf3[4] = '\n';

    printf("%s", (char *)(buf1 + 50));
    fflush(stdout);    
    printf("%s", (char *)(buf2 + 60));
    fflush(stdout);
    printf("%s", (char *)(buf3));
    fflush(stdout);

    // test that empty nodes merge correctly
    pet_dump();
    pet_free(buf2);
    pet_dump();
    pet_free(buf1);
    pet_dump();
    pet_free(buf3);
    pet_dump();

    // this should now segfault
    //printf("%s", (char *)(buf1 + 50));
    //fflush(stdout);

    pet_dump();
*/

/* test 2 - allocate full 128Mb - must disable fault limit in harness.c (~line 90) 
					and can comment out printf SIGSEGV (~line 70) */
/*
    char * tmp = NULL;
    int i = 0;
    int pages = 200;
    tmp = pet_malloc(pages*4096);
    for(i=0; i<pages; i++){
	if(i % 100 == 0)
		printf("i: %d\n", i);
        tmp[50+i*4096] = 1;
    } 

    return 0;
*/


/* test 3 - allocate more than 512 physical pages and check that freeing works */

    char * tmp = NULL;
    int i = 0;
    int pages = 513;
    tmp = pet_malloc(pages*4096);
   
    // get physical pages
    for(i=0; i<pages; i++){
	tmp[50+4096*i] = 1;
    }

    // free memeory
    pet_free(tmp);

   











}	
