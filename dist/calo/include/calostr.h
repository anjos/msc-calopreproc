#ifndef CALOSTR_H

#define CALOSTR_H

#include "error.h"
#include "common.h"
#include "data.h"
#include "portable.h"

typedef struct StringCell {
  Energy energy;
  double EtaCenter;
  double PhiCenter;
}StringCell;

typedef struct StringLayer {
  Calorimeter calo;
  LayerLevel level;
  int NoOfCells;
  StringCell* cell;
} StringLayer;

typedef struct CaloStringRoI {
  int NoOfLayers;
  StringLayer* layer;
  Area region;
  Flag PhiWrap;
} CaloStringRoI;

typedef struct Window {
  double EtaMax;
  double EtaMin;
  double PhiMax;
  double PhiMin;
  Flag PhiWrap;
}Window;

extern ErrorCode SplitCells(const ROI*, CaloStringRoI*);
extern void StringLayerAlloc(CaloStringRoI*);
extern void StringCellAlloc(StringLayer*);
extern void CopyCellToString(StringLayer*, const double, const CellInfo*);
extern int BackTranslateEMCoord(const double, const double);
extern void LayerGravity(const StringLayer*, const Flag, Point*);
extern void MakeWindow(const Point*, const double, const double, Window*);
extern double TranslateEMCoord(const int, const double);
extern Energy AddCellsInWindow(const StringLayer*, const Window*);
extern Energy AddCells(const StringLayer*);
extern Flag WindowPhiCorrect(double*, double*);
extern void FreeCaloStrings(CaloStringRoI*);

#endif
