/***********************************************************************
	d:/users/wata/src/ese/src/std/wsainit.cpp
	$Id$

	Copyright by ESESoft.
	You are granted the right to redistribute this file under
	the "BSD artistic license". See "ARTISTIC" for detail.
 ***********************************************************************/
#include "minwin32.h"
#include <winsock2.h>

#define MAJOR 2
#define MINOR 0

namespace {
    char __rcsid__[] = "$Id$";

    class wsainit
    {
    private:
	static bool initialized;
    public:
	inline wsainit(int major, int minor) {
	    if (initialized) return;
	    WSADATA wsadata;
	    if (WSAStartup(MAKEWORD(major,minor), &wsadata)) {
		initialized = true;
	    }
	}
	inline ~wsainit() {
	    if (initialized) {
		WSACleanup();
		initialized = false;
	    }
	}
    };

    bool wsainit::initialized = false;
    wsainit __wsainit__(MAJOR, MINOR); /* constructor will run before main */
}
