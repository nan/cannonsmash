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
#include "TrainingView.h"
#include "Training.h"
#include "LoadImage.h"
#include "Ball.h"
#include "BallView.h"

extern Ball theBall;

TrainingView::TrainingView() {
}

TrainingView::~TrainingView() {
}

bool
TrainingView::Init( Training *training ) {
  static char max[20] = "images/Max.ppm";
  ImageData Image;

  m_training = training;

  glGenTextures( 1, &m_max );
  Image.LoadPPM( max );
  glBindTexture( GL_TEXTURE_2D, m_max );
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  glTexImage2D(GL_TEXTURE_2D, 0, 3,
	       Image.GetWidth(), Image.GetHeight(), 
	       0, GL_RGBA, GL_UNSIGNED_BYTE, Image.GetImage() );

  return true;
}

bool
TrainingView::Redraw() {
  return true;
}

bool
TrainingView::RedrawAlpha() {
  glPushMatrix();
  glTranslatef( TABLEWIDTH/2-0.3, 0, TABLEHEIGHT+NETHEIGHT );

  glEnable(GL_TEXTURE_2D);
  glColor3f( 0.0, 0.0, 0.0 );

  glBindTexture(GL_TEXTURE_2D, m_max );
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 80.0/256.0); glVertex3f(-0.2, 0.0, 0.2 );
  glTexCoord2f(0.0,        0.0); glVertex3f(-0.2, 0.0, 0.4 );
  glTexCoord2f(1.0,        0.0); glVertex3f( 0.2, 0.0, 0.4 );
  glTexCoord2f(1.0, 80.0/256.0); glVertex3f( 0.2, 0.0, 0.2 );
  glEnd();

  if ( m_training->GetTrainingCount() < 10 ) {
    glBindTexture(GL_TEXTURE_2D, theBall.GetView()->m_number[m_training->GetTrainingCount()] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f( 0.2, 0.0, 0.0 );
    glTexCoord2f(0.0, 0.0); glVertex3f( 0.2, 0.0, 0.2 );
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.4, 0.0, 0.2 );
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.4, 0.0, 0.0 );
    glEnd();
  } else {	/* Y2K :-) */
    glBindTexture(GL_TEXTURE_2D, theBall.GetView()->m_number[(m_training->GetTrainingCount()/10)%10] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f( 0.2, 0.0, 0.0 );
    glTexCoord2f(0.0, 0.0); glVertex3f( 0.2, 0.0, 0.2 );
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.3, 0.0, 0.2 );
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.3, 0.0, 0.0 );
    glEnd();
    glBindTexture(GL_TEXTURE_2D, theBall.GetView()->m_number[m_training->GetTrainingCount()%10] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f( 0.3, 0.0, 0.0 );
    glTexCoord2f(0.0, 0.0); glVertex3f( 0.3, 0.0, 0.2 );
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.4, 0.0, 0.2 );
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.4, 0.0, 0.0 );
    glEnd();
  }
  if ( m_training->GetTrainingMax() < 10 ) {
    glBindTexture(GL_TEXTURE_2D,
		  theBall.GetView()->m_number[m_training->GetTrainingMax()] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f( 0.2, 0.0, 0.2 );
    glTexCoord2f(0.0, 0.0); glVertex3f( 0.2, 0.0, 0.4 );
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.4, 0.0, 0.4 );
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.4, 0.0, 0.2 );
    glEnd();
  } else {	/* Y2K :-) */
    glBindTexture(GL_TEXTURE_2D,
		  theBall.GetView()->m_number[(m_training->GetTrainingMax()/10)%10] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f( 0.2, 0.0, 0.2 );
    glTexCoord2f(0.0, 0.0); glVertex3f( 0.2, 0.0, 0.4 );
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.3, 0.0, 0.4 );
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.3, 0.0, 0.2 );
    glEnd();
    glBindTexture(GL_TEXTURE_2D,
		  theBall.GetView()->m_number[m_training->GetTrainingMax()%10] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0); glVertex3f( 0.3, 0.0, 0.2 );
    glTexCoord2f(0.0, 0.0); glVertex3f( 0.3, 0.0, 0.4 );
    glTexCoord2f(1.0, 0.0); glVertex3f( 0.4, 0.0, 0.4 );
    glTexCoord2f(1.0, 1.0); glVertex3f( 0.4, 0.0, 0.2 );
    glEnd();
  }

  glDisable(GL_TEXTURE_2D);
  glPopMatrix();

  return true;
}
