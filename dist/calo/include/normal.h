/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: normal.h,v 1.4 2000/09/19 00:33:22 andre Exp $ */

#ifndef __NORMAL_H
#define __NORMAL_H

#include "uniform.h"
#include "common.h"
#include "ring.h"

/* This function is a front end to the uniform_roi_t type. It will apply the
   correct type of normalization to the variable or dunno, if no normalization
   is demanded. */
void uniform_roi_normalize (uniform_roi_t*, const unsigned short*);

/* This function is the frontend to ringroi_t normalization. It will select the
   correct function call to apply in each occasion or dunno if asked so. The
   first argument is the ring RoI you have created. The second, a set of
   normalization flags, produced by string2normalization() bellow. The third
   argument is option and may be applied only if "unity+" normalization is
   required. It represents the sphere radius of normalization. */
void ring_normalize(ringroi_t*, const unsigned short*, const Energy*);

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

/* This function returns true if the unity normalization flag is active,
   meaning that ring-unity-normalization will be applied. */
bool_t normal_is_unity(const unsigned short*);

/* This function returns true if the unity+ normalization flag is active,
   meaning that ring-unity-normalization will be applied. */
bool_t normal_is_unityx(const unsigned short*);

#endif /* __NORMAL_H */
