/* Hello emacs, this is -*- c -*- */
/* copyleft Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: trigtowr.c,v 1.6 2000/06/28 15:58:18 rabello Exp $ */


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

extern double rint(double x);

bool_t point_belongs(const Point*, const Area*);
int get_idx(const CellInfo*, const int, const int, const Point*);
bool_t is_over_the_wrap(const float*, const float*);

ErrorCode create_calo_layer(CaloTriggerTower*, const CellInfo*);
ErrorCode init_calolayer(CaloLayer*, const CellInfo*);
CaloCell* create_calocells(CaloCell*, const int);

bool_t put_cell_in_tt(const CellInfo*, const Energy, CaloTriggerTower*, 
		      const Point*);

void free_tt(CaloTriggerTower*);

ErrorCode put_em_digis(emCalDigiType*, const int, CaloTTEMRoI*); 
CaloTTEMRoI* set_roi_borders (CaloTTEMRoI*, const ROIHEAD*);
CellInfo get_info_from_digi (const emCalDigiType*, const bool_t);

int verify_roi(const CaloTTEMRoI*);
void print_eta_line(FILE*, const CaloTriggerTower*);
void print_tt_line(FILE*, const CaloTriggerTower*, const int);

/*
  =========================
   FUNCTION IMPLEMENTATION
  =========================
*/

/* Build RoI from layers into trigger towers of type CaloTTEMRoI (3-D). The
   variable pointed by caloroi, has to be *fully* initialized before calling
   this function, otherwise, unexpected results my occur. */
ErrorCode build_roi(const ROI* roi, const bool_t fix_window,
		    CaloTTEMRoI* caloroi)
{
  int x,y;

  /* initializes roi */
  for(x = 0; x < 4; x++)
    for(y = 0; y < 4; y++) {
      caloroi->tt[x][y].NoOfLayers = 0;
      caloroi->tt[x][y].layer = NULL;
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
  
  /* doing a cast, emdigi == haddigi for the moment - 6.5.98
     This actually doesn't work because there are cases where the the cells
     have a .2x.2 granularity and procedures are *NOT* prepared for that yet.
  if (put_em_digis((emCalDigiType*)roi->calDigi.hadDigi, 
		   roi->calDigi.nhadDigi, caloroi) == CALO_ERROR) 
    {  
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't put ");
      fprintf(stderr, "**HADRONIC** digis in place\n"); 
      return(CALO_ERROR); 
    } 
  */

  return(CALO_SUCCESS);
  
} /* end BuildCaloTTS */

/* This function takes the RoI we are working with and sets it's parameters,
   taking into account its size and wheter it's lying over the phi wrap
   region. The parameters that are set are the RoI size and the RoI corner
   variables over the Eta x Phi plane. */
CaloTTEMRoI* set_roi_borders (CaloTTEMRoI* roi, const ROIHEAD* original_header)
{
  /* copies correlated variables */
  roi->Region.UpperRight.Eta = original_header->EtaMax;
  roi->Region.LowerLeft.Eta  = original_header->EtaMin;
  roi->Region.UpperRight.Phi = original_header->PhiMax;
  roi->Region.LowerLeft.Phi  = original_header->PhiMin;

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
	 roi->Region.UpperRight.Phi = original_header->PhiMin+ 2*M_PI;
	 roi->Region.LowerLeft.Phi  = original_header->PhiMax;
       }
       else {
	 roi->Region.UpperRight.Phi = original_header->PhiMax+ 2*M_PI;
	 roi->Region.LowerLeft.Phi  = original_header->PhiMin;
       }

     }

  /* provisional trick to use dumped files with wrong RoI window */
  if (roi->fixed == FALSE) {
    if( (roi->Region.LowerLeft.Phi -= 0.1) < 0. ) 
      roi->Region.LowerLeft.Phi += 2 * M_PI; 
    if( (roi->Region.UpperRight.Phi += 0.1) > 2 * M_PI ) /* M_PI and others are
							    declared in math.h
							    for the _BSD_SOURCE
							    defined codes */
      roi->Region.UpperRight.Phi -= 2 * M_PI; 
    roi->Region.LowerLeft.Eta -= 0.1;
    roi->Region.UpperRight.Eta += 0.1;
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


/*Puts the EM digis into the trigger tower, hopefully correctly. This
  functions is also used to put the HAD digis into the trigger towers because,
  actually EM and HAD digis are the same for now.  This function has provisions
  to deal with decoding errors from DecodeId. */
ErrorCode put_em_digis(emCalDigiType* digi, const int ndigi, CaloTTEMRoI* roi)
{
  int counter;

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
      area.LowerLeft.Phi = roi->Region.LowerLeft.Phi + phi * PhiTTSize;
      area.LowerLeft.Eta = roi->Region.LowerLeft.Eta + eta * EtaTTSize;
      area.UpperRight.Phi = area.LowerLeft.Phi + PhiTTSize;
      area.UpperRight.Eta = area.LowerLeft.Eta + EtaTTSize;

      /* Now, I've got to correct the cell center value, if it falls between
	 zero and phimax, IF the RoI is over the phi wrap region */
      if (roi->PhiWrap && cell.center.Phi < 2.0)
	cell.center.Phi += 2*M_PI;
      
      /* Does this digi belongs to this Trigger Tower ? */
      if ( point_belongs(&(cell.center), &area) == TRUE )
	
	/* If so, put cell in trigger tower */
	fit = put_cell_in_tt( &cell, digi[counter].Et,
			      &(roi->tt[phi][eta]),
			      &area.LowerLeft);

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



/* If the cell fall into this TT region, then this function will put the cell
   energy into the appropriate layer, if possible. Errors concerning layer
   creaction shall be reported if occur.  This function receives the current
   cell, it's energy and the trigger tower to insert the cell together with
   it's lower left corner. */
bool_t put_cell_in_tt(const CellInfo* c, const Energy ce, CaloTriggerTower* tt,
		      const Point* tt_lower_left)
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
	     Point* p)
{
  const double phistep = PhiTTSize / (double)phigran;
  const double etastep = EtaTTSize / (double)etagran;
  int x, y;

  for(y = 0; y < phigran; ++y)
    for(x = 0; x < etagran; ++x) { 

      /* loops over all possible positions */

      if( cell->center.Phi > (p->Phi + y * phistep ) && 
	  cell->center.Phi < (p->Phi + (y+1) * phistep ) && 
	  cell->center.Eta > (p->Eta + x * etastep ) &&
	  cell->center.Eta < (p->Eta + (x+1) * etastep) )   

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

  cell.center.Eta = d->eta;
  cell.center.Phi = d->phi;
  /* ==================
     End of magic trick   
     ================== */

  return cell;
}
	     
/* This function determines whether a generic point belongs or not to a
   particular area. */
bool_t point_belongs(const Point* p, const Area* a)
{
  if( p->Phi > a->LowerLeft.Phi && p->Phi < a->UpperRight.Phi && p->Eta >
      a->LowerLeft.Eta && p->Eta < a->UpperRight.Eta)    
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
  
  /* alloc space */
  tt->layer = (CaloLayer*) mxalloc(tt->layer,tt->NoOfLayers,sizeof(CaloLayer));
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
  const int etagran = (int)rint(EtaTTSize/cell->deta);
  const int phigran = (int)rint(PhiTTSize/cell->dphi);

  layer->EtaGran = etagran;
  layer->PhiGran = phigran;
  layer->level = cell->region;
  layer->calo = cell->calo;
  layer->cell = NULL;
  layer->NoOfCells = etagran * phigran;

  if ((layer->cell = create_calocells(layer->cell, layer->NoOfCells)) == NULL){
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't initialize cells\n");
    return(CALO_ERROR);
  }

  /* Now I should put numbers into cell Indexes. This is easier to use later,
     and for the time being I'm not concerned with time of execution. */
  for (y=0; y<phigran; ++y)
    for (x=0; x<etagran; ++x) {
      layer->cell[x + etagran * y].index.Phi = y;
      layer->cell[x + etagran * y].index.Eta = x;
    }

  return(CALO_SUCCESS);
}

CaloCell* create_calocells(CaloCell* cell, const int size)
{

  cell = (CaloCell*) mxalloc (cell, size, sizeof(CaloCell));

  if ( cell  == 0 )
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't alloc cells\n");

  return(cell);
}

/* This function should dump in a only line, the contents of the second layer
   of the electromagnetic calorimeter, using the sequence proposed in the
   put_em_digis()::get_idx() function call. Some time in the near future, one
   could bother about using libspec to write this in an organized way. Now, one
   only needs results. */
bool_t print_roi(FILE* fp, const CaloTTEMRoI* roi)
{
  int phi;
  int present = verify_roi(roi);

  /* check and report, if there are less trigger towers with 2nd. EM layer than
     expected */
  if (present != 16) {
    fprintf(stderr, "(trigtowr) Only %d tts have a 2nd.", present);
    fprintf(stderr, " EM layer.\n");
  }
  
  for (phi = 0; phi < EMRoIGran; ++phi)  print_eta_line(fp, &roi->tt[phi][0]);
  
  fprintf(fp, "\n");

  return(TRUE);

}



/* This function verifies the roi, checking whether trigger towers contain a EM
   middle layer or not. It returns the number of 2nd. EM layers found. */
int verify_roi(const CaloTTEMRoI* roi)
{
  int eta; 
  int phi; /* iterators */
  int present = 0; /* holds the number of tt's that have no 2nd. EM layer */

  /* Is the layer present over all TT's? */
  for (phi = 0; phi < EMRoIGran; ++phi)
    for (eta = 0; eta < EMRoIGran; ++eta)
      if (verify_tt(&roi->tt[phi][eta])) ++present;

  return (present);
	
} /* end of verify_roi() */


bool_t verify_tt(const CaloTriggerTower* tt)
{
  int layer; /* iterator */

  for (layer = 0; layer < tt->NoOfLayers; ++layer)
    if (is_middle(tt->layer[layer].calo, tt->layer[layer].level))
      return (TRUE);  

  return (FALSE);

} /* end of verify_tt() */

/* Print one line of trigger towers for the same value of phi */
void print_eta_line(FILE* fp, const CaloTriggerTower* line)
{
  int eta;
  int sphi; /* Tells which subphi to take within the TT */
  int cell; /* iterators */

  for (sphi = 0; sphi < 4; ++sphi)
    for (eta = 0; eta < EMRoIGran; ++eta) {
      if (verify_tt(&line[eta])) /* I can do the right stuff */
	print_tt_line(fp, &line[eta], sphi);
    
      else /* Well, then I have to print zeros... */
	for (cell = 0; cell < 4; ++cell)
	  fprintf(fp, "%d ", 0);
  }
}

/* Prints the current trigger tower line and returns. This function assumes
   that there can't be 2 middle EM layers over the same trigger tower. */
void print_tt_line(FILE* fp, const CaloTriggerTower* tt, const int phi)
{
  int layer_idx = -1;
  int i; /* iterator */

  for (i = 0; i < tt->NoOfLayers; i++)
    if (is_middle(tt->layer[i].calo, tt->layer[i].level)) {
      layer_idx = i;
      break;
    }
  
  for (i = 0; i < 4; ++i)
    /* The default separator has to be the new line since paw, for instance,
       cannot read too big lines with vec/read */
    fprintf(fp, "%e\n", tt->layer[layer_idx].cell[i+4*phi].energy);
  
}


bool_t is_middle(const Calorimeter c, const int r)
{
  switch ( c ) {
  case EMBARREL:
    if (r == 2) return (TRUE);
    break;

  case EMENDCAP: /* Attention: region 8 is also MIDDLE! */
    if ( (r == 2) | (r == 8) ) return (TRUE);
    break;
      
  default:
    return (FALSE);
  
  }

  return (FALSE); /* just to guarantee things... */
  
}


void free_roi(CaloTTEMRoI* roi)
{
  int eta, phi;
  
  for(phi = 0; phi < EMRoIGran; phi++)
    for(eta = 0; eta < EMRoIGran; eta++)
      free_tt(&roi->tt[phi][eta]);
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


