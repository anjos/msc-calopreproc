#ifndef PORTABLE_H
#define PORTABLE_H

#include "common.h"
#include "error.h"

/* This define will change the way the EMEndCap has its calorimeter regions
   corrected. Such correction have to exist because the regions of EmEndCap
   cannot be fully specified by the m bits and auxiliar information has to be
   taken from the cell eta position. The old DICE routines use one calorimeter
   configuration and the new DICE another one. This define makes the program
   switch between the correction types for each case. Choose 1 for the old DICE
   geometry and 0 for the new one. HINT: If you have doubts, choose 1 since
   most files follow such system. */
#define DICEOLD 1

/* This defines the size of strings that will host a Universal Cell Enconding
   Number (UCN). They have to be 30 chars big because UCN has 25 bits plus 4
   dots and a (char)NULL at the end to mark the end of the string. */
#define SIZE_OF_UCN_STRING 30 

typedef struct CellInfo{
  int calo;
  int region;
  Point center;
  double deta;
  double dphi;
}CellInfo;

/* Get a cell id at the 1st. argument and converts into eta, phi, calo and
   region information the answer is recorded at the 2nd. argument. The third
   argument is there to provide a simple solution to the phi wrapping
   problem. The problem consists in having an RoI that is placed over the
   region 2*pi <-> 0 at the calorimeter. Over these regions phimin may be
   greater then phimax, depending the first level trigger classification. One
   should correct this problem or just point out it's happening so other
   programs can take this in consideration. Here the second argument indicates
   whether one should correct the Phiwraping (ON) or not (OFF). If yes,
   correction is applied by shifting cell's center->Phi to center->Phi + 2*pi
   IF the center of such cell is place between 0 and the PI. */
ErrorCode GetCellInfo(const int, CellInfo*, const Flag);

/* Converts a UCN integer into string format. This format is given by Stefan
   Simion and is basically CCCC.mmm.s.eeeeeeeee.pppppppp where each bit codes
   values for the (C)alorimeter, (m)odule, (s)side, (e)ta and (p)hi of the cell
*/ 
void i2ucn(const unsigned int, char*);

/* Compares, given an maximum error diference at the third argument, the two
   floats of the first and second arguments. */
ErrorCode fcomp(const double, const double, const double); 

#endif



