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
extern long wins;

extern bool isComm;

PlayerSelectView::PlayerSelectView() {
  m_offset = 0;
  m_textures[0] = 0;
}

PlayerSelectView::~PlayerSelectView() {
}

bool
PlayerSelectView::Init( PlayerSelect *playerSelect ) {
  ImageData image;
  int i, j;

  static char pname[][30] = {"images/PenSmash.ppm", "images/ShakeCut.ppm",
			     "images/PenDrive.ppm"};

  m_playerSelect = playerSelect;

  if ( m_textures[0] == 0 ) {
    glGenTextures( PLAYERS+2, m_textures );

    for ( i = 0 ; i < PLAYERS ; i++ ){
      image.LoadPPM( &(pname[i][0]) );
      glBindTexture(GL_TEXTURE_2D, m_textures[i] );
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

      glTexImage2D(GL_TEXTURE_2D, 0, 3, image.GetWidth(), image.GetHeight(), 
		   0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );
    }
  }

#ifndef HAVE_LIBZ
  FILE *fp;

  if( (fp = fopen("images/SelectPlayer.ppm", "r")) == NULL ){
    return false;
  }
#else
  gzFile fp;
  if (NULL == (fp = gzopenx("images/SelectPlayer.ppm", "rs"))) return false;
#endif

  for ( i = 69 ; i >= 0 ; i-- ) {
    for ( j = 0 ; j < 400/8 ; j++ ) {
      m_selectPlayer[i*50+j] = strtol( getWord(fp), NULL, 16 );
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
PlayerSelectView::Redraw() {
  int i;

  glEnable(GL_TEXTURE_2D);

  glColor4f( 0.0, 0.0, 0.0, 0.0 );

  if ( m_playerSelect->GetSelected() > 0 ) {
    int player;

    if ( m_playerSelect->GetRotate() < 0 )
      player = (360+(m_playerSelect->GetRotate()%360))/(360/PLAYERS);
    else
      player = (m_playerSelect->GetRotate()%360)/(360/PLAYERS);

    glPushMatrix();
    if ( m_playerSelect->GetSelected() < 100 ) {
      glTranslatef( -0.01*m_playerSelect->GetSelected(),
		    -1.5+0.01*m_playerSelect->GetSelected(), 1.4 );
      glRotatef( m_playerSelect->GetSelected()*360.0/100.0, 0.0, 0.0, 1.0 );
    } else
      glTranslatef( -0.01*100, -1.5+0.01*100, 1.4 );

    glBindTexture(GL_TEXTURE_2D, m_textures[player] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f(-0.60, 0.0, -0.84);
    glTexCoord2f(0.0, 0.0); glVertex3f(-0.60, 0.0,  0.84);
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.60, 0.0,  0.84);
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.60, 0.0, -0.84);
    glEnd();
    glPopMatrix();

    if ( !isComm ) {
      if ( m_playerSelect->GetSelected() > 100 ) {
	glPushMatrix();
	if ( m_playerSelect->GetSelected() < 150 )
	  glTranslatef( 0.02*(m_playerSelect->GetSelected()-100),
			18.5-0.4*(m_playerSelect->GetSelected()-100), 1.4 );
	else
	  glTranslatef( 0.01*100, -1.5+0.01*100, 1.4 );

	glBindTexture(GL_TEXTURE_2D, m_textures[(player+wins+1)%PLAYERS] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-0.60, 0.0, -0.84);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.60, 0.0,  0.84);
	glTexCoord2f(1.0, 0.0); glVertex3f( 0.60, 0.0,  0.84);
	glTexCoord2f(1.0, 1.0); glVertex3f( 0.60, 0.0, -0.84);
	glEnd();
	glPopMatrix();
      }
    }
  } else {
    for ( i = 0 ; i < PLAYERS ; i++ ){
      glPushMatrix();
        glRotatef( m_playerSelect->GetRotate()-i*360/PLAYERS, 0.0, 0.0, 1.0 );
	glBindTexture(GL_TEXTURE_2D, m_textures[i] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-0.60, -1.5, 1.4-0.84);
	glTexCoord2f(0.0, 0.0); glVertex3f(-0.60, -1.5, 1.4+0.84);
	glTexCoord2f(1.0, 0.0); glVertex3f( 0.60, -1.5, 1.4+0.84);
	glTexCoord2f(1.0, 1.0); glVertex3f( 0.60, -1.5, 1.4-0.84);
	glEnd();
      glPopMatrix();
    }

    glColor4f( 1.0, 1.0, 1.0, 0.0 );

    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0.0, (GLfloat)winWidth, 0.0, (GLfloat)winHeight );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRasterPos2i( 200, 100 );
    glBitmap( 400, 70, 0.0, 0.0, 0.0, 0, m_selectPlayer );

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  glDisable(GL_TEXTURE_2D);

  return true;
}

bool
PlayerSelectView::RedrawAlpha() {
  return true;
}
