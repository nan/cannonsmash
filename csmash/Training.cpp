/* $Id$ */

// Copyright (C) 2000, 2001, 2002  神南 吉宏(Kanna Yoshihiro)
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
#include "Training.h"
#include "TrainingView.h"
#include "BaseView.h"
#include "Player.h"
#include "Ball.h"
#include "Event.h"

extern Player* thePlayer;
extern Player* comPlayer;
extern Ball theBall;
extern long mode;

Training::Training() {
  m_View = NULL;
  m_trainingCount = 0;
  m_trainingMax = 0;
}

Training::~Training() {
  if ( m_View ){
    BaseView::TheView()->RemoveView( m_View );
    delete m_View;
  }
}

bool
Training::Init() {
  m_View = (TrainingView *)View::CreateView( VIEW_TRAINING );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  return true;
}

void
Training::Create( long player, long com ) {
  Event::ClearObject();

  m_theControl = new Training();
  m_theControl->Init();

  thePlayer = Player::Create( player, 1, 2 );
  comPlayer = Player::Create( com, -1, 3 );

  thePlayer->Init();
  comPlayer->Init();

  // Move it to view?
  SDL_ShowCursor(0);
  SDL_WM_GrabInput( SDL_GRAB_ON );
}

bool
Training::Move( unsigned long *KeyHistory, long *MouseXHistory,
		long *MouseYHistory, unsigned long *MouseBHistory,
		int Histptr ) {
  bool reDraw = false;
  long ballStatus = theBall.GetStatus();

  if ( KeyHistory[Histptr] == 'Q' ) {
    mode = MODE_TITLE;
    return true;
  }

  theBall.Move();
  reDraw |= thePlayer->Move( KeyHistory, MouseXHistory,
			     MouseYHistory, MouseBHistory, Histptr );
  reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

  if ( ballStatus != theBall.GetStatus() ) {
    if ( theBall.GetStatus() == 8 ) {
      if ( m_trainingCount > m_trainingMax )
	m_trainingMax = m_trainingCount;
      m_trainingCount = 0;
    }
  }

  return reDraw;
}

bool
Training::LookAt( double &srcX, double &srcY, double &srcZ,
		  double &destX, double &destY, double &destZ ) {
  if (thePlayer) {
    srcX = thePlayer->GetX() + thePlayer->GetEyeX();
    srcY = thePlayer->GetY() + thePlayer->GetEyeY();
    srcZ = thePlayer->GetZ() + thePlayer->GetEyeZ();
    destX = thePlayer->GetLookAtX();
    destY = thePlayer->GetLookAtY();
    destZ = thePlayer->GetLookAtZ();
  }

  return true;
}

void
Training::AddTrainingCount() {
  m_trainingCount++;
}
