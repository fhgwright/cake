/*
    (C) 1997 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/
#include <exec/types.h>
#include <proto/exec.h>
#include <proto/graphics.h>
#include <graphics/layers.h>


#define DEBUG 0
#include <aros/debug.h>
#undef kprintf

/*****************************************************************************

    NAME */
#include <proto/layers.h>
//#include "layers_intern.h"
#include <aros/libcall.h>

	AROS_LH2(LONG, DeleteLayer,

/*  SYNOPSIS */
	AROS_LHA(LONG          , dummy, A0),
	AROS_LHA(struct Layer *, LD   , A1),

/*  LOCATION */
	struct LayersBase *, LayersBase, 15, Layers)

/*  FUNCTION

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS
      No call to any refresh functions of any layers.

    SEE ALSO

    INTERNALS

    HISTORY
	27-11-96    digulla automatically created from
			    layers_lib.fd and clib/layers_protos.h

*****************************************************************************/
{
  AROS_LIBFUNC_INIT
  AROS_LIBBASE_EXT_DECL(struct LayersBase *,LayersBase)

  /*
    This is how  we do it:
      The layer to be deleted is called LD
      
    - All layers in front of the layer LD
      are touched in no way by this operation.

    - The ClipRects of the layer LD that are hidden by
      another layer can be deallocated immediately as
      no refresh has to be made there.
      These ClipRects can be recognized by lobs != NULL.
      A bitmap is also found there and must be deallocated. 

    - For the layer immediately behind the layer to be deleted:
      If a ClipRect->lobs of that layer is pointing to the
      layer to be destroyed then the bitmap found there can be 
      restored, the bitmap deallocated and the pointer lobs
      has to be zeroed. The restored ClipRect has to be cut
      out (splitting the rectangle) of the list of the 
      cliprects of the layer LD.
      
    - For all layers further behind:
      If a ClipRect is found with lobs pointing to layer LD
      ask in which layer (staring with the layer behind LD) the 
      upper left corner is located. If it is located in the layer
      the ClipRect belongs to, then restore the rectangle, cut
      it out of the ClipRect list of the layer LD. 
      Otherwise make lobs point to the result of the call to 
      internal_WhichLayer();
  
  Ok, here we go: 
  */
  struct Layer_Info * LI = LD->LayerInfo;
  struct ClipRect * DCR,  * CR;
  struct Region * R;
  struct RegionRectangle * RR;
  /* no one may interrupt me */
  LockLayers(LI);

  /* take this Layer out of the list of layers */
  if (NULL != LD->front)
  {
    LD->front->back = LD->back;
    LD->back->front = LD->front;
  }
  else /* take out the very first layer */
  {
    LI -> top_layer = LD -> back;
    if (NULL != LD->back)
      LD->back->front = NULL;
  }
 
  /* Let's delete the ClipRects of the layer LD that are
     hidden themselves. 
     The other ClipRects I add to the damage List for
     me to refresh (clear) through at the end.
  */

  /* clear the region that is there */
  ClearRegion(LD->DamageList);
  
  CR = LD->ClipRect;
  while (NULL != CR)
  {
    struct ClipRect * _CR = CR->Next;
    if (NULL != CR->lobs)
      FreeBitMap(CR->BitMap);
    else
    {
      /* Add this rectangle to the region which we might have
         to clear later on
      */
      OrRectRegion(LD->DamageList, &CR->bounds);
    }
    FreeMem(CR, sizeof(struct ClipRect));      
    CR = _CR;
  }
  /* just to make sure...
     Remember: The ClipRects list is now invalid!
  */
  
  /* there is a damagelist left and there is 
     a layer behind */
  if (NULL != LD->DamageList->RegionRectangle && NULL != LD->back)
  {
    /* let's visit the layers that are behind this layer */
    /* start out with the one immediately behind the layer
       LD */
    struct Layer * L_behind = LD->back;
    CR = L_behind -> ClipRect;

    while (NULL != L_behind)
    {
      CR = L_behind -> ClipRect;
      while (NULL != CR)
      {
        if (LD == CR->lobs)
        {
          struct Layer * Ltmp = internal_WhichLayer(
            LD->back,
            CR->bounds.MinX,
            CR->bounds.MinY
          );
          /* if this layer is now visible 
             (it was previously hidden by the layer ld,
              but not anymore) 
          */
          if (Ltmp == L_behind)
          {  /* restore the bitmap stuff found there */

            BltBitMap(
              CR->BitMap,
              CR->bounds.MinX & 0x0f,
              0,
              LD->rp->BitMap,
              CR->bounds.MinX,
              CR->bounds.MinY,
              CR->bounds.MaxX - CR->bounds.MinX + 1,
              CR->bounds.MaxY - CR->bounds.MinY + 1,
              0x0c0, /* copy */
              0xff,
              NULL
             );
            /* Free the bitmap */
            FreeBitMap(CR->BitMap);
            CR->BitMap = NULL;
            CR->lobs = NULL;
            /* leave a mark for the garbage collector that
               will combine small cliprects to larger ones */
            CR->reserved = 1;
            ClearRectRegion(LD->DamageList, &CR->bounds);
          } /* if */  
        } /* if */
        CR = CR->Next;
      } /* while */
      L_behind = L_behind -> back;
    }
  
    /* !! this is not implemented !! */
    /* The List of the ClipRects of the layer LD should
       now only contain these parts that have to be cleared
       in the bitmap. 
    */
    R = LD->DamageList;

    SetAPen(LD->rp, 255);

    RR = R->RegionRectangle;
    /* check if a region is empty */
    while (NULL != RR)
    {

      Move(LD->rp,RR->bounds.MinX,RR->bounds.MinY);
      Draw(LD->rp,RR->bounds.MaxX,RR->bounds.MinY);
      Draw(LD->rp,RR->bounds.MaxX,RR->bounds.MaxY);
      Draw(LD->rp,RR->bounds.MinX,RR->bounds.MaxY);
      Draw(LD->rp,RR->bounds.MinX,RR->bounds.MinY);
      Delay(10);

      printf("Found RectangleRegion: (%i,%i) - (%i,%i)\n",
        RR->bounds.MinX,
        RR->bounds.MinY,
        RR->bounds.MaxX,
        RR->bounds.MaxY);

      BltBitMap(
        LD->rp->BitMap, /* don't need a source but mustn't give NULL!!!*/
        0,
        0,
        LD->rp->BitMap,
        RR->bounds.MinX + R->bounds.MinX,
        RR->bounds.MinY + R->bounds.MinY,
        RR->bounds.MaxX - RR->bounds.MinX + 1,
        RR->bounds.MaxY - RR->bounds.MinY + 1,
        0x000, /* clear destination */
        0xff,
        NULL
      );      
      RR = RR->Next;
    } /* while */
  
  } /* if */
  
  FreeMem(LD->rp, sizeof(struct RastPort));
  DisposeRegion(LD->DamageList);
  FreeMem(LD, sizeof(struct Layer));
  
  /* ok, I'm done */
  UnlockLayers(LI);
  return TRUE;
  
  AROS_LIBFUNC_EXIT
} /* DeleteLayer */
