/* $Id$ */

// Copyright (C) 2001, 2004, 2007  神南 吉宏(Kanna Yoshihiro)
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
#include "NetPenAttack.h"
#include "Ball.h"
#include "Event.h"

extern Ball   theBall;

NetPenAttack::NetPenAttack() : PenAttack() {
}

NetPenAttack::NetPenAttack(long side) : PenAttack(side) {
}

NetPenAttack::NetPenAttack( long playerType, long side,
			    const vector3d x, const vector3d v,
			    long status, long swing,
			    long swingType, bool swingSide,
			    long afterSwing, long swingError,
			    const vector2d target,
			    const vector3d eye,
			    const vector3d lookAt, 
			    long pow, const vector2d spin,
			    double stamina, long statusMax,
			    long dragX, long dragY ) :
  PenAttack( playerType, side, x, v, status, swing, swingType, swingSide, 
	     afterSwing, swingError, target, eye, lookAt, pow, spin, 
	     stamina, statusMax, dragX, dragY ) {
}

NetPenAttack::~NetPenAttack() {
}

bool
NetPenAttack::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		    long *MouseYHistory, unsigned long *MouseBHistory,
		    int Histptr ) {
  PenAttack::Move( KeyHistory, MouseXHistory, MouseYHistory,MouseBHistory,
		   Histptr );

  // Calc the ball location of 0.1 second later. 
  // This part seems to be the same as Swing(). Consider again. 
  Ball *tmpBall;
  vector3d tmpBallX;
  vector2d tmpX;

  tmpBall = new Ball( &theBall );

  for ( int i = 0 ; i < 9 ; i++ )
    tmpBall->Move();
  tmpX[0] = m_x[0]+m_v[0]*0.08;
  tmpX[1] = m_x[1]+m_v[1]*0.08;

  if ( ((tmpBall->GetStatus() == 3 && m_side == 1) ||
	(tmpBall->GetStatus() == 1 && m_side == -1)) &&
       (tmpX[1]-tmpBall->GetX()[1])*m_side < 0.3 &&
       (tmpX[1]-tmpBall->GetX()[1])*m_side > -0.6 &&
       m_swing <= 10 ) {

    tmpBallX = tmpBall->GetX();

    // If the ball location becomes better at 1/100 second later, wait. 
    tmpBall->Move();
    if ( fabs(tmpX[1]+m_v[1]*0.01-tmpBall->GetX()[1]) >
	 fabs(tmpX[1]-tmpBallX[1]) ) {
      if ( (m_x[0]-tmpBallX[0])*m_side < 0 )
	Swing(3);
      else
	Swing(1);
    }
  }
  delete tmpBall;

  return true;
}
