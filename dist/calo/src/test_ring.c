/* Hello emacs, this is -*- c -*- */
/* Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: test_ring.c,v 1.1 2000/07/07 18:49:51 rabello Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "ttdef.h"
#include "ring.h"

bool_t read_calolayer(FILE*, CaloLayer*);

/* The next functions are not public */
extern ring_t* ring_sum_around (const CaloLayer*, const int, ring_t*);
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
    fprintf(stdout, "(eta_granularity:int) (phi_granularity:int)\n\n");
    fprintf(stdout, "(cells:Energy)\n\n");
    fprintf(stdout, "obs1: 'cells' must have at least 1 space");
    fprintf(stdout, " between them.\n");
    fprintf(stdout, "obs2: Besides obs1 'cells' can be arranged");
    fprintf(stdout, " in any order.\n");
    exit(0);
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

  fprintf(stdout, "(test_ring): Peak -> ");

  /* get peak */
  max = get_max_idx(&cl);
  fprintf(stdout, "eta: %d, phi: %d\n", cl.cell[max].index.eta,
	  cl.cell[max].index.phi);
  
  /* do the ring sum */
  ring_sum_around(&cl,max,&ring);
  
  print_ring(stdout, &ring);

  return (0);
}

/* Read in the calolayer descriptor from file */
bool_t read_calolayer(FILE* fp, CaloLayer* clp)
{
  int i; /* iterator */
  div_t result;

  fscanf(fp,"%ud", &clp->EtaGran);
  fscanf(fp,"%ud", &clp->PhiGran);

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

  


  
