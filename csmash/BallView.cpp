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
extern Player* thePlayer;
extern Player *comPlayer;
extern long mode;

extern bool isLighting;

GLuint BallView::m_number[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

BallView::BallView() {
}

BallView::~BallView() {
}

bool
BallView::Init() {
  ImageData numImage;
#ifdef HAVE_LIBZ
  static char num[][20] = {"images/zero.ppm.gz", "images/one.ppm.gz",
			   "images/two.ppm.gz", "images/three.ppm.gz",
			   "images/four.ppm.gz", "images/five.ppm.gz",
			   "images/six.ppm.gz", "images/seven.ppm.gz",
			   "images/eight.ppm.gz", "images/nine.ppm.gz"};
#else
  static char num[][20] = {"images/zero.ppm", "images/one.ppm",
			   "images/two.ppm", "images/three.ppm",
			   "images/four.ppm", "images/five.ppm",
			   "images/six.ppm", "images/seven.ppm",
			   "images/eight.ppm", "images/nine.ppm"};
#endif

  if ( m_number[0] == 0 ) {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures( 10, m_number );

    for ( int i = 0 ; i < 10 ; i++ ) {
      numImage.LoadPPM( &num[i][0] );
      glBindTexture( GL_TEXTURE_2D, m_number[i] );
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

      glTexImage2D(GL_TEXTURE_2D, 0, 3,
		   numImage.GetWidth(), numImage.GetHeight(), 
		   0, GL_RGBA, GL_UNSIGNED_BYTE,
		   numImage.GetImage() );
    }
  }

  return true;
}
 
bool
BallView::Redraw() {
  double rad;
  Ball* tmpBall;
  const static GLfloat mat_yel[] = { 1.0, 0.8, 0.0, 0.0 };

  // Ball自体の描画
  glColor4f(1.0, 0.8, 0.0, 0.0);
  if ( isLighting ) {
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_yel);
  }

  glPushMatrix();
    glTranslatef( theBall.GetX(), theBall.GetY(), theBall.GetZ() );
    glutSolidSphere( BALL_R, 12, 12 );
  glPopMatrix();

  // 影を描画する
  glColor4f(0.0, 0.0, 0.0, 0.0);
  if ( theBall.GetY() > -TABLELENGTH/2 && theBall.GetY() < TABLELENGTH/2 ){
    glBegin(GL_POLYGON);
      for ( rad = 0.0 ; rad < 3.141592*2 ; rad += 3.141592/4.0 )
	glVertex3f( theBall.GetX()+BALL_R*cos(rad),
		    theBall.GetY()+BALL_R*sin(rad),
		    TABLEHEIGHT+0.01 );
    glEnd();
  }
  else{
    glBegin(GL_POLYGON);
      for ( rad = 0.0 ; rad < 3.141592*2 ; rad += 3.141592/4.0 )
	glVertex3f( theBall.GetX()+BALL_R*cos(rad),
		    theBall.GetY()+BALL_R*sin(rad),
		    0.01 );
    glEnd();
  }

  // ボールの将来の軌跡を表示する
  if ( theBall.GetStatus() == 2 || theBall.GetStatus() == 3 ){
    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );
    long t1 = 0, t2 = 0;
    // 打球点に到達するまでの時間を求める
    while ( tmpBall->GetStatus() != -1 ){
      tmpBall->Move();
      if ( tmpBall->GetY() < thePlayer->GetY() && tmpBall->GetStatus() == 3 )
	break;
      t1++;
    }
    if ( tmpBall->GetStatus() == -1 )
      t1 += 100000;	// 赤玉表示なし

    delete tmpBall;

    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    while ( tmpBall->GetStatus() != -1 ){
      tmpBall->Move();
      t2++;
      if ( t1-10 == t2 ){
	glColor4f(1.0, 0.0, 0.0, 0.0);
	if ( isLighting ) {
	  const static GLfloat mat_red[] = { 1.0, 0.0, 0.0, 0.0 };
	  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_red);
	}
	glPushMatrix();
	glTranslatef( tmpBall->GetX(), tmpBall->GetY(), tmpBall->GetZ() );
	glutSolidSphere( BALL_R/2, 3, 3 );
	glPopMatrix();
      } else if ( t1 == t2 ){
	glColor4f(1.0, 0.8, 0.0, 0.0);
	if ( isLighting )
	  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_yel);
	glPushMatrix();
	glTranslatef( tmpBall->GetX(), tmpBall->GetY(), tmpBall->GetZ() );
	glutSolidSphere( BALL_R/4, 3, 3 );
	glPopMatrix();
      } else if ( (t2%5) == (t1%5) ){
	glColor4f(0.8, 0.8, 0.8, 0.0);
	if ( isLighting ) {
	  const static GLfloat mat_white[] = { 0.8, 0.8, 0.8, 0.0 };
	  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_white);
	}
	glPushMatrix();
	glTranslatef( tmpBall->GetX(), tmpBall->GetY(), tmpBall->GetZ() );
	glutSolidSphere( BALL_R/8, 3, 3 );
	glPopMatrix();
      }
    }
    delete tmpBall;
  }

  return true;
}

bool
BallView::RedrawAlpha() {
  // Score

  if ( mode == MODE_PLAY || mode == MODE_DEMO ){
    glPushMatrix();
    glTranslatef( -TABLEWIDTH/2-0.3, 0, TABLEHEIGHT );

    glEnable(GL_TEXTURE_2D);
    glColor3f( 0.0, 0.0, 0.0 );

    if ( theBall.GetScore(thePlayer) < 10 ) {
      glBindTexture(GL_TEXTURE_2D, m_number[theBall.GetScore(thePlayer)] );
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, -0.4, 0.0 );
      glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, -0.4, 0.2 );
      glTexCoord2f(1.0, 0.0); glVertex3f( 0.0, -0.2, 0.2 );
      glTexCoord2f(1.0, 1.0); glVertex3f( 0.0, -0.2, 0.0 );
      glEnd();
    } else {	/* Y2K :-) */
      glBindTexture(GL_TEXTURE_2D, m_number[theBall.GetScore(thePlayer)/10] );
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, -0.4, 0.0 );
      glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, -0.4, 0.2 );
      glTexCoord2f(1.0, 0.0); glVertex3f( 0.0, -0.3, 0.2 );
      glTexCoord2f(1.0, 1.0); glVertex3f( 0.0, -0.3, 0.0 );
      glEnd();
      glBindTexture(GL_TEXTURE_2D, m_number[theBall.GetScore(thePlayer)%10] );
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, -0.3, 0.0 );
      glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, -0.3, 0.2 );
      glTexCoord2f(1.0, 0.0); glVertex3f( 0.0, -0.2, 0.2 );
      glTexCoord2f(1.0, 1.0); glVertex3f( 0.0, -0.2, 0.0 );
      glEnd();
    }

    if ( theBall.GetScore(comPlayer) < 10 ) {
      glBindTexture(GL_TEXTURE_2D, m_number[theBall.GetScore(comPlayer)] );
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, 0.2, 0.0 );
      glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, 0.2, 0.2 );
      glTexCoord2f(1.0, 0.0); glVertex3f( 0.0, 0.4, 0.2 );
      glTexCoord2f(1.0, 1.0); glVertex3f( 0.0, 0.4, 0.0 );
      glEnd();
    } else {	/* Y2K :-) */
      glBindTexture(GL_TEXTURE_2D, m_number[theBall.GetScore(comPlayer)/10] );
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, 0.2, 0.0 );
      glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, 0.2, 0.2 );
      glTexCoord2f(1.0, 0.0); glVertex3f( 0.0, 0.3, 0.2 );
      glTexCoord2f(1.0, 1.0); glVertex3f( 0.0, 0.3, 0.0 );
      glEnd();
      glBindTexture(GL_TEXTURE_2D, m_number[theBall.GetScore(comPlayer)%10] );
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex3f( 0.0, 0.3, 0.0 );
      glTexCoord2f(0.0, 0.0); glVertex3f( 0.0, 0.3, 0.2 );
      glTexCoord2f(1.0, 0.0); glVertex3f( 0.0, 0.4, 0.2 );
      glTexCoord2f(1.0, 1.0); glVertex3f( 0.0, 0.4, 0.0 );
      glEnd();
    }

    glDisable(GL_TEXTURE_2D);

    glPopMatrix();
  }
}
