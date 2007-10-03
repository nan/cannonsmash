/**
 * @file
 * @brief Implementation of TrainingPenDriveController class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2007  神南 吉宏(Kanna Yoshihiro)
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
#include "ComTrainingPenDriveController.h"
#include "Ball.h"
#include "PlayGame.h"

extern Ball theBall;

/**
 * Default constructor. 
 */
ComTrainingPenDriveController::ComTrainingPenDriveController()
  : ComPenDriveController() {
}

/**
 * Constructor. 
 * Set player type and side. 
 * 
 * @param side side of the player. 
 */
ComTrainingPenDriveController::ComTrainingPenDriveController(Player *parent)
  : ComPenDriveController(parent) {
}

/**
 * Destructor. 
 * Do nothing. 
 */
ComTrainingPenDriveController::~ComTrainingPenDriveController() {
}

/**
 * Decide the movement of this player object. 
 * 
 * @return returns true if succeeds. 
 */
bool
ComTrainingPenDriveController::Think() {
  double hitTX, hitTY;	// estimation time until ball reaches _hitX, _hitY
  double mx;

  // If the ball status changes, change _hitX, _hitY
  if ( _prevBallstatus != theBall.GetStatus() && m_parent->GetSwing() == 0 ){
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

  if ( fabs( _hitX[0]-(m_parent->GetX()[0]+m_parent->GetSide()*0.3) ) <
       fabs( _hitX[0]-(m_parent->GetX()[0]-m_parent->GetSide()*0.3) ) ||
       theBall.GetStatus() == 8 || _hitX[0]*m_parent->GetSide() > 0 )
    mx = m_parent->GetX()[0]+m_parent->GetSide()*0.3;
  else
    mx = m_parent->GetX()[0]-m_parent->GetSide()*0.3;

  vector3d v = m_parent->GetV();
  if ( hitTX > 0.0 ) {
    if ( v[0] > 0 && mx + v[0]*hitTX < _hitX[0] )
      v[0] += 0.1;
    else if ( v[0] < 0 && mx + v[0]*hitTX > _hitX[0] )
      v[0] -= 0.1;
    else if ( v[0]*fabs(v[0]*0.1)/2 < _hitX[0] - mx )
      v[0] += 0.1;
    else
      v[0] -= 0.1;
  } else {
    if ( v[0]*fabs(v[0]*0.1)/2 < _hitX[0] - mx )
      v[0] += 0.1;
    else
      v[0] -= 0.1;
  }

  if ( hitTY > 0.0 ) {
    if ( v[1] > 0 && m_parent->GetX()[1] + v[1]*hitTY < _hitX[1] )
      v[1] += 0.1;
    else if ( v[1] < 0 && m_parent->GetX()[1] + v[1]*hitTY > _hitX[1] )
      v[1] -= 0.1;
    else if ( v[1]*fabs(v[1]*0.1)/2 < _hitX[1] - m_parent->GetX()[1] )
      v[1] += 0.1;
    else
      v[1] -= 0.1;
  } else {
    if ( v[1]*fabs(v[1]*0.1)/2 < _hitX[1] - m_parent->GetX()[1] )
      v[1] += 0.1;
    else
      v[1] -= 0.1;
  }

  if ( v[0] > 5.0 )
    v[0] = 5.0;
  else if ( v[0] < -5.0 )
    v[0] = -5.0;
  if ( v[1] > 5.0 )
    v[1] = 5.0;
  else if ( v[1] < -5.0 )
    v[1] = -5.0;

  m_parent->SetV(v);

  // Toss
  if ( theBall.GetStatus() == 8 &&
       ((PlayGame *)Control::TheControl())->GetService() == m_parent->GetSide() &&
       fabs(v[0]) < 0.1 && fabs(v[1]) < 0.1 &&
       fabs(m_parent->GetX()[0]+m_parent->GetSide()*0.3-_hitX[0]) < 0.1 && fabs(m_parent->GetX()[1]-_hitX[1]) < 0.1 &&
       m_parent->GetSwing() == 0 ){
    theBall.Toss( m_parent, 2 );
    m_parent->StartServe(3);
    vector2d target = m_parent->GetTarget();
    target[1] = TABLELENGTH/8*m_parent->GetSide();
    m_parent->SetTarget(target);

    return true;
  }

  // Calc the ball location of 0.1 second later. 
  // This part seems to be the same as Swing(). Consider again. 
  Ball *tmpBall;

  tmpBall = new Ball( &theBall );

  for ( int i = 0 ; i < 10 ; i++ )
    tmpBall->Move();

  if ( ((tmpBall->GetStatus() == 3 && m_parent->GetSide() == 1) ||
	(tmpBall->GetStatus() == 1 && m_parent->GetSide() == -1)) &&
       (m_parent->GetX()[1]-tmpBall->GetX()[1])*m_parent->GetSide() < 0.3 &&
       (m_parent->GetX()[1]-tmpBall->GetX()[1])*m_parent->GetSide() > 0.0 ) {
    _hitX[0] = tmpBall->GetX()[0];
    _hitX[1] = tmpBall->GetX()[1];

    Player *opponent;
    if ( m_parent->GetSide() == -1 )
      opponent = Control::TheControl()->GetThePlayer();
    else
      opponent = Control::TheControl()->GetComPlayer();

    SetTargetX( opponent );

    m_parent->Swing( 3 );
  }
  delete tmpBall;

  return true;
}
