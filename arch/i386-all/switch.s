#    (C) 1995-96 AROS - The Amiga Replacement OS
#    $Id$
#    $Log$
#    Revision 1.9  1996/10/24 15:51:13  aros
#    Use the official AROS macros over the __AROS versions.
#
#    Revision 1.8  1996/10/23 08:04:26  aros
#    Use generated offsets which makes porting much easier
#
#    Revision 1.7  1996/09/11 16:54:30	digulla
#    Always use __AROS_SLIB_ENTRY() to access shared external symbols, because
#	some systems name an external symbol "x" as "_x" and others as "x".
#	(The problem arises with assembler symbols which might differ)
#
#    Revision 1.6  1996/09/11 14:41:17	digulla
#    Removed tick which made problems with CPP
#    Closed comment
#
#    Revision 1.5  1996/08/23 16:49:22	digulla
#    With some systems, .align 16 aligns to 64K instead of 16bytes. Therefore
#	I replaced it with .balign which does what we want.
#
#    Revision 1.4  1996/08/13 14:03:20	digulla
#    Added standard headers
#
#    Revision 1.3  1996/08/01 17:48:52	digulla
#    Added description
#
#    Revision 1.2  1996/08/01 17:41:26	digulla
#    Added standard header for all files
#
#    Desc:
#    Lang:

#*****************************************************************************
#
#   NAME
#
#	AROS_LH0(void, Switch,
#
#   LOCATION
#	struct ExecBase *, SysBase, 6, Exec)
#
#   FUNCTION
#	Tries to switch to the first task in the ready list. This
#	function works almost like Dispatch() with the slight difference
#	that it may be called at any time and as often as you want and
#	that it does not lose the current task if it is of type TS_RUN.
#
#   INPUTS
#
#   RESULT
#
#   NOTES
#	This function is CPU dependant.
#
#	This function is for internal use by exec only.
#
#	This function preserves all registers.
#
#   EXAMPLE
#
#   BUGS
#
#   SEE ALSO
#	Dispatch()
#
#   INTERNALS
#
#   HISTORY
#
#******************************************************************************

	.include "machine.i"

	.text
	.balign 16
	.globl	_Exec_Switch
	.type	_Exec_Switch,@function
_Exec_Switch:
	/* Make room for Dispatch() address. */
	subl	$4,%esp

	/* Preserve registers */
	pushl	%eax
	pushl	%ebx
	pushl	%ecx
	pushl	%edx

	/* Get SysBase */
	movl	24(%esp),%ebx

	/* If current state is TS_RUN and TF_EXCEPT is 0... */
	movl	ThisTask(%ebx),%ecx
	movw	tc_Flags(%ecx),%eax
	andb	$TF_EXCEPT,%al
	cmpw	$TS_RUN*256,%ax
	jne	disp

	/* ...move task to the ready list */
	movb	$TS_READY,tc_State(%ecx)
	leal	Enqueue(%ebx),%edx
	pushl	%ebx
	pushl	%ecx
	leal	TaskReady(%ebx),%eax
	pushl	%eax
	call	*%edx
	addl	$12,%esp

	/* Prepare dispatch */
disp:	leal	Dispatch(%ebx),%eax
	movl	%eax,16(%esp)

	/* restore registers and dispatch */
	popl	%edx
	popl	%ecx
	popl	%ebx
	popl	%eax
	ret
