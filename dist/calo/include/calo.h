#ifndef CALO_H
#define CALO_H

#include <stdio.h>
#include <stdlib.h>
#include "data.h"
#include "common.h"
#include "calostr.h"

typedef double (*CaloFex) (const ROI*);

extern Energy calofexJ(const ROI*);
extern Energy calofexEM(const ROI*);
extern Energy commonfex(const ROI*);

#endif /* CALO_H */
