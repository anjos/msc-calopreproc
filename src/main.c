/* Hello emacs, this is -*- c -*- */

/* This package doesn't belong to any other package I've known. It just takes
   ASCII files produced at CERN and provides a better interface to them. This
   way they can be plotted or so. It uses only the CaloDigi's that may be
   present in the file. The building of this file is accomplished by make (1).
*/ 

/* $Id: main.c,v 1.4 2000/07/07 18:51:38 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "util.h"
#include "data.h" /* the data file description */
#include "trigtowr.h" /* for processing the data into trigger towers */
#include "ring.h"
#include "uniform.h"

#ifdef TRACE_DEBUG
#include <mcheck.h>
#endif

int getopt(int argc, char* const argv[], const char* optstring);
extern char* optarg;
extern int optind, opterr, optopt;

int main (int argc, char* argv[])
{
  /* I want to capture the errors produced by uniformization. */
  extern int uniform_contour_err;

  FILE* infile = NULL;
  FILE* outfile = NULL; 

  char* name = NULL;

  long int number = -1;
  long int roi = -1;
  long int i;

  bool_t process_all = FALSE;

  bool_t dumptt = TRUE;

  char c;

  tt_roi_t ttroi; /* The calorimeter RoI */

  EVENT event;

#ifdef TRACE_DEBUG
  mtrace();
#endif

  while (EOF != (c=getopt(argc, argv, "e:f:r:o:ha") ) ) {
    switch (c) {
    case 'f':

      /* Ok, open the file we're going to work with, 
	 in case of erro, give up */ 
      if (NULL==(infile=fopen(optarg,"r"))) {
	fprintf(stderr, "(main) Can\'t open input file %s\n", optarg);
	exit(EXIT_FAILURE);
      }

      /* and copy the string to name */
      name = malloc((strlen(optarg)+1)*sizeof(char));
      strcpy(name, optarg);
      break;

    case 'a':
      dumptt = FALSE;
      break;

    case 'e':
      number = to_valid_long(optarg);
      break;

    case 'r':
      roi = to_valid_long(optarg);
      break;

    case 'o':
      if (NULL==(outfile=fopen(optarg,"w"))) {
	fprintf(stderr, "(main) Can\'t open output file %s\n", optarg);
	exit(EXIT_FAILURE);
      }
      break;

    case 'h': /* give help */
    default:
      fprintf(stderr, "(main) Syntax for %s is\n", argv[0]);
      fprintf(stderr, "(main) %s -f <input file> [-e <event>]", argv[0]);
      fprintf(stderr, " [-r <roi>] [-a] [-o <output file>]\n", argv[0]);
      fprintf(stderr, "(main) -a dumps rings instead of cells.\n");
      exit(EXIT_SUCCESS);
      break;

    } /* end of switch on (c) */
  } /* end of while on (getopt) */

  /* we should have at least the input filename */
  if (infile == NULL) {
    fprintf(stderr, "(main) at least the input filename must be supplied.\n");
    exit(EXIT_FAILURE);
  }

   /* I do not have an event selected. I will process all of them */
  if (number < 0) { 
    fprintf(stderr, "(main) I'll be processing all events in file.\n");
    process_all = TRUE;
    roi = 0;
  }

   /* I do not have an RoI selected. Get number of RoIs in event, print and
      exit  */
  if (roi < 0) {
    
     /* wastes the initial VERSION information */
    waste_VERSION(infile);

    /* scan for the event I need to dump */
    event = search_event(infile, number);

    /* now print to the output the number of RoIs in this event */
    fprintf(stderr, "(main) The number of RoIs in event %d is: ", number);
    fprintf(stderr, "%d.\n", event.nroi);
    free_EVENT(&event);
    exit(EXIT_SUCCESS);
  }

  /* if I did not select an output file, dump to stdout */
  if (outfile == NULL) /* output to stdout */ outfile = stdout;

  /* wastes the initial VERSION information */
  waste_VERSION(infile);

  if (!process_all) {
    ring_t ring;
    uniform_roi_t ur;

    /* converts the numbers I have to work with */
    /* Now we can read the data files */
    event = search_event(infile,number);

    if (CALO_ERROR == build_roi(&(event.roi[roi-1]), FALSE, &ttroi)) {
      fprintf(stderr,"(main) Can't put RoI into trigger tower format\n");
      exit(EXIT_FAILURE);
    }

    /* let's check the functionality */
    if (!dumptt) {
      ring_sum (&ttroi, &ring);
      print_ring (outfile, &ring);
      free_ring(&ring);
    }

    else {
      /* Let's do it with uniform::*() */
      uniformize (&ttroi,&ur);
      print_uniform_layer (outfile,&ur,2);
      free_uniform_roi (&ur);
    }

    /* free all resources */
    free_roi(&ttroi);

    /* free the event as always */
    free_EVENT(&event);

  }
  
  else {
    long int counter = 0;
    bool_t has_info = FALSE;
    ring_t ring;
    uniform_roi_t ur;
    int i; /* iterator */

    waste_initial_info(infile);
    
    fprintf(stderr," Progress | Rejected | Dumped \n", counter);
    fprintf(stderr,"----------+----------+-------\n", counter);
    fprintf(stderr,"  %5ld   | %6d   | %5d", counter, uniform_contour_err,
	    counter - uniform_contour_err);
    
    while (read_EVENT(infile,&event) == ERR_SUCCESS) {
      ++counter; 
      
      /* for(i=0; i<event.nroi; ++i) { This line was substituted because, RoI's
				       that are *NOT* the first ones, usually
				       are not well set by libspec */

      for (i=0; i<1; ++i) {
	
	if(event.roi[i].calDigi.nEmDigi == 0 &&
	   event.roi[i].calDigi.nhadDigi == 0) {
	  fprintf(stderr,"\n(main) event[%d].", counter);
	  fprintf(stderr,"roi[%d] has no digi info!\n", i);
	}
   
	if (CALO_ERROR == build_roi(&(event.roi[i]), FALSE, &ttroi)) {
	  fprintf(stderr,"\n(main) Can't put RoI into tt format.\n");
	  exit(EXIT_FAILURE);
	}

	/* let's check the functionality */
	if (!dumptt) {
	  ring_sum (&ttroi,&ring);
	  print_ring (outfile,&ring);
	  free_ring(&ring);
	}

	else
	  /* Let's do it with uniform::*() */
	  if ( uniformize (&ttroi,&ur) != NULL ) {
	    print_uniform_roi (outfile,&ur); 
	    free_uniform_roi (&ur); 
	  }
	
	/* free all resources */
	free_roi(&ttroi);
      
      } /* roi loop */
      
      /* free the event as always */
      free_EVENT(&event);
     
      /* progress report */
      for(i=0; i<28; ++i) fprintf(stderr,"\b");
      fprintf(stderr,"  %5ld   | %6d   | %5d", counter, uniform_contour_err,
	      counter - uniform_contour_err);
      fflush(stderr); /* just makes sure the number is written to the screen, 
			 otherwise it would stay jumping from 1 to 126 with no
			 logic at all */
      
    } /* event loop */
    
    fprintf(stderr,"\n----------+----------+-------\n", counter);
    fprintf(stderr,"(main) I've dumped %ld RoIs.\n", counter -
	    uniform_contour_err);
    
    
  } /* else process all events */

  /* exit gracefully */
  return (EXIT_SUCCESS);
  
} /* end main function */


