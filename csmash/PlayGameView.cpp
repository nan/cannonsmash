/**
 * @file
 * @brief Implementation of PlayGameView class. 
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
#include "PlayGameView.h"
#include "PlayGame.h"
#include "LoadImage.h"
#include "BaseView.h"

/**
 * Default constructor. 
 */
PlayGameView::PlayGameView() {
  m_image = 0;
}

/**
 * Destructor. 
 * Delete image handler. 
 */
PlayGameView::~PlayGameView() {
  if ( m_image )
    delete m_image;
}

/**
 * Initializer method. 
 * Load "Pause" image texture. 
 * 
 * @param playGame attached PlayGame object. 
 * @return returns true if succeeds. 
 */
bool
PlayGameView::Init( PlayGame *playGame ) {
  static char imageFile[30] = "images/Pause";

  m_playGame = playGame;

  char filename[256];
  sprintf( filename, _("%s.pbm"), imageFile );

  m_image = new ImageData();
  m_image->LoadFile( filename );

  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_GrabInput( SDL_GRAB_ON );

  return true;
}

/**
 * Redraw valid objects. 
 * Do nothing. 
 * 
 * @return returns true if succeeds. 
 */
bool
PlayGameView::Redraw() {
  return true;
}

/**
 * Redraw transparent objects. 
 * If the game is paused, show "Pause" texture. 
 * 
 * @return returns true if succeeds. 
 */
bool
PlayGameView::RedrawAlpha() {
  if ( m_playGame->IsPause() ) {
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D( 0.0, (GLfloat)BaseView::GetWinWidth(),
		0.0, (GLfloat)BaseView::GetWinHeight() );
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glDepthMask(0);

    static GLfloat white_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
    static GLfloat white_dif[] = { 0.0F, 0.0F, 0.0F, 1.0F };
    static GLfloat white_amb[] = { 5.0F, 5.0F, 5.0F, 1.0F };
    static GLfloat white_shininess[] = { 5.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, white_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, white_shininess);

    glRasterPos2i( BaseView::GetWinWidth()/2-128, BaseView::GetWinHeight()/2 );
    glBitmap( 256, 150, 0.0F, 0.0F, 0.0F, 0, m_image->GetImage() );

    glDepthMask(1);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

  return true;
}
