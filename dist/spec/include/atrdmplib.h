#ifndef CLIB_H_
#define CLIB_H_

/*
 * $Id: atrdmplib.h,v 1.2 2000/07/07 18:24:01 rabello Exp $
 */

/*
 * runtime support for C input/output routines
 */

#include <stdio.h>
#include <stdlib.h>
#include "dbmalloc.h"

/*
 * error codes returned by reading routines
 */

#define ERR_SUCCESS  (0)
#define ERR_EOF      (1)
#define ERR_NO_TAG   (2)
#define ERR_NULL_TAG (3)

typedef char *string;

/*
 * read/write basic types
 */

#ifdef __cplusplus
extern "C" 
{
#endif

int read_int(FILE *fp, int *data);
int write_int(FILE *fp, int *data);
#define create_ints(n) (int *)malloc(sizeof(int) * (n))
#define free_int(ptr) free(ptr)

int read_float(FILE *fp, float *data);
int write_float(FILE *fp, float *data);
#define create_floats(n) (float *)malloc(sizeof(float) * (n))
#define free_float(ptr) free(ptr)

int read_string(FILE *fp, char **data);
int write_string(FILE *fp, char **data);

void free_string(char **data);

int read_short(FILE *fp, short int *data);
int write_short(FILE *fp, short int *data);
#define create_shorts(n) (short *)malloc(sizeof(short) * (n))
#define free_short(ptr) free(ptr)

int check_start_tag(FILE *fp, char *tag);
int check_end_tag(FILE *fp, char *tag);

#ifdef __cplusplus
}
#endif

#endif /* CLIB_H_ */
