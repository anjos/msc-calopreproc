/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: uniform.c,v 1.1 2000/07/07 18:50:32 rabello Exp $ */

#include <stdlib.h>
#include <stdio.h>

#include "ttdef.h"
#include "common.h"
#include "uniform.h"
#include "trigtowr.h"

/* This global holds the number of errors due to contour problems. Contourning
   problems are usually related to the lack of uniform granularity of certain
   RoIs. I cannot use such RoI's for ring processing or uniformization. I could
   simply not count the events, but I have to have an estimate of how many
   RoIs per file do not go into the specification for some processing. */
int uniform_contour_err = 0;

/* In order to make things simpler, I should enumerate a new type that holds
   only 2 sub-types of calorimeters. */
typedef enum mycalo_t {PS=PSBARRREL, EM=EMBARREL, HAD=TILECAL} mycalo_t;

/* The next types are defined for the sake of simplicity */
typedef CaloTriggerTower emtt_t[EMROIGRAN][EMROIGRAN];
typedef CaloTriggerTower hadtt_t[HADROIGRAN][HADROIGRAN];

/* As well as other small things */
typedef index_t gran_t;
typedef struct uni_layerinfo_t 
{
  gran_t granularity;
  int nphi_per_tt;
  mycalo_t calorimeter;
  LayerLevel level;
} uni_layerinfo_t;

/* EMROIGRAN and HADROIGRAN are macros defined in common.h */
const static int EMRoIGran = EMROIGRAN;
const static int HadRoIGran = HADROIGRAN;

bool_t uniformize_layer(const tt_roi_t*, uniform_roi_t*, 
			const uni_layerinfo_t*);
bool_t uniformize_em_tt(const emtt_t, CaloLayer*);
bool_t uniformize_had_tt(const hadtt_t, CaloLayer*);

void free_had_tt (hadtt_t);

void print_uniform_layer (FILE*, const CaloLayer*);

int search_layer(const CaloTriggerTower*, const mycalo_t, const LayerLevel);
bool_t is_layer(const CaloLayer*, const mycalo_t, const LayerLevel);

void copy_had_tt(hadtt_t, const hadtt_t);
CaloLayer* copy_layer (CaloLayer*, const CaloLayer*, const size_t);
bool_t gather_tilecal_layers(hadtt_t);
bool_t add_equal_layers(CaloLayer*, CaloLayer*);

void print_uniform_roi (FILE* fp, const uniform_roi_t* rp)
{
  int i;
  
  for (i=0; i< rp->nlayer; ++i)
    print_uniform_layer(fp, &rp->layer[i]);
    
  return;
}


/* This function prints the given layer (2nd. argument). The first argument
   should be a valid FILE*.
 
   The output organization is done using the layer granularity. So, for
   instance, if the layer is 16x16 (phixeta), there will be 16 lines with 16
   numbers each, separated by spaces. If the layer is 10x5, there will be 10
   lines with 5 numbers in each one */
void print_uniform_layer (FILE* fp, const CaloLayer* lp)
{
  int eta,phi;
  int cell_index;
  
  for(phi=0; phi < lp->PhiGran; ++phi) {
    for(eta=0; eta < lp->EtaGran; ++eta) {
      cell_index = eta + phi* lp->EtaGran;
      fprintf(fp, "%e ", lp->cell[cell_index].energy);
    }
    fprintf(fp, "\n");
  }
  
  return;
}


uniform_roi_t* uniformize (const tt_roi_t* r, uniform_roi_t* ur)
{
  uni_layerinfo_t info;

  /* Well, if you kept something here, then goodbye! */
  ur->nlayer = 0;
  ur->layer = NULL;

  /***************/
  /* PRESAMPLER  */
  /***************/
  /* Uniformizes the layer 1 of EM on r -> results to ur. */
  /* The order is eta and phi granulairity, nphi_per_tt, calo and level */
  info.granularity.eta = 16;
  info.granularity.phi = 4;
  info.nphi_per_tt = 1;
  info.calorimeter = PS;
  info.level = 1;
  
  /* If I can't uniformize this RoI, them I have to count 1 more contour
     error and will return 0. */
  if (!uniformize_layer(r,ur,&info)) {
    ++uniform_contour_err;
    return(ur=NULL);
  }

  /***************/
  /* EM - FRONT  */
  /***************/
  /* Uniformizes the layer 1 of EM on r -> results to ur. */
  info.granularity.eta = 128;
  info.granularity.phi = 4;
  info.nphi_per_tt = 1;
  info.calorimeter = EM;
  info.level = 1;
  
  /* If I can't uniformize this RoI, them I have to count 1 more contour
     error and will return 0. */
  if (!uniformize_layer(r,ur,&info)) {
    ++uniform_contour_err;
    return(ur=NULL);
  }

  /***************/
  /* EM - MIDDLE */
  /***************/
  /* Uniformizes the layer 2 of EM on r -> results to ur. */
  info.granularity.eta = 16;
  info.granularity.phi = 16;
  info.nphi_per_tt = 4;
  info.calorimeter = EM;
  info.level = 2;
  
  /* If I can't uniformize this RoI, them I have to count 1 more contour
     error and will return 0. */
  if (!uniformize_layer(r,ur,&info)) {
    ++uniform_contour_err;
    return(ur=NULL);
  }

  /*************/
  /* EM - BACK */
  /*************/
  /* Uniformizes the layer 1 of EM on r -> results to ur. */
  info.granularity.eta = 8;  
  info.granularity.phi = 16;  
  info.nphi_per_tt = 4;
  info.calorimeter = EM;  
  info.level = 3;

  /* If I can't uniformize this RoI, them I have to count 1 more contour
     error and will return 0. */
  if (!uniformize_layer(r,ur,&info)) {
    ++uniform_contour_err;
    return(ur=NULL);
  }

  /***************/
  /* HAD - FRONT */
  /***************/
  /* Uniformizes the layer 1 of HAD on r -> results to ur. */
  info.granularity.eta = 4;  
  info.granularity.phi = 4;  
  info.nphi_per_tt = 1;
  info.calorimeter = HAD;  
  info.level = 1;

  /* If I can't uniformize this RoI, them I have to count 1 more contour
     error and will return 0. */
  if (!uniformize_layer(r,ur,&info)) {
    ++uniform_contour_err;
    return(ur=NULL);
  }

  /****************/
  /* HAD - MIDDLE */
  /****************/
  /* Uniformizes the layer 1 of HAD on r -> results to ur. */
  info.granularity.eta = 4;  
  info.granularity.phi = 4;  
  info.nphi_per_tt = 1;
  info.calorimeter = HAD;  
  info.level = 2;

  /* If I can't uniformize this RoI, them I have to count 1 more contour
     error and will return 0. */
  if (!uniformize_layer(r,ur,&info)) {
    ++uniform_contour_err;
    return(ur=NULL);
  }

  /**************/
  /* HAD - BACK */
  /**************/
  /* Uniformizes the layer 1 of HAD on r -> results to ur. */
  info.granularity.eta = 4;
  info.granularity.phi = 4;  
  info.nphi_per_tt = 1;
  info.calorimeter = HAD;
  info.level = 3;

  /* If I can't uniformize this RoI, them I have to count 1 more contour
     error and will return 0. */
  if (!uniformize_layer(r,ur,&info)) {
    ++uniform_contour_err;
    return(ur=NULL);
  }

  return(ur);
}


/* It should check whether the RoI has an uniform granularity over the proposed
   range and them place the cells apropriately. This function should be a
   private 'back-end' to other public function. */
bool_t uniformize_layer(const tt_roi_t* roi, uniform_roi_t* ur,
			const uni_layerinfo_t* info) 
{
  int e,p;
  const int clay = ur->nlayer; /* current layer, where I'm going to work */

  /* Look: one has to initialize the uniform_roi_t *BEFORE* calling this
     function. Look again: this is pretty generic and it's *not* EM
     dependent. */
  ur->layer = (CaloLayer*) mxalloc(ur->layer, ++ur->nlayer, sizeof(CaloLayer));
  
  /* Initializes the newly created layer */
  ur->layer[clay].EtaGran = info->granularity.eta;
  ur->layer[clay].PhiGran = info->granularity.phi;
  ur->layer[clay].NoOfCells = info->granularity.eta * info->granularity.phi;
  ur->layer[clay].calo = info->calorimeter;
  ur->layer[clay].level = info->level;
  ur->layer[clay].cell = (CaloCell*) mxalloc(NULL, ur->layer[clay].NoOfCells,
					  sizeof(CaloCell));

  /* Now, index the newly created cells. This is generic also. */
  for (p=0; p < ur->layer[clay].PhiGran; ++p)
    for (e=0; e < ur->layer[clay].EtaGran; ++e) {
      ur->layer[clay].cell[e+ p * ur->layer[clay].EtaGran].index.phi = p;
      ur->layer[clay].cell[e+ p * ur->layer[clay].EtaGran].index.eta = e;
    }
  
  /* if I want to uniformize an EM layer I should do */
  if (info->calorimeter == EM || info->calorimeter == PS) {
    if (!uniformize_em_tt(roi->em_tt, &ur->layer[clay])) {
      /* fprintf(stderr,"(uniform.c)ERROR:Can't make this layer uniform\n");*/ 
    
      /* In such case, I have to free everything up, and decrease the number of
	 layers on uniform_roi_t*. */
      free_uniform_roi(ur);
      return (FALSE);
    }
  }
  
  /* if I want to uniformize a HAD layer I should do */
  else
    if (!uniformize_had_tt(roi->had_tt, &ur->layer[clay])) {
      /* fprintf(stderr,"(uniform.c)ERROR:Can't make this layer uniform\n");*/ 
      
      /* In such case, I have to free everything up, and decrease the number of
	 layers on uniform_roi_t*. */
      free_uniform_roi(ur);
      return (FALSE);
    }

  
  return (TRUE);
}

/* Given a roi and a layer to uniformize, this function can do it, verifying
   the RoI integrity for the specific layer and applying the correct algorithm
   for placing cells correctly. */
bool_t uniformize_em_tt(const emtt_t tt, CaloLayer* layer)
{
  int tt_eta; /* The trigger tower position within a layer */
  int tt_phi; /* The trigger tower position within a layer */
  int  cell_eta; /* The cell position within a trigger tower */
  int  cell_phi; /* The cell position within a trigger tower */
  const int maxcell_eta = layer->EtaGran / EMRoIGran; /* Granularity within a
                                                         TT */
  const int maxcell_phi = layer->PhiGran / EMRoIGran; /* Granularity within a
                                                         TT */
  index_t d2_index; /* The 2-D and 1-D indexes respectively */
  int d1_index;


  for(tt_phi = 0; tt_phi < EMRoIGran; ++tt_phi)
    for(tt_eta = 0; tt_eta < EMRoIGran; ++tt_eta) {

      /* What I have to do for each trigger tower */

      /* 1) Find out if the layer is there */
      int layer_idx = -1; /* default is: no layer found == -1 */

      layer_idx = search_layer(&tt[tt_phi][tt_eta], layer->calo, layer->level);

      if (layer_idx == -1) {
	/* fprintf(stderr, "(uniform.c)ERROR: Can't find layer in TT\n"); */
	return (FALSE); /* No such layer */       
      }
	  
      /* 2) If it is, then I can process each cell */
      for(cell_phi = 0; cell_phi < maxcell_phi; ++cell_phi)
	for(cell_eta = 0; cell_eta < maxcell_eta; ++cell_eta) {

	  /* What I have to do for each cell */

	  /* A. Evaluate the position of the cell into the original TT */
	  int ttindex = cell_phi *
	    tt[tt_phi][tt_eta].layer[layer_idx].EtaGran + cell_eta; 
	  
	  /* B. Evaluate the cell linear position into the new uniform layer*/
	  d2_index.eta = tt_eta * maxcell_eta + cell_eta;
	  d2_index.phi = tt_phi * maxcell_phi + cell_phi; 
	  d1_index = d2_index.phi * layer->EtaGran + d2_index.eta;

	  /* C. Finally, copy the energy values */
	  layer->cell[d1_index].energy = 
	    tt[tt_phi][tt_eta].layer[layer_idx].cell[ttindex].energy;
	  
	} /* end for all cells into TT */
      
    } /* end for all TT's into RoI */

  return (TRUE);

} /* end of uniformize_em_tt() */ 

/* Given a roi and a layer to uniformize, this function can do it, verifying
   the RoI integrity for the specific layer and applying the correct algorithm
   for placing cells correctly. */
bool_t uniformize_had_tt(const hadtt_t tt, CaloLayer* layer)
{
  int tt_eta; /* The trigger tower position within a layer */
  int tt_phi; /* The trigger tower position within a layer */
  int  cell_eta; /* The cell position within a trigger tower */
  int  cell_phi; /* The cell position within a trigger tower */
  const int maxcell_eta = layer->EtaGran / HadRoIGran; /* Granularity within a
                                                          TT */
  const int maxcell_phi = layer->PhiGran / HadRoIGran; /* Granularity within a
                                                          TT */
  index_t d2_index; /* The 2-D and 1-D indexes respectively */
  int d1_index;
  hadtt_t mytt;
  
  /* The next 2 instructions should flatten the information on layers 1/4, 2/5
     and 3/6 on the TILECAL. This is important because the energy information
     is splitted among different parts of it and a 1st. layer is not exactly
     defined. See descriptions. The other layers are ignored for the time
     being. */
  copy_had_tt(mytt,tt);
  if (!gather_tilecal_layers(mytt))  return (FALSE);

  for(tt_phi = 0; tt_phi < HadRoIGran; ++tt_phi)
    for(tt_eta = 0; tt_eta < HadRoIGran; ++tt_eta) {

      /* What I have to do for each trigger tower */

      /* 1) Find out if the layer is there */
      int layer_idx = -1; /* default is: no layer found == -1 */

      layer_idx = search_layer(&mytt[tt_phi][tt_eta], 
			       layer->calo, layer->level);

      if (layer_idx == -1) {
	/* fprintf(stderr, "(uniform.c)ERROR: Can't find layer in TT\n"); */
	return (FALSE); /* No such layer */       
      }
	  
      /* 2) If it is, then I can process each cell */
      for(cell_phi = 0; cell_phi < maxcell_phi; ++cell_phi)
	for(cell_eta = 0; cell_eta < maxcell_eta; ++cell_eta) {

	  /* What I have to do for each cell */

	  /* A. Evaluate the position of the cell into the original TT */
	  int ttindex = cell_phi *
	    mytt[tt_phi][tt_eta].layer[layer_idx].EtaGran + cell_eta; 
	  
	  /* B. Evaluate the cell linear position into the new uniform layer*/
	  d2_index.eta = tt_eta * maxcell_eta + cell_eta;
	  d2_index.phi = tt_phi * maxcell_phi + cell_phi; 
	  d1_index = d2_index.phi * layer->EtaGran + d2_index.eta;

	  /* C. Finally, copy the energy values */
	  layer->cell[d1_index].energy = 
	    mytt[tt_phi][tt_eta].layer[layer_idx].cell[ttindex].energy;
	  
	} /* end for all cells into TT */
      
    } /* end for all TT's into RoI */

  /* Before finishing, I have to free what I've allocated */
  free_had_tt (mytt);

  return (TRUE);
} /* end of uniformize_had_tt() */ 

/* Well, it does the obvious stuff. */
void free_had_tt (hadtt_t tt)
{
  int e,p;
  int layer;
  
  for (p=0; p<HadRoIGran; ++p)
    for (e=0; e<HadRoIGran; ++e)
      for(layer = 0; layer < tt[p][e].NoOfLayers; layer++)
	free_layer(&tt[p][e].layer[layer]);
}

/* Given some properties, one can find out if there's a layer at one trigger
   tower, with such characteristics. This function performs that. The last
   argument is expected to be an uniformized CaloLayer*. Note here, that if
   there are two layers with the same characteristics then only the first one
   is taken into account. For that, one has to be prepared previously. */
int search_layer(const CaloTriggerTower* tt, const mycalo_t calo, const
		 LayerLevel level)
{
  int layer_idx = -1;
  int i; /* iterator */

  for (i = 0; i < tt->NoOfLayers; i++)
    if (is_layer(&tt->layer[i], calo, level)) {
      layer_idx = i;
      break;
    }
      
  return (layer_idx);
  
} /* end fo search_layer() */

/* Well, this one does what it is supposed to: verify if this layer is a level
   'level' layer. This is a bit tricky in some cases, so pay attention! It's
   primary use is to verify if a layer from a trigger tower contains some
   features what make them usable for some type of processing. If it's the
   case, it should return TRUE. */
bool_t is_layer(const CaloLayer* l,const mycalo_t calo,const LayerLevel level) 
{
  switch (calo) { /* What kind of calorimeter am I looking for? */

  case PS: /* Presamplers */

    /* Don't pay attention to level */
    if ( ( l->calo == PSBARRREL || l->calo == PSENDCAP ) &&
	 l->NoOfCells == 4 ) return (TRUE);
    else return (FALSE);

  case EM: /* LAr Barrel or EndCap */

    if (l->calo == EMBARREL || l->calo == EMENDCAP) {

      switch (l->level) {
      case 1: case 4: case 5: case 6: case 7:
	if (level == 1 && l->NoOfCells == 32) return (TRUE);
	else return (FALSE);

      case 2: case 8:
	if (level == 2 && l->NoOfCells == 16) return (TRUE);
	else return (FALSE);

      case 3: case 9:
	if (level == 3 && l->NoOfCells == 8) return (TRUE);
	else return (FALSE);
      
      default:
	return (FALSE);
	/* what is that? */
      } /* end switch on EM->level */

    }
    
    else
      return (FALSE);
      
  case HAD: /* Hadronic -> Tile or Hadronic EndCap */

    switch (level) { /* Such calorimeter can have such layers */
    case 1:
      switch (l->calo) {
      case TILECAL:
	if ( (l->level == 1 || l->level == 4 ||
	      l->level == 9) && l->NoOfCells == 4 )
	  return (TRUE);
	else 
	  return (FALSE);

      case HADENDCAP:
	if ( (l->level == 1 && l->NoOfCells == 4) ||
	     (l->level == 2 && l-> NoOfCells == 2) )
	  return (TRUE);
	else 
	  return (FALSE);

      default: /* what is that? */
	return (FALSE);
      }

    case 2:
      switch (l->calo) {
      case TILECAL: /* for now, ignore the plugin (7) */
	if ( (l->level == 2 || l->level == 5 ||
	      l->level == 10) && l->NoOfCells == 4 )
	  return (TRUE);
	else 
	  return (FALSE);
	
      case HADENDCAP:
	if ( (l->level == 3 && l->NoOfCells == 4) ||
	     (l->level == 4 && l-> NoOfCells == 2) )
	  return (TRUE);
	else
	  return (FALSE);

      default: /* what is that? */
	return (FALSE);
      }

    case 3:
      switch (l->calo) {
      case TILECAL:  /* for now, ignore the plugin (8) */
	if ( (l->level == 3 || l->level == 6 ||
	      l->level == 11) && l->NoOfCells == 4)
	  return (TRUE);
	else 
	  return (FALSE);

      case HADENDCAP:
	if ( (l->level == 5 && l->NoOfCells == 4) ||
	     (l->level == 6 && l-> NoOfCells == 2) )
	  return (TRUE);
	else
	  return (FALSE);

      default: /* what is that? */
	return (FALSE);
      }
      
    default:
      return (FALSE);
      /* what is that? */
    } /* end switch on EM->level */
    
  default:

    return (FALSE);
    /* what is that? */

  } /* end switch on calo */

} /* end of is_layer() */


/* Free allocated space for uniform trigger towers */
bool_t free_uniform_roi (uniform_roi_t* u)
{
  int layer;
  
  for(layer = 0; layer < u->nlayer; layer++)
    free_layer(&u->layer[layer]);

  free(u->layer);
  return (TRUE);
}

/* This function copies the information on the first had_tt to the second,
   without, nevertheless, copying the address. Memory shall be allocated if
   necessary, and has to be freed after usage. */
void copy_had_tt(hadtt_t dest, const hadtt_t src)
{
  int p,e; /* iterators */
  
  for(p=0; p<HadRoIGran; ++p)
    for(e=0; e<HadRoIGran; ++e) {
      dest[p][e].NoOfLayers = src[p][e].NoOfLayers;
      dest[p][e].layer = 
	(CaloLayer*) calloc (src[p][e].NoOfLayers, sizeof(CaloLayer) );

      copy_layer(dest[p][e].layer, src[p][e].layer, src[p][e].NoOfLayers);

    }
}

/* Fast copies one layer (src) to another place (dest). The number of layers to
   be copied is also given as an argument. */
CaloLayer* copy_layer (CaloLayer* dest, const CaloLayer* src, const size_t n) 
{
  int layer;

  for(layer=0; layer<n; ++layer) {
    dest[layer].EtaGran = src[layer].EtaGran;
    dest[layer].PhiGran = src[layer].PhiGran;
    dest[layer].calo = src[layer].calo;
    dest[layer].level = src[layer].level;
    dest[layer].NoOfCells = src[layer].NoOfCells;

    dest[layer].cell = (CaloCell*) calloc 
      (src[layer].NoOfCells, sizeof(CaloCell));

    memcpy(dest[layer].cell, src[layer].cell,
	   src[layer].NoOfCells*sizeof(CaloCell));
  }
  
  return (dest);
}

/* Gathers layers which are supposed to occupy the same layer position on the
   Tilecal. This function is smart enough to live with search_layer()
   defficiency of returning the first layer that matches the required depth
   (see discussion above on search_layer(). Actually, the layer returned by
   search_layer() is the one that is going to be used as destination to all
   other layers that have the same depth, but cannot be picked-up by
   search_layer(). This is Ok for tilecal because all layers have the same
   granularity approximately. */
bool_t gather_tilecal_layers(hadtt_t tt)
{
  int i; /* iterator */
  int tt_e,tt_p; /* iterators */
  int layer_idx;
  
  for (tt_p=0; tt_p < HadRoIGran; ++tt_p)
    for (tt_e=0; tt_e < HadRoIGran; ++tt_e)
      for (i=0; i < tt[tt_p][tt_e].NoOfLayers; ++i)
	if (tt[tt_p][tt_e].layer[i].calo == TILECAL) {
	  bool_t sum_ok = TRUE;

	  /* If I've got one of these and and previous layer which is *NOT*
	     those, I have to add them up. */
	  switch (tt[tt_p][tt_e].layer[i].level) {
	    
	  case 1: case 4: case 9:
	    layer_idx = search_layer(&tt[tt_p][tt_e], HAD, 1);
	    if (layer_idx != i)
	      sum_ok = add_equal_layers(&tt[tt_p][tt_e].layer[layer_idx],
					&tt[tt_p][tt_e].layer[i]);
	    break;
		  
	  case 2: case 5: case 7: case 10:
	    layer_idx = search_layer(&tt[tt_p][tt_e], HAD, 2);
	    if (layer_idx != i)
	      sum_ok = add_equal_layers(&tt[tt_p][tt_e].layer[layer_idx],
					&tt[tt_p][tt_e].layer[i]);
	    break;
	  case 3: case 6: case 8: case 11:
	    layer_idx = search_layer(&tt[tt_p][tt_e], HAD, 3);
	    if (layer_idx != i)
	      sum_ok = add_equal_layers(&tt[tt_p][tt_e].layer[layer_idx],
					&tt[tt_p][tt_e].layer[i]);
	    break;
	  }

	  /* If something went wrong I'll warn, but that's it, after that I'll
	     wash my hands. */
	  if (!sum_ok) {
	    /* fprintf(stderr, "(uniform.c)ERROR: Can't add layers\n"); */
	    return (FALSE);
	  }
	  
	}

  return (TRUE);
}

/* This function add two layers, the result goes to the first argument. The
   layers have to have the same granularity. */ 
bool_t add_equal_layers(CaloLayer* dest, CaloLayer* src) 
{
  int c; /* iterator */
  
  if (dest->EtaGran != src->EtaGran || dest->PhiGran != src->PhiGran)
    return (FALSE);
  
  for (c = 0; c < dest->NoOfCells; ++c)
    dest->cell[c].energy += src->cell[c].energy;
  
  return (TRUE);
}

