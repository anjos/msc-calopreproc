/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: parameter.h,v 1.5 2000/09/19 00:34:20 andre Exp $ */

/* The door keepers */
#ifndef __PARAMETER_H
#define __PARAMETER_H

#include <stdio.h>
#include <obstack.h>

#include "common.h"

/* The maximum input file name */
#define MAX_FILENAME 256

/* Particle types available */
typedef enum particle_t {ELECTRON = 0, JET = 1} particle_t;

/* The following type define the parameters for program execution */
typedef struct pararamter_t 
{
  /* The input file pointer */
  FILE* ifp; 

  /* The input file buffer */
  char* ifbuf; 

  /* The output file pointer */
  FILE* ofp;

  /* The output file buffer */
  char* ofbuf;

  /* Shall I use an output file? */
  bool_t output_file;

  /* The output object stack. (for fast execution) */
  struct obstack* output_obsp;

  /* The output file name hint will be copied into here */
  char ofhint[MAX_FILENAME+1];

  /* The input file name will be copied into here */
  char ifname[MAX_FILENAME+1]; 

  /* The file where the config will be stored */
  FILE* cfp;

  /* Shall I use a configuration file? */
  bool_t config_file;

  /* The file where the energy data will be stored if asked */
  FILE* efp;

  /* Shall I use an energy file? */
  bool_t energy_file;

  /* The energy object stack. (for fast execution) */
  struct obstack* energy_obsp;

  /* The type of particle present on file */
  particle_t particle; 

  /* Which layers to process. See uniform.[ch] */
  unsigned short layer_flags; 

  /* Which layers to dump. See uniform.[ch] */
  unsigned short print_flags; 

  /* Normalization scheme. See module normal */
  unsigned short normalization; 

  /* Holds the value of radius to use when normalizing using the unity+
     technique. */
  Energy max_radius;

  /* The event number to process */
  long int event_no; 

  /* The roi number to process */
  long int roi_no; 

  /* The file where the event numbers will be stored if asked */
  FILE* evfp; 

  /* Shall I use a event number file? */
  bool_t eventno_file;

  /* The event number object stack. (for fast execution) */
  struct obstack* eventno_obsp;

  /* Do I have to output data to memory instead of file? */
  bool_t run_fast;

  /* Do I have to dump event numbers? */
  bool_t dump_eventno; 

  /* The string that should contain the initial characters to print with the
     event number variables. It's usefull for printing comment like information
     for many script languages and processors like MatLab or PAW. */
  char event_comment_str[5]; 

  /* Do I have to dump rings? */
  bool_t dump_rings; 

  /* Do I have to dump digis? */
  bool_t dump_digis; 

  /* Do I have to dump digis from uniform RoIs? */
  bool_t dump_uniform_digis; 

  /* Dumps data using SNNS formatting */
  bool_t format_snns; 

  /* Chooses to dump the energy variables or not */
  unsigned short dump_energy; 

  /* The string that should contain the initial characters to print with the
     energy variables. It's usefull for printing comment like information for
     many script languages and processors like MatLab or PAW. */
  char edump_comment_str[5];

  /* Do I have to process all events? */
  bool_t process_all_events; 

  /* Do I have to process all rois for each event? */
  bool_t process_all_rois; 

  /* Do I have to process all rois for each event? */
  bool_t verbose;

  /* How many events to load per loop. This variable defines how many events
     should I read from file before start processing the output. The important
     thing is that the more events you read in, the more memory you consume and
     the faster you go, so, that depends on how many memory you have and
     time. Typical dump files are big and have around thousand events. Each
     event is about 150 to 200 kilobytes, so, from there you can find how many
     events to load per processing loop. Don't forget you consume other memory
     resources by the kernel and by this and other user level processes running
     locally. */
  long int load_events;

} parameter_t;

/* This function sets up the default parameters for the variable pointed by the
   only argument. The space for that should be pre-allocated. I suggest using
   static allocation for that and not to bother with memory freeing
   afterwards. */ 
parameter_t* init_parameters (parameter_t*);

/* This function will process the input flags sent by the command line options
   and will activate and/or change the appropriate flags of the parameter_t*
   accordingly. */
void process_flags (parameter_t*, const int, char**);

/* This function will close all unclosed files, free dinamically allocated
   memory and return gracefully. Use this before ending your program */
void terminate_parameters (parameter_t* p);

/* This function will write the event into file or memory bank, depending on
   the users' choice. As parameters, it receives the output file, the output
   object stack, whether to dump to the memory (TRUE) or to directly to file
   (FALSE) and the information on a C-style string. */
void output_string(FILE*, struct obstack*, const bool_t, const char*);

#endif /* __PARAMETER_H */




