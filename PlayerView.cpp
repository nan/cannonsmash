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
extern MotionData *motion_Fnormal;
extern MotionData *motion_Bnormal;
extern MotionData *motion_Fdrive;
extern MotionData *motion_Fcut;
extern MotionData *motion_Bcut;
extern MotionData *motion_Fpeck;
extern MotionData *motion_Bpeck;
extern MotionData *motion_Fsmash;

bool RotateMatrix( double Mat[][3], double degx, double degy, double degz );

MotionData::MotionData() {
}

MotionData::~MotionData() {
}

bool
MotionData::LoadData( char *fname, int points ) {
  int i, j, k;
  long poly;

  for ( i = 0 ; i < 50 ; i++ ) {
    char inFileName[256];
#ifndef HAVE_LIBZ
    FILE *fp;
#else
    gzFile fp;
#endif

    sprintf( inFileName, fname, i+1 );

#ifndef HAVE_LIBZ
    if ( (fp = fopen( inFileName, "rb" )) == NULL )
      return false;

    fread( &(m_points[i][0][0]), sizeof(GLfloat), points*3, fp );
    fread( &poly, sizeof(long), 1, fp );
    fread( &(m_poly[i][0][0]), sizeof(GLshort), poly*4, fp );

    m_poly[i][poly][0] = m_poly[i][poly][1] = m_poly[i][poly][2] =
      m_poly[i][poly][3] = -1;

    fclose( fp );
#else
    fp = gzopenx(inFileName, "rbs");
    if (NULL == fp) return false;

    gzread(fp, &(m_points[i][0][0]), sizeof(GLfloat) * points*3);
    gzread(fp, &poly, sizeof(long) * 1);
    gzread(fp, &(m_poly[i][0][0]), sizeof(GLshort) * poly*4);

    m_poly[i][poly][0] = m_poly[i][poly][1] = m_poly[i][poly][2] =
      m_poly[i][poly][3] = -1;

    gzclose(fp);
#endif

// test implement. ポリゴンをsortして手前から描くようにすれば速くなるか?
#if 0
    int flag;
    double max;
    long p0, p1, p2, p3;
    double maxTable[1024];

    for ( j = 0 ; ; j++ ) {
      if ( m_poly[i][j][0] < 0 )
	break;

      maxTable[j] = -300.0;
      if ( m_points[i][m_poly[i][j][0]][1] > maxTable[j] )
	maxTable[j] = m_points[i][m_poly[i][j][0]][1];
      if ( m_points[i][m_poly[i][j][1]][1] > maxTable[j] )
	maxTable[j] = m_points[i][m_poly[i][j][1]][1];
      if ( m_points[i][m_poly[i][j][2]][1] > maxTable[j] )
	maxTable[j] = m_points[i][m_poly[i][j][2]][1];
      if (m_poly[i][j][3]>=0 && m_points[i][m_poly[i][j][3]][1]>maxTable[j])
	maxTable[j] = m_points[i][m_poly[i][j][3]][1];
    }

    while (1) {
      flag = 0;
      for ( j = 1 ; ; j++ ) {
	if ( m_poly[i][j][0] < 0 )
	  break;

	if ( maxTable[j] > maxTable[j-1] ) {
	  flag = 1;
	  p0 = m_poly[i][j][0];
	  p1 = m_poly[i][j][1];
	  p2 = m_poly[i][j][2];
	  p3 = m_poly[i][j][3];

	  m_poly[i][j][0] = m_poly[i][j-1][0];
	  m_poly[i][j][1] = m_poly[i][j-1][1];
	  m_poly[i][j][2] = m_poly[i][j-1][2];
	  m_poly[i][j][3] = m_poly[i][j-1][3];

	  m_poly[i][j-1][0] = p0;
	  m_poly[i][j-1][1] = p1;
	  m_poly[i][j-1][2] = p2;
	  m_poly[i][j-1][3] = p3;

	  max = maxTable[j];
	  maxTable[j] = maxTable[j-1];
	  maxTable[j-1] = max;
	}
      }
      if ( !flag )
	break;
    }

    printf( "%d\n", i );
#endif
// end test implement

    for ( j = 0 ; ; j++ ) {
      int p0, p1, p2;
      double vx1, vy1, vz1, vx2, vy2, vz2;

      p0 = m_poly[i][j][0];
      p1 = m_poly[i][j][1];
      p2 = m_poly[i][j][2];

      if ( p0 < 0 )
	break;

      vx1 = m_points[i][p1][0] - m_points[i][p0][0];
      vy1 = m_points[i][p1][1] - m_points[i][p0][1];
      vz1 = m_points[i][p1][2] - m_points[i][p0][2];

      vx2 = m_points[i][p2][0] - m_points[i][p1][0];
      vy2 = m_points[i][p2][1] - m_points[i][p1][1];
      vz2 = m_points[i][p2][2] - m_points[i][p1][2];

      m_normal[i][j][0] = vy1*vz2-vz1*vy2;
      m_normal[i][j][1] = vz1*vx2-vx1*vz2;
      m_normal[i][j][2] = vx1*vy2-vy1*vx2;
    }
  }

  return true;
}

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

  if ( m_player->GetSide () < 0 ) {
    if ( isLighting ) {
      static GLfloat mat_green[] = { 0.0, 0.3, 0.0, 1.0 };
      static GLfloat mat_black[] = { 0.0, 0.0, 0.0, 1.0 };
    
      glColor4f( 0.9, 0.9, 0.9, 0.0 );
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_green);
    } else {
      glColor4f( 0.3, 0.3, 0.3, 1.0 );
    }

    return SubRedraw();
  }
}

bool
PlayerView::RedrawAlpha() {
  if ( m_player->GetSide() > 0 ) {
    if ( isLighting ) {
      static GLfloat mat_green[] = { 0.0, 0.3, 0.0, 1.0 };
      static GLfloat mat_black[] = { 0.0, 0.0, 0.0, 1.0 };

      glColor4f( 0.01, 0.01, 0.01, 1.0 );
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_black);
    } else {
      glColor4f( 0.1, 0.1, 0.1, 1.0 );
    }

    return SubRedraw();
  }
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

  if ( m_player->GetSide() < 0 )
    glRotatef( 180.0, 0.0, 0.0, 1.0 );

  if ( isPolygon ) {
    int i;

    if ( m_player->GetSide() > 0 ) {
      glDisable(GL_DEPTH_TEST);
      glDepthMask(0);
    }

    int swing;
    MotionData *motion;

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

#if 1
    int lastP = 4;
    glBegin(GL_QUADS);
    for ( i = 0 ; ; i++ ) {
      int p0, p1, p2, p3;

      p0 = motion->m_poly[swing][i][0];
      p1 = motion->m_poly[swing][i][1];
      p2 = motion->m_poly[swing][i][2];
      p3 = motion->m_poly[swing][i][3];

      if ( p0 < 0 ) {
	glEnd();
	break;
      }

      if ( p3 < 0 && lastP != 3 ) {
	glEnd();
	glBegin(GL_TRIANGLES);
	lastP = 3;
      } else if ( p3 >= 0 && lastP != 4 ) {
	glEnd();
	glBegin(GL_QUADS);
	lastP = 4;
      }

      glNormal3fv( &(motion->m_normal[swing][i][0]) );
      glVertex3fv( &(motion->m_points[swing][p0][0]) );
      glVertex3fv( &(motion->m_points[swing][p1][0]) );
      glVertex3fv( &(motion->m_points[swing][p2][0]) );
      if ( p3 >= 0 ) {
	glNormal3fv( &(motion->m_normal[swing][i][0]) );
	glVertex3fv( &(motion->m_points[swing][p3][0]) );
      }
    }
#else
//    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer( 3, GL_FLOAT, 0, &(motion->m_points[swing][0][0]) );
//    glNormalPointer();
    for ( i = 0 ; ; i++ ) {
      if ( motion->m_poly[swing][i][0] < 0 )
	break;

      if ( motion->m_poly[swing][i][3] < 0 )
	glDrawElements( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, 
			&(motion->m_poly[swing][i][0]) );
      else
	glDrawElements( GL_QUADS, 4, GL_UNSIGNED_SHORT, 
			&(motion->m_poly[swing][i][0]) );
    }
//    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
#endif
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
  glColor4f( 0.2, 0.0, 0.0, 1.0 );
  if ( isLighting ) {
    static GLfloat mat_default[] = { 0.0, 0.0, 0.0, 1.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);
  }

  if ( m_player->GetSide() > 0 ) {
    double rad, Xdiff, Ydiff;

    Xdiff = TABLEWIDTH/2*(220-m_player->GetStatus())/220;
    Ydiff = TABLELENGTH/4*(220-m_player->GetStatus())/220;
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
