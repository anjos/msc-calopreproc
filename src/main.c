/* Hello emacs, this is -*- c -*- */

/* This package doesn't belong to any other package I've known. It just takes
   ASCII files produced at CERN and provides a better interface to them. This
   way they can be plotted or so. It uses only the CaloDigi's that may be
   present in the file. The building of this file is accomplished by make (1).
*/ 

/* $Id: main.c,v 1.5 2000/07/12 04:31:19 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>

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

/* The following type define the parameters for program execution */
typedef struct pararamter_t 
{
  FILE* ifp; /* The input file pointer */
  FILE* ofp; /* The output file pointer */
  long int event_no; /* The event number to process */
  long int roi_no; /* The roi number to process */
  bool_t dump_rings; /* Do I have to dump rings? */
  bool_t dump_digis; /* Do I have to dump digis? */
  bool_t dump_uniform_digis; /* Do I have to dump digis from uniform RoIs? */
  bool_t process_all_events; /* Do I have to process all events? */
  bool_t process_all_rois; /* Do I have to process all rois for each event? */
  bool_t verbose; /* Do I have to process all rois for each event? */
} parameter_t;

void process_flags (parameter_t*, const int, char**);
void test_flags (parameter_t*);
void print_help_msg(FILE*, const char*);
void print_progress(FILE*, const bool_t, const int, const int);
bool_t process_ROI(const ROI*, const parameter_t*);
void process_EVENT (const EVENT*, const parameter_t*);

int main (int argc, char* argv[])
{
  parameter_t params;

  /* I want to capture the errors produced by uniformization. */
  extern int uniform_contour_err;
  long int i;
  EVENT event;

#ifdef TRACE_DEBUG
  mtrace();
#endif
  
  /* define the default parameters */
  params.ifp = 0;
  params.ofp = stdout; /* by default, dump to screen */
  params.event_no = 0;
  params.roi_no = 0;
  params.dump_rings = FALSE; /* rings are a specific case */
  params.dump_digis = FALSE; /* digis are a specific case */
  params.dump_uniform_digis = FALSE; /* uniform_digis are a specific case */
  params.process_all_events = TRUE; /* process all by default */
  params.process_all_rois = TRUE; /* processs all rois */
  params.verbose = FALSE; /* verbose output? */

  /* capture new optional parameters */
  process_flags(&params,argc,argv);

  if (!params.process_all_events) {
    event = search_event(params.ifp, params.event_no);
    process_EVENT(&event, &params);
    free_EVENT(&event);
  }
  
  else {
    long int counter = 0;

    waste_initial_info(params.ifp);
    print_progress(stderr, params.verbose, counter, uniform_contour_err);
    
    while (read_EVENT(params.ifp,&event) == ERR_SUCCESS) {
      process_EVENT(&event, &params);
      free_EVENT(&event);
      ++counter;      
      print_progress(stderr, params.verbose, counter, uniform_contour_err);
    } /* event loop */

    if (params.verbose) fprintf(stderr,"\n"); /* just to make sure we don't
						 start over in the middle of
						 the screen */

  } /* else process all events */

  /* exit gracefully */
  return (EXIT_SUCCESS);
  
} /* end main function */

/* This function processes the initial '-- and -' pararamteres, changing the
   the structure of parameters accordingly. */
void process_flags (parameter_t* p, const int argc, char** argv)
{
  int c;
  int option_index;

  /* This global defines the options to take */
  static struct option long_options[] = 
  { 
    /* These options set a flag. */ 
    {"input-file", 1, 0, 'f'},
    {"output-file", 1, 0, 'o'}, 
    {"help", 0, 0, 'h'},
    {"event-number", 1, 0, 'e'}, 
    {"roi-number", 1, 0, 'r'}, 
    {"dump-rings", 0, 0, 'a'},
    {"dump-digis", 0, 0, 'd'},
    {"dump-uniform-digis", 0, 0, 'g'},
    {"verbose", 0, 0, 'v'},
    {0, 0, 0, 0}
  };
  
  while (EOF != (c=getopt_long(argc, argv, "e:f:r:o:havd", 
			       long_options, &option_index) ) ) {
    switch (c) {

    case 0: /* Got a flagged option */
      break;
      
    case 'f': /* input from file 'infile' */
      
      /* Ok, open the file we're going to work with, in case of error, 
	 give up */ 
      if (NULL==(p->ifp=fopen(optarg,"r"))) {
	fprintf(stderr, "(main) Can't open input file %s\n", optarg);
	exit(EXIT_FAILURE);
      }
      break;

    case 'a': /* dump rings instead of plain RoIs */
      p->dump_rings = TRUE;
      break;

    case 'd': /* dump digis instead of plain RoIs */
      p->dump_digis = TRUE;
      break;

    case 'g': /* dump uniformizable RoI digis instead of plain RoIs */
      p->dump_uniform_digis = TRUE;
      break;

    case 'v': /* dump rings instead of plain RoIs */
      p->verbose = TRUE;
      break;

    case 'e': /* dump only event 'number' */
      p->event_no = to_valid_long(optarg);
      p->process_all_events = FALSE;
      break;

    case 'r': /* dump only the roi 'roi'. This option has only sense with -e
		 comming first! */
      p->roi_no = to_valid_long(optarg);
      p->process_all_rois = FALSE;
      break;

    case 'o': /* output to file 'outfile' */
      if (NULL==(p->ofp=fopen(optarg,"w"))) {
	fprintf(stderr, "(main) Can\'t open output file %s\n", optarg);
	exit(EXIT_FAILURE);
      }
      break;

    case 'h': /* give help */
    case '?':
    default:
      print_help_msg(stderr,argv[0]);
      exit(EXIT_SUCCESS);
      break;

    } /* end of switch on (c) */
  } /* end of while on (getopt) */

  /* Now we test the coherency of flags */

  test_flags(p);

} /* end of process flags */

/* This function will test the conditions of program execution and see if one
   has a coherent set of flags activated */
void test_flags (parameter_t* p)
{
  /* Will process all ROIs of 1 event -> not acceptable */
  if (!p->process_all_events && p->process_all_rois) {
    fprintf(stderr, "(main)WARN: Can't process all ROIs in one event.\n");
  }

  /* Will process a specific roi for all events */
  if (p->process_all_events && !p->process_all_rois) {
    fprintf(stderr,"(main)ERROR: No sense in not setting the event number.\n");
    exit(EXIT_FAILURE);
  }

  /* we should have at least the input filename */
  if (p->ifp == NULL) {
    fprintf(stderr, "(main) at least the input filename must be supplied.\n");
    exit(EXIT_FAILURE);
  }
  
  /* I can't do 2 things at once */
  if ((p->dump_digis || p->dump_uniform_digis) && p->dump_rings){
    fprintf(stderr, "(main) I can't dump rings and digis things at once.\n");
    exit(EXIT_FAILURE);
  }

  /* I can't do 2 things at once */
  if (p->dump_uniform_digis && p->dump_digis){
    fprintf(stderr, "(main)WARN: I'll dump uniform RoI digis only.\n");
    p->dump_digis = FALSE;
  }
}

void print_help_msg(FILE* fp, const char* prog)
{
  fprintf(fp, "Calorimeter ASCII Data Preprocessor version 0.2\n");
  fprintf(fp, "author: André Rabello dos Anjos <Andre.dos.Anjos@cern.ch>\n");
  fprintf(fp, "usage: %s -f file [-a] [-o file] [-v] [-e # -r #]\n", prog);
  fprintf(fp, "       %s -h or --help prints this help message.\n", prog);
  fprintf(fp, "[OPTIONS SUMMARY]\n");
  fprintf(fp, "-f file | --input-file file\n\t sets the input file name\n");
  fprintf(fp, "-a | --dump-rings\n\t dumps rings instead of plain RoIs\n");
  fprintf(fp, "-d | --dump-digis\n\t dumps raw digis instead of plain RoIs\n");
  fprintf(fp, "-g | --dump-uniform-digis\n\t ");
  fprintf(fp, "dumps raw digis instead of plain RoIs such digis\n");
  fprintf(fp, "\t belong to events that can be uniformized\n");
  fprintf(fp, "-v | --verbose\n\t prints more output than be default\n");
  fprintf(fp, "-o file | --output-file file\n\t sets the output file name\n");
  fprintf(fp, "-e # | --event-number #\n\t only preprocess event #\n");
  fprintf(fp, "-r # | --roi-number #\n\t only preprocess roi #\n");
}

/* This function prints the progress of execution if the v flag is on. The
   first integer is an event counter while the second represent the rejection
   errors. */
void print_progress(FILE* fp, const bool_t v, const int c, const int e)
{
  int i; /* iterator for backspaces */

  if (!v) return;

  /* Prints the initial table information */
  if(!c) {
    fprintf(fp," Progress | Rejected | Dumped\n");
    fprintf(fp,"----------+----------+-------\n");
    fprintf(fp,"  %5ld   | %6d   | %5d", c, e, c-e);
    return;
  }

  /* progress report */
  for(i=0; i<28; ++i) fprintf(stderr,"\b");
  fprintf(fp,"  %5ld   | %6d   | %5d", c, e, c-e);
  fflush(stderr);

  return;
}

bool_t process_ROI(const ROI* roi, const parameter_t* p)
{
  tt_roi_t ttroi; /* The calorimeter RoI */

  if(roi->calDigi.nEmDigi == 0 && roi->calDigi.nhadDigi == 0) {
    fprintf(stderr,"\n");
    fprintf(stderr,"(main)ERROR: ROI has no digi info. ");
    return (FALSE);
  }
   
  if (p->dump_digis) {
    dump_DIGIS(p->ofp,roi);
  }

  else { /* Then I have to put all into TT format */
    if ( !build_roi(roi, FALSE, &ttroi) ) {
      fprintf(stderr,"\n");
      fprintf(stderr,"(main)ERROR: ROI can't go in tt format. ");
      return (FALSE);
    }

    if (p->dump_rings) { /* dump preprocessing into ring format */
      dump_ring(p->ofp, &ttroi);
    }

    else { /* dump preprocessing into raw format */
      uniform_roi_t ur;
      if ( uniformize (&ttroi,&ur) != NULL ) {

	if (p->dump_uniform_digis) dump_DIGIS(p->ofp,roi);
	else print_uniform_roi (p->ofp,&ur); 
	free_uniform_roi (&ur);
      }
    }

    /* free all resources */
    free_roi(&ttroi);
  }

  return (TRUE);
}

void process_EVENT (const EVENT* ev, const parameter_t* p)
{
  int i; /* iterator */

  if (!p->process_all_events) {
    if(p->process_all_rois) { /* I count the number of rois and exit */
      fprintf(stderr, "(main) RoIs in event %d ", p->event_no);
      fprintf(stderr, "-> %d\n", ev->nroi);
      return;
    }

    if(p->roi_no > ev->nroi) { /* I count the number of rois and exit */
      fprintf(stderr, "(main) RoIs in event %d ", p->event_no);
      fprintf(stderr, "-> %d\n", ev->nroi);
      fprintf(stderr, "(main)ERROR: You've asked too much!\n");
      return;
    }
    
    if (!process_ROI(&ev->roi[p->roi_no-1], p)) { /*I try to process the ROI*/
      fprintf(stderr, "(ROI -> %d)\n",i);
      exit(EXIT_FAILURE);
    }

  }

  else {
    for (i=0; i<ev->nroi; ++i)
      if (!process_ROI(&ev->roi[i], p)) {
	fprintf(stderr, "(ROI -> %d\n)",i);
	exit(EXIT_FAILURE);
      }
  }
  
  return; /* normal exit */
}





