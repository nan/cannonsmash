/***********************************************************************
	d:/users/wata/src/include/minwin32.h
	$Id$

	Copyright by ESESoft.
	You are granted the right to redistribute this file under
	the "BSD artistic license". See "ARTISTIC" for detail.
 ***********************************************************************/
#ifndef __wata_ESESoft__minwin32_h__INCLUDED_
#define __wata_ESESoft__minwin32_h__INCLUDED_
/***********************************************************************/
/* __BEGIN__BEGIN__ */
#ifdef __cplusplus
#ifndef NOMINMAX
#define NOMINMAX
#endif
#endif

#define NOGDI
#define NONLS
#define NOUSER
#define NOSOUND
#define NOKERNEL
#define NOSERVICE
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN

#include <windows.h>

#ifndef snprintf
# define snprintf _snprintf
# define vsnprintf _vsnprintf
#endif

/* __END__END__ */
/***********************************************************************/
#endif
/***********************************************************************
 *	END OF minwin32.h
 ***********************************************************************/
