/* Hello emacs, this is -*- c -*- */

/* $Id: zstt.c,v 1.4 2000/07/07 18:49:07 rabello Exp $ */

#include "zstt.h"

#include <math.h>
extern double rint(double x);

/* The definition of some globals */
const static int MaxNumberOfLayers = 8;
const static double EtaTTSize = 0.1;
const static double PhiTTSize = 0.1;

extern ErrorCode CreateZSCaloLayer(CaloTriggerTower* tt, const CellInfo* cell)
{
  /* update and verify */
  if( ++tt->NoOfLayers > MaxNumberOfLayers) {
    fprintf(stderr, "ERROR(trigtowr.c): ");
    fprintf(stderr, "Max no of layers (%d) exceeded\n",
	    MaxNumberOfLayers);
    return(CALO_ERROR);
  }
  
  /* alloc space */
  if ( (tt->layer = mxalloc(tt->layer, tt->NoOfLayers, sizeof(CaloLayer)))
       == NULL ) 
    {
      fprintf(stderr, "ERROR(trigtowr.c): No space for layer\n");
      return(CALO_ERROR);
    }

  /* init */
  if ( InitZSCaloLayer(&tt->layer[tt->NoOfLayers-1], cell) == CALO_ERROR )
    {
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't init layer\n");
      return(CALO_ERROR);
    }
 
  return(CALO_SUCCESS);

}

extern ErrorCode InitZSCaloLayer(CaloLayer* layer, const CellInfo* cell)
{
  const int etagran = (int)rint(EtaTTSize/cell->deta);
  const int phigran = (int)rint(PhiTTSize/cell->dphi);

  layer->EtaGran = etagran;
  layer->PhiGran = phigran;
  layer->level = cell->region;
  layer->calo = cell->calo;
  layer->cell = NULL;
  layer->NoOfCells = 0; /* updated after PlaceCell() call */

  return(CALO_SUCCESS);
}

extern ErrorCode PlaceZSCell(const Energy energy, const CellInfo* cell, const
			     point_t* p, CaloLayer* layer)
{
  layer->NoOfCells++;

  if ( (layer->cell = mxalloc(layer->cell, layer->NoOfCells,
			      sizeof(CaloCell))) == NULL ) 
    {  
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't alloc cells\n");
      return(CALO_ERROR);
    }


  layer->cell[layer->NoOfCells-1].energy = energy;

  layer->cell[layer->NoOfCells-1].index = GetZSIndex(cell, layer->EtaGran,
						     layer->PhiGran, p);
  
  if ( layer->cell[layer->NoOfCells-1].index.eta < 0 ) {
    fprintf(stderr, "ERROR(trigtowr.c): No index for cell\n");
    return(CALO_ERROR);
  }


  return(CALO_SUCCESS);
}

extern index_t GetZSIndex(const CellInfo* cell, const int etagran, const int
			  phigran, const point_t* p)
{
  const double etastep = EtaTTSize / (double)etagran;
  const double phistep = PhiTTSize / (double)phigran;
  int x, y;
  index_t index;

  for(x = 0; x < phigran; x++)
    for(y = 0; y < etagran; y++) { /* loops over positions */
      if( cell->center.phi > (p->phi + x * phistep ) && cell->center.phi <
	  (p->phi + (x+1) * phistep ) && cell->center.eta >
	  (p->eta + y * etastep ) && cell->center.eta < (p->eta
						 + (y+1) * etastep) )   
	{ /* fits in this cell */
	  index.eta = y;
	  index.phi = x;
	  return( index );
	}
    } /* end loop */

  /* Oops! */
  index.eta = -1;
  index.phi = -1;
  return( index );

} 

