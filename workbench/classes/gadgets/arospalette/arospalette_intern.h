#ifndef AROSPALETTE_INTERN_H
#define AROSPALETTE_INTERN_H

#undef  AROS_ALMOST_COMPATIBLE 
#define AROS_ALMOST_COMPATIBLE 

#ifndef EXEC_TYPES_H
#   include <exec/types.h>
#endif
#ifndef EXEC_LIBRARIES_H
#   include <exec/libraries.h>
#endif
#ifndef DOS_BPTR_H
#   include <dos/bptr.h>
#endif
#ifndef GRAPHICS_RASPORT_H
#   include <graphics/rastport.h>
#endif
#ifndef INTUITION_CGHOOKS_H
#   include <intuition/cghooks.h>
#endif
#ifndef INTUITION_CLASSES_H
#   include <intuition/classes.h>
#endif
#ifndef INTUITION_INTUITION_H
#   include <intuition/intuition.h>
#endif

/* Predeclaration */
struct PaletteBase_intern;

#define GLOBAL_INTUIBASE

struct PaletteData
{
    UWORD	pd_NumColors;
    UBYTE	*pd_ColorTable;
    UBYTE	pd_Color;
    /* For state info, to know what selected entry to delete in GM_RENDER, GREDRAW_UPDATE */
    UBYTE	pd_OldColor; 
    
    UBYTE	pd_ColorOffset;
    UWORD	pd_IndWidth;
    UWORD	pd_IndHeight;

    struct IBox pd_GadgetBox;	 /* Box surrounding whole palette 	*/
    struct IBox pd_PaletteBox;	 /* Box surrounding palette 		*/
    struct IBox pd_IndicatorBox; /* Box surrounding indicator		*/
    
    UWORD	pd_ColWidth;
    UWORD	pd_RowHeight;
    UBYTE	pd_NumCols;
    UBYTE	pd_NumRows;

    /* This one is used to backup pd_Color if left is clicked. This
    ** way the old state can be restored if the left is released
    ** outside the gadget.
    */
    UBYTE	pd_ColorBackup;
    struct TextAttr *pd_TAttr;
    LONG	pd_LabelPlace;
    Object	*pd_Frame;
    
};


/**************
**  Defines  **
**************/
#undef EG
#define EG(o) ((struct ExtGadget *)o)



#define HSPACING	2
#define VSPACING	3

#define HBORDER	HSPACING
#define VBORDER (VSPACING - 1)


#define HSELBORDER	1
#define VSELBORDER	1

/*****************
**  Prototypes  **
*****************/

UWORD GetPalettePen(struct PaletteData *, UWORD);
UBYTE Colors2Depth(UWORD);

VOID RenderFrame(struct PaletteData *, struct RastPort *, struct IBox *,
		struct DrawInfo *, BOOL, BOOL, struct PaletteBase_intern *);

VOID RenderPalette(struct PaletteData *, struct RastPort *,
		struct PaletteBase_intern *);


VOID UpdateActiveColor( struct PaletteData *, struct DrawInfo *,
 			struct RastPort *, struct PaletteBase_intern *);

VOID GetGadgetIBox(Object *, struct GadgetInfo *, struct IBox *);
UWORD ComputeColor(struct PaletteData *, WORD, WORD);
BOOL InsidePalette(struct PaletteData *, WORD, WORD);

void DrawDisabledPattern(struct RastPort *, struct IBox *, UWORD,
			 struct PaletteBase_intern *);


struct TextFont *PrepareFont(struct RastPort *, struct IntuiText *,
		struct TextFont **, struct PaletteBase_intern *);


void DisposeFont(struct RastPort *, struct TextFont *, struct TextFont *,
		struct PaletteBase_intern *);
			 
BOOL RenderLabel( struct Gadget *gad, struct IBox *, 
		 struct RastPort *, LONG labelplace,
                 struct PaletteBase_intern *);

/********************
**  Library stuff  **
********************/
struct PaletteBase_intern
{
    struct Library 	library;
    struct ExecBase	*sysbase;
    BPTR		seglist;

    #ifndef GLOBAL_INTUIBASE
    struct IntuitionBase *intuitionbase;
    #endif
    struct GfxBase	*gfxbase;
    struct Library	*utilitybase;
    
    struct IClass	*classptr;
	
};

/* The following typedefs are necessary, because the names of the global
   variables storing the library base pointers	and the corresponding
   structs are equal.
   This is a hack, of course. */
typedef struct GfxBase GraphicsBase;
typedef struct IntuitionBase IntuiBase;

#undef PB
#define PB(b) ((struct PaletteBase_intern *)b)
#undef UtilityBase
#define UtilityBase 	PB(AROSPaletteBase)->utilitybase


#ifndef GLOBAL_INTUIBASE
#undef IntuitionBase
#define IntuitionBase	PB(AROSPaletteBase)->intuitionbase
#endif

#undef GfxBase
#define GfxBase		PB(AROSPaletteBase)->gfxbase
#undef SysBase
#define SysBase		PB(AROSPaletteBase)->sysbase


#define expunge() \
AROS_LC0(BPTR, expunge, struct LVBase_intern *, AROSPaletteBase, 3, AROSPalette)

#endif /* AROSPALETTE_INTERN_H */
