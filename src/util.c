/* Hello emacs, this is -*- c -*- */

/* This is an utility library for the dumping routines */

/* $Id: util.c,v 1.5 2000/06/28 15:47:54 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>

#include "data.h"
#include "util.h"
#include "common.h"
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

  fprintf(out, "(util) Using Calorimeter data.\n");
  fprintf(out, "(util) Major version number for Calorimenter is ");
  fprintf(out, "%ld.\n", version.t2caVersion.major);
  fprintf(out, "(util) Minor version number for Calorimenter is ");
  fprintf(out, "%ld.\n", version.t2caVersion.minor);

  free_VERSION(&version);

  return(EXIT_SUCCESS);
}

EVENT search_event(FILE* in, const long evno)
{
  long itor;
  int has_info;
  EVENT event;
  bool_t eliminated = FALSE;

  waste_initial_info(in);

  /* Read in the designated event (evno) */
  for(itor=1; itor<=evno; ++itor) {
    if(read_EVENT(in,&event) != ERR_SUCCESS) {
      fprintf(stderr,"(util) No event %d in file.\n", evno);
      exit(EXIT_FAILURE);
    }

    if (itor < evno) {
      free_EVENT(&event); /* Clearing unneeded event */
      if (itor == 1) fprintf(stderr,"(util) Discarded events -> %4ld", itor);
      fprintf(stderr,"\b\b\b\b");
      fprintf(stderr, "%4ld",itor);
      fflush(stderr); /* just makes sure the number is written to the screen,
		       otherwise it would stay jumping from 1 to 126 with no
		       logic at all */
      eliminated = TRUE;
    }

  } 

  if (eliminated) fprintf(stderr,"\n");
  fprintf(stderr,"(util) Will be processing event %ld...\n", itor-1);

  /* Look into event and find out if it has or has not valid calo information
   */
  if(event.nroi > 0) {
    int i;
    for(i=0; i<event.nroi; ++i)
      if(event.roi[i].calDigi.nEmDigi > 0 || \
	 event.roi[i].calDigi.nhadDigi > 0) {
	has_info = TRUE;
      }
  }
  else {
    fprintf(stderr,"(util) WARNING: event %d has NO RoIs.\n", evno);
  }

  if (! has_info) {
    fprintf(stderr,"(util) WARNING: event %d has NO valid data.\n",evno);
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
    fprintf(stderr,"(util) -%s- is not a valid integer.\n", str);
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
    fprintf(stderr,"(util) -%s- is not a valid float.\n", str);
    exit(EXIT_FAILURE);
  }

  return(number);
}

long count_events(FILE* fp)
{
  long count = 0;
  EVENT event;

  waste_initial_info(fp);
  fprintf(stderr,"%4ld", count); /* Is it ok to write to stdout? */
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

    fprintf(stderr,"\b\b\b\b");
    fprintf(stderr, "%4ld",count);

    fflush(stderr); /* just makes sure the number is written to the screen,
		       otherwise it would stay jumping from 1 to 126 with no
		       logic at all */
  }

  fprintf(stderr,"\b\b\b\b");
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

