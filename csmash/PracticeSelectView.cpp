/**
 * @file
 * @brief Implementation of PracticeSelectView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2001-2003, 2007  Kanna Yoshihiro
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
#include "PracticeSelectView.h"
#include "PracticeSelect.h"
#include "LoadImage.h"
#include "BaseView.h"
#include "RCFile.h"

extern RCFile *theRC;

/**
 * Default constructor. 
 */
PracticeSelectView::PracticeSelectView() : PlayerSelectView() {
}

/**
 * Destructor. 
 * Delete image handler. 
 */
PracticeSelectView::~PracticeSelectView() {
}

/**
 * Redraw background and player panels. 
 * 
 * @return returns true if succeeds. 
 */
bool
PracticeSelectView::Redraw() {
  int i;

  if ( theRC->gmode != GMODE_SIMPLE ) {
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
  }

  if ( m_playerSelect->GetSelected() > 0 ) {
    int player;

    player = m_playerSelect->GetPlayerNum();

    glPushMatrix();
    if ( m_playerSelect->GetSelected() < 100 ) {
      glTranslatef( -1.0, -1.0F+0.01F*m_playerSelect->GetSelected(), 1.0F );
      glRotatef( m_playerSelect->GetSelected()*360.0F/100, 0.0F, 0.0F, 1.0F );
    } else {
      glTranslatef( -0.01F*100, -1.0F+0.01F*100, 1.0F );
    }

    glBindTexture(GL_TEXTURE_2D, m_textures[player] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0F, 1.0F); glVertex3f(-0.40F, 0.0F, -0.56F);
    glTexCoord2f(0.0F, 0.0F); glVertex3f(-0.40F, 0.0F,  0.56F);
    glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.40F, 0.0F,  0.56F);
    glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.40F, 0.0F, -0.56F);
    glEnd();
    glPopMatrix();
  } else {
    for ( i = 0 ; i < PLAYERS ; i++ ){
      glPushMatrix();
        glTranslatef( -1.0, -0.0, 0.0 );
        glRotatef( m_playerSelect->GetRotate()-i*360.0F/PLAYERS,
		   0.0F, 0.0F, 1.0F );
	glBindTexture(GL_TEXTURE_2D, m_textures[i] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0F, 1.0F); glVertex3f(-0.40F, -1.0F, 1.0F-0.56F);
	glTexCoord2f(0.0F, 0.0F); glVertex3f(-0.40F, -1.0F, 1.0F+0.56F);
	glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.40F, -1.0F, 1.0F+0.56F);
	glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.40F, -1.0F, 1.0F-0.56F);
	glEnd();
      glPopMatrix();
    }
  }

  long selected = ((PracticeSelect *)m_playerSelect)->GetOpponentSelected();

  if ( selected > 0 ) {
    int player;

    player = ((PracticeSelect *)m_playerSelect)->GetOpponentNum();

    glPushMatrix();
    if ( selected < 100 ) {
      glTranslatef( 1.0F, -1.0F+0.01F*selected, 1.0F );
      glRotatef( selected*360.0F/100, 0.0F, 0.0F, 1.0F );
    } else {
      glTranslatef( 0.01F*100, -1.0F+0.01F*100, 1.0F );
    }

    glBindTexture(GL_TEXTURE_2D, m_textures[player] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0F, 1.0F); glVertex3f(-0.40F, 0.0F, -0.56F);
    glTexCoord2f(0.0F, 0.0F); glVertex3f(-0.40F, 0.0F,  0.56F);
    glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.40F, 0.0F,  0.56F);
    glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.40F, 0.0F, -0.56F);
    glEnd();
    glPopMatrix();
  } else {
    long rotate = ((PracticeSelect *)m_playerSelect)->GetOpponentRotate();

    if ( theRC->gmode != GMODE_SIMPLE || (rotate%360)%(360/PLAYERS) == 0 )
      glEnable(GL_TEXTURE_2D);

    for ( i = 0 ; i < PLAYERS ; i++ ){
      glPushMatrix();
        glTranslatef( 1.0, -0.0, 0.0 );
	glRotatef( rotate-i*360.0F/PLAYERS, 0.0F, 0.0F, 1.0F );
	glBindTexture(GL_TEXTURE_2D, m_textures[i] );
	glBegin(GL_QUADS);
        if ( m_playerSelect->GetSelected() < 100 ) {
	  glTexCoord2f(0.0F, 1.0F); glVertex3f(-0.40F, -0.5F, 1.0F-0.56F);
	  glTexCoord2f(0.0F, 0.0F); glVertex3f(-0.40F, -0.5F, 1.0F+0.56F);
	  glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.40F, -0.5F, 1.0F+0.56F);
	  glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.40F, -0.5F, 1.0F-0.56F);
	} else {
	  glTexCoord2f(0.0F, 1.0F); glVertex3f(-0.40F, -1.0F, 1.0F-0.56F);
	  glTexCoord2f(0.0F, 0.0F); glVertex3f(-0.40F, -1.0F, 1.0F+0.56F);
	  glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.40F, -1.0F, 1.0F+0.56F);
	  glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.40F, -1.0F, 1.0F-0.56F);
	}
	glEnd();
      glPopMatrix();
    }
  }

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0F, (GLfloat)BaseView::GetWinWidth(),
	      0.0F, (GLfloat)BaseView::GetWinHeight() );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glRasterPos2i( 200, 100 );
  glBitmap( 400, 70, 0.0F, 0.0F, 0.0F, 0, m_selectPlayer->GetImage() );

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glDisable(GL_TEXTURE_2D);

  return true;
}

/**
 * Redraw transparent objects. 
 * Do nothing. 
 * 
 * @return returns true if succeeds. 
 */
bool
PracticeSelectView::RedrawAlpha() {
  return true;
}
