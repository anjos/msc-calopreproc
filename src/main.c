/* Hello emacs, this is -*- c -*- */

/* This package doesn't belong to any other package I've known. It just takes
   ASCII files produced at CERN and provides a better interface to them. This
   way they can be plotted or so. It uses only the CaloDigi's that may be
   present in the file. The building of this file is accomplished by make (1).
*/ 

/* $Id: main.c,v 1.12 2000/08/18 03:10:38 andre Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <obstack.h>
#include <string.h>
#include <time.h>

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
#include "energy.h"

#ifdef TRACE_DEBUG
#include <mcheck.h>
#endif

/* The maximum input file name */
#define MAX_FILENAME 255

typedef enum particle_t {ELECTRON = 0, JET = 1} particle_t;

/* The following type define the parameters for program execution */
typedef struct pararamter_t 
{
  FILE* ifp; /* The input file pointer */
  char* ifbuf; /* The input file buffer */
  FILE* ofp; /* The output file pointer */
  char* ofbuf; /* The output file buffer */

  /* The output file name hint will be copied into here */
  char ofhint[MAX_FILENAME+1];

  /* The input file name will be copied into here */
  char ifname[MAX_FILENAME+1]; 

  FILE* cfp; /* The file where the config will be stored */
  FILE* efp; /* The file where the energy data will be stored if asked */

  particle_t particle; /* The type of particle present on file */
  unsigned short layer_flags; /* Which layers to process. See uniform.[ch] */
  unsigned short print_flags; /* Which layers to dump. See uniform.[ch] */
  unsigned short normalization; /* Normalization scheme. See module uniform */

  long int event_no; /* The event number to process */
  long int roi_no; /* The roi number to process */

  FILE* evfp; /* The file where the event numbers will be stored if asked */
  bool_t dump_eventno; /* Do I have to dump event numbers? */
  char event_comment_str[5]; /* The string that should contain the initial
				characters to print with the event number
				variables. It's usefull for printing comment
				like information for many script languages and
				processors like MatLab or PAW. */

  bool_t dump_rings; /* Do I have to dump rings? */
  bool_t dump_digis; /* Do I have to dump digis? */
  bool_t dump_uniform_digis; /* Do I have to dump digis from uniform RoIs? */
  bool_t format_snns; /* Dumps data using SNNS formatting */
  unsigned short dump_energy; /* Chooses to dump the energy variables or not */

  char edump_comment_str[5];/* The string that should contain the initial
			       characters to print with the energy
			       variables. It's usefull for printing comment
			       like information for many script languages and
			       processors like MatLab or PAW. */
  
  bool_t process_all_events; /* Do I have to process all events? */
  bool_t process_all_rois; /* Do I have to process all rois for each event? */
  bool_t verbose; /* Do I have to process all rois for each event? */
} parameter_t;

void process_flags (parameter_t*, const int, char**);
void test_flags (parameter_t*);
void print_help_msg (FILE*, const char*);
void fprintf_progress (FILE*, const bool_t);
bool_t process_ROI (const ROI*, const parameter_t*);
void process_EVENT (const EVENT*, const parameter_t*);
int dump_rings (const uniform_roi_t*, const parameter_t*);
void dump_config(FILE*, const parameter_t*);

static int _iunits = 0; /* This shall, during execution, be set to the
			   number of dumped features */
static int dumped_events = 0; /* This shall be set to the number of events
				 dumped to file */ 
static int event_counter = 0; /* This shall keep the number of events processed
				 so far */

int main (int argc, char* argv[])
{
  parameter_t params;

  EVENT event;

#ifdef TRACE_DEBUG
  mtrace();
#endif
  
  /* define the default parameters */
  params.ifp = 0;
  params.ifbuf = 0;
  params.ofp = stdout; /* by default, dump to screen */
  params.ofbuf = 0;
  params.cfp = stdout; /* by default dump configuration to screen */
  params.efp = stdout; /* the energy file will be stdout by default */
  params.evfp = stdout; /* the event number file will be stdout by default */
  params.particle = ELECTRON; /* by default, use electron */
  params.event_no = 0;
  params.roi_no = 0;
  params.dump_rings = FALSE; /* rings are a specific case */
  params.dump_digis = FALSE; /* digis are a specific case */
  params.dump_uniform_digis = FALSE; /* uniform_digis are a specific case */
  params.dump_eventno = FALSE; /* do not dump event numbers by default */
  params.format_snns = FALSE; /* SNNS formating for output */
  params.process_all_events = TRUE; /* process all by default */
  params.process_all_rois = TRUE; /* processs all rois */
  params.verbose = FALSE; /* verbose output? */
  strcpy(params.edump_comment_str, ""); /* by default, nothing is to be printed
					   before the energy parameters */

  strcpy(params.event_comment_str, ""); /* by default, nothing is to be printed
					   before the event numbers */

  /* The input file hint will be "default" */
  strncpy(params.ofhint, "default", MAX_FILENAME);

  /* Require all layers by default */
  string2layer(&params.layer_flags, "all");
  /* Dump all layers by default */
  string2layer(&params.print_flags, "all");
  /* Don't normalize by default */
  string2normalization(&params.normalization, "none");

  /* Print ALL energy values before event data */
  string2edump(&params.dump_energy, "all");
  
  /* get and treat parameters */
  process_flags(&params,argc,argv);

  if (!params.process_all_events) {
    event = search_event(params.ifp, params.event_no);
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

    if (params.verbose) fprintf(stderr,"\n"); /* just to make sure we don't
						 start over in the middle of
						 the screen */

    /* The final printout, independent of verbosity */
    fprintf(stderr, "(main) I've dumped %d events\n", dumped_events);
/*      fprintf(stderr, "(main) I've dumped %d events\n", event_counter -  */
/*  	    uniform_contour_err); */

  } /* else process all events */
  
  /* close the files we may open: event-number and energy */
  if ( (params.evfp != params.ofp) && (params.evfp != stdout) )
    fclose(params.evfp);
  if ( (params.efp != params.ofp) && (params.efp != stdout) )
    fclose(params.efp);

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
    {"energy-file", 0, 0, 'a'},
    {"dump-eventno", 0, 0, 'b'},
    {"config-file", 0, 0, 'c'},
    {"dump", 1, 0, 'd'},
    {"event-number", 1, 0, 'e'},
    {"format", 1, 0, 'f'},
    {"dump-energy", 1, 0, 'g'},
    {"help", 0, 0, 'h'},
    {"input-file", 1, 0, 'i'},
    {"eventno-file", 0, 0, 'j'},
    {"eventno-comment", 1, 0, 'k'},
    {"layer", 1, 0, 'l'},
    {"normalization", 1, 0, 'n'},
    {"output-file", 1, 0, 'o'},
    {"particle", 1, 0, 'p'},
    {"roi-number", 1, 0, 'r'},
    {"select", 1, 0, 's'},
    {"energy-comment", 1, 0, 't'},
    {"verbose", 0, 0, 'v'},
    {0, 0, 0, 0}
  };
  
  while (EOF != (c=getopt_long(argc, argv, "i:o:bcajk:g:he:r:d:f:p:l:s:n:vt:",
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

      /* If opened the input filename, copy the filename to a safe place, so it
	 can be re-used latter */
      strncpy(p->ifname,optarg, MAX_FILENAME);

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
      }

      else if ( strcasecmp(optarg, "rings" ) == 0 ) {
	p->dump_rings = TRUE;
      }

      else if ( strcasecmp(optarg, "udigis" ) == 0 ) {
	p->dump_uniform_digis = TRUE;
      }

      /* this is the default, do nothing */
      else if ( strcasecmp(optarg, "rois" ) == 0 ) {}

      else {
	fprintf(stderr,"(main)ERROR: Can't recognize format -> %s\n", optarg);
	exit(EXIT_FAILURE);
      }
      break;

    case 'f': /* What to dump? */
      if ( strcasecmp(optarg, "snns" ) == 0 ) {
	p->format_snns = TRUE;
      }

      /* this is the default, do nothing */
      else if ( strcasecmp(optarg, "raw" ) == 0 ) {}

      else {
	fprintf(stderr,"(main)ERROR: Can't recognize format -> %s\n", optarg);
	exit(EXIT_FAILURE);
      }
      break;

    case 'p': /* What's the target? */
      if ( strcasecmp(optarg, "jet" ) == 0 ) {
	p->particle = JET;
      }

      else if ( strcasecmp(optarg, "electron" ) == 0 ) {
	p->particle = ELECTRON;
      }

      else {
	fprintf(stderr,"(main)ERROR: A type of particle ?? -> %s\n", optarg);
	exit(EXIT_FAILURE);
      }
      break;

    case 'l': /* Which layers to require? */
      string2layer(&p->layer_flags,optarg);
      break;

    case 'g': /* What energy values would you like printed? */
      string2edump(&p->dump_energy,optarg);
      break;

    case 's': /* Which layers to print to output? */
      string2layer(&p->print_flags,optarg);
      break;

    case 'n': /* Which normalization to use? */
      string2normalization(&p->normalization,optarg);
      break;

    case 'v': /* dumps information on screen? v == verbose */
      p->verbose = TRUE;
      break;

    case 'b': /* dump event numbers? */
      p->dump_eventno = TRUE;
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
      char* temp;
      strncpy(p->ofhint, optarg, MAX_FILENAME);
      fprintf(stderr,"(main)WARN: will use hint -> %s\n", p->ofhint);
      asprintf(&temp, "%s.data", p->ofhint);

      if (NULL==(p->ofp=fopen(temp,"w"))) {
	fprintf(stderr, "(main) Can\'t open output file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      
      /* get file status, includding optimal buffer block size */
      stat(temp, &buf);
      /* set the output file buffer to be optimal, or we'll take longer to 
	 process the output */
      p->ofbuf = (char*)malloc(buf.st_blksize);
      if (setvbuf(p->ofp, p->ofbuf, _IOFBF, buf.st_blksize) != 0) {
	fprintf(stderr, "(main) Can't create output file buffer\n");
	exit(EXIT_FAILURE);
      }

      /* Set the energy file to be the same as me */
      if (p->efp != stdout) {
	fprintf(stderr, "(main)WARN: had to close energy file. It was not\n");
	fprintf(stderr, "            coherent with the output file. If you\n");
	fprintf(stderr, "            want to place energy in a different \n");
	fprintf(stderr, "            file, put the -a option after the -o\n");
	fprintf(stderr, "            The energy output will be placed at\n");
	fprintf(stderr, "            the default output file: %s\n", temp);
	fclose(p->efp);
      }
      p->efp = p->ofp;

      /* Set the event numbers file to be the same as me */
      if (p->evfp != stdout) {
	fprintf(stderr, "(main)WARN: had to close events file. It was not\n");
	fprintf(stderr, "            coherent with the output file. If you\n");
	fprintf(stderr, "            want to place events in a different \n");
	fprintf(stderr, "            file, put the -j option after the -o\n");
	fprintf(stderr, "            The event output will be placed at\n");
	fprintf(stderr, "            the default output file: %s\n", temp);
	fclose(p->efp);
      }
      p->evfp = p->ofp;

      /* set the config file to be stdout if not coherent */
      if (p->cfp != stdout) {
	fprintf(stderr, "(main)WARN: had to close config file. It was not\n");
	fprintf(stderr, "            coherent with the output file. If you\n");
	fprintf(stderr, "            want to place config in a different \n");
	fprintf(stderr, "            file, put the -c option after the -o\n");
	fprintf(stderr, "            The config output will be placed at\n");
	fprintf(stderr, "            the standart output: stdout\n");
	fclose(p->cfp);
	p->cfp = stdout;
      }

      free(temp);
      break;
    }

    case 't': { /* the string to be used when dumping the RoI energy values */
      strncpy(p->edump_comment_str, optarg, 4);
      break;
    }

    case 'k': { /* the string to be used when dumping the event numbers */
      strncpy(p->event_comment_str, optarg, 4);
      break;
    }

    case 'a': { /* output energy parameters to file 'energy-file' */
      char* temp;
      asprintf(&temp, "%s.energy",p->ofhint);
      if (NULL==(p->efp=fopen(temp,"w"))) {
	fprintf(stderr, "(main) Can\'t open energy file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      free(temp);
      break;
    }

    case 'j': { /* output event numbers to file 'eventno-file' */
      char* temp;
      asprintf(&temp, "%s.eventno",p->ofhint);
      if (NULL==(p->evfp=fopen(temp,"w"))) {
	fprintf(stderr, "(main) Can\'t open event number file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      free(temp);
      break;
    }

    case 'c': { /* output configuration parameters to file 'config-file' */
      char* temp;
      asprintf(&temp, "%s.config",p->ofhint);
      if (NULL==(p->cfp=fopen(temp,"w"))) {
	fprintf(stderr, "(main) Can\'t open config file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      free(temp);
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

  /* dump configuration. It shall close the config file automatically. Do *NOT*
     attempt to write on it afterwards! */
  dump_config(p->cfp, p);

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

  /* No sense in printing something that is not required */
  if ( validate_print_selection(&p->layer_flags, &p->print_flags) == FALSE ) {
    fprintf(stderr, "(main)ERROR: Can't continue. Will abort\n");
    exit(EXIT_FAILURE);
  }

  /* No sense in requiring to print ROI energies if dumping digis */
  if (p->dump_digis || p->dump_uniform_digis) {
    string2layer(&p->layer_flags, "none");
    string2edump(&p->dump_energy, "none");
  }

  /* No sense in print energy values that are NOT uniformized */
  if ( validate_energy_selection(&p->layer_flags, &p->dump_energy) == FALSE ) {
    fprintf(stderr, "(main)ERROR: Can't continue. Will abort\n");
    exit(EXIT_FAILURE);
  }

}

void print_help_msg(FILE* fp, const char* prog)
{
  fprintf(fp, "Calorimeter ASCII Data Preprocessor version 0.2\n");
  fprintf(fp, "author: André Rabello dos Anjos <Andre.dos.Anjos@cern.ch>\n");
  fprintf(fp, "usage: %s -i file [-d what] [-f format] [-v]", prog);
  fprintf(fp, " [-n normalization] [-l layer] [-p particle]");
  fprintf(fp, " [-o file] [-abcj] [-e # -r #] [-g energies] [-t en-string]\n");
  fprintf(fp, "       %s -h or --help prints this help message.\n", prog);
  fprintf(fp, "[OPTIONS SUMMARY]\n");

  fprintf(fp, "-i file | --input-file=file\n");
  fprintf(fp, "\t sets the input file name\n");

  fprintf(fp, "-v | --verbose\n");
  fprintf(fp, "\t prints more output than be default\n");

  fprintf(fp, "-o hint | --output-file=hint\n");
  fprintf(fp, "\t sets the output file name (default is stdout) to\n");
  fprintf(fp, "\t 'hint.data'. This file will be created if doesn't exist\n");
  fprintf(fp, "or truncated if it does.\n");

  fprintf(fp, "-c | --config-file\n");
  fprintf(fp, "\t Will place the configuration on a file given by the hint\n");
  fprintf(fp, "\t on -o option (or 'default') concatenated by the string\n");
  fprintf(fp, "\t '.config'. This file (with such filename) will be \n");
  fprintf(fp, "\t  created if doesn't exist or truncated if it does.\n");

  fprintf(fp, "-a | --energy-file\n");
  fprintf(fp, "\t Will place the energy events on a file given by the hint\n");
  fprintf(fp, "\t on -o option (or 'default') concatenated by the string\n");
  fprintf(fp, "\t '.energy'. This file (with such filename) will be \n");
  fprintf(fp, "\t  created if doesn't exist or truncated if it does.\n");

  fprintf(fp, "-j | --eventno-file\n");
  fprintf(fp, "\t Will place the event numbers on a file given by the hint\n");
  fprintf(fp, "\t on -o option (or 'default') concatenated by the string\n");
  fprintf(fp, "\t '.eventno'. This file (with such filename) will be \n");
  fprintf(fp, "\t  created if doesn't exist or truncated if it does.\n");

  fprintf(fp, "-b | --dump-eventno\n");
  fprintf(fp, "\t Will dump event numbers in the output file if this flag\n");
  fprintf(fp, "\t is given.\n");

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

  fprintf(fp, "-g string | --dump-energy=string\n");
  fprintf(fp, "\t dumps energy information according to the values in\n");
  fprintf(fp, "\t 'string'. The 'string' should be a comma separated list\n");
  fprintf(fp, "\t of values. A list may contain one or more of the\n");
  fprintf(fp, "\t following values:\n");
  fprintf(fp, "\t   db_et    - dump the value of transverse energy as it\n");
  fprintf(fp, "\t              be detected by L2\n");
  fprintf(fp, "\t   db_et    - dump the value of transverse energy as it\n");
  fprintf(fp, "\t              be detected by L2 over the hadronic section\n");
  fprintf(fp, "\t   db_t1et  - L1 energy threshold\n");
  fprintf(fp, "\t   roi_et   - The total energy calculated by the point of\n");
  fprintf(fp, "\t              view the selected layers of the uniform RoI\n");
  fprintf(fp, "\t   roi_etem - The total energy on the EM (EM and PS)\n");
  fprintf(fp, "\t              sections calculated using the selected\n");
  fprintf(fp, "\t              layers of the uniform RoI\n");
  fprintf(fp, "\t   roi_ethad- The total energy on the HAD sections\n");
  fprintf(fp, "\t              calculated using the selected layers of the\n");
  fprintf(fp, "\t               uniform RoI\n");
  fprintf(fp, "\t   all      - Print all information described above.\n");
  fprintf(fp, "\t              this is the default behaviour\n");

  fprintf(fp, "-t string | --energy-comment=string\n");
  fprintf(fp, "\t preceeds the energy values by this string when dumping \n");
  fprintf(fp, "\t Only 5 characters are allowed, the rest will be ignored \n");
  fprintf(fp, "\t The default behaviour is to print nothing before values\n");

  fprintf(fp, "-k string | --eventno-comment=string\n");
  fprintf(fp, "\t preceeds the event numbers by this string when dumping \n");
  fprintf(fp, "\t Only 5 characters are allowed, the rest will be ignored \n");
  fprintf(fp, "\t The default behaviour is to print nothing before values\n");

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
  fprintf(fp, "\t Requires information of the layers specified by string.\n"); 
  fprintf(fp, "\t to be present on event. Event is eliminated if do not \n");
  fprintf(fp, "\t contain this information. This has no importance when \n");
  fprintf(fp, "\t dumping digis. The layers shall be separated by comman\n");
  fprintf(fp, "\t and/or spaces and may be one or more of the following:\n");
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

  fprintf(fp, "-s string | --select=string\n");
  fprintf(fp, "\t Prints layer information of the layers specified by\n");
  fprintf(fp, "\t string. This flag has only sense when dumping urois or\n");
  fprintf(fp, "\t rings. If The layers shall be separated by comma and/or\n");
  fprintf(fp, "\t spaces and may be one or more of the following:\n");
  fprintf(fp, "\t   ps   - require PreSample information\n");
  fprintf(fp, "\t   em1  - require EM front layer\n");
  fprintf(fp, "\t   em2  - require EM middle layer\n");
  fprintf(fp, "\t   em3  - require EM back layer\n");
  fprintf(fp, "\t   had1 - require HAD front layer\n");
  fprintf(fp, "\t   had2 - require HAD middle layer\n");
  fprintf(fp, "\t   had3 - require HAD back layer\n");
  fprintf(fp, "\t   all  - require all layers. If this is activated, the\n");
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
bool_t process_ROI(const ROI* roi, const parameter_t* p)
{
  tt_roi_t ttroi; /* The calorimeter RoI */

  if(roi->calDigi.nEmDigi == 0 && roi->calDigi.nhadDigi == 0) {
    fprintf(stderr,"\n");
    fprintf(stderr,"(main)ERROR: ROI has no digi info. ");
    return (FALSE);
  }

  ++dumped_events; /* Can we count one more valid event? */
   
  if (p->dump_digis) {
    if(p->dump_eventno) /* do I have to dumpe event numbers? */
      fprintf(p->evfp, "%s %d %d\n", p->event_comment_str,
	      event_counter, dumped_events);
    dump_DIGIS(p->ofp,roi);
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

      if(p->dump_eventno) /* do I have to dumpe event numbers? */
      fprintf(p->evfp, "%s %d %d\n", p->event_comment_str,
	      event_counter, dumped_events);

      /* Do we have to dump digis? If yes, we can't print anything else */
      if (p->dump_uniform_digis) dump_DIGIS(p->ofp,roi);

      else {
	/* Do we have to dump energy information? */
	if ( p->dump_energy ) {
	  char* t=get_energy(roi, &ur, p->dump_energy, p->edump_comment_str);
	  fprintf(p->efp, "%s\n", t);
	  free(t);
	}

	/* Do we have to dump rings or uniform RoIs? */
	if (p->dump_rings) dump_rings(&ur, p);

	/* To print the uniform RoIs is what is left for us... */
	else print_uniform_roi (p->ofp,&ur,p->print_flags);
      }
	
      /* An extra space between outputs */
      fprintf(p->ofp, "\n"); 

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
int dump_rings (const uniform_roi_t* ur, const parameter_t* p)
{
  char* dump;
  char* temp;
  ringroi_t ringroi;

  /* I have to change the value pointed by rp */
  ringroi.nring = ring_sum(ur, &ringroi, p->print_flags);

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
      if (dumped_events == 1 && p->format_snns) 
	fprintf_SNNS_header(p->ofp, 1, _iunits, 1);

      fprintf(p->ofp,"%s",dump);

      free_ring_vector(ringroi.ring, ringroi.nring);
      free(dump); /* the information to print */
  }
  return (dumped_events);
}

/* This procedure will dump the configuration data into a organized format,
   optionally into a file. Between the dumped data one may find the current
   date of the test and the output and input file name. For understand these
   lines, go through the printed strings. It can't be hard. */
void dump_config(FILE* fp, const parameter_t* par) 
{
  time_t current_time = time(NULL); /* just a dummy variable for timing */
  char temp[60]; /* for temporary strings */

  fprintf(fp, "-+- CONFIGURATION PARAMETERS\n");
  fprintf(fp, " +- Date: %s", ctime(&current_time));
  fprintf(fp, " +- Input File: %s\n", par->ifname);
  if (par->ofp != stdout) 
    fprintf(fp, " +- Output File: %s.data \n", par->ofhint);

  fprintf(fp, " +- Output Information: ");
  if (par->dump_rings) fprintf(fp, "Rings\n");
  else if (par->dump_digis) fprintf(fp, "Digis\n");
  else if (par->dump_digis) fprintf(fp, "Uniform Digis\n");
  else fprintf(fp, "Uniform Rois\n");

  /* The type of dumped information */
  if (! par->dump_digis ) {
    fprintf(fp, " +- Uniformizing Selection: %s\n", 
	    layer2string(&par->layer_flags,temp));
    if (! par->dump_uniform_digis ) {
      fprintf(fp, " +- Printing Selection: %s\n", 
	      layer2string(&par->print_flags,temp));
      fprintf(fp, " +- Normalization: %s\n", 
	      normalization2string(&par->normalization,temp));
    }
  }

  /* The event accouting */
  fprintf(fp, " +- Event number dumping: %s\n", 
	  (par->dump_eventno)?"Yes":"No" );

  /* The event number filename */
  if (par->evfp != stdout) {
    if (par->evfp != par->ofp)
      fprintf(fp, " +- Event-number File: %s.eventno \n", par->ofhint); 
    else
      fprintf(fp, " +- Event-number File: %s.data \n", par->ofhint);
  }

  /* The event number comment string, if it exists */
  if (strcmp(par->event_comment_str, "") != 0 )
    fprintf(fp, " +- Event number comment string: %s\n", 
	    par->event_comment_str);

  /* What types of energy are going to be printed */
  fprintf(fp, " +- Energy printing: %s\n", 
	  edump2string(&par->dump_energy,temp));

  /* The energy filename */
  if (par->efp != stdout) {
    if (par->efp != par->ofp)
      fprintf(fp, " +- Energy File: %s.energy \n", par->ofhint); 
    else 
      fprintf(fp, " +- Energy File: %s.data \n", par->ofhint);
  }

  /* The energy comment string, if it exists */
  if (strcmp(par->edump_comment_str, "") != 0 )
    fprintf(fp, " +- Energy comment string: %s\n", par->edump_comment_str);

  fprintf(fp, " +- Output Format: ");
  if (par->format_snns) {
    fprintf(fp, "SNNS\n");
    fprintf(fp, " +- Particle Type for SNNS targets: ");
    if (par->particle == JET) fprintf(fp, "Jet\n");
    else fprintf(fp, "Electron\n");
  }
  else fprintf(fp, "Raw\n");

  if (par->process_all_rois)
    fprintf(fp, " +- Processing: All Events\n");
  else
    fprintf(fp, " +- Processing: Event %ld, RoI %ld\n", 
	    par->event_no, par->roi_no);

  fprintf(fp, "==========================================\n");

  /* close the configuration file pointer, so, no need to do it afterwards...*/
  fclose(fp);
  
}


