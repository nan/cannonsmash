/**
 * @file
 * @brief Implementation of ComPenDrive class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004  神南 吉宏(Kanna Yoshihiro)
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
#include "comPenDrive.h"
#include "Control.h"
#include "Ball.h"
#include "Player.h"
#include "PlayGame.h"
#include "RCFile.h"

extern RCFile *theRC;

extern Ball   theBall;

/**
 * Default constructor. 
 */
ComPenDrive::ComPenDrive() : PenDrive(), ComPlayer() {
}

/**
 * Constructor. 
 * Set player type and side. 
 * 
 * @param side side of the player. 
 */
ComPenDrive::ComPenDrive(long side) : PenDrive(side), ComPlayer() {
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
ComPenDrive::ComPenDrive( long playerType, long side, const vector3d x,
			  const vector3d v, long status, long swing, 
			  long swingType, bool swingSide, long afterSwing,
			  long swingError, const vector2d target, 
			  const vector3d eye, long pow, const vector2d spin, 
			  double stamina,long statusMax ) :
  PenDrive( playerType, side, x, v, status, swing, swingType,
	    swingSide, afterSwing, swingError, target, 
	    eye, pow, spin, stamina, statusMax ), ComPlayer() {
}

/**
 * Destructor. 
 * Do nothing. 
 */
ComPenDrive::~ComPenDrive() {
}

/**
 * Move this player object. 
 * Move this player and change m_status. Then, call Think() to decide
 * where to move and whether to swing or not. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
ComPenDrive::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		 long *MouseYHistory, unsigned long *MouseBHistory,
		 int Histptr ) {
  vector3d prevV;
  prevV = m_v;

  PenDrive::Move( KeyHistory, MouseXHistory, MouseYHistory, MouseBHistory,
		Histptr );

  Think();

// Calc status
  if ( (m_v-prevV).len() > 0.8-theRC->gameLevel*0.1 ) {
    AddStatus(-1);
  }

  return true;
}

/**
 * Decide where to move and when to swing. 
 *
 * @return returns true if succeeds. 
 */
bool
ComPenDrive::Think() {
  double hitT;	// estimation time until ball reaches _hitX, _hitY
  double mx;

  // If the ball status changes, change _hitX, _hitY
  if ( _prevBallstatus != theBall.GetStatus() && theBall.GetStatus() >= 0 ) {
    Hitarea( _hitX );

    _prevBallstatus = theBall.GetStatus();
  }

  if ( theBall.GetV()[1] != 0.0 )
    hitT = (_hitX[1] - theBall.GetX()[1])/theBall.GetV()[1]-TICK;
  else
    hitT = -1.0;

  mx = m_x[0]+m_side*0.3;

  if ( m_swing > 10 && m_swing <= 20 ) {
  } else {
    if ( hitT > 0.0 ) {
      double vx = (_hitX[0]-mx)/hitT;
      if ( vx > m_v[0]+0.1 )
	m_v[0] += 0.1;
      else if ( vx < m_v[0]-0.1 )
	m_v[0] -= 0.1;
      else
	m_v[0] = vx;
    } else {
      if ( m_v[0]*fabs(m_v[0]*0.1)/2 < _hitX[0] - mx )
	m_v[0] += 0.1;
      else
	m_v[0] -= 0.1;
    }
  }

  if ( m_swing > 10 && m_swing <= 20 ) {
  } else {
    if ( hitT > 0.0 ) {
      double vy = (_hitX[1]-m_x[1])/hitT;
      if ( vy > m_v[1]+0.1 )
	m_v[1] += 0.1;
      else if ( vy < m_v[1]-0.1 )
	m_v[1] -= 0.1;
      else
	m_v[1] = vy;
    } else {
      if ( m_v[1]*fabs(m_v[1]*0.1)/2 < _hitX[1] - m_x[1] )
	m_v[1] += 0.1;
      else
	m_v[1] -= 0.1;
    }
  }

  if ( m_swing == 19 ) {
    Player *opponent;
    if ( m_side == -1 )
      opponent = Control::TheControl()->GetThePlayer();
    else
      opponent = Control::TheControl()->GetComPlayer();

    SetTargetX( opponent );
  }

  if ( (theBall.GetStatus() == 0 && m_side == -1) ||
       (theBall.GetStatus() == 1 && m_side == -1) ||
       (theBall.GetStatus() == 2 && m_side == 1) ||
       (theBall.GetStatus() == 3 && m_side == 1) ||
       (theBall.GetStatus() == 4 && m_side == -1) ||
       (theBall.GetStatus() == 5 && m_side == 1) ) {
    if ( m_v[0] > 5.0 )
      m_v[0] = 5.0;
    else if ( m_v[0] < -5.0 )
      m_v[0] = -5.0;
    if ( m_v[1] > 5.0 )
      m_v[1] = 5.0;
    else if ( m_v[1] < -5.0 )
      m_v[1] = -5.0;
  } else {
    if ( m_v.len() >= 1.5 ) {
      double v = m_v.len();
      m_v = m_v/v*1.5;
    }
  }

  // Toss
#ifdef SCREENSHOT
  if ( theBall.GetStatus() == 8 &&
       ( (Control::TheControl()->IsPlaying() &&
	  ((PlayGame *)Control::TheControl())->GetService() == GetSide()) ) &&
       fabs(m_v[0]) < 0.2 && fabs(m_v[1]) < 0.2 &&
       fabs(m_x[0]+m_side*0.3-_hitX[0]) < 0.1 && fabs(m_x[1]-_hitX[1]) < 0.1 &&
       m_swing == 0 ){
#else
  if ( theBall.GetStatus() == 8 &&
       ( (Control::TheControl()->IsPlaying() &&
	  ((PlayGame *)Control::TheControl())->GetService() == GetSide()) ||
	 (!Control::TheControl()->IsPlaying() && GetSide() == 1) ) &&
       fabs(m_v[0]) < 0.2 && fabs(m_v[1]) < 0.2 &&
       fabs(m_x[0]+m_side*0.3-_hitX[0]) < 0.1 && fabs(m_x[1]-_hitX[1]) < 0.1 &&
       m_swing == 0 ){
#endif
    theBall.Toss( this, 2 );
    StartServe(3);
    m_target[1] = TABLELENGTH/6*m_side;

    return true;
  }

  if ( fabs( theBall.GetX()[1]+theBall.GetV()[1]*0.1 - _hitX[1] ) < 0.2 /*&&
	  m_swing == 0*/ ){
    // Calc the ball location of 0.1 second later. 
    // This part seems to be the same as Swing(). Consider again. 
    Ball *tmpBall;
    vector3d  tmpBallX;
    vector2d tmpX;

    tmpBall = new Ball(&theBall);

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
      if ( fabs(tmpX[1]+m_v[1]*0.01-tmpBall->GetX()[1]) <
	   fabs(tmpX[1]-tmpBallX[1]) &&
	   fabs(tmpX[1]-tmpBallX[1]) > LEVELMARGIN ) {
	delete tmpBall;
	return true;
      }

      _hitX[0] = tmpBallX[0];
      _hitX[1] = tmpBallX[1];

      if ( (tmpBallX[2]-TABLEHEIGHT)/fabs(tmpBallX[1] - m_target[1]) < 0.0 )
	m_target[1] = TABLELENGTH/4*m_side;
      else if ( (tmpBallX[2]-TABLEHEIGHT)/fabs(tmpBallX[1]-m_target[1]) < 0.1 )
	m_target[1] = TABLELENGTH/3*m_side;
      else
	m_target[1] = TABLELENGTH/16*6*m_side;

      if ( (m_x[0]-tmpBallX[0])*m_side < 0 )
	Swing(3);
      else
	Swing(1);

      m_pow = 7 + theRC->gameLevel;
    }
    delete tmpBall;
  }

  return true;
}

/**
 * Calc the point of hitting ball. 
 * If the ball haven't bound, calc bound point. 
 * Then, calc hit point from current ball location or bound location. 
 * 
 * @param hitX point of hitting [out]. 
 * @return returns true if succeeds. 
 */
bool
ComPenDrive::Hitarea( vector2d &hitX ) {
  double max = -1.0;             /* highest point of the ball */
  vector2d maxX = vector2d(0.0);

  if ( (theBall.GetStatus() == 3 && m_side == 1) ||
       (theBall.GetStatus() == 2 && m_side == 1) ||
       (theBall.GetStatus() == 0 && m_side == -1) ||
       (theBall.GetStatus() == 1 && m_side == -1) ||
       (theBall.GetStatus() == 4 && m_side == -1) ||
       (theBall.GetStatus() == 5 && m_side == 1) ) {
    max = GetBallTop( maxX, this );

    if ( max > 0 ) {
      hitX = maxX;
    }
  } else if ( theBall.GetStatus() == 8 ) {
    if ( (Control::TheControl()->IsPlaying() &&
	  ((PlayGame *)Control::TheControl())->GetService() == GetSide()) ||
	 GetSide() == 1 ) {
      if ( RAND(2) )
	hitX[0] = m_target[0];
      else
	hitX[0] = -m_target[0];
    } else
      hitX[0] = 0.0;
    hitX[1] = -(TABLELENGTH/2+0.2)*m_side;
  } else if ( theBall.GetStatus() < 6 ) {
    hitX[0] = 0.0;
    hitX[1] = -(TABLELENGTH/2+1.0)*m_side;
  }

  return true;
}

/**
 * Decide the x-coordinates of the target. 
 * 
 * @param opponent Player object of the opponent. 
 * @return returns true if succeeds. 
 */
bool
ComPenDrive::SetTargetX( Player *opponent ) {
  double width;

  switch ( theRC->gameLevel ) {
  case LEVEL_EASY:
    width = TABLEWIDTH/4;
    break;
  case LEVEL_NORMAL:
    width = TABLEWIDTH/2;
    break;
  case LEVEL_HARD:
  case LEVEL_TSUBORISH:
    width = TABLEWIDTH;
    break;
  default:
    return false;
  }

  if ( opponent->GetPlayerType() == PLAYER_PENDRIVE ) {
    switch ( RAND(4) ) {
    case 0:
      m_target[0] = -width*7/16;
      break;
    case 1:
      m_target[0] = -width*5/16;
      break;
    case 2:
      m_target[0] = -width*3/16;
      break;
    case 3:
      m_target[0] = -width*1/16;
      break;
    }
  } else {
    switch ( RAND(8) ) {
    case 0:
      m_target[0] = -width*7/16;
      break;
    case 1:
      m_target[0] = width*5/16;
      break;
    case 2:
      m_target[0] = -width*3/16;
      break;
    case 3:
      m_target[0] = -width*1/16;
      break;
    case 4:
      m_target[0] = width*1/16;
      break;
    case 5:
      m_target[0] = width*3/16;
      break;
    case 6:
      m_target[0] = width*5/16;
      break;
    case 7:
      m_target[0] = width*7/16;
      break;
    }
  }

  if ( theRC->gameLevel == LEVEL_TSUBORISH ) {
    if ( opponent->GetX()[0]+opponent->GetV()[0]*0.5 < 0.0 ) {
      m_target[0] = width*7/16;
    } else {
      m_target[0] = -width*7/16;
    }

    if ( RAND(4) == 0 ) {
      m_target[0] = -m_target[0];
    }
  }

  if ( m_v[0] > 1.5 ) {
    m_target[0] += TABLEWIDTH/2;
  } else if ( m_v[0] > 0.5 ) {
    m_target[0] += TABLEWIDTH/4;
  } else if ( m_v[0] < -1.5 ) {
    m_target[0] -= TABLEWIDTH/2;
  } else if ( m_v[0] < -0.5 ) {
    m_target[0] -= TABLEWIDTH/4;
  }

  if ( m_target[0] > TABLEWIDTH/2 )
    m_target[0] = TABLEWIDTH*7/16;
  if ( m_target[0] < -TABLEWIDTH/2 )
    m_target[0] = -TABLEWIDTH*7/16;

  return true;
}
