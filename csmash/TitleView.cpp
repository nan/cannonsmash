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

extern long gameLevel;
extern long gameMode;

extern Sound theSound;

extern bool isWireFrame;

TitleView::TitleView() {
}

TitleView::~TitleView() {
}

bool
TitleView::Init( Title *title ) {
  int i, j, k;
  static char menu[][30] = {"images/StartGame.ppm", "images/Training.ppm",
			    "images/Howto.ppm", "images/Config.ppm",
			    "images/Quit.ppm"};
  static char configMenu[][30] = {"images/Easy.ppm", "images/Normal.ppm",
				  "images/Hard.ppm", "images/Tsuborish.ppm", 
				  "images/5point.ppm", "images/11point.ppm",
				  "images/21point.ppm",
				  "images/WireFrame.ppm", "images/Transparent.ppm" };
  static char configTitle[][30] = {"images/LevelSelect.ppm",
				   "images/ModeSelect.ppm"
//				   ,"images/SoundSelect.ppm" };
				    };

#ifndef HAVE_LIBZ
  FILE *fp;
#else
  gzFile fp;
#endif

  ImageData image;

  m_title = title;

  for ( i = 0 ; i < m_title->GetMenuNum( MENU_MAIN ) ; i++ ) {
#ifndef HAVE_LIBZ
    if( (fp = fopen(&menu[i][0], "r")) == NULL ){
      return false;
    }
#else
    if (NULL == (fp = gzopenx(&menu[i][0], "rs"))) return false;
#endif

    for ( j = 69 ; j >= 0 ; j-- ) {
      for ( k = 0 ; k < 400/8 ; k++ ) {
	m_choice[i][j*50+k] = strtol( getWord(fp), NULL, 16 );
      }
    }

#ifndef HAVE_LIBZ
    fclose(fp);
#else
    gzclose(fp);
#endif
  }

  for ( i = 0 ; i < m_title->GetMenuNum( MENU_CONFIG, MENU_ALL )-1 ; i++ ) {
#ifndef HAVE_LIBZ
    if( (fp = fopen(&configMenu[i][0], "r")) == NULL ){
      return false;
    }
#else
    if (NULL == (fp = gzopenx(&configMenu[i][0], "rs"))) return false;
#endif

    for ( j = 34 ; j >= 0 ; j-- ) {
      for ( k = 0 ; k < 200/8 ; k++ ) {
	m_configChoice[i][j*25+k] = strtol( getWord(fp), NULL, 16 );
      }
    }

#ifndef HAVE_LIBZ
    fclose(fp);
#else
    gzclose(fp);
#endif
  }

  for ( i = 0 ; i < 2 ; i++ ) {
#ifndef HAVE_LIBZ
    if( (fp = fopen(&configTitle[i][0], "r")) == NULL ){
      return false;
    }
#else
    if (NULL == (fp = gzopenx(&configTitle[i][0], "rs"))) return false;
#endif

    for ( j = 34 ; j >= 0 ; j-- ) {
      for ( k = 0 ; k < 200/8 ; k++ ) {
	m_configTitle[i][j*25+k] = strtol( getWord(fp), NULL, 16 );
      }
    }

#ifndef HAVE_LIBZ
    fclose(fp);
#else
    gzclose(fp);
#endif
  }

  return true;
}

bool
TitleView::Redraw() {
  return true;
}

bool
TitleView::RedrawAlpha() {
  int i;

  glColor4f( 1.0, 1.0, 1.0, 0.0 );

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLfloat)BaseView::GetWinWidth(),
	      0.0, (GLfloat)BaseView::GetWinHeight() );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDepthMask(0);

  switch ( m_title->GetSelectMode() ) {
  case MENU_MAIN:
    for ( i = 0 ; i < m_title->GetMenuNum( MENU_MAIN ) ; i++ ) {
      if ( m_title->GetSelected() == i )
	glColor4f( 1.0, 1.0, 0.0, 0.0 );
      else
	glColor4f( 1.0, 1.0, 1.0, 0.0 );

      glRasterPos2i( 200, 450-i*100 );
      glBitmap( 400, 70, 0.0, 0.0, 0.0, 0, &m_choice[i][0] );
    }
    glColor4f( 1.0, 1.0, 0.0, 0.0 );
    glBegin(GL_TRIANGLES);
    glVertex2i( 120, 450-m_title->GetSelected()*100 );
    glVertex2i( 180, 485-m_title->GetSelected()*100 );
    glVertex2i( 120, 520-m_title->GetSelected()*100 );
    glEnd();
    break;
  case MENU_CONFIG:
    // タイトル
    glColor4f( 0.0, 0.2, 0.0, 1.0 );
    glBegin(GL_QUADS);
    glVertex2i(  30, 250 );
    glVertex2i( 330, 250 );
    glVertex2i( 330, 570 );
    glVertex2i(  30, 570 );

    glVertex2i( 430, 320 );
    glVertex2i( 730, 320 );
    glVertex2i( 730, 570 );
    glVertex2i( 430, 570 );

    glVertex2i( 430, 120 );
    glVertex2i( 730, 120 );
    glVertex2i( 730, 300 );
    glVertex2i( 430, 300 );
    glEnd();

    glColor4f( 1.0, 1.0, 1.0, 0.0 );
    glRasterPos2i( 80, 520 );
    glBitmap( 200, 35, 0.0, 0.0, 0.0, 0, &m_configTitle[0][0] );
    glRasterPos2i( 480, 520 );
    glBitmap( 200, 35, 0.0, 0.0, 0.0, 0, &m_configTitle[1][0] );
//    glRasterPos2i( 480, 250 );
//    glBitmap( 200, 35, 0.0, 0.0, 0.0, 0, &m_configTitle[2][0] );

    for ( i = 0 ; i < m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)-1 ;
	  i++ ) {
      if ( gameLevel == i )
	glColor4f( 1.0, 1.0, 0.0, 0.0 );
      else
	glColor4f( 1.0, 1.0, 1.0, 0.0 );

      glRasterPos2i( 100, 450-i*60 );
      glBitmap( 200, 35, 0.0, 0.0, 0.0, 0, &m_configChoice[i][0] );
    }

    // for Tsuborish
    glColor4f( 0.0, 0.0, 0.0, 0.0 );
    glRasterPos2i( 100, 450-i*60 );
    glBitmap( 200, 35, 0.0, 0.0, 0.0, 0, &m_configChoice[i][0] );

    for ( i = 0 ; i < m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_MODE) ;
	  i++ ) {
      if ( gameMode == i )
	glColor4f( 1.0, 1.0, 0.0, 0.0 );
      else
	glColor4f( 1.0, 1.0, 1.0, 0.0 );

      glRasterPos2i( 500, 450-i*60 );
      glBitmap( 200, 35, 0.0, 0.0, 0.0, 0,
		&m_configChoice
		[i+m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)][0] );
    }

    for ( i = 0 ; i < m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_PLAYER) ;
	  i++ ) {
#if 0
      if ( theSound.GetSoundMode() == i )
	glColor4f( 1.0, 1.0, 0.0, 0.0 );
      else
	glColor4f( 1.0, 1.0, 1.0, 0.0 );
#else
      if ( (isWireFrame && i == 0) || (!isWireFrame && i == 1) )
	glColor4f( 1.0, 1.0, 0.0, 0.0 );
      else
	glColor4f( 1.0, 1.0, 1.0, 0.0 );
#endif

      glRasterPos2i( 500, 190-i*60 );
      glBitmap( 200, 35, 0.0, 0.0, 0.0, 0,
		&m_configChoice
		[i+m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)
		  +m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_MODE)][0] );
    }

    if (m_title->GetSelected() == m_title->GetMenuNum(MENU_CONFIG, MENU_ALL)-1)
      glColor4f( 1.0, 1.0, 0.0, 0.0 );
    else
      glColor4f( 1.0, 1.0, 1.0, 0.0 );
    glRasterPos2i( 300, 20 );
    glBitmap( 400, 70, 0.0, 0.0, 0.0, 0, &m_choice[4][0] );

    glColor4f( 1.0, 1.0, 0.0, 0.0 );
    glBegin(GL_TRIANGLES);
    if ( m_title->GetSelected() <
	 m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)-1 ) {
      glVertex2i( 60, 450-m_title->GetSelected()*60 );
      glVertex2i( 90, 468-m_title->GetSelected()*60 );
      glVertex2i( 60, 486-m_title->GetSelected()*60 );
    } else if ( m_title->GetSelected() <
		m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)+
		m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_MODE) ) {
      glVertex2i( 460, 690-m_title->GetSelected()*60 );
      glVertex2i( 490, 708-m_title->GetSelected()*60 );
      glVertex2i( 460, 726-m_title->GetSelected()*60 );
    } else if ( m_title->GetSelected() <
		m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)+
		m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_MODE)+
		m_title->GetMenuNum(MENU_CONFIG, MENU_CONFIG_PLAYER) ) {
      glVertex2i( 460, 615-m_title->GetSelected()*60 );
      glVertex2i( 490, 633-m_title->GetSelected()*60 );
      glVertex2i( 460, 651-m_title->GetSelected()*60 );
    } else {
      glVertex2i( 170, 20 );
      glVertex2i( 230, 55 );
      glVertex2i( 170, 90 );
    }
    glEnd();
    break;
  }

  glDepthMask(1);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  return true;
}
