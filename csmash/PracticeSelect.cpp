/**
 * @file
 * @brief Implementation of PracticeSelect class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2001, 2002, 2004  神南 吉宏(Kanna Yoshihiro)
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
#include "PracticeSelect.h"
#include "BaseView.h"
#include "Player.h"
#include "Sound.h"
#include "PracticeSelectView.h"
//#include "PracticeSelectView2D.h"
#include "Event.h"

extern long mode;

extern bool isComm;

extern long wins;

/**
 * Default constructor. 
 * Initialize member variables to 0 or NULL. 
 */
PracticeSelect::PracticeSelect() {
  m_rotate = 0; m_opponentRotate = 0;
  m_View = NULL;
  m_selected = 0; m_opponentSelected = 0;
}

/**
 * Destructor. 
 */
PracticeSelect::~PracticeSelect() {
}

/**
 * Initializer method. 
 * Create PracticeSelectView object and attach it to this object. 
 * 
 * @return returns true if succeeds. 
 */
bool
PracticeSelect::Init() {
  m_View = (PracticeSelectView *)View::CreateView( VIEW_PRACTICESELECT );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  return true;
}

/**
 * PracticeSelect object creater. 
 * This method creates singleton PracticeSelect object. 
 */
void
PracticeSelect::Create() {
  Control::ClearControl();

  m_theControl = new PracticeSelect();
  m_theControl->Init();
}

/**
 * Move player panels as the game player moves mouse. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
PracticeSelect::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		    long *MouseYHistory, unsigned long *MouseBHistory,
		    int Histptr ) {
  static long lastRotate = 0;
  static long nothing = 0;

  long *rotate;
  long *selected;

  if ( (KeyHistory[Histptr].unicode == SDLK_ESCAPE ||
	KeyHistory[Histptr].unicode == 'Q') && !isComm ) {
    mode = MODE_TITLE;
    return true;
  }

  if ( m_opponentSelected > 500 ) {
    mode = MODE_PRACTICE;
    return true;
  }

  if ( m_selected > 100 ) {
    rotate = &m_opponentRotate;
    selected = &m_opponentSelected;
  } else {
    rotate = &m_rotate;
    selected = &m_selected;
  }

  if ( MouseBHistory[Histptr]&BUTTON_LEFT && 
       ( (Histptr > 0 && !(MouseBHistory[Histptr-1]&BUTTON_LEFT)) ||
	 (Histptr == 0 && !(MouseBHistory[MAX_HISTORY]&BUTTON_LEFT)) ) ) {
    nothing = 0;
    if ( *selected == 0 ) {
      *selected = 1;
      Sound::TheSound()->Play( SOUND_CLICK, vector3d(0.0) );
    } else if ( *selected > 100 ) {
      *selected = 500;
      return true;
    }
  }

  if ( *selected > 0 ) {
    (*selected)++;
    return true;
  }

  if ( lastRotate == 0 ) {
    if ( MouseXHistory[Histptr] - BaseView::GetWinWidth()/2 > 10 ) {
      nothing = 0;
      lastRotate = 2;
    } else if ( MouseXHistory[Histptr] - BaseView::GetWinWidth()/2 < -10 ) {
      nothing = 0;
      lastRotate = -2;
    } else
      nothing++;

    if ( lastRotate != 0 ) {
      *rotate += lastRotate;
      if ( *rotate < 0 )
	*rotate += 360;
      else
	*rotate %= 360;
    }
  } else {
    long nextRotate = *rotate + lastRotate;

    if ( nextRotate < 0 )
      nextRotate += 360;
    else
      nextRotate %= 360;

    if ( (*rotate)/(360/PLAYERS) != nextRotate/(360/PLAYERS) ) {
      *rotate = (nextRotate+360/PLAYERS/2)/(360/PLAYERS)*(360/PLAYERS);
      lastRotate = 0;
      Sound::TheSound()->Play( SOUND_CLICK, vector3d(0.0) );
    } else
      *rotate = nextRotate;
  }

  if ( nothing > 1000 ) {
    nothing = 0;
    mode = MODE_TITLE;
  }

  if ( nothing != 0 )
    return false;
  else
    return true;
}

/**
 * Get player ID of the opponent player type of which the panel is shown in front. 
 * 
 * @return returns player ID
 */
long
PracticeSelect::GetOpponentNum() {
  if ( GetOpponentRotate() < 0 )
    return (360+(GetOpponentRotate()%360))/(360/PLAYERS);
  else
    return (GetOpponentRotate()%360)/(360/PLAYERS);
}
