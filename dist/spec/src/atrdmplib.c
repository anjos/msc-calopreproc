
/*
 * $Id: atrdmplib.c,v 1.2 2000/07/07 18:24:03 rabello Exp $
 */

/* 
 * run-time support for C input/output routines
 */

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "atrdmplib.h"

#define MAX_STRING  512
#define MAX_NUMBER   32

/* current line number */
static int lineno = 1;

static void error(const char *msg)
{
  fprintf(stderr,"line %d: %s\n", lineno, msg);
}

/* skip whitespace, count lines */
static void skip_ws(FILE *fp)
{
  int c;

  while(isspace(c = fgetc(fp))){ 
    if (c == '\n') 
      lineno++;
  }

  if(c != EOF)
    ungetc(c, fp);
}

int read_int(FILE *fp, int *data)
{
  char buffer[MAX_NUMBER];
  int  index;
  int  c;

  assert(data != 0);

  /* skip whitespace */
  skip_ws(fp);

  /* read digits */
  index = 0;
  while(isdigit(c = fgetc(fp)) ||
	(c == '-') ||
	(c == '+')) {
    buffer[index++] = c;
  }

  buffer[index] = '\0';

  ungetc(c, fp);

  if(index > 0) {
    *data = strtol(buffer,0,0);
    return 1;
  } else {
    error("integer number expected");
    return 0;
  }
}

int write_int(FILE *fp, int *data)
{
  assert(data != 0);
  return (fprintf(fp," %d", *data) == 1);
}

int read_float(FILE *fp, float *data)
{

  char buffer[MAX_NUMBER];
  int  index;
  int  c;

  assert(data != 0);

  /* skip whitespace */
  skip_ws(fp);

  /* read digits */
  index = 0;
  while(isdigit(c = fgetc(fp)) ||
	(c == 'e') ||
	(c == 'E') ||
	(c == '+') ||
	(c == '-') ||
	(c == '.')) {
    buffer[index++] = c;
  }

  buffer[index] = '\0';

  ungetc(c, fp);

  if(index > 0) {
    *data = strtod(buffer,0);
    return 1;
  } else {
    error("floating point number expected");
    return 0;
  }  
}

int write_float(FILE *fp, float *data)
{
  assert(data != 0);
  return (fprintf(fp," %e", *data) == 1);
}

int read_string(FILE *fp, char **data)
{
  char buffer[MAX_STRING];
  int  index;
  int  c;

  assert(data != 0);

  /* skip whitespace */
  skip_ws(fp);

  index = 0;
  while(!isspace(c = fgetc(fp))) {
    if(c == EOF)
      break;
    buffer[index++] = c;
  }

  buffer[index] = '\0';

  ungetc(c,fp);

  if(index > 0) {

    /* allocate data */
    *data = (char *)malloc(sizeof(char) * strlen(buffer) + 1);

    if(*data == 0)
      return 0;
  
    strcpy(*data, buffer);

    return 1;
  } else {
    error("string expected");
    return 0;
  }
}

int write_string(FILE *fp, char **data)
{
  assert((data != 0) && (*data != 0));
  fprintf(fp,"%s",*data);
  return 1;
}

void free_string(char **data)
{
  assert((data != 0) && (*data != 0));
  free(*data);
}

int read_short(FILE *fp, short int *data)
{
  int result;

  assert(data != 0);

  if(read_int(fp,&result)) {
    *data = (short int)result;
    return 1;
  }
  return 0;
}

int write_short(FILE *fp, short int *data)
{
  assert(data != 0);
  fprintf(fp," %hd", *data);
  return 1;
}

/* keep results of invalid start tag */
static int  start_tag_found = 0;
static char buffer[MAX_STRING];

/* read int next tag */
static int get_tag(FILE *fp)
{
  int ch;
  int index;

  /* skip any whitespace */
  skip_ws(fp);

  index = 0;

  while(isalnum(ch = fgetc(fp))) {
    buffer[index++] = ch;
  }
  
  buffer[index] = '\0';
  
  ungetc(ch, fp);

  return ERR_SUCCESS;
}

/* check if data block is empty */
static int check_empty(FILE *fp, char *tag)
{
  int ch;

  /* check for empty blocks */
  skip_ws(fp);
  
  /* support only
   *    { TAG } TAG 
   */
  if((ch = fgetc(fp)) == '}') {
    ungetc(ch, fp);
    if(check_end_tag(fp,tag) == ERR_SUCCESS)
      return ERR_NULL_TAG;
    
    error("invalid empty block: no matching end tag");
    return ERR_NO_TAG;
  }
  
  ungetc(ch, fp);
  return ERR_SUCCESS;
}

int check_start_tag(FILE *fp, char *tag)
{
  int  ch;
  int  res;

  /* check if previous call found a start tag */
  if(start_tag_found) {

    /* yes, compare it with the current tag */
    if(strcmp(buffer,tag) == 0) {
      start_tag_found = 0;
      return ERR_SUCCESS;	/* found it */
    }
    return ERR_NO_TAG;		/* wrong again */
  }

  /* skip white space */
  skip_ws(fp);
  
  ch = fgetc(fp);
  
  /* check for end of file */
  if(ch == EOF)
    return ERR_EOF;
  
  /* char must be start token */
  if(ch != '{') {
    ungetc(ch, fp);
    error("start of tag expected");
    return ERR_NO_TAG;
  }

  if((res = get_tag(fp)) != ERR_SUCCESS)
    return res;

  /* return result */
  if(strcmp(tag,buffer) == 0) {
    return check_empty(fp, tag);
  } else {
    start_tag_found = 1;
    return ERR_NO_TAG;
  }

}

int check_end_tag(FILE *fp, char *tag)
{
  int ch;
  int res;

  /* skip white space */
  skip_ws(fp);
  
  ch = fgetc(fp);
  
  /* nothing found */
  if(ch == EOF)
    return ERR_EOF;
  
  /* must find start/end token */
  if(ch != '}') {
    ungetc(ch, fp);
    error("end tag expected");
    return ERR_NO_TAG;
  }

  if((res = get_tag(fp)) != ERR_SUCCESS)
    return res;

  /* return result */
  if(strcmp(tag, buffer) == 0) 
    return ERR_SUCCESS;

  error("wrong end tag");

  return ERR_NO_TAG;
}
