/***********************************************************************
 *	csmash/glARB.cpp
 *	$Id$
 *
 *	Copyright by ESESoft.
 *
 *	Redistribution and use in source and binary forms, with or without
 *	modification, are permitted provided that the following conditions
 *	are met:
 *
 *	Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer. 
 *
 *	Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer
 *	in the documentation and/or other materials provided with the
 *	distribution. 
 *
 *	The name of the author may not be used to endorse or promote
 *	products derived from this software without specific prior written
 *	permission. 
 *
 *	THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 *	OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *	ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *	DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 *	GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *	WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***********************************************************************/
#include "ttinc.h"

#if defined(GL_ARB_multitexture)
# define GL_H_HAS_ARB_multitexture 1
#endif

#include "glARB.h"

#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#  define for if(0);else for
#endif

/***********************************************************************
 *	Local data and functions
 ***********************************************************************/

#ifdef _WIN32
# define glGetProcAddr(A) wglGetProcAddress(A)
#else
# define glGetProcAddr(A) glXGetProcAddressARB(A)
#endif

/***********************************************************************/

#if !defined(GL_H_HAS_ARB_multitexture)
// gl.h does not have GL_ARB_multitexture, so it has to be initialized

extern "C" {
PFNGLACTIVETEXTUREARBPROC glActiveTextureARB = 0;
PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB = 0;
PFNGLMULTITEXCOORD1DARBPROC glMultiTexCoord1dARB = 0;
PFNGLMULTITEXCOORD1DVARBPROC glMultiTexCoord1dvARB = 0;
PFNGLMULTITEXCOORD1FVARBPROC glMultiTexCoord1fARB = 0;
PFNGLMULTITEXCOORD1FVARBPROC glMultiTexCoord1fvARB = 0;
PFNGLMULTITEXCOORD1IARBPROC glMultiTexCoord1iARB = 0;
PFNGLMULTITEXCOORD1IVARBPROC glMultiTexCoord1ivARB = 0;
PFNGLMULTITEXCOORD1SARBPROC glMultiTexCoord1sARB = 0;
PFNGLMULTITEXCOORD1SVARBPROC glMultiTexCoord1svARB = 0;
PFNGLMULTITEXCOORD2DARBPROC glMultiTexCoord2dARB = 0;
PFNGLMULTITEXCOORD2DVARBPROC glMultiTexCoord2dvARB = 0;
PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB = 0;
PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB = 0;
PFNGLMULTITEXCOORD2IARBPROC glMultiTexCoord2iARB = 0;
PFNGLMULTITEXCOORD2IVARBPROC glMultiTexCoord2ivARB = 0;
PFNGLMULTITEXCOORD2SARBPROC glMultiTexCoord2sARB = 0;
PFNGLMULTITEXCOORD2SVARBPROC glMultiTexCoord2svARB = 0;
PFNGLMULTITEXCOORD3DARBPROC glMultiTexCoord3dARB = 0;
PFNGLMULTITEXCOORD3DVARBPROC glMultiTexCoord3dvARB = 0;
PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB = 0;
PFNGLMULTITEXCOORD3FVARBPROC glMultiTexCoord3fvARB = 0;
PFNGLMULTITEXCOORD3IARBPROC glMultiTexCoord3iARB = 0;
PFNGLMULTITEXCOORD3IVARBPROC glMultiTexCoord3ivARB = 0;
PFNGLMULTITEXCOORD3SARBPROC glMultiTexCoord3sARB = 0;
PFNGLMULTITEXCOORD3SVARBPROC glMultiTexCoord3svARB = 0;
PFNGLMULTITEXCOORD4DARBPROC glMultiTexCoord4dARB = 0;
PFNGLMULTITEXCOORD4DVARBPROC glMultiTexCoord4dvARB = 0;
PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB = 0;
PFNGLMULTITEXCOORD4FVARBPROC glMultiTexCoord4fvARB = 0;
PFNGLMULTITEXCOORD4IARBPROC glMultiTexCoord4iARB = 0;
PFNGLMULTITEXCOORD4IVARBPROC glMultiTexCoord4ivARB = 0;
PFNGLMULTITEXCOORD4SARBPROC glMultiTexCoord4sARB = 0;
PFNGLMULTITEXCOORD4SVARBPROC glMultiTexCoord4svARB = 0;
} /* extern "C" */

extern "C"
int initglARBmultitexture()
{
#define LOAD(A, R) \
    do { \
      *(void**)&A = (void*)glGetProcAddr(#A); \
      R &= (A!=0); \
    } while (0)

    bool r = true;

    LOAD(glActiveTextureARB, r);
    LOAD(glClientActiveTextureARB, r);
    LOAD(glMultiTexCoord1dARB, r);
    LOAD(glMultiTexCoord1dvARB, r);
    LOAD(glMultiTexCoord1fARB, r);
    LOAD(glMultiTexCoord1fvARB, r);
    LOAD(glMultiTexCoord1iARB, r);
    LOAD(glMultiTexCoord1ivARB, r);
    LOAD(glMultiTexCoord1sARB, r);
    LOAD(glMultiTexCoord1svARB, r);
    LOAD(glMultiTexCoord2dARB, r);
    LOAD(glMultiTexCoord2dvARB, r);
    LOAD(glMultiTexCoord2fARB, r);
    LOAD(glMultiTexCoord2fvARB, r);
    LOAD(glMultiTexCoord2iARB, r);
    LOAD(glMultiTexCoord2ivARB, r);
    LOAD(glMultiTexCoord2sARB, r);
    LOAD(glMultiTexCoord2svARB, r);
    LOAD(glMultiTexCoord3dARB, r);
    LOAD(glMultiTexCoord3dvARB, r);
    LOAD(glMultiTexCoord3fARB, r);
    LOAD(glMultiTexCoord3fvARB, r);
    LOAD(glMultiTexCoord3iARB, r);
    LOAD(glMultiTexCoord3ivARB, r);
    LOAD(glMultiTexCoord3sARB, r);
    LOAD(glMultiTexCoord3svARB, r);
    LOAD(glMultiTexCoord4dARB, r);
    LOAD(glMultiTexCoord4dvARB, r);
    LOAD(glMultiTexCoord4fARB, r);
    LOAD(glMultiTexCoord4fvARB, r);
    LOAD(glMultiTexCoord4iARB, r);
    LOAD(glMultiTexCoord4ivARB, r);
    LOAD(glMultiTexCoord4sARB, r);
    LOAD(glMultiTexCoord4svARB, r);

#undef LOAD
    assert(r == true);

    return r ? 0 : -1;
}

#else /* GL_H_HAS_ARB_multitexture */
// gl.h has GL_ARB_multitexture

extern "C"
int initglARBmultitexture()
{
    return 0;
}

#endif /* GL_H_HAS_ARB_multitexture */

/***********************************************************************
 *	END OF glARB.cpp
 ***********************************************************************/
