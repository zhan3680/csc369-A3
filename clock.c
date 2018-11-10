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
	pgtbl_entry_t *cur = coremap[hand].pte;
	while(cur->frame & PG_REF){
		cur->frame &= ~PG_REF;
		hand = (hand + 1)%memsize;
	}
        coremap[hand].in_use = 0;
	return hand;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	p->frame |= PG_REF;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm. 
 */
void clock_init() {
	hand = 0;
}
