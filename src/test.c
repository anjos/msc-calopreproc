/* Hello emacs, this is -*- c -*- */

/* This package doesn't belong to any other package I've known. It just takes
   ASCII files produced at CERN and provides a better interface to them. This
   way they can be plotted or so. It uses only the CaloDigi's that may be
   present in the file. The building of this file is accomplished by make (1).
*/ 

/* $Id: test.c,v 1.3 2000/07/07 18:51:50 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "util.h"
#include "data.h" /* the data file description */
#include "trigtowr.h" /* for processing the data into trigger towers */
#include "flat.h" /* for flattening the trigger towers */
#include "portable.h" /* for CellInfo type */

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

  tt_roi_t ttroi; /* The calorimeter RoI */

  FlatEM em; /* The calorimeter EM part of the RoI flattened */
  FlatHad had; /* the hadronic part flattened */

  EVENT event;

  double MAXERR = 0.001;

  while (EOF != (c=getopt(argc, argv, "e:f:r:o:hm:") ) ) {
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

    case 'm':
      MAXERR = to_valid_double(optarg);
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

  /* Shows MAXERR */
  fprintf(outfile, "(teste.c) The maximmum acceptable error for doubles ");
  fprintf(outfile, "is %f.\n", MAXERR);

  /* converts the numbers I have to work with */
  /* Now we can read the data files */
  event = search_event(infile,number);

  /*************************************
   * Let's get to the calorimeter part *
   *************************************/
  fprintf(outfile, "(teste.c) Examining EM part.\n");
  for(i=0;i<event.roi[roi-1].calDigi.nEmDigi;++i) {
    CellInfo cell;
    char ucn[30];
    i2ucn(event.roi[roi-1].calDigi.emDigi[i].id, ucn);

    DecodeId(event.roi[roi-1].calDigi.emDigi[i].id, &cell);
    if(cell.calo*100+cell.region !=
       event.roi[roi-1].calDigi.emDigi[i].CaloRegion) {
      fprintf(outfile,"(teste.c) Cell %d: DecodeId gives me %d and file %d.\n",
	      i+1, cell.calo*100+cell.region,
	      event.roi[roi-1].calDigi.emDigi[i].CaloRegion);
      fprintf(outfile,"(teste.c) UCN for this cell is %s.\n", ucn);
    }
    if(fcomp(cell.center.Phi,event.roi[roi-1].calDigi.emDigi[i].phi,MAXERR)==CALO_ERROR) {
      fprintf(outfile,"(teste.c) Cell %d: DecodeId gives me a phi of ",i+1);
      fprintf(outfile,"%e and the file, %e.\n", cell.center.Phi,
	      event.roi[roi-1].calDigi.emDigi[i].phi);
      fprintf(outfile,"(teste.c) The total difference is %e.\n",
	      (cell.center.Phi-event.roi[roi-1].calDigi.emDigi[i].phi));
      fprintf(outfile,"(teste.c) UCN for this cell is %s.\n", ucn);
    }
    if(fcomp(cell.center.Eta,event.roi[roi-1].calDigi.emDigi[i].eta,0.001)==CALO_ERROR) {
      fprintf(outfile,"(teste.c) Cell %d: DecodeId gives me an eta of ",i+1);
      fprintf(outfile,"%e and the file, %e.\n", cell.center.Eta,
	      event.roi[roi-1].calDigi.emDigi[i].eta);
      fprintf(outfile,"(teste.c) The total difference is %e.\n",
	      (cell.center.Eta-event.roi[roi-1].calDigi.emDigi[i].eta));
      fprintf(outfile,"(teste.c) UCN for this cell is %s.\n", ucn);
    } 
  }

  fprintf(outfile, "(teste.c) Examining HAD part.\n");
  for(i=0;i<event.roi[roi-1].calDigi.nhadDigi;++i) {
    CellInfo cell;
    char ucn[30];
    i2ucn(event.roi[roi-1].calDigi.hadDigi[i].id, ucn);

    DecodeId(event.roi[roi-1].calDigi.hadDigi[i].id, &cell);
    if(cell.calo*100+cell.region !=
       event.roi[roi-1].calDigi.hadDigi[i].CaloRegion) {
      fprintf(outfile,"(teste.c) Cell %d: DecodeId gives me %d and file %d.\n",
	      i+1, cell.calo*100+cell.region,
	      event.roi[roi-1].calDigi.hadDigi[i].CaloRegion);
      fprintf(outfile,"(teste.c) UCN for this cell is %s.\n", ucn);
    }
    if(fcomp(cell.center.Phi,event.roi[roi-1].calDigi.hadDigi[i].phi,MAXERR)==CALO_ERROR) {
      fprintf(outfile,"(teste.c) Cell %d: DecodeId gives me a phi of ",i);
      fprintf(outfile,"%e and the file, %e.\n", i+1, cell.center.Phi,
	      event.roi[roi-1].calDigi.hadDigi[i].phi);
      fprintf(outfile,"(teste.c) The total difference is %e.\n",
	      (cell.center.Phi-event.roi[roi-1].calDigi.hadDigi[i].phi));
      fprintf(outfile,"(teste.c) UCN for this cell is %s.\n", ucn);
    }
    if(fcomp(cell.center.Eta,event.roi[roi-1].calDigi.hadDigi[i].eta,MAXERR)==CALO_ERROR) {
      fprintf(outfile,"(teste.c) Cell %d: DecodeId gives me an eta of ",i);
      fprintf(outfile,"%e and the file, %e.\n", i+1, cell.center.Eta, 
	      event.roi[roi-1].calDigi.hadDigi[i].eta);
      fprintf(outfile,"(teste.c) The total difference is %e.\n",
	      (cell.center.Eta-event.roi[roi-1].calDigi.hadDigi[i].eta));
      fprintf(outfile,"(teste.c) UCN for this cell is %s.\n", ucn);
    } 

  }

  /* free the event as always */
  free_EVENT(&event);

  /* exit gracefully */
  return (EXIT_SUCCESS);
  
} /* end main function */

