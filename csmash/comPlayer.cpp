/**
 * @file
 * @brief Implementation of ComPlayer class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

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
#include "Player.h"
#include "comPlayer.h"
#include "Ball.h"

extern Ball   theBall;

/**
 * Default constructor. 
 */
ComPlayer::ComPlayer() {
  _prevBallstatus = 0;
  _hitX[0] = 0;
  _hitX[1] = -TABLELENGTH/3;
}

/**
 * Destructor. 
 * Do nothing. 
 */
ComPlayer::~ComPlayer() {
}

/**
 * Calculate peak of the ball. 
 * 
 * @param maxX peak point [out]
 * @param p Player object who want to hit the ball at peak point. 
 * @return returns the height of the peak. 
 */
double
ComPlayer::GetBallTop( vector2d &maxX, Player *p ) {
  Ball *tmpBall;
  double max = -1.0;             /* highest point of the ball */

  tmpBall = new Ball(&theBall);

  while ( tmpBall->GetStatus() != -1 ) {
    if ( (tmpBall->GetStatus() == 3 && p->GetSide() == 1) ||
	 (tmpBall->GetStatus() == 1 && p->GetSide() == -1) ) {
      if ( tmpBall->GetX()[2] > max &&
	   fabs(tmpBall->GetX()[1]) < TABLELENGTH/2+1.0 &&
	   fabs(tmpBall->GetX()[1]) > TABLELENGTH/2-0.5 ) {
	maxX[0] = tmpBall->GetX()[0];
	maxX[1] = tmpBall->GetX()[1];
	max = tmpBall->GetX()[2];
      }
    }
    tmpBall->Move();
  }

  delete tmpBall;
  return max;
}
