/*
    (C) 1995-97 AROS - The Amiga Replacement OS
    $Id$

    Desc:
    Lang: english
*/


#include <libraries/mathffp.h>
#include <aros/libcall.h>
#include <proto/mathffp.h>
#include <proto/mathtrans.h>
#include <proto/exec.h>
#include <exec/types.h>
#include "mathtrans_intern.h"


/*****************************************************************************

    NAME */

      AROS_LH1(LONG, SPLog,

/*  SYNOPSIS */

      AROS_LHA(LONG, fnum1, D0),

/*  LOCATION */

      struct MathtransBase *, MathtransBase, 14, Mathtrans)

/*  FUNCTION

      Calculate logarithm (base 10) of the given ffp number

    INPUTS

      fnum1 - Motorola floating point number

    RESULT

      ffp-number

      flags:
        zero     : result is zero
        negative : result is negative
        overflow : argument was negative

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO

    INTERNALS
      ALGORITHM:

      If the Argument is negative set overflow-flag and return 0.
      If the Argument is 0 return 0xffffffff.

      All other cases:

      (ld is the logarithm with base 2)
      (ln is the logarithm with base e)
      fnum1 = M * 2^E

      ln fnum1 = ln ( M * 2^E ) =

               = ln M + ln 2^E =

               = ln M + E * ln (2) =

                 ld M        ld 2
               = ----- + E * ----- =      [ld 2 = 1]
                 ld e        ld e

                 ld M + E
               = --------
                 ld e

      ld e can be precalculated, of course.
      For calculating ld M see file intern_spld.c

    HISTORY

******************************************************************************/

{
AROS_LIBFUNC_INIT
  LONG ld_M, Exponent, Mask = 0x40, i, Sign;
  /* check for negative sign */
  if ((char) fnum1 < 0)
  {
    SetSR(Overflow_Bit, Zero_Bit | Negative_Bit | Overflow_Bit);
    return 0;
  }

  /* check for argument == 0 -> return (-infinity) */
  if (fnum1 == 0)
    return FFP_Ninfty;

  /* convert the Exponent of the argument (fnum1) to the ffp-format */
  Exponent = (fnum1 & FFPExponent_Mask) - 0x40;
  if (Exponent < 0 )
  {
    Exponent =-Exponent;
    Sign = FFPSign_Mask;
  }
  else
    Sign = 0;

  /* find the number of the highest set bit in the exponent */
  if (Exponent != 0)
  {
    i = 0;
    while ( (Mask & Exponent) == 0)
    {
      i ++;
      Mask >>= 1;
    }
  Exponent <<= (25 + i);
  Exponent |= (0x47 - i + Sign);
  }

  ld_M = intern_SPLd((struct MathtransBase *)MathtransBase, 
                     (fnum1 & FFPMantisse_Mask) | 0x40);

  /*               ld M + E
  ** log(fnum1) =  --------
  **                 ld e
  */

  return SPMul( SPAdd(ld_M, Exponent), InvLde);
AROS_LIBFUNC_EXIT
} /* SPLog */
