/*
    (C) 1997 AROS - The Amiga Research OS
    $Id$

    Desc:
    Lang: english
*/
#include <aros/libcall.h>


/*****************************************************************************

    NAME */
#include <proto/layers.h>
#include "layers_intern.h"

	AROS_LH4(LONG, SizeLayer,

/*  SYNOPSIS */
	AROS_LHA(LONG          , dummy, A0),
	AROS_LHA(struct Layer *, l    , A1),
	AROS_LHA(LONG          , dx   , D0),
	AROS_LHA(LONG          , dy   , D1),

/*  LOCATION */
	struct LayersBase *, LayersBase, 11, Layers)

/*  FUNCTION
        Resizes the given layer by adding dx to its width and dy
        to its height.
        If parts of simple layers become visible those parts are
        added to the damage list and a refresh is triggered for
        those layers. 
        If the new layer is bigger than before the additional parts
        are added to a damage list if the layer is a non-super-
        bitmap layer. Refresh is also triggered for this layer.

    INPUTS
        l    - pointer to layer to be resized
        dx   - delta to be added to the width
        dy   - delta to be added to the height

    RESULT
        TRUE  - layer could be resized
        FALSE - error occurred (out of memory)

    NOTES

    EXAMPLE

    BUGS

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
    In this case I will create a layer behind the current layer and
    I will try to clear as little of the layer that was above it as
    possible.
  */    

  struct Layer * l_tmp;
  struct ClipRect * CR;
  struct RastPort * RP;
  struct Layer_Info * LI = l->LayerInfo;

  /* Check coordinates as there's no suport for layers outside the displayed
     bitmap. I might add this feature later. */
  if (l->bounds.MaxX - l->bounds.MinX + 1 + dx <= 0 ||
      l->bounds.MaxY - l->bounds.MinY + 1 + dy <= 0 ||
      l->bounds.MaxX+dx > GetBitMapAttr(l->rp->BitMap, BMA_WIDTH) ||
      l->bounds.MaxY+dy > GetBitMapAttr(l->rp->BitMap, BMA_HEIGHT))
    return FALSE; 
  

  /* Lock all other layers while I am resizing this layer */
  LockLayers(LI);

  /* 
     Here's how I do it:
     I create a new layer behind the given layer at the new position,
     copy the bitmaps from the old layer to the new layer via ClipBlit() 
     and delete the old layer. 
     In order to maintain the pointer of the layer I will create a Layer
     structure, link it into the list in front of the new layer, 
     copy important data to the newly created structure and connect 
     the cliprects to it, of course.
   */
  
  l_tmp = (struct Layer *)AllocMem(sizeof(struct Layer)   , MEMF_CLEAR|MEMF_PUBLIC);
  CR = _AllocClipRect(LI);
  RP = (struct RastPort *)AllocMem(sizeof(struct RastPort), MEMF_CLEAR|MEMF_PUBLIC);

  if (NULL != l_tmp && NULL != CR && NULL != RP)
  {
    struct CR_tmp;
    struct Layer * l_behind; 
    LONG width, height;
    /* link the temporary layer in front of the layer to resize */
    l_tmp->back  = l;
    l_tmp->front = l->front;

    if (LI->top_layer == l)
      LI->top_layer = l_tmp;
    else
      l->front->back = l_tmp;

    l->front = l_tmp;
    /* copy important data to the temporary layer. this list might be 
       shrinkable
       depending on what data deletelayer() needs later on */
    l_tmp->ClipRect   = l->ClipRect;
    l_tmp->rp         = RP;
    l_tmp->bounds     = l->bounds;
    l_tmp->Flags      = l->Flags;
    l_tmp->LayerInfo  = LI;
    l_tmp->DamageList = l->DamageList;
    l_tmp->SuperBitMap= l->SuperBitMap;
    l_tmp->Scroll_X   = l->Scroll_X;
    l_tmp->Scroll_Y   = l->Scroll_Y;

    /* init the rastport structure of the temporary layer */
    InitRastPort(RP);
    RP -> Layer  = l_tmp;
    RP -> BitMap = l->rp->BitMap;

    /* I have to go through all the cliprects of the layers that are 
       behind this layer and have an enty in lobs pointing to l. I
       have to change this pointer to l_tmp, so that everything still
       works fine later, especially the DeleteLayer() */

    l_behind = l_tmp->back;
    while (NULL != l_behind)
    {
      struct ClipRect * _CR = l_behind->ClipRect;
      while (NULL != _CR)
      {
        if (_CR->lobs == l)
	{
          _CR->lobs = l_tmp;
	}
        _CR = _CR->Next;
      } /* while */
      l_behind = l_behind ->back;
    } /* while */

   
    InitSemaphore(&l_tmp->Lock);
    LockLayer(0, l_tmp);

    /* modify the layer l's structure for the new size */
    l->bounds.MaxX += dx;
    l->bounds.MaxY += dy;

    l->ClipRect = CR;

    l->DamageList = NewRegion();

    /* Copy the bounds */
    CR->bounds = l->bounds;
    /* 
      Now create all ClipRects of all Layers correctly.   
      Comment: CreateClipRects is the only function that does the
               job correctly if you want to create a layer somewhere
               behind other layers.
    */
    CreateClipRects(LI, l); 
    /*
       Ok, all other layers were visited and pixels are backed up.
       Now we can draw the new layer by copying all parts of the
       temporary layer's cliprects to the new layer via ClipBlit.
    */
    
    /* 
       The resized layer  might be smaller than the old
       one, so I have to find out about the width and height that I
       am allowed to copy 
     */
    if (dx < 0)
      width  = l    ->bounds.MaxX - l    ->bounds.MinX + 1;
    else
      width =  l_tmp->bounds.MaxX - l_tmp->bounds.MinX + 1;
    
    if (dy < 0)
      height = l    ->bounds.MaxY - l    ->bounds.MinY + 1;
    else
      height = l_tmp->bounds.MaxY - l_tmp->bounds.MinY + 1;

    ClipBlit(l_tmp->rp,
             0,
             0,
             l->rp,
             0,
             0,
             width,
             height,
             0x0c0);
    /*
      If the layer is a superbitmapped layer and the width or
      the height of the resized layer has decreased, then I
      have to backup the visible part into the superbitmap so
      the infomation doesn't get lost.
     */
    /* 
      only do this if the width or the height has decreased 
      and only for a superbitmapped layer!
     */
 
    if (0 != (l->Flags & LAYERSUPER) && (dx < 0 || dy < 0))
    {
      struct BitMap * bm = l->rp->BitMap;

      CR = l_tmp->ClipRect;
      while (NULL != CR)
      {
        /* only need to back up if CR is visible */
        if (NULL == CR->lobs)
	{
          LONG SrcX, SrcY;
          if (dx < 0 && (CR->bounds.MaxX - l_tmp->bounds.MinX) > width)
	  {
            if ((CR->bounds.MinX - l_tmp->bounds.MinX) > width)
              SrcX = CR->bounds.MinX;
            else
              SrcX = l_tmp->bounds.MinX + width;

            BltBitMap(
              bm,             /* Source Bitmap = rastport's bitmap */
              SrcX,
              CR->bounds.MinY,
              l->SuperBitMap, /* Destination Bitmap = SuperBitMap */
              SrcX            - l->bounds.MinX + l->Scroll_X,
              CR->bounds.MinY - l->bounds.MinY + l->Scroll_Y,
              CR->bounds.MaxX - SrcX            + 1,
              CR->bounds.MaxY - CR->bounds.MinY + 1,
              0x0c0, /* copy */
              0xff,
              NULL
            );
	  }

          if (dy < 0 && (CR->bounds.MaxY - l_tmp->bounds.MinY) > height)
	  {
            if ((CR->bounds.MinY - l_tmp->bounds.MinY) > height)
              SrcY = CR->bounds.MinY;
            else
              SrcY = l_tmp->bounds.MinY + height;

            BltBitMap(
              bm,             /* Source Bitmap = rastport's bitmap */
              CR->bounds.MinX,
              SrcY,
              l->SuperBitMap, /* Destination Bitmap = SuperBitMap */
              CR->bounds.MinX - l->bounds.MinX + l->Scroll_X,
              SrcY            - l->bounds.MinY + l->Scroll_Y,
              CR->bounds.MaxX - CR->bounds.MinX + 1,
              CR->bounds.MaxY - SrcY            + 1,
              0x0c0, /* copy */
              0xff,
              NULL
            );
	  }

	}
        CR = CR->Next;
      } 
    }

    /* 
      The layer that was resized is totally visible now but right on
      top of it is the old layer. I delete the old layer from its
      now.
      !!! I should not call DeleteLayer here as what becomes visible
      now will just look alike except for in certain areas where the
      new layer is bigger or smaller than before! It can be done
      with less calls to BltBitMap than when calling DeleteLayer().
    */
    DeleteLayer(0, l_tmp);
    /*
       The new layer might be larger than the previously shown layer,
       so I clear those areas of the new layer that are outside the
       area that ClipBlit reached into.
    */
    /* only do this if the size increased */

    if (dx > 0 || dy > 0)
    {
      struct BitMap * bm = l->rp->BitMap;
      CR = l->ClipRect;
      while (NULL != CR)
      {
        /* only show those CR that are not hidden, of course. */
        if (NULL == CR->lobs)
        {
          LONG DestX, DestY;
          struct Rectangle bounds;

          if (dx > 0 && (CR->bounds.MaxX - l->bounds.MinX) > width)
	  {
            if ((CR->bounds.MinX - l->bounds.MinX) > width)
              DestX = CR->bounds.MinX;
            else
              DestX = l->bounds.MinX + width;

            if (0 == (l->Flags & LAYERSUPER))
	    {
	      bounds.MinX = DestX;
	      bounds.MinY = CR->bounds.MinY;
	      bounds.MaxX = CR->bounds.MaxX;
	      bounds.MaxY = CR->bounds.MaxY;
          
              /* clearing the visible area with the backfill hook */
              _CallLayerHook(l->BackFill,
                             l->rp,
                             l,
                             &bounds,
                             bounds.MinX,
                             bounds.MinY);
	    
              /* no superbitmap */
/*
              BltBitMap(
                bm,// Source Bitmap - we don't need one for clearing, but this
                   // one will also do :-) 
                0,
                0,
                bm, // Destination Bitmap - 
                DestX,
                CR->bounds.MinY,
                CR->bounds.MaxX - DestX           + 1,
                CR->bounds.MaxY - CR->bounds.MinY + 1,
                0x000, // supposed to clear the destination
                0xff,
                NULL
              );
*/
              OrRectRegion(l->DamageList, &CR->bounds);
              l->Flags |= LAYERREFRESH;
	    }
            else
	    {
              /* with superbitmap */
              BltBitMap(
                l->SuperBitMap /* Source Bitmap = superbitmap */,
                DestX           - l->bounds.MinX + l->Scroll_X,
                CR->bounds.MinY - l->bounds.MinY + l->Scroll_Y,
                bm             /* Destination Bitmap */,
                DestX,
                CR->bounds.MinY,
                CR->bounds.MaxX-DestX+1,
                CR->bounds.MaxY-CR->bounds.MinY+1,
                0x0c0 /* copy */,
                0xff,
                NULL
              );
            }
	  }

          if (dy > 0 && (CR->bounds.MaxY - l->bounds.MinY) > height)
	  {
            if ((CR->bounds.MinY - l->bounds.MinY) > height)
              DestY = CR->bounds.MinY;
            else
              DestY = l->bounds.MinY + height;

            if (0 == (l->Flags & LAYERSUPER))
	    {
	      bounds.MinX = CR->bounds.MinX;
	      bounds.MinY = DestY;
	      bounds.MaxX = CR->bounds.MaxX;
	      bounds.MaxY = CR->bounds.MaxY;
          
              /* clearing visible area with the backfill hook */
              _CallLayerHook(l->BackFill,
                             l->rp,
                             l,
                             &bounds,
                             bounds.MinX,
                             bounds.MinY);
              /* no superbitmap */
/*
              BltBitMap(
                bm, // Source Bitmap - we don't need one for clearing, but this
                    // one will also do :-) 
                0,
                0,
                bm, // Destination Bitmap
                CR->bounds.MinX,
                DestY,
                CR->bounds.MaxX-CR->bounds.MinX+1,
                CR->bounds.MaxY-DestY+1,
                0x000, // supposed to clear the destination
                0xff,
                NULL
              );
*/
              OrRectRegion(l->DamageList, &CR->bounds);
              l->Flags |= LAYERREFRESH;
	    }
            else
	    {
              /* with superbitmap */
              BltBitMap(
                l->SuperBitMap /* Source Bitmap = superbitmap*/,
                CR->bounds.MinX - l->bounds.MinX + l->Scroll_X,
                DestY           - l->bounds.MinY + l->Scroll_Y,
                bm /* Destination Bitmap */,
                CR->bounds.MinX,
                DestY,
                CR->bounds.MaxX-CR->bounds.MinX+1,
                CR->bounds.MaxY-DestY+1,
                0x0c0 /* copy */,
                0xff,
                NULL
              );
	    }
	  }
        }
        else
        {
          /* this is a hidden ClipRect */
          /* if it's from a smart layer then I will have to fill it
             with the backfill hook. */
          if (LAYERSMART == (l->Flags & (LAYERSMART|LAYERSUPER)) &&
              l->BackFill != LAYERS_BACKFILL &&  /* try to avoid wasting time */
              l->BackFill != LAYERS_NOBACKFILL)
          {
            /* it's a pure smart layer that needs to be filled with a
               !!pattern!!. LAYERS_BACKFILL & LAYERS_NOBACKFILL wouldn't
               do anything good here at all.
            */
            struct Rectangle bounds;
            struct BitMap * bm = l->rp->BitMap;
            bounds.MinX = (CR->bounds.MinX & 0x0f);
            bounds.MinY = 0;
            bounds.MaxX = CR->bounds.MaxX - CR->bounds.MinX + (CR->bounds.MinX & 0x0f);
            bounds.MaxY = CR->bounds.MaxY;

            /* filling the hidden cliprect's bitmap with the pattern */
            l->rp->BitMap = CR->BitMap;            
            _CallLayerHook(l->BackFill,
                           l->rp,
                           l,
                           &bounds,
                           CR->bounds.MinX - l->bounds.MinX + l->Scroll_X,
                           CR->bounds.MinY - l->bounds.MinY + l->Scroll_Y);
            l->rp->BitMap = bm;             
          }  
        }
      CR = CR->Next;
      }
    }
   
    /* That's it folks! */

    /* Now everybody else may play with the layers again */
    UnlockLayers(l->LayerInfo);
    return TRUE;
  } 
  else /* not enough memory */
  {
    if (NULL != CR   ) _FreeClipRect(CR, l_tmp->LayerInfo);
    if (NULL != RP   ) FreeMem(RP, sizeof(struct RastPort));
    if (NULL != l_tmp) FreeMem(l_tmp, sizeof(struct Layer));
  }

  return FALSE;
   
  AROS_LIBFUNC_EXIT

} /* SizeLayer */
