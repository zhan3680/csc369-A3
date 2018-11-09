#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#define MAXLINE 256

extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char *tracefile; 

struct refnode{
	addr_t refadd;
	struct refnode* next;
};

struct refnode *refhead;

struct refnode *curnode;

// The line of current reference correspond to tracefile.
int refindex = 0;

// The total number of reference.
int totalref;

// The map to store all references.
addr_t *allref;

/*
 * Helper function to find the distance between next use of the address.
 */
int cal_nextuse(addr_t addr){
	int res = 0;
	struct refnode* cur = curnode;
	while(cur != NULL){
		if(cur->refadd == addr){
			return res;
		}
		cur = cur->next;
		res += 1;
	}
	return -1;
}

/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	// Want to translate between 'vaddr' and 'coremap.pte' to find the 
	// furthest use.
	int maxdistance = 0;
	int curdistance = 0;
	int framenum = 0;
	for(int i = 0; i < memsize; i++){
		curdistance = cal_nextuse(coremap[i].vaddr);
		if(curdistance == -1){
			return i;
		}
		if(curdistance > maxdistance){
			framenum = i;
		}
	}

	return framenum;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	int frame = (p->frame) >> PAGE_SHIFT;
	coremap[frame].vaddr = curnode->refadd;
	free(curnode);
	curnode = curnode->next;
	refindex += 1;
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	//  Read the file, store it into an array of addresses.
	FILE *tfp = stdin;
	char buf[MAXLINE];
	char type;
	totalref = 0;
	addr_t vaddr = 0;
	refhead = malloc(sizeof(struct refnode));
	curnode = refhead;
	
	// Open the source file.
	if(tracefile != NULL) {
		if((tfp = fopen(tracefile, "r")) == NULL) {
			perror("Error opening tracefile:");
			exit(1);
		}
	}
	
	while(fgets(buf, MAXLINE, tfp) != NULL){
		if(buf[0] != '=') {
			sscanf(buf, "%c %lx", &type, &vaddr);
			curnode->refadd = vaddr;
			curnode->next = malloc(sizeof(struct refnode));
			curnode = curnode->next;
		} else {
			continue;
		}
		totalref += 1;
	}
	curnode->next = NULL;
	curnode = refhead;
	fclose(tfp);
}

