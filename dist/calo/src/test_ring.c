/* Hello emacs, this is -*- c -*- */
/* Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: test_ring.c,v 1.2 2000/07/20 00:46:37 rabello Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "ttdef.h"
#include "ring.h"

bool_t read_calolayer(FILE*, CaloLayer*);

/* One global, not to mess too much */
static int global_nfeat;

/* The next functions are not public, they are borrowed from ring.c */
ring_t* ring_sum_around (const CaloLayer*, ring_t*, const int);
extern int get_max_idx(const CaloLayer*);

int main (int argc, char** argv)
{
  CaloLayer cl;
  FILE* fp;
  int max;
  ring_t ring;

  /* Just check whether we have an input file or not */
  if (argc == 1 || argc > 2) {
    fprintf(stdout, "usage: %s [matrix file]\n", argv[0]);
    fprintf(stdout, "matrix file is an ASCII file containing");
    fprintf(stdout, " the following fields:\n\n");
    fprintf(stdout, "(eta_gran:int) (phi_gran:int) (nfeat:int)\n\n");
    fprintf(stdout, "(cells:Energy)\n\n");
    fprintf(stdout, "obs1: 'cells' must have at least 1 space");
    fprintf(stdout, " between them.\n");
    fprintf(stdout, "obs2: Besides obs1 'cells' can be arranged");
    fprintf(stdout, " in any order.\n");
    exit(EXIT_SUCCESS);
  }

  if ( (fp=fopen(argv[1],"r")) == NULL ) {
    fprintf(stdout, "(test_ring): can't open input file.\n");
    exit(-1);
  }

  if (!read_calolayer(fp,&cl)) {
    fprintf(stdout, "(test_ring): can't read in data.\n");
    exit(-1);
  }

  fprintf(stdout, "(test_ring): Granularity -> ");
  fprintf(stdout, "eta: %d, phi: %d\n", cl.EtaGran, cl.PhiGran);
  fprintf(stdout, "number of features to collect -> %d\n", global_nfeat);

  fprintf(stdout, "(test_ring): Peak -> ");

  /* get peak */
  max = get_max_idx(&cl);
  fprintf(stdout, "eta: %d, phi: %d\n", cl.cell[max].index.eta,
	  cl.cell[max].index.phi);

  /* Allocate ring space */
  ring.feat = (Energy*) calloc (global_nfeat,sizeof(Energy));
  ring.nfeat = global_nfeat;
  
  /* do the ring sum */
  ring_sum_around(&cl,&ring,max);
  
  print_ring(stdout, &ring);

  return (EXIT_SUCCESS);
}

/* Read in the calolayer descriptor from file */
bool_t read_calolayer(FILE* fp, CaloLayer* clp)
{
  int i; /* iterator */
  div_t result;

  fscanf(fp,"%ud", &clp->EtaGran);
  fscanf(fp,"%ud", &clp->PhiGran);

  fscanf(fp,"%ud", &global_nfeat);

  if (clp->EtaGran == 0 || clp->PhiGran == 0) {
    fprintf(stdout, "(test_ring): eta or phi granularity is zero.\n");
    return (FALSE);
  }

  clp->NoOfCells = clp->EtaGran * clp->PhiGran;

  clp->cell = (CaloCell*) mxalloc (NULL, clp->NoOfCells, sizeof(CaloCell));

  for(i=0; i<clp->NoOfCells; ++i) {
    fscanf(fp, "%lf", &clp->cell[i].energy);

    /* The indexes */
    result = div(i, clp->EtaGran);
    clp->cell[i].index.eta = result.rem;
    clp->cell[i].index.phi = result.quot;
  }

  clp->calo = EMBARREL;
  clp->level = 2;

  return (TRUE);
}

  


  
