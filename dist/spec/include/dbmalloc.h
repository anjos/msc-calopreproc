#ifndef _DBMALLOC_H_
#define _DBMALLOC_H_

/*
 * $Id: dbmalloc.h,v 1.2 2000/07/07 18:24:02 rabello Exp $
 */

/* use debugging routines only if DBMALLOC is defined */
#ifdef DBMALLOC

void *dbmalloc(size_t size);
void dbfree(void *ptr);

#define malloc(size) dbmalloc(size)
#define free(ptr)    dbfree(ptr)

#else

#define dbmalloc(size) malloc(size)
#define dbfree(ptr)    free(ptr)

#endif /* DBMALLOC */

#endif /* _DBMALLOC_H_ */
