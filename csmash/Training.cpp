/* $Id$ */

// Copyright (C) 2000  $B?@Fn(B $B5H9((B(Kanna Yoshihiro)
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

extern Player* thePlayer;
extern Player* comPlayer;
extern Ball theBall;

extern long trainingCount;

Training::Training() {
  m_View = NULL;
  m_trainingCount = 0;
}

Training::~Training() {
  if ( m_View ){
    theView.RemoveView( m_View );
    delete m_View;
  }
}

bool
Training::Init() {
  m_View = new TrainingView();
  m_View->Init( this );

  theView.AddView( m_View );

  return true;
}

Training*
Training::Create( long player, long com ) {
  Training *newTraining;

  Event::ClearObject();

  newTraining = new Training();
  newTraining->Init();

  thePlayer = Player::Create( player, 1, 2 );
  comPlayer = Player::Create( com, -1, 3 );

  thePlayer->Init();
  comPlayer->Init();

  // View, $B$+(B?
  glutSetCursor( GLUT_CURSOR_NONE );

  return newTraining;
}

bool
Training::Move( unsigned long *KeyHistory, long *MouseXHistory,
		long *MouseYHistory, unsigned long *MouseBHistory,
		int Histptr ) {
  bool reDraw = false;
  long ballStatus = theBall.GetStatus();

  theBall.Move();
  reDraw |= thePlayer->Move( KeyHistory, MouseXHistory,
			     MouseYHistory, MouseBHistory, Histptr );
  reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

  if ( ballStatus != theBall.GetStatus() ) {
    if ( theBall.GetStatus() == 8 ) {
      if ( theBall.m_count > m_trainingCount )
	m_trainingCount = theBall.m_count;
      theBall.m_count = 0;
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