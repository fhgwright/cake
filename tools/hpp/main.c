#include <stdio.h>
#include <stdiocb.h>
#include <error.h>
#include "html.h"
#include "parse.h"
#include "var.h"

void main (int argc, char ** argv)
{
    StdioStream * ss;
    int t, rc;

    Var_Init ();
    HTML_Init ();

    Var_Set ("outputFormat", "html");

    for (t=1; t<argc; t++)
    {
	ss = StdStr_New (argv[t], "r");

	if (!ss)
	    PrintErrorStack ();
	else
	{
	    rc = HTML_Parse (StdioGetCharCB, ss, NULL);

	    if (rc == T_ERROR)
	    {
		PushError ("%s:%d:", StdStr_GetName (ss), StdStr_GetLine (ss));
		PrintErrorStack ();
	    }

	    StdStr_Delete (ss);
	} /* if (ss) */
    } /* for all args */

    HTML_Exit ();
    Var_Exit ();

    ErrorExit (0);
}

