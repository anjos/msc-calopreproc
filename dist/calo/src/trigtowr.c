/* Hello emacs, this is -*- c -*- */
/* copyleft Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: trigtowr.c,v 1.5 2000/06/16 23:10:58 rabello Exp $ */

#include "trigtowr.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "data.h"
#include "error.h"
#include "common.h"
#include "portable.h"
#include "ttdef.h"
#include "zstt.h"

extern double rint(double x);

bool_t PointIsInRegion(const Point*, const Area*);
int GetIndex(const CellInfo*, const int, const int, const Point*);

ErrorCode CreateCaloLayer(CaloTriggerTower*, const CellInfo*);
ErrorCode InitCaloLayer(CaloLayer*, const CellInfo*);
CaloCell* CreateCaloCells(CaloCell*, const int);

ErrorCode PlaceCell(const Energy, const CellInfo*, const Point*, CaloLayer*); 

void FreeCaloTriggerTower(CaloTriggerTower*);
void FreeCaloLayer(CaloLayer* layer);

ErrorCode PutEMDigis(emCalDigiType*, const int, const bool_t, CaloTTEMRoI*); 
CaloTTEMRoI* FixRoIParams (CaloTTEMRoI*);
CellInfo DecodeDigi (const emCalDigiType*, const bool_t);

/*
  =========================
   FUNCTION IMPLEMENTATION
  =========================
*/

/* Build RoI from layers into trigger towers of type CaloTTEMRoI (3-D) */
ErrorCode BuildCaloTTS(const ROI* roi, const bool_t zsup, CaloTTEMRoI* caloroi) 
{
  int x,y;

  /* initialises roi */
  for(x = 0; x < 4; x++)
    for(y = 0; y < 4; y++) {
      caloroi->tt[x][y].NoOfLayers = 0;
      caloroi->tt[x][y].layer = NULL;
    }
  caloroi->PhiWrap = FALSE;
  caloroi->fixed = FALSE;

  /* copies correlated variables */
  caloroi->Region.UpperRight.Phi = roi->header.PhiMax;
  caloroi->Region.UpperRight.Eta = roi->header.EtaMax;
  caloroi->Region.LowerLeft.Phi = roi->header.PhiMin;
  caloroi->Region.LowerLeft.Eta = roi->header.EtaMin;

  if (PutEMDigis(roi->calDigi.emDigi, roi->calDigi.nEmDigi, zsup, caloroi) ==
      CALO_ERROR)
    {
       /* Oops! */
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't put EM digis in place\n");
      return(CALO_ERROR);
    }
  
  /* doing a cast, emdigi == haddigi for the moment - 6.5.98 */
  if (PutEMDigis((emCalDigiType*)roi->calDigi.hadDigi, roi->calDigi.nhadDigi,
		 zsup, caloroi) == CALO_ERROR) { 
    /* Oops! */
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't put HAD digis in place\n");
    return(CALO_ERROR);
  }

  return(CALO_SUCCESS);
  
} /* end BuildCaloEMRoI */

/*
  Puts the EM digis into the trigger tower, hopefully correctly. This
  functions is also used to put the HAD digis into the trigger towers because,
  actually EM and HAD digis are the same. 

  This function has provisions to deal with decoding errors from DecodeId and
  with RoI's that fall into the wraparound regions of phi.
*/
ErrorCode PutEMDigis(emCalDigiType* digi, const int NoOfDigis, 
		     const bool_t zsup, CaloTTEMRoI* roi)
{
  int counter;
  ErrorCode (*ccallay) (CaloTriggerTower*, const CellInfo*);
  ErrorCode (*pcell) (const Energy, const CellInfo*, const Point*, CaloLayer*);

  /* switch to the right functions */
  if ( zsup == TRUE ) {
    ccallay = &CreateZSCaloLayer;
    pcell = &PlaceZSCell;
  }
  else {
    ccallay = &CreateCaloLayer;
    pcell = &PlaceCell;
  }

  /* Fix RoI size and corners */
  roi = FixRoIParams (roi);

  /* finds out cells and layers associated to each TT */
  if (NoOfDigis == 0) return (CALO_SUCCESS);
  for(counter = 0; counter < NoOfDigis; ++counter) {

    CellInfo cell;
    bool_t fit = FALSE;
    int eta, phi;
    Area area; /* lower left and upper right */

    /* the next call mask what we really do here (see PutEMDigis.fig) */
    cell = DecodeDigi(&digi[counter], roi->PhiWrap);

    /* cell center is taken into account */
    for(phi = 0; phi < EMRoIGran; phi++)
      for(eta = 0; eta < EMRoIGran; eta++) {

	/* define the limits of this RoI */
	area.LowerLeft.Phi = roi->Region.LowerLeft.Phi + phi * PhiTTSize;
	area.LowerLeft.Eta = roi->Region.LowerLeft.Eta + eta * EtaTTSize;
	area.UpperRight.Phi = area.LowerLeft.Phi + PhiTTSize;
	area.UpperRight.Eta = area.LowerLeft.Eta + EtaTTSize;
	
	/* Does this digi belongs to this Trigger Tower ? */
	if ( PointIsInRegion(&(cell.center), &area) == TRUE )
	{
	    /* The cell fits into this TT! */

	    bool_t layer_exist = FALSE;
	    int LayerIndex;
	    int i; /* iterator */

	    fit = TRUE;

	    /* Tests if layer was already initialized before */
	    for (i = 0; i < roi->tt[phi][eta].NoOfLayers; i++)

	      if (cell.calo == roi->tt[phi][eta].layer[i].calo &&
		  cell.region == roi->tt[phi][eta].layer[i].level) {
		layer_exist = TRUE;
		LayerIndex = i;
	      }

	    /* If not, then create this layer */
	    if (layer_exist == FALSE) {
	      if ( ccallay(&(roi->tt[phi][eta]), &cell) == CALO_ERROR ) {
		fprintf(stderr, "ERROR(trigtowr.c): Couldn't create layer\n");
		return(CALO_ERROR);
	      }
	      LayerIndex = roi->tt[phi][eta].NoOfLayers - 1;
	    }
	      
	    /* And finally, put the cell into it */
	    if ( pcell( (digi+counter)->Et, &cell, &(area.LowerLeft),
			&roi->tt[phi][eta].layer[LayerIndex]) == CALO_ERROR ) {
	      fprintf(stderr, "ERROR(trigtowr.c): No index for cell\n");
	      return(CALO_ERROR);
	    }
	    
	  } /* fits in tt[phi][eta] */
	  
      } /* end loop over tt's */

    if(fit == FALSE) {
      fprintf(stderr, "ERROR(trigtowr.c): cell %d is out of RoI scope\n",
	      counter);
      return(CALO_ERROR);
    }
  
  } /* loop over cells */
  return(CALO_SUCCESS);
  
} /* end PutDigis */

/* This function takes the RoI we are working with and fix it's parameters,
   taking into account its size and wheter it's lying over the phi wrap
   region. The fixed parameters are the RoI size and the RoI corner variables
   over the Eta x Phi plane.
*/ 
CaloTTEMRoI* FixRoIParams (CaloTTEMRoI* roi)
{
  /* provisional trick to use dumped files with wrong RoI window */
  if (roi->fixed == FALSE) {
    if( (roi->Region.LowerLeft.Phi -= 0.1) < 0. ) 
      roi->Region.LowerLeft.Phi += 2 * PI; 
    if( (roi->Region.UpperRight.Phi += 0.1) > 2 * PI ) 
      roi->Region.UpperRight.Phi -= 2 * PI; 
    roi->Region.LowerLeft.Eta -= 0.1;
    roi->Region.UpperRight.Eta += 0.1;
    roi->fixed = TRUE;

    /* check whether we are talking about the phi wrap around area */
    roi->PhiWrap = PhiWrap(&(roi->Region.UpperRight.Phi),
			   &(roi->Region.LowerLeft.Phi));
  }

  return roi;
}

/* This function masks the usage of GetCellInfo. It also, for the time being,
   implements a way to bypass the errors on calo digi identification numbers
   (UCN) */
CellInfo DecodeDigi (const emCalDigiType* d, const bool_t pw)
{
  CellInfo cell;

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
  if(fcomp(cell.center.Eta, d->eta, 0.0001)== CALO_ERROR ) {
    div_t result;
    result = div(d->CaloRegion, 100);
    cell.calo = result.quot;
    cell.region = result.rem;
    
    cell.center.Eta = d->eta;
    cell.center.Phi = d->phi;
  }
  /* End of magic trick */

  return cell;
}
	     
bool_t PointIsInRegion(const Point* p, const Area* a)
{
  if( p->Phi > a->LowerLeft.Phi && p->Phi < a->UpperRight.Phi && p->Eta >
      a->LowerLeft.Eta && p->Eta < a->UpperRight.Eta)    
    return(TRUE);
  
  return(FALSE);

}

/* Given the left lower corner of the RoI p (the last argument), this functions
   returns the linear index of the cell, taking into account the granularity of
   this layer, given by the second and the third arguments. The index of a cell
   is extracted by finding the position of this particular cell on the TT and
   transforming this 2-D coordinate into 1-D info by counting forward from the
   botom left cell till the top right going from left to right, beginning at
   zero.  

   Note here, that I consider phi to be in the x direction (horizontal) while,
   eta to be in the y direction (vertical). */
int GetIndex(const CellInfo* cell, const int etagran, const int phigran, const
	     Point* p)
{
  const double etastep = EtaTTSize / (double)etagran;
  const double phistep = PhiTTSize / (double)phigran;
  int x, y;

  for(x = 0; x < phigran; x++)
    for(y = 0; y < etagran; y++) { 

      /* loops over positions */

      if( cell->center.Phi > (p->Phi + x * phistep ) && 
	  cell->center.Phi < (p->Phi + (x+1) * phistep ) && 
	  cell->center.Eta > (p->Eta + y * etastep ) &&
	  cell->center.Eta < (p->Eta + (y+1) * etastep) )   

	  return( x + phigran * y );

    }

  /* Oops! */
  return(-1);

}

/* Create Non-zero suppressed calo layers */
ErrorCode CreateCaloLayer(CaloTriggerTower* tt, const CellInfo* cell)
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
  if ( InitCaloLayer(&tt->layer[tt->NoOfLayers-1], cell) == CALO_ERROR ) {
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't init layer\n");
    return(CALO_ERROR);
  }
 
  return(CALO_SUCCESS);
}

ErrorCode InitCaloLayer(CaloLayer* layer, const CellInfo* cell) 
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

  if ((layer->cell = CreateCaloCells(layer->cell, layer->NoOfCells)) == NULL){
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't initialize cells\n");
    return(CALO_ERROR);
  }

  /* Now I should put numbers into cell Indexes. This is easier to use later,
     and for the time being I'm not concerned with time of execution. */
  for (y=0; y<etagran; ++y)
    for (x=0; x<phigran; ++x) {
      layer->cell[x + phigran * y].index.Eta = y;
      layer->cell[x + phigran * y].index.Phi = x;
    }

  return(CALO_SUCCESS);
}

CaloCell* CreateCaloCells(CaloCell* cell, const int size)
{

  cell = (CaloCell*) mxalloc (cell, size, sizeof(CaloCell));

  if ( cell  == 0 )
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't alloc cells\n");

  return(cell);
}

ErrorCode PlaceCell(const Energy energy, const CellInfo* cell, const Point* p,
		    CaloLayer* layer)  
{
  int CellIndex;

  if ( (CellIndex = GetIndex(cell, layer->EtaGran, layer->PhiGran, p)) < 0 ) {
    fprintf(stderr, "ERROR(trigtowr.c): No index for cell\n");
    return(CALO_ERROR);
  }

  layer->cell[CellIndex].energy = energy;

  return(CALO_SUCCESS);
}

void FreeCaloEMRoI(CaloTTEMRoI* roi)
{
  int eta, phi;
  
  for(phi = 0; phi < EMRoIGran; phi++)
    for(eta = 0; eta < EMRoIGran; eta++)
      FreeCaloTriggerTower(&roi->tt[phi][eta]);
  return;
}

void FreeCaloTriggerTower(CaloTriggerTower* tt) 
{
  int layer;
  
  for(layer = 0; layer < tt->NoOfLayers; layer++)
    FreeCaloLayer(&tt->layer[layer]);
  free(tt->layer);
  return;
}

void FreeCaloLayer(CaloLayer* layer)
{
  free(layer->cell);
  return;
}
