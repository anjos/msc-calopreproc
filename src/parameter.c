/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: parameter.c,v 1.2 2000/08/27 16:22:55 andre Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <time.h>
#include <sys/stat.h>

/* Obstack stands for object stack and is a kind of memory bank that can hold
   several different types of variables together. In principal, one could start
   adding things to it and print them to a file afterwards. An obstack is smart
   enough to grow it self up and make more room for incoming data. One only has
   to say which function to use for that. I chose stdlib::malloc() and
   stdlib::free() for allocation and de-allocation procedures. This is usefull
   for extra-fast processing without writing results to disk all the time,
   which can be time consuming if the file is across the network. For more
   information on obstacks do 'info libc' at your (linux?) prompt. */
#include <obstack.h>

#include "parameter.h"
#include "energy.h"
#include "uniform.h"
#include "util.h"

/* The definition of obstack initialization and destruction */
#define obstack_chunk_alloc malloc 
#define obstack_chunk_free free

/**********************/
/* The internal stuff */
/**********************/

/* This function puts a string into an obstack of strings. It eliminates the
   NULL char at the end of the argument string automatically and places it at
   the end of the whole set. The obstack pointer should point to an initialized
   obstack. This function returns the number of chars into object at the
   instant after the allocation for the current string. */
int grow_obstring(struct obstack*, const char*);

/* Terminates the obstring as determined by its implementation. This also puts
   a null character at the end of the string so one can use the string
   actively. This function will return the final obstack address. */
char* close_obstring(struct obstack*);

/* This function will test the conditions of program execution and see if one
   has a coherent set of flags activated */
void test_flags (parameter_t*);

/* Prints a rather long help message containing all possible command line
   options and explanation on those options */
void print_help_msg (FILE*, const char*);

/* Dump the actual configuration into a file or to stdout */
void dump_config(FILE*, const parameter_t*);

/* This funtion just returns a valid long based on a string. It's an
   implemenatation of atol(), but with the verification of strtol(). */
long to_valid_long(const char*);

/* This funtion just returns a valid long based on a string. It's an
   implemenatation of atol(), but with the verification of strtol(). */
double to_valid_double(const char*);


parameter_t* init_parameters (parameter_t* p)
{

  /* The input file and associated buffer */
  p->ifp = 0;
  p->ifbuf = 0;

  /* by default, dump to screen */
  p->ofp = stdout; 
  p->ofbuf = 0;

  /* by default dump configuration to screen */
  p->cfp = stdout; 

  /* the energy file will be stdout by default */
  p->efp = stdout; 

  /* Print ALL energy values before event data */
  string2edump(&p->dump_energy, "all");
  
  /* by default, nothing is to be printed before the energy parameters */
  strcpy(p->edump_comment_str, ""); 

  /* the event number file will be stdout by default */
  p->evfp = stdout;
  p->event_no = 0;
  p->roi_no = 0;

  /* do not dump event numbers by default */
  p->dump_eventno = FALSE; 

  /* by default, nothing is to be printed before the event numbers */
  strcpy(p->event_comment_str, ""); 

  /* process all by default */
  p->process_all_events = TRUE;

  /* processs all rois */
  p->process_all_rois = TRUE; 

  /* by default, use electron */
  p->particle = ELECTRON;  

  /* rings are a specific case */
  p->dump_rings = FALSE; 

  /* digis are a specific case */
  p->dump_digis = FALSE; 

  /* uniform_digis are a specific case */
  p->dump_uniform_digis = FALSE; 

  /* SNNS formating for output */
  p->format_snns = FALSE;

  /* verbose output? */
  p->verbose = FALSE;

  /* The input file hint will be "default" */
  strncpy(p->ofhint, "default", MAX_FILENAME);

  /* Require all layers by default */
  string2layer(&p->layer_flags, "all");

  /* Dump all layers by default */
  string2layer(&p->print_flags, "all");

  /* Don't normalize by default */
  string2normalization(&p->normalization, "none");

  /* by, default, run using local file deposition instead of memory (slower)*/
  p->run_fast = FALSE;

  return p;
}

/* This function processes the command line options ('-- and -' pararamteres),
   changing the structure of parameters accordingly. */
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
    {"fast-output", 0, 0, 'u'},
    {"verbose", 0, 0, 'v'},
    {0, 0, 0, 0}
  };
  
  while (EOF != (c=getopt_long(argc, argv, "i:o:bcajk:g:he:r:d:f:p:l:s:n:t:uv",
			       long_options, &option_index) ) ) {
    switch (c) {

    case 0: /* Got a flagged option */
      break;
      
    case 'i': { /* input from file 'infile' */
      struct stat buf;
      /* Ok, open the file we're going to work with, in case of error,
	 give up */
      if (NULL==(p->ifp=fopen(optarg,"r"))) {
	fprintf(stderr, "(param) Can't open input file %s\n", optarg);
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
	fprintf(stderr, "(param) Can't create input file buffer\n");
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
	fprintf(stderr,"(param)ERROR: Can't recognize format -> %s\n", optarg);
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
	fprintf(stderr,"(param)ERROR: Can't recognize format -> %s\n", optarg);
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
	fprintf(stderr,"(param)ERROR: A type of particle ?? -> %s\n", optarg);
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
      fprintf(stderr,"(param)WARN: will use hint -> %s\n", p->ofhint);
      asprintf(&temp, "%s.data", p->ofhint);

      if (NULL==(p->ofp=fopen(temp,"w"))) {
	fprintf(stderr, "(param) Can\'t open output file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      
      /* get file status, includding optimal buffer block size */
      stat(temp, &buf);
      /* set the output file buffer to be optimal, or we'll take longer to 
	 process the output */
      p->ofbuf = (char*)malloc(buf.st_blksize);
      if (setvbuf(p->ofp, p->ofbuf, _IOFBF, buf.st_blksize) != 0) {
	fprintf(stderr, "(param) Can't create output file buffer\n");
	exit(EXIT_FAILURE);
      }

      /* Set the energy file to be the same as me */
      if (p->efp != stdout) {
	fprintf(stderr, "(param)WARN: had to close energy file. It was not\n");
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
	fprintf(stderr, "(param)WARN: had to close events file. It was not\n");
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
	fprintf(stderr, "(param)WARN: had to close config file. It was not\n");
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
	fprintf(stderr, "(param) Can\'t open energy file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      free(temp);
      break;
    }

    case 'j': { /* output event numbers to file 'eventno-file' */
      char* temp;
      asprintf(&temp, "%s.eventno",p->ofhint);
      if (NULL==(p->evfp=fopen(temp,"w"))) {
	fprintf(stderr, "(param) Can\'t open event number file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      free(temp);
      break;
    }

    case 'c': { /* output configuration parameters to file 'config-file' */
      char* temp;
      asprintf(&temp, "%s.config",p->ofhint);
      if (NULL==(p->cfp=fopen(temp,"w"))) {
	fprintf(stderr, "(param) Can\'t open config file %s\n", temp);
	exit(EXIT_FAILURE);
      }
      free(temp);
      break;
    }

    /* The runner whats fast output. This means obstacks for all outputs and
       their initialization, and of course lots of memory */
    case 'u': { 
     obstack_init(&p->eventno_obs);
     obstack_init(&p->output_obs);
     obstack_init(&p->energy_obs);
     p->run_fast = TRUE;
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

void test_flags (parameter_t* p)
{
  /* Will process all ROIs of 1 event -> not acceptable */
  if (!p->process_all_events && p->process_all_rois) {
    fprintf(stderr, "(param)WARN: Can't process all ROIs in one event.\n");
  }

  /* Will process a specific roi for all events */
  if (p->process_all_events && !p->process_all_rois) {
    fprintf(stderr,"(param)ERROR: No sense in not setting the event number.\n");
    exit(EXIT_FAILURE);
  }

  /* we should have at least the input filename */
  if (p->ifp == NULL) {
    fprintf(stderr, "(param) at least the input filename must be supplied.\n");
    exit(EXIT_FAILURE);
  }
  
  /* I can't do 2 things at once */
  if ((p->dump_digis || p->dump_uniform_digis) && p->dump_rings){
    fprintf(stderr, "(param) I can't dump rings *AND* digis at once.\n");
    exit(EXIT_FAILURE);
  }

  /* I can't do 2 things at once */
  if (p->dump_uniform_digis && p->dump_digis){
    fprintf(stderr, "(param)WARN: I'll dump uniform RoI digis only.\n");
    p->dump_digis = FALSE;
  }

  /* No sense in SNNS formatting and digi output */
  if ( (! p->dump_rings) && p->format_snns) {
    fprintf(stderr, "(param)WARN: No sense in formatting (SNNS) for ");
    fprintf(stderr, "something that is *NOT* a ring.\n");
    p->format_snns = FALSE;
    fprintf(stderr, "(param)WARN: SNNS formatting DEACTIVATED!\n");
  }

  /* No sense in printing something that is not required */
  if ( validate_print_selection(&p->layer_flags, &p->print_flags) == FALSE ) {
    fprintf(stderr, "(param)ERROR: Can't continue. Will abort\n");
    exit(EXIT_FAILURE);
  }

  /* No sense in requiring to print ROI energies if dumping digis */
  if (p->dump_energy && p->dump_digis) {
    fprintf(stderr, "(param)WARN: Can't print energies or layers when digis");
    fprintf(stderr, "are required to be printed.\n");
    fprintf(stderr, "             I'll adjust layer and energy properties");
    fprintf(stderr, " myself \n");
    string2edump(&p->dump_energy, "none");
    string2layer(&p->layer_flags, "none");
  }

  /* No sense in print energy values that are NOT uniformized */
  if ( validate_energy_selection(&p->layer_flags, &p->dump_energy) == FALSE ) {
    fprintf(stderr, "(param)ERROR: Can't continue. Will abort\n");
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
  fprintf(fp, "\t   roi_digis- The total energy found summing all digis\n");
  fprintf(fp, "\t              on a RoI without any preprocessing\n");
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

  fprintf(fp, "-u | --fast-output\n");
  fprintf(fp, "\t when this option is activated, the output for each\n");
  fprintf(fp, "\t event will happen to a memory bank instead of \n");
  fprintf(fp, "\t direct access to a file. This will save access time\n");
  fprintf(fp, "\t what can be crucial at network operations but may \n");
  fprintf(fp, "\t consume huges amounts of memory, so be warned.\n");
  fprintf(fp, "\t At the end, the memory banks are dumped into files.\n");

}

void terminate_parameters (parameter_t* p)
{
  /* Dump, if fast-executing, the obstacks into files */
  if (p->run_fast) {
    fprintf ( p->efp, "%s", close_obstring ( &p->energy_obs ) );
    fprintf ( p->evfp, "%s", close_obstring ( &p->eventno_obs ) );
    fprintf ( p->ofp, "%s", close_obstring ( &p->output_obs ) );
  }

  /* close the files we may open: event-number and energy */
  if ( (p->evfp != p->ofp) && (p->evfp != stdout) )
    fclose(p->evfp);

  if ( (p->efp != p->ofp) && (p->efp != stdout) )
    fclose(p->efp);

  /* close streams and free pointers */
  fclose(p->ifp);
  free(p->ifbuf);

  if ( p->ofp != stdout ) {
    fclose(p->ofp);
    free(p->ofbuf);
  }

  return;
}

/* This procedure will dump the configuration data into an organized format,
   optionally into a file. Between the dumped data one may find the current
   date of the test and the output and input file name. For understand these
   lines, go through the printed strings. It can't be hard. */
void dump_config(FILE* fp, const parameter_t* par) 
{
  time_t current_time = time(NULL); /* just a dummy variable for timing */
  char temp[100]; /* for temporary strings */

  fprintf(fp, "-+- CONFIGURATION PARAMETERS\n");
  fprintf(fp, " +- Date: %s", ctime(&current_time));
  fprintf(fp, " +- Input File: %s\n", par->ifname);
  if (par->ofp != stdout) 
    fprintf(fp, " +- Output File: %s.data \n", par->ofhint);
  else
    fprintf(fp, " +- Output File: [redirected to standard output]\n");

  fprintf(fp, " +- Output Information: ");
  if (par->dump_rings) fprintf(fp, "Rings\n");
  else if (par->dump_digis) fprintf(fp, "Digis\n");
  else if (par->dump_uniform_digis) fprintf(fp, "Uniform Digis\n");
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
  if (par->evfp != stdout && par->dump_eventno) {
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
  if (par->efp != stdout && par->dump_energy) {
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

  /* close the configuration file pointer, so, no need to do it
     afterwards. This can only be done *IF* the pointer is *NOT* a pointer to
     stdout! */
  if (fp != stdout) fclose(fp);
  
}

/* Converts a string into a valid double or issue an error message */
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

/* Converts a string int a valid double or issue an error message */
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

/* This function puts a string into an obstack of strings. It eliminates the
   NULL char at the end of the argument string automatically and places it at
   the end of the whole set. The obstack pointer should point to an initialized
   obstack. This function returns the number of chars into object at the
   instant after the allocation for the current string. */
int grow_obstring(struct obstack* to, const char* from)
{
  const int slen = strlen(from);
  obstack_grow (to, from, slen);
  return obstack_object_size(to);
}

/* Terminates the obstring as determined by its implementation. This also puts
   a null character at the end of the string so one can use the string
   actively. This function will return the final obstack address. */
char* close_obstring(struct obstack* strob)
{
  /* Places NULL char at the end of string */
  obstack_1grow(strob, 0);

  /* Closes the obstack-string */
  return obstack_finish(strob);
}

/* This function will write the event into file or memory bank, depending on
   the users' choice. As parameters, it receives the output file, the output
   object stack, whether to dump to the memory (TRUE) or to directly to file
   (FALSE) and the information on a C-style string. */
void output_string(FILE* fp, struct obstack* obp, const bool_t run_fast, 
		   const char* info)
{
  if (run_fast) grow_obstring(obp, info);
  else fprintf(fp, "%s", info);
  return;
}
