/* Hello emacs, this is -*- c -*- */

/* This is an utility library for the dumping routines */

/* $Id: util.c,v 1.1 2000/03/13 21:03:42 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>

#include "data.h"
#include "util.h"
#include <math.h> 

double fabs(double x);

/* functions only seen by this file */
void waste_initial_info(FILE*);
void waste_CALGEOM(FILE*);
void waste_TRTGEOM(FILE*);
void waste_SCTGEOM(FILE*);

int fprint_filespec(FILE* in, FILE* out)
{
  VERSION version;

  if (read_VERSION(in,&version) != ERR_SUCCESS) {
    fprintf(stderr, "(util) Can't locate VERSION module.\n");
    exit(EXIT_FAILURE);
  }

  fprintf(out, "(main) Using Calorimeter data.\n");
  fprintf(out, "(main) Major version number for Calorimenter is ");
  fprintf(out, "%ld.\n", version.t2caVersion.major);
  fprintf(out, "(main) Minor version number for Calorimenter is ");
  fprintf(out, "%ld.\n", version.t2caVersion.minor);

  free_VERSION(&version);

  return(EXIT_SUCCESS);
}

EVENT search_event(FILE* in, const long evno)
{
  long itor;
  EVENT event;

  waste_initial_info(in);

  for(itor=1; itor<=evno; ++itor) {
    if(read_EVENT(in,&event) != ERR_SUCCESS) {
      fprintf(stderr,"(main) No event %d in file.\n", evno);
      exit(EXIT_FAILURE);
    }
    if (itor < evno) free_EVENT(&event); /* I don't need this particular
					    event, I can clear it then. */
    else break;
  }

  return event;
}

long to_valid_long(const char* str)
{
  char** invalid_number = (char**)malloc(sizeof(char*));
  long number;

  *invalid_number = NULL;

  number = strtol(str,invalid_number,10);
  if (**invalid_number != '\0') {
    fprintf(stderr,"(main) -%s- is not a valid integer.\n", str);
    exit(EXIT_FAILURE);
  }

  return(number);
}

double to_valid_double(const char* str)
{
  char** invalid_number = (char**)malloc(sizeof(char*));
  double number;

  *invalid_number = NULL;

  number = strtod(str,invalid_number);
  if (**invalid_number != '\0') {
    fprintf(stderr,"(main) -%s- is not a valid float.\n", str);
    exit(EXIT_FAILURE);
  }

  return(number);
}

long count_events(FILE* fp)
{
  long count = 0;
  EVENT event;

  waste_initial_info(fp);

  while(read_EVENT(fp,&event) == ERR_SUCCESS)
  {
    if(event.nroi > 0) {
      int i;
      for(i=0; i<event.nroi; ++i)
	if(event.roi[i].calDigi.nEmDigi > 0 || \
	   event.roi[i].calDigi.nhadDigi > 0) {
	  ++count;
	  break;
	}
    }
    free_EVENT(&event);
  }
  return count;
}

void waste_initial_info(FILE* fp)
{
  waste_CALGEOM(fp);
  waste_TRTGEOM(fp);
  waste_SCTGEOM(fp);
}

void waste_VERSION(FILE* fp)
{
  VERSION v;
  int errorcode;

  /* the version info may not be present */
  if( (errorcode = read_VERSION(fp,&v)) != ERR_SUCCESS && \
      errorcode != ERR_NULL_TAG) {
    fprintf(stderr,"(util) File contains no version info.\n");
    exit(EXIT_FAILURE);
  }
  free_VERSION(&v);
}

void waste_CALGEOM(FILE* fp)
{
  CALGEOM calg;
  int errorcode;

  /* the calorimeter geometry info may not be present */
  if( (errorcode = read_CALGEOM(fp,&calg)) != ERR_SUCCESS && \
      errorcode != ERR_NULL_TAG) {
    fprintf(stderr,"(util) File contains no calorimeter geometry info.\n");
    exit(EXIT_FAILURE);
  }
  free_CALGEOM(&calg);
}

void waste_TRTGEOM(FILE* fp)
{
  TRTGEOM trtg;
  int errorcode;

  if( (errorcode = read_TRTGEOM(fp,&trtg)) != ERR_SUCCESS && \
      errorcode != ERR_NULL_TAG) {
    fprintf(stderr,"(util) File contains no TRT geometry info.\n");
    exit(EXIT_FAILURE);
  }
  free_TRTGEOM(&trtg);
}

void waste_SCTGEOM(FILE* fp)
{
  SCTGEOM sctg;
  int errorcode;

  if( (errorcode = read_SCTGEOM(fp,&sctg)) != ERR_SUCCESS && \
      errorcode != ERR_NULL_TAG) {
    fprintf(stderr,"(util) File contains no SCT geometry info.\n");
    exit(EXIT_FAILURE);
  }
  free_SCTGEOM(&sctg);

  return;
}

