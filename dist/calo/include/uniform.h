/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: uniform.h,v 1.1 2000/07/07 18:29:07 rabello Exp $ */

#ifndef _UNIFORM_H
#define _UNIFORM_H

#include "ttdef.h"

typedef struct uniform_roi_t
{
  int nlayer;
  CaloLayer* layer;
}uniform_roi_t;

/* This function uniformizes the roi given by the only argument, generating an
   uniform EM middle layer. It allocates memory for the return variable,
   therefore one should free it after usage. */
uniform_roi_t* uniformize (const tt_roi_t*, uniform_roi_t*);

/* This functions frees the contents of a uniform_roi_t pointer. The pointed
   variable is *not* freed in here. If you created it, you must free it as
   well. */
bool_t free_uniform_roi (uniform_roi_t*);

/* This function prints all layers of the uniform roi given
   (2nd. argument). The first argument should be a valid FILE*. For now this
   function can only work with EM layers since it relies on is_layer() to
   correctly select the layer of interest.

   The output organization is done using the layer granularity. So, for
   instance, if the layer is 16x16 (phixeta), there will be 16 lines with 16
   numbers each, separated by spaces. If the layer is 10x5, there will be 10
   lines with 5 numbers in each one */
void print_uniform_roi (FILE*, const uniform_roi_t*);

#endif /* _UNIFORM_H */




