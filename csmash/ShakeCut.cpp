/* $Id$ */

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
#include "ShakeCut.h"
#include "Ball.h"
#include "Event.h"
#include "Network.h"
#include "Control.h"
#include "RCFile.h"

extern RCFile *theRC;

extern Ball   theBall;

extern long mode;

ShakeCut::ShakeCut() {
  m_playerType = PLAYER_SHAKECUT;
}

ShakeCut::ShakeCut(long side) : Player(side) {
  m_playerType = PLAYER_SHAKECUT;
}

ShakeCut::ShakeCut( long playerType, long side, const vector3d x, const vector3d v,
		    long status, long swing, long swingType, bool swingSide, long afterSwing,
		    long swingError, const vector2d target, const vector3d eye,
		    long pow, const vector2d spin, double stamina, long statusMax ) :
  Player( playerType, side, x, v, status, swing, swingType, swingSide, afterSwing,
	  swingError, target, eye, pow, spin, stamina, statusMax ) {
}

ShakeCut::~ShakeCut() {
}

bool
ShakeCut::AddStatus( long diff ) {
  // Add something in the future
  return Player::AddStatus( diff );
}

bool
ShakeCut::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
	      long *MouseYHistory, unsigned long *MouseBHistory,
	      int Histptr ) {
  vector3d prevV;
  prevV = m_v;

  Player::Move( KeyHistory, MouseXHistory, MouseYHistory,MouseBHistory,
		Histptr );

// Calc status
  if ( m_v.len() < 2.0 ) {
    AddStatus( 1 );
  }
  if ( (m_v-prevV).len() > 0.8-theRC->gameLevel*0.1 ) {
    AddStatus(-1);
  }

  return true;
}

bool
ShakeCut::Swing( long spin ) {
  Ball *tmpBall;

  if ( theBall.GetStatus() == 6 || theBall.GetStatus() == 7 )
    return false;

  if ( m_swing > 10 && m_swing < 30 )
    return false;

  if ( m_swing >= 30 )
    AddStatus( -(50-m_swing)*2 );

  m_swing = 11;
  m_pow = 8;

  // Decide SwingType by the hit point and spin, etc. 
  // Calc the ball location of 0.1 second later
  tmpBall = new Ball(&theBall);

  for ( int i = 0 ; i < 10 ; i++ )
    tmpBall->Move();

  if ( spin < 3 )
    m_swingSide = false;
  else
    m_swingSide = true;

  SwingType( tmpBall, spin );

  delete tmpBall;

  if (Control::TheControl()->GetThePlayer() == this && mode == MODE_MULTIPLAY)
    ::SendSwing( this );

  return true;
}

bool
ShakeCut::StartSwing( long spin ) {
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  if ( m_swing == 0 ){
    m_swing = 1;
    m_pow = 0;

    // Decide SwingType by the hit point and spin, etc. 
    // Calc the ball location of 0.2 second later
    tmpBall = new Ball(&theBall);

    for ( int i = 0 ; i < 20 ; i++ )
      tmpBall->Move();

    if ( (theBall.GetStatus() == 6 && m_side == 1) ||
	(theBall.GetStatus() == 7 && m_side == -1) ){	// Serve
      switch ( spin-1 ) {
      case 0:
	m_spin[0] = 0.0;
	m_spin[1] = 0.2;	// straight
	m_swingType = SWING_NORMAL;
	break;
      case 1:
	m_spin[0] = 0.0;
	m_spin[1] = -0.1;	// knuckle
	m_swingType = SWING_POKE;
	break;
      case 2:
	m_spin[0] = 0.0;
	m_spin[1] = -1.0;
	m_swingType = SWING_POKE;
	break;
      }

      m_swingSide = true;

      if ( Control::TheControl()->GetThePlayer() == this &&
	   mode == MODE_MULTIPLAY )
	::SendSwing( this );
    } else {
      if ( (m_x[0]-tmpBall->GetX()[0])*m_side > 0 )
	m_swingSide = false;
      else
	m_swingSide = true;

      SwingType( tmpBall, spin );
    }

    delete tmpBall;
  }

  return true;
}

bool
ShakeCut::HitBall() {
  vector3d v;
  double diff;
  double level;

  // Serve
  if ( ( (m_side == 1 && theBall.GetStatus() == 6) ||
         (m_side ==-1 && theBall.GetStatus() == 7) ) &&
       fabs( m_x[0]-theBall.GetX()[0] ) < 0.6 && fabs( m_x[1]-theBall.GetX()[1] ) < 0.3 ) {
    AddStatus( (long)-fabs(fabs(m_x[0]-theBall.GetX()[0])-0.3)*100 );
    diff = fabs( m_x[1]-theBall.GetX()[1] )*0.3;

    SwingError();

    if ( fabs(m_target[1]) < TABLELENGTH/16*2 )
      level = 0.95 - diff*1.0;
    else if ( fabs(m_target[1]) < TABLELENGTH/16*4 )
      level = 0.93-diff*1.5;
    else if ( fabs(m_target[1]) < TABLELENGTH/16*6 )
      level = 0.90-diff*2.0;
    else
      level = 0.90-diff*2.0;

    theBall.TargetToVS( m_target, level, m_spin, v );

    theBall.Hit( v, m_spin, this );
  } else if ( ((m_side == 1 && theBall.GetStatus() == 3) ||
	       (m_side ==-1 && theBall.GetStatus() == 1)) &&
	      fabs(m_x[0]-theBall.GetX()[0]) < 0.6 &&
	      ((GetSwingSide() && (m_x[0]-theBall.GetX()[0])*m_side < 0 ) ||
	       (!GetSwingSide() && (m_x[0]-theBall.GetX()[0])*m_side > 0 )) &&
	      (m_x[1]-theBall.GetX()[1])*m_side < 0.3 &&
	      (m_x[1]-theBall.GetX()[1])*m_side > -0.6 ) {

    //AddStatus( -fabs(fabs(m_x-theBall.GetX())-0.3)*100 );
    vector2d target;

    GetModifiedTarget( target );

    double maxVy;
    CalcLevel( &theBall, diff, level, maxVy );

    theBall.TargetToV( target, level, m_spin, v, 0.1, maxVy );

    // If your target is near the edge of the table, much more status point
    // is necessary. 

    if ((m_status-StatusBorder())*3/2000.0 <
	fabs(fabs(m_x[0]-theBall.GetX()[0])-0.3))
      AddError( v );

    // Reduce status
    m_afterSwing = (long)
      (hypot( theBall.GetV()[0]*0.8-v[0], theBall.GetV()[1]*0.8+v[1] )
       * (1.0+diff*10.0) + m_spin.len()*3.0 + fabs(theBall.GetSpin()[1])*2.0);

    if ( m_swingType == SWING_POKE || m_swingType == SWING_CUT )
      AddStatus( -m_afterSwing );
    else if ( ForeOrBack() )
      AddStatus( -m_afterSwing*2 );
    else
      AddStatus( -m_afterSwing*3 );

    if ( m_status == 1 )
      m_afterSwing *= 3;

    theBall.Hit( v, m_spin, this );
  } else {
    m_swingError = SWING_MISS;
    if ( Control::TheControl()->GetThePlayer() == this &&
	 mode == MODE_MULTIPLAY )
      Event::TheEvent()->SendBall();
  }

  return true;
}

bool
ShakeCut::SwingType( Ball *ball, long spin ) {
  m_spin[0] = 0.0;
  if ( (ball->GetStatus() == 3 && m_side == 1) ||
       (ball->GetStatus() == 1 && m_side == -1) ) {
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

// Target will be modified by the spin
// (now invalid)
#if 0
bool
ShakeCut::GetModifiedTarget( double &targetX, double &targetY ) {
  targetX = m_targetX;
  targetY = m_targetY + theBall.GetSpinY()*m_side*0.2;

  return true;
}
#else
bool
ShakeCut::GetModifiedTarget( vector2d &target ) {
  target = m_target;

  return true;
}
#endif

void
ShakeCut::CalcLevel( Ball *ball, double &diff, double &level, double &maxVy ) {
  vector2d target;

  GetModifiedTarget( target );

  if ( (m_x[1]-ball->GetX()[1])*m_side < 0 )
    diff = fabs( m_x[1]-ball->GetX()[1] )*0.1;
  else
    diff = fabs( m_x[1]-ball->GetX()[1] )*0.15;

  diff *= fabs(ball->GetSpin()[1])+1;

  SwingError();

  if ( m_swingType == SWING_CUT || m_swingType == SWING_POKE )
    level = 1 - fabs(target[1])/(TABLELENGTH/16)/40/2 -
      diff*fabs(target[1])/(TABLELENGTH/16)*0.5;
  else
    level = 1 - fabs(target[1])/(TABLELENGTH/16)/40 -
      diff*fabs(target[1])/(TABLELENGTH/16)*1.2;

  level *= (double)m_pow/20.0 + 0.5;

  if ( ForeOrBack() ) {
    switch ( m_swingType ) {
    case SWING_CUT:
    case SWING_POKE:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 8.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1])/3)*2.0;
      break;
    case SWING_NORMAL:
    case SWING_DRIVE:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 10.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1]))*4.0;
      break;
    case SWING_SMASH:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 15.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1]))*4.0;
      break;
    default:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 10.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1]))*4.0;
    }
  } else {
    switch ( m_swingType ) {
    case SWING_CUT:
    case SWING_POKE:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 7.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1])/3)*3.0;
      break;
    case SWING_NORMAL:
    case SWING_DRIVE:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 8.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1]))*5.0;
      break;
    case SWING_SMASH:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 12.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1]))*5.0;
      break;
    default:
      maxVy = hypot(ball->GetV()[0], ball->GetV()[1])*0.4 + 8.0 -
	(fabs(m_spin[1])+fabs(ball->GetSpin()[1]))*5.0;
    }
  }

  if ( level > 1.0 )
    level = 1.0;
}
