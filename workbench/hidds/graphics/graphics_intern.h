#ifndef GRAPHICS_HIDD_INTERN_H
#define GRAPHICS_HIDD_INTERN_H

/* Include files */

#ifndef EXEC_LIBRARIES_H
#   include <exec/libraries.h>
#endif
#ifndef OOP_OOP_H
#   include <oop/oop.h>
#endif
#ifndef HIDD_GRAPHICS_H
#   include <hidd/graphics.h>
#endif
#include <dos/dos.h>


struct HIDDGraphicsData
{
    Class *bitMapClass;  /* bitmap class     */
    Class *gcClass;      /* graphics context */
};


struct HIDDBitMapData
{
    ULONG width;         /* width of the bitmap in pixel  */
    ULONG height;        /* height of the bitmap in pixel */
    ULONG depth;         /* depth of the bitmap in        */
    BOOL  displayable;   /* bitmap displayable?           */
    ULONG format;        /* planar or chunky              */
    ULONG flags;         /* see hidd/graphic.h 'flags for */
                         /* HIDD_Graphics_CreateBitMap'   */
    ULONG bytesPerRow;   /* bytes per row                 */
    ULONG bytesPerPixel; /* bytes per pixel               */
    APTR  colorTab;      /* color table of the bitmap     */
    APTR  buffer;        /* content of the bitmap         */
};


struct HIDDGCData
{
    APTR bitMap;     /* bitmap to which this gc is connected             */
    ULONG fg;        /* foreground color                                 */
    ULONG bg;        /* background color                                 */
    ULONG drMode;    /* drawmode                                         */
    /* WARNING: type of font could be change */
    APTR  font;      /* current fonts                                    */
    ULONG colMask;   /* ColorMask prevents some color bits from changing */
    UWORD linePat;   /* LinePattern                                      */
    APTR  planeMask; /* Pointer to a shape bitMap                        */
    APTR  userData;  /* pointer to own data                              */
    /* WARNING: structure could be extented in the future                */
};


struct class_static_data
{
    struct ExecBase * sysbase;
    struct Library       * utilitybase;
    struct Library       * oopbase;

    Class                *gfxhiddclass; /* graphics hidd class    */
    Class                *bitmapclass;  /* bitmap class           */
    Class                *gcclass;      /* graphics context class */
};


/* Library base */

struct IntHIDDGraphicsBase
{
    struct Library            hdg_LibNode;
    struct ExecBase          *hdg_SysBase;
    struct Library           *hdg_UtilityBase;
    BPTR                      hdg_SegList;

    struct class_static_data *hdg_csd;
};


#define CSD(x) ((struct class_static_data *)x)

#undef SysBase
#define SysBase (CSD(cl->UserData)->sysbase)

#undef UtilityBase
#define UtilityBase (CSD(cl->UserData)->utilitybase)

#undef OOPBase
#define OOPBase (CSD(cl->UserData)->oopbase)


/* pre declarations */

Class *init_gfxhiddclass(struct class_static_data *csd);
void   free_gfxhiddclass(struct class_static_data *csd);

Class *init_bitmapclass(struct class_static_data *csd);
void   free_bitmapclass(struct class_static_data *csd);

Class *init_gcclass(struct class_static_data *csd);
void   free_gcclass(struct class_static_data *csd);

VOID  gc_writepixeldirect_q(Class *cl, Object *obj, struct pHidd_GC_WritePixelDirect *msg);
ULONG gc_readpixel_q(Class *cl, Object *obj, struct pHidd_GC_ReadPixel *msg);

#endif /* GRAPHICS_HIDD_INTERN_H */
