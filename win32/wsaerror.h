/***********************************************************************
	d:/users/wata/src/ese/src/std/wsaerror.h
	$Id$

	Copyright by ESESoft.
	You are granted the right to redistribute this file under
	the "BSD artistic license". See "ARTISTIC" for detail.
 ***********************************************************************/
#ifndef __wata_ESESoft_0308__wsaerror_h__INCLUDED__
#define __wata_ESESoft_0308__wsaerror_h__INCLUDED__
/***********************************************************************/
/* __BEGIN__BEGIN__ */
#ifndef __cplusplus
/* This is C code */
const char *wsaerrorstring(int err);

#else
/* This is C++ code */
extern "C" const char *wsaerrorstring(int err);

class wsa_error
{
public:
    int rr;

    inline explicit wsa_error(int rr) throw() : rr(rr) {}
    inline wsa_error() throw() : rr(WSAGetLastError()) {}
};

/*__napi__*/ std::ostream & operator <<(std::ostream &, const wsa_error &);

#endif

/* __END__END__ */
/***********************************************************************/
#endif
/***********************************************************************
 *	END OF wsaerror.h
 ***********************************************************************/
