/* Hello emacs, this is -*- c -*- */
/* André Rabello dos Anjos <Andre.Rabello@ufrj.br> */

/* $Id: normal.h,v 1.5 2000/10/23 02:24:09 andre Exp $ */

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
   correct function call to apply in each occasion or dunno if asked so. Note
   that the first argument is mandatory in all situations since the process
   requires the rings to be there, also the normalization type. The third and
   fourth parameters are there for some specific methods: 
   3rd parameter -> UNITY+ normalization, indicating the normalization radius
   4th parameter -> WEIGTHED normalization stop, indicating the maximum ring to
   normalize with changing parameters, meaning the next rings will be
   normalized according a constant principle. */
void ring_normalize(ringroi_t*, const unsigned short*, const Energy*,
		    const config_weighted_t*);

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

/* These next 2 functions test whether we have one of the normalizations
   described. */
bool_t normal_is_weighted_seg(const unsigned short*);
bool_t normal_is_weighted_all(const unsigned short*);

#endif /* __NORMAL_H */
