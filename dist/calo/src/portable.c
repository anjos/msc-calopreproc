/***************************************************************************
 * Decodification from Universal Cell Number to integers, more info on 
 * http://wwwcn.cern.ch/~simions/Lyon/atlas/pileup/P02
 * for reference, one can use the bit description bellow:
 *
 *            [1098765.4321.098.7.654321098.76543210]
 *             0000000.KKKK.mmm.S.eeeeeeeee.pppppppp
 *
 * 0 - zero; K - calo; m - module; S - side or signal; e - eta and p is phi.
 *                        (region)
 * $Log: portable.c,v $
 * Revision 1.1  2000/03/13 21:03:46  rabello
 * Initial revision
 *
 * Revision 1.1.1.1  1998/07/09 16:09:08  rabello
 * Calorimeter C Library
 *
 * ************************************************************************* */
 
#include "portable.h"

/* These functions can only be seen from inside this file */
/* They just apply patches to the output numbers of GetCellInfo() */

ErrorCode DecodeId(const unsigned int, CellInfo*);
ErrorCode ResolveLayer(CellInfo*);
void CorrectCell(CellInfo*);

ErrorCode CorrectPSBarrel(const unsigned int id, CellInfo*);
ErrorCode CorrectEMBarrel(const unsigned int id, CellInfo*);
ErrorCode CorrectEMEndCap(const unsigned int id, CellInfo*);
ErrorCode CorrectTileCal(const unsigned int id, CellInfo*);
ErrorCode CorrectHadEndCap(const unsigned int id, CellInfo*);
ErrorCode CorrectPSEndCap(const unsigned int id, CellInfo*);

double round(double, int precision);

/* From a cell id, it writes the eta, phi, calorimeter and region
   of the cell */
ErrorCode DecodeId(const unsigned int id, CellInfo* cell) 
{
  /* variable declaration */
  int side;

  /* This pointer will point to the correct function that is going to apply the
     cell correction */
  ErrorCode (*CorrectParam) (const unsigned int id, CellInfo*);
  
  /* default attributions */
  cell->calo = (id >> 21) & 0xf;
  cell->region = (id >> 18) & 0x7;
  side = (id >> 17) & 0x1;

  cell->center.Eta = (id >> 8) & 0x1ff;
  cell->center.Eta *= DETA;

  cell->center.Phi = id & 0xff;
  cell->center.Phi *= DPHI;
  
  /* special cases corrections */
  switch(cell->calo) {

  case 1: /* PreSampler */
    CorrectParam = &CorrectPSBarrel;
    break;

  case 2: /* EM Barrel - no exceptions */
    CorrectParam = &CorrectEMBarrel;
    break;
    
  case 3: /* EM EndCap */
    CorrectParam = &CorrectEMEndCap;
    break;
    
  case 4: /* TileCal */
    CorrectParam = &CorrectTileCal;
    break;
    
  case 5: /* HadEnd */
    CorrectParam = &CorrectHadEndCap;
    break;
    
  case 11: /* PreSampler on the EndCap */
    CorrectParam = &CorrectPSEndCap;
    break;

  default:
    /* Oops! */
    fprintf(stderr, "ERROR(portable.c): No such calorimeter in ATLAS\n");
    return(ERROR);

  } /* switch(calo) */

  if ( CorrectParam(id, cell) == ERROR ){
    /* Oops! */
    fprintf(stderr, "ERROR(portable.c): Couldn't correct cell\n");
    return(ERROR);
  }

  /* correction for cell coordinates (center) */
  cell->center.Eta += cell->deta/2;
  cell->center.Phi += cell->dphi/2;

  /* side/signal test */
  if(side) cell->center.Eta *= -1;

#ifdef ROUND_OUTPUT
  /* correction (rounding) */
  cell->center.Phi = round(cell->center.Phi, RND_PREC);
  cell->center.Eta = round(cell->center.Eta, RND_PREC);
  cell->dphi = round(cell->dphi, RND_PREC);
  cell->deta = round(cell->deta, RND_PREC);
#endif

  return(SUCCESS);

}/* end decoding */

ErrorCode CorrectPSBarrel(const unsigned int id, CellInfo* c)
{
  c->region = 1;
  c->deta = DETA*8;
  c->dphi = DPHI*4;
  return(SUCCESS);
}

ErrorCode CorrectEMBarrel(const unsigned int id, CellInfo* c)
{
  /* In the end of the barrel, 1st layer disappears and 2nd takes
     its place. 3rd layer replaces second */
  switch(c->region) {
  case 1: /* 1st layer */
    if(c->center.Eta < E_14) {
      c->dphi = DPHI*4;
      c->deta = DETA;
    }
    else {
      c->dphi = DPHI;
      c->deta = DETA*8;
    }
    break;
  case 2: /* 2nd layer */
    if(c->center.Eta < E_14) {
      c->deta = DETA*8;
    }
    else {
      c->deta = DETA*16;
    }
    c->dphi = DPHI;
    break;
    
  case 3: /* last layer */
    c->deta = DETA*16;
    c->dphi = DPHI;
    break;
    
  default:
    /* Oops! */
    fprintf(stderr, "ERROR(portable.c): No such region in EMBarrel\n");
    return(ERROR);
  }

  return(SUCCESS);
}

ErrorCode CorrectEMEndCap(const unsigned int id, CellInfo* c)
{
  double maxerr = MAX_ABS_ERROR;

  switch(c->region) {
  case 1:

    c->center.Eta += E_14; /* the start of EM Endcap */

    /* correct Eta in case we're between 1.8 and 2.0 */
    if(c->center.Eta - E_18 > -maxerr && 
       c->center.Eta - E_20 < maxerr) {
      
      c->center.Eta = ((id >> 11) & 0x3f) * DETA*8 + ((id >> 8) & 0x7) 
	* DETA * ((DETAIL)?(8./6.):1) + E_14;
    }

#ifdef DICEOLD
    if(c->center.Eta - E_14 > -maxerr && 
       c->center.Eta - E_15 < maxerr) { 
      c->region = 7;
      c->deta = DETA*8;
      if (c->center.Eta < 1.42) c->center.Eta -= 0.025; /* correct a problem I
							    can't describe.
							    This was fixed in
							    29.02.2000 
							 */ 
    }

    if(c->center.Eta - E_15 > -maxerr && 
       c->center.Eta - E_18 < maxerr) { 
      c->region = 6;
      c->deta = DETA;
    }

    if(c->center.Eta - E_18 > -maxerr && 
       c->center.Eta - E_20 < maxerr) { 
      c->region = 5;
      c->deta = DETA*8/6;
    }

    if(c->center.Eta - E_20 > -maxerr && 
       c->center.Eta - E_25 < maxerr) {
      c->region = 4;
      c->deta = DETA*2;
    }

    if(c->center.Eta - E_25 > -maxerr && 
       c->center.Eta - E_32 < maxerr) {
      c->region = 1;
      c->deta = DETA*32;
    }
      
#else
    if(c->center.Eta - E_14 > -maxerr && 
       c->center.Eta - E_15 < maxerr) {
       c->region = 1;
      c->deta = DETA*8;
    }

    if(c->center.Eta - E_15 > -maxerr && 
       c->center.Eta - E_18 < maxerr) {
      c->region = 4;
      c->deta = DETA;
    }

    if(c->center.Eta - E_18 > -maxerr &&
       c->center.Eta - E_20 < maxerr) {
      c->region = 5;
      c->deta = DETA*8/6;
    }

    if(c->center.Eta - E_20 > -maxerr && 
       c->center.Eta - E_24 < maxerr) {
      c->region = 6;
      c->deta = DETA*2;
    }

    if(c->center.Eta - E_24 > -maxerr) {
      c->region = 7;
      c->deta = DETA*8;
    }

#endif

    c->dphi = DPHI*4;
    break;

  case 2:
    c->region = 8;
    c->center.Eta += E_14;
    c->deta = DETA*8;
    c->dphi = DPHI;
    break;
      
  case 3:
    c->region = 9;
    c->center.Eta += E_14 + 0.0125; /* the 0.0125 term
				       was added (3.apr.98) to correct
				       some differences which I can't 
				       explain now... */
    c->deta = DETA*16;
    c->dphi = DPHI;
    break;
      
  case 5:
    c->region = 1;
    c->center.Eta += E_25;
    c->deta = DETA*32;
    c->dphi = DPHI*4;
    break;
      
  case 6:
    c->region = 2;
    c->center.Eta += E_25;
    c->deta = DETA*32;
    c->dphi = DPHI*4;
    break;

  default:
    /* Oops! */
    fprintf(stderr, "ERROR(portable.c): No such region in EM EndCap\n");
    return(ERROR);

  } /* region for BarEnd */
 
  return(SUCCESS);
}

ErrorCode CorrectTileCal(const unsigned int id, CellInfo* c)
{
  switch(c->region) {
  case 0: /* scintillator (9, 10, 11) */
    c->region = ((id >> 14) & 0x7) + 4;
    c->deta = DETA*32;
    break;
      
  case 1:
    c->deta = DETA*32;
    break;
      
  case 2:
    if(((id >> 8) & 0x1ff) == 9) c->region = 7;
    c->deta = DETA*32;
    break;

  case 3:
    if(((id >> 8) & 0x1ff) == 8) c->region = 8;
    c->deta = DETA*64;
    break;


  case 5: /* ExtBar */
  case 6:
    c->region -= 1;
    c->deta = DETA*32;
    break;
      
  case 7:
    c->region -= 1;
    c->deta = DETA*64; /* approximative */
    break;

  default:
    /* Oops! */
    fprintf(stderr, "ERROR(portable.c): No such region in TileCal\n");
    return(ERROR);

  }/* end of TileCal */

  c->dphi = DPHI*4;

  return(SUCCESS);
}

ErrorCode CorrectHadEndCap(const unsigned int id, CellInfo* c)
{
  c->center.Eta *= 2; /* correction, no space for bits in correct order, so 
			    they had to occupy the previous house (0.05) as
			    well the price to pay is a multiplication */
  c->center.Eta += E_15;
  switch(c->region) {
  case 1:
    break;
  case 2:
  case 3:
    c->region = 3;
    break;
  default:
    c->region = 5;
  }
  if(c->center.Eta >= E_25) c->region++;
    
  switch(c->region) {
  case 2: /* eta > 2.5 && eta < 3.2 */
  case 4: 
  case 6:
    c->dphi = DPHI*8;
    if(c->center.Eta >= E_25 && c->center.Eta < E_31) c->deta =
							      DETA*64; 
    else c->deta = DETA*32;
    break;
  default:
    c->dphi = DPHI*4;
    c->deta = DETA*32;
  }

  return(SUCCESS);
}

ErrorCode CorrectPSEndCap(const unsigned int id, CellInfo* c)
{
  c->center.Eta += E_14;
  c->region = 1;
  c->deta = DETA*8;
  c->dphi = DPHI*4;
  
  return(SUCCESS);
}

/* rounds the double with precision */
double round(double d, int precision) 
{ 
  d = rint(d * pow(10, precision));
  return(d / pow(10, precision));
}

/* converts integer to UCN */
void i2ucn(unsigned int i, char *s) 
{
  int a;
  
  i <<= 7; /* clean zeros */
  for(a=0; a<29; a++) {
    switch(a) {
    case 4: 
    case 8:
    case 10:
    case 20:
      *(s+a) = '.';
      a++;
    }
    
    if( i & 0x80000000 ) *(s+a) = '1';
    else *(s+a) = '0';
    i <<= 1; /* next */
  }
  *(s+29) = (char)NULL; /* for string like printing */
}

ErrorCode GetCellInfo(const int id, const Flag wrap, const double minimum, 
		      CellInfo* cell)
{
  if(DecodeId(id, cell) == ERROR) {
      fprintf(stderr, "ERROR(trigtowr.c): Couldn't decode cell\n");
      return(ERROR);
  }

  if (wrap == ON && cell->center.Phi < minimum) CorrectCell(cell);
  
  if(ResolveLayer(cell) == ERROR) {/* loose some region info */
    fprintf(stderr, "ERROR(trigtowr.c): Couldn't find region for digi\n"); 
    return(ERROR);
  }
  
  return(SUCCESS);
}

ErrorCode ResolveLayer(CellInfo* cell) 
{
  switch(cell->calo) {

  case 1:
  case 11:
    cell->region = 1;
    break;
    
  case 2:
    break;

  case 3:
    switch(cell->region) {
    case 1:
    case 4:
    case 5:
    case 6:
    case 7:
      cell->region = 1;
      break;
    case 2:
    case 8:
      cell->region = 2;
      break;
    case 3:
    case 9:
      cell->region = 3;
      break;
    default:
      /* Oops! */
      fprintf(stderr, "ERROR(portable.c): No such layer exists for a cell\n");
      return(ERROR);
    }
    break;

  case 4:
    switch(cell->region) {
    case 1:
    case 4:
    case 9:
      cell->region = 1;
      break;
    case 2:
    case 5:
    case 7:
    case 10:
      cell->region = 2;
      break;
    case 3:
    case 6:
    case 8:
    case 11:
      cell->region = 3;
      break;
    default:
      /* Oops! */
      fprintf(stderr, "ERROR(portable.c): No such layer exists for a cell\n");
      return(ERROR);
    }
    break;

  case 5:
    switch(cell->region) {

    case 1:
    case 2:
      cell->region = 1;
      break;
      
    case 3:
    case 4:
      cell->region = 2;
      break;
      
    case 5:
    case 6:
      cell->region = 3;
      break;

    default:
      /* Oops! */
      fprintf(stderr, "ERROR(calolib.c): No such layer exists for a cell\n");
      return(ERROR);
      
    }
    
  }

  return(SUCCESS);
  
} /* end ResolveLayer */

/* For the phi wrap around region */
void CorrectCell(CellInfo* cell)
{
  if(cell->center.Phi > 0) cell->center.Phi += 2 * PI;
}

ErrorCode fcomp(const double x, const double y, const double maxerr)
{
  if (fabs(x-y) <= maxerr) return SUCCESS;
  else return ERROR;
}

