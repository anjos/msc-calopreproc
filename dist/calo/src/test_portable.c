/* Hello emacs, this is -*- c -*- */
/* Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: test_portable.c,v 1.2 2000/05/31 13:45:08 rabello Exp $ */

#include "test_portable.h"

#include <stdlib.h>
#include <unistd.h>

#define _GNU_SOURCE
#include <getopt.h>

#include "common.h" /* some definitions at libcalo.a */

#include "util.h"

int getopt(int argc, char* const argv[], const char* optstring);
extern char* optarg;
extern int optind, opterr, optopt;

/* This one is the maximum difference between doubles */
static double MAXERR = 0.003; /* aprox. DETA */

int main (int argc, char* argv[])
{
  FILE* infile = NULL;
  FILE* outfile = NULL; 

  char* name = NULL;

  long int number = -1;
  long int roi = -1;
  long int number_of_rois = 0;
  long int j;

  char c;

  EVENT event;

  event_statistics_t* event_stats = 
    (event_statistics_t*) malloc ( sizeof (event_statistics_t) );

  /* initialize the events statistics */
  event_stats = init_event_statistics(event_stats);

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
      fprintf(stderr, "(main) syntax for %s is\n", argv[0]);
      fprintf(stderr, "(main) %s -f <input file> -e <event>", argv[0]);
      fprintf(stderr, " -r <roi> -o <output file>\n");
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
    fclose(infile);
    exit(EXIT_SUCCESS);
  }

  /* I have to read the event any how from this point on */

  /* wastes the initial VERSION information */
  waste_VERSION(infile);
  /* scan for the event I need to dump */
  event = search_event(infile, number);

   /* I do not have an RoI selected. Get number of RoIs in event, print and
      exit if the number of RoIs is zero. If the number of RoI's is greater
      than zero, tests all of them. */
  if (roi < 0) {
    
    /* now print to the output the number of RoIs in this event */
    fprintf(stderr, "(main) The number of RoIs in event %ld is: ", number);
    fprintf(stderr, "%d.\n", event.nroi);

    if (event.nroi == 0) {
      free_EVENT(&event);
      fclose(infile);
      exit(EXIT_SUCCESS);
    }

    else {
      fprintf(stderr, "(main) Testing all RoIs...\n");
      roi = 0;
    }

  } /* end if (roi < 0) */

  /* if I did not select an output file, dump to stdout */
  if (outfile == NULL) /* output to stdout */ {
    outfile = stdout;
    fprintf(outfile, "(main) Output is stdout (terminal).\n");
  }

  /* Shows MAXERR */
  fprintf(outfile, "(main) The maximmum acceptable error for doubles ");
  fprintf(outfile, "is %f.\n", MAXERR);

  /*************************************
   * Let's get to the calorimeter part *
   *************************************/
  if (roi == 0) {
    number_of_rois = event.nroi; /* I will analyze all RoIs */
    roi = event.nroi;
  }

  else number_of_rois = 1; /* I will analyze only a specific RoI */

  for(j=0;j<number_of_rois;j++) {

    fprintf(outfile, "(main) Examining RoI %ld.\n",j);
    fprintf(outfile, "--------------------------------------------------\n");

    /* I should check roi-1 and decrement roi. Instead of doing both I
       moncentrated these 2 actions in --roi bellow */
    event_stats->roi_stats = check_roi( &(event.roi[--roi]), outfile);

  } /* end of event analysis */

  /* free the event as always */
  free_EVENT(&event);
  fclose(infile);

  /* exit gracefully */
  return (EXIT_SUCCESS);
  
} /* end main function */

bool_t check_int (FILE* of, const int my, const int current, const char* what)
{
  if(my != current) {
    fprintf(of,"([%s] = %d) %d\n", what, current, my);
    return FALSE;
  }
  
  return TRUE;
}

bool_t check_float (FILE* of, const float my, const float current, 
		    const char* what)
{
  if(fcomp(my,current,MAXERR)== CALO_ERROR) {
    fprintf(of,"([%s] = %e) %e\n", what, current, my);
    return FALSE;
  }

  return TRUE;
}

bool_t check_double (FILE* of, const double my, const double current, 
		    const char* what)
{
  if(fcomp(my,current,MAXERR)== CALO_ERROR) {
    fprintf(of,"([%s] = %e) %e\n", what, current, my);
    return FALSE;
  }

  return TRUE;
}

bool_t check_em_digi (roi_statistics_t* stat, const CellInfo* cell, 
		      const emCalDigiType* digi, FILE* of)
{
  /* This is a cell error bool_t. In case of error it becomes TRUE */
  bool_t has_error = FALSE;

  int caloregion = cell->calo * 100 + cell ->region;

  /* use check_int in order to check the values for Calorimeter and
     Region. Those are given by their sum in the CaloRegion member of 
     the current "cell". */
  if ( ! check_int(of, caloregion, digi->CaloRegion, "CaloRegion") ) {
    has_error = TRUE;
    ++(stat->caloregion_errors);
  }
  
  /* use check_float for checking on the values for phi */
  if ( ! check_double(of, cell->center.Phi, digi->phi, "PHI") ) {
    has_error = TRUE;
    ++(stat->phi_errors);
  }
  
  /* use check_float for checking on the values for eta */
  if ( ! check_double(of, cell->center.Eta, digi->eta, "ETA") ) {
    has_error = TRUE;
    ++(stat->eta_errors);
  }

  /* check if the digi has any errors, if yes, count 1 wrong digi */
  if (has_error) {
    ++(stat->ncells_with_errors);
    return TRUE;
  }

  return FALSE;

} /* end of check_em_digi() */

roi_statistics_t* init_roi_statistics (roi_statistics_t* stat)
{
  stat->ncells = 0;
  stat->ncells_with_errors = 0;
  stat->caloregion_errors = 0;
  stat->phi_errors = 0;
  stat->eta_errors = 0;
  return stat;
}

event_statistics_t* init_event_statistics (event_statistics_t* stat)
{
  stat->nevents = 0;
  stat->nevents_with_errors = 0;
  stat->roi_stats = (roi_statistics_t*) malloc ( sizeof (roi_statistics_t) );
  stat->roi_stats = init_roi_statistics ( stat->roi_stats );
  return stat;
}

roi_statistics_t* check_roi (const ROI* theROI, FILE* of) 
{

  roi_statistics_t* stats =
    (roi_statistics_t*) malloc ( sizeof(roi_statistics_t) );

  stats = init_roi_statistics(stats);

  /* Just say we're about to start EM part */
  fprintf(of, "%s", "\n << EM PART >>\n");

  stats = check_em_part(theROI->calDigi.emDigi, 
			theROI->calDigi.nEmDigi, stats, of);

  /* Just say we're about to start HAD part */
  fprintf(of, "%s", "\n << HAD PART >>\n");

  /* Now we take advantage of the fact that hadCalDigiType = emCalDigiType, and
     thus we can monvert one into the other */
  stats = check_em_part( (emCalDigiType*)theROI->calDigi.hadDigi,
			 theROI->calDigi.nhadDigi, stats, of);

  if ( stats->ncells_with_errors != 0 ) {
    float percentage;

    /* monverts the integer code into a human readable form */
    
    fprintf(of, "------------------------------------------------------\n");
    fprintf(of, "I've found errors on this RoI, the statistics: \n");
    fprintf(of, "%d  (100.0%%) EM Cells were decodified\n", stats->ncells);
    percentage = ( (stats->ncells_with_errors) / (stats->ncells) ) * 100;
    fprintf(of, "%d  (%3.2f%%) are NOT well coded\n", 
	    stats->ncells_with_errors, percentage);
    fprintf(of, " +++ Those errors can be quantified as: \n");
    percentage = ( stats->caloregion_errors / stats->ncells_with_errors ) *
      100; 
    fprintf(of, "%d  (%3.2f%%) are CaloRegion (de)codifications errors\n",
	    stats->caloregion_errors, percentage);
    percentage = ( stats->eta_errors / stats->ncells_with_errors ) * 100;
    fprintf(of, "%d  (%3.2f%%) are eta (de)codification errors\n",
	    stats->eta_errors, percentage);
    percentage = ( stats->phi_errors / stats->ncells_with_errors ) * 100;
    fprintf(of, "%d  (%3.2f%%) are phi (de)codification errors\n",
	    stats->phi_errors, percentage);
    fprintf(of, "------------------------------------------------------\n");
  }

  return stats;
}

roi_statistics_t* check_em_part (const emCalDigiType* digi, const long n_digi,
				 roi_statistics_t* global_stats, FILE* of) 
{  

  CellInfo cell;
  char ucn[SIZE_OF_UCN_STRING];
  long i; /* an iterator */

  roi_statistics_t* stats =
    (roi_statistics_t*) malloc ( sizeof(roi_statistics_t) );

  stats = init_roi_statistics(stats);
  stats->ncells = n_digi;

  for(i=0; i < n_digi; ++i) {
    /* 1) Decode normally the digi */
    DecodeId(digi[i].id, &cell);

    /* 2) Checks whether the digi is corrupted or not (do not print UCN) */
    /* check_em_digi (stats, &cell, &(digi[i]), of); */

    /* If you want to output the UCN, comment out the previous line and
       uncomment the next lines. Else, mon't forget to uncomment the variable
       declaration above. */

    if ( check_em_digi (stats, &cell, &(digi[i]), of) ) {
      i2ucn(digi[i].id, ucn);
      fprintf(of, "UCN for digi %ld is %s (%d)\n", i, ucn, digi[i].id);
      fprintf(of, "[ETA = %e] [PHI = %e] [CR = %d]\n",cell.center.Eta,
	      cell.center.Phi, 100*cell.calo + cell.region);
      fprintf(of, "-------------------------------------------------------\n");
    }


  } /* end loop over all digis of RoI */

  /* now, I update the stats global structure */
  global_stats->ncells += stats->ncells;
  global_stats->ncells_with_errors += stats->ncells_with_errors;
  global_stats->caloregion_errors += stats->caloregion_errors;
  global_stats->eta_errors += stats->eta_errors;
  global_stats->phi_errors += stats->phi_errors;

  free(stats); /* free the montents of this local statitics structure */

  return global_stats; /* return the updated structure */

} /* end of check_EM_part() */

