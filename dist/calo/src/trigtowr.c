/* Hello emacs, this is -*- c -*- */
/* copyleft Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: trigtowr.c,v 1.7 2000/07/07 18:48:57 rabello Exp $ */


#include "trigtowr.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h> /* Don't forget to #define _BSD_SOURCE or to gcc
		     -D_BSD_SOURCE for using the constants! */
#include "data.h"
#include "error.h"
#include "common.h"
#include "portable.h"
#include "ttdef.h"

typedef point_t tt_size_t;


/* The definition of some constants used by the module */

/* The granularity of RoIs: how many trigger towers in eta or phi diretion? */
const static double EMRoIGran = EMROIGRAN;
const static double HadRoIGran = HADROIGRAN;

const static int MaxNumberOfLayers = 12;

/* The real size of each trigger tower, I mean in real life */
const static tt_size_t emtt_size = {0.1,0.1};
const static tt_size_t hadtt_size = {0.2,0.2};

/* This pointer should correctly point to the size one needs. The
   simplification added by doing this is questionable, but I didn't want to
   change the functions prototypes at the time being... */
static const tt_size_t* _current_size = &emtt_size;

extern double rint(double x);

bool_t point_belongs(const point_t*, const Area*);
int get_idx(const CellInfo*, const int, const int, const point_t*);
bool_t is_over_the_wrap(const float*, const float*);

ErrorCode create_calo_layer(CaloTriggerTower*, const CellInfo*);
ErrorCode init_calolayer(CaloLayer*, const CellInfo*);

bool_t put_cell_in_tt(const CellInfo*, const Energy, CaloTriggerTower*, 
		      const point_t*);

void free_tt(CaloTriggerTower*);

ErrorCode put_em_digis(emCalDigiType*, const int, tt_roi_t*); 
ErrorCode put_had_digis(hadCalDigiType*, const int, tt_roi_t*); 
tt_roi_t* set_roi_borders (tt_roi_t*, const ROIHEAD*);
CellInfo get_info_from_digi (const emCalDigiType*, const bool_t);

/*
  =========================
   FUNCTION IMPLEMENTATION
  =========================
*/

/* Build RoI from layers into trigger towers of type tt_roi_t (3-D). The
   variable pointed by caloroi, has to be *fully* initialized before calling
   this function, otherwise, unexpected results my occur. */
ErrorCode build_roi(const ROI* roi, const bool_t fix_window,
		    tt_roi_t* caloroi)
{
  int x,y;

  /* initializes roi */
  for(x = 0; x < EMRoIGran; x++)
    for(y = 0; y < EMRoIGran; y++) {
      caloroi->em_tt[x][y].NoOfLayers = 0;
      caloroi->em_tt[x][y].layer = NULL;
    }
  for(x = 0; x < HadRoIGran; x++)
    for(y = 0; y < HadRoIGran; y++) {
      caloroi->had_tt[x][y].NoOfLayers = 0;
      caloroi->had_tt[x][y].layer = NULL;
    }
  caloroi->PhiWrap = FALSE;

  /* Do we want to repair wrong window sizes? */
  caloroi->fixed = (fix_window) ? FALSE : TRUE;  

  /* Set RoI borders */
  caloroi = set_roi_borders (caloroi, &roi->header);

  if (put_em_digis(roi->calDigi.emDigi, roi->calDigi.nEmDigi, 
		   caloroi) == CALO_ERROR)
    {
      /* Oops! */
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't put ");
      fprintf(stderr, "**ELECTROMAGNETIC** digis in place\n");
      return(CALO_ERROR);
    }
  
  if (put_had_digis(roi->calDigi.hadDigi, 
		    roi->calDigi.nhadDigi, caloroi) == CALO_ERROR) 
    {
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't put ");
      fprintf(stderr, "**HADRONIC** digis in place\n"); 
      return(CALO_ERROR); 
    } 

  return(CALO_SUCCESS);
  
} /* end BuildCaloTTS */

/* This function takes the RoI we are working with and sets it's parameters,
   taking into account its size and wheter it's lying over the phi wrap
   region. The parameters that are set are the RoI size and the RoI corner
   variables over the.eta x Phi plane. */
tt_roi_t* set_roi_borders (tt_roi_t* roi, const ROIHEAD* original_header)
{
  /* copies correlated variables */
  roi->Region.UpperRight.eta = original_header->EtaMax;
  roi->Region.LowerLeft.eta  = original_header->EtaMin;
  roi->Region.UpperRight.phi = original_header->PhiMax;
  roi->Region.LowerLeft.phi  = original_header->PhiMin;

  /* For the maximum and minimum values of eta, no problem, since they should
     point correctly to the lower left and upper right corners, since this is
     linear.

     The problem arrives with phi. Since it wraps in 2*PI, it's a bit more
     difficult to set which one belongs to the lower left corner and the one
     that belongs to the upper right corner. 

     There's a figure at <root>/doc/fig/phiwrap.fig that discuss this problem
     and a simple solution, that is applied here. */

  /* check whether we are talking about the phi wrap around area */
   if ( is_over_the_wrap(&original_header->PhiMax, &original_header->PhiMin) )
     {
       roi->PhiWrap = TRUE;

       if (original_header->PhiMax > original_header->PhiMin) {
	 roi->Region.UpperRight.phi = original_header->PhiMin+ 2*M_PI;
	 roi->Region.LowerLeft.phi  = original_header->PhiMax;
       }
       else {
	 roi->Region.UpperRight.phi = original_header->PhiMax+ 2*M_PI;
	 roi->Region.LowerLeft.phi  = original_header->PhiMin;
       }

     }

  /* provisional trick to use dumped files with wrong RoI window */
  if (roi->fixed == FALSE) {
    if( (roi->Region.LowerLeft.phi -= 0.1) < 0. ) 
      roi->Region.LowerLeft.phi += 2 * M_PI; 
    if( (roi->Region.UpperRight.phi += 0.1) > 2 * M_PI ) /* M_PI and others are
							    declared in math.h
							    for the _BSD_SOURCE
							    defined codes */
      roi->Region.UpperRight.phi -= 2 * M_PI; 
    roi->Region.LowerLeft.eta -= 0.1;
    roi->Region.UpperRight.eta += 0.1;
    roi->fixed = TRUE;
  }

  return roi;
}

/* Tests if the RoI is over the phi wrap only */
bool_t is_over_the_wrap(const float* max, const float* min)  
{
  const double max_phi_window = 2.0;

  if ( fabs(*max - *min) > max_phi_window)
    return(TRUE);

  return(FALSE);
}


/* Puts the EM digis into the trigger tower, hopefully correctly.  This
   function has provisions to deal with decoding errors from DecodeId. */
ErrorCode put_em_digis(emCalDigiType* digi, const int ndigi, tt_roi_t* roi)
{
  int counter;

  _current_size = &emtt_size; /* sets the correct global */

  /* finds out cells and layers associated to each TT */
  if (ndigi == 0) return (CALO_SUCCESS);
  for(counter = 0; counter < ndigi; ++counter) {

    CellInfo cell;
    bool_t fit = FALSE;
    int eta, phi; /* iterators */
    Area area; /* lower left and upper right */

    /* the next call mask what we really do here (see put_em_digis.fig) */
    cell = get_info_from_digi(&digi[counter], roi->PhiWrap);

    /* cell center is taken into account */
    phi = 0;
    eta = 0;

    /* Let's test the value of 'fit' and if the cell was in the layer, we can
       just go to the other cell */
    while (TRUE) { /* FOREVER */

      /* define the limits of this RoI */
      area.LowerLeft.phi = roi->Region.LowerLeft.phi + phi * emtt_size.phi;
      area.LowerLeft.eta = roi->Region.LowerLeft.eta + eta * emtt_size.eta;
      area.UpperRight.phi = area.LowerLeft.phi + emtt_size.phi;
      area.UpperRight.eta = area.LowerLeft.eta + emtt_size.eta;

      /* Now, I've got to correct the cell center value, if it falls between
	 zero and phimax, IF the RoI is over the phi wrap region */
      if (roi->PhiWrap && cell.center.phi < 2.0)
	cell.center.phi += 2*M_PI;
      
      /* Does this digi belongs to this Trigger Tower ? */
      if ( point_belongs(&(cell.center), &area) == TRUE ) {
	
	/* If so, put cell in trigger tower */
	fit = put_cell_in_tt( &cell, digi[counter].Et,
			      &(roi->em_tt[phi][eta]),
			      &area.LowerLeft);
      }

      /* If I got it, then I have to jump out of here! */
      if (fit) break;

      /* Update the iterator variables eta and phi. eta is the variable that
	 runs faster, therefore it's update all the time. */
      ++eta;

      /* Test if we have to change the value of phi and load zero at eta */
      if (eta == EMRoIGran) {
	++phi;
	eta = 0;
      } /* ... and continue the loop */

      /* In the case phi has got it's maximum value, than we can no longer
	 continue and are obliged to issue an error message. */
      if (phi == EMRoIGran) {
	fprintf(stderr, "ERROR(trigtowr.c): cell %d is out of RoI scope\n",
		counter);
	return(CALO_ERROR);
      }
      
    } /* end loop over tt's */

  } /* loop over cells */

  /* Uncomment out these lines if you'd like to have the number of cells
     printed to stdout each time the program passes by this point
    fprintf(stderr, "(trigtowr.c): just put %d ", counter);
    fprintf(stderr, "cells into place.\n");
  */

  /* Well, if we get here, then it's diserved! */
  return(CALO_SUCCESS);
  
} /* end of put_em_digis() */

/* Puts the HAD digis into the trigger tower, hopefully correctly. This
   function has provisions to deal with decoding errors from DecodeId. */
ErrorCode put_had_digis(hadCalDigiType* digi, const int ndigi, tt_roi_t* roi)
{
  int counter;

  _current_size = &hadtt_size; /* sets the correct global */

  /* finds out cells and layers associated to each TT */
  if (ndigi == 0) return (CALO_SUCCESS);
  for(counter = 0; counter < ndigi; ++counter) {

    CellInfo cell;
    bool_t fit = FALSE;
    int eta, phi; /* iterators */
    Area area; /* lower left and upper right */

    /* the next call mask what we really do here (see put_em_digis.fig) */
    cell = get_info_from_digi((emCalDigiType*)&digi[counter], roi->PhiWrap);

    /* cell center is taken into account */
    phi = 0;
    eta = 0;

    /* Let's test the value of 'fit' and if the cell was in the layer, we can
       just go to the other cell */
    while (TRUE) { /* FOREVER */

      /* define the limits of this RoI */
      area.LowerLeft.phi = roi->Region.LowerLeft.phi + phi * hadtt_size.phi;
      area.LowerLeft.eta = roi->Region.LowerLeft.eta + eta * hadtt_size.eta;
      area.UpperRight.phi = area.LowerLeft.phi + hadtt_size.phi;
      area.UpperRight.eta = area.LowerLeft.eta + hadtt_size.eta;

      /* Now, I've got to correct the cell center value, if it falls between
	 zero and phimax, IF the RoI is over the phi wrap region */
      if (roi->PhiWrap && cell.center.phi < 2.0)
	cell.center.phi += 2*M_PI;
      
      /* Does this digi belongs to this Trigger Tower ? */
      if ( point_belongs(&(cell.center), &area) == TRUE ) {
	
	/* If so, put cell in trigger tower */
	fit = put_cell_in_tt( &cell, digi[counter].Et,
			      &(roi->had_tt[phi][eta]),
			      &area.LowerLeft);
      }

      /* If I got it, then I have to jump out of here! */
      if (fit) break;

      /* Update the iterator variables eta and phi. eta is the variable that
	 runs faster, therefore it's update all the time. */
      ++eta;

      /* Test if we have to change the value of phi and load zero at eta */
      if (eta == HadRoIGran) {
	++phi;
	eta = 0;
      } /* ... and continue the loop */

      /* In the case phi has got it's maximum value, than we can no longer
	 continue and are obliged to issue an error message. */
      if (phi == HadRoIGran) {
	fprintf(stderr, "ERROR(trigtowr.c): cell %d is out of RoI scope\n",
		counter);
	return(CALO_ERROR);
      }
      
    } /* end loop over tt's */

  } /* loop over cells */

  /* Uncomment out these lines if you'd like to have the number of cells
     printed to stdout each time the program passes by this point
    fprintf(stderr, "(trigtowr.c): just put %d ", counter);
    fprintf(stderr, "cells into place.\n");
  */

  /* Well, if we get here, then it's diserved! */
  return(CALO_SUCCESS);
  
} /* end of put_had_digis() */



/* If the cell fall into this TT region, then this function will put the cell
   energy into the appropriate layer, if possible. Errors concerning layer
   creaction shall be reported if occur.  This function receives the current
   cell, it's energy and the trigger tower to insert the cell together with
   it's lower left corner. */
bool_t put_cell_in_tt(const CellInfo* c, const Energy ce, CaloTriggerTower* tt,
		      const point_t* tt_lower_left)
{
  bool_t layer_exist = FALSE;
  int layer_idx;
  int inlayer_linear_idx;
  int i; /* iterator */

  /* Tests if layer was already initialized before */
  for (i = 0; i < tt->NoOfLayers; i++)
    
    if (c->calo == tt->layer[i].calo &&
	c->region == tt->layer[i].level) {
      layer_exist = TRUE;
      layer_idx = i;
      break;
    }

  /* If not, then create this layer */
  if (layer_exist == FALSE) {
    if ( create_calo_layer(tt, c) == CALO_ERROR ) {
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't create layer.\n");
      return(FALSE);
    }
    layer_idx = tt->NoOfLayers - 1;
  }

  /* Now, get the cell place within the current layer */
  if ( (inlayer_linear_idx = get_idx(c, tt->layer[i].EtaGran, 
				     tt->layer[i].PhiGran, 
				     tt_lower_left)) < 0 ) {
    fprintf(stderr, "ERROR(trigtowr.c): No index for cell.\n");
    return(FALSE);
  }
  
  tt->layer[i].cell[inlayer_linear_idx].energy = ce;
  return TRUE;
}

/* Given the left lower corner of the RoI p (the last argument), this functions
   returns the linear index of the cell, taking into account the granularity of
   this layer, given by the second and the third arguments. The index of a cell
   is extracted by finding the position of this particular cell on the TT and
   transforming this 2-D coordinate into 1-D info by counting forward from the
   botom left cell till the top right going from left to right, beginning at
   zero.

   Note here, that I consider eta to be in the x direction (horizontal) while,
   phi to be in the y direction (vertical). */
int get_idx(const CellInfo* cell, const int etagran, const int phigran, const
	     point_t* p)
{
  const double phistep = _current_size->phi / (double)phigran;
  const double etastep = _current_size->eta / (double)etagran;
  int x, y;

  for(y = 0; y < phigran; ++y)
    for(x = 0; x < etagran; ++x) { 

      /* loops over all possible positions */

      if( cell->center.phi > (p->phi + y * phistep ) && 
	  cell->center.phi < (p->phi + (y+1) * phistep ) && 
	  cell->center.eta > (p->eta + x * etastep ) &&
	  cell->center.eta < (p->eta + (x+1) * etastep) )   

	  return( x + etagran * y );

    }

  /* Oops! */
  return(-1);

}

/* This function masks the usage of GetCellInfo. It also, for the time being,
   implements a way to bypass the errors on calo digi identification numbers
   (UCN) */
CellInfo get_info_from_digi (const emCalDigiType* d, const bool_t pw)
{
  CellInfo cell;
  div_t result;

  if ( GetCellInfo( d->id, &cell, pw) == CALO_ERROR) {
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't get cell info.\n"); 
    exit(CALO_ERROR); 
  } 

  /* This is a special trick that should NOT be used regurlarly. It replaces
     the eta that is found to be wrong. Phi decoding is fine, but eta
     decoding presents some inconsistencies. Those are not a problem of this
     library, but mistakes in the way files are dumped into ASCII format or
     even at the original hbook files. This should be taken out as soon as
     this problem is correct, though it will do no harm, if decoding is fine
  */ 
  result = div(d->CaloRegion, 100); /* Got dizzy? Just say "info libc" at your
				       prompt and go to the mathematics library
				       to find out what this means. */
  cell.calo = result.quot;
  cell.region = result.rem;

  cell.center.eta = d->eta;
  cell.center.phi = d->phi;
  /* ==================
     End of magic trick   
     ================== */

  return cell;
}
	     
/* This function determines whether a generic point belongs or not to a
   particular area. */
bool_t point_belongs(const point_t* p, const Area* a)
{
  if( p->phi > a->LowerLeft.phi && p->phi < a->UpperRight.phi && p->eta >
      a->LowerLeft.eta && p->eta < a->UpperRight.eta)    
    return(TRUE);
  
  return(FALSE);

}

/* Create Non-zero suppressed calo layers */
ErrorCode create_calo_layer(CaloTriggerTower* tt, const CellInfo* cell)
{
  /* update and verify */
  if( ++tt->NoOfLayers > MaxNumberOfLayers) {
    fprintf(stderr, "ERROR(trigtowr.c): ");
    fprintf(stderr, "Max no of layers (%d) exceeded\n",
	    MaxNumberOfLayers);
    return(CALO_ERROR);
  }
  
  /* realloc space */
  tt->layer = (CaloLayer*) realloc(tt->layer,tt->NoOfLayers*sizeof(CaloLayer));
  if ( tt->layer == 0 )  {
    fprintf(stderr, "ERROR(trigtowr.c): No space for layer\n");
    return(CALO_ERROR);
  }

  /* else init */
  if ( init_calolayer(&tt->layer[tt->NoOfLayers-1], cell) == CALO_ERROR ) {
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't init layer\n");
    return(CALO_ERROR);
  }
 
  return(CALO_SUCCESS);
}

ErrorCode init_calolayer(CaloLayer* layer, const CellInfo* cell) 
{
  int x,y;
  const int etagran = (int)rint(_current_size->eta/cell->deta);
  const int phigran = (int)rint(_current_size->phi/cell->dphi);

  layer->EtaGran = etagran;
  layer->PhiGran = phigran;
  layer->level = cell->region;
  layer->calo = cell->calo;
  layer->NoOfCells = etagran * phigran;

  if ((layer->cell = (CaloCell*) calloc (layer->NoOfCells,sizeof(CaloCell))) 
      == NULL) {
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't initialize cells\n");
    return(CALO_ERROR);
  }

  /* Now I should put numbers into cell Indexes. This is easier to use later,
     and for the time being I'm not concerned with time of execution. */
  for (y=0; y<phigran; ++y)
    for (x=0; x<etagran; ++x) {
      layer->cell[x + etagran * y].index.phi = y;
      layer->cell[x + etagran * y].index.eta = x;
      layer->cell[x + etagran * y].energy = 0;
    }

  return(CALO_SUCCESS);
}

void free_roi(tt_roi_t* roi)
{
  int eta, phi;
  
  for(phi = 0; phi < EMRoIGran; ++phi)
    for(eta = 0; eta < EMRoIGran; ++eta)
      free_tt(&roi->em_tt[phi][eta]);
  for(phi = 0; phi < HadRoIGran; ++phi)
    for(eta = 0; eta < HadRoIGran; ++eta)
      free_tt(&roi->had_tt[phi][eta]);
  return;
}

void free_tt(CaloTriggerTower* tt) 
{
  int layer;
  
  for(layer = 0; layer < tt->NoOfLayers; layer++)
    free_layer(&tt->layer[layer]);
  free(tt->layer);
  return;
}

void free_layer(CaloLayer* layer)
{
  free(layer->cell);
  return;
}



