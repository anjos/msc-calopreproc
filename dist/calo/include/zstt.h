#ifndef ZSTT_H
#define ZSTT_H

#include "error.h"
#include "ttdef.h"
#include "portable.h"
#include "common.h"
#include "trigtowr.h"

extern ErrorCode CreateZSCaloLayer(CaloTriggerTower*, const CellInfo*);
extern ErrorCode InitZSCaloLayer(CaloLayer*, const CellInfo*);

extern ErrorCode PlaceZSCell(const Energy, const CellInfo*, const Point*,
			     CaloLayer*); 

extern Index GetZSIndex(const CellInfo*, const int, const int, const Point*);

#endif
