/* 
    Copyright � 2002-2003, The AROS Development Team. All rights reserved.
    $Id$
*/

#ifndef _MUI_CLASSES_BALANCE_H
#define _MUI_CLASSES_BALANCE_H

/*** Name *******************************************************************/
#define MUIC_Balance        "Balance.mui"

/*** Identifier base (for Zune extensions) **********************************/
#define MUIB_Balance        (MUIB_ZUNE | 0x00000300)

/*** Attributes *************************************************************/
/* undocumented attr, gave it a random tag value -dlc */
#define MUIA_Balance_Quiet  (MUIB_MUI|0x00426688) /* V20 i   LONG */

extern const struct __MUIBuiltinClass _MUI_Balance_desc; /* PRIV */

#endif /* _MUI_CLASSES_BALANCE_H */
