/***********************************************************************
 *	glARB.h
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
#ifndef __ESESOFT_YOTSUYA_98889799__glARB_h__INCLUDED__
#define __ESESOFT_YOTSUYA_98889799__glARB_h__INCLUDED__
/***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

/* __BEGIN__BEGIN__ */

/* OpenGL ARB extensions */

#ifndef GL_ARB_multitexture
#define GL_ARB_multitexture                 1

/* OpenGL ARB multitexture extension */

#define GL_ACTIVE_TEXTURE_ARB               0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB        0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB            0x84E2
#define GL_TEXTURE0_ARB                     0x84C0
#define GL_TEXTURE1_ARB                     0x84C1
#define GL_TEXTURE2_ARB                     0x84C2
#define GL_TEXTURE3_ARB                     0x84C3
#define GL_TEXTURE4_ARB                     0x84C4
#define GL_TEXTURE5_ARB                     0x84C5
#define GL_TEXTURE6_ARB                     0x84C6
#define GL_TEXTURE7_ARB                     0x84C7
#define GL_TEXTURE8_ARB                     0x84C8
#define GL_TEXTURE9_ARB                     0x84C9
#define GL_TEXTURE10_ARB                    0x84CA
#define GL_TEXTURE11_ARB                    0x84CB
#define GL_TEXTURE12_ARB                    0x84CC
#define GL_TEXTURE13_ARB                    0x84CD
#define GL_TEXTURE14_ARB                    0x84CE
#define GL_TEXTURE15_ARB                    0x84CF
#define GL_TEXTURE16_ARB                    0x84D0
#define GL_TEXTURE17_ARB                    0x84D1
#define GL_TEXTURE18_ARB                    0x84D2
#define GL_TEXTURE19_ARB                    0x84D3
#define GL_TEXTURE20_ARB                    0x84D4
#define GL_TEXTURE21_ARB                    0x84D5
#define GL_TEXTURE22_ARB                    0x84D6
#define GL_TEXTURE23_ARB                    0x84D7
#define GL_TEXTURE24_ARB                    0x84D8
#define GL_TEXTURE25_ARB                    0x84D9
#define GL_TEXTURE26_ARB                    0x84DA
#define GL_TEXTURE27_ARB                    0x84DB
#define GL_TEXTURE28_ARB                    0x84DC
#define GL_TEXTURE29_ARB                    0x84DD
#define GL_TEXTURE30_ARB                    0x84DE
#define GL_TEXTURE31_ARB                    0x84DF

typedef GLvoid (APIENTRY * PFNGLACTIVETEXTUREARBPROC) (GLenum texture);
typedef GLvoid (APIENTRY * PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1DARBPROC) (GLenum texture, GLdouble s);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1DVARBPROC) (GLenum texture, const GLdouble *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1FARBPROC) (GLenum texture, GLfloat s);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1FVARBPROC) (GLenum texture, const GLfloat *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1IARBPROC) (GLenum texture, GLint s);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1IVARBPROC) (GLenum texture, const GLint *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1SARBPROC) (GLenum texture, GLshort s);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD1SVARBPROC) (GLenum texture, const GLshort *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2DARBPROC) (GLenum texture, GLdouble s, GLdouble t);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2DVARBPROC) (GLenum texture, const GLdouble *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2FARBPROC) (GLenum texture, GLfloat s, GLfloat t);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2FVARBPROC) (GLenum texture, const GLfloat *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2IARBPROC) (GLenum texture, GLint s, GLint t);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2IVARBPROC) (GLenum texture, const GLint *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2SARBPROC) (GLenum texture, GLshort s, GLshort t);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD2SVARBPROC) (GLenum texture, const GLshort *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3DARBPROC) (GLenum texture, GLdouble s, GLdouble t, GLdouble r);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3DVARBPROC) (GLenum texture, const GLdouble *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3FARBPROC) (GLenum texture, GLfloat s, GLfloat t, GLfloat r);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3FVARBPROC) (GLenum texture, const GLfloat *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3IARBPROC) (GLenum texture, GLint s, GLint t, GLint r);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3IVARBPROC) (GLenum texture, const GLint *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3SARBPROC) (GLenum texture, GLshort s, GLshort t, GLshort r);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD3SVARBPROC) (GLenum texture, const GLshort *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4DARBPROC) (GLenum texture, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4DVARBPROC) (GLenum texture, const GLdouble *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4FARBPROC) (GLenum texture, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4FVARBPROC) (GLenum texture, const GLfloat *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4IARBPROC) (GLenum texture, GLint s, GLint t, GLint r, GLint q);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4IVARBPROC) (GLenum texture, const GLint *v);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4SARBPROC) (GLenum texture, GLshort s, GLshort t, GLshort r, GLshort q);
typedef GLvoid (APIENTRY * PFNGLMULTITEXCOORD4SVARBPROC) (GLenum texture, const GLshort *v);

/* Implementation */

extern PFNGLACTIVETEXTUREARBPROC glActiveTextureARB;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB;
extern PFNGLMULTITEXCOORD1DARBPROC glMultiTexCoord1dARB;
extern PFNGLMULTITEXCOORD1DVARBPROC glMultiTexCoord1dvARB;
extern PFNGLMULTITEXCOORD1FARBPROC glMultiTexCoord1fARB;
extern PFNGLMULTITEXCOORD1FVARBPROC glMultiTexCoord1fvARB;
extern PFNGLMULTITEXCOORD1IARBPROC glMultiTexCoord1iARB;
extern PFNGLMULTITEXCOORD1IVARBPROC glMultiTexCoord1ivARB;
extern PFNGLMULTITEXCOORD1SARBPROC glMultiTexCoord1sARB;
extern PFNGLMULTITEXCOORD1SVARBPROC glMultiTexCoord1svARB;
extern PFNGLMULTITEXCOORD2DARBPROC glMultiTexCoord2dARB;
extern PFNGLMULTITEXCOORD2DVARBPROC glMultiTexCoord2dvARB;
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB;
extern PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB;
extern PFNGLMULTITEXCOORD2IARBPROC glMultiTexCoord2iARB;
extern PFNGLMULTITEXCOORD2IVARBPROC glMultiTexCoord2ivARB;
extern PFNGLMULTITEXCOORD2SARBPROC glMultiTexCoord2sARB;
extern PFNGLMULTITEXCOORD2SVARBPROC glMultiTexCoord2svARB;
extern PFNGLMULTITEXCOORD3DARBPROC glMultiTexCoord3dARB;
extern PFNGLMULTITEXCOORD3DVARBPROC glMultiTexCoord3dvARB;
extern PFNGLMULTITEXCOORD3FARBPROC glMultiTexCoord3fARB;
extern PFNGLMULTITEXCOORD3FVARBPROC glMultiTexCoord3fvARB;
extern PFNGLMULTITEXCOORD3IARBPROC glMultiTexCoord3iARB;
extern PFNGLMULTITEXCOORD3IVARBPROC glMultiTexCoord3ivARB;
extern PFNGLMULTITEXCOORD3SARBPROC glMultiTexCoord3sARB;
extern PFNGLMULTITEXCOORD3SVARBPROC glMultiTexCoord3svARB;
extern PFNGLMULTITEXCOORD4DARBPROC glMultiTexCoord4dARB;
extern PFNGLMULTITEXCOORD4DVARBPROC glMultiTexCoord4dvARB;
extern PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;
extern PFNGLMULTITEXCOORD4FVARBPROC glMultiTexCoord4fvARB;
extern PFNGLMULTITEXCOORD4IARBPROC glMultiTexCoord4iARB;
extern PFNGLMULTITEXCOORD4IVARBPROC glMultiTexCoord4ivARB;
extern PFNGLMULTITEXCOORD4SARBPROC glMultiTexCoord4sARB;
extern PFNGLMULTITEXCOORD4SVARBPROC glMultiTexCoord4svARB;

#endif /* GL_ARB_multitexture */

/*
 * OpenGL extension functions may change if pixelformat changes.
 * If you change pixelformat, you must call initglARB??? again.
 */

int initglARBmultitexture();

#ifndef GL_ARB_texture_env_combine
#define GL_ARB_texture_env_combine  1

#define GL_COMBINE_ARB        0x8570
#define GL_COMBINE_RGB_ARB    0x8571
#define GL_COMBINE_ALPHA_ARB  0x8572
#define GL_SOURCE0_RGB_ARB    0x8580
#define GL_SOURCE1_RGB_ARB    0x8581
#define GL_SOURCE2_RGB_ARB    0x8582
#define GL_SOURCE0_ALPHA_ARB  0x8588
#define GL_SOURCE1_ALPHA_ARB  0x8589
#define GL_SOURCE2_ALPHA_ARB  0x858A
#define GL_OPERAND0_RGB_ARB   0x8590
#define GL_OPERAND1_RGB_ARB   0x8591
#define GL_OPERAND2_RGB_ARB   0x8592
#define GL_OPERAND0_ALPHA_ARB 0x8598
#define GL_OPERAND1_ALPHA_ARB 0x8599
#define GL_OPERAND2_ALPHA_ARB 0x859A
#define GL_RGB_SCALE_ARB      0x8573
#define GL_ADD_SIGNED_ARB     0x8574
#define GL_INTERPOLATE_ARB    0x8575
#define GL_CONSTANT_ARB       0x8576
#define GL_PRIMARY_COLOR_ARB  0x8577
#define GL_PREVIOUS_ARB       0x8578
#define GL_SUBTRACT_ARB       0x84E7

#endif /* GL_ARB_texture_env_combine */

/* __END__END__ */
#ifdef __cplusplus
} /* extern "C" */
#endif
/***********************************************************************/
#endif
/***********************************************************************
 *	END OF glARB.h
 ***********************************************************************/
