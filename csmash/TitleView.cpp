/* $Id$ */

// Copyright (C) 2000  神南 吉宏(Kanna Yoshihiro)
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

extern long winWidth;
extern long winHeight;

TitleView::TitleView() {
}

TitleView::~TitleView() {
}

bool
TitleView::Init( Title *title ) {
  int i, j, k;
#ifndef HAVE_LIBZ
  FILE *fp;
#else
  gzFile fp;
#endif 
  static char menu[][30] = {"images/StartGame.ppm", "images/Howto.ppm",
			    "images/Quit.ppm"};

  ImageData image;

  m_title = title;

  for ( i = 0 ; i < m_title->GetMenuNum() ; i++ ) {
#ifndef HAVE_LIBZ
    if( (fp = fopen(&menu[i][0], "r")) == NULL ){
      return false;
    }
#else
    if (NULL == (fp = gzopenx(&menu[i][0], "rs"))) return false;
#endif

    for ( j = 69 ; j >= 0 ; j-- ) {
      for ( k = 0 ; k < 400/8 ; k++ ) {
	m_choice[i][j*50+k] = strtol( getWord(fp), NULL, 16 );
      }
    }

#ifndef HAVE_LIBZ
    fclose(fp);
#else
    gzclose(fp);
#endif
  }

  return true;
}

bool
TitleView::Redraw() {
  return true;
}

bool
TitleView::RedrawAlpha() {
  int i;

  glColor4f( 1.0, 1.0, 1.0, 0.0 );

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLfloat)winWidth, 0.0, (GLfloat)winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDepthMask(0);

  for ( i = 0 ; i < m_title->GetMenuNum() ; i++ ) {
    if ( m_title->GetSelected() == i )
      glColor4f( 1.0, 1.0, 0.0, 0.0 );
    else
      glColor4f( 1.0, 1.0, 1.0, 0.0 );

    glRasterPos2i( 200, 330-i*100 );
    glBitmap( 400, 70, 0.0, 0.0, 0.0, 0, &m_choice[i][0] );
  }

  glColor4f( 1.0, 1.0, 0.0, 0.0 );
  glBegin(GL_TRIANGLES);
  glVertex2i( 120, 330-m_title->GetSelected()*100 );
  glVertex2i( 180, 365-m_title->GetSelected()*100 );
  glVertex2i( 120, 400-m_title->GetSelected()*100 );
  glEnd();
  glDepthMask(1);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  return true;
}
