#include "fexdef.h"

ErrorCode FreeCaloFeatures(CaloFeatures* feat)
{
  free(feat->feature);
  return(CALO_SUCCESS);
}

void PrintCaloFeatures(FILE* log, const CaloFeatures* feat) 
{
  int i;
  
  for(i=0; i<feat->NoOfFeatures; i++)
    fprintf(log, "%e\n", feat->feature[i]);

  return;
}

