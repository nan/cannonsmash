/**
 * @file
 * @brief Implementation of TrainingPenDrive class. 
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
#include "TrainingPenDrive.h"
#include "Ball.h"

extern Ball   theBall;

/**
 * Default constructor. 
 */
TrainingPenDrive::TrainingPenDrive() : PenDrive() {
}

/**
 * Constructor. 
 * Set player type and side. 
 * 
 * @param side side of the player. 
 */
TrainingPenDrive::TrainingPenDrive(long side) : PenDrive(side) {
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
TrainingPenDrive::TrainingPenDrive( long playerType, long side, 
				    const vector3d x, const vector3d v, 
				    long status, long swing, 
				    long swingType, bool swingSide, 
				    long afterSwing, long swingError, 
				    const vector2d target, 
				    const vector3d eye, long pow, 
				    const vector2d spin, double stamina, 
				    long statusMax ) : 
  PenDrive( playerType, side, x, v, status, swing, swingType, swingSide, 
	    afterSwing, swingError, target, eye, pow, spin, stamina, 
	    statusMax ) {
}

/**
 * Destructor. 
 * Do nothing. 
 */
TrainingPenDrive::~TrainingPenDrive() {
}

/**
 * Change status value. 
 * Do nothing because training mode doesn't support status. 
 * 
 * @param diff this value is added to m_status. 
 * @return returns true if succeeds. 
 */
bool
TrainingPenDrive::AddStatus( long diff ) {
  // Add something in the future
  return true;
}

/**
 * Move this player object. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
TrainingPenDrive::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
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

/**
 * Hit the ball with racket. 
 * Referring the relative location of player and the ball, this method
 * decides the velocity and spin of the ball. 
 * 
 * @return returns true if succeeds. 
 */
bool
TrainingPenDrive::HitBall() {
  vector3d v;

  // Serve
  if ( ( (m_side == 1 && theBall.GetStatus() == 6) ||
         (m_side ==-1 && theBall.GetStatus() == 7) ) &&
       fabs( m_x[0]-theBall.GetX()[0] ) < 0.6 &&
       fabs( m_x[1]-theBall.GetX()[1] ) < 0.3 ) {
    SwingError();

    vector2d spin;
    spin[0] = 0.0;
    spin[1] = 0.5;
    theBall.TargetToVS( m_target, 0.8, spin, v );

    theBall.Hit( v, spin, this );
  } else {
    PenDrive::HitBall();
  }
  return true;
}
