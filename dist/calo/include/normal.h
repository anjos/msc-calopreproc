/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: normal.h,v 1.1 2000/09/06 14:58:45 andre Exp $ */

#ifndef __NORMAL_H
#define __NORMAL_H

#include "uniform.h"
#include "common.h"

/* Just divides all energy values on the pointer to the uniform RoI by the
   energy value contained in nfactor. OBS: the uniform RoI is changed during
   this call. */
void uniform_roi_normalize (uniform_roi_t*, const short);

/* Converts the string token to one of the possible normalization parameters,
   defined on the body of uniform.c:normal_t. This function checks
   automatically for ambiguities, so there's no problem in trying to define
   more than one type of normalization. In the last case, a warning shall be
   issued. */
unsigned short* string2normalization(unsigned short*, const char*);

/* Commits the inverse processing of string2normalization(). The char* must
   have its space pre-allocated since it won't be inside the function. The
   minimum space requirement is 60 bytes */
char* normalization2string(const unsigned short*, char*);

#endif /* __NORMAL_H */
