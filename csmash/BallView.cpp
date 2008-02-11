/**
 * @file
 * @brief Implementation of BallView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004, 2007  Kanna Yoshihiro
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

/**
 * Default constructor. 
 */
BallView::BallView() {
}

/**
 * Destructor. 
 * Do nothing. 
 */
BallView::~BallView() {
}

/**
 * Initialize BallView. 
 * Load texture and initialize glu. 
 * 
 * @return returns true if succeeds. 
 */
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

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		 Image.GetWidth(), Image.GetHeight(), 
		 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 Image.GetImage() );
  }

  m_quad = gluNewQuadric();
  gluQuadricDrawStyle( m_quad, (GLenum)GLU_FILL );
  gluQuadricNormals( m_quad, (GLenum)GLU_SMOOTH );

  return true;
}
 
/**
 * Redraw ball and related objects. 
 * 
 * @return returns true if succeeds. 
 */
bool
BallView::Redraw() {
  DrawBall();
  DrawShadow();

  return true;
}

/**
 * Redraw transparent objects. 
 * Redraw ball location indicator, hit point, score. 
 * 
 * @return returns true if succeeds. 
 */
bool
BallView::RedrawAlpha() {
  Ball* tmpBall;
  static vector3d tx = vector3d(0.0, 0.0, TABLEHEIGHT+NETHEIGHT);

  Player *thePlayer = Control::TheControl()->GetThePlayer();

  // Draw the ball location in the future
  if ( thePlayer &&
       (((theBall.GetStatus() == 2 || theBall.GetStatus() == 3) &&
	thePlayer->GetSide() > 0) ||
       ((theBall.GetStatus() == 0 || theBall.GetStatus() == 1) &&
	thePlayer->GetSide() < 0)) ){
    tmpBall = new Ball( &theBall );

    long t1 = 0, t2 = 0;
    double t1x;

    // get time until the ball reaches hit point
    while ( tmpBall->GetStatus() != -1 ){
      tmpBall->Move();
      if ( (thePlayer->GetSide() > 0 && tmpBall->GetX()[1] < thePlayer->GetX()[1]
	   && tmpBall->GetStatus() == 3) ||
	   (thePlayer->GetSide() < 0 && tmpBall->GetX()[1] > thePlayer->GetX()[1]
	   && tmpBall->GetStatus() == 1) )
	break;
      t1++;
    }
    if ( tmpBall->GetStatus() == -1 )
      t1 += 100000;	// Not red ball

    t1x = tmpBall->GetX()[0];

    delete tmpBall;

    tmpBall = new Ball( &theBall );

    static GLfloat line_spc[] = { 1.0F, 1.0F, 1.0F, 1.0F };
    static GLfloat line_dif[] = { 1.0F, 1.0F, 1.0F, 1.0F };
    static GLfloat line_amb[] = { 1.0F, 1.0F, 1.0F, 1.0F };
    static GLfloat line_shininess[] = { 5.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, line_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, line_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, line_shininess);

    vector3d prev, diff;

    prev = tmpBall->GetX();

    while ( tmpBall->GetStatus() >= 0 && t1-10 > t2 ) {
      tmpBall->Move();
      t2++;

      glPushMatrix();
        glTranslated( prev[0], prev[1], prev[2] );

	diff = tmpBall->GetX()-prev;

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

      prev = tmpBall->GetX();
    }

    if ( t1-10 == t2 ) {
      static GLfloat red_spc[] = { 1.0F, 0.0F, 0.0F, 1.0F };
      static GLfloat red_dif[] = { 1.0F, 0.0F, 0.0F, 1.0F };
      static GLfloat red_amb[] = { 1.0F, 0.0F, 0.0F, 1.0F };
      static GLfloat red_shininess[] = { 5.0 };
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, red_spc);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, red_dif);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, red_amb);
      glMaterialfv(GL_FRONT, GL_SHININESS, red_shininess);

      glPushMatrix();
        glTranslatef( (float)tmpBall->GetX()[0],
		      (float)tmpBall->GetX()[1],
		      (float)tmpBall->GetX()[2] );
	gluQuadricDrawStyle( m_quad, (GLenum)GLU_FILL );
	gluQuadricNormals( m_quad, (GLenum)GLU_SMOOTH );
	gluSphere( m_quad, BALL_R, 12, 12 );

      glPopMatrix();

      vector3d px = thePlayer->GetX();
      px[0] = t1x;
      tx = tmpBall->GetX() - px;

      glPushMatrix();
        glTranslatef( tx[0]+thePlayer->GetX()[0]+0.3,
		      tx[1]+thePlayer->GetX()[1],
		      tx[2]+thePlayer->GetX()[2] );
	DrawTargetCircle();
      glPopMatrix();

      glPushMatrix();
        glTranslatef( tx[0]+thePlayer->GetX()[0]-0.3,
		      tx[1]+thePlayer->GetX()[1],
		      tx[2]+thePlayer->GetX()[2] );
	DrawTargetCircle();
      glPopMatrix();
    } else {
      static GLfloat red_spc[] = { 1.0F, 0.0F, 0.0F, 0.5F };
      static GLfloat red_dif[] = { 1.0F, 0.0F, 0.0F, 0.5F };
      static GLfloat red_amb[] = { 1.0F, 0.0F, 0.0F, 0.5F };
      static GLfloat red_shininess[] = { 5.0 };
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, red_spc);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, red_dif);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, red_amb);
      glMaterialfv(GL_FRONT, GL_SHININESS, red_shininess);

      glPushMatrix();
        glTranslatef( tx[0]+thePlayer->GetX()[0]+0.3,
		      tx[1]+thePlayer->GetX()[1],
		      tx[2]+thePlayer->GetX()[2] );
	DrawTargetCircle();
      glPopMatrix();

      glPushMatrix();
        glTranslatef( tx[0]+thePlayer->GetX()[0]-0.3,
		      tx[1]+thePlayer->GetX()[1],
		      tx[2]+thePlayer->GetX()[2] );
	DrawTargetCircle();
      glPopMatrix();
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, line_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, line_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, line_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, line_shininess);

    glBegin(GL_LINE_STRIP);
    while ( tmpBall->GetStatus() >= 0 ) {
      tmpBall->Move();

      glVertex3f( (float)tmpBall->GetX()[0], (float)tmpBall->GetX()[1],
		  (float)tmpBall->GetX()[2] );
    }
    glEnd();
      
    delete tmpBall;
  } else if ( Control::TheControl()->IsPlaying() ) {
    static GLfloat red_spc[] = { 1.0F, 0.0F, 0.0F, 0.2F };
    static GLfloat red_dif[] = { 1.0F, 0.0F, 0.0F, 0.2F };
    static GLfloat red_amb[] = { 1.0F, 0.0F, 0.0F, 0.2F };
    static GLfloat red_shininess[] = { 5.0 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, red_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, red_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, red_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, red_shininess);

    glPushMatrix();
      glTranslatef( tx[0]+thePlayer->GetX()[0]+0.3,
		    tx[1]+thePlayer->GetX()[1],
		    tx[2]+thePlayer->GetX()[2] );
      DrawTargetCircle();
    glPopMatrix();

    glPushMatrix();
      glTranslatef( tx[0]+thePlayer->GetX()[0]-0.3,
		    tx[1]+thePlayer->GetX()[1],
		    tx[2]+thePlayer->GetX()[2] );
      DrawTargetCircle();
    glPopMatrix();
  }


  // Score
  if ( mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY || mode == MODE_PRACTICE ){
    glPushMatrix();
    glTranslatef( -TABLEWIDTH/2-0.3F, 0, TABLEHEIGHT );

    if ( theRC->isTexture || theBall.GetStatus() < -10 ) {
      long score1, score2;

      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

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

      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glDisable(GL_TEXTURE_2D);
    }

    glPopMatrix();
  }

  return true;
}

/** 
 * Draw the ball itself. 
 */
void
BallView::DrawBall() {
  static GLfloat ball_spc[] = { 1.0F, 0.8F, 0.0F, 0.0F };
  static GLfloat ball_dif[] = { 1.0F, 0.8F, 0.0F, 0.0F };
  static GLfloat ball_amb[] = { 1.0F, 0.8F, 0.0F, 0.0F };
  static GLfloat ball_shininess[] = { 5.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, ball_spc);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, ball_dif);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ball_amb);
  glMaterialfv(GL_FRONT, GL_SHININESS, ball_shininess);

  glPushMatrix();
    glTranslatef( (float)theBall.GetX()[0],
		  (float)theBall.GetX()[1],
		  (float)theBall.GetX()[2] );

    gluSphere( m_quad, BALL_R, 12, 12 );
  glPopMatrix();
}

/** 
 * Draw the ball shadow. 
 */
void
BallView::DrawShadow() {
  double rad;
  float height;

  if ( theBall.GetX()[1] > -TABLELENGTH/2 && theBall.GetX()[1] < TABLELENGTH/2 )
    height = TABLEHEIGHT+0.01F;
  else
    height = 0.01F;

  static GLfloat sdw_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat sdw_dif[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat sdw_amb[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat sdw_shininess[] = { 5.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sdw_spc);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, sdw_dif);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, sdw_amb);
  glMaterialfv(GL_FRONT, GL_SHININESS, sdw_shininess);

  glBegin(GL_POLYGON);
    for ( rad = 0.0F ; rad < 3.141592F*2 ; rad += 3.141592F/4 )
      glVertex3f( (float)(theBall.GetX()[0]+BALL_R*cos(rad)),
		  (float)(theBall.GetX()[1]+BALL_R*sin(rad)),
		  height );
  glEnd();
}

/**
 * Draw target circle in front of the player. 
 */
void
BallView::DrawTargetCircle() {
  static double angle = 0.0;

  angle++;

  float radius;
  Player *p = Control::TheControl()->GetThePlayer();
  radius = (p->GetStatus()-p->StatusBorder())*3/2000.0;
  if ( radius < 0.0 )
    radius = 0.05;

  glBegin(GL_LINES);
    glVertex3f( -radius-0.05, 0, 0 );
    glVertex3f(  radius+0.05, 0, 0 );
    glVertex3f( 0, 0, -radius-0.05 );
    glVertex3f( 0, 0,  radius+0.05 );

    float r = 0.05;
    while ( r < radius ) {
      glVertex3f( -r, 0, -0.01 );
      glVertex3f( -r, 0,  0.01 );
      glVertex3f(  r, 0, -0.01 );
      glVertex3f(  r, 0,  0.01 );
      glVertex3f( -0.01, 0, -r );
      glVertex3f(  0.01, 0, -r );
      glVertex3f( -0.01, 0,  r );
      glVertex3f(  0.01, 0,  r );

      r += 0.05;
    }
  glEnd();

  glRotatef( 90.0, 1.0, 0.0, 0.0 );
  gluDisk( m_quad, radius-0.01, radius, 18, 1 );
  gluPartialDisk( m_quad, radius-0.025, radius-0.02, 3, 1, angle, 60.0 );
  gluPartialDisk( m_quad, radius-0.025, radius-0.02, 3, 1, angle+120, 60.0 );
  gluPartialDisk( m_quad, radius-0.025, radius-0.02, 3, 1, angle+240, 60.0 );
}
