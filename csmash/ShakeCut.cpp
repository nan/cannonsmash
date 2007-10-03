/**
 * @file
 * @brief Implementation of ShakeCut class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004, 2007  神南 吉宏(Kanna Yoshihiro)
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
#include "ShakeCut.h"
#include "Ball.h"
#include "Event.h"
#include "Network.h"
#include "Control.h"
#include "RCFile.h"

extern RCFile *theRC;

extern Ball   theBall;

extern long mode;

const double shakecutMaxForehandSpeed[6] = {10.0, 9.0, 15.0, 10.0, 9.0, 10.0};
const double shakecutMaxBackhandSpeed[6] = {9.0, 8.0, 12.0, 9.0, 8.0, 9.0};
const double shakecutDiffCoeff[6] = {1.2, 0.5, 1.2, 1.2, 0.5, 1.2};

const double shakecutServeParam[][7] = 
  { {SERVE_NORMAL,     0.0, 0.0,  0.0,  0.1,  0.0,  0.2}, 
    {SERVE_POKE,       0.0, 0.0,  0.0, -0.4,  0.0, -1.0}, 
    {SERVE_SIDESPIN1, -0.6, 0.2, -0.8,  0.0, -0.6, -0.2}, 
    {SERVE_SIDESPIN2,  0.6, 0.2,  0.8,  0.0,  0.6, -0.2},
    {-1,               0.0, 0.0,  0.0,  0.0,  0.0,  0.0}};

/**
 * Default constructor. 
 * Set player type to shake cut. 
 */
ShakeCut::ShakeCut() {
  m_playerType = PLAYER_SHAKECUT;

  WALKSPEED = 2.0;
  WALKBONUS = 1;

  FOREHAND_BOUNCE_RATE = 0.4;
  FOREHAND_SPINEFFECT_RATE = 4.0;
  BACKHAND_BOUNCE_RATE = 0.4;
  BACKHAND_SPINEFFECT_RATE = 5.0;
  MAX_FOREHAND_SPEED = (double *)shakecutMaxForehandSpeed;
  MAX_BACKHAND_SPEED = (double *)shakecutMaxBackhandSpeed;

  DIFF_COEFF = (double *)shakecutDiffCoeff;

  AFTERSWING_PENALTY = 0.95;

  memcpy(SERVEPARAM, shakecutServeParam, sizeof(shakecutServeParam));
}

/**
 * Constructor. 
 * Set player type and side. 
 * 
 * @param side side of the player. 
 */
ShakeCut::ShakeCut(long side) : Player(side) {
  m_playerType = PLAYER_SHAKECUT;

  WALKSPEED = 2.0;
  WALKBONUS = 1;

  FOREHAND_BOUNCE_RATE = 0.4;
  FOREHAND_SPINEFFECT_RATE = 4.0;
  BACKHAND_BOUNCE_RATE = 0.4;
  BACKHAND_SPINEFFECT_RATE = 5.0;
  MAX_FOREHAND_SPEED = (double *)shakecutMaxForehandSpeed;
  MAX_BACKHAND_SPEED = (double *)shakecutMaxBackhandSpeed;

  DIFF_COEFF = (double *)shakecutDiffCoeff;

  AFTERSWING_PENALTY = 0.95;

  memcpy(SERVEPARAM, shakecutServeParam, sizeof(shakecutServeParam));
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
ShakeCut::ShakeCut( long playerType, long side, const vector3d x,
		    const vector3d v, long status, long swing,
		    long swingType, bool swingSide, long afterSwing,
		    long swingError, const vector2d target,
		    const vector3d eye, const vector3d lookAt, 
		    long pow, const vector2d spin, double stamina,
		    long statusMax, long dragX, long dragY ) :
  Player( playerType, side, x, v, status, swing, swingType, swingSide, 
	  afterSwing, swingError, target, eye, lookAt, pow, spin, 
	  stamina, statusMax, dragX, dragY ) {

  WALKSPEED = 2.0;
  WALKBONUS = 1;

  FOREHAND_BOUNCE_RATE = 0.4;
  FOREHAND_SPINEFFECT_RATE = 4.0;
  BACKHAND_BOUNCE_RATE = 0.4;
  BACKHAND_SPINEFFECT_RATE = 5.0;
  MAX_FOREHAND_SPEED = (double *)shakecutMaxForehandSpeed;
  MAX_BACKHAND_SPEED = (double *)shakecutMaxBackhandSpeed;

  DIFF_COEFF = (double *)shakecutDiffCoeff;

  AFTERSWING_PENALTY = 0.95;

  memcpy(SERVEPARAM, shakecutServeParam, sizeof(shakecutServeParam));
}

/**
 * Destructor. 
 * Do nothing. 
 */
ShakeCut::~ShakeCut() {
}

/**
 * Decide swing type. 
 * Swing type is defined by the ball location and player type. 
 * 
 * @param ball the ball to be hit
 * @param spin spin of which the player intend to set. 
 * @return returns true if succeeds
 */
bool
ShakeCut::SwingType( Ball *ball, long spin ) {
  m_spin[0] = 0.0;
  if (canHitBall(ball)) {
    if ( fabs(ball->GetX()[0]) < TABLEWIDTH/2 &&
	 fabs(ball->GetX()[1]) < TABLELENGTH/2 &&
	 (ball->GetX()[2]-TABLEHEIGHT-NETHEIGHT)/fabs(ball->GetX()[1]) <
	 NETHEIGHT/(TABLELENGTH/2) ) {	// low ball on the table
      if ( ball->GetSpin()[1] <= -0.2 ) {
	m_swingType = SWING_POKE;
	//m_spin = -spin*0.2-0.4;
	m_spin[1] = -0.8;
      } else {
	m_swingType = SWING_NORMAL;
	m_spin[1] = 0.2;
      }
    } else if ( fabs(ball->GetX()[1]) > TABLELENGTH/2 &&
		ball->GetX()[2] < TABLEHEIGHT+NETHEIGHT*2 ){ // middle height
      if ( ball->GetV()[2] < 0 ) {
	m_swingType = SWING_CUT;
	//m_spin = -spin*0.3-0.4;
	m_spin[1] = -1.0;
      } else {
	m_swingType = SWING_DRIVE;
	//m_spin = spin*0.2+0.2;
	m_spin[1] = 0.6;
      }
    } else {	// high ball
      m_swingType = SWING_SMASH;
      m_spin[1] = 0.2;
    }
  }
  else{
    m_swingType = SWING_NORMAL;
    m_spin[1] = 0.2;
  }

  return true;
}
