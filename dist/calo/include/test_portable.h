/* hello emacs, this is -*- c -*- */
/* Andre Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: test_portable.h,v 1.2 2000/05/31 13:47:40 rabello Exp $ */

/* A doorkeeper for this header */
#ifndef __TEST_PORTABLE_H
#define __TEST_PORTABLE_H

#include <stdio.h>
#include "data.h" /* for ROI and EVENT */
#include "common.h" /* for bool_t */
#include "portable.h" /* the UCN decodification */

/* This type defines some form of gathering information of an RoI */
typedef struct roi_statistics_t {
  int ncells;
  int ncells_with_errors;
  int caloregion_errors;
  int phi_errors;
  int eta_errors;
} roi_statistics_t;

/* And this one, some type of gathering event information */
/* I've stopped here, but still have to create the event stats type and fill up
   the calorimeter analysis part on the main function */
typedef struct event_statistics_t {
  int nevents;
  int nevents_with_errors;
  roi_statistics_t* roi_stats;
} event_statistics_t;

/* the prototypes I use around here */

/* This functions checks integers that should be equal at the third and second
   argument. In the case they are not the same, a message is reported to the
   file pointed by "of", the first argument. It should return FALSE if the
   integers are different and TRUE if the they are the equal. The fourth
   parameter is "what is being checked. */
bool_t check_int (FILE*, const int, const int, const char*);

/* This functions checks floats that should be equal at the third and second
   argument. In the case they are not close enough, a message is reported to
   the file pointed by "of", the first argument. It should return FALSE if the
   floats are different and TRUE if the they are the equal. The fourth
   parameter is "what is being checked. */
bool_t check_float (FILE*, const float, const float, const char*);

/* This functions checks doubles that should be equal at the third and second
   argument. In the case they are not close enough, a message is reported to
   the file pointed by "of", the first argument. It should return FALSE if the
   floats are different and TRUE if the they are the equal. The fourth
   parameter is "what is being checked. */
bool_t check_double (FILE*, const double, const double, const char*);

/* This function checks the EM digi variables against the values decoded by
   portable.h::DecodeId(). The digis carry some decodification check values,
   i.e., CaloRegion, eta and phi of the cell. We try a match against those
   values and return TRUE if the check info matches the decodifyied values from
   DecodeId() and FALSE if not. The "stat" variable is modified, updating the
   statistics for the current RoI in terms of errors found. */
bool_t check_em_digi(roi_statistics_t*, const CellInfo*, 
		     const emCalDigiType*, FILE*);

/* This function initializes the structure given by the only argument. It uses
   references to pass and return variables to the function. This way, the
   argument must have a valid address (i.e., have to be allocated previously)
*/ 
roi_statistics_t* init_roi_statistics (roi_statistics_t*); 

/* This function initializes the structure given by the only argument. It uses
   references to pass and return variables to the function. This way, the
   argument must have a valid address (i.e., have to be allocated previously)
*/ 
event_statistics_t* init_event_statistics (event_statistics_t*);

/* This functions checks the RoI digis looking for UCN conversion and
   de-conversion errors. Such errors are reported to the second argument and
   returned by the function in the form of a statistics structure, defined
   above. */
roi_statistics_t* check_roi (const ROI*, FILE*);

/* This funtion checks the Electromagnetic part of the ROI given by the first
   argument, assuming we do have the number of digis given by the second
   argument. A statistics structure is then updated based on the errors found
   by checking these cells. A reference to this structured is passed at the
   3rd. argument and returned by this function. This way, the reference must
   have been created previously or, at least, information will be lost. Text
   output is dumped to the fourth argument. */
roi_statistics_t* check_em_part (const emCalDigiType*, const long,
				 roi_statistics_t*, FILE*);


#endif /* __TEST_PORTABLE_H */





