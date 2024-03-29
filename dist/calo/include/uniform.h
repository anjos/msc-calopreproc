/* Hello emacs, this is -*- c -*- */
/* Andr� Rabello dos Anjos <Andre.dos.Anjos@cern.ch> */

/* $Id: uniform.h,v 1.9 2000/11/28 21:00:39 rabello Exp $ */

#ifndef _UNIFORM_H
#define _UNIFORM_H

#include "ttdef.h"

/* This global holds the number of errors due to contour problems. Contourning
   problems are usually related to the lack of uniform granularity of certain
   RoIs. I cannot use such RoI's for ring processing or uniformization. I could
   simply not count the events, but I have to have an estimate of how many
   RoIs per file do not go into the specification for some processing. */
extern int uniform_contour_err;

/* For layer specification and selection */
typedef enum flag_t {FLAG_PS=0x40, FLAG_EM1=0x20, FLAG_EM2=0x10, FLAG_EM3=0x8, 
		     FLAG_HAD1=0x4, FLAG_HAD2=0x2, FLAG_HAD3=0x1, 
		     FLAG_ALL=0x7F} flag_t;

typedef struct uniform_roi_t
{
  int nlayer;
  CaloLayer* layer;
}uniform_roi_t;

/* This function uniformizes the roi given by the only argument, generating an
   uniform EM middle layer. It allocates memory for the return variable,
   therefore one should free it after usage. The short on the last argument
   should specify the layers to include in uniformization. The syntax for layer
   inclusion is very simple and will be scanned by simple mechanisms as
   well. This short contains at least 8 bits, and the routine will use the 7
   less significative bits of it by AND'ing it with 0x7F. Each bit, from the
   most to the least significative of these 7 bits will represent 1 layer to be
   uniformized: ps, em1, em2, em3, had1, had2, had3. For ease of use and for
   the sake of simplicity I provide a public method for converting a string
   like "ps,em1,em2,em3,had1,had2,had3" or "ps, had3,em1" into the proper short
   to be used in conjunction with uniformize(). See string2layer() bellow.

   The last parameter indicates the normalization type to use. The
   normalization can be done using all energy in all calorimeters (all),
   the energy of the layer in question (layer) or the energy on the section
   being processed, EM or HAD (section). The structure is done using the same
   principles as in string2layer(), but with a different organization for
   numbers. Note that one might have all bits or the short unset, but not more
   then one bit set at the same time. For ease of use, I have created
   string2normalization() function that can correctly set the bits. It shall
   work like string2layer(), also checking for ambiguities. */
uniform_roi_t* uniformize (const tt_roi_t*, uniform_roi_t*,
			   const unsigned short* , const unsigned short*);

/* Converts the const string on the second argument in a short to be used by
   uniformize(). The string will be scanned from left to right looking for
   patterns of 'ps', 'em1' and so, in order to build the final short. If a
   field is found that doesn't belong to the expected classes, a warning of
   ignorement will be issued, so, don't worry:) The function will return a
   pointer to a short, that should have been allocated previously. I advise to
   pass a regular static short reference to this function instead of worrying
   with data allocation on your main routines. This function accepts the
   following field separators: ',' (comma) or ' ' (space). If the special tag
   'all' is found in any position or by itself, all layers are enabled. */
unsigned short* string2layer(unsigned short*, const char*);

/* Converts the packed information on the flags of the first argument onto the
   string pointed (pre-allocated) by the second argument. It does the inverted
   function of the function layer2string(); */
char* layer2string(const unsigned short*, char*);

/* This function checks returning TRUE/FALSE if there's a coherency between
   what is required to be present on event and what should be printed on output
   in terms of layers. Note that it's NOT possible to print something that is
   not required. Therefore, in those cases, the function should return FALSE
   and let all other cases to go along with a 'clean' TRUE. If one wants to
   print all that is being selected, he/she should not have to worry with
   selecting stuff to be printed, this should be done automatically by this
   function. The first argument holds a pointer to the layer_flags and the
   second to the print_flags. */
bool_t validate_print_selection(const unsigned short*, unsigned short*);

/* This functions frees the contents of a uniform_roi_t pointer. The pointed
   variable is *not* freed in here. If you created it, you must free it as
   well. */
bool_t free_uniform_roi (uniform_roi_t*);

/* This function prints all layers of the uniform roi given (1st. argument)
   into a C-style string that is returned to the user (therefore one must free
   afterwards). The output organization is done using the layer
   granularity. So, for instance, if the layer is 16x16 (phixeta), there will
   be 16 lines with 16 numbers each, separated by spaces. If the layer is 10x5,
   there will be 10 lines with 5 numbers in each one */
char* get_uniform_roi (const uniform_roi_t*, const unsigned short*);

/* Returns TRUE if the flags contain a description for inclusion of layer on
   the current processing and FALSE otherwise. */
bool_t flag_contains_layer(const unsigned short*, const CaloLayer*);

/* Counts the number of layers that should be printed. This is useful for
   functions that have to preallocate space based on the number of layers that
   should be processed. */
short flag_contains_nlayers(const unsigned short*);

/* This function will find the peak of energy on the current layer. When found,
   the function will return (2nd to 4th. parameters), the peak index, its real
   eta and real phi values (relative to the RoI lower left corner). */
void peak_find(const CaloLayer*, int*, double*, double*);

/* This function will just calculate the relative point of the cell into the
   RoI. The EM RoI is 0.4 by 0.4 in eta x phi and using the granularity of eta
   and phi it's not difficult to calculate where the cell is. The RoI scanning
   is defined on <root>/doc/fig/roi-scanning.fig. The first two arguments
   define eta and phi granularity. The third is the index inside the layer
   vector of the cell. The fourth and fifth are used to return the actual
   values eta and phi. */
void vector2point(const int*, const int*, const int*, double*, double*);

#endif /* _UNIFORM_H */




