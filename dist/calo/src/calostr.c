/* Hello emacs, this is -*- c -*- */

/* $Id: calostr.c,v 1.3 2000/05/31 12:01:24 rabello Exp $ */

#include "calostr.h"

ErrorCode SplitCells(const ROI* roi, CaloStringRoI* stringroi)
{
  int counter;
  CellInfo cellinfo;

  /* init */
  stringroi->NoOfLayers = 0;
  stringroi->layer = NULL;

  /* copies correlated variables */
  stringroi->region.UpperRight.Phi = roi->header.PhiMax;
  stringroi->region.UpperRight.Eta = roi->header.EtaMax;
  stringroi->region.LowerLeft.Phi = roi->header.PhiMin;
  stringroi->region.LowerLeft.Eta = roi->header.EtaMin;

  /* provisional trick to use dumped files with wrong RoI window */
  if( (stringroi->region.LowerLeft.Phi -= 0.1) < 0. )   
    stringroi->region.LowerLeft.Phi += 2 * PI;   
  if( (stringroi->region.UpperRight.Phi += 0.1) > 2 * PI )   
    stringroi->region.UpperRight.Phi -= 2 * PI;  
  stringroi->region.LowerLeft.Eta -= 0.1;  
  stringroi->region.UpperRight.Eta += 0.1;

  /* check whether we are talking about the phi wrap around area */
  stringroi->PhiWrap = PhiWrap(&(stringroi->region.UpperRight.Phi),
			 &(stringroi->region.LowerLeft.Phi));

  
  for(counter = 0; counter < roi->calDigi.nEmDigi; counter++) {

    StringLayer* current = NULL;
    int i;

    if (GetCellInfo(roi->calDigi.emDigi[counter].id, &cellinfo, 
		    stringroi->PhiWrap) == CALO_ERROR) {
      fprintf(stderr, "ERROR(calostr.c): Couldn't get cell info\n");
      return(CALO_ERROR);
    }

    for (i = 0; i < stringroi->NoOfLayers; i++)
      if (cellinfo.calo == stringroi->layer[i].calo &&
	  cellinfo.region == stringroi->layer[i].level)
	current = &stringroi->layer[i];

    if (current == NULL) {
      StringLayerAlloc(stringroi);
      current = &stringroi->layer[stringroi->NoOfLayers-1];
      
      /* init layer */
      current->calo = cellinfo.calo;
      current->level = cellinfo.region;
      current->NoOfCells = 0;
      current->cell = NULL;
    }
    
    /* put things in place */
    StringCellAlloc(current);
    CopyCellToString(current, roi->calDigi.emDigi[counter].Et, &cellinfo);
  }

  return(CALO_SUCCESS);
}

void StringLayerAlloc(CaloStringRoI* roi)
{
  if ( (roi->layer = SmartAlloc(roi->layer, (roi->NoOfLayers + 1) *
				sizeof(StringLayer))) != NULL ) { 
    roi->NoOfLayers++;
    return;
  }
  fprintf(stderr, "ERROR(calostr.c): No space for layer\n");
  return;
  
}


void StringCellAlloc(StringLayer* s)
{
  if ( (s->cell = SmartAlloc(s->cell, (s->NoOfCells + 1) * sizeof(StringCell)))
       != NULL ) { 
    s->NoOfCells++;
    return;
  }
  fprintf(stderr, "ERROR(calostr.c): No space for cell\n");
  return;
}

void CopyCellToString(StringLayer* s, const double energy, const CellInfo*
		      info) 
{
  s->cell[s->NoOfCells-1].energy = energy;
  s->cell[s->NoOfCells-1].EtaCenter = info->center.Eta;
  s->cell[s->NoOfCells-1].PhiCenter = info->center.Phi;
  return;
}

int BackTranslateEMCoord(const double pos, const double min)
{
  int i;
  
  for(i = 0; i < 16; i++)
    if ( pos > ( (i * 0.025) + min ) && pos < ( ((i+1) * 0.025) + min) )
      return(i);
  
  fprintf(stderr, "ERROR(calostr.c): Couldn't get index for real position\n");
  return(-1);
}

void LayerGravity(const StringLayer* s, const bool_t wrap, Point* p)
{
  int i;
  double EnergySum = 0.;
  double phiadj;
  
  p->Eta = 0.;
  p->Phi = 0.;
  
  for(i = 0; i < s->NoOfCells; i++) {
    p->Eta += s->cell[i].energy * s->cell[i].EtaCenter;

    /* watch out, phi may wrap ! */
    if(wrap == TRUE && s->cell[i].PhiCenter < 2.0) phiadj = s->cell[i].PhiCenter
						   + 2 * PI;
    else phiadj = s->cell[i].PhiCenter;
    p->Phi += s->cell[i].energy * phiadj;
    EnergySum += s->cell[i].energy;
  }
  
  if (EnergySum == 0.) {
    fprintf(stderr, "ERROR(calostr.c): Total Energy is zero\n"); 
    return;
  }
  
  /* readjust phi, if necessary */
  p->Eta /= EnergySum;
  if ( (p->Phi /= EnergySum) > (2 * PI) ) p->Phi -= 2 * PI;
  
  return;
}


void MakeWindow(const Point* p, const double etasz, const double phisz,
		Window* w)
{
  w->EtaMin = p->Eta - etasz / 2.0;
  w->EtaMax = p->Eta + etasz / 2.0;
  
  w->PhiMin = p->Phi - phisz / 2.0;
  w->PhiMax = p->Phi + phisz / 2.0;

  w->PhiWrap = WindowPhiCorrect(&w->PhiMax, &w->PhiMin);

  return;
}


double TranslateEMCoord(const int index, const double min)
{
  return(min+ ( 0.025 * (double)index ) - 0.0125);
}

Energy AddCellsInWindow(const StringLayer* s, const Window* w) 
{
  int i;
  double adjphi;
  Energy energy = 0.;
  
  for(i = 0; i < s->NoOfCells; i++) {
    if(w->PhiWrap == TRUE) {
      adjphi = s->cell[i].PhiCenter;
      if (adjphi < 2.0) adjphi += 2. * PI;
    }
    else
      adjphi = s->cell[i].PhiCenter;
    
    if(s->cell[i].EtaCenter > w->EtaMin && s->cell[i].EtaCenter < w->EtaMax && 
       adjphi > w->PhiMin && adjphi < w->PhiMax)
      energy += s->cell[i].energy;
  }

  return(energy);
}

Energy AddCells(const StringLayer* s)
{
  int i;
  Energy energy = 0.;
  
  for(i = 0; i < s->NoOfCells; i++) {
    energy += s->cell[i].energy;
  }

  return(energy);
}

bool_t WindowPhiCorrect(double *PhiMax, double*PhiMin)
{
    if (*PhiMin < 0.) 
      {
	*PhiMin += 2. * PI;
	*PhiMax += 2. * PI;
	return(TRUE);
      }

    if (*PhiMax > 2. * PI) return(TRUE);
    
    if (*PhiMax - *PhiMin > 2.0) /* Oulala, this is wrong */
      {
	double aux;
	aux = *PhiMin;
	*PhiMin = *PhiMax;
	*PhiMax = aux + 2. * PI;
	return(TRUE);
      }

  return(FALSE);
}

extern void FreeCaloStrings(CaloStringRoI* roi)
{
  int i;
  
  for (i = 0; i < roi->NoOfLayers; i++) free(roi->layer[i].cell);
  free(roi->layer);
  return;
}
