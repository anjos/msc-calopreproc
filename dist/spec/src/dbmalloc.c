
/*
 * $Id: dbmalloc.c,v 1.1.1.1 2000/03/13 21:03:44 rabello Exp $
 */

/*
 * very simple debugging versions of malloc() and free()
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static const int prefix_size  = 16;
static const int postfix_size = 16;

static const unsigned char prefix_byte  = 0x5a;
static const unsigned char postfix_byte = 0xa5;

static const unsigned char clear_byte   = 0xcd;
static const unsigned char destroy_byte = 0xee;

typedef enum { false = 0, true = 1} bool;

typedef enum status {
  Allocated, Freed, Undefined
} status;

typedef struct new_entry {
  size_t size;			/* size of allocated region */
  void   *base;			/* base of allocated region */
  void   *addr;			/* (user) address of allocated region */
  status stat;			/* status of allocated region */
} new_entry;

static const int table_size = 1024;

typedef struct block {
  struct block     *next;	/* link to next block */
  struct new_entry table[1024];	/* table of entries */
  int              last;	/* last entry */
} block;

static block *head, *current;

static void init_block(block *bptr)
{
  int i;
  for(i = 0; i < table_size; i++) {
    bptr->table[i].base = 0;
    bptr->table[i].addr = 0;
    bptr->table[i].size = 0;
    bptr->table[i].stat = Undefined;
  }
  bptr->next = 0;
  bptr->last = 0;
}

static block *new_block()
{
  block *bptr = (block *)malloc(sizeof(block));
  if(bptr == 0)
    abort();
  init_block(bptr);
  return bptr;
}

static new_entry *get_new_entry()
{
  /* on first call */
  if(head == 0) {
    head    = new_block();
    current = head;
  }

  /* if table is full, allocate new one */
  if(current->last == table_size) {
    block *bptr   = new_block();
    current->next = bptr;
    current       = bptr;
  }

  return &current->table[current->last++];
}

static new_entry *find_entry(void *addr, status stat)
{
  block *bptr;
  for(bptr = head; bptr != 0; bptr = bptr->next) {
    int i;
    for(i = 0; i < bptr->last; i++) {
      if((bptr->table[i].stat == stat) &&
	 (bptr->table[i].addr == addr)) {
	return &bptr->table[i];
      }
    }
  }

  return 0;
}

static void exit_handler()
{
  block *bptr;
  for(bptr = head; bptr != 0; bptr = bptr->next) {
    int i;
    for(i = 0; i < bptr->last; i++) {
      if(bptr->table[i].stat == Allocated) {
	fprintf(stderr,"MEMCHK: memory region not freed on exit:\n");
	fprintf(stderr,"MEMCHK:   address = 0x%lx\n", 
		(unsigned long)bptr->table[i].addr);
	fprintf(stderr,"MEMCHK:   size    = %ld\n",bptr->table[i].size);
      }
    }
  }
}

static bool installed = false;

void* dbmalloc(size_t size)
{
  new_entry *eptr;
  void      *base;

  if(!installed) {
    atexit(exit_handler);
    installed = true;
  }
  /* fprintf(stderr,"memory allocation before installation\n"); */

  eptr = get_new_entry();
  
  base = malloc(size + prefix_size + postfix_size);
  
  if(base == 0)
    abort();

  eptr->size = size;
  eptr->base = base;
  eptr->addr = (void *)((char *)base + prefix_size);
  eptr->stat = Allocated;

  memset(base, prefix_byte, prefix_size);
  memset((char *)eptr->addr + size, postfix_byte, postfix_size);
  memset(eptr->addr, clear_byte, eptr->size);
  return eptr->addr;
}


void dbfree(void *ptr)
{
  new_entry     *eptr;
  unsigned char *cptr;

  if(ptr == 0)
    return;

  eptr = find_entry(ptr, Allocated);

  if(eptr == 0) {

    if((eptr = find_entry(ptr, Freed)) != 0) {
      fprintf(stderr,"MEMCHK: trying to delete memory twice: 0x%p\n",ptr);
    } else {
      fprintf(stderr,"MEMCHK: invalid pointer deleted: 0x%p\n", ptr);
    }
    abort();
  }

  for(cptr = (unsigned char *)eptr->base;
      cptr < (unsigned char *)eptr->addr;
      cptr++) {
    if(*cptr != prefix_byte) {
      fprintf(stderr,"MEMCHK: prefix area destroyed: 0x%p\n",ptr);
      abort();
    }
  }

  for(cptr = (unsigned char *)eptr->addr + eptr->size;
      cptr < (unsigned char *)eptr->addr + eptr->size + postfix_size;
      cptr++) {
    if(*cptr != postfix_byte) {
      fprintf(stderr,"MEMCHK: postfix area destroyed: 0x%p",ptr);
      abort();
    }
  }

  /* destroy memory */
  memset(eptr->addr, destroy_byte, eptr->size);

  /* free it */
  free(eptr->base);

  eptr->stat = Freed;
}
