#ifndef GADGETS_AROSCYCLE_H
#define GADGETS_AROSCYCLE_H

/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc: MethodIDs and AttrIDs for the AROS cycle class.
    Lang: english
*/
#ifndef EXEC_TYPES_H
#   include <exec/types.h>
#endif
#ifndef UTILITY_TAGITEM_H
#   include <utility/tagitem.h>
#endif
#ifndef LIBRARIES_GADTOOLS_H
#   include <libraries/gadtools.h>
#endif

/* The AROSCycleClass ist a subclass of GadgetClass. */

/* Use that #define instead of a string. */
#define AROSCYCLECLASS "cycle.aros"


/* Tags to be passed to AROSCYCLECLASS. */
#define AROSCYCLE_Dummy 40000L
  /* [ISG] (STRPTR *) Null-terminated list of labels for gadget. */
#define AROSCYCLE_Labels GTCY_Labels
  /* [ISG] (UWORD) Active label (starting with 0). */
#define AROSCYCLE_Active GTCY_Active

#endif /* GADGETS_AROSCYCLE_H */
