/**
 * @file
 * @brief Implementation of Player class. 
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
#include "Player.h"
#include "Ball.h"
#include "BaseView.h"
#include "Control.h"
#include "Event.h"
#include "PenAttack.h"
#include "PenDrive.h"
#include "ShakeCut.h"
#include "comPenAttack.h"
#include "comPenDrive.h"
#include "comShakeCut.h"
#include "TrainingPenAttack.h"
#include "TrainingPenDrive.h"
#include "ComTrainingPenAttack.h"
#include "ComTrainingPenDrive.h"
#include "HitMark.h"
#include "PlayGame.h"
#include "Network.h"
#include "RCFile.h"
#ifdef LOGGING
#include "Logging.h"
#endif

extern RCFile *theRC;

extern Ball   theBall;

extern long mode;

/***********************************************************************
 *	Class  Player
 ***********************************************************************/

/**
 * Default constructor. 
 */
Player::Player() : 
  m_x((const double[]){0.0, -TABLELENGTH/2-0.2, 1.4}), 
  m_v((const double[]){0.0, 0.0, 0.0}),
  m_target((const double[]){0.0, TABLELENGTH/16*5}),
  m_eye((const double[]){0.0, -1.0, 0.2}),
  m_lookAt((const double[]){0.0, TABLELENGTH/2, TABLEHEIGHT}), 
  m_spin((const double[]){0.0, 0.0}),
  m_lastSendX((const double[]){0.0, 0.0, 0.0}), 
  m_lastSendV((const double[]){0.0, 0.0, 0.0}) {

  m_side = 1;
  m_playerType = PLAYER_PROTO;

  m_status = 200;
  m_swing = 0;
  m_afterSwing = 0;
  m_swingType = SWING_NORMAL;
  m_swingError = SWING_PERFECT;

  m_pow = 0;
  m_lookAt[1] *= m_side;
  m_stamina = 80.0;
  m_statusMax = 200;
  m_lastSendCount = 0;

  m_View = NULL;

}

/**
 * Constructor. 
 * Set player side. 
 * 
 * @param side side of the player. 
 */
Player::Player( long side ) :
  m_x((const double[]){0.0, -TABLELENGTH/2-0.2, 1.4}), 
  m_v((const double[]){0.0, 0.0, 0.0}),
  m_target((const double[]){0.0, TABLELENGTH/16*5}),
  m_eye((const double[]){0.0, -1.0, 0.2}),
  m_lookAt((const double[]){0.0, TABLELENGTH/2, TABLEHEIGHT}), 
  m_spin((const double[]){0.0, 0.0}),
  m_lastSendX((const double[]){0.0, 0.0, 0.0}), 
  m_lastSendV((const double[]){0.0, 0.0, 0.0}) {

  m_side = side;
  if ( side < 0 ) {
    m_x[1] = -m_x[1];
    m_target[1] = -m_target[1];
    m_eye[1] = -m_eye[1];
  }

  m_playerType = PLAYER_PROTO;

  m_status = 200;
  m_swing = 0;
  m_afterSwing = 0;
  m_swingType = SWING_NORMAL;
  m_swingError = SWING_PERFECT;

  m_pow = 0;
  m_lookAt[1] *= m_side;
  m_stamina = 80.0;
  m_lastSendCount = 0;

  m_View = NULL;

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
Player::Player( long playerType, long side, vector3d x, const vector3d v,
		long status, long swing, long swingType, bool swingSide, long afterSwing,
		long swingError, const vector2d target, const vector3d eye,
		long pow, const vector2d spin, double stamina,long statusMax ) :
  m_lookAt((const double[]){0.0, TABLELENGTH/2, TABLEHEIGHT}), 
  m_lastSendX((const double[]){0.0, 0.0, 0.0}), 
  m_lastSendV((const double[]){0.0, 0.0, 0.0}) {
  m_side = side;
  m_playerType = playerType;

  m_x = x;
  m_v = v;

  m_status = status;
  m_swing = swing;
  m_afterSwing = afterSwing;
  m_swingType = swingType;
  m_swingSide = swingSide;
  m_swingError = swingError;
  m_target = target;

  m_eye = eye;

  m_pow = pow;
  m_lookAt[1] *= m_side;
  m_spin = spin;
  m_stamina = stamina;
  m_statusMax = statusMax;
  m_lastSendCount = 0;

  m_View = NULL;

}

/**
 * Destructor. 
 * Detach view class. 
 */
Player::~Player() {
  if ( m_View ){
    BaseView::TheView()->RemoveView( m_View );
    delete m_View;
  }
}

/**
 * Copy operator. 
 */
void
Player::operator=(Player& p) {
  m_playerType = p.m_playerType;
  m_side = p.m_side;

  m_x = p.m_x;
  m_v = p.m_v;

  m_status = p.m_status;
  m_swing = p.m_swing;
  m_swingType = p.m_swingType;
  m_swingSide = p.m_swingSide;
  m_afterSwing = p.m_afterSwing;
  m_swingError = p.m_swingError;
  m_target = p.m_target;

  m_eye = p.m_eye;

  m_lookAt = p.m_lookAt;

  m_pow = p.m_pow;
  m_spin = p.m_spin;

  m_stamina = p.m_stamina;

  m_dragX = p.m_dragX;
  m_dragY = p.m_dragY;

  m_View = NULL;
}

/**
 * Creator method of subclasses of Player class. 
 * 
 * @param player player type
 * @param side player side
 * @param type class type of the player (normal, com, training, etc. )
 * @return returns created player object. 
 */
Player*
Player::Create( long player, long side, long type ) {
  switch (type) {
  case 0:	// normal
    switch (player) {
    case 0:
      return new PenAttack(side);
    case 1:
      return new ShakeCut(side);
    case 2:
      return new PenDrive(side);
    }
    break;
  case 1:	// Com
    switch (player) {
    case 0:
      return new ComPenAttack(side);
    case 1:
      return new ComShakeCut(side);
    case 2:
      return new ComPenDrive(side);
    }
    break;
  case 2:	// Training
    switch (player) {
    case 0:
      return new TrainingPenAttack(side);
    case 1:
      return new TrainingPenDrive(side);
    }
    break;
  case 3:	// ComTraining
    switch (player) {
    case 0:
      return new ComTrainingPenAttack(side);
    case 1:
      return new ComTrainingPenDrive(side);
    }
    break;
  }

  printf( _("no player %ld\n"), player );
  exit(1);
}

/**
 * Initializer method. 
 * Create PlayerView object and attach it to this object. 
 * 
 * @return returns true if succeeds. 
 */
bool
Player::Init() {
  m_View = (PlayerView *)View::CreateView( VIEW_PLAYER );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  if ( theRC->gmode != GMODE_2D )
    HitMark::Init();

  return true;
}

/**
 * Reset member variables. 
 * 
 * @param p player object. Member variables of the object is set to this object. 
 * @return returns true if succeeds. 
 */
bool
Player::Reset( Player *p ) {
  *this = *p;

  return true;
}

/**
 * Move this player object. 
 * Move this player and change m_status. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
Player::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
	      long *MouseYHistory, unsigned long *MouseBHistory,
	      int Histptr ) {
  //static double  lastSendX = 0,  lastSendY = 0,  lastSendZ = 0;
  //static double lastSendVX = 0, lastSendVY = 0, lastSendVZ = 0;
  //static long lastSendCount = 0;

// swing
  if ( m_swing > 0 ){
    if ( m_swing > 30 && m_afterSwing > 0 ) {
      m_afterSwing--;
    } else {
      if ( theBall.GetStatus() == 6 || theBall.GetStatus() == 7 ) {
	if ( theBall.GetV()[2] < 0 )
	  m_swing++;
      } else {
	if ( m_swing == 10 ) {
	  if ( theBall.GetStatus() == -1 )
	    m_swing = 0;
	  else if ( ( m_side > 0 && theBall.GetStatus() == 0) ||
		    ( m_side < 0 && theBall.GetStatus() == 2) ) {
	    m_swing++;
	  }
	} else
	  m_swing++;
      }
    }
  }

  // If the ball goes out of sight, look at the ball direction
  vector3d x = m_x + m_eye;
  vector3d tx; tx[0] = 0.0; tx[1] = TABLELENGTH/2*m_side; tx[2] = TABLEHEIGHT;
  vector3d vx1 = tx-x;
  vector3d vxt;
  vector3d vx2;
  double p, q;
  double sinP, cosP;

  vx1.normalize();

  vxt = theBall.GetX()-x;
  if ( theBall.GetStatus() == 6 || theBall.GetStatus() == 7 )
    vxt[2] = TABLEHEIGHT+0.15-x[2];

  vxt.normalize();

  if ( (cosP = vx1*vxt) < cos(3.141592/180.0*15) &&
       fabs(theBall.GetX()[1]) > fabs(x[1]) ) {
    sinP = sqrt(1-cosP*cosP);
    p = cos(3.141592/180.0*15) - sin(3.141592/180.0*15)*cosP/sinP;
    q = sin(3.141592/180.0*15)/sinP;

    vx2 = p*vxt+q*vx1;

    m_lookAt = x+vx2;
  } else {
    vector2d vx12; vx12[0] = vx1[1]; vx12[1] = vx1[2];
    vector2d vxt2; vxt2[0] = vxt[1]; vxt2[1] = vxt[2];
    if ( (cosP = (vx12*vxt2)/(vx12.len()*vxt2.len())) < cos(3.141592/180.0*15) &&
	 theBall.GetX()[2] > x[2] &&
	 (theBall.GetStatus() == 0 || theBall.GetStatus() == 2) ) {
      sinP = sqrt(1-cosP*cosP);
      p = cos(3.141592/180.0*15) - sin(3.141592/180.0*15)*cosP/sinP;
      q = sin(3.141592/180.0*15)/sinP;

      vx2 = p*vxt+q*vx1;

      m_lookAt = x+vx2;
    } else {
      m_lookAt = tx;
    }
  }

// backswing and inpact
  if ( m_swing == 20 ){
    HitBall();

    if ( Control::TheControl()->GetThePlayer() == this &&
	 theRC->gmode != GMODE_2D ) {
      HitMark *hit;

      vector3d hitX = theBall.GetX();
      hitX[1] = m_x[1];

      hit = new HitMark();
      hit->Hit( hitX, theBall.GetV(), GetSwingError() );

      BaseView::TheView()->AddView( hit );
    }
    m_spin[0] = m_spin[1] = 0.0;
  }
  else if ( m_swing == 50 ){
    m_swing = 0;
    m_swingType = SWING_NORMAL;
  }

  // Automatically move towards the ball
  // Only for human. 
  if ( (mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY ||
	mode == MODE_PRACTICE) && KeyHistory &&
       theRC->gameLevel != LEVEL_TSUBORISH ) {
    if ( m_swing > 10 && m_swing < 20 ) {
      Ball *tmpBall;

      tmpBall = new Ball(&theBall);

      for ( int i = 0 ; i < 20-m_swing ; i++ )
	tmpBall->Move();

      if ( ((tmpBall->GetStatus() == 3 && m_side == 1) ||
	    (tmpBall->GetStatus() == 1 && m_side == -1)) ) {
	double hitpointX = m_swingSide ? m_x[0]+0.3*m_side : m_x[0]-0.3*m_side;
	double xdiff = tmpBall->GetX()[0] - (hitpointX+m_v[0]*(20-m_swing)*TICK);
	double ydiff = tmpBall->GetX()[1] - (   m_x[1]+m_v[1]*(20-m_swing)*TICK);

	double vxdiff, vydiff;
	vxdiff = xdiff/TICK/(20-m_swing);

	if ( vxdiff > 2.0 )
	  vxdiff = 2.0;
	else if ( vxdiff < -2.0 )
	  vxdiff = -2.0;

	vxdiff /= theRC->gameLevel+1;

	m_v[0] += vxdiff;

	if ( fabs(ydiff) > 0.3 ) {
	  vydiff = ydiff/TICK/(20-m_swing);
	  if ( vydiff > 2.0 )
	    vydiff = 2.0;
	  else if ( vydiff < -2.0 )
	    vydiff = -2.0;

	  vydiff /= theRC->gameLevel+1;

	  m_v[1] += vydiff;
	}
      }
      delete tmpBall;
    }
  }

// move player
  vector3d xNext = m_x + m_v*TICK;

  if ( xNext[0] < -AREAXSIZE/2 ){
    m_x[0] = -AREAXSIZE/2;
    m_v[0] = 0.0;
  } else if ( xNext[0] > AREAXSIZE/2 ){
    m_x[0] = AREAXSIZE/2;
    m_v[0] = 0.0;
  }
#ifndef DEBUG_NOLIMITMOVE
  else if ( m_x[0] <= -TABLEWIDTH/2 && xNext[0] >= -TABLEWIDTH/2 &&
	    m_x[1] > -TABLELENGTH/2+0.5 && m_x[1] < TABLELENGTH/2-0.5 ){
    m_x[0] = -TABLEWIDTH/2;
    m_v[0] = 0.0;
  }
  else if ( m_x[0] >= TABLEWIDTH/2 && xNext[0] <= TABLEWIDTH/2 &&
	    m_x[1] > -TABLELENGTH/2+0.5 && m_x[1] < TABLELENGTH/2-0.5 ){
    m_x[0] = TABLEWIDTH/2;
    m_v[0] = 0.0;
  }
#endif
  else
    m_x[0] = xNext[0];

  if ( xNext[1] < -AREAYSIZE/2 ) {
    m_x[1] = -AREAYSIZE/2;
    m_v[1] = 0.0;
  } else if ( xNext[1] > AREAYSIZE/2 ) {
    m_x[1] = AREAYSIZE/2;
    m_v[1] = 0.0;
  }
#ifndef DEBUG_NOLIMITMOVE
  else if ( m_x[1] <= -TABLELENGTH/2+0.5 && xNext[1] >= -TABLELENGTH/2+0.5
	    && m_x[0] > -TABLEWIDTH/2 && m_x[0] < TABLEWIDTH/2 ) {
    m_x[1] = -TABLELENGTH/2+0.5;
    m_v[1] = 0.0;
  }
  else if ( m_x[1] >= TABLELENGTH/2-0.5 && xNext[1] <= TABLELENGTH/2-0.5
	      && m_x[0] > -TABLEWIDTH/2 && m_x[0] < TABLEWIDTH/2 ) {
    m_x[1] = TABLELENGTH/2-0.5;
    m_v[1] = 0.0;
  }
#endif
  else
    m_x[1] = xNext[1];

// Go back to the endline before serve
  if ( Control::TheControl()->IsPlaying() && theBall.GetStatus() == 8 &&
       ((PlayGame *)Control::TheControl())->GetService() == GetSide() ) {
#ifndef DEBUG_NOLIMITMOVE
    if ( m_side > 0 && m_x[1] > -TABLELENGTH/2 )
      m_x[1] = -TABLELENGTH/2;
    else if ( m_side < 0 && m_x[1] < TABLELENGTH/2 )
      m_x[1] = TABLELENGTH/2;
#endif
  }

  // Auto backswing
  if ( m_swing == 0 ) {
    Ball *tmpBall;

    tmpBall = new Ball(&theBall);

    for ( int i = 0 ; i < 30 ; i++ ) {	/* A bit earlier */
      tmpBall->Move();

      if ( ((tmpBall->GetStatus() == 3 && m_side == 1) ||
	    (tmpBall->GetStatus() == 1 && m_side == -1)) &&
	   (m_x[1]-tmpBall->GetX()[1])*m_side < 0.3 &&
	   (m_x[1]-tmpBall->GetX()[1])*m_side > -0.05 ){
	StartSwing(3);
	break;
      }
    }
    delete tmpBall;
  }

// calc status
  if ( m_v.len() > 2.0 )
    AddStatus( -1 );

  if ( m_swing > 10 )
    AddStatus( -1 );

  if ( theBall.GetStatus() == 8 || theBall.GetStatus() == -1 )
    AddStatus( 200 );

  if ( Control::TheControl()->IsPlaying() &&
	 !((PlayGame *)Control::TheControl())->IsPause() )
    KeyCheck( KeyHistory, MouseXHistory, MouseYHistory, MouseBHistory,Histptr );

  if ( Control::TheControl()->GetThePlayer() == this &&
       mode == MODE_MULTIPLAY ) {
    m_lastSendCount++;

    m_lastSendX += m_lastSendV*TICK;

    if ( m_lastSendCount >= 100 ||
	 (m_lastSendV-m_v).len() >= 0.25 ) {
      Event::TheEvent()->SendPlayer( this );
    }

    // theBall goes out of hitting area. 
    if ( ((theBall.GetStatus() == 1 && m_side == -1) ||
	  (theBall.GetStatus() == 3 && m_side == 1 ) ) &&
	 m_swing <= 10 &&
	 (m_x[1]-theBall.GetX()[1])*m_side > 0.3 &&
	 (m_x[1]+m_v[1]*TICK-(theBall.GetX()[1]+theBall.GetV()[1]*TICK))*m_side > 0.3 )
      Event::TheEvent()->SendBall();
  }

  return true;
}

/**
 * Check keyboard input, mouse move and click to move player. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if succeeds. 
 */
bool
Player::KeyCheck( SDL_keysym *KeyHistory, long *MouseXHistory,
		  long *MouseYHistory, unsigned long *MouseBHistory,
		  int Histptr ) {
  long mouse, lastmouse;

  const char keytable[][5] = {
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'1', '1', '1', '[', '\0'},
  {'2', '2', '2', '7', '\0'},
  {'3', '3', '3', '5', '\0'},
  {'4', '4', '4', '3', '\0'},
  {'5', '5', '5', '1', '\0'},
  {'6', '6', '6', '9', '\0'},
  {'7', '7', '7', '0', '\0'},
  {'8', '8', '8', '2', '\0'},
  {'9', '9', '9', '6', '\0'},
  {'0', '0', '0', '8', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'q', 'q', 'a', '/', '\0'},
  {'w', 'w', 'z', ',', '\0'},
  {'e', 'e', 'e', '.', '\0'},
  {'r', 'r', 'r', 'p', '\0'},
  {'t', 't', 't', 'y', '\0'},
  {'y', 'z', 'y', 'f', '\0'},
  {'u', 'u', 'u', 'g', '\0'},
  {'i', 'i', 'i', 'c', '\0'},
  {'o', 'o', 'o', 'r', '\0'},
  {'p', 'p', 'p', 'l', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'a', 'a', 'q', 'a', '\0'},
  {'s', 's', 's', 'o', '\0'},
  {'d', 'd', 'd', 'e', '\0'},
  {'f', 'f', 'f', 'u', '\0'},
  {'g', 'g', 'g', 'i', '\0'},
  {'h', 'h', 'h', 'd', '\0'},
  {'j', 'j', 'j', 'h', '\0'},
  {'k', 'k', 'k', 't', '\0'},
  {'l', 'l', 'l', 'n', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},	// {          'm', 's', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},
  {'z', 'y', 'w', ';', '\0'},
  {'x', 'x', 'x', 'q', '\0'},
  {'c', 'c', 'c', 'j', '\0'},
  {'v', 'v', 'v', 'k', '\0'},
  {'b', 'b', 'b', 'x', '\0'},
  {'n', 'n', 'n', 'b', '\0'},
  {'m', 'm'     , 'm', '\0', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},	// {               'w', '\0'},
  {'\0', '\0', '\0', '\0', '\0'},	// {               'v', '\0'},
  {'\0', '\0', '\0', '\0', '\0'}	// {               'z', '\0'}
  };

// COM
  if ( !KeyHistory || !MouseXHistory || !MouseYHistory || !MouseBHistory )
    return true;

// key input
  // Check keyboard type and modify keycode. 
  int code = -1;

  if ( KeyHistory[Histptr].scancode < 54 ) {
    int i = 0;
    while (keytable[KeyHistory[Histptr].scancode][i]) {
      if ( keytable[KeyHistory[Histptr].scancode][i]
	   == KeyHistory[Histptr].unicode ) {
	code = keytable[KeyHistory[Histptr].scancode][0];
	break;
      }
      i++;
    }
  }

  if ( KeyHistory[Histptr].scancode >= 8 && 
       KeyHistory[Histptr].scancode < 62 && 
       code < 0 ) {	// for X11
    int i = 0;
    while (keytable[KeyHistory[Histptr].scancode-8][i]) {
      if ( keytable[KeyHistory[Histptr].scancode-8][i]
	   == KeyHistory[Histptr].unicode ) {
	code = keytable[KeyHistory[Histptr].scancode-8][0];
	break;
      }
      i++;
    }
  }

  if ( code < 0 )
    code = KeyHistory[Histptr].unicode;


  switch ( code ) {
  case '1':  case 'q':  case 'a':  case 'z':
  case '2':  case 'w':  case 's':  case 'x':
  case '3':
    m_target[0] = -TABLEWIDTH/2*0.9*GetSide();
    break;
  case 'e':
    m_target[0] = -TABLEWIDTH/2*0.75*GetSide();
    break;
  case 'd':
    m_target[0] = -TABLEWIDTH/2*0.6*GetSide();
    break;
  case '4':  case 'c':
    m_target[0] = -TABLEWIDTH/2*0.45*GetSide();
    break;
  case 'r':
    m_target[0] = -TABLEWIDTH/2*0.3*GetSide();
    break;
  case 'f':
    m_target[0] = -TABLEWIDTH/2*0.15*GetSide();
    break;
  case '5':  case 'v':
    m_target[0] = 0;
    break;
  case 't':
    m_target[0] = TABLEWIDTH/2*0.15*GetSide();
    break;
  case 'g':
    m_target[0] = TABLEWIDTH/2*0.3*GetSide();
    break;
  case '6':  case 'b':
    m_target[0] = TABLEWIDTH/2*0.45*GetSide();
    break;
  case 'y':
    m_target[0] = TABLEWIDTH/2*0.6*GetSide();
    break;
  case 'h':
    m_target[0] = TABLEWIDTH/2*0.75*GetSide();
    break;
  case '7':  case 'n':  case 'u':  case 'j':
  case '8':  case 'm':  case 'i':  case 'k':
  case '9':  case ',':  case 'o':  case 'l':
  case '0':  case '.':  case 'p':  case ';':
    m_target[0] = TABLEWIDTH/2*0.9*GetSide();
    break;
  }

  switch ( code ){
  case '1':  case '2':  case '3':  case '4':  case '5':  case '6':
  case '7':  case '8':  case '9':  case '0':  case '-':  case '^':
    m_target[1] = TABLELENGTH/12*5*GetSide();
    break;
  case 'q':  case 'w':  case 'e':  case 'r':  case 't':  case 'y':
  case 'u':  case 'i':  case 'o':  case 'p':  case '@':  case '[':
    m_target[1] = TABLELENGTH/12*4*GetSide();
    break;
  case 'a':  case 's':  case 'd':  case 'f':  case 'g':  case 'h':
  case 'j':  case 'k':  case 'l':  case ';':  case ':':  case ']':
    m_target[1] = TABLELENGTH/12*3*GetSide();
    break;
  case 'z':  case 'x': case 'c':  case 'v':  case 'b':  case 'n':
  case 'm':  case ',':  case '.':  case '/':  case '\\':
    m_target[1] = TABLELENGTH/12*2*GetSide();
    break;
  }


  if ( (Histptr == 0 &&
	KeyHistory[Histptr].unicode != KeyHistory[MAX_HISTORY-1].unicode) ||
       (Histptr != 0 &&
	KeyHistory[Histptr].unicode != KeyHistory[Histptr-1].unicode) ) {
    switch ( KeyHistory[Histptr].unicode ) {
    case 'H':
      m_eye[0] -= 0.05;
      break;
    case 'J':
      m_eye[2] -= 0.05;
      break;
    case 'K':
      m_eye[2] += 0.05;
      break;
    case 'L':
      m_eye[0] += 0.05;
      break;
    case '<':
      m_eye[1] -= 0.05;
      break;
    case '>':
      m_eye[1] += 0.05;
      break;

    case 'A':
      m_lookAt[0] -= 0.05;
      break;
    case 'S':
      m_lookAt[2] -= 0.05;
      break;
    case 'D':
      m_lookAt[2] += 0.05;
      break;
    case 'F':
      m_lookAt[0] += 0.05;
      break;
    case 'C':
      m_lookAt[1] -= 0.05;
      break;
    case 'V':
      m_lookAt[1] += 0.05;
      break;

    }
  }

  // Sorry in Japanese...
  // スイング中は速度が変わらないようにする. こうすることで
  // MultiPlay 時により同期をとりやすくなる. 
  // その理由は, スイングを開始した時点で Player のインパクト
  // までの行動が決定されるため, スイング開始時に情報交換すれば
  // 同期がとれるようになるためである. これまでは, インパクト時に
  // 情報を交換していたので, 0.1秒早く同期がとれることになる. 

  // ちなみに, スイング後のマウスのドラッグによってボールの回転を
  // 制御する場合, この方法は使えないかも知れない. 

  if ( !Control::TheControl()->IsPlaying() ||
       ((PlayGame *)Control::TheControl())->IsPause() )
    return true;

  if ( m_swing > 10 && m_swing <= 20 ) {
    long hptr = Histptr-(m_swing-11);
    if ( hptr < 0 )
      hptr += MAX_HISTORY;

    m_dragX = MouseXHistory[Histptr]-MouseXHistory[hptr];
    m_dragY = MouseYHistory[Histptr]-MouseYHistory[hptr];
  } else {
    m_v[0] = (MouseXHistory[Histptr] - BaseView::GetWinWidth()/2) /
      (BaseView::GetWinWidth()/40)*GetSide();
    m_v[1] = -(MouseYHistory[Histptr] - BaseView::GetWinHeight()/2) /
      (BaseView::GetWinHeight()/40)*GetSide();
    m_v /= 4;
  }

  mouse = MouseBHistory[Histptr];
  if ( Histptr-1 < 0 )
    lastmouse = MouseBHistory[MAX_HISTORY-1];
  else
    lastmouse = MouseBHistory[Histptr-1];

  if ( (mouse & BUTTON_RIGHT) && !(lastmouse & BUTTON_RIGHT) ){
    if ( theBall.GetStatus() == 8 &&
	 ((PlayGame *)Control::TheControl())->GetService() == GetSide() ) {
      theBall.Toss( this, 3 );
      StartServe(3);
    } else {
      AddStatus( (m_swing-10)*10 );
      Swing(3);
    }
  }
  else if ( (mouse & BUTTON_MIDDLE) && !(lastmouse & BUTTON_MIDDLE) ){
    if ( theBall.GetStatus() == 8 &&
	 ((PlayGame *)Control::TheControl())->GetService() == GetSide() ) {
      theBall.Toss( this, 2 );
      StartServe(2);
    } else {
      AddStatus( (m_swing-10)*10 );
      Swing(2);
    }
  }
  else if ( (mouse & BUTTON_LEFT) && !(lastmouse & BUTTON_LEFT) ){
    if ( theBall.GetStatus() == 8 &&
	 ((PlayGame *)Control::TheControl())->GetService() == GetSide() ) {
      theBall.Toss( this, 1 );
      StartServe(1);
    } else {
      AddStatus( (m_swing-10)*10 );
      Swing(1);
    }
  }

  return true;
}

/**
 * Change status value. 
 * diff is added to m_status. 
 * 
 * @param diff this value is added to m_status. 
 * @return returns true if succeeds. 
 */
bool
Player::AddStatus( long diff ) {
  if ( diff == 200 ) {	// Not good...
    m_statusMax = 200;
    m_status = 200;
  } else {
    m_status += diff;

    if ( m_status > m_statusMax )
      m_status = m_statusMax;

    if ( m_status < 1 ) {
      m_stamina += (m_status-1) / 10.0;
      m_status = 1;
    }

    if ( diff < -3 ) {	// Not good... When status decreased without moving...
      if ( Control::TheControl()->GetThePlayer() == this ) {
	switch (theRC->gameLevel) {
	case LEVEL_EASY:
	  m_statusMax += diff/4;
	  break;
	case LEVEL_NORMAL:
	  m_statusMax += diff/3;
	  break;
	case LEVEL_HARD:
	  m_statusMax += diff/2;
	  break;
	case LEVEL_TSUBORISH:
	  m_statusMax += diff/2;
	  break;
	}
      } else {
	m_statusMax += diff/4;	/* 打球位置でのペナルティを相殺 */
      }
    }
  }

  return true;
}

/**
 * Check whether this player swings forehand or backhand. 
 * 
 * @return if this player swings forhand, returns true. Otherwise returns false. 
 */
bool
Player::ForeOrBack() {
  return GetSwingSide();
}

/**
 * Referring the relative location of player and the ball, this method checks the error level of hitting (perfect, good, miss, etc. )
 * 
 * @return returns true if succeeds. 
 */
bool
Player::SwingError() {
  double diff;

  if ( (m_x[1]-theBall.GetX()[1])*m_side < 0 )
    diff = fabs(m_x[1]-theBall.GetX()[1])/2;
  else
    diff = fabs(m_x[1]-theBall.GetX()[1]);

  if ( diff < 0.03 )
    m_swingError = SWING_PERFECT;
  else if ( diff < 0.1 )
    m_swingError = SWING_GREAT;
  else if ( diff < 0.2 )
    m_swingError = SWING_GOOD;
  else
    m_swingError = SWING_BOO;

  return true;
}

/**
 * Set location and velocity of the player. 
 * 
 * @param x location of the player
 * @param v velocity of the player
 * @return returns true if succeeds. 
 */
bool
Player::Warp( const vector3d &x, const vector3d &v ) {
  m_x = x;
  m_v = v;

  return true;
}

/**
 * Set location and velocity of the player. 
 * 
 * @param buf stream of location/velocity data. 
 * @return returns true if succeeds. 
 */
bool
Player::Warp( char *buf ) {
  char *b = buf;
  for ( int i = 0 ; i < 3 ; i++ )
    b = ReadDouble( b, m_x[i] );

  for ( int i = 0 ; i < 3 ; i++ )
    b = ReadDouble( b, m_v[i] );

  return true;
}

/**
 * Set spin and swing of the player. 
 * 
 * @param pow power
 * @param spin spin
 * @param swingType swing type
 * @param swing swing status
 * @return returns true if succeeds. 
 */
bool
Player::ExternalSwing( long pow, const vector2d &spin,
		       long swingType, long swing ) {
  m_swing = swing;
  m_pow = pow;
  m_spin = spin;
  m_swingType =swingType;

  return true;
}

/**
 * Set spin and swing of the player. 
 * 
 * @param buf stream of power/spin/swingType/swingSide/swing data. 
 * @return returns true if succeeds. 
 */
bool
Player::ExternalSwing( char *buf ) {
  char *b = buf;
  long swingSide;
  b = ReadLong( b, m_pow );
  b = ReadDouble( b, m_spin[0] );
  b = ReadDouble( b, m_spin[1] );
  b = ReadLong( b, m_swingType );
  b = ReadLong( b, swingSide );
  b = ReadLong( b, m_swing );

  m_swingSide = (bool)(swingSide != 0);

  return true;
}

/**
 * Send swing information to the opponent. 
 * 
 * @param buf swing power/spin/type/side/status are set to this buffer. 
 * @return returns pointer to buf. 
 */
char *
Player::SendSwing( char *buf ) {
  long l;
  double d;

  l = SwapLong(m_pow);
  memcpy( buf, (char *)&l, 4 );
  d = SwapDbl(m_spin[0]);
  memcpy( &(buf[4]), (char *)&d, 8 );
  d = SwapDbl(m_spin[1]);
  memcpy( &(buf[12]), (char *)&d, 8 );
  l = SwapLong(m_swingType);
  memcpy( &(buf[20]), (char *)&l, 4 );
  l = SwapLong((long)m_swingSide);
  memcpy( &(buf[24]), (char *)&l, 4 );
  l = SwapLong(m_swing);
  memcpy( &(buf[28]), (char *)&l, 4 );

#ifdef LOGGING
  Logging::GetLogging()->LogSendPSMessage( this );
#endif

  return buf;
}

/**
 * Send location/velocity of the player to the opponent. 
 * 
 * @param buf location/velocity are set to this buffer. 
 * @return returns pointer to buf. 
 */
char *
Player::SendLocation( char *buf ) {
  double d;
  int c = 0;

  for ( int i = 0 ; i < 3 ; i++ ) {
    d = SwapDbl(m_x[i]);
    memcpy( &(buf[c]), (char *)&d, 8 );
    c += 8;
  }

  for ( int i = 0 ; i < 3 ; i++ ) {
    d = SwapDbl(m_v[i]);
    memcpy( &(buf[c]), (char *)&d, 8 );
    c += 8;
  }

  UpdateLastSend();

#ifdef LOGGING
  Logging::GetLogging()->LogSendPVMessage( this );
#endif

  return buf;
}

/**
 * Send all player information to the opponent. 
 * 
 * @param sd socket descriptor. 
 * @return returns true if succeeds. 
 */
bool
Player::SendAll( int sd ) {
  SendLong( sd, m_playerType );
  SendLong( sd, m_side );

  for ( int i = 0 ; i < 3 ; i++ ) {
    SendDouble( sd, m_x[i] );
  }

  for ( int i = 0 ; i < 3 ; i++ ) {
    SendDouble( sd, m_v[i] );
  }

  SendLong( sd, m_status );
  SendLong( sd, m_swing );
  SendLong( sd, m_swingType );
  SendLong( sd, (long)m_swingSide );
  SendLong( sd, m_afterSwing );
  SendLong( sd, m_swingError );

  for ( int i = 0 ; i < 2 ; i++ ) {
    SendDouble( sd, m_target[i] );
  }

  for ( int i = 0 ; i < 3 ; i++ ) {
    SendDouble( sd, m_eye[i] );
  }

  SendLong( sd, m_pow );

  for ( int i = 0 ; i < 2 ; i++ ) {
    SendDouble( sd, m_spin[i] );
  }

  SendDouble( sd, m_stamina );

  SendLong( sd, m_statusMax );

  return true;
}

/**
 * Modify location of the target. 
 * This method must be overridden. 
 * 
 * @param target original target and modified target. [in, out]
 * @return returns true if succeeds. 
 */
bool
Player::GetModifiedTarget( vector2d &target ) {
  return false;
}

/**
 * Calculate the level of ball to be hit. 
 * This method must be overridden. 
 * 
 * @param ball the ball object to be hit. 
 * @param diff difference from ideal hit. [out]
 * @param level level of the hit. [out]
 * @param maxVy maximum ball speed of y-coodinate [out]
 */
void
Player::CalcLevel( Ball *ball, double &diff, double &level, double &maxVy ) {
}

/**
 * Start swing (backswing is already done). 
 * This must be overridden. 
 * 
 * @param spin spin level. Currently this parameter is used for deciding forehand/backhand. 
 * @return returns true if succeeds. 
 */
bool
Player::Swing( long spin ) {
  return false;
}

/**
 * Start swing (backswing). 
 * This method is called when the player starts backswing automatically. 
 * This method checks whether this player can start backswing, and decide
 * swing type. 
 * 
 * @param spin spin level. Currently this parameter is used on serve only. 
 * @return returns true if succeeds. 
 */
bool
Player::StartSwing( long spin ) {
  Ball *tmpBall;

  if ( m_swing > 10 )
    return false;

  if ( m_swing == 0 ) {
    m_swing = 1;
    m_pow = 0;

    // Decide SwingType by the hit point and spin, etc. 
    // Calc the ball location of 0.2 second later
    tmpBall = new Ball(&theBall);

    for ( int i = 0 ; i < 20 ; i++ )
      tmpBall->Move();

    if ( (m_x[0]-tmpBall->GetX()[0])*m_side > 0 )
      m_swingSide = false;
    else
      m_swingSide = true;

    SwingType( tmpBall, spin );

    delete tmpBall;
  }

  return true;
}

/**
 * Start serve (backswing). 
 * 
 * @param spin spin level. 
 * @return returns true if succeeds. 
 */
bool
Player::StartServe( long spin ) {
  if ( m_swing > 10 )
    return false;

  if ( m_swing == 0 ){
    m_swing = 1;
    m_pow = 0;

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
#if 0	// sidespin
#else
	m_spin[0] = 1.0;
#endif
      m_spin[1] = -0.6;
      m_swingType = SWING_POKE;
      break;
    }

    m_swingSide = true;

    if ( Control::TheControl()->GetThePlayer() == this &&
	 mode == MODE_MULTIPLAY )
      ::SendSwing( this );
  }

  return true;
}

/**
 * Hit the ball with racket. 
 * This method must be overridden. 
 * 
 * @return returns true if succeeds. 
 */
bool
Player::HitBall() {
  return false;
}

/**
 * Update last send information. 
 * This method is called when some player information is sent to the 
 * opponent machine. This method reset m_lastSendCount and update
 * m_lastSendX and m_lastSendV. 
 */
void
Player::UpdateLastSend() {
  m_lastSendCount = 0;

  m_lastSendX = m_x;
  m_lastSendV = m_v;
}

/**
 * Add error to volocity of the ball. 
 * Referring the relative location of player and the ball, this method
 * add error to the velocity of the ball. 
 * 
 * @param v velocity of the ball [in, out]
 */
void
Player::AddError( vector3d &v ) {
  double vl;
  vector3d n1, n2;
  double radDiff, radRand;

#if 1
  double xDiff = (fabs(m_x[0]-theBall.GetX()[0])-0.3)/0.3;
  double yDiff = (m_x[1]-theBall.GetX()[1])/0.3;
  radDiff = hypot( xDiff*(1+fabs(theBall.GetSpin()[0])), 
		   yDiff*(1+fabs(theBall.GetSpin()[1])) );
  //radDiff = sqrt( radDiff );
  radDiff *= (double)(200-m_status)/200*3.141592/12;
#else
  radDiff = (double)(200-m_status)/200*3.141592/12;
#endif

  vl = v.len();
  n1[0] =  v[1]/hypot(v[0], v[1]) * vl*tan(radDiff);
  n1[1] = -v[0]/hypot(v[0], v[1]) * vl*tan(radDiff);
  n1[2] = 0;
  n2[0] =             v[0]*v[2]/(vl*hypot(v[0], v[1])) * vl*tan(radDiff);
  n2[1] =             v[1]*v[2]/(vl*hypot(v[0], v[1])) * vl*tan(radDiff);
  n2[2] = (v[0]*v[0]+v[1]*v[1])/(vl*hypot(v[0], v[1])) * vl*tan(radDiff);

  // Hit the ball too fast --- net miss
  // Hit the ball too slow --- over miss
  if ( (m_x[1]-theBall.GetX()[1])*m_side < 0 )
    radRand = (RAND(180)+180)*3.141592/180.0;
  else
    radRand = RAND(180)*3.141592/180.0;

  v += n1*cos(radRand) + n2*sin(radRand);
}

/**
 * Referring the location of the target, calculate the border of the status. 
 * If status point is less than the border, player will miss. 
 * 
 * @return returns border. 
 */
long
Player::StatusBorder() {
  double nearEdge = TABLEWIDTH/2-fabs(m_target[0]);
  if ( TABLELENGTH/4-fabs(fabs(m_target[1])-TABLELENGTH/4) < nearEdge )
    nearEdge = TABLELENGTH/4-fabs(fabs(m_target[1])-TABLELENGTH/4);

  return (long)(50+(TABLELENGTH/4-nearEdge)*40);

}

/**
 * Returns dominant hand. 
 * 
 * @return returns true if the dominant hand is right. Otherwise returns false. 
 */
bool
Player::GetDominantHand() {
  switch (m_playerType) {
  case PLAYER_PROTO:
  case PLAYER_PENATTACK:
  case PLAYER_SHAKECUT:
  case PLAYER_PENDRIVE:
    return true;
  }
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
Player::SwingType( Ball *ball, long spin ) {
  return false;
}
