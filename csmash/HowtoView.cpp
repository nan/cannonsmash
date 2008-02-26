/**
 * @file
 * @brief Implementation of HowtoView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2002, 2007  Kanna Yoshihiro
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
#include "HowtoView.h"
#include "Howto.h"
#include "BaseView.h"
#include "RCFile.h"

extern RCFile *theRC;

GLuint HowtoView::m_textures[4] = {0, 0, 0, 0};

/**
 * Default constructor. 
 * Do nothing. 
 */
HowtoView::HowtoView() {
}

/**
 * Destructor. 
 * Do nothing. 
 */
HowtoView::~HowtoView() {
}

/**
 * Initializer method. 
 * This method loads all textures for tutorial. 
 * 
 * @param howto Howto object of which HowtoView is attached to. 
 * @return returns true if succeeds. 
 */
bool
HowtoView::Init( Howto *howto ) {
  ImageData image;
  int i, j, k;

  static char fname[][30] = {"images/Mouse1.jpg", "images/Mouse2.jpg",
			     "images/Mouse3.jpg", "images/Mouse4.jpg"};
  static char arrowname[][30] = {"images/rightArrow", "images/downArrow",
				 "images/leftArrow", "images/upArrow"};
  static char howtoText[][30] = {"images/Howto1", "images/Howto2", 
				 "images/Howto3", "images/Howto4", 
				 "images/Howto5", "images/Howto6"};
  char filename[256];

  m_howto = howto;

  if ( m_textures[0] == 0 ) {
    glGenTextures( 4, m_textures );

    for ( i = 0 ; i < 4 ; i++ ){
      image.LoadFile( &(fname[i][0]) );
      for ( j = 0 ; j < image.GetWidth() ; j++ ) {
	for ( k = 0 ; k < image.GetHeight() ; k++ ) {
	  if ( image.GetPixel( j, k, 0 ) >= 5 ||
	       image.GetPixel( j, k, 1 ) >= 5 ||
	       image.GetPixel( j, k, 2 ) >= 5 )
	    image.SetPixel( j, k, 3 , 255 );
	  else
	    image.SetPixel( j, k, 3 , 0 );
	}
      }

      glBindTexture(GL_TEXTURE_2D, m_textures[i] );
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      glTexImage2D(GL_TEXTURE_2D, 0, 4, image.GetWidth(), image.GetHeight(), 
		   0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );
    }
  }

  for ( i = 0 ; i < 6 ; i++ ) {
    sprintf( filename, _("%s.pbm"), howtoText[i] );
    m_howtoText[i].LoadFile(filename);
  }

  for ( i = 0 ; i < 4 ; i++ ) {
    sprintf( filename, _("%s.pbm"), arrowname[i] );
    m_arrow[i].LoadFile(filename);
  }

  glGenTextures( 1, m_keyboard );
  image.LoadPPM( "images/Keyboard.ppm" );
  for ( j = 0 ; j < image.GetWidth() ; j++ ) {
    for ( k = 0 ; k < image.GetHeight() ; k++ ) {
      if ( image.GetPixel( j, k, 0 ) >= 50 ||
	   image.GetPixel( j, k, 1 ) >= 50 ||
	   image.GetPixel( j, k, 2 ) >= 50 )
	image.SetPixel( j, k, 3 , 255 );
      else
	image.SetPixel( j, k, 3 , 0 );
    }
  }

  glBindTexture(GL_TEXTURE_2D, m_keyboard[0]);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 4, image.GetWidth(), image.GetHeight(),
	       0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );

  return true;
}

/**
 * Redraw objects. 
 * Do nothing. 
 */
bool
HowtoView::Redraw() {
  return true;
}

/**
 * Redraw transparent objects. 
 * Referring m_mode and m_count of Howto, this method shows textures for
 * tutorial. 
 */
bool
HowtoView::RedrawAlpha() {
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLfloat)BaseView::GetWinWidth(),
	      0.0, (GLfloat)BaseView::GetWinHeight() );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);

  static GLfloat bg_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat bg_dif[] = { 0.0F, 0.0F, 0.0F, 0.3F };
  static GLfloat bg_amb[] = { 0.0F, 0.0F, 0.0F, 0.3F };
  static GLfloat bg_shininess[] = { 5.0 };
  switch ( m_howto->GetMode() ) {	// Background
  case 0:
  case 1:
  case 3:
  case 4:
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, bg_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, bg_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, bg_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, bg_shininess);

    glBegin(GL_QUADS);
    glVertex2i(             0,               0);
    glVertex2i(IMAGE_WIDTH+50,               0);
    glVertex2i(IMAGE_WIDTH+50, IMAGE_HEIGHT+50);
    glVertex2i(             0, IMAGE_HEIGHT+50);
    glEnd();

    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, m_textures[m_howto->GetMouseB()] );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);
    glVertex2i( m_howto->GetMouseX(), m_howto->GetMouseY() );
    glTexCoord2f(1.0, 1.0);
    glVertex2i( IMAGE_WIDTH+m_howto->GetMouseX(), m_howto->GetMouseY() );
    glTexCoord2f(1.0, 0.0);
    glVertex2i( IMAGE_WIDTH+m_howto->GetMouseX(),
		IMAGE_HEIGHT+m_howto->GetMouseY() );
    glTexCoord2f(0.0, 0.0);
    glVertex2i( m_howto->GetMouseX(), IMAGE_HEIGHT+m_howto->GetMouseY() );
    glEnd();
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_2D);

    break;
  case 2:
    if ( m_howto->GetCount() > 100 && m_howto->GetCount() < 1000 ) {
      static GLfloat key_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
      static GLfloat key_dif[] = { 0.0F, 0.0F, 0.0F, 1.0F };
      static GLfloat key_amb[] = { 0.0F, 0.0F, 0.0F, 1.0F };
      static GLfloat key_shininess[] = { 5.0 };
      glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, key_spc);
      glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, key_dif);
      glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, key_amb);
      glMaterialfv(GL_FRONT, GL_SHININESS, key_shininess);

      glEnable(GL_TEXTURE_2D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      glBindTexture(GL_TEXTURE_2D, m_keyboard[0] );
      glBegin(GL_QUADS);
      glTexCoord2f(0.0, 1.0); glVertex2i( 120, -180 );
      glTexCoord2f(1.0, 1.0); glVertex2i( 800, -180 );
      glTexCoord2f(1.0, 0.0); glVertex2i( 800,  300 );
      glTexCoord2f(0.0, 0.0); glVertex2i( 120,  300 );
      glEnd();
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glDisable(GL_TEXTURE_2D);
    }
    break;
  }

  static GLfloat arrow_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat arrow_dif[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat arrow_amb[] = { 5.0F, 5.0F, 5.0F, 1.0F };
  static GLfloat arrow_shininess[] = { 5.0 };
  static GLfloat push_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat push_dif[] = { 0.0F, 0.0F, 0.0F, 0.7F };
  static GLfloat push_amb[] = { 0.0F, 3.0F, 0.0F, 0.7F };
  static GLfloat push_shininess[] = { 5.0 };
  switch ( m_howto->GetMode() ) {
  case 0:
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, arrow_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, arrow_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, arrow_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, arrow_shininess);

    switch ( (m_howto->GetCount()%400)/100 ) {
    case 0:
      glRasterPos2i( IMAGE_WIDTH, IMAGE_HEIGHT/2 );
      break;
    case 1:
      glRasterPos2i( IMAGE_WIDTH/2, 0 );
      break;
    case 2:
      glRasterPos2i( 0, IMAGE_HEIGHT/2 );
      break;
    case 3:
      glRasterPos2i( IMAGE_WIDTH/2, IMAGE_HEIGHT );
      break;
    }

    glBitmap( 50, 50, 0.0, 0.0, 0.0, 0, 
	      m_arrow[(m_howto->GetCount()%400)/100].GetImage() );
    break;
  case 2:
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, push_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, push_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, push_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, push_shininess);
    if ( m_howto->GetCount() > 200 && m_howto->GetCount() < 1000 &&
	 m_howto->GetCount()%100 > 0 && m_howto->GetCount()%100 < 30 ) {
      glBegin(GL_QUADS);
      //glVertex2i( 202+m_howto->GetMouseX()*36, 297-m_howto->GetMouseY()*72 );
      //glVertex2i( 235+m_howto->GetMouseX()*36, 297-m_howto->GetMouseY()*72 );
      //glVertex2i( 235+m_howto->GetMouseX()*36, 252-m_howto->GetMouseY()*72 );
      //glVertex2i( 202+m_howto->GetMouseX()*36, 252-m_howto->GetMouseY()*72 );
      glVertex2i( 192+m_howto->GetMouseX()*27, 296-m_howto->GetMouseY()*56 );
      glVertex2i( 262+m_howto->GetMouseX()*27, 296-m_howto->GetMouseY()*56 );
      glVertex2i( 262+m_howto->GetMouseX()*27, 242-m_howto->GetMouseY()*56 );
      glVertex2i( 192+m_howto->GetMouseX()*27, 242-m_howto->GetMouseY()*56 );
      glEnd();
    }
    break;
  case 4:
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, arrow_spc);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, arrow_dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, arrow_amb);
    glMaterialfv(GL_FRONT, GL_SHININESS, arrow_shininess);
    if ( m_howto->GetCount() < 1000 )
      glRasterPos2i( 460, 170 );
    else
      glRasterPos2i( 240, 170 );

    glBitmap( 50, 50, 0.0, 0.0, 0.0, 0, m_arrow[1].GetImage() );
    break;
  }

  static GLfloat text_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat text_dif[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat text_amb[] = { 5.0F, 5.0F, 5.0F, 1.0F };
  static GLfloat text_shininess[] = { 5.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, text_spc);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, text_dif);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, text_amb);
  glMaterialfv(GL_FRONT, GL_SHININESS, text_shininess);

  if ( m_howto->GetCount()%100 < 75 ) {
    glRasterPos2i( 10, 450 );
    glBitmap( 400, 100, 0.0, 0.0, 0.0, 0,
	      m_howtoText[m_howto->GetMode()].GetImage() );
  }

  if ( theRC->gmode != GMODE_SIMPLE )
    glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  return true;
}
