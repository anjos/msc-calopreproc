/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: parameter.h,v 1.1 2000/08/22 02:51:46 andre Exp $ */

/* The door keepers */
#ifndef __PARAMETER_H
#define __PARAMETER_H

#include <stdio.h>

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

  /* The output file name hint will be copied into here */
  char ofhint[MAX_FILENAME+1];

  /* The input file name will be copied into here */
  char ifname[MAX_FILENAME+1]; 

  /* The file where the config will be stored */
  FILE* cfp; 

  /* The file where the energy data will be stored if asked */
  FILE* efp; 

  /* The type of particle present on file */
  particle_t particle; 

  /* Which layers to process. See uniform.[ch] */
  unsigned short layer_flags; 

  /* Which layers to dump. See uniform.[ch] */
  unsigned short print_flags; 

  /* Normalization scheme. See module uniform */
  unsigned short normalization; 

  /* The event number to process */
  long int event_no; 

  /* The roi number to process */
  long int roi_no; 

  /* The file where the event numbers will be stored if asked */
  FILE* evfp; 

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

#endif /* __PARAMETER_H */




