/***********************************************************************
 *	d:/Werk/src/csmash-0.3.8.new/conv/win32.h
 *	$Id$
 *
 *	Copyright by ESESoft.
 *	You are granted the right to redistribute this file under
 *	the "Artistic license". See "ARTISTIC" for detail.
 *
 ***********************************************************************/
#ifndef __wata_ESESoft_2635__win32_h__INCLUDED__
#define __wata_ESESoft_2635__win32_h__INCLUDED__
/***********************************************************************/
/* __BEGIN__BEGIN__ */
#ifdef __cplusplus
#define NOMINMAX
#endif
#define NOGDI
/*#define NONLS /* */
#define NOUSER
#define NOKERNEL
#define NOSERVICE
#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#define snprintf _snprintf
#define vsnprintf _vsnprintf

/* __END__END__ */
/***********************************************************************/
#endif
/***********************************************************************
 *	END OF win32.h
 ***********************************************************************/
