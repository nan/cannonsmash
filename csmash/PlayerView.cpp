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

extern bool isPolygon;
extern bool isLighting;

#if HAVE_LIBPTHREAD
extern pthread_mutex_t loadMutex;
#endif

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

bool RotateMatrix( double Mat[][3], double degx, double degy, double degz );

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

void
PlayerView::LoadData() {
  motion_Fnormal = new partsmotion("Parts/Fnormal/Fnormal");
  motion_Bnormal = new partsmotion("Parts/Bnormal/Bnormal");
  motion_Fdrive = new partsmotion("Parts/Fdrive/Fdrive");
  motion_Fcut = new partsmotion("Parts/Fcut/Fcut");
  motion_Bcut = new partsmotion("Parts/Bcut/Bcut");
  motion_Fpeck = new partsmotion("Parts/Fpeck/Fpeck");
  motion_Bpeck = new partsmotion("Parts/Bpeck/Bpeck");
  motion_Fsmash = new partsmotion("Parts/Fsmash/Fsmash");
}

bool
PlayerView::Init( Player *player ) {
  m_player = player;

#if HAVE_LIBPTHREAD
  pthread_mutex_lock( &loadMutex );
  m_Fnormal = motion_Fnormal;
  m_Bnormal = motion_Bnormal;
  m_Fdrive = motion_Fdrive;
  m_Fcut = motion_Fcut;
  m_Bcut = motion_Bcut;
  m_Fpeck = motion_Fpeck;
  m_Bpeck = motion_Bpeck;
  m_Fsmash = motion_Fsmash;
  pthread_mutex_unlock( &loadMutex );
#else
  m_Fnormal = motion_Fnormal;
  m_Bnormal = motion_Bnormal;
  m_Fdrive = motion_Fdrive;
  m_Fcut = motion_Fcut;
  m_Bcut = motion_Bcut;
  m_Fpeck = motion_Fpeck;
  m_Bpeck = motion_Bpeck;
  m_Fsmash = motion_Fsmash;
#endif

  return true;
}

bool
PlayerView::Redraw() {
  static GLfloat mat_green[] = { 0.4, 0.4, 0.4, 1.0 };

  if ( m_player == comPlayer ) {
    if ( isLighting ) {
      glColor4f( 0.4, 0.4, 0.4, 0.0 );
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_green);
    } else {
      glColor4f( 0.3, 0.3, 0.3, 1.0 );
    }

    return SubRedraw();
  }

  return true;
}

bool
PlayerView::RedrawAlpha() {
//  static GLfloat mat_black[] = { 0.0, 0.0, 0.0, 1.0 };

  if ( m_player == thePlayer ) {
    if ( isLighting ) {
// Configで切替えるようにする
//      glColor4f( 0.01, 0.01, 0.01, 1.0 );
//      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_black);
      glColor4f( 1.0, 1.0, 1.0, 1.0 );
    } else {
      glColor4f( 0.1, 0.1, 0.1, 1.0 );
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
    glTranslatef( m_player->GetX(), m_player->GetY(), 0 );
  else
    glTranslatef( m_player->GetX(), m_player->GetY(), m_player->GetZ() );

  glRotatef( -atan2( m_player->GetTargetX()-m_player->GetX(),
		     m_player->GetTargetY()-m_player->GetY() )*180.0/3.141592, 
	     0.0, 0.0, 1.0 );

  //if ( m_player->GetSide() < 0 )
  //glRotatef( 180.0, 0.0, 0.0, 1.0 );

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
    if (m_player == comPlayer) {
      motion->render(swing);
    }
    if (m_player == thePlayer) {
      motion->renderWire(swing);
    }

//    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

  } else {
    m_player->GetShoulder( x, y, deg );

    glTranslatef( -0.15, 0.0, 0.0 );

    // 頭
    glPushMatrix();
    glTranslatef( x, 0.0, 0.0 );
    if ( m_player->GetSide() < 0 )
      glutWireSphere( 0.15, 8, 8 );
    glPopMatrix();

    // 体
    glRotatef( deg, 0.0, 0.0, 1.0 );

    // 台上処理
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
      glVertex3f( 0.15, ydiff, -0.15-0.3+y );
      glVertex3f( -0.15, ydiff, -0.15-0.3+y );
      glVertex3f( -0.15+x, 0.0, 0.15-0.3+y );
      glVertex3f( 0.15+x, 0.0, 0.15-0.3+y );
    glEnd();

    // 左腕
    glBegin(GL_LINE_STRIP);
      glVertex3f( -0.15+x, 0.0, 0.15-0.3+y );
      glVertex3f( -0.15-0.10+x, 0.0, 0.15-0.3-0.25+y );
      glVertex3f( -0.15-0.10+x, 0.30, 0.15-0.3-0.25+y );
    glEnd();

    // 右腕
    // 右肩を原点に
    glTranslatef( 0.15+x, 0.0, -0.15+y );

    // 右上腕
    m_player->GetElbow( degx, degy );
    glRotatef( degx, 1.0, 0.0, 0.0 );
    glRotatef( degy, 0.0, 1.0, 0.0 );

    glBegin(GL_LINE_STRIP);
      glVertex3f( 0.0, 0.0, 0.0 );
      glVertex3f( 0.0, 0.0, -UPPERARM );
    glEnd();

    // 右肘を原点に
    glTranslatef( 0.0, 0.0, -UPPERARM );

    // 右下腕
    m_player->GetHand( degx, degy, degz );
    glRotatef( degz, 0.0, 0.0, 1.0 );
    glRotatef( degx, 1.0, 0.0, 0.0 );
    glRotatef( degy, 0.0, 1.0, 0.0 );

    glBegin(GL_LINE_STRIP);
      glVertex3f( 0.0, 0.0, 0.0 );
      glVertex3f( 0.0, FOREARM, 0.0 );
    glEnd();

    // ラケット
    glBegin(GL_LINE_LOOP);
      glVertex3f( 0.0, FOREARM, -0.05 );
      glVertex3f( 0.0, FOREARM, 0.05 );
      glVertex3f( 0.0, FOREARM+0.2, 0.05 );
      glVertex3f( 0.0, FOREARM+0.2, -0.05 );
    glEnd();
  }

  glPopMatrix();

  // 落下目標位置
  if ( m_player->GetStatus() > 100 )
    glColor4f( 0.2, 0.0, 0.0, 1.0 );
  else
    glColor4f( 0.5, 0.0, 0.0, 1.0 );

  if ( isLighting ) {
    static GLfloat mat_default[] = { 0.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);
  }

  if ( m_player == thePlayer ) {
    double rad, Xdiff, Ydiff;

    Xdiff = TABLEWIDTH/4*(220-m_player->GetStatus())/220;
    Ydiff = TABLELENGTH/8*(220-m_player->GetStatus())/220;
    glBegin(GL_POLYGON);
      for ( rad = 0.0 ; rad < 3.141592*2 ; rad += 3.141592/8.0 )
	glVertex3f( m_player->GetTargetX()+Xdiff*cos(rad),
		    m_player->GetTargetY()+Ydiff*sin(rad),
		    TABLEHEIGHT+0.01 );
    glEnd();

    glColor4f( 1.0, 0.0, 0.0, 0.0 );
    glBegin(GL_POLYGON);
      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f( m_player->GetTargetX()-0.08, m_player->GetTargetY(),
		  TABLEHEIGHT+1.7320508*0.08 );
      glVertex3f( m_player->GetTargetX()+0.08, m_player->GetTargetY(),
		  TABLEHEIGHT+1.7320508*0.08 );
      glVertex3f( m_player->GetTargetX(), m_player->GetTargetY(),
		  TABLEHEIGHT );
    glEnd();
  }

  glDepthMask(1);

  return true;
}

// x軸, y軸, z軸まわりで, それぞれdegx, degy, degzだけ回転させる
// 回転行列を求める. 
bool
RotateMatrix( double Mat[][3], double degx, double degy, double degz ) {
  double MRot[3][3];
  double M[3][3];
  int i, j;

  memset(MRot, 0, sizeof(double)*9);
  MRot[0][0] = 1;
  MRot[1][1] = cos(degx*3.141592/180);
  MRot[1][2] = -sin(degx*3.141592/180);
  MRot[2][1] = sin(degx*3.141592/180);
  MRot[2][2] = cos(degx*3.141592/180);

  for ( i = 0 ; i < 3 ; i++ ) {
    for ( j = 0 ; j < 3 ; j++ ) {
      M[i][j] = Mat[i][0]*MRot[0][j] + Mat[i][1]*MRot[1][j]
	+ Mat[i][2]*MRot[2][j];
    }
  }

  memset(MRot, 0, sizeof(double)*9);
  MRot[1][1] = 1;
  MRot[2][2] = cos(degy*3.141592/180);
  MRot[2][0] = -sin(degy*3.141592/180);
  MRot[0][2] = sin(degy*3.141592/180);
  MRot[0][0] = cos(degy*3.141592/180);

  for ( i = 0 ; i < 3 ; i++ ) {
    for ( j = 0 ; j < 3 ; j++ ) {
      Mat[i][j] = M[i][0]*MRot[0][j] + M[i][1]*MRot[1][j]
	+ M[i][2]*MRot[2][j];
    }
  }

  memset(MRot, 0, sizeof(double)*9);
  MRot[2][2] = 1;
  MRot[0][0] = cos(degz*3.141592/180);
  MRot[0][1] = -sin(degz*3.141592/180);
  MRot[1][0] = sin(degz*3.141592/180);
  MRot[1][1] = cos(degz*3.141592/180);

  for ( i = 0 ; i < 3 ; i++ ) {
    for ( j = 0 ; j < 3 ; j++ ) {
      M[i][j] = Mat[i][0]*MRot[0][j] + Mat[i][1]*MRot[1][j]
	+ Mat[i][2]*MRot[2][j];
    }
  }

  for ( i = 0 ; i < 3 ; i++ ) {
    for ( j = 0 ; j < 3 ; j++ ) {
      Mat[i][j] = M[i][j];
    }
  }

  return true;
}
