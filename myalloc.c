#include "myalloc.h"
#include <stdlib.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <assert.h>
#include <signal.h>

void *myalloc2(int size);
void coalesce();
void mark(int *p);
void sweep(int *p);

//helper functions
int *isPtr(int *p);
int blockMarked(int *p);
void markBlock(int *p);
void unmarkBlock(int *p);
int blockAllocated(int *p);
void allocateBlock(int *p);
void unallocateBlock(int *p);
int length(int *p);
int *nextBlock(int *p);

int heapLen;

int *start;

void myallocinit(int size) {
	int blockLen = ((size+3)/8 + 1) * 8;
	heapLen = blockLen;
	start = (int *)malloc(blockLen+8);

	if (start == NULL){
		fprintf(stderr, "Error initializing space.\n");
		exit(1);
	}

	start++;
	*start = blockLen;
	start++;

	*(nextBlock(start)) = 0;
}

void printallocation() {
	int size = *(start-1);
	int *ptr = start;
	
	while (length(ptr)){
		printf("Mem: %p - Control: %d - Size: %d - M: %d A: %d\n", 
		ptr, *(ptr-1), length(ptr), blockMarked(ptr), blockAllocated(ptr));
		ptr = nextBlock(ptr);
	}
}

void * myalloc(int size) {
	void *myalloc(int size);

	void *ptr = myalloc2(size);

	if (ptr == NULL) {
		coalesce();
		ptr = myalloc2(size);
	}
	return ptr;
}

void *myalloc2(int size) {
    int mem_size = ((size+3)/8 + 1) * 8;

    int *ptr = start;

	int newSize=0;

	int oldsize = *(start-1);

	while(oldsize != 0){
		if (oldsize >= mem_size && !blockAllocated(ptr)){
			*(ptr-1) = mem_size; 	
			allocateBlock(ptr); 
			newSize = oldsize - mem_size;
			if (newSize > 0)
				*(nextBlock(ptr)-1) = newSize;
			return ptr;
		}
		ptr = nextBlock(ptr);

		oldsize = *(ptr-1);
	}
	fprintf(stderr, "No space to allocate more\n");
  	return NULL;
}

void myfree(void *p) {
	unallocateBlock(p); 
}

void coalesce() {
	int *ptr = start;
  	int *nextptr; 
  	
  	while (length(ptr)){
		nextptr = nextBlock(ptr);

		//need to check for conditions to coalesce
		// if conditions not met, move on to next block till end

		if (*(nextptr-1) == 0 || blockAllocated(ptr) || blockAllocated(nextptr)){
			ptr = nextBlock(ptr);
			continue;
		}
		*(ptr-1) += length(nextptr);
	}
}

void mygc() {
  //int **max = (int **) 0xbfffffffUL;   // the address of the top of the stack
  unsigned long stack_bottom;
  int **max =  (int **) GC_init();   // get the address of the bottom of the stack
  int* q;
  int **p = &q;    // the address of the bottom of the stack
  
  while (p < max) {
    //printf("0. p: %u, *p: %u max: %u\n",p,*p,max);
    mark(*p);
    p++;
  }

    //now fill in the rest think sweep , think coalesce
  sweep(q);
  coalesce();
}


void mark(int *p) {
  int size;
  int *ptr;
  int i = 0;

  //go for it
  ptr = isPtr(p);
  if (ptr != NULL && blockAllocated(ptr)){
  	if (!blockMarked(ptr)){
  		markBlock(ptr);
  	}
  	size = length(ptr)/4;
  	while (++i < size){
  		mark(ptr+i);
  	}
  }
}

void sweep(int *ptr) {
	ptr = start;

	while (length(ptr)){
		if (blockAllocated(ptr) && !blockMarked(ptr)){
			myfree(ptr);
		} else{
			unmarkBlock(ptr);
		}
		ptr = nextBlock(ptr);
	}
 
}

int *isPtr(int *p) {
 //return the pointer or NULL
  int *head = start;
  
  //now do the rest

  if (p <= (head+heapLen) && head <= p){
  	return p;
  }

  return NULL;
}

int blockMarked(int *p) {
	return *(p-1) != (*(p-1) & ~0x2);  // ~0x2 checks num against 1...1101
}

void markBlock(int *p) {
	*(p-1) = (*(p-1)+2); // changes second-last bit to 1
}

void unmarkBlock(int *p) {
	*(p-1) = (*(p-1)-2); // changes second-last bit to 0
}

int blockAllocated(int *p) {
	return (*(p-1)%2 != 0); // if not div by 2, its allocated (last bit == 1)  
}

void allocateBlock(int *p) {
	*(p-1) = (*(p-1)+1);
}

void unallocateBlock(int *p) {
	*(p-1) = (*(p-1)-1); 
}

int *nextBlock(int *p) {
	return (p + length(p)/4); 
}

int length(int *p) {
	return (*(p-1)/8*8); // because div is floored, forces ctrl word to 000 leaving only size
}