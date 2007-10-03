/**
 * @file
 * @brief Implementation of TrainingPenDrive class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2004, 2007  神南 吉宏(Kanna Yoshihiro)
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
#include "ComTrainingPenDrive.h"
#include "Ball.h"
#include "Player.h"
#include "Control.h"
#include "PlayGame.h"
#include "Training.h"

extern Ball   theBall;

/**
 * Default constructor. 
 */
ComTrainingPenDrive::ComTrainingPenDrive() : PenDrive() {
  m_playerType = PLAYER_PENDRIVETRAINER;
}

/**
 * Constructor. 
 * Set player type and side. 
 * 
 * @param side side of the player. 
 */
ComTrainingPenDrive::ComTrainingPenDrive(long side) :
  PenDrive(side) {
}

/**
 * Constructor which specifies almost all member variables. 
 * 
 * @param playerType player type (pen attack, etc. )
 * @param side side (1 or -1)
 * @param x location of the player
 * @param v velocity of the player
 * @param status status of the player (0 - 200)
 * @param swing swing status of the player (0-50)
 * @param swingType type of swing (smash, cut, etc. )
 * @param swingSide side of swing (forehand or backhand)
 * @param afterSwing afterswing stop penalty
 * @param swingError valuation of the swing (good, bad, miss, etc. )
 * @param target location of the target
 * @param eye location of the camera
 * @param pow power to hit the ball
 * @param spin spin to hit the ball
 * @param stamina stamina (not used currently)
 * @param statusMax max of the status
 */
ComTrainingPenDrive::ComTrainingPenDrive( long playerType, long side,
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
  PenDrive( playerType, side, x, v, status, swing, swingType, swingSide, 
	    afterSwing, swingError, target, eye, lookAt, pow, spin, 
	    stamina, statusMax, dragX, dragY ) {
}

/**
 * Destructor. 
 * Do nothing. 
 */
ComTrainingPenDrive::~ComTrainingPenDrive() {
}

/**
 * Hit the ball with racket. 
 * Referring the relative location of player and the ball, this method
 * decides the velocity and spin of the ball. 
 * 
 * @return returns true if succeeds. 
 */
bool
ComTrainingPenDrive::HitBall() {
  vector3d v;
  double level;

  // Serve
  if (canServe(&theBall) &&
      fabs( m_x[0]-theBall.GetX()[0] ) < 0.6 &&
      fabs( m_x[1]-theBall.GetX()[1] ) < 0.3 ) {
    PenDrive::HitBall();
  } else {
    if (canHitBall(&theBall)) {
      level = 1.0 - 1.0/((double)((Training *)Control::TheControl())->GetTrainingCount()/20.0+1.5);

      vector2d target;
      target[0] = -TABLEWIDTH/5*2 + 
	TABLEWIDTH/5*4*(((Training *)Control::TheControl())->GetTrainingCount()%2);
      target[1] = TABLELENGTH/5*2*m_side;
      theBall.TargetToV( target, level, m_spin, v, 0.1, 20.0 );

      theBall.Hit( v, m_spin, this );
    } else
      m_swingError = SWING_MISS;
  }

  return true;
}
