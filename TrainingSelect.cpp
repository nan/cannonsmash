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
#include "TrainingSelect.h"
#include "BaseView.h"
#include "Player.h"
#include "Sound.h"
#include "TrainingSelectView.h"
#include "Event.h"

extern BaseView theView;
extern long mode;

extern Player *thePlayer;

extern Sound theSound;

extern long wins;

TrainingSelect::TrainingSelect() : PlayerSelect() {
}

bool
TrainingSelect::Init() {
  m_View = new TrainingSelectView();
  m_View->Init( this );

  theView.AddView( m_View );

  return true;
}

TrainingSelect*
TrainingSelect::Create() {
  TrainingSelect *newTrainingSelect;

  Event::ClearObject();

  newTrainingSelect = new TrainingSelect();
  newTrainingSelect->Init();

  glutSetCursor( GLUT_CURSOR_NONE );

  return newTrainingSelect;
}

bool
TrainingSelect::Move( unsigned long *KeyHistory, long *MouseXHistory,
		    long *MouseYHistory, unsigned long *MouseBHistory,
		    int Histptr ) {
  static long lastRotate = 0;
  static long nothing = 0;

  // 次の対戦相手
  if ( wins > 0 && m_selected == 0 ) {
    m_selected = 1;
    m_rotate = (thePlayer->GetPlayerType()-1)*(360/TRAININGPLAYERS);
    return true;
  }

  if ( KeyHistory[Histptr] == 27 ) {	// ESC
    mode = MODE_TITLE;
    return true;
  }

  if ( m_selected > 500 ) {
    mode = MODE_TRAINING;
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
      mode = MODE_TRAINING;
      return true;
    }
  }

  if ( m_selected > 0 ) {
    m_selected++;
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

    if ( m_rotate/(360/TRAININGPLAYERS) != nextRotate/(360/TRAININGPLAYERS) ) {
      m_rotate = (nextRotate+360/TRAININGPLAYERS/2)/(360/TRAININGPLAYERS)*
	(360/TRAININGPLAYERS);
      lastRotate = 0;
      theSound.Play( SOUND_CLICK );
    } else
      m_rotate = nextRotate;
  }

  if ( nothing > 1000 ) {
    nothing = 0;
    mode = MODE_TITLE;
  }

  return true;
}
