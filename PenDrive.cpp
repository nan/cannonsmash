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

extern Ball   theBall;
extern Player *thePlayer;

extern Event theEvent;
extern long mode;

PenDrive::PenDrive() {
  m_playerType = PLAYER_PENDRIVE;
}

PenDrive::PenDrive(long side) : Player(side) {
  m_playerType = PLAYER_PENDRIVE;
}

PenDrive::PenDrive( long playerType, long side, double x, double y, double z,
		    double vx, double vy, double vz,long status, long swing,
		    long swingType, long afterSwing, long swingError,
		    double targetX, double targetY, double eyeX, double eyeY,
		    double eyeZ, long pow, double spin, double stamina ) :
  Player( playerType, side, x, y, z, vx, vy, vz, status, swing, swingType,
	  afterSwing, swingError, targetX, targetY, eyeX, eyeY, eyeZ,
	  pow, spin, stamina ) {
}

PenDrive::~PenDrive() {
}

bool
PenDrive::AddStatus( long diff ) {
  // 将来的には特色を出す. 
  return Player::AddStatus( diff );
}

bool
PenDrive::Move( unsigned long *KeyHistory, long *MouseXHistory,
	      long *MouseYHistory, unsigned long *MouseBHistory,
	      int Histptr ) {
  Player::Move( KeyHistory, MouseXHistory, MouseYHistory,MouseBHistory,
		Histptr );

// status 計算
  if ( hypot( m_vx, m_vy ) < 1.5 )
    AddStatus( 2 );

  return true;
}

bool
PenDrive::Swing( long spin ) {
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  m_swing = 11;
  m_pow = 0;

  // 予想打球点とボールの性質から打ち方を決める
  // 0.1秒後のボールの位置を推定する
  tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
		      theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
		      theBall.GetSpin(), theBall.GetStatus() );

  for ( int i = 0 ; i < 10 ; i++ )
    tmpBall->Move();

  SwingType( tmpBall, spin );

  delete tmpBall;

  if ( thePlayer == this && mode == MODE_MULTIPLAY )
    theEvent.SendSwing( this );

  return true;
}

bool
PenDrive::StartSwing( long spin ) { // 引数はサーブ時のみ有効
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  if ( m_swing == 0 ) {
    m_swing = 1;
    m_pow = 0;

    // 予想打球点とボールの性質から打ち方を決める
    // 0.2秒後のボールの位置を推定する
    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    for ( int i = 0 ; i < 20 ; i++ )
      tmpBall->Move();

    if ( (theBall.GetStatus() == 6 && m_side == 1) ||
	(theBall.GetStatus() == 7 && m_side == -1) ) {	// サーブ
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

      if ( thePlayer == this && mode == MODE_MULTIPLAY )
	theEvent.SendSwing( this );
    } else
      SwingType( tmpBall, spin );

    delete tmpBall;
  }

  return true;
}

bool
PenDrive::HitBall() {
  double vx, vy, vz;
  double diff;
  double level;

// サーブ
  if ( ( (m_side == 1 && theBall.GetStatus() == 6) ||
         (m_side ==-1 && theBall.GetStatus() == 7) ) &&
       fabs( m_x-theBall.GetX() ) < 0.6 && fabs( m_y-theBall.GetY() ) < 0.3 ) {
    AddStatus( -fabs(fabs(m_x-theBall.GetX())-0.3)*100 );
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
	 (m_y-theBall.GetY())*m_side < 0.3 &&
	 (m_y-theBall.GetY())*m_side > -0.6 ) {
      AddStatus( -fabs(fabs(m_x-theBall.GetX())-0.3)*100 );

      if ( (m_y-theBall.GetY())*m_side < 0 )
	diff = fabs( m_y-theBall.GetY() )*0.15;
      else
	diff = fabs( m_y-theBall.GetY() )*0.3;

      SwingError();

      if ( m_swingType == SWING_DRIVE )
	level = 1 - fabs(targetY)/(TABLELENGTH/16)/40/1.5 -
	  diff*fabs(targetY)/(TABLELENGTH/16)*0.8;
      else if ( m_swingType == SWING_NORMAL || m_swingType == SWING_SMASH )
	level = 1 - fabs(targetY)/(TABLELENGTH/16)/40 -
	  diff*fabs(targetY)/(TABLELENGTH/16);
      else
	level = 1 - fabs(targetY)/(TABLELENGTH/16)/40 -
	  diff*fabs(targetY)/(TABLELENGTH/16)*1.2;

      level -= (1-level)*m_spin/2;

      level *= (double)m_pow/20.0 + 0.5;

      double maxVy;

      if ( ForeOrBack() ) {
	switch ( m_swingType ) {
	case SWING_CUT:
	case SWING_POKE:
	case SWING_NORMAL:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.4 + 12.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*3.0;
	  break;
	case SWING_DRIVE:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.4 + 15.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin())/2)*3.0;
	  break;
	case SWING_SMASH:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.4 + 20.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*3.0;
	  break;
	default:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.4 + 12.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*3.0;
	}
      } else {
	switch ( m_swingType ) {
	case SWING_CUT:
	case SWING_POKE:
	case SWING_NORMAL:
	case SWING_DRIVE:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.4 + 8.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*4.0;
	  break;
	case SWING_SMASH:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.4 + 10.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*4.0;
	  break;
        default:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.4 + 8.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*4.0;
        }
      }

      double rad, Xdiff, Ydiff;
      if ( theBall.GetX()-m_x > 0 )
	Xdiff = (theBall.GetX()-m_x-0.3)/0.3*TABLEWIDTH/4*(220-m_status)/220;
      else
	Xdiff = (theBall.GetX()-m_x+0.3)/0.3*TABLEWIDTH/4*(220-m_status)/220;

      if ( (m_y-theBall.GetY())*m_side < 0.3 &&
	   (m_y-theBall.GetY())*m_side > 0 )
	Ydiff = (theBall.GetY()-m_y)/0.3*TABLELENGTH/8*(220-m_status)/220;
      else
	Ydiff = (theBall.GetY()-m_y)/0.6*TABLELENGTH/8*(220-m_status)/220;

      Xdiff *= (double)m_pow/10.0;
      Ydiff *= (double)m_pow/10.0;

      rad = RAND(360)*3.141592/180.0;

      theBall.TargetToV( targetX + Xdiff*cos(rad),
			 targetY + Ydiff*sin(rad),
			 level, m_spin, vx, vy, vz, 0.1, maxVy );

      // ボールの強さによって体勢ゲージを減らす
      m_afterSwing = (long)(hypot(m_vx*0.8-vx, m_vy*0.8+vy)*(1.0+diff*10.0) +
			    fabs(m_spin)*5.0+fabs(theBall.GetSpin())*4.0)+10;

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
PenDrive::SwingType( Ball *ball, long spin ) {
  if ( (ball->GetStatus() == 3 && m_side == 1) ||
       (ball->GetStatus() == 1 && m_side == -1) ) {
    if ( fabs(ball->GetX()) < TABLEWIDTH/2 &&
	 fabs(ball->GetY()) < TABLELENGTH/2 &&
	 (ball->GetZ()-TABLEHEIGHT-NETHEIGHT)/fabs(ball->GetY()) <
	 NETHEIGHT/(TABLELENGTH/2)*0.5 ) {	// 台上の低い球
      if ( ball->GetSpin() < 0 ) {
	m_swingType = SWING_POKE;
	m_spin = -spin*0.2-0.3;
      } else {
	m_swingType = SWING_NORMAL;
	m_spin = spin*0.2;
      }
    } else if ( ball->GetZ() < TABLEHEIGHT+NETHEIGHT*2 ) {	// ネットの下
      if ( ForeOrBack() ) {
	m_swingType = SWING_DRIVE;
	m_spin = spin*0.3+0.4;
      } else {
	if ( ball->GetSpin() < 0 ) {
	  m_swingType = SWING_POKE;
	  m_spin = -spin*0.2-0.3;
	} else {
	  m_swingType = SWING_NORMAL;
	  m_spin = spin*0.2;
	}
      }
    } else if ( fabs(ball->GetY()) < TABLELENGTH/2+1.0 &&
		ball->GetZ() > TABLEHEIGHT+NETHEIGHT && ForeOrBack() ) {
      m_swingType = SWING_SMASH;
      m_spin = 0.2;
    } else {
      m_swingType = SWING_NORMAL;
      m_spin = spin*0.2;
    }
  } else {
    m_swingType = SWING_NORMAL;
    m_spin = spin*0.2;
  }

  return true;
}

#if 0
// 回転によって修正されたtargetを表示する
bool
PenDrive::GetModifiedTarget( double &targetX, double &targetY ) {
  targetX = m_targetX;
  targetY = m_targetY + theBall.GetSpin()*m_side*0.4;

  return true;
}
#else	// しばらく封印
bool
PenDrive::GetModifiedTarget( double &targetX, double &targetY ) {
  targetX = m_targetX;
  targetY = m_targetY;

  return true;
}
#endif
