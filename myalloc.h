#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void myallocinit(int size);
void *myalloc(int size);
void myfree(void *ptr);
void printallocation();
void mygc();
