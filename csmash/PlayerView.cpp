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
#include "PlayerView.h"
#include "Player.h"
#include "parts.h"
#include "Ball.h"

extern bool isPolygon;
extern bool isLighting;
extern bool isWireFrame;
extern bool isSimple;

extern SDL_mutex *loadMutex;

partsmotion *PlayerView::motion_Fnormal = NULL;
partsmotion *PlayerView::motion_Bnormal = NULL;
partsmotion *PlayerView::motion_Fdrive = NULL;
partsmotion *PlayerView::motion_Fcut = NULL;
partsmotion *PlayerView::motion_Bcut = NULL;
partsmotion *PlayerView::motion_Fpeck = NULL;
partsmotion *PlayerView::motion_Bpeck = NULL;
partsmotion *PlayerView::motion_Fsmash = NULL;

extern Player *thePlayer;
extern Player *comPlayer;
extern Ball   theBall;

extern long mode;

PlayerView::PlayerView() {
  m_player = NULL;

  m_Fnormal = m_Bnormal = NULL;
  m_Fdrive  = m_Bdrive  = NULL;
  m_Fcut    = m_Bcut    = NULL;
  m_Fpeck   = m_Bpeck   = NULL;
  m_Fsmash  = m_Bsmash  = NULL;
}

PlayerView::~PlayerView() {
}

void *
PlayerView::LoadData(void *dum) {
  motion_Fnormal = new partsmotion("Parts/Fnormal/Fnormal");
  motion_Bnormal = new partsmotion("Parts/Bnormal/Bnormal");
  motion_Fdrive = new partsmotion("Parts/Fdrive/Fdrive");
  motion_Fcut = new partsmotion("Parts/Fcut/Fcut");
  motion_Bcut = new partsmotion("Parts/Bcut/Bcut");
  motion_Fpeck = new partsmotion("Parts/Fpeck/Fpeck");
  motion_Bpeck = new partsmotion("Parts/Bpeck/Bpeck");
  motion_Fsmash = new partsmotion("Parts/Fsmash/Fsmash");

  return NULL;
}

bool
PlayerView::Init( Player *player ) {
  m_player = player;

  SDL_mutexP( loadMutex );

  m_Fnormal = motion_Fnormal;
  m_Bnormal = motion_Bnormal;
  m_Fdrive = motion_Fdrive;
  m_Fcut = motion_Fcut;
  m_Bcut = motion_Bcut;
  m_Fpeck = motion_Fpeck;
  m_Bpeck = motion_Bpeck;
  m_Fsmash = motion_Fsmash;

  SDL_mutexV( loadMutex );

  return true;
}

bool
PlayerView::Redraw() {
  static GLfloat mat_green[] = { 0.4F, 0.4F, 0.4F, 1.0F };

  if ( m_player == comPlayer ) {
    if ( isLighting ) {
      glColor4f( 0.4F, 0.4F, 0.4F, 0.0F );
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_green);
    } else {
      glColor4f( 0.3F, 0.3F, 0.3F, 1.0F );
    }

    return SubRedraw();
  }

  return true;
}

bool
PlayerView::RedrawAlpha() {
  static GLfloat mat_black[] = { 0.0F, 0.0F, 0.0F, 1.0F };

  if ( m_player == thePlayer ) {
    if ( isLighting ) {
      if ( isWireFrame ) {
	glColor4f( 1.0F, 1.0F, 1.0F, 1.0F );
      } else {
	glColor4f( 0.05F, 0.05F, 0.05F, 1.0F );
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_black);
      }
    } else {
      glColor4f( 0.1F, 0.1F, 0.1F, 1.0F );
    }

    return SubRedraw();
  }

  return true;
}

bool
PlayerView::SubRedraw() {
  double x, y, deg;
  double degx, degy, degz;
  double ydiff;

  glPushMatrix();
  if ( isPolygon )
    glTranslatef( m_player->GetX()-0.3F*m_player->GetSide(),
		  m_player->GetY(), 0 );
  else
    glTranslatef( m_player->GetX()-0.3F*m_player->GetSide(),
		  m_player->GetY(), m_player->GetZ() );

  glRotatef( -atan2( m_player->GetTargetX()-m_player->GetX(),
		     m_player->GetTargetY()-m_player->GetY() )*180/3.141592F, 
	     0.0F, 0.0F, 1.0F );

  if ( isPolygon ) {
    int swing;
    partsmotion *motion;

    swing = m_player->GetSwing();
    switch( m_player->GetSwingType() ) {
    case SWING_NORMAL:
      if ( m_player->ForeOrBack() )
	motion = m_Fnormal;
      else
	motion = m_Bnormal;
      break;
    case SWING_POKE:
      if ( m_player->ForeOrBack() )
	motion = m_Fpeck ? m_Fpeck : m_Fnormal;
      else
	motion = m_Bpeck ? m_Bpeck : m_Bnormal;
      break;
    case SWING_SMASH:
      if ( m_player->ForeOrBack() )
	motion = m_Fsmash ? m_Fsmash : m_Fnormal;
      else
	motion = m_Bsmash ? m_Bsmash : m_Bnormal;
      break;
    case SWING_DRIVE:
      if ( m_player->ForeOrBack() )
	motion = m_Fdrive ? m_Fdrive : m_Fnormal;
      else
	motion = m_Bdrive ? m_Bdrive : m_Bnormal;
      break;
    case SWING_CUT:
      if ( m_player->ForeOrBack() )
	motion = m_Fcut ? m_Fcut : m_Fnormal;
      else
	motion = m_Bcut ? m_Bcut : m_Bnormal;
      break;
    default:
      return false;
    }

//    glEnable(GL_CULL_FACE);
    if ( isSimple )
      motion->renderWire(swing);
    else {
      if (m_player == comPlayer) {
	motion->render(swing);
      }
      if (m_player == thePlayer) {
	if ( isWireFrame )
	  motion->renderWire(swing);
	else {
	  glDisable(GL_DEPTH_TEST);
	  glDepthMask(0);
	  motion->render(swing);
	  glDepthMask(1);
	  glEnable(GL_DEPTH_TEST);
	}
      }
    }

//    glDisable(GL_CULL_FACE);

  } else {
    m_player->GetShoulder( x, y, deg );

    glTranslatef( -0.15F, 0.0F, 0.0F );

    // Head
    glPushMatrix();
    glTranslatef( x, 0.0F, 0.0F );
    if ( m_player->GetSide() < 0 ) {
      //glutWireSphere( 0.15F, 8, 8 );
    }
    glPopMatrix();

    // Body
    glRotatef( deg, 0.0F, 0.0F, 1.0F );

    // On the table
    if ( m_player->GetY() > -TABLELENGTH/2 &&
	 m_player->GetY() <= -TABLELENGTH/2+0.5 &&
	 m_player->GetX() > -TABLEWIDTH/2 &&
	 m_player->GetX() < TABLEWIDTH/2 )
      ydiff = -m_player->GetY() - TABLELENGTH/2;
    else if ( m_player->GetY() < TABLELENGTH/2 &&
	      m_player->GetY() >= TABLELENGTH/2-0.5 &&
	      m_player->GetX() > -TABLEWIDTH/2 &&
	      m_player->GetX() < TABLEWIDTH/2 )
      ydiff = m_player->GetY() - TABLELENGTH/2;
    else
      ydiff = 0.0;

    glBegin(GL_LINE_LOOP);
      glVertex3f( 0.15F, ydiff, -0.15F-0.3F+y );
      glVertex3f( -0.15F, ydiff, -0.15F-0.3F+y );
      glVertex3f( -0.15F+x, 0.0F, 0.15F-0.3F+y );
      glVertex3f( 0.15F+x, 0.0F, 0.15F-0.3F+y );
    glEnd();

    // Left arm
    glBegin(GL_LINE_STRIP);
      glVertex3f( -0.15F+x, 0.0F, 0.15F-0.3F+y );
      glVertex3f( -0.15F-0.10F+x, 0.0F, 0.15F-0.3F-0.25F+y );
      glVertex3f( -0.15F-0.10F+x, 0.30F, 0.15F-0.3F-0.25F+y );
    glEnd();

    // Right arm
    // origin = right shoulder
    glTranslatef( 0.15F+x, 0.0F, -0.15F+y );

    m_player->GetElbow( degx, degy );
    glRotatef( degx, 1.0F, 0.0F, 0.0F );
    glRotatef( degy, 0.0F, 1.0F, 0.0F );

    glBegin(GL_LINE_STRIP);
      glVertex3f( 0.0F, 0.0F, 0.0F );
      glVertex3f( 0.0F, 0.0F, -UPPERARM );
    glEnd();

    glTranslatef( 0.0F, 0.0F, -UPPERARM );

    m_player->GetHand( degx, degy, degz );
    glRotatef( degz, 0.0F, 0.0F, 1.0F );
    glRotatef( degx, 1.0F, 0.0F, 0.0F );
    glRotatef( degy, 0.0F, 1.0F, 0.0F );

    glBegin(GL_LINE_STRIP);
      glVertex3f( 0.0F, 0.0F, 0.0F );
      glVertex3f( 0.0F, FOREARM, 0.0F );
    glEnd();

    // Racket
    glBegin(GL_LINE_LOOP);
      glVertex3f( 0.0F, FOREARM, -0.05F );
      glVertex3f( 0.0F, FOREARM, 0.05F );
      glVertex3f( 0.0F, FOREARM+0.2F, 0.05F );
      glVertex3f( 0.0F, FOREARM+0.2F, -0.05F );
    glEnd();
  }

  glPopMatrix();

  // Target
  glColor4f( 0.5F, 0.0F, 0.0F, 1.0F );

  if ( isLighting ) {
    static GLfloat mat_default[] = { 0.0F, 0.0F, 0.0F, 1.0F };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);
  }

  if ( m_player == thePlayer ) {
    double targetX, targetY;
    if ( theBall.GetStatus() == 2 || theBall.GetStatus() == 3 )
      m_player->GetModifiedTarget( targetX, targetY );
    else {
      targetX = m_player->GetTargetX();
      targetY = m_player->GetTargetY();
    }

    double diff;

    diff = (double)(220-m_player->GetStatus())/220*3.141592F/18;
    DrawTargetCircle( diff );

    targetX = thePlayer->GetTargetX();
    targetY = thePlayer->GetTargetY();

    glColor4f( 1.0F, 0.0F, 0.0F, 0.0F );
    glBegin(GL_POLYGON);
      glNormal3f( 0.0F, 1.0F, 0.0F );
      glVertex3f( targetX-0.08F, targetY, TABLEHEIGHT+1.7320508F*0.08F );
      glVertex3f( targetX+0.08F, targetY, TABLEHEIGHT+1.7320508F*0.08F );
      glVertex3f( targetX, targetY, TABLEHEIGHT );
    glEnd();

    // Hit point
    static long count = 0;

    glColor4f(0.5F, 0.0F, 0.0F, 1.0F );
    glPushMatrix();
      glTranslatef( thePlayer->GetX()+0.3F, thePlayer->GetY(), 1.0F );
      glRotatef( count, 0.0F, 0.0F, 1.0F );
      glBegin(GL_QUADS);
        glVertex3f( -0.1F, -0.01F, 0.0F );
        glVertex3f( -0.1F,  0.01F, 0.0F );
        glVertex3f(  0.1F,  0.01F, 0.0F );
        glVertex3f(  0.1F, -0.01F, 0.0F );
        glVertex3f( -0.01F, -0.1F, 0.0F );
        glVertex3f( -0.01F,  0.1F, 0.0F );
        glVertex3f(  0.01F,  0.1F, 0.0F );
        glVertex3f(  0.01F, -0.1F, 0.0F );
      glEnd();
    glPopMatrix();

    glPushMatrix();
      glTranslatef( thePlayer->GetX()-0.3F, thePlayer->GetY(), 1.0F );
      glRotatef( count, 0.0F, 0.0F, 1.0F );
      glBegin(GL_QUADS);
        glVertex3f( -0.1F, -0.01F, 0.0F );
        glVertex3f( -0.1F,  0.01F, 0.0F );
        glVertex3f(  0.1F,  0.01F, 0.0F );
        glVertex3f(  0.1F, -0.01F, 0.0F );
        glVertex3f( -0.01F, -0.1F, 0.0F );
        glVertex3f( -0.01F,  0.1F, 0.0F );
        glVertex3f(  0.01F,  0.1F, 0.0F );
        glVertex3f(  0.01F, -0.1F, 0.0F );
      glEnd();
    glPopMatrix();
    count++;
  }

//  glDepthMask(1);

  return true;
}

void
PlayerView::DrawTargetCircle( double diff ) {
  Ball *tmpBall;
  double vx, vy, vz;
  double rad;
  static double ballHeight = 1.4F;
  static bool count = true;

  if (isSimple) {
    count = !count;
    if ( count )
      return;
  }

  // Make pseudo ball near the Player
  if ( (m_player->GetSide() == 1 &&
	(theBall.GetStatus() == 2 || theBall.GetStatus() == 3 || 
       theBall.GetStatus() == 5)) ||
       (m_player->GetSide() == -1 &&
	(theBall.GetStatus() == 0 || theBall.GetStatus() == 1 || 
       theBall.GetStatus() == 4)) ) {
    ballHeight = theBall.GetZ();
    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    while ( tmpBall->GetStatus() >= 0 &&
	    tmpBall->GetY()*m_player->GetSide() >
	    m_player->GetY()*m_player->GetSide() )
      tmpBall->Move();

    if ( tmpBall->GetStatus() < 0 )
      tmpBall->Warp( m_player->GetX()+0.3F*m_player->GetSide(),
		     m_player->GetY(), ballHeight, 
		     0.0F, 0.0F, 0.0F, 0.0F, 3 );
  } else {
    tmpBall = new Ball( m_player->GetX()+0.3F*m_player->GetSide(),
			m_player->GetY(), ballHeight, 
			0.0F, 0.0F, 0.0F, 0.0F, 3 );
  }

  // Calc initial speed of the ball when the player hit the ball
  double dum, level, maxVy;

  m_player->CalcLevel( tmpBall, dum, level, maxVy );
  tmpBall->TargetToV( m_player->GetTargetX(), m_player->GetTargetY(), 
		      level, m_player->GetSpin(), vx, vy, vz, 0.1F, maxVy );

  // Add difference to the initial speed. Calc bound location
  double v, v1x, v1y, v1z;
  double n1x, n1y, n1z, n2x, n2y, n2z;
  double bx = tmpBall->GetX(), by = tmpBall->GetY(), bz = tmpBall->GetZ();
  double polygon1[64][3], polygon2[64][3];
  int polyNum1 = 0, polyNum2 = 0;
  int i;

  v = sqrt(vx*vx+vy*vy+vz*vz);
  n1x = vy/hypot(vx, vy) * v*tan(diff);
  n1y = -vx/hypot(vx, vy) * v*tan(diff);
  n1z = 0;
  n2x = vx*vz/(v*hypot(vx, vy)) * v*tan(diff);
  n2y = vy*vz/(v*hypot(vx, vy)) * v*tan(diff);
  n2z = (vx*vx+vy*vy)/(v*hypot(vx, vy)) * v*tan(diff);

  for ( rad = 0.0 ; rad < 3.141592F*2 ; rad += 3.141592F/32 ) {
    v1x = vx+n1x*cos(rad)+n2x*sin(rad);
    v1y = vy+n1y*cos(rad)+n2y*sin(rad);
    v1z = vz+n1z*cos(rad)+n2z*sin(rad);

    if ( m_player->GetSide() > 0 ) {
      tmpBall->Warp( bx, by, bz, 
		     v1x, v1y, v1z, m_player->GetSpin(), 0 );
      while ( tmpBall->GetZ() > TABLEHEIGHT &&
	      tmpBall->GetZ()+tmpBall->GetVZ()*TICK > TABLEHEIGHT &&
	      tmpBall->GetVY() > 0.0 &&	// When the ball hits the net
	      tmpBall->GetStatus() == 0 )
	tmpBall->Move();
    } else {
      tmpBall->Warp( bx, by, bz, 
		     v1x, v1y, v1z, m_player->GetSpin(), 2 );
      while ( tmpBall->GetZ() > TABLEHEIGHT &&
	      tmpBall->GetZ()+tmpBall->GetVZ()*TICK > TABLEHEIGHT &&
	      tmpBall->GetVY() < 0.0 &&	// When the ball hits the net
	      tmpBall->GetStatus() == 2 )
	tmpBall->Move();
    }

    if ( tmpBall->GetY()*m_player->GetSide() > 0.0 ) {
      polygon1[polyNum1][0] = tmpBall->GetX();
      polygon1[polyNum1][1] = tmpBall->GetY();
      polygon1[polyNum1][2] = TABLEHEIGHT+0.01F;
      polyNum1++;
    } else {
      polygon2[polyNum2][0] = tmpBall->GetX();
      polygon2[polyNum2][1] = tmpBall->GetY();
      polygon2[polyNum2][2] = tmpBall->GetZ();
      polyNum2++;
    }
  }

  glBegin(GL_POLYGON);
    for ( i = 0 ; i < polyNum1 ; i++ )
      glVertex3f( polygon1[i][0], polygon1[i][1], polygon1[i][2] );
  glEnd();

  glBegin(GL_POLYGON);
    for ( i = 0 ; i < polyNum2 ; i++ )
      glVertex3f( polygon2[i][0], polygon2[i][1], polygon2[i][2] );
  glEnd();

  delete tmpBall;
}
