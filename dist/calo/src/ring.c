/* Hello emacs, this is -*- c -*- */

/* $Id: ring.c,v 1.3 2000/07/12 04:34:00 rabello Exp $ */

#include <stdio.h>
#include <stdlib.h>

#include "ring.h"
#include "ttdef.h"
#include "common.h"
#include "trigtowr.h"
#include "uniform.h"

/* I can only accept this types of calorimeter */
typedef enum ringcalo_t {PS=PSBARRREL, EM=EMBARREL, HAD=TILECAL} ringcalo_t;

int get_max_idx(const CaloLayer*);
ring_t* ring_sum_around (const CaloLayer*, ring_t*, const int);
bool_t put_ring(ring_t*, const Energy, int);
bool_t free_ring_vector (ring_t*, const int);
void print_ring_vector (FILE*, const ring_t*, const int);
void alloc_feature_space(ring_t*, const ringcalo_t, const LayerLevel level);

void dump_ring (FILE* fp, const tt_roi_t* roi)
{
  ring_t* rp = (ring_t*) calloc(1,sizeof(ring_t));
  int nring; /* the number of rings produced */

  nring = ring_sum (roi, &rp); /* I have to change the value pointed by rp */
  if (nring > 0) {
    print_ring_vector (fp, rp, nring);
    free_ring_vector(rp, nring);
  }

  return;
}

/* This functions prints the ring vector pointed by rp and containing nring
   elements. The output goes to the file pointed by fp. */
void print_ring_vector (FILE* fp, const ring_t* rp, const int nring)
{
  int i; /* iterator */
  for(i=0; i<nring; ++i) print_ring(fp, &rp[i]);
  return;
}

void print_ring (FILE* fp, const ring_t* rp)
{
  int i;
  for (i = 0; i < rp->nfeat; ++i)
    fprintf(fp, "%e ",rp->feat[i]);
  
  fprintf(fp, "\n");
  return;
}


int ring_sum (const tt_roi_t* r, ring_t* ring[])
{
  int max;
  uniform_roi_t ur;
  int i; /* iterator */

  ur.nlayer = 0;
  ur.layer = NULL;

  /* Transform the tt_roi_t in succesive layers of a uniform TT*/
  if (uniformize(r,&ur) != NULL) {

    (*ring) = (ring_t*) calloc(ur.nlayer, sizeof(ring_t));

    for (i=0; i<ur.nlayer; ++i) {
      /* Search for the highest energy value. */
      max = get_max_idx(&ur.layer[i]);

      alloc_feature_space(&(*ring)[i], ur.layer[i].calo, ur.layer[i].level);

      /* Evaluate the rings */
      ring_sum_around(&ur.layer[i],&(*ring)[i],max);
    }

    free_uniform_roi(&ur);

    return (ur.nlayer); /* freeing will not erase nlayer! */
  }

  return 0;
}

/* This function only evaluates the maximum energy peak and return the results
   to the caller. */
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

/* This function will allocate space for ring features depending on the type of
   calorimeter we are talking about. This shall limit the number of features
   per layer. Remember that features that would exceed the normal number of
   features are dumped to the last features and that layers that do not fulfill
   the number of features required will have the remaining features set to
   zero. */
void alloc_feature_space(ring_t* rp, const ringcalo_t calo, 
			 const LayerLevel level)
{
  switch(calo) {
  case PS:
    rp->feat = (Energy*) calloc (6,sizeof(Energy));
    rp->nfeat = 6;
    break;

  case EM:
    switch(level) {
    case 1:
      rp->feat = (Energy*) calloc (33,sizeof(Energy));
      rp->nfeat = 33;
      break;

    case 2:
      rp->feat = (Energy*) calloc (10,sizeof(Energy));
      rp->nfeat = 10;
      break;

    case 3:
      rp->feat = (Energy*) calloc (9,sizeof(Energy));
      rp->nfeat = 9;
      break;

    default:
      fprintf(stderr, "(ring)ERROR: EM layer level %d is wrong\n",level);
      exit(EXIT_FAILURE);
    }
    break;

  case HAD:  
    rp->feat = (Energy*) calloc (3,sizeof(Energy));
    rp->nfeat = 3;
    break;

  default:
      fprintf(stderr, "(ring)ERROR: calo %d is wrong\n",calo);
      exit(EXIT_FAILURE);
  }

  return;
}


/* Evaluates the sum around the peak energy, given by the third argument. This
   function shall not allocate the space for the numbers in ring_t, since it is
   the only one who knows have many features to get from each layer. The number
   of rings is limited by the number of allocated positions within the ring. If
   the number of rings is not filled up the remaining positions are filled up
   with zeroes (actually unchanged) if the number of rings if filled up before
   the end of layer processing, the remaining cell energies are added to the
   last ring. */
ring_t* ring_sum_around (const CaloLayer* l, ring_t* ring, const int max)
{
  int x1,x2,y1,y2; /* helpers for placement */
  int nf = 0; 
  
  /* Place the cell with maximum energy at the front */
  put_ring(ring,l->cell[max].energy, nf); /* nf == 0 */
  ++nf;
  
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
    
    /* lock the value of nf to at maximum ring->nfeat */
    if (ring->nfeat > nf) ++nf;

    if (x1 == 0 && x2 == (l->EtaGran-1) && 
	y1 == 0 && y2 == (l->PhiGran-1) ) /* I've evaluated it all! */
      break;

  }

  return (ring);
}

/* This function only inserts an energy value into the ring output given by the
   first argument. The second argument is the energy to add and the third, the
   ring to put it in. This function will *NOT* allocate space for new
   rings. Instead, it will gather energies into the last ring. Placing a cell
   energy where there's already space (nr<f->nfeat), the function will sum the
   values. */
bool_t put_ring(ring_t* r, const Energy e, int nr)
{
  if (r->nfeat > nr) /* I put the energy where it should be */
    r->feat[nr] += e;
  else /* I have to gather into the last cell */
    r->feat[r->nfeat-1] += e;
  return (TRUE);
}

/* This function frees a vector of rings pointed by r and having nring 
   elements. */ 
bool_t free_ring_vector (ring_t* r, const int nring)
{
  int i; /* iterator */
  for (i=0; i<nring; ++i) free_ring(&r[i]);
  return (TRUE);
}

bool_t free_ring (ring_t* r)
{
  free(r->feat);
  return (TRUE);
}

