/* Hello emacs, this is -*- c -*- */

/* $Id: ring.c,v 1.2 2000/07/07 18:50:09 rabello Exp $ */

#include "ring.h"
#include "ttdef.h"
#include "common.h"
#include "trigtowr.h"
#include "uniform.h"

int get_max_idx(const CaloLayer*);
ring_t* ring_sum_around (const CaloLayer*, const int, ring_t*);
bool_t put_ring(ring_t*, const Energy, int);


void print_ring (FILE* fp, const ring_t* rp)
{
  int i;
  for (i = 0; i < rp->nfeat; ++i)
    fprintf(fp, "%e ",rp->feat[i]);
  
  fprintf(fp, "\n");
  return;
}


ring_t* ring_sum (const tt_roi_t* r, ring_t* ring)
{
  int max;
  uniform_roi_t ur;

  ur.nlayer = 0;
  ur.layer = NULL;
  
  /* Transform the tt_roi_t in succesive layers of a uniform TT*/
  uniformize(r,&ur);

  /* Search for the highest energy value. For now working only with one layer
   */ 
  max = get_max_idx(&ur.layer[0]);

  /* Evaluate the rings */
  ring_sum_around(&ur.layer[0],max,ring);
  
  free_uniform_roi(&ur);

  return (ring);
}

/* This function only evaluates the maximum energy peak and return the results
   to the caller */
int get_max_idx(const CaloLayer* l)
{
  int eta, phi; /* iterators */

  int linear_idx = 0;

  for (phi = 0; phi < l->PhiGran; ++phi)
    for (eta = 0; eta < l->EtaGran; ++eta) {
      int tmp1 = eta + (l->EtaGran * phi);
      if (l->cell[tmp1].energy > l->cell[linear_idx].energy) linear_idx = tmp1;
    }
  
  return linear_idx;
}

/* Evaluates the sum around the peak energy, given by the second argument. This
   function shall allocate the space for the numbers in ring_t. The user *MUST*
   free this afterwards, using free_ring(). */
ring_t* ring_sum_around (const CaloLayer* l, const int max, ring_t* ring)
{
  int x1,x2,y1,y2; /* helpers for placement */
  int nf = 0; 
  
  /* Place the cell with maximum energy at the front */
  ring->nfeat = 0;
  put_ring(ring,l->cell[max].energy, nf++);
  
  /* set the helpers in order to start */
  x1 = l->cell[max].index.eta;
  x2 = x1;
  y1 = l->cell[max].index.phi;
  y2 = y1;

  /* Now let's get to the neighbors */
  while (TRUE) {
    int e,p; /* iterators */

    if (x1 != 0) --x1;
    if (x2 != l->EtaGran-1) ++x2;
    if (y1 != 0) --y1;
    if (y2 != l->PhiGran-1) ++y2;

    for (p = y1; p <= y2; ++p)
      for (e = x1; e <= x2; ++e)
	put_ring(ring,l->cell[e+p*l->EtaGran].energy,nf);

    /* Since I've added ring(nf-1) as well on the above loop, I should take
       it out (discount). */
    for (p = 0; p <= nf-1; ++p)
      put_ring(ring,-ring->feat[p],nf);
    
    ++nf;

    if (x1 == 0 && x2 == (l->EtaGran-1) && 
	y1 == 0 && y2 == (l->PhiGran-1) ) /* I've evaluated it all! */
      break;

  }

  return (ring);
}

/* This function only inserts an energy value into the ring output given by the
   first argument. The second argument is the energy to add and the third, the
   ring to put it in. This function will allocate more and more space as it
   needs in order to do its job. Placing a cell energy where there's already
   something is OK, the function will sum the values, instead of allocating
   more space (case of new rings). */
bool_t put_ring(ring_t* r, const Energy e, int nr)
{
  if (r->nfeat == 0) /* first allocation */ {
    r->feat = (Energy*) mxalloc(NULL,1,sizeof(Energy));
    r->feat[0] = e;
    r->nfeat = 1;
    return (TRUE);
  }
  
  /* else we have to check the need for reallocation of memory */
  if ( nr < r->nfeat ) /* no need for realloc, just adding */
    r->feat[nr] += e;

  else {/* In this case the caller asked me to allocate in a new feature */
    if ( nr != r->nfeat ) /* Is it asking to allocate 2 or more new feats? */
      return (FALSE);
  
    /* If everything goes smooth, I can realloc here */
    r->feat = (Energy*) mxalloc(r->feat, ++r->nfeat, sizeof(Energy));
    r->feat[nr] = e;
    
  }
  
  return (TRUE);
  
}

bool_t free_ring (ring_t* r)
{
  free(r->feat);
  return (TRUE);
}

