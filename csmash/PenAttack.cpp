/* $Id$ */

// Copyright (C) 2000  神南 吉宏(Kanna Yoshihiro)
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
#include "PenAttack.h"
#include "Ball.h"
#include "Event.h"

extern Ball   theBall;
extern Player *thePlayer;

extern Event theEvent;
extern long mode;

PenAttack::PenAttack() {
  m_playerType = PLAYER_PENATTACK;
}

PenAttack::PenAttack(long side) : Player(side) {
  m_playerType = PLAYER_PENATTACK;
}

PenAttack::PenAttack( long playerType, long side, double x, double y, double z,
		      double vx, double vy, double vz,long status, long swing,
		      long swingType, bool swingSide, long afterSwing,
		      long swingError,
		      double targetX, double targetY, double eyeX, double eyeY,
		      double eyeZ, long pow, double spin, double stamina,
		      long statusMax ) :
  Player( playerType, side, x, y, z, vx, vy, vz, status, swing, swingType,
	  swingSide, afterSwing, swingError, targetX, targetY,
	  eyeX, eyeY, eyeZ, pow, spin, stamina, statusMax ) {
}

PenAttack::~PenAttack() {
}

bool
PenAttack::AddStatus( long diff ) {
  // Add something in the future
  return Player::AddStatus( diff );
}

bool
PenAttack::Move( unsigned long *KeyHistory, long *MouseXHistory,
	      long *MouseYHistory, unsigned long *MouseBHistory,
	      int Histptr ) {
  Player::Move( KeyHistory, MouseXHistory, MouseYHistory,MouseBHistory,
		Histptr );

// Calc status
  static bool stAdj = false;
  if ( hypot( m_vx, m_vy ) < 1.0 && m_swing <= 10 ) {
    if ( stAdj ) {
      AddStatus( 1 );
      stAdj = false;
    } else
      stAdj = true;
  }

  return true;
}

bool
PenAttack::Swing( long spin ) {
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  m_swing = 11;
  m_pow = 8;

  // Decide SwingType by the hit point and spin, etc. 
  // Calc the ball location of 0.1 second later
  tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
		      theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
		      theBall.GetSpin(), theBall.GetStatus() );

  for ( int i = 0 ; i < 10 ; i++ )
    tmpBall->Move();

#if 0
#else
  if ( spin < 3 )
    m_swingSide = false;
  else
    m_swingSide = true;
#endif

  SwingType( tmpBall, spin );

  delete tmpBall;

  if ( thePlayer == this && mode == MODE_MULTIPLAY )
    theEvent.SendSwing( this );

  return true;
}

bool
PenAttack::StartSwing( long spin ) { // Argument is valid only on serve
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  if ( m_swing == 0 ){
    m_swing = 1;
    m_pow = 0;

    // Decide SwingType by the hit point and spin, etc. 
    // Calc the ball location of 0.2 second later
    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    for ( int i = 0 ; i < 20 ; i++ )
      tmpBall->Move();

    if ( (theBall.GetStatus() == 6 && m_side == 1) ||
	(theBall.GetStatus() == 7 && m_side == -1) ){	// Serve
      switch ( spin-1 ) {
      case 0:
	m_spin = 0.2;	// straight
	m_swingType = SWING_NORMAL;
	break;
      case 1:
	m_spin = -0.1;	// knuckle
	m_swingType = SWING_POKE;
	break;
      case 2:
	m_spin = -0.6;
	m_swingType = SWING_POKE;
	break;
      }

      m_swingSide = true;

      if ( thePlayer == this && mode == MODE_MULTIPLAY )
	theEvent.SendSwing( this );
    } else {
      if ( (m_x-tmpBall->GetX())*m_side > 0 )
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
PenAttack::HitBall() {
  double vx, vy, vz;
  double diff;
  double level;

  // Serve
  if ( ( (m_side == 1 && theBall.GetStatus() == 6) ||
         (m_side ==-1 && theBall.GetStatus() == 7) ) &&
       fabs( m_x-theBall.GetX() ) < 0.6 && fabs( m_y-theBall.GetY() ) < 0.3 ){
    AddStatus( (long)-fabs(fabs(m_x-theBall.GetX())-0.3F)*100 );
    diff = fabs( m_y-theBall.GetY() )*0.3;

    SwingError();

    if ( fabs(m_targetY) < TABLELENGTH/16*2 )
      level = 0.95 - diff*1.0;
    else if ( fabs(m_targetY) < TABLELENGTH/16*4 )
      level = 0.93-diff*1.5;
    else if ( fabs(m_targetY) < TABLELENGTH/16*6 )
      level = 0.90-diff*2.0;
    else
      level = 0.90-diff*2.0;

    theBall.TargetToVS( m_targetX, m_targetY, level, m_spin, vx, vy, vz );

    theBall.Hit( vx, vy, vz, m_spin, this );
  } else {
    double targetX, targetY;

    GetModifiedTarget( targetX, targetY );

    if ( ((m_side == 1 && theBall.GetStatus() == 3) ||
	  (m_side ==-1 && theBall.GetStatus() == 1)) &&
	 fabs(m_x-theBall.GetX()) < 0.6 && 
	 ((GetSwingSide() && (m_x-theBall.GetX())*m_side < 0 ) ||
	  (!GetSwingSide() && (m_x-theBall.GetX())*m_side > 0 )) &&
	 (m_y-theBall.GetY())*m_side < 0.3 &&
	 (m_y-theBall.GetY())*m_side > -0.6 ) {

      //AddStatus( -fabs(fabs(m_x-theBall.GetX())-0.3)*100 );

      double maxVy;
      CalcLevel( &theBall, diff, level, maxVy );

      theBall.TargetToV( targetX, targetY, level, m_spin, vx, vy, vz,
			 0.1, maxVy );

      double v;
      double n1x, n1y, n1z, n2x, n2y, n2z;
      double radDiff, radRand;

      radDiff = hypot( fabs(fabs(m_x-theBall.GetX())-0.3)/0.3, 
		       fabs(m_y-theBall.GetY())/0.3 );
      radDiff = sqrt( radDiff );
      radDiff *= (double)(200-m_status)/200*3.141592/18;

      v = sqrt(vx*vx+vy*vy+vz*vz);
      n1x = vy/hypot(vx, vy) * v*tan(radDiff);
      n1y = -vx/hypot(vx, vy) * v*tan(radDiff);
      n1z = 0;
      n2x = vx*vz/(v*hypot(vx, vy)) * v*tan(radDiff);
      n2y = vy*vz/(v*hypot(vx, vy)) * v*tan(radDiff);
      n2z = (vx*vx+vy*vy)/(v*hypot(vx, vy)) * v*tan(radDiff);

      // Hit the ball too fast --- net miss
      // Hit the ball too slow --- over miss
      printf( "Ball: y= %f vy= %f Player: %f\n",
	      theBall.GetY(), theBall.GetVY(), m_y );
      if ( (m_y-theBall.GetY())*m_side < 0 )
	radRand = (RAND(180)+180)*3.141592/180.0;
      else
	radRand = RAND(180)*3.141592/180.0;

      vx += n1x*cos(radRand)+n2x*sin(radRand);
      vy += n1y*cos(radRand)+n2y*sin(radRand);
      vz += n1z*cos(radRand)+n2z*sin(radRand);

      // Reduce status
      m_afterSwing = (long)(
	hypot( theBall.GetVX()*0.8-vx, theBall.GetVY()*0.8+vy )
	* (1.0+diff*10.0) + fabs(m_spin)*5.0 + fabs(theBall.GetSpin())*4.0);

      if ( ForeOrBack() || m_swingType == SWING_POKE )
	AddStatus( -m_afterSwing*2 );
      else
	AddStatus( -m_afterSwing*3 );

      if ( m_status == 1 )
	m_afterSwing *= 3;

      theBall.Hit( vx, vy, vz, m_spin, this );
    } else
      m_swingError = SWING_MISS;
  }
  return true;
}

bool
PenAttack::SwingType( Ball *ball, long spin ) {
  if ( (ball->GetStatus() == 3 && m_side == 1) ||
       (ball->GetStatus() == 1 && m_side == -1) ){
    if ( fabs(ball->GetX()) < TABLEWIDTH/2 &&
	 fabs(ball->GetY()) < TABLELENGTH/2 &&
	 (ball->GetZ()-TABLEHEIGHT-NETHEIGHT)/fabs(ball->GetY()) <
	 NETHEIGHT/(TABLELENGTH/2)*0.5 ){	// low ball on the table
      if ( ball->GetSpin() < 0 ){
	m_swingType = SWING_POKE;
#if 0
	m_spin = -spin*0.2-0.4;
#else
	m_spin = -0.8;
#endif
      } else{
	m_swingType = SWING_NORMAL;
#if 0
	m_spin = spin*0.2;
#else
	m_spin = 0.4;
#endif
      }
    } else if ( ball->GetZ() < TABLEHEIGHT+NETHEIGHT ) { // under the net
      if ( ForeOrBack() ) {
	m_swingType = SWING_DRIVE;
#if 0
	m_spin = spin*0.2+0.4;
#else
	m_spin = 0.8;
#endif
      } else {
	if ( ball->GetSpin() < 0 ) {
	  m_swingType = SWING_POKE;
#if 0
	  m_spin = -spin*0.2-0.4;
#else
	  m_spin = -0.8;
#endif
	} else {
	  m_swingType = SWING_NORMAL;
#if 0
	  m_spin = spin*0.2;
#else
	  m_spin = 0.4;
#endif
	}
      }	    
    } else if ( fabs(ball->GetY()) < TABLELENGTH/2+1.0 &&
		ball->GetZ() > TABLEHEIGHT+NETHEIGHT && ForeOrBack() ){
      m_swingType = SWING_SMASH;
      m_spin = 0.2;
    } else {
      m_swingType = SWING_NORMAL;
#if 0
      m_spin = spin*0.2;
#else
      m_spin = 0.4;
#endif
    }
  } else{
    m_swingType = SWING_NORMAL;
#if 0
    m_spin = spin*0.2;
#else
    m_spin = 0.4;
#endif
  }

  return true;
}

// Target will be modified by the spin
// (now invalid)
#if 0
bool
PenAttack::GetModifiedTarget( double &targetX, double &targetY ) {
  targetX = m_targetX;
  targetY = m_targetY + theBall.GetSpin()*m_side*0.5;

  return true;
}
#else
bool
PenAttack::GetModifiedTarget( double &targetX, double &targetY ) {
  targetX = m_targetX;
  targetY = m_targetY;

  return true;
}
#endif

void
PenAttack::CalcLevel(Ball *ball, double &diff, double &level, double &maxVy) {
  double targetX, targetY;

  GetModifiedTarget( targetX, targetY );

  if ( (m_y-ball->GetY())*m_side < 0 )
    diff = fabs( m_y-ball->GetY() )*0.15;
  else
    diff = fabs( m_y-ball->GetY() )*0.3;

  SwingError();

  level = 1 - fabs(targetY)/(TABLELENGTH/16)/40 -
    diff*fabs(targetY)/(TABLELENGTH/16);

  level -= (1-level)*m_spin/2;

  level *= (double)m_pow/20.0 + 0.5;

  if ( ForeOrBack() ) {
    switch ( m_swingType ) {
    case SWING_CUT:
    case SWING_POKE:
    case SWING_NORMAL:
    case SWING_DRIVE:
      maxVy = hypot(ball->GetVX(), ball->GetVY())*0.6 + 15.0 -
	(fabs(m_spin)+fabs(ball->GetSpin()))*3.0;
      break;
    case SWING_SMASH:
      maxVy = hypot(ball->GetVX(), ball->GetVY())*0.6 + 25.0 -
	(fabs(m_spin)+fabs(ball->GetSpin()))*3.0;
      break;
    default:
      maxVy = hypot(ball->GetVX(), ball->GetVY())*0.6 + 15.0 -
	(fabs(m_spin)+fabs(ball->GetSpin()))*3.0;
    }
  } else {
    switch ( m_swingType ) {
    case SWING_CUT:
    case SWING_POKE:
    case SWING_NORMAL:
    case SWING_DRIVE:
      maxVy = hypot(ball->GetVX(), ball->GetVY())*0.6 + 12.0 -
	(fabs(m_spin)+fabs(ball->GetSpin()))*4.0;
      break;
    case SWING_SMASH:
      maxVy = hypot(ball->GetVX(), ball->GetVY())*0.6 + 15.0 -
	(fabs(m_spin)+fabs(ball->GetSpin()))*4.0;
      break;
    default:
      maxVy = hypot(ball->GetVX(), ball->GetVY())*0.6 + 12.0 -
	(fabs(m_spin)+fabs(ball->GetSpin()))*4.0;
    }
  }
}
