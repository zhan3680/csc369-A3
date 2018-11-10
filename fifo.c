#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

static int evict;

/* Page to evict is chosen using the fifo algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int fifo_evict() {
	int res = evict;
        evict = (evict + 1) % memsize; 
        coremap[res].in_use = 0; 
	return res;
}

/* This function is called on each access to a page to update any information
 * needed by the fifo algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void fifo_ref(pgtbl_entry_t *p) {             
	return;
}

/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void fifo_init() {   
        for(int k = 0; k < memsize; k++){  
                coremap[k].in_use = 0;       
                coremap[k].pte = NULL;
                coremap[k].vaddr = -1;
        }
        evict = 0;
}
