/* $Id$ */

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
#include "ComTrainingPenDrive.h"
#include "Ball.h"
#include "Player.h"
#include "Control.h"
#include "PlayGame.h"
#include "Training.h"

extern Ball   theBall;

ComTrainingPenDrive::ComTrainingPenDrive() : ComPenDrive() {
}

ComTrainingPenDrive::ComTrainingPenDrive(long side) :
  ComPenDrive(side) {
}

ComTrainingPenDrive::ComTrainingPenDrive( long playerType, long side,
					  const vector3d x,const vector3d v,
					  long status, long swing, 
					  long swingType, bool swingSide,
					  long afterSwing, long swingError,
					  const vector2d target, 
					  const vector3d eye, long pow,
					  const vector2d spin, 
					  double stamina,long statusMax ) :
  ComPenDrive( playerType, side, x, v, status, swing,
	       swingType, swingSide, afterSwing, swingError, target, 
	       eye, pow, spin, stamina, statusMax ) {
}

ComTrainingPenDrive::~ComTrainingPenDrive() {
}

bool
ComTrainingPenDrive::Think() {
  double hitTX, hitTY;	// estimation time until ball reaches _hitX, _hitY
  double mx;

  // If the ball status changes, change _hitX, _hitY
  if ( _prevBallstatus != theBall.GetStatus() && m_swing == 0 ){
    Hitarea( _hitX );

    _prevBallstatus = theBall.GetStatus();
  }

  if ( theBall.GetV()[0] != 0.0 )
    hitTX = (_hitX[0] - theBall.GetX()[0])/theBall.GetV()[0];
  else
    hitTX = -1.0;

  if ( theBall.GetV()[1] != 0.0 )
    hitTY = (_hitX[1] - theBall.GetX()[1])/theBall.GetV()[1];
  else
    hitTY = -1.0;

  if ( fabs( _hitX[0]-(m_x[0]+m_side*0.3) ) <
       fabs( _hitX[0]-(m_x[0]-m_side*0.3) ) ||
       theBall.GetStatus() == 8 || _hitX[0]*m_side > 0 )
    mx = m_x[0]+m_side*0.3;
  else
    mx = m_x[0]-m_side*0.3;

  if ( hitTX > 0.0 ) {
    if ( m_v[0] > 0 && mx + m_v[0]*hitTX < _hitX[0] )
      m_v[0] += 0.1;
    else if ( m_v[0] < 0 && mx + m_v[0]*hitTX > _hitX[0] )
      m_v[0] -= 0.1;
    else if ( m_v[0]*fabs(m_v[0]*0.1)/2 < _hitX[0] - mx )
      m_v[0] += 0.1;
    else
      m_v[0] -= 0.1;
  } else {
    if ( m_v[0]*fabs(m_v[0]*0.1)/2 < _hitX[0] - mx )
      m_v[0] += 0.1;
    else
      m_v[0] -= 0.1;
  }

  if ( hitTY > 0.0 ) {
    if ( m_v[1] > 0 && m_x[1] + m_v[1]*hitTY < _hitX[1] )
      m_v[1] += 0.1;
    else if ( m_v[1] < 0 && m_x[1] + m_v[1]*hitTY > _hitX[1] )
      m_v[1] -= 0.1;
    else if ( m_v[1]*fabs(m_v[1]*0.1)/2 < _hitX[1] - m_x[1] )
      m_v[1] += 0.1;
    else
      m_v[1] -= 0.1;
  } else {
    if ( m_v[1]*fabs(m_v[1]*0.1)/2 < _hitX[1] - m_x[1] )
      m_v[1] += 0.1;
    else
      m_v[1] -= 0.1;
  }

  if ( m_v[0] > 5.0 )
    m_v[0] = 5.0;
  else if ( m_v[0] < -5.0 )
    m_v[0] = -5.0;
  if ( m_v[1] > 5.0 )
    m_v[1] = 5.0;
  else if ( m_v[1] < -5.0 )
    m_v[1] = -5.0;

  // Toss
  if ( theBall.GetStatus() == 8 &&
       ((PlayGame *)Control::TheControl())->GetService() == GetSide() &&
       fabs(m_v[0]) < 0.1 && fabs(m_v[1]) < 0.1 &&
       fabs(m_x[0]+m_side*0.3-_hitX[0]) < 0.1 && fabs(m_x[1]-_hitX[1]) < 0.1 &&
       m_swing == 0 ){
    theBall.Toss( this, 2 );
    StartSwing( 3 );
    m_target[1] = TABLELENGTH/8*m_side;

    return true;
  }

  // Calc the ball location of 0.1 second later. 
  // This part seems to be the same as Swing(). Consider again. 
  Ball *tmpBall;

  tmpBall = new Ball( &theBall );

  for ( int i = 0 ; i < 10 ; i++ )
    tmpBall->Move();

  if ( ((tmpBall->GetStatus() == 3 && m_side == 1) ||
	(tmpBall->GetStatus() == 1 && m_side == -1)) &&
       (m_x[1]-tmpBall->GetX()[1])*m_side < 0.3 &&
       (m_x[1]-tmpBall->GetX()[1])*m_side > 0.0 ) {
    _hitX[0] = tmpBall->GetX()[0];
    _hitX[1] = tmpBall->GetX()[1];

    Player *opponent;
    if ( m_side == -1 )
      opponent = Control::TheControl()->GetThePlayer();
    else
      opponent = Control::TheControl()->GetComPlayer();

    SetTargetX( opponent );

    Swing( 3 );
  }
  delete tmpBall;

  return true;
}

bool
ComTrainingPenDrive::HitBall() {
  vector3d v;
  double level;

  // Serve
  if ( ( (m_side == 1 && theBall.GetStatus() == 6) ||
         (m_side ==-1 && theBall.GetStatus() == 7) ) &&
       fabs( m_x[0]-theBall.GetX()[0] ) < 0.6 &&
       fabs( m_x[1]-theBall.GetX()[1] ) < 0.3 ) {
    PenDrive::HitBall();
  } else {
    if ( ((m_side == 1 && theBall.GetStatus() == 3) ||
	  (m_side ==-1 && theBall.GetStatus() == 1)) ) {
      ((Training *)Control::TheControl())->AddTrainingCount();

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
