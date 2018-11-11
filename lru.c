#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int empty;

struct stacknode{
	int number;
	struct stacknode* next;
	struct stacknode* prev;
};

struct mapentry{
	int isref;
	struct stacknode* node;
};

struct mapentry* refmap;

struct stacknode* headnode;

struct stacknode* tail;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int lru_evict() {
	int evict_frame = headnode->number;
        struct stacknode *evict_node = headnode;
	refmap[evict_frame].node = NULL;
	refmap[evict_frame].isref = 0;
	headnode = headnode->next;
	headnode->prev = NULL;
	free(evict_node);
        coremap[evict_frame].in_use = 0;
	return evict_frame;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
    int frame = (p->frame) >> PAGE_SHIFT;
	if(refmap[frame].isref){
		struct stacknode* curnode = refmap[frame].node;
                if(curnode->next != NULL){ //otherwise already at the tail and don't need to be moved to tail
                        if(curnode->prev != NULL){
 		                (curnode->prev)->next = (curnode->next);
		                (curnode->next)->prev = curnode->prev;
		                curnode->prev = tail;
                                curnode->next = NULL;
		                tail->next = curnode;
		                tail = curnode;
                        }else{ //at the front
                                headnode = headnode->next; //update headnode in this case!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                                (curnode->next)->prev = NULL;
		                curnode->prev = tail;
                                curnode->next = NULL;
		                tail->next = curnode;
		                tail = curnode;
                        }
                }
	} else {
                struct stacknode* newnode = malloc(sizeof(struct stacknode));
                newnode->number = frame;
                newnode->next = NULL;
                newnode->prev = NULL;
                if(empty){
                        headnode = newnode;                       
                        tail = newnode;
                        empty = 0;

                }else{
                        tail->next = newnode;
		        newnode->prev = tail;
                        tail = newnode;
                }		
		refmap[frame].isref = 1;
		refmap[frame].node = newnode;
	}
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
        for(int k = 0; k < memsize; k++){  
                coremap[k].in_use = 0;       
                coremap[k].pte = NULL;
                coremap[k].vaddr = -1;
        }
        empty = 1;
	refmap = malloc(sizeof(struct mapentry) * memsize);
	/*headnode = malloc(sizeof(struct stacknode));
	headnode->next = NULL;
	headnode->prev = NULL;
        headnode->number = -1;
	tail = headnode;*/
        headnode = NULL;
        tail = NULL;
	for(int i = 0; i < memsize; i++){
		refmap[i].isref = 0;
		refmap[i].node = NULL;
	}
}
