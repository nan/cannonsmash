/* $Id$ */

// Copyright (C) 2000, 2004  神南 吉宏(Kanna Yoshihiro)
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
#include "TrainingPenAttack.h"
#include "Ball.h"

extern Ball   theBall;

TrainingPenAttack::TrainingPenAttack() : PenAttack() {
}

TrainingPenAttack::TrainingPenAttack(long side) : PenAttack(side) {
}

TrainingPenAttack::TrainingPenAttack( long playerType, long side, 
				      const vector3d x, const vector3d v, 
				      long status, long swing, 
				      long swingType, bool swingSide, 
				      long afterSwing, long swingError, 
				      const vector2d target, 
				      const vector3d eye, long pow, 
				      const vector2d spin, double stamina, 
				      long statusMax ) : 
  PenAttack( playerType, side, x, v, status, swing, swingType, swingSide, 
	     afterSwing, swingError, target, eye, pow, spin, stamina, 
	     statusMax ) {
}

TrainingPenAttack::~TrainingPenAttack() {
}

bool
TrainingPenAttack::AddStatus( long diff ) {
  // Add something in the future
  return true;
}

bool
TrainingPenAttack::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
			 long *MouseYHistory, unsigned long *MouseBHistory,
			 int Histptr ) {
  Player::Move( KeyHistory, MouseXHistory, MouseYHistory,MouseBHistory,
		Histptr );

  m_v[1] = m_v[2] = 0.0;
  m_x[1] = -(TABLELENGTH/2+0.3)*m_side;
  //m_targetX = -TABLEWIDTH/5*2*m_side;
  m_target[0] = -TABLEWIDTH/5*m_side;
  //m_targetY = TABLELENGTH/16*5*m_side;
  m_target[1] = TABLELENGTH/16*4*m_side;

  return true;
}

bool
TrainingPenAttack::HitBall() {
  vector3d v;

  // Serve
  if ( ( (m_side == 1 && theBall.GetStatus() == 6) ||
         (m_side ==-1 && theBall.GetStatus() == 7) ) &&
       fabs( m_x[0]-theBall.GetX()[0] ) < 0.6 &&
       fabs( m_x[1]-theBall.GetX()[1] ) < 0.3 ){
    SwingError();

    vector2d spin;
    spin[0] = 0.0;
    spin[1] = 0.5;
    theBall.TargetToVS( m_target, 0.8, spin, v );

    theBall.Hit( v, spin, this );
  } else {
    PenAttack::HitBall();
  }
  return true;
}
