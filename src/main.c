/* Hello emacs, this is -*- c -*- */

/* This package doesn't belong to any other package I've known. It just takes
   ASCII files produced at CERN and provides a better interface to them. This
   way they can be plotted or so. It uses only the CaloDigi's that may be
   present in the file. The building of this file is accomplished by make (1).
*/ 

/* $Id: main.c,v 1.6 2000/07/20 00:42:51 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <obstack.h>
#include <string.h>

/* Functions that shall be initialized */
FILE * open_obstack_stream (struct obstack*);

/* The definition of obstack initialization and destruction */
#define obstack_chunk_alloc malloc 
#define obstack_chunk_free free

#include <getopt.h>

#include "util.h"
#include "data.h" /* the data file description */
#include "trigtowr.h" /* for processing the data into trigger towers */
#include "ring.h"
#include "uniform.h"

#ifdef TRACE_DEBUG
#include <mcheck.h>
#endif

typedef enum particle_t {ELECTRON = 0, JET = 1} particle_t;

/* The following type define the parameters for program execution */
typedef struct pararamter_t 
{
  FILE* ifp; /* The input file pointer */
  char* ifbuf; /* The input file buffer */
  FILE* ofp; /* The output file pointer */
  char* ofbuf; /* The output file buffer */
  char ofname[256]; /* The output file name */

  particle_t particle; /* The type of particle present on file */
  unsigned short layer_flags; /* Which layers to process. See module uniform */
  unsigned short normalization; /* Normalization scheme. See module uniform */

  long int event_no; /* The event number to process */
  long int roi_no; /* The roi number to process */

  bool_t dump_rings; /* Do I have to dump rings? */
  bool_t dump_digis; /* Do I have to dump digis? */
  bool_t dump_uniform_digis; /* Do I have to dump digis from uniform RoIs? */
  bool_t format_snns; /* Dumps data using SNNS formatting */
  
  bool_t process_all_events; /* Do I have to process all events? */
  bool_t process_all_rois; /* Do I have to process all rois for each event? */
  bool_t verbose; /* Do I have to process all rois for each event? */
} parameter_t;

void process_flags (parameter_t*, const int, char**);
void test_flags (parameter_t*);
void print_help_msg (FILE*, const char*);
void fprintf_progress (FILE*, const bool_t, const int, const int);
bool_t process_ROI (const ROI*, const parameter_t*);
void process_EVENT (const EVENT*, const parameter_t*);
int dump_rings (const tt_roi_t*, const parameter_t*);

static int _iunits = 0; /* This shall, during execution, be set to the
			   number of dumped features */
static int _nevents = 0; /* This shall be set to the number of events dumped to
			    file */

int main (int argc, char* argv[])
{
  parameter_t params;

  /* I want to capture the errors produced by uniformization. */
  extern int uniform_contour_err;
  EVENT event;

#ifdef TRACE_DEBUG
  mtrace();
#endif
  
  /* define the default parameters */
  params.ifp = 0;
  params.ifbuf = 0;
  params.ofp = stdout; /* by default, dump to screen */
  params.ofbuf = 0;
  params.particle = ELECTRON; /* by default, use electron */
  params.event_no = 0;
  params.roi_no = 0;
  params.dump_rings = FALSE; /* rings are a specific case */
  params.dump_digis = FALSE; /* digis are a specific case */
  params.dump_uniform_digis = FALSE; /* uniform_digis are a specific case */
  params.format_snns = FALSE; /* SNNS formating for output */
  params.process_all_events = TRUE; /* process all by default */
  params.process_all_rois = TRUE; /* processs all rois */
  params.verbose = FALSE; /* verbose output? */

  /* Enable all layers by default */
  string2layer(&params.layer_flags, "all");
  /* Don't normalize by default */
  string2normalization(&params.normalization, "none");
  
  /* get and treat parameters */
  process_flags(&params,argc,argv);

  if (!params.process_all_events) {
    event = search_event(params.ifp, params.event_no);
    process_EVENT(&event, &params);
    free_EVENT(&event);
  }
  
  else {
    long int counter = 0;

    waste_initial_info(params.ifp);
    fprintf_progress(stderr, params.verbose, counter, uniform_contour_err);

    while (read_EVENT(params.ifp,&event) == ERR_SUCCESS) {
      ++counter;
      process_EVENT(&event, &params);
      free_EVENT(&event);
      fprintf_progress(stderr, params.verbose, counter, uniform_contour_err); 
    } /* event loop */

    if (params.verbose) fprintf(stderr,"\n"); /* just to make sure we don't
						 start over in the middle of
						 the screen */

    /* The final printout, independent of verbosity */
    fprintf(stderr, "(main) I've dumped %ld events\n", counter - 
	    uniform_contour_err);

  } /* else process all events */
  
  /* close streams and free pointers */
  fclose(params.ifp);
  free(params.ifbuf);
  fclose(params.ofp);
  free(params.ofbuf);

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
    {"input-file", 1, 0, 'i'},
    {"output-file", 1, 0, 'o'},
    {"help", 0, 0, 'h'},
    {"event-number", 1, 0, 'e'},
    {"roi-number", 1, 0, 'r'},
    {"dump", 1, 0, 'd'},
    {"format", 1, 0, 'f'},
    {"particle", 1, 0, 'p'},
    {"layer", 1, 0, 'l'},
    {"normalization", 1, 0, 'n'},
    {"verbose", 0, 0, 'v'},
    {0, 0, 0, 0}
  };
  
  while (EOF != (c=getopt_long(argc, argv, "i:o:he:r:d:f:p:l:n:v", 
			       long_options, &option_index) ) ) {
    switch (c) {

    case 0: /* Got a flagged option */
      break;
      
    case 'i': { /* input from file 'infile' */
      struct stat buf;
      /* Ok, open the file we're going to work with, in case of error,
	 give up */
      if (NULL==(p->ifp=fopen(optarg,"r"))) {
	fprintf(stderr, "(main) Can't open input file %s\n", optarg);
	exit(EXIT_FAILURE);
      }

      /* get file status, includding optimal buffer block size */
      stat(optarg, &buf);
      /* set the output file buffer to be optimal, or we'll take longer to 
	 process the output */
      p->ifbuf = (char*)malloc(buf.st_blksize);
      if (setvbuf(p->ifp, p->ifbuf, _IOFBF, buf.st_blksize) != 0) {
	fprintf(stderr, "(main) Can't create input file buffer\n");
	exit(EXIT_FAILURE);
      }

      break;
    }

    case 'd': /* What to dump? */
      if ( strcasecmp(optarg, "digis" ) == 0 ) {
	p->dump_digis = TRUE;
	break;
      }

      else if ( strcasecmp(optarg, "rings" ) == 0 ) {
	p->dump_rings = TRUE;
	break;
      }

      else if ( strcasecmp(optarg, "udigis" ) == 0 ) {
	p->dump_uniform_digis = TRUE;
	break;
      }

      else if ( strcasecmp(optarg, "rois" ) == 0 )
	break;

      else {
	fprintf(stderr,"(main)ERROR: Can't recognize format -> %s\n", optarg);
	exit(EXIT_FAILURE);
      }

    case 'f': /* What to dump? */
      if ( strcasecmp(optarg, "snns" ) == 0 ) {
	p->format_snns = TRUE;
	break;
      }

      else if ( strcasecmp(optarg, "raw" ) == 0 ) {
	break;
      }

      else {
	fprintf(stderr,"(main)ERROR: Can't recognize format -> %s\n", optarg);
	exit(EXIT_FAILURE);
      }


    case 'p': /* What's the target? */
      if ( strcasecmp(optarg, "jet" ) == 0 ) {
	p->particle = JET;
	break;
      }

      else if ( strcasecmp(optarg, "electron" ) == 0 ) {
	p->particle = ELECTRON;
	break;
      }

      else {
	fprintf(stderr,"(main)ERROR: A type of particle ?? -> %s\n", optarg);
	exit(EXIT_FAILURE);
      }

    case 'l': /* Which layers to use? */
      string2layer(&p->layer_flags,optarg);
      break;

    case 'n': /* Which normalization to use? */
      string2normalization(&p->normalization,optarg);
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

    case 'o': { /* output to file 'outfile' */
      struct stat buf;
      if (NULL==(p->ofp=fopen(optarg,"w"))) {
	fprintf(stderr, "(main) Can\'t open output file %s\n", optarg);
	exit(EXIT_FAILURE);
      }
      
      /* If opened the output filename, copy the filename to a safe place, so
	 it can be re-used latter */
      strcpy(p->ofname, optarg);

      /* get file status, includding optimal buffer block size */
      stat(optarg, &buf);
      /* set the output file buffer to be optimal, or we'll take longer to 
	 process the output */
      p->ofbuf = (char*)malloc(buf.st_blksize);
      if (setvbuf(p->ofp, p->ofbuf, _IOFBF, buf.st_blksize) != 0) {
	fprintf(stderr, "(main) Can't create output file buffer\n");
	exit(EXIT_FAILURE);
      }
      
      break;
    }

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
    fprintf(stderr, "(main) I can't dump rings *AND* digis at once.\n");
    exit(EXIT_FAILURE);
  }

  /* I can't do 2 things at once */
  if (p->dump_uniform_digis && p->dump_digis){
    fprintf(stderr, "(main)WARN: I'll dump uniform RoI digis only.\n");
    p->dump_digis = FALSE;
  }

  /* No sense in SNNS formatting and digi output */
  if ( (! p->dump_rings) && p->format_snns) {
    fprintf(stderr, "(main)WARN: No sense in formatting (SNNS) for ");
    fprintf(stderr, "something that is *NOT* a ring.\n");
    p->format_snns = FALSE;
    fprintf(stderr, "(main)WARN: SNNS formatting DEACTIVATED!\n");
  }

}

void print_help_msg(FILE* fp, const char* prog)
{
  fprintf(fp, "Calorimeter ASCII Data Preprocessor version 0.2\n");
  fprintf(fp, "author: André Rabello dos Anjos <Andre.dos.Anjos@cern.ch>\n");
  fprintf(fp, "usage: %s -i file [-d what] [-f format] [-v]", prog);
  fprintf(fp, "[-o file] [-e # -r #]\n");
  fprintf(fp, "       %s -h or --help prints this help message.\n", prog);
  fprintf(fp, "[OPTIONS SUMMARY]\n");

  fprintf(fp, "-i file | --input-file=file\n");
  fprintf(fp, "\t sets the input file name\n");

  fprintf(fp, "-v | --verbose\n");
  fprintf(fp, "\t prints more output than be default\n");

  fprintf(fp, "-o file | --output-file=file\n");
  fprintf(fp, "\t sets the output file name (default is stdout)\n");
  fprintf(fp, "\t 'file' will be created if doesn't exist or truncated if");
  fprintf(fp, " it does\n");

  fprintf(fp, "-e # | --event-number=#\n");
  fprintf(fp, "\t only preprocess event #\n");

  fprintf(fp, "-r # | --roi-number=#\n");
  fprintf(fp, "\t only preprocess roi #\n\n");

  fprintf(fp, "-d string | --dump=string\n");
  fprintf(fp, "\t dumps the pattern specified by 'string'. It can be:\n");
  fprintf(fp, "\t   digis  - dump the digis of all RoIs in file\n");
  fprintf(fp, "\t   udigis - dump the digis of RoIs that can be");
  fprintf(fp, " uniformized\n");
  fprintf(fp, "\t   rois   - dump the cells of uniformizable RoIs");
  fprintf(fp, " (default)\n");
  fprintf(fp, "\t   rings  - dump rings around energy peaks for each\n");
  fprintf(fp, "\t            layer on all uniformizable RoIs\n");

  fprintf(fp, "-f string | --format=string\n");
  fprintf(fp, "\t dumps using the format specified by 'string'. It can be:\n");
  fprintf(fp, "\t   raw  - only dump the numbers\n");
  fprintf(fp, "\t   snns - dump the rings for use with SNNS\n");

  fprintf(fp, "-p string | --particle=string\n");
  fprintf(fp, "\t The 'string' will identify the type of particle in file \n");
  fprintf(fp, "\t it might be usefull when dumping SNNS target and others:\n");
  fprintf(fp, "\t   electron - electrons file (target = +1)\n");
  fprintf(fp, "\t   jets     - jets file (target = -1)\n");

  fprintf(fp, "-l string | --layer=string\n");
  fprintf(fp, "\t dumps the layers specified by string. The layers shall\n");
  fprintf(fp, "\t separated by comman and/or spaces and may be one of\n");
  fprintf(fp, "\t the following:\n");
  fprintf(fp, "\t   ps   - dump PreSample information\n");
  fprintf(fp, "\t   em1  - dump EM front layer\n");
  fprintf(fp, "\t   em2  - dump EM middle layer\n");
  fprintf(fp, "\t   em3  - dump EM back layer\n");
  fprintf(fp, "\t   had1 - dump HAD front layer\n");
  fprintf(fp, "\t   had2 - dump HAD middle layer\n");
  fprintf(fp, "\t   had3 - dump HAD back layer\n");
  fprintf(fp, "\t   all  - dump all layers. If such flag is activated, the\n");
  fprintf(fp, "\t          others will be ignored since are redundant\n");
  fprintf(fp, "\t          this is the default behaviour \n");

  fprintf(fp, "-n string | --normalization=string\n");
  fprintf(fp, "\t the dumped data will be normalized according to the\n");
  fprintf(fp, "\t algorithm selected. Do select only one algorithm:\n");
  fprintf(fp, "\t   all     - normalize using sum of energy of all cells\n");
  fprintf(fp, "\t   section - normalize using section energy (EM|HAD)\n");
  fprintf(fp, "\t   layer   - normalize using layer energy\n");
  fprintf(fp, "\t   none    - no normalization at all (default)\n");

}

/* This function prints the progress of execution if the v flag is on. The
   first integer is an event counter while the second represent the rejection
   errors. */
void fprintf_progress(FILE* fp, const bool_t v, const int c, const int e)
{
  int i; /* iterator for backspaces */

  if (!v) return;

  /* Prints the initial table information */
  if(!c) {
    fprintf(fp," Progress | Rejected | Dumped\n");
    fprintf(fp,"----------+----------+-------\n");
    fprintf(fp,"  %5d   | %6d   | %5d", c, e, c-e);
    return;
  }

  /* progress report */
  for(i=0; i<28; ++i) fprintf(stderr,"\b");
  fprintf(fp,"  %5d   | %6d   | %5d", c, e, c-e);
  fflush(stderr);

  return;
}

/* Process an RoI, checking what to dump. The returned value is the number of
   dumped fields to file, or zero */
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

    if (p->dump_rings) dump_rings(&ttroi, p);

    else { /* dump preprocessing into raw format */
      uniform_roi_t ur;
      if ( uniformize (&ttroi,&ur,p->layer_flags,p->normalization) != NULL ) {

	if (p->dump_uniform_digis) dump_DIGIS(p->ofp,roi);
	else print_uniform_roi (p->ofp,&ur);
	free_uniform_roi (&ur);
      }
    }

    /* free all resources */
    free_roi(&ttroi);
  }

  /* well if I didn't return anything, I can return 1 */
  return (TRUE);
}

/* Do all event checkings and process the whole event, return the number of
   fields processed per RoI. If the number of fields per RoI is variable, it
   should exit with an error message. */
void process_EVENT (const EVENT* ev, const parameter_t* p)
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
      fprintf(stderr, "(ROI -> %d)\n",i);
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
int dump_rings (const tt_roi_t* roi, const parameter_t* p)
{
  char* dump;
  char* temp;
  ring_t* rp;
  int nring; /* the number of rings produced */

  /* I have to change the value pointed by rp */
  nring = ring_sum (roi, &rp, p->layer_flags, p->normalization);

  if (nring > 0) {
      ++_nevents;

      _iunits = asprintf_ring_vector (&dump, rp, nring);

      if (p->format_snns) {
	temp = dump;
	asprintf(&dump, "## INPUT roi=%d features=%d\n%s", 
		 _nevents, _iunits, dump);
	free(temp);

	temp = dump;
	asprintf(&dump, "%s## TARGET roi=%d ", dump, _nevents);
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
      if (_nevents == 1) fprintf_SNNS_header(p->ofp, 1, _iunits, 1);

      fprintf(p->ofp,"%s",dump);

      free_ring_vector(rp, nring);
      free(dump); /* the information to print */
  }
  return (_nevents);
}


