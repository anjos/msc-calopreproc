#ifndef ZSTT_H
#define ZSTT_H

#include "error.h"
#include "ttdef.h"
#include "portable.h"
#include "common.h"
#include "trigtowr.h"

extern ErrorCode CreateZSCaloLayer(CaloTriggerTower*, const CellInfo*);
extern ErrorCode InitZSCaloLayer(CaloLayer*, const CellInfo*);

extern ErrorCode PlaceZSCell(const Energy, const CellInfo*, const point_t*,
			     CaloLayer*); 

extern index_t GetZSIndex(const CellInfo*, const int, const int, const point_t*);

#endif
