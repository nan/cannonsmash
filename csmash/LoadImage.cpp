/**
 * @file
 * @brief Implementation of ImageData class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2002  神南 吉宏(Kanna Yoshihiro)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

#include "ttinc.h"
#include "LoadImage.h"

#if 0
extern "C" {
#define JDLLDECL
#include <jpeglib.h>
}
#endif

#ifdef HAVE_LIBZ
#include "z.h"
#endif

/**
 * Default constructor. 
 * Initialize member variable to 0. 
 */
ImageData::ImageData() {
  m_image = NULL;
  m_width = m_height = m_bytes = 0;
}

/**
 * Constructor. 
 * Specify image width, height and depth. 
 * Size of the image becomes width*height*depth bytes. 
 * 
 * @param width width of the image. 
 * @param height height of the image. 
 * @param bytes depth of the image. 
 */
ImageData::ImageData( long width, long height, long bytes) {
  m_image = new GLubyte[width*height*bytes];

  if ( m_image ) {
    m_width = width;
    m_height = height;
    m_bytes = bytes;
  }
}

/**
 * Destructor. 
 * Delete image data. 
 */
ImageData::~ImageData() {
  if ( m_image )
    delete m_image;
}

/**
 * Get a pixel color. 
 * 
 * @param x x coordinates of the pixel
 * @param y y coordinates of the pixel
 * @param bytes bytes of the pixel. 
 * 
 * @return returns the pixel color. If x, y, bytes exceeds the width, height, bytes of the image, returns 0. 
 */
GLubyte
ImageData::GetPixel( long x, long y, long bytes ) {
  if ( x > m_width || x < 0 || y > m_height || y < 0 ||
       bytes > m_bytes || bytes < 0 )
    return 0;

  return m_image[y*m_width*m_bytes + x*m_bytes + bytes];
}

/**
 * Change pixel color. 
 * 
 * @param x x coordinates of the pixel
 * @param y y coordinates of the pixel
 * @param bytes bytes of the pixel. 
 * @param val color of the pixel. 
 * 
 * @return returns true if succeeds. If x, y, bytes exceeds the width, height, bytes of the image, returns false. 
 */
bool
ImageData::SetPixel( long x, long y, long bytes , GLubyte val ) {
  if ( x > m_width || x < 0 || y > m_height || y < 0 ||
       bytes > m_bytes || bytes < 0 )
    return false;

  m_image[y*m_width*m_bytes + x*m_bytes + bytes] = val;
  return true;
}

/**
 * Load PPM image. 
 * This method loads image from the specified PPM file or gzipped PPM file. 
 * This method supports P2, P4, P5, P6 PPM file. 
 * 
 * @param filename PPM file name
 * @return returns true if succeeds. 
 */
bool
ImageData::LoadPPM(const char* filename ) {
  int i, j;
#ifndef HAVE_LIBZ
  FILE *fp;
#else
  gzFile fp;
#endif
  char *w;
  int pb;

#ifndef HAVE_LIBZ
  if( (fp = fopen(filename, "r")) == NULL ) {
    return false;
  }
#else
  if (NULL == (fp = gzopenx(filename, "rs"))) return false;
#endif

  w = getWord(fp);
  if( !strcmp("P3", w) )
    pb = 3;
  else if( !strcmp("P2", w) )
    pb = 2;
  else if( !strcmp("P4", w) )
    pb = 4;
  else if ( !strcmp("P5", w) )
    pb = 5;
  else if ( !strcmp("P6", w) )
    pb = 6;
  else
    return false;

  m_width = atoi( getWord(fp) );
  m_height = atoi( getWord(fp) );
  m_bytes = 4;

  if ( pb != 4 )
    getWord( fp );

  if ( m_image )
    delete m_image;

  m_image = new GLubyte[m_width*m_height*m_bytes];
  if ( !m_image ) {
    m_width = m_height = m_bytes = 0;
    return false;
  }

  if ( pb == 5 ) {
    GLubyte b;
    for ( i = 0 ; i < m_height ; i++ ){
      for ( j = 0 ; j < m_width ; j++ ){
#ifndef HAVE_LIBZ
	fread( &b , 1, 1, fp );
#else
	gzread(fp, &b, 1 * 1);
#endif
	SetPixel( j, i, 0, b );
	SetPixel( j, i, 1, b );
	SetPixel( j, i, 2, b );
	SetPixel( j, i, 3, 255 );
      }
    }
  } else if ( pb == 6 ) {
    GLubyte b;
    for ( i = 0 ; i < m_height ; i++ ){
      for ( j = 0 ; j < m_width ; j++ ){
#ifndef HAVE_LIBZ
	fread( &b , 1, 1, fp ); SetPixel( j, i, 0, b );
	fread( &b , 1, 1, fp ); SetPixel( j, i, 1, b );
	fread( &b , 1, 1, fp ); SetPixel( j, i, 2, b );
#else
	gzread(fp, &b , 1 * 1); SetPixel( j, i, 0, b );
	gzread(fp, &b , 1 * 1); SetPixel( j, i, 1, b );
	gzread(fp, &b , 1 * 1); SetPixel( j, i, 2, b );
#endif
	SetPixel( j, i, 3, 255 );
      }
    }
  } else if ( pb == 4 ) {
    int rowbytes = m_width/8;
    if ( m_width%8 > 0 )
      rowbytes++;

#ifndef HAVE_LIBZ
    fread( m_image, 1, rowbytes*m_height, fp );
#else
    gzread( fp, m_image, rowbytes*m_height );
#endif
  } else {
    for ( i = 0 ; i < m_height ; i++ ){
      for ( j = 0 ; j < m_width ; j++ ){
	if ( pb == 3 ) {
	  SetPixel( j, i, 0, (GLubyte)atoi(getWord( fp ) ) );
	  SetPixel( j, i, 1, (GLubyte)atoi(getWord( fp ) ) );
	  SetPixel( j, i, 2, (GLubyte)atoi(getWord( fp ) ) );
	  SetPixel( j, i, 3, 255 );
	} else if ( pb == 2 ) {
	  SetPixel( j, i, 0, (GLubyte)atoi(getWord( fp ) ) );
	  SetPixel( j, i, 1, (GLubyte)atoi(getWord( fp ) ) );
	  SetPixel( j, i, 2, (GLubyte)atoi(getWord( fp ) ) );
	  SetPixel( j, i, 3, (GLubyte)atoi(getWord( fp ) ) );
	}
      }
    }
  }

#ifndef HAVE_LIBZ
  fclose( fp );
#else
  gzclose(fp);
#endif

  return true;
}

#ifdef HAVE_LIBZ
/**
 * Get a word from file. 
 * This method reads a word from the file. 
 * 
 * @param fp file pointer of the input file. Input file can be gzipped. 
 * @return returns a word. 
 */
char*
getWord( gzFile fp ) {
  static char buf[256];
  static char* ptr = buf;
  char* ptr2;

  while(1) {
    if( *ptr == 0 || *ptr == '#' ) {
      gzgets(fp, buf, 256);
      ptr = buf;
      continue;
    } else if( isspace(*ptr) ) {
      ptr++;
      continue;
    } else
      break;
  }

  ptr2 = ptr;
  while( !isspace(*ptr) && *ptr != 0 ) {
    ptr++;
  }
  *ptr = 0;
  ptr++;

  return ptr2;
}
#else
/**
 * Get a word from file. 
 * This method reads a word from the file. 
 * 
 * @param fp file pointer of the input file. 
 * @return returns a word. 
 */
char*
getWord( FILE *fp ) {
  static char buf[256];
  static char* ptr = buf;
  char* ptr2;

  while(1) {
    if( *ptr == 0 || *ptr == '#' ) {
      fgets( buf, 256, fp );
      ptr = buf;
      continue;
    } else if( isspace(*ptr) ) {
      ptr++;
      continue;
    } else
      break;
  }

  ptr2 = ptr;
  while( !isspace(*ptr) && *ptr != 0 ) {
    ptr++;
  }
  *ptr = 0;
  ptr++;

  return ptr2;
}
#endif

/**
 * Load JPEG image. 
 * This method loads image from the specified JPEG file. 
 * 
 * @param filename JPEG file name
 * @return returns true if succeeds. 
 */
bool ImageData::LoadJPG(const char *filename)
{
  SDL_Surface *img = SDL_GL_LoadTexture((char *)filename);
  m_width = img->w;
  m_height = img->h;
  m_bytes = 4;

  m_image = new GLubyte[m_width*m_height*m_bytes];
  memcpy( m_image, img->pixels, m_width*m_height*m_bytes );

  SDL_FreeSurface(img);

  return true;
}

/**
 * Check file extention. 
 * 
 * @param filename filename to be tested. 
 * @param ext extension name
 * @return returns true if extension of the filename matches ext. 
 */
inline bool extmatch(const char *filename, const char *ext)
{
    int lf = strlen(filename);
    int le = strlen(ext);
    return 0 == strcmp(&filename[lf-le], ext);
}

/**
 * Load image file. 
 * This method checks the extension of the file name to find the image type, 
 * and load the image. 
 * 
 * @param filename image file name
 * @return returns true if succeeds. 
 */
bool ImageData::LoadFile(const char *filename)
{
    if      (extmatch(filename, ".jpg")) {
	return LoadJPG(filename);
    }
    else if (extmatch(filename, ".ppm") || extmatch(filename, ".ppm.gz") ||
	     extmatch(filename, ".pbm") || extmatch(filename, ".pbm.gz") ) {
	return LoadPPM(filename);
    }
    else {
	return false;
    }
}

// Copyed from testgl.c of SDL source code
/**
 * Load image file and set it to texture. 
 * This method is copyed from testgl.c of SDL source code. 
 * 
 * @param filename image file name
 * @return returns SDL_Surface object of texture. return NULL if failed. 
 */
SDL_Surface* SDL_GL_LoadTexture(char *filename)
{
  SDL_Surface *image, *jpg;
  SDL_Rect area;
  Uint32 saved_flags;
  Uint8  saved_alpha;

  jpg = IMG_Load( filename );
  /* Use the surface width and height expanded to powers of 2 */

  image = SDL_CreateRGBSurface(
			       SDL_SWSURFACE,
			       jpg->w, jpg->h,
			       32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			       0x000000FF, 
			       0x0000FF00, 
			       0x00FF0000, 
			       0xFF000000
#else
			       0xFF000000,
			       0x00FF0000, 
			       0x0000FF00, 
			       0x000000FF
#endif
			       );
  if ( image == NULL ) {
    return 0;
  }

  /* Save the alpha blending attributes */
  saved_flags = jpg->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
  saved_alpha = jpg->format->alpha;
  if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
    SDL_SetAlpha(jpg, 0, 0);
  }

  /* Copy the surface into the GL texture image */
  area.x = 0;
  area.y = 0;
  area.w = jpg->w;
  area.h = jpg->h;
  SDL_BlitSurface(jpg, &area, image, &area);

  /* Restore the alpha blending attributes */
  if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
    SDL_SetAlpha(jpg, saved_flags, saved_alpha);
  }

  return image;
}
