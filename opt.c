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

// whether there is element in the linked list
int empty;

/*
 * Helper function to find the distance between next use of the address.
 */
int cal_nextuse(addr_t addr){
	int res = 0;
	struct refnode* cur = refhead;
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
                        maxdistance = curdistance;
		}
	}
        coremap[framenum].in_use = 0;
	return framenum;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
	int frame_index = (p->frame) >> PAGE_SHIFT;
        if(refhead != NULL){
	        coremap[frame_index].vaddr = refhead->refadd;
                struct refnode *referenced = refhead;
                refhead = refhead -> next;
                free(referenced);
        }else {
                printf("error: nothing to be referenced!\n");
        }     
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	//  Read the file, store it into an array of addresses.
	FILE *tfp = stdin;
	char buf[MAXLINE];
	char type;
	empty = 1;
	addr_t vaddr = 0;
        refhead = NULL;
        struct refnode *curnode = NULL;
	struct refnode *newnode = NULL;
        for(int k = 0; k < memsize; k++){  
                coremap[k].in_use = 0;       
                coremap[k].pte = NULL;
                coremap[k].vaddr = -1;
        }
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
                        newnode = (struct refnode *)malloc(sizeof(struct refnode));
			newnode->refadd = vaddr;
                        newnode->next = NULL;
                        if(empty){
                                refhead = newnode;
                                curnode = newnode;
                                empty = 0;
                        }else{
                                curnode->next = newnode;
                                curnode = curnode->next;
                        }
		} else {
			continue;
		}		
	}
	fclose(tfp);
}

