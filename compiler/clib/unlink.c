/*
    (C) 1995-96 AROS - The Amiga Replacement OS
    $Id$

    Desc: Delete a file.
    Lang: english
*/
#include <proto/dos.h>
#include "__errno.h"

/*****************************************************************************

    NAME */
#include <string.h>

	int unlink (

/*  SYNOPSIS */
	const char * pathname)

/*  FUNCTION
	Delete a file from disk.

    INPUTS
	pathname - Complete path to the file

    RESULT
	0 on success and -1 on error. In case of an error, errno is set.

    NOTES

    EXAMPLE
	// Delete the file xyz in the current directory
	unlink ("xyz");

    BUGS

    SEE ALSO

    INTERNALS

    HISTORY

******************************************************************************/
{
    if (!DeleteFile ((STRPTR)pathname))
    {
	errno = IoErr2errno (IoErr());
	return -1;
    }

    return 0;
} /* unlink */
