/* $Id$ */

// Copyright (C) 2000, 2001  神南 吉宏(Kanna Yoshihiro)
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
#include "BallView.h"
#include "Ball.h"
#include "Player.h"
#include "Control.h"
#include "LoadImage.h"
#include "PlayGame.h"

extern Ball   theBall;
extern Player* thePlayer;
extern Player *comPlayer;
extern long mode;

extern Control* theControl;

extern bool isLighting;
extern bool isTexture;

GLuint BallView::m_number[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

BallView::BallView() {
}

BallView::~BallView() {
}

bool
BallView::Init() {
  ImageData Image;
  static char num[][20] = {"images/zero.ppm", "images/one.ppm",
			   "images/two.ppm", "images/three.ppm",
			   "images/four.ppm", "images/five.ppm",
			   "images/six.ppm", "images/seven.ppm",
			   "images/eight.ppm", "images/nine.ppm"};

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures( 10, m_number );

  for ( int i = 0 ; i < 10 ; i++ ) {
    Image.LoadPPM( &num[i][0] );
    glBindTexture( GL_TEXTURE_2D, m_number[i] );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, 3,
		 Image.GetWidth(), Image.GetHeight(), 
		 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 Image.GetImage() );
  }

  m_quad = gluNewQuadric();

  return true;
}
 
bool
BallView::Redraw() {
  double rad;
  Ball* tmpBall;
  const static GLfloat mat_yel[] = { 1.0F, 0.8F, 0.0F, 0.0F };

  // Draw the Ball itself
#if 1
  glColor4f(1.0F, 0.8F, 0.0F, 0.0F);
#else
  if ( theBall.GetSpin() > 0.0F )
    glColor4f(1.0F, 0.8F-theBall.GetSpin()*0.8F, 0.0F, 0.0F);
  else
    glColor4f(1.0F+theBall.GetSpin(), 0.8F+theBall.GetSpin()*0.8F,
	      -theBall.GetSpin(), 0.0F);
#endif
  if ( isLighting ) {
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_yel);
  }

  glPushMatrix();
    glTranslatef( (float)theBall.GetX(), (float)theBall.GetY(), (float)theBall.GetZ() );

    gluQuadricDrawStyle( m_quad, (GLenum)GLU_FILL );
    gluQuadricNormals( m_quad, (GLenum)GLU_SMOOTH );
    gluSphere( m_quad, BALL_R, 12, 12 );
  glPopMatrix();

  // Draw the ball shadow
  glColor4f(0.0, 0.0, 0.0, 0.0);
  if ( theBall.GetY() > -TABLELENGTH/2 && theBall.GetY() < TABLELENGTH/2 ){
    glBegin(GL_POLYGON);
      for ( rad = 0.0F ; rad < 3.141592F*2 ; rad += 3.141592F/4 )
	glVertex3f( (float)(theBall.GetX()+BALL_R*cos(rad)),
		    (float)(theBall.GetY()+BALL_R*sin(rad)),
		    TABLEHEIGHT+0.01F );
    glEnd();
  } else {
    glBegin(GL_POLYGON);
      for ( rad = 0.0F ; rad < 3.141592F*2 ; rad += 3.141592F/4 )
	glVertex3f( (float)(theBall.GetX()+BALL_R*cos(rad)),
		    (float)(theBall.GetY()+BALL_R*sin(rad)),
		    0.01F );
    glEnd();
  }

  // Draw the ball location in the future
  if ( thePlayer &&
       (((theBall.GetStatus() == 2 || theBall.GetStatus() == 3) &&
	thePlayer->GetSide() > 0) ||
       ((theBall.GetStatus() == 0 || theBall.GetStatus() == 1) &&
	thePlayer->GetSide() < 0)) ){
    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );
    long t1 = 0, t2 = 0;
    // get time until the ball reaches hit point
    while ( tmpBall->GetStatus() != -1 ){
      tmpBall->Move();
      if ( tmpBall->GetY() < thePlayer->GetY() && tmpBall->GetStatus() == 3 )
	break;
      t1++;
    }
    if ( tmpBall->GetStatus() == -1 )
      t1 += 100000;	// Not red ball

    delete tmpBall;

    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    while ( tmpBall->GetStatus() != -1 ){
      tmpBall->Move();
      t2++;
      if ( t1-10 == t2 ){
	glColor4f(1.0F, 0.0F, 0.0F, 0.0F);
	if ( isLighting ) {
	  const static GLfloat mat_red[] = { 1.0F, 0.0F, 0.0F, 0.0F };
	  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_red);
	}
	glPushMatrix();
	glTranslatef( (float)tmpBall->GetX(), (float)tmpBall->GetY(), (float)tmpBall->GetZ() );

	gluQuadricDrawStyle( m_quad, (GLenum)GLU_FILL );
	gluQuadricNormals( m_quad, (GLenum)GLU_SMOOTH );
	gluSphere( m_quad, BALL_R/2, 3, 3 );

	glPopMatrix();
      } else if ( t1 == t2 ){
	glColor4f(1.0F, 0.8F, 0.0F, 0.0F);
	if ( isLighting )
	  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_yel);
	glPushMatrix();
	glTranslatef( (float)tmpBall->GetX(), (float)tmpBall->GetY(), (float)tmpBall->GetZ() );

	gluQuadricDrawStyle( m_quad, (GLenum)GLU_FILL );
	gluQuadricNormals( m_quad, (GLenum)GLU_SMOOTH );
	gluSphere( m_quad, BALL_R/4, 3, 3 );

	glPopMatrix();
      } else if ( (t2%5) == (t1%5) ){
	glColor4f(0.8F, 0.8F, 0.8F, 0.0F);
	if ( isLighting ) {
	  const static GLfloat mat_white[] = { 0.8F, 0.8F, 0.8F, 0.0F };
	  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_white);
	}
	glPushMatrix();
	glTranslatef( (float)tmpBall->GetX(), (float)tmpBall->GetY(), (float)tmpBall->GetZ() );

	gluQuadricDrawStyle( m_quad, GLU_FILL );
	gluQuadricNormals( m_quad, GLU_SMOOTH );
	gluSphere( m_quad, BALL_R/8, 3, 3 );

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
  if ( mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY ){
    glPushMatrix();
    glTranslatef( -TABLEWIDTH/2-0.3F, 0, TABLEHEIGHT );

    if ( isTexture || theBall.GetStatus() < -10 ) {
      long score1, score2;

      glEnable(GL_TEXTURE_2D);
      glColor3f( 0.0F, 0.0F, 0.0F );

      score1 = ((PlayGame *)theControl)->GetScore(1);
      score2 = ((PlayGame *)theControl)->GetScore(-1);

      if ( score1 < 10 ) {
	glBindTexture(GL_TEXTURE_2D, m_number[score1] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0F, 1.0F); glVertex3f( 0.0F, -0.4F, 0.0F );
	glTexCoord2f(0.0F, 0.0F); glVertex3f( 0.0F, -0.4F, 0.2F );
	glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.0F, -0.2F, 0.2F );
	glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.0F, -0.2F, 0.0F );
	glEnd();
      } else {	/* Y2K :-) */
	glBindTexture(GL_TEXTURE_2D, m_number[score1/10] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0F, 1.0F); glVertex3f( 0.0F, -0.4F, 0.0F );
	glTexCoord2f(0.0F, 0.0F); glVertex3f( 0.0F, -0.4F, 0.2F );
	glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.0F, -0.3F, 0.2F );
	glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.0F, -0.3F, 0.0F );
	glEnd();
	glBindTexture(GL_TEXTURE_2D, m_number[score1%10] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0F, 1.0F); glVertex3f( 0.0F, -0.3F, 0.0F );
	glTexCoord2f(0.0F, 0.0F); glVertex3f( 0.0F, -0.3F, 0.2F );
	glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.0F, -0.2F, 0.2F );
	glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.0F, -0.2F, 0.0F );
	glEnd();
      }

      if ( score2 < 10 ) {
	glBindTexture(GL_TEXTURE_2D, m_number[score2] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0F, 1.0F); glVertex3f( 0.0F, 0.2F, 0.0F );
	glTexCoord2f(0.0F, 0.0F); glVertex3f( 0.0F, 0.2F, 0.2F );
	glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.0F, 0.4F, 0.2F );
	glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.0F, 0.4F, 0.0F );
	glEnd();
      } else {	/* Y2K :-) */
	glBindTexture(GL_TEXTURE_2D, m_number[score2/10] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0F, 1.0F); glVertex3f( 0.0F, 0.2F, 0.0F );
	glTexCoord2f(0.0F, 0.0F); glVertex3f( 0.0F, 0.2F, 0.2F );
	glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.0F, 0.3F, 0.2F );
	glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.0F, 0.3F, 0.0F );
	glEnd();
	glBindTexture(GL_TEXTURE_2D, m_number[score2%10] );
	glBegin(GL_QUADS);
	glTexCoord2f(0.0F, 1.0F); glVertex3f( 0.0F, 0.3F, 0.0F );
	glTexCoord2f(0.0F, 0.0F); glVertex3f( 0.0F, 0.3F, 0.2F );
	glTexCoord2f(1.0F, 0.0F); glVertex3f( 0.0F, 0.4F, 0.2F );
	glTexCoord2f(1.0F, 1.0F); glVertex3f( 0.0F, 0.4F, 0.0F );
	glEnd();
      }

      glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
  }

  return true;
}
