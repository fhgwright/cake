/*
    Copyright (C) 2000 AROS - The Amiga Research OS
    $Id$

    Desc: Varargs version of graphics.library/GetExtSpriteA()
    Lang: english
*/
#define AROS_TAGRETURNTYPE LONG
#include <graphics/sprite.h>
#include <utility/tagitem.h>

/*****************************************************************************

    NAME */
#define NO_INLINE_STDARG /* turn off inline def */
#include <proto/graphics.h>

	LONG GetExtSprite (

/*  SYNOPSIS */
	struct ExtSprite * sprite,
	Tag tag1,
	...)

/*  FUNCTION
        This is the varargs version of graphics.library/GetExtSpriteA().
        For information see graphics.library/GetExtSpriteA().

    INPUTS

    RESULT

    NOTES

    EXAMPLE

    BUGS

    SEE ALSO
        graphics/GetExtSpriteA()

    INTERNALS

    HISTORY

*****************************************************************************/
{
    AROS_SLOWSTACKTAGS_PRE(tag1)
    GetExtSpriteA (sprite, AROS_SLOWSTACKTAGS_ARG(tag1));
    AROS_SLOWSTACKTAGS_POST
} /* GetExtSprite */
