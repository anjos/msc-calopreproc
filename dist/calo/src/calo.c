#include "calo.h"

static FILE* log_err;

Energy calofexJ(const ROI* roi)
{
  return commonfex(roi);
}

Energy calofexEM(const ROI* roi)
{
  return commonfex(roi);
}

Energy commonfex(const ROI* roi)
{
  CaloStringRoI sroi;
  Energy total = 0.;
  int it;

  /* just separate each sublayer of the RoI */
  if ( SplitCells(roi, &sroi) == CALO_ERROR ) {  
    fprintf(log_err, "ERROR(calo.c): Couldn't split EM Cells\n");
    return(0.);
  }

  /* add the interesting layers, PS, FL, ML and BL */
  for (it = 0; it < sroi.NoOfLayers; it++)
    switch (sroi.layer[it].calo) {
    case EMBARREL:
    case EMENDCAP:
      total += AddCells(&sroi.layer[it]);
      break;
      
    case PSBARRREL:
    case PSENDCAP:
    case TILECAL:
    case HADENDCAP:
      break;
    }

  return total;

}
