/*
    Copyright � 1995-2001, The AROS Development Team. All rights reserved.
    $Id$

    Desc:
    Lang: english
*/
#include "keymap_intern.h"

#include <aros/debug.h>
#define DEBUG_MAPANSI(x) D(x)

/*****************************************************************************

    NAME */
#include <clib/keymap_protos.h>

	AROS_LH5(LONG, MapANSI,

/*  SYNOPSIS */
	AROS_LHA(STRPTR         , string, A0),
	AROS_LHA(LONG           , count, D0),
	AROS_LHA(STRPTR         , buffer, A1),
	AROS_LHA(LONG           , length, D1),
	AROS_LHA(struct KeyMap *, keyMap, A2),

/*  LOCATION */
	struct Library *, KeymapBase, 8, Keymap)

/*  FUNCTION
    	Convert an ANSI byte string to rawkey codes
	
    INPUTS
    	string - the ANSI byte string to convert

	count - the number of characters in string

	buffer - a byte buffer which must be large enough
	    	 to hold all anticipated code/qualifier pairs
		 which are generated by this function

    	length - maximum anticipation, ie. the buffer size in
	         WORDs (buffer size in bytes divided by two).
		 WORDs, because one code/qualifier pair consists
		 of 2 bytes.
	
    RESULT
    	actual - the number of code/qualifier pairs this function
	         generated in buffer. Or an negative value to indicate
		 an error.
    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS
    	Contributed by MorphOS team

    HISTORY
	27-11-96    digulla automatically created from
			    keymap_lib.fd and clib/keymap_protos.h

*****************************************************************************/
{
    AROS_LIBFUNC_INIT
    AROS_LIBBASE_EXT_DECL(struct Library *,KeymapBase)

    LONG	OrigLength=length;
    int		MaxDeadIndex=0;
    int		MaxDoubleDeadIndex=0;
    int		NumDeads=0;
    UBYTE	DeadCode[16];
    UBYTE	DeadQual[16];
    UBYTE	DoubleDeadCode[16];
    UBYTE	DoubleDeadQual[16];
    UBYTE	*Types;
    IPTR	*Descrs;
    int		BaseCode;
    int		Code;
    int		k;

    static const UBYTE NumKeys[] = { 1, 2, 2, 4, 2, 4, 4, 4 };
    static const UWORD Qualifiers[8][4] =
    {
      { 0, },
      { 0, IEQUALIFIER_LSHIFT, },
      { 0, IEQUALIFIER_LALT, },
      { 0, IEQUALIFIER_LSHIFT, IEQUALIFIER_LALT, IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT },
      { 0, IEQUALIFIER_CONTROL, },
      { 0, IEQUALIFIER_LSHIFT, IEQUALIFIER_CONTROL, IEQUALIFIER_LSHIFT | IEQUALIFIER_CONTROL },
      { 0, IEQUALIFIER_LALT, IEQUALIFIER_CONTROL, IEQUALIFIER_LALT | IEQUALIFIER_CONTROL },
      { 0, IEQUALIFIER_LSHIFT, IEQUALIFIER_LALT, IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT },
    };
    static const UBYTE NumKeys2[] = { 1, 2, 2, 4, 2, 4, 4, 8 };
    static const UWORD Qualifiers2[8][8] =
    {
      { 0, },
      { 0, IEQUALIFIER_LSHIFT, },
      { 0, IEQUALIFIER_LALT, },
      { 0, IEQUALIFIER_LSHIFT, IEQUALIFIER_LALT, IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT },
      { 0, IEQUALIFIER_CONTROL, },
      { 0, IEQUALIFIER_LSHIFT, IEQUALIFIER_CONTROL, IEQUALIFIER_LSHIFT | IEQUALIFIER_CONTROL },
      { 0, IEQUALIFIER_LALT, IEQUALIFIER_CONTROL, IEQUALIFIER_LALT | IEQUALIFIER_CONTROL },
      { 0, IEQUALIFIER_LSHIFT, IEQUALIFIER_LALT, IEQUALIFIER_LSHIFT | IEQUALIFIER_LALT,
      IEQUALIFIER_CONTROL, IEQUALIFIER_CONTROL | IEQUALIFIER_LSHIFT,
      IEQUALIFIER_CONTROL | IEQUALIFIER_LALT,
      IEQUALIFIER_CONTROL | IEQUALIFIER_LALT | IEQUALIFIER_LSHIFT },
    };

    DEBUG_MAPANSI(bug("MapANSI: string 0x%lx count %ld buffer 0x%lx length %ld KeyMap 0x%lx\n",
			  string, count, buffer, length, keyMap));

    if (keyMap == NULL)
    {
       keyMap = KMBase(KeymapBase)->DefaultKeymap;
       DEBUG_MAPANSI(bug("MapANSI: KeyMap 0x%lx\n", keyMap));
    }


    /* First get the list of dead keys.
     */

    Types		=	keyMap->km_HiKeyMapTypes + (0x68 - 0x40);
    Descrs	=	keyMap->km_HiKeyMap + (0x68 - 0x40);
    BaseCode	=	0x40;
    Code		=	0x67 - 0x40;

    for (k = 0; k < sizeof(DeadQual); ++k)
    {
      DeadQual[k]		=	0xff;
      DoubleDeadQual[k]	=	0xff;
    }

    do
    {
      do
      {
	unsigned	Type = *--Types;
	IPTR	Descr = *--Descrs;

	DEBUG_MAPANSI(bug("MapANSI: Code 0x%02x Type 0x%02x Descr 0x%08lx\n",
			      BaseCode + Code, Type, Descr));

	if ((Type & KCF_NOP) == 0 && (Type & KCF_DEAD) != 0)
	{
	  int	Num = NumKeys2[Type & 7];
	  int	k;
	  UBYTE	*StrDescr = (UBYTE *)Descr;

	  for (k = 0; k < Num; ++k)
	  {
	    if (StrDescr[2 * k] == DPF_DEAD)
	    {
	      int		Index = StrDescr[2 * k + 1] & DP_2DINDEXMASK;
	      int		DoubleDead = StrDescr[2 * k + 1] >> DP_2DFACSHIFT;
	      UBYTE	MyQual = Qualifiers2[Type & 7][k];

	      DEBUG_MAPANSI(bug("MapANSI: found dead key index 0x%02lx at 0x%02lx\n",
				    StrDescr[2 * k + 1], BaseCode + Code));

	      if (Index > MaxDeadIndex ||
		  (DeadQual[Index] & MyQual) == MyQual)
	      {
		DeadCode[Index]	=	BaseCode + Code;
		DeadQual[Index]	=	MyQual;

		if (Index > MaxDeadIndex)
		{
		  MaxDeadIndex	=	Index;
		}
	      }

	      if (DoubleDead)
	      {
		if (Index > MaxDoubleDeadIndex ||
		    (DoubleDeadQual[Index] & MyQual) == MyQual)
		{
		  DoubleDeadCode[Index]	=	BaseCode + Code;
		  DoubleDeadQual[Index]	=	MyQual;

		  if (Index > MaxDoubleDeadIndex)
		  {
		    MaxDoubleDeadIndex	=	  Index;
		  }
		}
	      }
	    }
	  }
	}
      }
      while (--Code >= 0);

      Types	=	keyMap->km_LoKeyMapTypes + 0x40;
      Descrs	=	keyMap->km_LoKeyMap + 0x40;
      BaseCode	-=	0x40;
      Code	=	0x3f;
    }
    while (BaseCode >= 0);

    NumDeads	=	(MaxDoubleDeadIndex + 1) * (MaxDeadIndex + 1);

    DEBUG_MAPANSI(bug("MapANSI: MaxDeadIndex %ld MaxDoubleDeadIndex %ld NumDeads %ld\n",
			  MaxDeadIndex, MaxDoubleDeadIndex, NumDeads));

    /* Now do the real work.
     */
    while (count)
    {
      LONG	FoundLen = 0;
      unsigned	FoundCode;
      unsigned	FoundQual = ~0U;
      unsigned	DeadKeyIndex = 0;
      UBYTE	MyChar = *string;

      DEBUG_MAPANSI(bug("MapANSI: MyChar 0x%02lx count %ld\n",
			    MyChar, count));

      Types	=	keyMap->km_HiKeyMapTypes + (0x68 - 0x40);
      Descrs	=	keyMap->km_HiKeyMap + (0x68 - 0x40);
      BaseCode	=	0x40;
      Code	=	0x67 - 0x40;

      do
      {
	do
	{
	  unsigned	Type = *--Types;
	  IPTR		Descr = *--Descrs;

	  if (Type & KCF_NOP)
	  {
	  }
	  else if (Type & KCF_DEAD)
	  {
	    if (FoundLen <= 1)
	    {
	      int		Num = NumKeys2[Type & 7];
	      int		k;
	      UBYTE	*StrDescr = (UBYTE *)Descr;

	      for (k = 0; k < Num; ++k)
	      {
		switch (StrDescr[2 * k])
		{
		 case 0:
		  if (StrDescr[2 * k + 1] == MyChar)
		  {
		    unsigned	MyQual = Qualifiers2[Type & 7][k];

		    if ((FoundQual & MyQual) == MyQual)
		    {
		      FoundLen		=	1;
		      FoundCode		=	BaseCode + Code;
		      FoundQual		=	MyQual;
		      DeadKeyIndex	=	0;
		    }
		  }
		  break;

		 case DPF_MOD:
		  {
		    UBYTE	*DeadKeys = StrDescr + StrDescr[2 * k + 1];
		    int	l;

		    for (l = 0; l < NumDeads; ++l)
		    {
		      if (DeadKeys[l] == MyChar)
		      {
			unsigned	MyQual = Qualifiers2[Type & 7][k];

			if (FoundLen == 0 ||
			    (l <= DeadKeyIndex &&
			     (FoundQual & MyQual) == MyQual))
			{
			  FoundLen	=	1;
			  FoundCode	=	BaseCode + Code;
			  FoundQual	=	MyQual;
			  DeadKeyIndex	=	l;
			}
		      }
		    }
		  }
		  break;
		}
	      }
	    }
	  }
	  else if (Type & KCF_STRING)
	  {
	    int	Num = NumKeys2[Type & 7];
	    int	k;
	    UBYTE *StrDescr = (UBYTE *)Descr;

	    for (k = 0; k < Num; ++k)
	    {
	      int		Len = StrDescr[2 * k];
	      UBYTE	*KeyStr = StrDescr + StrDescr[2 * k + 1];

	      if (Len <= count && Len >= FoundLen)
	      {
		int	i = 0;

		while (i < Len && string[i] == KeyStr[i])
		{
                  ++i;
		}

		if (i == Len)
		{
		  unsigned	MyQual = Qualifiers2[Type & 7][k];

		  if (Len > FoundLen || DeadKeyIndex ||
		      (FoundQual & MyQual) == MyQual)
		  {
		    FoundLen	=	Len;
		    FoundCode	=	BaseCode + Code;
		    FoundQual	=	MyQual;
		    DeadKeyIndex	=	0;
		  }
		}
	      }
	    }
	  }
	  else if (FoundLen <= 1)
	  {
	    int		Num = NumKeys[Type & 7];
	    int		k;
	    IPTR		Descr2 = Descr;

	    for (k = 0; k < Num; ++k)
	    {
	      if ((UBYTE)Descr == MyChar)
	      {
		unsigned	MyQual = Qualifiers[Type & 7][k];

		if (DeadKeyIndex || (FoundQual & MyQual) == MyQual)
		{
		  FoundLen	=	1;
		  FoundCode	=	BaseCode + Code;
		  FoundQual	=	MyQual;
		  DeadKeyIndex	=	0;
		}
		break;
	      }
	      Descr	>>=	8;
	    }

	    if (Type == KC_VANILLA && (MyChar & 0x60) == 0 &&
		(FoundLen == 0 || DeadKeyIndex != 0))
	    {
	      UBYTE	MyQual = IEQUALIFIER_CONTROL;

	      if (MyChar & 0x80)
	      {
		MyQual	|=	IEQUALIFIER_LALT;
		Descr2	=	(Descr2 >> 16) | (Descr2 << 16);
	      }

	      if ((Descr2 & 0xc0) == 0x40 && (Descr2 & 0x1f) != (MyChar & 0x1f))
	      {
		MyQual	|=	IEQUALIFIER_LSHIFT;
		Descr2	>>=	8;
	      }

	      k	=	0;
	      while (k < 4 && (Descr2 & 0xc0) != 0x40)
	      {
		++k;
		Descr2	>>=	8;
	      }

	      if (k < 4 && (Descr2 & 0x1f) == (MyChar & 0x1f))
	      {
		FoundLen		=	1;
		FoundCode		=	BaseCode + Code;
		FoundQual		=	MyQual;
		DeadKeyIndex	=	0;
	      }
	    }
	  }
	}
	while (--Code >= 0);

	Types	=	keyMap->km_LoKeyMapTypes + 0x40;
	Descrs	=	keyMap->km_LoKeyMap + 0x40;
	BaseCode	-=	0x40;
	Code	=	0x3f;
      }
      while (BaseCode >= 0);

      if (FoundLen == 0)
      {
	DEBUG_MAPANSI(bug("MapANSI: not found\n"));
	return 0;
      }

      length	-=	1 + (DeadKeyIndex != 0) + (DeadKeyIndex > MaxDeadIndex);
      if (length < 0)
      {
	DEBUG_MAPANSI(bug("MapANSI: buffer overflow\n"));
	return -1;
      }

      DEBUG_MAPANSI(bug("MapANSI: FoundLen %ld Code 0x%02lx Qual 0x%02lx DeadKeyIndex %ld\n",
			    FoundLen, FoundCode, FoundQual, DeadKeyIndex));

      if (DeadKeyIndex != 0)
      {
	if (DeadKeyIndex > MaxDeadIndex)
	{
	  *buffer++	=	DoubleDeadCode[DeadKeyIndex % (MaxDeadIndex + 1)];
	  *buffer++	=	DoubleDeadQual[DeadKeyIndex % (MaxDeadIndex + 1)];
	  DeadKeyIndex	/=	(MaxDeadIndex + 1);
	  *buffer++	=	DoubleDeadCode[DeadKeyIndex];
	  *buffer++	=	DoubleDeadQual[DeadKeyIndex];
	}
	else
	{
	  *buffer++	=	DeadCode[DeadKeyIndex];
	  *buffer++	=	DeadQual[DeadKeyIndex];
	}
      }
      *buffer++	=	FoundCode;
      *buffer++	=	FoundQual;

      count	-=	FoundLen;
      string		+=	FoundLen;
    }

    return OrigLength - length;

    AROS_LIBFUNC_EXIT

} /* MapANSI */
