/* Hello emacs, this is -*- c -*- */

/* This routine just takes ASCII files produced at CERN and provides a better
   interface to them. This way they can be plotted or so. It uses only the
   CaloDigi's that may be present in the file. The building of this file is
   accomplished by make (1).  */

/* $Id: main.c,v 1.17 2000/09/01 01:34:16 andre Exp $ */

#include <stdlib.h>
#include <stdio.h>

#include <obstack.h>

#include <string.h>

#include "util.h"
#include "parameter.h" /* the default input parameters, sizes and so on */

#include "data.h" /* the data file description */
#include "trigtowr.h" /* for processing the data into trigger towers */
#include "ring.h"
#include "uniform.h"
#include "energy.h"

#ifdef TRACE_DEBUG
#include <mcheck.h>
#endif

void fprintf_progress (FILE*, const bool_t);
bool_t process_ROI (const ROI*, parameter_t*);
void process_EVENT (const EVENT*, parameter_t*);
int dump_rings (const uniform_roi_t*, parameter_t*);

/* This shall, during execution, be set to the number of dumped features */
static int _iunits = 0; 

/* This shall be set to the number of events dumped to file */
static int dumped_events = 0; 

/* This shall keep the number of events processed so far */
static int event_counter = 0;

int main (int argc, char* argv[])
{
  /* parameters to the main routine */
  parameter_t params;

  EVENT event;

#ifdef TRACE_DEBUG
  mtrace();
#endif

  /* sets up the default parameters */
  init_parameters(&params);

  /* get and treat parameters */
  process_flags(&params,argc,argv);

  if (!params.process_all_events) {
    event = search_event(params.ifp, params.event_no);
    event_counter = params.event_no; /* Update the event counter */
    process_EVENT(&event, &params);
    free_EVENT(&event);
  }

  else {
    waste_initial_info(params.ifp);
    fprintf_progress(stderr, params.verbose);

    while (read_EVENT(params.ifp,&event) == ERR_SUCCESS) {
      ++event_counter;
      process_EVENT(&event, &params);
      free_EVENT(&event);
      fprintf_progress(stderr, params.verbose); 
    } /* event loop */

    /* just to make sure we don't start over in the middle of the screen */
    if (params.verbose) fprintf(stderr,"\n");

    /* The final printout, independent of verbosity */
    fprintf(stderr, "(main) I've dumped %d events\n", dumped_events);

  } /* else process all events */

  /* This will close unclosed files and free all memory needed so far for
     saving the run parameters. It will also dump, if needed, the memory banks
     containing event data. */
  terminate_parameters(&params);

  /* exit gracefully */
  return (EXIT_SUCCESS);

} /* end main function */



/* This function prints the progress of execution if the v flag is on. The
   numbers printed come from global variables that control the number of events
   processed automatically. */
void fprintf_progress(FILE* fp, const bool_t v)
{
  int i; /* iterator for backspaces */

  /* Well, if we choose not to be verbose I can just get out of here...*/
  if(!v) return;

  /* Prints the initial table information */
  if(!event_counter) {
    fprintf(fp," Progress | Rejected | Dumped\n");
    fprintf(fp,"----------+----------+-------\n");
    fprintf(fp,"  %5d   | %6d   | %5d", event_counter, event_counter -
	    dumped_events, dumped_events);
    return;
  }

  /* progress report */
  for(i=0; i<28; ++i) fprintf(stderr,"\b");
  fprintf(fp,"  %5d   | %6d   | %5d", event_counter, event_counter -
	  dumped_events, dumped_events);
  fflush(stderr);

  return;
}

/* Process an RoI, checking what to dump. The returned value is a bool
   indicating the RoI was correctly processed or not. Event accouting was also
   moved into here since this place is perfect for doing it: event rejection
   can only happen here. */
bool_t process_ROI(const ROI* roi, parameter_t* p)
{
  tt_roi_t ttroi; /* The calorimeter RoI */
  char* info;

  if(roi->calDigi.nEmDigi == 0 && roi->calDigi.nhadDigi == 0) {
    fprintf(stderr,"\n");
    fprintf(stderr,"(main)ERROR: ROI has no digi info. ");
    return (FALSE);
  }

  ++dumped_events; /* Can we count one more valid event? */

  /* do I have to dump event numbers? */
  if(p->dump_eventno) {
    /* create the string containing the event number data */
    asprintf(&info, "%s %d %d\n", p->event_comment_str, event_counter, 
	     dumped_events);
    
    /* put string to final destination */
    output_string(p->evfp, p->eventno_obsp, p->run_fast, info);
    
    /* clear the output string */
    free(info);
  }

  if (p->dump_digis) {
    /* get the digis into string format */
    info = get_DIGIS(roi);

    /* Put the string into memory pool or file */
    output_string(p->ofp, p->output_obsp, p->run_fast, info);
    
    /* Free the allocated space */
    free(info);
  }

  else { /* Then I have to put all into TT format */
    uniform_roi_t ur; /* will need this temporary to hold uniformized RoI */

    if ( !build_roi(roi, FALSE, &ttroi) ) {
      fprintf(stderr,"\n");
      fprintf(stderr,"(main)ERROR: ROI can't go in tt format. ");
      --dumped_events; /* Actually, I won't use this one */
      return (FALSE);
    }

    /* Uniformize, normalize and check for correction */
    if ( uniformize (&ttroi,&ur,p->layer_flags,p->normalization) != NULL ) {

      /* Do we have to dump energy information? */
      if ( p->dump_energy ) {
	/* get the energies for this event, in a string */
	info = get_energy(roi, &ur, p->dump_energy, p->edump_comment_str);

	/* Put the string into memory pool or file */
	output_string(p->efp, p->energy_obsp, p->run_fast, info);
    
	/* Free the allocated space */
	free(info);
      }

      if (p->dump_uniform_digis) {
	/* get the digis into string format */
	info = get_DIGIS(roi);

	/* Put the string into memory pool or file */
	output_string(p->ofp, p->output_obsp, p->run_fast, info);
    
	/* Free the allocated space */
	free(info);
      }

      else {
	/* Do we have to dump rings or uniform RoIs? */
	if (p->dump_rings) dump_rings(&ur, p);

	/* To print the uniform RoIs is what is left for us... */
	else {
	  info = get_uniform_roi (&ur,p->print_flags);
	  output_string(p->ofp, p->output_obsp, p->run_fast, info);
	  free(info);
	}
      }

      /* An extra space between outputs */
      {
	char space[] = "\n";
	output_string(p->ofp, p->output_obsp, p->run_fast, space);
      }

      /* free the required resources */
      free_uniform_roi (&ur);
      
    }

    else { /* I couldn't uniformize the RoI... */
      --dumped_events; /* Actually, I won't use this one */
    }

    /* free the required resources */
    free_roi(&ttroi);
  }

  return (TRUE);
}

/* Do all event checkings and process the whole event, return the number of
   fields processed per RoI. If the number of fields per RoI is variable, it
   should exit with an error message. */
void process_EVENT (const EVENT* ev, parameter_t* p)
{
  int i; /* iterator */

  if (!p->process_all_events) {
    if(p->process_all_rois) { /* I count the number of rois and exit */
      fprintf(stderr, "(main) RoIs in event %ld ", p->event_no);
      fprintf(stderr, "-> %d\n", ev->nroi);
      return;
    }

    if(p->roi_no > ev->nroi) { /* I count the number of rois and exit */
      fprintf(stderr, "(main) RoIs in event %ld ", p->event_no);
      fprintf(stderr, "-> %d\n", ev->nroi);
      fprintf(stderr, "(main)ERROR: You've asked too much!\n");
      return;
    }
    
    if (!process_ROI(&ev->roi[p->roi_no-1], p)) { /*I try to process the ROI*/
      fprintf(stderr, "(main)ERROR: In ROI -> %d\n",i);
      exit(EXIT_FAILURE);
    }

  }

  else
    /* I can't process events with nroi > 1 since they usually have problems. I
       choose to ignore events that contain more than 1 RoI. */
      for (i=0; i<1; ++i)
	if (!process_ROI(&ev->roi[i], p)) {
	  fprintf(stderr, "(ROI -> %d)\n",i);
	  exit(EXIT_FAILURE);
	}

  return; /* normal exit */
}

/* Dumps using the given parameters, the roi in form of rings. The value
   returned is the number of times dump_ring() was successfully called so far,
   meaning successful, if the RoI is uniformizable. This function will allocate
   more and more space as it needs to place the output information, at the end,
   it finally dumps everything to the output file.

   In order to increase efficiency, this function will use a static obstack to
   do it's job. The obstack is passed as an extra paramater and is used to dump
   the contents produced at each iteration. The obstack must be initialized
   previously. */
int dump_rings (const uniform_roi_t* ur, parameter_t* p)
{
  char* dump;
  char* temp;
  ringroi_t ringroi;

  /* I have to change the value pointed by rp */
  ring_sum(ur, &ringroi, p->print_flags);

  if (ringroi.nring > 0) {
      _iunits = asprintf_ring_vector (&dump, ringroi.ring, ringroi.nring);

      if (p->format_snns) {
	temp = dump;
	asprintf(&dump, "## INPUT roi=%d features=%d\n%s",
		 dumped_events, _iunits, dump);
	free(temp);

	temp = dump;
	asprintf(&dump, "%s## TARGET roi=%d ", dump, dumped_events);
	free(temp);

	temp = dump;
	asprintf(&dump, "%starget=%s\n", dump,
		 (p->particle==JET)?"JET":"ELECTRON");
	free(temp);

	temp = dump;
        asprintf(&dump, "%s%s\n", dump, (p->particle==JET)?"-1":"+1"); 
	free(temp);
      }

      /* If this is the first print out, include the header */
      if (dumped_events == 1 && p->format_snns) {
	char* t = get_SNNS_header(1, _iunits, 1);
	output_string(p->ofp, p->output_obsp, p->run_fast, t);
	free(t);
      }

      output_string(p->ofp, p->output_obsp, p->run_fast, dump);

      free_ring_vector(ringroi.ring, ringroi.nring);
      free(dump); /* the information to print */
  }
  return (dumped_events);
}
