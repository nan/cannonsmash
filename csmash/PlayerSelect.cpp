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

extern BaseView theView;
extern long mode;

extern bool isComm;

extern Player *thePlayer;

extern long winWidth;
extern Sound theSound;

extern long wins;

PlayerSelect::PlayerSelect() {
  m_rotate = 0;
  m_View = NULL;
  m_selected = 0;
}

PlayerSelect::~PlayerSelect() {
  if ( m_View ){
    theView.RemoveView( m_View );
    delete m_View;
  }
}

bool
PlayerSelect::Init() {
  m_View = new PlayerSelectView();
  m_View->Init( this );

  theView.AddView( m_View );

  return true;
}

PlayerSelect*
PlayerSelect::Create() {
  PlayerSelect *newPlayerSelect;

  Event::ClearObject();

  newPlayerSelect = new PlayerSelect();
  newPlayerSelect->Init();

  glutSetCursor( GLUT_CURSOR_NONE );

  return newPlayerSelect;
}

bool
PlayerSelect::Move( unsigned long *KeyHistory, long *MouseXHistory,
		    long *MouseYHistory, unsigned long *MouseBHistory,
		    int Histptr ) {
  static long lastRotate = 0;
  static long nothing = 0;

  // 次の対戦相手
  if ( wins > 0 && m_selected == 0 ) {
    m_selected = 1;
    m_rotate = (thePlayer->GetPlayerType()-1)*(360/PLAYERS);
    return true;
  }

  if ( KeyHistory[Histptr] == 27 && !isComm ) {	// ESC
    mode = MODE_TITLE;
    return true;
  }

  if ( m_selected > 500 ) {
    mode = MODE_PLAY;
    return true;
  }

  if ( MouseBHistory[Histptr]&BUTTON_LEFT && 
       ( (Histptr > 0 && !(MouseBHistory[Histptr-1]&BUTTON_LEFT)) ||
	 (Histptr == 0 && !(MouseBHistory[MAX_HISTORY]&BUTTON_LEFT)) ) ){
    nothing = 0;
    if ( m_selected == 0 ) {
      m_selected = 1;
      theSound.Play( SOUND_CLICK );
    } else if ( m_selected > 100 ) {
      mode = MODE_PLAY;
      return true;
    }
  }

  if ( m_selected > 0 ) {
    m_selected++;
    return true;
  }

  if ( lastRotate == 0 ) {
    if ( MouseXHistory[Histptr] - winWidth/2 > 10 ) {
      nothing = 0;
      lastRotate = 2;
    } else if ( MouseXHistory[Histptr] - winWidth/2 < -10 ) {
      nothing = 0;
      lastRotate = -2;
    } else
      nothing++;

    m_rotate += lastRotate;
    if ( m_rotate < 0 )
      m_rotate += 360;
    else
      m_rotate %= 360;
  } else {
    long nextRotate = m_rotate + lastRotate;

    if ( nextRotate < 0 )
      nextRotate += 360;
    else
      nextRotate %= 360;

    if ( m_rotate/(360/PLAYERS) != nextRotate/(360/PLAYERS) ) {
      m_rotate = (nextRotate+360/PLAYERS/2)/(360/PLAYERS)*(360/PLAYERS);
      lastRotate = 0;
      theSound.Play( SOUND_CLICK );
    } else
      m_rotate = nextRotate;
  }

  if ( nothing > 1000 && !isComm ) {
    nothing = 0;
    //mode = MODE_DEMO;
    mode = MODE_TITLE;
  }

  return true;
}

long
PlayerSelect::GetRotate() {
  return m_rotate;
}

long
PlayerSelect::GetPlayerNum() {
  if ( GetRotate() < 0 )
    return (360+(GetRotate()%360))/(360/PLAYERS);
  else
    return (GetRotate()%360)/(360/PLAYERS);
}

long
PlayerSelect::GetSelected() {
  return m_selected;
}

bool
PlayerSelect::LookAt( double &srcX, double &srcY, double &srcZ,
		      double &destX, double &destY, double &destZ ) {
  srcX = 0.0;
  srcY = -TABLELENGTH-1.2;
  srcZ = 1.4;

  return true;
}
