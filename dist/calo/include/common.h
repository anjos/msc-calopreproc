/* hello emacs, this is -*- c -*- */
/* Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: common.h,v 1.11 2000/12/08 15:19:27 rabello Exp $ */

#ifndef COMMON_H
#define COMMON_H

#define PI 3.141592653589793

/* BEGIN OF TYPE DEFINITIONS */

typedef enum bool_t {TRUE = 1, FALSE = 0} bool_t;

typedef enum Calorimeter {PSBARRREL = 1, EMBARREL = 2, EMENDCAP = 3, TILECAL =
			  4, HADENDCAP = 5, PSENDCAP = 11} Calorimeter;

/* In order to make things simpler, I should enumerate a new type that holds
   only 3 sub-types of calorimeters, or sections. */
typedef enum section_t {PS=PSBARRREL, EM=EMBARREL, HAD=TILECAL} section_t;

/* typedef enum LayerLevel { SCINTILLATOR = 0, FRONT = 1, MIDDLE = 2, BACK = 3}
  LayerLevel; */

typedef int LayerLevel;

typedef double Energy;

typedef struct index_t {
  int eta;
  int phi;
} index_t;

typedef struct point_t {
  double eta;
  double phi;
} point_t;

typedef struct Area {
  point_t LowerLeft;
  point_t UpperRight;
} Area;

/* END OF TYPE DEFINITIONS */


/* FUNCTION PROTOTYPES */
/* =================== */

/* 

   The L1 RoI selector, may mark the value of PhiMax as being the max value of
   phi among the corners of the RoI or the one in the end following the
   counter-clockwise direction of the RoI along phi. This functions examins the
   4 cases and correct the RoI phi values:

   1) RoI has phi.min < phi.max, no correction needed
   
   2) RoI has phi.max < phi.min, I have to exchange both and set PHIWRAP to
   OFF, since it does not wrap around 2pi;
  
   3) RoI has phi.max < phi.min and the RoI is lying in the limit 0 -> 2*pi, I
   have to add 2*pi to phi.max and set PHIWRAP to ON.

   4) RoI has phi.min < phi.max and the RoI is lying in the limit 0 -> 2*pi, I
   have to exchange both, and after that add 2*PI to phi.max

   In order to test if I have to return TRUE (meaning that the transition 2*pi
   -> 0 belongs to the RoI area) I can subtract the minimum and maximum values
   for phi. If those are greater than the maximum allowed difference, than it's
   likely that this RoI contains the transition limits for phi.

   I say likely, because there's no way to uniquely determining the RoI inner
   side (if I may say that) based on the values of phimin and phimax. 

   *ATTENTION*: For simplification I'll presume that only cases 1 and 4 may
   happen. In this sense, I can uniquely determined the RoI inner side based on
   sums and subtractions. This is pretty reasonable, since I extract these
   numbers from the files and there the RoI limits are marked by its maximum
   and minimum values of eta and phi.

*/
bool_t PhiWrap(double*, double*);

/* This function is capable of allocating a given number of positions of size
   given by the last argument , testing whether the allocation was performed
   right. If the first argument is not null, than, it reallocates space for the
   current pointer and tests it. The allocated space is initialized if this is
   call to obtain new space. */
void* mxalloc(void*, const int, const int);

/* Concatenate one string to the other, returning the first. The memory needed
   is allocated as with malloc(). If *to is NULL, newly allocated memory is
   returned. The returned value is the number of chars passed to the first
   argument. ATTENTION: The first argument is a double-char-pointer. The string
   which is going to be allocated is going to be placed there. I suggest using
   &((char*)charptr) instead of creating a double-char-pointer. */
int ascat (char**, const char*);

/* Concatenate one string to a double forming a new string with the double
   value included. The precision used is the default of XXprintf() functions in
   general, i.e. "%e". The memory needed is allocated as with malloc(). If *to
   is NULL, newly allocated memory is returned. The returned value is the
   number of chars passed to the first argument. ATTENTION: The first argument
   is a double-char-pointer. The string which is going to be allocated is going
   to be placed there. I suggest using &((char*)charptr) instead of creating a
   double-char-pointer. */
int ascat_double (char**, const double*);

/* Concatenate one string to a float  forming a new string with the float 
   value included. The precision used is the default of XXprintf() functions in
   general, i.e. "%e". The memory needed is allocated as with malloc(). If *to
   is NULL, newly allocated memory is returned. The returned value is the
   number of chars passed to the first argument. ATTENTION: The first argument
   is a double-char-pointer. The string which is going to be allocated is going
   to be placed there. I suggest using &((char*)charptr) instead of creating a
   double-char-pointer. */
int ascat_float (char**, const float*);

/* Concatenate one string to an int forming a new string with the int value
   included. The precision used is the default of XXprintf() functions in
   general, i.e. "%d". The memory needed is allocated as with malloc(). If *to
   is NULL, newly allocated memory is returned. The returned value is the
   number of chars passed to the first argument. ATTENTION: The first argument
   is a double-char-pointer. The string which is going to be allocated is going
   to be placed there. I suggest using &((char*)charptr) instead of creating a
   double-char-pointer. */
int ascat_int (char**, const int*);


#endif /* COMMON_H */
