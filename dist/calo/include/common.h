/* hello emacs, this is -*- c -*- */
/* Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: common.h,v 1.6 2000/06/16 21:26:42 rabello Exp $ */

#ifndef COMMON_H
#define COMMON_H

#define PI 3.141592653589793

/* BEGIN OF TYPE DEFINITIONS */

typedef enum bool_t {TRUE = 1, FALSE = 0} bool_t;

typedef enum Calorimeter {PSBARRREL = 1, EMBARREL = 2, EMENDCAP = 3, TILECAL =
			  4, HADENDCAP = 5, PSENDCAP = 11} Calorimeter;

typedef enum LayerLevel { SCINTILLATOR = 0, FRONT = 1, MIDDLE = 2, BACK = 3}
LayerLevel; 

typedef enum CaloType {ELECTROMAGNETIC = 0, HADRONIC = 1} CaloType;

typedef double Energy;

typedef struct Index {
  int Eta;
  int Phi;
} Index;

typedef struct Point {
  double Eta;
  double Phi;
}Point;

typedef struct Location {
  Index index;
  Point point;
} Location;

typedef struct Area {
  Point LowerLeft;
  Point UpperRight;
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

#endif /* COMMON_H */
