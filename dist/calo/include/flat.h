#ifndef FLAT_H
#define FLAT_H

#include <stdio.h>
#include <math.h>
#include "error.h"
#include "trigtowr.h"

#define FLATEMGRAN 16
#define FLATHADGRAN 4

typedef Energy FlatEM[FLATEMGRAN][FLATEMGRAN];
typedef Energy FlatHad[FLATHADGRAN][FLATHADGRAN];

typedef ErrorCode (*AddEMFunPtr) (const CaloLayer*, const int, const int,
				  FlatEM); 

extern double rint(double x);

extern ErrorCode Flatten(const CaloTTEMRoI*, FlatEM, FlatHad);

extern ErrorCode AddEMLayer(FlatEM, const int, const int, CaloLayer*);
extern CaloLayer* CreateProvStripLayer(const CaloLayer*);

extern ErrorCode AddEMCells(const CaloLayer*, const int, const int, FlatEM,
			    AddEMFunPtr, AddEMFunPtr);

extern ErrorCode AddPSNZS(const CaloLayer*, const int, const int, FlatEM);
extern ErrorCode AddPSZS(const CaloLayer*, const int, const int, FlatEM);

extern ErrorCode AddEMBarrelNZS(const CaloLayer*, const int, const int,
				FlatEM); 
extern ErrorCode AddEMBarrelZS(const CaloLayer*, const int, const int, FlatEM);

extern ErrorCode AddEMEndcapNZS(const CaloLayer*, const int, const int,
				FlatEM); 
extern ErrorCode AddEMEndcapZS(const CaloLayer*, const int, const int, FlatEM);


extern ErrorCode AddHadCells(const CaloLayer*, const int, const int, FlatHad);

extern Energy ExtractEMAreaEnergy(const FlatEM, const int, const int, const
				  int, const int);
extern Energy ExtractHadAreaEnergy(const FlatHad, const int, const int, const
				   int, const int); 

extern Energy Get3x7EnergyPeak(const FlatEM, int*, int*);
extern void GetEnergyPeak(const FlatEM, int*, int*);

void fprintf_FLATEM(FILE*, const FlatEM);
void fprintf_FLATHAD(FILE*, const FlatHad);

#endif
