#ifndef _PROPGADGETS_H_
#define _PROPGADGETS_H_
/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Header for PROP Gadgets
    Lang: english
*/
#ifndef INTUITION_INTUITION_H
#   include <intuition/intuition.h>
#endif
#ifndef INTUITION_INTUITIONBASE_H
#   include <intuition/intuitionbase.h>
#endif
#ifndef _GADGETS_H_
#   include "gadgets.h"
#endif

int CalcKnobSize (struct Gadget * propGadget, struct BBox * knobbox);
void RefreshPropGadget (struct Gadget * gadget, struct Window * window,
	struct IntuitionBase * IntuitionBase);
void RefreshPropGadgetKnob (UWORD flags, struct BBox * clear,
	struct BBox * knob, struct Window * window,
	struct IntuitionBase * IntuitionBase);

#endif /* _PROPGADGETS_H_ */
