/* $Id$ */

// Copyright (C) 2000, 2001  神南 吉宏(Kanna Yoshihiro)
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
#include "MenuItemView.h"
#include "MenuItem.h"
#include "LoadImage.h"
#include "BaseView.h"

MenuItemView::MenuItemView() {
  m_image = NULL;
}

MenuItemView::~MenuItemView() {
  if ( m_image )
    free(m_image);
}

bool
MenuItemView::Init( MenuItem *menu, char *fileName ) {
  char fname[256];

  sprintf( fname, "%s.ppm", fileName );

#ifndef HAVE_LIBZ
  FILE *fp;
#else
  gzFile fp;
#endif

  ImageData image;

  int j, k;

  m_menuItem = menu;

#ifndef HAVE_LIBZ
  if( (fp = fopen(fname, "r")) == NULL ) {
    return false;
  }
#else
  if (NULL == (fp = gzopenx(fname, "rs"))) return false;
#endif

  m_image = 
    (GLubyte *)malloc( m_menuItem->GetHeight()*m_menuItem->GetWidth()/8 );

  for ( j = m_menuItem->GetHeight()-1 ; j >= 0 ; j-- ) {
    for ( k = 0 ; k < m_menuItem->GetWidth()/8 ; k++ ) {
      m_image[j*m_menuItem->GetWidth()/8+k] = (unsigned char)strtol( getWord(fp), NULL, 16 );
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
MenuItemView::Redraw() {
  return true;
}

bool
MenuItemView::RedrawAlpha() {
  glColor4f( 1.0F, 1.0F, 1.0F, 0.0F );

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0F, (GLfloat)BaseView::GetWinWidth(),
	      0.0F, (GLfloat)BaseView::GetWinHeight() );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDepthMask(0);

  if ( m_menuItem->GetSelected() )
    glColor4f( 1.0F, 1.0F, 0.0F, 0.0F );
  else
    glColor4f( 1.0F, 1.0F, 1.0F, 0.0F );

  glRasterPos2i( m_menuItem->GetX(), m_menuItem->GetY() );
  glBitmap( m_menuItem->GetWidth(), m_menuItem->GetHeight(),
	    0.0F, 0.0F, 0.0F, 0, m_image );

  glDepthMask(1);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  return true;
}
