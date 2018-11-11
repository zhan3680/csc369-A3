#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

//The hand of the clock.
static int hand;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
        int res;
	pgtbl_entry_t *cur = coremap[hand].pte;
	while(cur->frame & PG_REF){
		cur->frame &= ~PG_REF;
		hand = (hand + 1)%memsize;
                cur = coremap[hand].pte;
	}
        coremap[hand].in_use = 0;
        res = hand;
        hand = (hand + 1)%memsize;
	return res;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
        //update corresponding pte in coremap; 
	int frame_num = p->frame >> PAGE_SHIFT;
        coremap[frame_num].pte = p;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
        for(int k = 0; k < memsize; k++){  
                coremap[k].in_use = 0;       
                coremap[k].pte = NULL;
                coremap[k].vaddr = -1;
        }        
	hand = 0;
}
