/**
 * @file
 * @brief Implementation of PlayerView class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2000-2004  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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
#if defined(CHIYO)
#include "loadparts.h"
#endif
#include "Ball.h"
#include "BaseView.h"
#include "RCFile.h"
#include "Control.h"

//////////////////////////////////////////////////////////////////////////
#define RENDER_ANIMEEDGE 1

//////////////////////////////////////////////////////////////////////////

extern RCFile *theRC;
extern SDL_mutex *loadMutex;
extern Ball   theBall;
extern long mode;

#if !defined(CHIYO)
partsmotion_t *PlayerView::motion_Fnormal = NULL;
partsmotion_t *PlayerView::motion_Bnormal = NULL;
partsmotion_t *PlayerView::motion_Fdrive = NULL;
partsmotion_t *PlayerView::motion_Fcut = NULL;
partsmotion_t *PlayerView::motion_Bcut = NULL;
partsmotion_t *PlayerView::motion_Fpeck = NULL;
partsmotion_t *PlayerView::motion_Bpeck = NULL;
partsmotion_t *PlayerView::motion_Fsmash = NULL;
#endif

/***********************************************************************
 *	Class PlayerView
 ***********************************************************************/

/**
 * Default constructor. 
 * Initialize member variables to 0 or NULL. 
 */
PlayerView::PlayerView() {
  m_player = NULL;

  m_Fnormal = m_Bnormal = NULL;
  m_Fdrive  = m_Bdrive  = NULL;
  m_Fcut    = m_Bcut    = NULL;
  m_Fpeck   = m_Bpeck   = NULL;
  m_Fsmash  = m_Bsmash  = NULL;

  m_diff[0] = m_diff[1] = m_diff[2] = 0.0;
}

/**
 * Destructor. 
 * Do nothing. 
 */
PlayerView::~PlayerView() {
}

/**
 * Load parts and motion data. 
 * 
 * @param dum dummy. not used. 
 */
void *
PlayerView::LoadData(void *dum) {
#if !defined(CHIYO)
  partsmotion::loadmodel("Parts/model/");
  motion_Fnormal = new partsmotion("Parts/Fnormal/Fnormal");
  motion_Bnormal = new partsmotion("Parts/Bnormal/Bnormal");
  motion_Fdrive = new partsmotion("Parts/Fdrive/Fdrive");
  motion_Fcut = new partsmotion("Parts/Fcut/Fcut");
  motion_Bcut = new partsmotion("Parts/Bcut/Bcut");
  motion_Fpeck = new partsmotion("Parts/Fpeck/Fpeck");
  motion_Bpeck = new partsmotion("Parts/Bpeck/Bpeck");
  motion_Fsmash = new partsmotion("Parts/Fsmash/Fsmash");
#else /* CHIYO */
  chdir("Parts");
  parts::loadobjects("body.txt");
  chdir("..");
#endif /* !CHIYO */

  return NULL;
}

/**
 * Initializer method. 
 * Load parts and motion data of specified player. 
 * 
 * @param player Player object of which this object is attached. 
 * @return returns true if succeeds. 
 */
bool
PlayerView::Init( Player *player ) {
  m_player = player;

  SDL_mutexP( loadMutex );

#if !defined(CHIYO)
  m_Fnormal = motion_Fnormal;
  m_Bnormal = motion_Bnormal;
  m_Fdrive = motion_Fdrive;
  m_Fcut = motion_Fcut;
  m_Bcut = motion_Bcut;
  m_Fpeck = motion_Fpeck;
  m_Bpeck = motion_Bpeck;
  m_Fsmash = motion_Fsmash;
#else /* CHIYO */
# define GETBODY(name, ptype) \
    m_##name = reinterpret_cast<body_parts*>(parts::getobject(#name#ptype)); \
	if (!m_##name || parts::sym_body != m_##name->type()) {\
        printf("Could not load " #name #ptype "\n"); exit(1); \
    }

  switch ( m_player->GetPlayerType() ) {
  case PLAYER_PENATTACK:
  case PLAYER_PENDRIVE:
    GETBODY(Fnormal, Pen);
    GETBODY(Bnormal, Pen);
    GETBODY(Fdrive, Pen);
    GETBODY(Fcut, Pen);
    GETBODY(Bcut, Pen);
    GETBODY(Fpeck, Pen);
    GETBODY(Bpeck, Pen);
    GETBODY(Fsmash, Pen);
    break;
  case PLAYER_SHAKECUT:
    GETBODY(Fnormal, Shake);
    GETBODY(Bnormal, Shake);
    GETBODY(Fdrive, Shake);
    GETBODY(Fcut, Shake);
    GETBODY(Bcut, Shake);
    GETBODY(Fpeck, Shake);
    GETBODY(Bpeck, Shake);
    GETBODY(Fsmash, Shake);
    break;
  default:
    GETBODY(Fnormal, Pen);
    GETBODY(Bnormal, Pen);
    GETBODY(Fdrive, Pen);
    GETBODY(Fcut, Pen);
    GETBODY(Bcut, Pen);
    GETBODY(Fpeck, Pen);
    GETBODY(Bpeck, Pen);
    GETBODY(Fsmash, Pen);
    break;
  }

# undef GETBODY
#endif /* !CHIYO */

  SDL_mutexV( loadMutex );
  return true;
}

/**
 * Redraw player object. 
 * 
 * @return returns true if it is necessary to redraw. 
 */
bool
PlayerView::Redraw() {
  static GLfloat mat_green[] = { 0.1F, 0.1F, 0.1F, 1.0F };

  if ( Control::TheControl()->GetComPlayer() == m_player ) {
    glColor4f( 0.4F, 0.4F, 0.4F, 0.0F );
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_green);

    return SubRedraw();
  }

  return true;
}

/**
 * Redraw transparent player object. 
 * 
 * @return returns true if succeeds. 
 */
bool
PlayerView::RedrawAlpha() {
  static GLfloat mat_black[] = { 0.0F, 0.0F, 0.0F, 1.0F };

  if ( Control::TheControl()->GetThePlayer() == m_player ) {
    if ( theRC->myModel == MODEL_WIREFRAME ||
	 theRC->myModel == MODEL_ARMONLY ) {
      glColor4f( 1.0F, 1.0F, 1.0F, 1.0F );
    } else {
      glColor4f( 0.05F, 0.05F, 0.05F, 1.0F );
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_black);
    }

    return SubRedraw();
  }

  return true;
}

/**
 * Main body of drawing player model. 
 * 
 * @return  returns true if succeeds. 
 */
bool
PlayerView::SubRedraw() {

  DrawPlayer();

  // Target
  if ( Control::TheControl()->GetThePlayer() == m_player ) {
    DrawTarget();
#if 0
    if ( mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY ||
	 mode == MODE_PRACTICE )
      DrawMeter();
#endif
  }

  return true;
}

/**
 * Currently not used. 
 */
void
PlayerView::DrawTargetCircle( double diff ) {
  Ball *tmpBall;
  vector3d v;
  double rad;
  static double ballHeight = 1.4F;
  static bool count = true;

  if ( theRC->gmode == GMODE_SIMPLE ) {
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
    ballHeight = theBall.GetX()[2];
    tmpBall = new Ball(&theBall);

    while ( tmpBall->GetStatus() >= 0 &&
	    tmpBall->GetX()[1]*m_player->GetSide() >
	    m_player->GetX()[1]*m_player->GetSide() )
      tmpBall->Move();

    if ( tmpBall->GetStatus() < 0 ) {
      vector3d x;
      x[0] = m_player->GetX()[0]+0.3*m_player->GetSide();
      x[1] = m_player->GetX()[1];
      x[2] = ballHeight;
      tmpBall->Warp( x, vector3d(0.0), vector2d(0.0), 3 );
    }
  } else {
    vector3d x;
    x[0] = m_player->GetX()[0]+0.3*m_player->GetSide();
    x[1] = m_player->GetX()[1];
    x[2] = ballHeight;

    tmpBall = new Ball( x, vector3d(0.0), vector2d(0.0), 3 );
  }

  // Calc initial speed of the ball when the player hit the ball
  double dum, level, maxVy;

  m_player->CalcLevel( tmpBall, dum, level, maxVy );
  tmpBall->TargetToV( m_player->GetTarget(), level, m_player->GetSpin(), 
		      v, 0.1F, maxVy );

  // Add difference to the initial speed. Calc bound location
  double vl;
  vector3d v1;
  vector3d n1, n2;
  vector3d bx = tmpBall->GetX();
  double polygon1[64][3], polygon2[64][3];
  int polyNum1 = 0, polyNum2 = 0;
  int i;

  vl = v.len();
  n1[0] =  v[1]/hypot(v[0], v[1]) * vl*tan(diff);
  n1[1] = -v[0]/hypot(v[0], v[1]) * vl*tan(diff);
  n1[2] = 0;
  n2[0] =             v[0]*v[2]/(vl*hypot(v[0], v[1])) * vl*tan(diff);
  n2[1] =             v[1]*v[2]/(vl*hypot(v[0], v[1])) * vl*tan(diff);
  n2[2] = (v[0]*v[0]+v[1]*v[1])/(vl*hypot(v[0], v[1])) * vl*tan(diff);

  for ( rad = 0.0 ; rad < 3.141592F*2 ; rad += 3.141592F/32 ) {
    v1 = v + n1*cos(rad) + n2*sin(rad);

    if ( m_player->GetSide() > 0 ) {
      tmpBall->Warp( bx, v1, m_player->GetSpin(), 0 );

      while ( tmpBall->GetX()[2] > TABLEHEIGHT &&
	      tmpBall->GetX()[2]+tmpBall->GetV()[2]*TICK > TABLEHEIGHT &&
	      tmpBall->GetV()[1] > 0.0 &&	// When the ball hits the net
	      tmpBall->GetStatus() == 0 )
	tmpBall->Move();
    } else {
      tmpBall->Warp( bx, v1, m_player->GetSpin(), 2 );
      while ( tmpBall->GetX()[2] > TABLEHEIGHT &&
	      tmpBall->GetX()[2]+tmpBall->GetV()[2]*TICK > TABLEHEIGHT &&
	      tmpBall->GetV()[1] < 0.0 &&	// When the ball hits the net
	      tmpBall->GetStatus() == 2 )
	tmpBall->Move();
    }

    if ( tmpBall->GetX()[1]*m_player->GetSide() > 0.0 ) {
      polygon1[polyNum1][0] = tmpBall->GetX()[0];
      polygon1[polyNum1][1] = tmpBall->GetX()[1];
      polygon1[polyNum1][2] = TABLEHEIGHT+0.01F;
      polyNum1++;
    } else {
      polygon2[polyNum2][0] = tmpBall->GetX()[0];
      polygon2[polyNum2][1] = tmpBall->GetX()[1];
      polygon2[polyNum2][2] = tmpBall->GetX()[2];
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

// BallView ¤È¤«¤Ö¤ë. 
double
PlayerView::GetHitpointY() {
  Ball* tmpBall;

  if ( Control::TheControl()->GetThePlayer() &&
       (((theBall.GetStatus() == 2 || theBall.GetStatus() == 3) &&
	 Control::TheControl()->GetThePlayer()->GetSide() > 0) ||
	((theBall.GetStatus() == 0 || theBall.GetStatus() == 1) &&
	 Control::TheControl()->GetThePlayer()->GetSide() < 0)) ) {
    tmpBall = new Ball(&theBall);
    long t1 = 0, t2 = 0;
    // get time until the ball reaches hit point
    while ( tmpBall->GetStatus() != -1 ) {
      tmpBall->Move();
      if ( tmpBall->GetX()[1] < Control::TheControl()->GetThePlayer()->GetX()[1] &&
	   tmpBall->GetStatus() == 3 )
	break;
      t1++;
    }
    if ( tmpBall->GetStatus() == -1 )
      return m_player->GetX()[1]+0.6;

    delete tmpBall;

    tmpBall = new Ball(&theBall);

    while ( tmpBall->GetStatus() != -1 ){
      tmpBall->Move();
      t2++;
      if ( t1-10 == t2 ){
	return tmpBall->GetX()[1];
      }
    }
    delete tmpBall;
  }

  return m_player->GetX()[1]+0.6;
}

/**
 * Draw player object. 
 */
void
PlayerView::DrawPlayer() {
  double swing;
  partsmotion_t *motion;

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
  case SERVE_NORMAL:
    if ( m_player->ForeOrBack() )
      motion = m_Fnormal;
    else
      motion = m_Bnormal;
    break;
  case SERVE_POKE:
    if ( m_player->ForeOrBack() )
      motion = m_Fpeck ? m_Fpeck : m_Fnormal;
    else
      motion = m_Bpeck ? m_Bpeck : m_Bnormal;
    break;
  default:
    return;
  }

  vector3F diff;
  diff[0] = diff[1] = diff[2] = 0.0;

  if ( m_player->GetSwing() > 10 && m_player->GetSwing() < 20 ) {
    Ball *tmpBall;

    tmpBall = new Ball(&theBall);

    for ( int i = m_player->GetSwing() ; i < 20 ; i++ )
      tmpBall->Move();

    if ( ((tmpBall->GetStatus() == 3 && m_player->GetSide() == 1) ||
	  (tmpBall->GetStatus() == 1 && m_player->GetSide() == -1)) ) {
      double hitpointX = m_player->GetSwingSide() ?
	m_player->GetX()[0]+0.3*m_player->GetSide() :
	m_player->GetX()[0]-0.3*m_player->GetSide();

      diff[0] = tmpBall->GetX()[0] -
	(hitpointX+m_player->GetV()[0]*(20-m_player->GetSwing())*TICK);

      diff[1] =
        (tmpBall->GetX()[1] -
         (m_player->GetX()[1]+m_player->GetV()[1]*(20-m_player->GetSwing())*TICK))*
        m_player->GetSide();

      diff[2] = tmpBall->GetX()[2] - 0.85;	/* temporary */
    }

    if ( diff[0] > 0.3 )
      diff[0] = 0.3;
    if ( diff[0] < -0.3 )
      diff[0] = -0.3;
    if ( diff[1] > 0.3 )
      diff[1] = 0.3;
    if ( diff[1] < -0.3 )
      diff[1] = -0.3;
  }

  if ( m_player->GetSwing() >= 30 ) {
    m_diff *= 0.95;
  } else {
    m_diff = diff;
  }

  glPushMatrix();
    glTranslatef( m_player->GetX()[0]-0.3F*m_player->GetSide(),
		  m_player->GetX()[1], 0 );

    glRotatef( -atan2( m_player->GetTarget()[0]-m_player->GetX()[0],
		       m_player->GetTarget()[1]-m_player->GetX()[1] )*180/3.141592F,
	       0.0F, 0.0F, 1.0F );

    if ( theRC->gmode == GMODE_SIMPLE ) {
#ifndef CHIYO
      if ( Control::TheControl()->GetThePlayer() == m_player &&
	   theRC->myModel == MODEL_ARMONLY )
	motion->renderArmOnly(swing, m_diff);
      else
	motion->renderWire(swing, m_diff);
#else
      motion->renderWire(swing);
#endif
    } else {
      if (Control::TheControl()->GetComPlayer() == m_player) {
#ifndef CHIYO
	motion->render(swing, m_diff);
#else
	motion->render(swing);
#if RENDER_ANIMEEDGE
	glColor3f(0,0,0);
	glPolygonOffset(1.0f, 1.0f);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glLineWidth(2.0f);
	motion->renderWire(swing);
	glPolygonOffset(0, 0);
	glDisable(GL_POLYGON_OFFSET_LINE);
	glLineWidth(1.0f);
#endif /* RENDER_ANIMEEDGE */
#endif /* CHIYO */
      }
      if (Control::TheControl()->GetThePlayer() == m_player) {
#ifndef CHIYO
	switch ( theRC->myModel ) {
	case MODEL_TRANSPARENT:
	  glEnable(GL_CULL_FACE);
	  glDisable(GL_DEPTH_TEST);
	  glDepthMask(0);
	  motion->render(swing, m_diff);
	  glDepthMask(1);
	  glEnable(GL_DEPTH_TEST);
	  glDisable(GL_CULL_FACE);
	  break;
	case MODEL_WIREFRAME:
	  motion->renderWire(swing, m_diff);
	  break;
	case MODEL_ARMONLY:
	  motion->renderArmOnly(swing, m_diff);
	  break;
	}
#else
	switch ( theRC->myModel ) {
	case MODEL_TRANSPARENT:
	  glEnable(GL_CULL_FACE);
	  glDisable(GL_DEPTH_TEST);
	  glDepthMask(0);
	  motion->render(swing);
	  glDepthMask(1);
	  glEnable(GL_DEPTH_TEST);
	  glDisable(GL_CULL_FACE);
	  break;
	case MODEL_WIREFRAME:
          motion->renderWire(swing);
	  break;
	}
#endif
      }
    }

  glPopMatrix();
}

/**
 * Draw target triangle. 
 */
void
PlayerView::DrawTarget() {
  glColor4f( 1.0F, 0.0F, 0.0F, 0.5F );

  static GLfloat mat_default[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);

  vector2d target;
  if ( theBall.GetStatus() == 2 || theBall.GetStatus() == 3 )
    m_player->GetModifiedTarget( target );
  else {
    target = m_player->GetTarget();
  }

  double diff;

  diff = (double)(220-m_player->GetStatus())/220*3.141592F/18;
  //DrawTargetCircle( diff );

  target = Control::TheControl()->GetThePlayer()->GetTarget();

  glColor4f( 1.0F, 0.0F, 0.0F, 1.0F );
  glBegin(GL_POLYGON);
    glNormal3f( 0.0F, 1.0F, 0.0F );
    glVertex3f( target[0]-0.08F, target[1], TABLEHEIGHT+1.7320508F*0.08F );
    glVertex3f( target[0]+0.08F, target[1], TABLEHEIGHT+1.7320508F*0.08F );
    glVertex3f( target[0], target[1], TABLEHEIGHT );
  glEnd();
}

void
PlayerView::DrawMeter() {
  static long count = 0;

  count++;

  long status = m_player->GetStatus();

  if ( status < m_player->StatusBorder() && count%10 < 5 )
    return;

  glDisable(GL_DEPTH_TEST);
  glDepthMask(0);

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLfloat)BaseView::GetWinWidth(),
	      (GLfloat)BaseView::GetWinHeight(), 0.0 );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  long statusBarX, statusBarY, statusBarWidth, statusBarHeight;
  statusBarX = 0;
  statusBarY = 1;

  statusBarWidth = BaseView::GetWinWidth()-2;
  statusBarHeight = BaseView::GetWinHeight()/20;

  glColor4f( 0.8F, 0.8F, 0.4F, 1.0F );
  glBegin(GL_QUADS);
    glVertex2i( statusBarX, statusBarY );
    glVertex2i( statusBarX+statusBarWidth*status/200,
		statusBarY );
    glVertex2i( statusBarX+statusBarWidth*status/200,
		statusBarY+statusBarHeight );
    glVertex2i( statusBarX, statusBarY+statusBarHeight );
  glEnd();

  glColor4f( 1.0F, 0.0F, 0.0F, 1.0F );
  glBegin(GL_QUADS);
    glVertex2i( statusBarX+statusBarWidth*status/200,
		statusBarY );
    glVertex2i( statusBarX+statusBarWidth*status/200,
		statusBarY+statusBarHeight );
    glVertex2i( statusBarX+statusBarWidth, statusBarY+statusBarHeight );
    glVertex2i( statusBarX+statusBarWidth, statusBarY );
  glEnd();

  glColor4f( 1.0F, 1.0F, 1.0F, 1.0F );
  glBegin(GL_LINE_LOOP);
    glVertex2i( statusBarX, statusBarY );
    glVertex2i( statusBarX+statusBarWidth, statusBarY );
    glVertex2i( statusBarX+statusBarWidth, statusBarY+statusBarHeight );
    glVertex2i( statusBarX, statusBarY+statusBarHeight );
  glEnd();

  glColor4f( 1.0F, 0.0F, 0.0F, 1.0F );
  glBegin(GL_LINES);
    glVertex2i( statusBarX+statusBarWidth*m_player->StatusBorder()/200,
		statusBarY );
    glVertex2i( statusBarX+statusBarWidth*m_player->StatusBorder()/200,
		statusBarY+statusBarHeight );
  glEnd();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glDepthMask(1);
  if ( theRC->gmode != GMODE_SIMPLE )
    glEnable(GL_DEPTH_TEST);
}
