#include "flat.h"

/* Flattens trigger towers of EM RoI */
ErrorCode Flatten(const CaloTTEMRoI* roi, FlatEM em, FlatHad had)
{
  const int EMRoIGran = EMROIGRAN;
  unsigned int eta, phi, i;
  CaloType section;
  AddEMFunPtr zsupfun, nzsupfun;
  CaloLayer* lay;

  /* Initialise em and had */
  for(phi = 0; phi < FLATEMGRAN; phi++)
    for(eta = 0; eta < FLATEMGRAN; eta++)
      em[phi][eta] = 0.;
  for(phi = 0; phi < FLATHADGRAN; phi++)
    for(eta = 0; eta < FLATHADGRAN; eta++)
      had[phi][eta] = 0.;
  
  /* flatten RoI info */
  for(phi = 0; phi < EMRoIGran; phi++)
    for(eta = 0; eta < EMRoIGran; eta++)
      for(i = 0; i < roi->tt[phi][eta].NoOfLayers; i++) {

	lay = &roi->tt[phi][eta].layer[i]; /* simplifies reading */

	switch(roi->tt[phi][eta].layer[i].calo) {
	case PSBARRREL:
	case PSENDCAP:
	  section = ELECTROMAGNETIC;
	  zsupfun = &AddPSZS;
	  nzsupfun = &AddPSNZS;
	  break;
	  
	case EMBARREL:
	  section = ELECTROMAGNETIC;
	  zsupfun = &AddEMBarrelZS;
	  nzsupfun = &AddEMBarrelNZS;
	  break;
	  
	case EMENDCAP:
	  section = ELECTROMAGNETIC;
	  zsupfun = &AddEMEndcapZS;
	  nzsupfun = &AddEMEndcapNZS;
	  break;
	  
	case TILECAL:
	case HADENDCAP:
	  section = HADRONIC;
	  break;
	  
	default:
	  fprintf(stderr, "ERROR(flat.c): No such calorimeter\n");
	  return(CALO_ERROR);
	  
	} /* end layer switch */

	/* now do the work */
	switch (section) {
	case ELECTROMAGNETIC:
	  if( AddEMCells(lay, eta, phi, em, nzsupfun, zsupfun) == CALO_ERROR) {
	    fprintf(stderr, "ERROR(flat.c): ");
	    fprintf(stderr, "Can't place EM cells TRUE Flat TT\n"); 
	    return(CALO_ERROR);
	}
	  break;

	case HADRONIC: /* map 1 to 1 always */
	  if ( AddHadCells(lay, eta, phi, had) == CALO_ERROR) { 
	    fprintf(stderr, "ERROR(flat.c): ");
	    fprintf(stderr, "Can't place Had cells TRUE Flat TT\n"); 
	    return(CALO_ERROR);
	  }
	  break;
	}
	
      } /* for all layers in TT */

  return(CALO_SUCCESS);
} /* end Flatten */

ErrorCode AddEMLayer(FlatEM em, const int tteidx, const int ttpidx, CaloLayer*
		     inlay) 
{
  int i;
  int etaoc, phioc;
  int div;
  const int TTGran = 4;
  CaloLayer* l;
  CaloLayer* aux = NULL;

  int eta, phi;
  double nrj;
  int estart, pstart;
  
  div = TTGran / inlay->EtaGran;

  /* get cases where Etagran < TTGran, i.e., there are strips */
  if( div < 1 ) {
    if( (aux = l = CreateProvStripLayer(inlay)) == NULL) {
      fprintf(stderr, "ERROR(flat.c): Couldn't create provisory layer\n");
      return(CALO_ERROR);
    }
  }
  else l = inlay;

  /* Normal processing */
  etaoc = TTGran / l->EtaGran;
  phioc = TTGran / l->PhiGran;
  div = etaoc * phioc;
  
  for(i=0; i< l->NoOfCells; i++) {
    nrj = l->cell[i].energy / div;
    estart = tteidx + l->cell[i].index.Eta * etaoc;
    pstart = ttpidx + l->cell[i].index.Phi * phioc;
    for(phi = pstart; phi < pstart + phioc; phi++)
      for(eta = estart; eta < estart + etaoc; eta++)
	em[phi][eta] = nrj;
  }
  
  /* frees, if used */
  if ( aux != NULL ) free(aux->cell), free(aux);

  return(CALO_SUCCESS);

} /* end AddEMLayer */

CaloLayer* CreateProvStripLayer(const CaloLayer* in)
{

  int i, eta;
  double sum;
  CaloLayer* out;
  const int TTGran = 4;

  /* alloc space for provisory layer */
  if ((out = malloc(sizeof(CaloLayer))) == NULL ) {
    fprintf(stderr, "ERROR(flat.c): No space for provisory layer\n");
    return(NULL);
  }
  out->NoOfCells = 4;
  out->EtaGran = 4;
  out->PhiGran = 1;
  if ((out->cell = malloc(out->NoOfCells * sizeof(CaloCell))) == NULL){
    fprintf(stderr, "ERROR(flat.c): No space for provisory cells\n");
    return(NULL);
  }

  /* init cells */
  for(i = 0; i < out->NoOfCells; i++) {
    out->cell[i].energy = 0.;
    out->cell[i].index.Eta = i;
    out->cell[i].index.Phi = 0;
  }

  /* evaluate cells */
  for(i = 0; i < in->NoOfCells; i++) {
    sum = in->cell[i].energy / 4;
    eta = in->cell[i].index.Eta / (in->EtaGran / TTGran);
    out->cell[eta].energy += sum;
  }

  return(out);

} /* end CreateProvStripLayer */

/* add PS cells into em flat layer */
ErrorCode AddEMCells(const CaloLayer* layer, const int TTEtaIndex, const int
		     TTPhiIndex, FlatEM em, AddEMFunPtr nzsfun, AddEMFunPtr
		     zsfun) 
{
  AddEMFunPtr CurrentFunction;

  /* need to use special zero suppresse routines ? */
  if ( layer->PhiGran * layer->EtaGran == layer->NoOfCells ) 
    CurrentFunction = nzsfun;
  else 
    CurrentFunction = zsfun;

  return( CurrentFunction(layer, TTEtaIndex, TTPhiIndex, em) );
} /* end AddPSCells */

ErrorCode AddPSNZS(const CaloLayer* layer, const int TTEtaIndex, const int
		   TTPhiIndex, FlatEM em) 
{
  int eta, phi, x;
  double sum;
  const int TTGran = 4; 

    for(eta = 4*TTEtaIndex, x=0; eta < 4*TTEtaIndex + TTGran; eta++, x++) {
      sum = layer->cell[x].energy / 4;
      for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	em[phi][eta] += sum;
    }
    return(CALO_SUCCESS);
}

ErrorCode AddPSZS(const CaloLayer* layer, const int TTEtaIndex, const int
		  TTPhiIndex, FlatEM em)
{
  int eta, phi, x;
  double sum;
  const int TTGran = 4;
  
    for(x = 0; x < layer->NoOfCells; x++) {
      sum = layer->cell[x].energy / 4;
      eta = 4 * TTEtaIndex + layer->cell[x].index.Eta;
      for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	em[phi][eta] += sum;
    }
    return(CALO_SUCCESS);
}

ErrorCode AddEMBarrelNZS(const CaloLayer* layer, const int TTEtaIndex, const
			 int TTPhiIndex, FlatEM em)
{
  int eta, phi, x, y, aux;
  int EtaDrop;
  const int TTGran = 4;
  double sum; 

  switch(layer->level) { /* trigger TRUE layer deepness */
      
  case 1: /* Fmont (or Strip) Layer */
    for(eta = 4 * TTEtaIndex, x = 0; eta < 4 * TTEtaIndex + TTGran; eta++,
	  x++) 
      { 
	EtaDrop = (int)rint((double)layer->EtaGran / (double)TTGran); 
	sum = 0.;
	for(aux = 0; aux < EtaDrop; aux++)
	  if(layer->cell[aux + EtaDrop * eta].energy != 0) /* add it up */
	    sum += layer->cell[aux + EtaDrop * eta].energy;
	
	for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	  em[phi][eta] += sum / TTGran;
      }
    break;

  case 2: /* Middle Layer, direct mapping */
    for(phi = 4 * TTPhiIndex, x=0; phi < 4 * TTPhiIndex + TTGran; phi++, x++)
      for(eta = 4 * TTPhiIndex, y=0; eta < 4 * TTPhiIndex + TTGran; eta++,
	    y++) 
	em[phi][eta] += layer->cell[y + layer->EtaGran * x].energy;
    break;
    
  case 3: /* Back Layer */
    for(phi = 4 * TTPhiIndex, x=0; phi < 4 * TTPhiIndex + TTGran; phi++, x++)
      for(eta = 4 * TTPhiIndex, y=0; eta < 4 * TTPhiIndex + 2; eta++, y++){
	em[phi][2 * eta] += layer->cell[y + layer->EtaGran * x].energy / 2.;
	em[phi][2 * eta +1] += em[phi][2 * eta];
      }
    break;
    
  default: /* Oops! */
    fprintf(stderr, "ERROR(calolib.c): No such layer in EM Barrel\n");
    return(CALO_ERROR);
    
  } /* switch */
  return(CALO_SUCCESS);
  
}

ErrorCode AddEMBarrelZS(const CaloLayer* layer, const int TTEtaIndex, const
			 int TTPhiIndex, FlatEM em)
{
  int eta, phi, x, aux;
  int EtaDrop;
  const int TTGran = 4;
  double sum;

  switch(layer->level) { /* trigger TRUE layer deepness */
      
    case 1: /* Fmont (or Strip) Layer */
      for(x = 0; x < layer->NoOfCells; x++) {
	EtaDrop = (int)rint((double)layer->EtaGran / (double)TTGran);
	eta = (int)rint((double)layer->cell[x].index.Eta / (double) EtaDrop);
	eta += TTEtaIndex;
	sum = layer->cell[x].energy / 4;
	for (aux = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	  em[phi][eta] += sum;
      }
      break;

    case 2: /* Middle Layer, direct mapping */
      for(x = 0; x < layer->NoOfCells; x++) {
	eta = 4 * TTEtaIndex + layer->cell[x].index.Eta;
	phi = 4 * TTPhiIndex + layer->cell[x].index.Phi;
	em[phi][eta] += layer->cell[x].energy;
      }
      break;
    
    case 3: /* Back Layer */
      for(x = 0; x < layer->NoOfCells; x++) {
	phi = 4 * TTPhiIndex + layer->cell[x].index.Phi;
	eta = 4 * TTEtaIndex + 2 * layer->cell[x].index.Eta;
	em[phi][eta] = layer->cell[x].energy;
	em[phi][eta+1] = layer->cell[x].energy;
      }
      break;
    
    default: /* Oops! */
      fprintf(stderr, "ERROR(calolib.c): No such layer in EM Barrel\n");
      return(CALO_ERROR);
    
    } /* switch */

  return(CALO_SUCCESS);
}

ErrorCode AddEMEndcapNZS(const CaloLayer* layer, const int TTEtaIndex, const
			 int TTPhiIndex, FlatEM em)  
{
  int eta, phi, aux, x, y;
  int EtaDrop;
  const int TTGran = (int)rint((double)FLATEMGRAN / (double)EMROIGRAN); 
  double sum;
  
  switch(layer->level) {
    
  case 1: /* Fmont (or Strip) Layer */
    switch (layer->EtaGran) {
    case 32:
    case 24:
    case 16:
    case 4:      
	
      for(eta = 4 * TTEtaIndex, x = 0; eta < 4 * TTEtaIndex + TTGran; eta++,
	    x++) 
	{ 
	  EtaDrop = (int)rint((double)layer->EtaGran / (double)TTGran); 
	  sum = 0.;
	  for(aux = 0; aux < EtaDrop; aux++)
	    if(layer->cell[aux + EtaDrop * x].energy != 0) /* add it up */
	      sum += layer->cell[aux + EtaDrop * x].energy;
	
	  for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	    em[phi][eta] += sum / TTGran;
	}
      break;

    case 1:
      sum = layer->cell[0].energy / (double)(TTGran * TTGran);
      for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	for(eta = 4 * TTPhiIndex; eta < 4 * TTPhiIndex + 2; eta++)
	  em[phi][eta] += sum;
      break;
    
    default: /* Oops ! */
      fprintf(stderr, "ERROR(flat.c): ");
      fprintf(stderr, "No such eta granularity in EM Endcap (Layer 1)\n");
      return(CALO_ERROR);
    }
    break;
    
  case 2: /* Middle layer */
    switch (layer->EtaGran) {
    case 4:
      for(phi = 4 * TTPhiIndex, x=0; phi < 4 * TTPhiIndex + TTGran; phi++,
	    x++) 
	for(eta = 4 * TTPhiIndex, y=0; eta < 4 * TTPhiIndex + TTGran; eta++,
	      y++) 
	  em[phi][eta] += layer->cell[y + layer->EtaGran * x].energy;
      break;

    case 1:
      sum = layer->cell[0].energy / (double)(TTGran * TTGran);
      for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	for(eta = 4 * TTPhiIndex; eta < 4 * TTPhiIndex + TTGran; eta++)
	  em[phi][eta] = sum;
      break;

    default:
      fprintf(stderr, "ERROR(flat.c): ");
      fprintf(stderr, "No such eta granularity in EM Endcap (Layer 2)\n");
      return(CALO_ERROR);
    }
    break;

  case 3: /* Back Layer */
    for(phi = 4 * TTPhiIndex, x=0; phi < 4 * TTPhiIndex + TTGran; phi++, x++)
      for(eta = 4 * TTPhiIndex, y=0; eta < 4 * TTPhiIndex + 2; eta++, y++){
	em[phi][2 * eta] += layer->cell[y + layer->EtaGran * x].energy / 2.;
	em[phi][2 * eta +1] += em[phi][2 * eta];
      }
    break;
    
  default:
    fprintf(stderr, "ERROR(flat.c): ");
    fprintf(stderr, "No such layer in EM Endcap >> %d\n", layer->level);
    return(CALO_ERROR);
  } /* switch TRUE layer */

  return(CALO_SUCCESS);
  
}

ErrorCode AddEMEndcapZS(const CaloLayer* layer, const int TTEtaIndex, const
			 int TTPhiIndex, FlatEM em)  
{
  int eta, phi, aux, x;
  int EtaDrop;
  const int TTGran = (int)rint((double)FLATEMGRAN / (double)EMROIGRAN); 
  double sum;
  
  switch(layer->level) {
    
  case 1: /* Fmont (or Strip) Layer */
    switch (layer->EtaGran) {
    case 32:
    case 24:
    case 16:
    case 4:      
      for(x = 0; x < layer->NoOfCells; x++) {
	EtaDrop = (int)rint((double)layer->EtaGran / (double)TTGran); 
	eta = (int)rint((double)layer->cell[x].index.Eta / (double)
			 EtaDrop); 
	eta += TTEtaIndex;
	sum = layer->cell[x].energy / 4;
	for (aux = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	  em[phi][eta] += sum;
      }
      break;

    case 1:
      sum = layer->cell[0].energy / (double)(TTGran * TTGran);
      for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	for(eta = 4 * TTPhiIndex; eta < 4 * TTPhiIndex + 2; eta++)
	  em[phi][eta] += sum;
      break;
    
    default: /* Oops ! */
      fprintf(stderr, "ERROR(calolib.c): ");
      fprintf(stderr, "No such eta granularity in EM Endcap (Layer 1)\n");
      return(CALO_ERROR);
    }
    break;
    
  case 2: /* Middle layer */
    switch (layer->EtaGran) {
    case 4:
      for(x = 0; x < layer->NoOfCells; x++) {
	eta = 4 * TTEtaIndex + layer->cell[x].index.Eta;
	phi = 4 * TTPhiIndex + layer->cell[x].index.Phi;
	em[phi][eta] += layer->cell[x].energy;
      }
      break;
	
    case 1:
      sum = layer->cell[0].energy / (double)(TTGran * TTGran);
      for(phi = 4 * TTPhiIndex; phi < 4 * TTPhiIndex + TTGran; phi++)
	for(eta = 4 * TTPhiIndex; eta < 4 * TTPhiIndex + TTGran; eta++)
	  em[phi][eta] = sum;
      break;

    default:
      fprintf(stderr, "ERROR(calolib.c): ");
      fprintf(stderr, "No such eta granularity in EM Endcap (Layer 2)\n");
      return(CALO_ERROR);
    }
    break;

  case 3: /* Back Layer */
    for(x = 0; x < layer->NoOfCells; x++) {
      phi = 4 * TTPhiIndex + layer->cell[x].index.Phi;
      eta = 4 * TTEtaIndex + 2 * layer->cell[x].index.Eta;
      em[phi][eta] = layer->cell[x].energy;
      em[phi][eta+1] = layer->cell[x].energy;
    }
    break;

  default:
    fprintf(stderr, "ERROR(calolib.c): ");
    fprintf(stderr, "No such layer in EM Endcap\n");
    return(CALO_ERROR);
  } /* switch TRUE layer */

  return(CALO_SUCCESS);
}

ErrorCode AddHadCells(const CaloLayer* layer, const int TTEtaIndex, const int
		      TTPhiIndex, FlatHad had) 
{
  /* easy, 1 to 1 mapping */
  had[TTPhiIndex][TTEtaIndex] = layer->cell[0].energy;
  return(CALO_SUCCESS);
}

Energy ExtractEMAreaEnergy(const FlatEM em, const int etast, const int phist,
			   const int etasz, const int phisz) 
{
  Energy energy = 0.;
  int eta, phi;
  
  for(phi = phist; phi < (phist+phisz); phi++)
    for(eta = etast; eta < (etast+etasz); eta++)
	energy += em[phi][eta];

  return(energy);
}

Energy ExtractHadAreaEnergy(const FlatHad had, const int etast, const int
			    phist, const int etasz, const int phisz)  
{
  Energy energy = 0.;
  int eta, phi;
  
  for(phi = phist; phi < (phist+phisz); phi++)
    for(eta = etast; eta < (etast+etasz); eta++)
      energy += had[phi][eta];

  return(energy);
}

Energy Get3x7EnergyPeak(const FlatEM em, int* eta_peak, int* phi_peak)
{
  Energy peak;
  int eta, phi;
  
  for(phi = 4; phi < 12; phi++)
    for(eta = 4; eta < 12; eta++) {
      Energy sum = 0.;
      sum = ExtractEMAreaEnergy(em, eta-1, phi-3, 3, 7);
      if(sum > peak) {
	peak = sum;
	*eta_peak = eta;
	*phi_peak = phi;
      }
    }
  
  return(peak);
}

void GetEnergyPeak(const FlatEM em, int* eta_peak,
		     int* phi_peak)
{
  Energy hold = 0.;
  int eta, phi;
  
  for(phi = 0; phi < FLATEMGRAN; phi++)
    for(eta = 0; eta < FLATEMGRAN; eta++) {
      if(em[phi][eta] > hold) {
	hold = em[phi][eta];
	*eta_peak = eta;
	*phi_peak = phi;
      }
    }
  
  return;
}

void fprintf_FLATEM(FILE* out, const FlatEM em)
{
  int eta, phi;

  for(eta=0; eta<FLATEMGRAN; ++eta) {
    for(phi=0; phi<FLATEMGRAN; ++phi) fprintf(out, "%e ", em[eta][phi]);
    fprintf(out, "\n");
  }
}


void fprintf_FLATHAD(FILE* out , const FlatHad had)
{
  int eta, phi;

  for(eta=0; eta<FLATHADGRAN; ++eta) {
    for(phi=0; phi<FLATHADGRAN; ++phi) fprintf(out, "%e ", had[eta][phi]);
    fprintf(out, "\n");
  }
}
