/* $Id: common.h,v 1.2.1.1 2000/04/07 20:13:56 rabello Exp $ */

#ifndef COMMON_H
#define COMMON_H

#define PI 3.141592653589793

typedef enum Flag {ON = 1, OFF = 0} Flag;
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

Flag PhiWrap(double*, double*);
void* SmartAlloc(void*, const int);

#endif /* COMMON_H */
