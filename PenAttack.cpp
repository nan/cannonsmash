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

PenAttack::PenAttack() {
  m_playerType = PLAYER_PEKO;
}

PenAttack::PenAttack(long side) {
  ::PenAttack();

  if ( side < 0 ) {
    m_side = -1;
    m_y = -m_y;
    m_targetY = -m_targetY;
  }
}

PenAttack::~PenAttack() {
}

bool
PenAttack::AddStatus( long diff ) {
  // 将来的には特色を出す. 
  return Player::AddStatus( diff );
}

bool
PenAttack::Move( unsigned long *KeyHistory, long *MouseXHistory,
	      long *MouseYHistory, unsigned long *MouseBHistory,
	      int Histptr ) {
  Player::Move( KeyHistory, MouseXHistory, MouseYHistory,MouseBHistory,
		Histptr );

// status 計算
  if ( hypot( m_vx, m_vy ) < 1.0 && m_swing <= 10 )
    AddStatus( 1 );

  return true;
}

bool
PenAttack::Swing( long power, double spin ) {
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  m_swing = 11;
  m_pow = power-1;

  // 予想打球点とボールの性質から打ち方を決める
  // 0.1秒後のボールの位置を推定する
  tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
		      theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
		      theBall.GetSpin(), theBall.GetStatus() );

  for ( int i = 0 ; i < 10 ; i++ )
    tmpBall->Move();

  SwingType( tmpBall );

  delete tmpBall;

  return true;
}

bool
PenAttack::StartSwing( long power, double spin ) { // 引数はサーブ時のみ有効
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  if ( m_swing == 0 ){
    m_swing = 1;
    m_pow = power-1;

    // 予想打球点とボールの性質から打ち方を決める
    // 0.2秒後のボールの位置を推定する
    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    for ( int i = 0 ; i < 20 ; i++ )
      tmpBall->Move();

    if ( (theBall.GetStatus() == 6 && m_side == 1) ||
	(theBall.GetStatus() == 7 && m_side == -1) ){	// サーブ
      m_swingType = SWING_POKE;
      m_spin = -m_pow*0.25;
    } else
      SwingType( tmpBall );

    delete tmpBall;
  }

  return true;
}

bool
PenAttack::HitBall() {
  double vx, vy, vz;
  double diff;
  double level;

// サーブ
  if ( ( (m_side == 1 && theBall.GetStatus() == 6) ||
         (m_side ==-1 && theBall.GetStatus() == 7) ) &&
       fabs( m_x-theBall.GetX() ) < 0.6 && fabs( m_y-theBall.GetY() ) < 0.3 ){
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

    theBall.Hit( vx, vy, vz, m_spin );
  } else {
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

      if ( fabs(m_targetY) < TABLELENGTH/16*2 )
	level = 0.95 - diff*1.0;
      else if ( fabs(m_targetY) < TABLELENGTH/16*4 )
	  level = 0.90-diff*2.0;
      else if ( fabs(m_targetY) < TABLELENGTH/16*6 )
	  level = 0.80-diff*4.0;
      else
	level = 0.80-diff*4.0;

      level -= (1-level)*m_spin/2;

      if ( diff*1000 > m_status ) {
	switch ( RAND(3) ) {
	case 0:		// オーバーミス
	  theBall.TargetToV( m_targetX, m_targetY+TABLELENGTH*m_side, level,
			     m_spin, vx, vy, vz, 0.1, 
			     hypot(theBall.GetVX(), theBall.GetVY())*0.6 +
			     10.0 - fabs(m_spin)*3.0 );
	  break;
	case 1:		// ネットミス
	  theBall.TargetToV( m_targetX, m_targetY, level, m_spin,
			     vx, vy, vz, 0.1, 
			     hypot(theBall.GetVX(), theBall.GetVY())*0.6 +
			     10.0 - fabs(m_spin)*3.0 );
	  vz -= fabs(vz)*0.3;	// 不十分?
	  break;
	case 2:		// コースミス
	  theBall.TargetToV( m_targetX+TABLEWIDTH*2.0, m_targetY+1.0, level,
			     m_spin, vx, vy, vz, 0.1, 
			     hypot(theBall.GetVX(), theBall.GetVY())*0.6 +
			     10.0 - fabs(m_spin)*3.0 );
	  break;
	}

	theBall.Hit( vx, vy, vz, m_spin );
	return true;
      }

      double maxVy;

      if ( ForeOrBack() ) {
	switch ( m_swingType ) {
	case SWING_CUT:
	case SWING_POKE:
	case SWING_NORMAL:
	case SWING_DRIVE:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.6 + 15.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*3.0;
	  break;
	case SWING_SMASH:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.6 + 25.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*3.0;
	  break;
	default:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.6 + 15.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*3.0;
	}
      } else {
	switch ( m_swingType ) {
	case SWING_CUT:
	case SWING_POKE:
	case SWING_NORMAL:
	case SWING_DRIVE:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.6 + 10.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*4.0;
	  break;
	case SWING_SMASH:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.6 + 12.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*4.0;
	  break;
        default:
	  maxVy = hypot(theBall.GetVX(), theBall.GetVY())*0.6 + 10.0 -
	    (fabs(m_spin)+fabs(theBall.GetSpin()))*4.0;
        }
      }

      double targetX, targetY;
      if ( theBall.GetX()-m_x > 0 )
	targetX = m_targetX + (theBall.GetX()-m_x-0.3)/0.6*TABLEWIDTH
	  *(220-m_status)/220;
      else
	targetX = m_targetX + (theBall.GetX()-m_x+0.3)/0.6*TABLEWIDTH
	  *(220-m_status)/220;

      if ( (m_y-theBall.GetY())*m_side < 0.3 &&
	   (m_y-theBall.GetY())*m_side > 0 )
	targetY = m_targetY + (theBall.GetY()-m_y)/0.6*TABLELENGTH/2
	  *(220-m_status)/220;
      else
	targetY = m_targetY + (theBall.GetY()-m_y)/1.2*TABLELENGTH/2
	  *(220-m_status)/220;

      theBall.TargetToV( targetX, m_targetY, level, m_spin,
			 vx, vy, vz, 0.1, maxVy );

      // ボールの強さによって体勢ゲージを減らす
      m_afterSwing = (long)(
	hypot( theBall.GetVX()*0.8-vx, theBall.GetVY()*0.8+vy )
	* (1.0+diff*10.0) + fabs(m_spin)*5.0 + fabs(theBall.GetSpin())*4.0);

      if ( ForeOrBack() || m_swingType == SWING_POKE )
	AddStatus( -m_afterSwing*2 );
      else
	AddStatus( -m_afterSwing*3 );

      if ( m_status == 1 )
	m_afterSwing *= 3;

      theBall.Hit( vx, vy, vz, m_spin );
    } else
      m_swingError = SWING_MISS;
  }
  return true;
}

bool
PenAttack::SwingType( Ball *ball ) {
  if ( (ball->GetStatus() == 3 && m_side == 1) ||
       (ball->GetStatus() == 1 && m_side == -1) ){
    if ( fabs(ball->GetX()) < TABLEWIDTH/2 &&
	 fabs(ball->GetY()) < TABLELENGTH/2 &&
	 (ball->GetZ()-TABLEHEIGHT-NETHEIGHT)/fabs(ball->GetY()) <
	 NETHEIGHT/(TABLELENGTH/2)*0.5 ){	// 台上の低い球
      if ( ball->GetSpin() < 0 ){
	m_swingType = SWING_POKE;
	m_spin = -m_pow*0.2-0.4;
      } else{
	m_swingType = SWING_NORMAL;
	m_spin = m_pow*0.2;
      }
    } else if ( ball->GetZ() < TABLEHEIGHT+NETHEIGHT ) {	// ネットの下
      if ( ForeOrBack() ) {
	m_swingType = SWING_DRIVE;
	m_spin = m_pow*0.2+0.4;
      } else {
	if ( ball->GetSpin() < 0 ) {
	  m_swingType = SWING_POKE;
	  m_spin = -m_pow*0.2-0.4;
	} else {
	  m_swingType = SWING_NORMAL;
	  m_spin = m_pow*0.2;
	}
      }	    
    } else if ( fabs(ball->GetY()) < TABLELENGTH/2+1.0 &&
		ball->GetZ() > TABLEHEIGHT+NETHEIGHT && ForeOrBack() ){
      m_swingType = SWING_SMASH;
      m_spin = 0.2;
    } else {
      m_swingType = SWING_NORMAL;
      m_spin = m_pow*0.2;
    }
  } else{
    m_swingType = SWING_NORMAL;
    m_spin = m_pow*0.2;
  }

  return true;
}
