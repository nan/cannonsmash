/* $Id$ */

// Copyright (C) 2000, 2001, 2002  $B?@Fn(B $B5H9((B(Kanna Yoshihiro)
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
#include "RCFile.h"

extern RCFile *theRC;

extern Ball   theBall;
extern long mode;

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
    for ( int j = 0 ; j < Image.GetWidth() ; j++ ) {
      for ( int k = 0 ; k < Image.GetHeight() ; k++ ) {
	if ( Image.GetPixel( j, k, 0 ) >= 5 )
	  Image.SetPixel( j, k, 3 , 255 );
	else
	  Image.SetPixel( j, k, 3 , 0 );
      }
    }

    glBindTexture( GL_TEXTURE_2D, m_number[i] );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
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
  const static GLfloat mat_yel[] = { 1.0F, 0.8F, 0.0F, 0.0F };

  // Draw the Ball itself
  glColor4f(1.0F, 0.8F, 0.0F, 0.0F);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_yel);

  glPushMatrix();
    glTranslatef( (float)theBall.GetX(),
		  (float)theBall.GetY(),
		  (float)theBall.GetZ() );

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

  return true;
}

bool
BallView::RedrawAlpha() {
  Ball* tmpBall;
  const static GLfloat mat_yel[] = { 1.0F, 0.8F, 0.0F, 0.0F };
  static double angle = 0.0;

  // Draw the ball location in the future
  if ( Control::TheControl()->GetThePlayer() &&
       (((theBall.GetStatus() == 2 || theBall.GetStatus() == 3) &&
	Control::TheControl()->GetThePlayer()->GetSide() > 0) ||
       ((theBall.GetStatus() == 0 || theBall.GetStatus() == 1) &&
	Control::TheControl()->GetThePlayer()->GetSide() < 0)) ){
    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    long t1 = 0, t2 = 0;
    double t1x;

    // get time until the ball reaches hit point
    while ( tmpBall->GetStatus() != -1 ){
      tmpBall->Move();
      if ( tmpBall->GetY() < Control::TheControl()->GetThePlayer()->GetY() &&
	   tmpBall->GetStatus() == 3 )
	break;
      t1++;
    }
    if ( tmpBall->GetStatus() == -1 )
      t1 += 100000;	// Not red ball

    t1x = tmpBall->GetX();

    delete tmpBall;

    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    glColor4f(0.8F, 0.8F, 0.8F, 1.0F);
    const static GLfloat mat_white[] = { 0.8F, 0.8F, 0.8F, 0.0F };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_white);

    float prev[3], diff[3];

    prev[0] = (float)tmpBall->GetX();
    prev[1] = (float)tmpBall->GetY();
    prev[2] = (float)tmpBall->GetZ();

    while ( tmpBall->GetStatus() >= 0 && t1-10 > t2 ) {
      tmpBall->Move();
      t2++;

      glPushMatrix();
        glTranslatef( prev[0], prev[1], prev[2] );
	diff[0] = (float)tmpBall->GetX()-prev[0];
	diff[1] = (float)tmpBall->GetY()-prev[1];
	diff[2] = (float)tmpBall->GetZ()-prev[2];

	glBegin(GL_QUADS);
	for ( double rad = 0.0 ; rad < 3.141592*2 ; rad += 3.141592/3 ) {
	  glVertex3f( BALL_R/2*cos(rad), 0.0F, BALL_R/2*sin(rad) );
	  glVertex3f( BALL_R/2*cos(rad+3.141592/3), 0.0F, 
		      BALL_R/2*sin(rad+3.141592/3) );
	  glVertex3f( diff[0]+BALL_R/2*cos(rad+3.141592/3),
		      diff[1], diff[2]+BALL_R/2*sin(rad+3.141592/3) );
	  glVertex3f( diff[0]+BALL_R/2*cos(rad),
		      diff[1], diff[2]+BALL_R/2*sin(rad) );
	}
	glEnd();
      glPopMatrix();

      prev[0] = (float)tmpBall->GetX();
      prev[1] = (float)tmpBall->GetY();
      prev[2] = (float)tmpBall->GetZ();
    }

    if ( t1-10 == t2 ) {
      glColor4f(1.0F, 0.0F, 0.0F, 1.0F);
      const static GLfloat mat_red[] = { 1.0F, 0.0F, 0.0F, 1.0F };
      glMaterialfv(GL_FRONT, GL_SPECULAR, mat_red);
      glPushMatrix();
        glTranslatef( (float)tmpBall->GetX(),
		      (float)tmpBall->GetY(),
		      (float)tmpBall->GetZ() );
	gluQuadricDrawStyle( m_quad, (GLenum)GLU_FILL );
	gluQuadricNormals( m_quad, (GLenum)GLU_SMOOTH );
	gluSphere( m_quad, BALL_R, 12, 12 );

      glPopMatrix();

      double px = Control::TheControl()->GetThePlayer()->GetX()
	+ (tmpBall->GetX()-t1x);
      angle++;

      glPushMatrix();
        glTranslatef( px+0.3,
		      (float)tmpBall->GetY(),
		      (float)tmpBall->GetZ() );
	glBegin(GL_LINES);
          glVertex3f( -0.25, 0, 0 );
	  glVertex3f(  0.25, 0, 0 );

          glVertex3f( -0.20, 0, -0.01 );
          glVertex3f( -0.20, 0,  0.01 );
          glVertex3f( -0.15, 0, -0.01 );
          glVertex3f( -0.15, 0,  0.01 );
          glVertex3f( -0.10, 0, -0.01 );
          glVertex3f( -0.10, 0,  0.01 );
          glVertex3f( -0.05, 0, -0.01 );
          glVertex3f( -0.05, 0,  0.01 );
          glVertex3f(  0.00, 0, -0.01 );
          glVertex3f(  0.00, 0,  0.01 );
          glVertex3f(  0.05, 0, -0.01 );
          glVertex3f(  0.05, 0,  0.01 );
          glVertex3f(  0.10, 0, -0.01 );
          glVertex3f(  0.10, 0,  0.01 );
          glVertex3f(  0.15, 0, -0.01 );
          glVertex3f(  0.15, 0,  0.01 );
          glVertex3f(  0.20, 0, -0.01 );
          glVertex3f(  0.20, 0,  0.01 );

	  glVertex3f( 0, 0, -0.25 );
	  glVertex3f( 0, 0,  0.25 );

          glVertex3f( -0.01, 0, -0.20 );
          glVertex3f(  0.01, 0, -0.20 );
          glVertex3f( -0.01, 0, -0.15 );
          glVertex3f(  0.01, 0, -0.15 );
          glVertex3f( -0.01, 0, -0.10 );
          glVertex3f(  0.01, 0, -0.10 );
          glVertex3f( -0.01, 0, -0.05 );
          glVertex3f(  0.01, 0, -0.05 );
          glVertex3f( -0.01, 0,  0.00 );
          glVertex3f(  0.01, 0,  0.00 );
          glVertex3f( -0.01, 0,  0.05 );
          glVertex3f(  0.01, 0,  0.05 );
          glVertex3f( -0.01, 0,  0.10 );
          glVertex3f(  0.01, 0,  0.10 );
          glVertex3f( -0.01, 0,  0.15 );
          glVertex3f(  0.01, 0,  0.15 );
          glVertex3f( -0.01, 0,  0.20 );
          glVertex3f(  0.01, 0,  0.20 );
	glEnd();

	glRotatef( 90.0, 1.0, 0.0, 0.0 );
	gluDisk( m_quad, 0.19, 0.20, 18, 1 );
	gluPartialDisk( m_quad, 0.175, 0.18, 3, 1, angle, 60.0 );
	gluPartialDisk( m_quad, 0.175, 0.18, 3, 1, angle+120, 60.0 );
	gluPartialDisk( m_quad, 0.175, 0.18, 3, 1, angle+240, 60.0 );
      glPopMatrix();

      glPushMatrix();
        glTranslatef( px-0.3,
		      (float)tmpBall->GetY(),
		      (float)tmpBall->GetZ() );
	glBegin(GL_LINES);
          glVertex3f( -0.25, 0, 0 );
	  glVertex3f(  0.25, 0, 0 );

          glVertex3f( -0.20, 0, -0.01 );
          glVertex3f( -0.20, 0,  0.01 );
          glVertex3f( -0.15, 0, -0.01 );
          glVertex3f( -0.15, 0,  0.01 );
          glVertex3f( -0.10, 0, -0.01 );
          glVertex3f( -0.10, 0,  0.01 );
          glVertex3f( -0.05, 0, -0.01 );
          glVertex3f( -0.05, 0,  0.01 );
          glVertex3f(  0.00, 0, -0.01 );
          glVertex3f(  0.00, 0,  0.01 );
          glVertex3f(  0.05, 0, -0.01 );
          glVertex3f(  0.05, 0,  0.01 );
          glVertex3f(  0.10, 0, -0.01 );
          glVertex3f(  0.10, 0,  0.01 );
          glVertex3f(  0.15, 0, -0.01 );
          glVertex3f(  0.15, 0,  0.01 );
          glVertex3f(  0.20, 0, -0.01 );
          glVertex3f(  0.20, 0,  0.01 );

	  glVertex3f( 0, 0, -0.25 );
	  glVertex3f( 0, 0,  0.25 );

          glVertex3f( -0.01, 0, -0.20 );
          glVertex3f(  0.01, 0, -0.20 );
          glVertex3f( -0.01, 0, -0.15 );
          glVertex3f(  0.01, 0, -0.15 );
          glVertex3f( -0.01, 0, -0.10 );
          glVertex3f(  0.01, 0, -0.10 );
          glVertex3f( -0.01, 0, -0.05 );
          glVertex3f(  0.01, 0, -0.05 );
          glVertex3f( -0.01, 0,  0.00 );
          glVertex3f(  0.01, 0,  0.00 );
          glVertex3f( -0.01, 0,  0.05 );
          glVertex3f(  0.01, 0,  0.05 );
          glVertex3f( -0.01, 0,  0.10 );
          glVertex3f(  0.01, 0,  0.10 );
          glVertex3f( -0.01, 0,  0.15 );
          glVertex3f(  0.01, 0,  0.15 );
          glVertex3f( -0.01, 0,  0.20 );
          glVertex3f(  0.01, 0,  0.20 );
	glEnd();

	glRotatef( 90.0, 1.0, 0.0, 0.0 );
	gluDisk( m_quad, 0.19, 0.20, 18, 1 );
	gluPartialDisk( m_quad, 0.175, 0.18, 3, 1, angle, 60.0 );
	gluPartialDisk( m_quad, 0.175, 0.18, 3, 1, angle+120, 60.0 );
	gluPartialDisk( m_quad, 0.175, 0.18, 3, 1, angle+240, 60.0 );
      glPopMatrix();

    }

    glColor4f(0.8F, 0.8F, 0.8F, 1.0F);
    glBegin(GL_LINE_STRIP);
    while ( tmpBall->GetStatus() >= 0 ) {
      tmpBall->Move();

      glVertex3f( (float)tmpBall->GetX(), (float)tmpBall->GetY(),
		  (float)tmpBall->GetZ() );
    }
    glEnd();
      
    delete tmpBall;
  }

  // Score
  if ( mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY || mode == MODE_PRACTICE ){
    glPushMatrix();
    glTranslatef( -TABLEWIDTH/2-0.3F, 0, TABLEHEIGHT );

    if ( theRC->isTexture || theBall.GetStatus() < -10 ) {
      long score1, score2;
      long game1, game2;

      glEnable(GL_TEXTURE_2D);
      glColor3f( 0.0F, 0.0F, 0.0F );

      score1 = ((PlayGame *)Control::TheControl())->GetScore(1);
      score2 = ((PlayGame *)Control::TheControl())->GetScore(-1);

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
