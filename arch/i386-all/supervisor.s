#    (C) 1995-96 AROS - The Amiga Replacement OS
#    $Id$
#    $Log$
#    Revision 1.3  1996/08/13 14:03:20  digulla
#    Added standard headers
#
#    Revision 1.2  1996/08/01 17:41:26	digulla
#    Added standard header for all files
#
#    Desc:
#    Lang:

#*****************************************************************************
#
#   NAME
#	__AROS_LH0(void, Switch,
#
#   LOCATION
#	struct ExecBase *, SysBase, 6, Exec)
#
#   FUNCTION
#
#   INPUTS
#
#   RESULT
#
#   NOTES
#
#   EXAMPLE
#
#   BUGS
#
#   SEE ALSO
#
#   INTERNALS
#
#   HISTORY
#
#******************************************************************************
	.text
	.align	16
	.globl	Exec_Supervisor
	.type	Exec_Supervisor,@function
Exec_Supervisor:
	/* The emulation has no real supervisor mode. */
	subl	$4,%esp
	pushl	%eax
	movl	12(%esp),%eax
	movl	%eax,4(%esp)
	popl	%eax
	ret
