/***********************************************************************
	d:/users/wata/src/ese/src/std/GetArgs.h
	$Id$

	Copyright by ESESoft.
	You are granted the right to redistribute this file under
	the "BSD artistic license". See "ARTISTIC" for detail.
 ***********************************************************************/
#ifndef __wata_ESESoft_8402__GetArgs_h__INCLUDED__
#define __wata_ESESoft_8402__GetArgs_h__INCLUDED__
/***********************************************************************/
/* __BEGIN__BEGIN__ */

#define GETARGS_ESCAPE_UNIX '\\'
#define GETARGS_ESCAPE_CMD '^'

int GetArgs(int *argc_return, char ***argv_return, const char *cmdline = NULL,
	    int escape_char = 0);

/* __END__END__ */
/***********************************************************************/
#endif
/***********************************************************************
 *	END OF GetArgs.h
 ***********************************************************************/

