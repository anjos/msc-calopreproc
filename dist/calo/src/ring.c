/* Hello emacs, this is -*- c -*- */

/* $Id: ring.c,v 1.1 2000/06/28 15:58:40 rabello Exp $ */

#include "ring.h"
#include "ttdef.h"
#include "common.h"
#include "trigtowr.h"

int get_max_idx(const CaloLayer*);
ring_t* ring_sum_around (const CaloLayer*, const int, ring_t*);
uniform_roi_t* uniformize (const CaloTTEMRoI*, uniform_roi_t*);
bool_t free_uniform_roi (uniform_roi_t*);
void uni_eta_line (CaloCell*, const CaloTriggerTower*);
void uni_tt_line(CaloCell*, const CaloTriggerTower*, const int);
bool_t put_ring(ring_t*, const Energy, int);


void print_ring (FILE* fp, const ring_t* rp)
{
  int i;
  for (i = 0; i < rp->nfeat; ++i)
    fprintf(fp, "%e ",rp->feat[i]);
  
  fprintf(fp, "\n");
  return;
}


ring_t* ring_sum (const CaloTTEMRoI* r, ring_t* ring)
{
  int max;
  uniform_roi_t* ur = (uniform_roi_t*) mxalloc(NULL,1,sizeof(uniform_roi_t));
  
  /* Transform the CaloTTEMRoI in succesive layers of a uniform TT*/
  uniformize(r,ur);

  /* Search for the highest energy value. For now working only with one layer
   */ 
  max = get_max_idx(&ur->layer[0]);

  /* Evaluate the rings */
  ring_sum_around(ur->layer,max,ring);
  
  free_uniform_roi(ur);

  return (ring);
}

/* This function uniformizes the roi given by the only argument, generating an
   uniform EM middle layer. It allocates memory for the return variable,
   therefore one should free it after usage.  */
uniform_roi_t* uniformize (const CaloTTEMRoI* r, uniform_roi_t* ur)
{
  int phi;
  int e,p;

  /* Initializes the uniform trigger tower */
  ur->nlayer = 0;
  ur->layer = (CaloLayer*) mxalloc(NULL, 1, sizeof(CaloLayer));
  ur->layer[0].EtaGran = 16;
  ur->layer[0].PhiGran = 16;
  ur->layer[0].NoOfCells = ur->layer[0].EtaGran * ur->layer[0].PhiGran;
  ur->layer[0].calo = EMBARREL; /* Not perfect, but that's one can do...*/
  ur->layer[0].level = 2;
  ur->layer[0].cell = (CaloCell*) mxalloc(NULL, ur->layer[0].NoOfCells,
					  sizeof(CaloCell));

  /* Now, index the newly created cells */
  for (p=0; p < ur->layer[0].PhiGran; ++p)
    for (e=0; e < ur->layer[0].EtaGran; ++e) {
      ur->layer[0].cell[e+ p*ur->layer[0].EtaGran].index.Phi = p;
      ur->layer[0].cell[e+ p*ur->layer[0].EtaGran].index.Eta = e;
    }
  
  /* The procedure here should be similar to the one applied when printing the
     second layer of EM's. */
  for (phi = 0; phi < EMRoIGran; ++phi)  {
    p = 4 * phi * ur->layer[0].EtaGran; /* the initial address of cells */
    uni_eta_line(&ur->layer[0].cell[p], &r->tt[phi][0]);
  }

  ur->nlayer = 1;  

  return(ur);
}



/* This function acts like trigtowr.c::print_eta_line(), but dumping it's
   results to a uniformtt_t*. There's no check for the stability of the address
   given by the caller. This function believes it should start dumping in
   cell[0]. */
void uni_eta_line (CaloCell* c, const CaloTriggerTower* line)
{
  int eta;
  int sphi; /* Tells which subphi to take within the TT */
  int cell; /* iterators */

  for (sphi = 0; sphi < 4; ++sphi)
    for (eta = 0; eta < EMRoIGran; ++eta) {
      if (verify_tt(&line[eta])) /* I can do the right stuff */
	uni_tt_line(&c[eta*4 + sphi*16], &line[eta], sphi);
    
      else /* Well, then I have to print zeros... */
	for (cell = 0; cell < 4; ++cell)
	  c[ cell+ eta*4 + sphi*16 ].energy = 0;
  }

  return;
  
}


/* This function acts like trigtowr.c::print_tt_line(), but dumping it's
   results to a uniformtt_t*. There's no check for the stability of the address
   given by the caller. This function believes it should start dumping in
   cell[0]. */
void uni_tt_line(CaloCell* c, const CaloTriggerTower* tt, const int phi)
{
  int layer_idx = -1;
  int i; /* iterator */

  for (i = 0; i < tt->NoOfLayers; i++)
    if (is_middle(tt->layer[i].calo, tt->layer[i].level)) {
      layer_idx = i;
      break;
    }
  
  for (i = 0; i < 4; ++i)
    c[i].energy = tt->layer[layer_idx].cell[i+4*phi].energy;
  
  return;
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
  /* The simplest solution here is with pointers, so, to those unfamiliar with
     pointers in C: hurry up and learn it, sometimes it can help! */
  CaloCell** c;
  int phi;
  int x1,x2,y1,y2; /* helpers for placement */
  int nf = 0; 
  
  /* Allocates space for these double pointers */
  c = (CaloCell**) mxalloc (NULL, l->PhiGran, sizeof(CaloCell**));

  /* initializes all pointers */
  for (phi=0; phi< l->PhiGran; ++phi) {
    c[phi] = &l->cell[l->EtaGran*phi];
  }
  
  /* Place the cell with maximum energy at the front */
  ring->nfeat = 0;
  put_ring(ring,l->cell[max].energy, nf++);
  
  /* set the helpers in order to start */
  x1 = l->cell[max].index.Eta;
  x2 = x1;
  y1 = l->cell[max].index.Phi;
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
	put_ring(ring,l->cell[e+p*l->PhiGran].energy,nf);

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

/* Free allocated space for uniform trigger towers */
bool_t free_uniform_roi (uniform_roi_t* u)
{
  free_layer(u->layer);
  return (TRUE);
}

bool_t free_ring (ring_t* r)
{
  free(r->feat);
  return (TRUE);
}

