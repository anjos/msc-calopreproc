#ifndef PORTABLE_H
#define PORTABLE_H

#include <math.h>
#include "common.h"
#include "error.h"

#define DETA 0.003125
#define DPHI (2 * PI / 256)
#define E_137 1.375
#define E_14 1.4
#define E_15 1.5
#define E_16 1.6
#define E_18 1.8
#define E_20 2.0
#define E_24 2.4
#define E_25 2.5
#define E_27 2.7
#define E_29 2.9
#define E_31 3.1
#define E_32 3.2
#define RND_PREC 15 /* required precision on the nth. decimal house */
#define MAX_ABS_ERROR 1e-10
#define DONT_ROUND_OUTPUT
#define DETAIL 0 /* corrects region of EM EndCap between eta 1.8 and 2.0 */
#define SIZE_OF_UCN_STRING 30

typedef double out_type;

typedef struct CellInfo{
  int calo;
  int region;
  Point center;
  double deta;
  double dphi;
}CellInfo;

/* Get a cell id and converts into eta, phi, calo and region information */
/* the answer is recorded at the 3rd. argument. */
extern ErrorCode GetCellInfo(const int, const Flag, const double, CellInfo*);

double rint(double x);
double pow(double x, double y);

extern void i2ucn(unsigned int, char*);

/* Compares, given an maximum error diference at the third argument the two
   floats of the first and second arguments. */
ErrorCode fcomp(const double, const double, const double); 

#define DICEOLD /* Make minor changes in EMEndCap correction routines */

#endif
