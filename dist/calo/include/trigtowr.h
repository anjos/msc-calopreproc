/* Hello emacs, this is -*- c -*- */

/* $Id: trigtowr.h,v 1.4 2000/07/07 18:27:25 rabello Exp $ */

#ifndef _TRIGTOWR_H
#define _TRIGTOWR_H

#include "data.h"
#include "error.h"
#include "common.h"
#include "ttdef.h"

/* Using the ROI, define by CERN specification files, build, optinally fixing
   the window size, a tt_roi_t */
ErrorCode build_roi(const ROI*, const bool_t, tt_roi_t*);

/* Free an *used* tt_roi_t* */
void free_roi(tt_roi_t*);

/* Free an *used* CaloLayer */
void free_layer(CaloLayer*);

#endif /* _TRIGTOWR_H */






