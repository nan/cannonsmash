/* $Id$ */

// Copyright (C) 2001  神南 吉宏(Kanna Yoshihiro)
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
#include "PlayGameView.h"
#include "PlayGame.h"
#include "LoadImage.h"
#include "BaseView.h"

PlayGameView::PlayGameView() {
}

PlayGameView::~PlayGameView() {
}

bool
PlayGameView::Init( PlayGame *playGame ) {
  int j, k;

  static char imageFile[30] = "images/Pause.ppm";

#ifndef HAVE_LIBZ
  FILE *fp;
#else
  gzFile fp;
#endif

  ImageData image;

  m_playGame = playGame;

#ifndef HAVE_LIBZ
  if( (fp = fopen(imageFile, "r")) == NULL ){
    return false;
  }
#else
  if (NULL == (fp = gzopenx(imageFile, "rs"))) return false;
#endif

  for ( j = 149 ; j >= 0 ; j-- ) {
    for ( k = 0 ; k < 256/8 ; k++ ) {
      m_image[j*32+k] = (unsigned char)strtol( getWord(fp), NULL, 16 );
    }
  }

#ifndef HAVE_LIBZ
  fclose(fp);
#else
  gzclose(fp);
#endif

  return true;
}

bool
PlayGameView::Redraw() {
  return true;
}

bool
PlayGameView::RedrawAlpha() {
  View *view;

  glColor4f( 1.0, 1.0, 1.0, 0.0 );

  if ( SDL_WM_GrabInput( SDL_GRAB_QUERY ) == SDL_GRAB_OFF ) {
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0.0, (GLfloat)BaseView::GetWinWidth(),
		0.0, (GLfloat)BaseView::GetWinHeight() );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthMask(0);

    glRasterPos2i( (GLfloat)BaseView::GetWinWidth()/2-128,
		   (GLfloat)BaseView::GetWinHeight()/2 );
    glBitmap( 256, 150, 0.0F, 0.0F, 0.0F, 0, m_image );

    glDepthMask(1);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  return true;
}
