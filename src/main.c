/* Hello emacs, this is -*- c -*- */

/* This package doesn't belong to any other package I've known. It just takes
   ASCII files produced at CERN and provides a better interface to them. This
   way they can be plotted or so. It uses only the CaloDigi's that may be
   present in the file. The building of this file is accomplished by make (1).
*/ 

/* $Id: main.c,v 1.2 2000/05/31 11:51:51 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "util.h"
#include "data.h" /* the data file description */
#include "trigtowr.h" /* for processing the data into trigger towers */
#include "flat.h" /* for flattening the trigger towers */

int getopt(int argc, char* const argv[], const char* optstring);
extern char* optarg;
extern int optind, opterr, optopt;

int main (int argc, char* argv[])
{
  FILE* infile = NULL;
  FILE* outfile = NULL; 

  char* name = NULL;

  long int number = -1;
  long int roi = -1;
  long int i;

  char c;

  CaloTTEMRoI ttroi; /* The calorimeter RoI */

  FlatEM em; /* The calorimeter EM part of the RoI flattened */
  FlatHad had; /* the hadronic part flattened */

  EVENT event;

  while (EOF != (c=getopt(argc, argv, "e:f:r:o:h") ) ) {
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
      fprintf(stderr, "(main) %s -f <input file> -e <event>", argv[0]);
      fprintf(stderr, "-r <roi> -o <output file>\n", argv[0]);
      exit(EXIT_SUCCESS);
      break;

    } /* end of switch on (c) */
  } /* end of while on (getopt) */

  /* we should have at least the input filename */
  if (infile == NULL) {
    fprintf(stderr, "(main) at least the input filename must be supplied.\n");
    exit(EXIT_FAILURE);
  }

   /* I do not have an event selected. Count no. of valid events and exit */
  if (number < 0) {
    fprint_filespec(infile,stderr);
    fprintf(stderr, "(main) The number of events ");
    fprintf(stderr, "(with 1 RoI with DIGIs, at least) is ");
    fprintf(stderr, "%ld.\n", count_events(infile));
    exit(EXIT_SUCCESS);
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

  /* converts the numbers I have to work with */
  /* Now we can read the data files */
  event = search_event(infile,number);
  

  /*************************************
   * Let's get to the calorimeter part *
   *************************************/
  /* Using libcalo.a. I will build a trigger tower, with zero suppression
     (actually, the library doesn't work right with zero suppression flag
     OFF) and will put the results in the variable tt */
  if (CALO_ERROR == BuildCaloTTS(&(event.roi[roi-1]), TRUE, &ttroi)) {
    fprintf(stderr,"(main) Can't put RoI into trigger tower format\n");
    exit(EXIT_FAILURE);
  }

  /* Flattening this roi. It's the easyiest way to produce something */
  if (CALO_ERROR == Flatten(&ttroi, em, had)) {
    fprintf(stderr,"(main) Can't flatten trigger tower\n");
    exit(EXIT_FAILURE);
  }

  /* Now, printing results in to file */
  fprintf_FLATEM(outfile, em);
  fprintf_FLATHAD(outfile, had);
  
  /* free all resources */
  FreeCaloEMRoI(&ttroi);

  /* free the event as always */
  free_EVENT(&event);

  /* exit gracefully */
  return (EXIT_SUCCESS);
  
} /* end main function */


