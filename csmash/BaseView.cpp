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

#ifdef HAVE_LIBZ
#include "z.h"
#endif

extern Player* thePlayer;
extern Player* comPlayer;
extern long mode;
extern Title*  theTitle;
extern Howto*  theHowto;

extern void SelectInit();

extern Ball theBall;

extern bool isLighting;
extern bool isFog;
extern bool isTexture;
extern bool isPolygon;
extern long winWidth;
extern long winHeight;

// x --- x座標軸はネットの底辺とその延長. x=0は, センターラインを含み
//       台に垂直な平面. 
// y --- y座標軸はセンターラインとその延長. y=0はネットを含む平面. 
// z --- z座標軸は台の中心を通る鉛直線. z=0は床面. 

BaseView::BaseView() {
  m_View = (View*)0;
  m_centerX = 0.0;
  m_centerY = TABLELENGTH/2;
  m_centerZ = TABLEHEIGHT;
}

BaseView::~BaseView() {
}

bool
BaseView::Init() {
  int i, j;
// Windowの生成, 初期化
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
//  glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH );
  glutInitWindowSize(winWidth, winHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("CannonSmash");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-2.0, 2.0, -2.0, 2.0, 2.0, -2.0);
  /* 有効領域設定. 左, 右, 下, 上, 奥, 手前 */
  gluPerspective(30.0, 1.0, 0.1, 20.0);
  /* 遠近法設定. 視野角, x/y比, 有効距離(手前), 有効距離(奥) */
  glMatrixMode(GL_MODELVIEW);

//  GLfloat light_position[] = { 0.0, -TABLELENGTH, 1.0, 1.0 };
  GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
  GLfloat light_intensity_amb[] = { 0.6, 0.6, 0.6, 1.0 };
  GLfloat light_intensity_dif[] = { 1.0, 1.0, 1.0, 1.0 };

  glShadeModel (GL_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glBlendFunc(GL_ONE, GL_SRC_ALPHA);

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_intensity_amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_intensity_dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_intensity_dif);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  if (isFog) {
    glEnable(GL_FOG);
    GLfloat fogcolor[] = {0.2, 0.2, 0.2, 1.0};
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogfv(GL_FOG_COLOR, fogcolor);
    glFogf(GL_FOG_DENSITY, 0.05);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 1.0);
    glFogf(GL_FOG_END, AREAYSIZE*2);
  }

  glClearColor (0.2, 0.2, 0.2, 1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// テクスチャの設定. 
  ImageData image;

  image.LoadPPM( "images/Floor.ppm" );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures( 1, &m_floor );
  glBindTexture( GL_TEXTURE_2D, m_floor );
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, image.GetWidth(), image.GetHeight(),
	       0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );

  glGenTextures( 1, &m_title );

  image.LoadPPM( "images/Title.ppm" );

  for ( i = 0 ; i < image.GetWidth() ; i++ ) {
    for ( j = 0 ; j < image.GetHeight() ; j++ ) {
      if ( image.GetPixel( i, j, 0 ) != 0 ||
	   image.GetPixel( i, j, 1 ) != 0 ||
	   image.GetPixel( i, j, 2 ) != 0 )
	image.SetPixel( i, j, 3 , 0 );
    }
  }

  glBindTexture( GL_TEXTURE_2D, m_title );
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 4, image.GetWidth(), image.GetHeight(),
	       0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );

  static char pname[][30] = {"images/Left.ppm", "images/Front.ppm", 
			     "images/Right.ppm", "images/Back.ppm" };

  glGenTextures( 4, m_wall );
  for ( i = 0 ; i < 4 ; i++ ) {
    image.LoadPPM( &(pname[i][0]) );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture( GL_TEXTURE_2D, m_wall[i] );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.GetWidth(), image.GetHeight(),
		 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );
  }

  m_offset = glGenLists(1);

  glNewList( m_offset, GL_COMPILE );

  glBindTexture( GL_TEXTURE_2D, m_floor );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  glBegin(GL_QUADS);			// 床の描画
    glNormal3f( 0.0, 0.0, 1.0 );
    glTexCoord2f(0.0, 0.0); glVertex3f( -AREAXSIZE*2,  AREAYSIZE*2, 0.0 );
    glTexCoord2f(4.0, 0.0); glVertex3f( -AREAXSIZE*2, -AREAYSIZE*2, 0.0 );
    glTexCoord2f(4.0, 4.0); glVertex3f(  AREAXSIZE*2, -AREAYSIZE*2, 0.0 );
    glTexCoord2f(0.0, 4.0); glVertex3f(  AREAXSIZE*2,  AREAYSIZE*2, 0.0 );
  glEnd();

  glEndList();

  return true;
}

bool
BaseView::RedrawAll() {
  View *view;

  SetViewPosition();

  if ( isLighting ) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  glColor4f(0.4, 0.2, 0.0, 0.0);
//  glEnable(GL_CULL_FACE);

  glDisable(GL_BLEND);
  Redraw();

  view = m_View;
  while ( view ){
    if ( view->Redraw() == false ){
      View *_view = m_View;

      if ( _view == view ){
	m_View = view->m_next;
	delete view;
	view = m_View;
      }
      else{
	while ( _view->m_next != view )
	  _view = _view->m_next;

	_view->m_next = view->m_next;
	delete view;
	view = _view;
      }
    }

    view = view->m_next;
  }

  glEnable(GL_BLEND);
//  glDisable(GL_CULL_FACE);
  RedrawAlpha();

  view = m_View;
  while ( view ){
    view->RedrawAlpha();
    view = view->m_next;
  }

  // タイトル
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLfloat)winWidth, 0.0, (GLfloat)winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f( 0.0, 0.0, 0.0, 1.0 );

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, m_title );
  glBegin(GL_QUADS);
  glTexCoord2f(0.0, 1.0);
  glVertex2i( winWidth-256, 0 );
  glTexCoord2f(1.0, 1.0);
  glVertex2i( winWidth, 0 );
  glTexCoord2f(1.0, 0.0);
  glVertex2i( winWidth, 256 );
  glTexCoord2f(0.0, 0.0);
  glVertex2i( winWidth-256, 256 );
  glEnd();

  glDisable(GL_TEXTURE_2D);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glutSwapBuffers();

//  if ( glGetError() != GL_NO_ERROR )
//    printf( "GL Error!\n" );

  return true;
}

bool
BaseView::Redraw() {
  if ( isTexture )
    glEnable(GL_TEXTURE_2D);

  glCallList( m_offset );

  glColor4f(0.8, 0.8, 0.8, 0.0);

  glBindTexture( GL_TEXTURE_2D, m_wall[0] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);			// 壁の描画
    glNormal3f( 1.0, 0.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f( -AREAXSIZE,  AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f( -AREAXSIZE,  AREAYSIZE, AREAZSIZE);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, m_wall[1] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);
    glNormal3f( 0.0, 1.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, AREAZSIZE);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, m_wall[2] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);
    glNormal3f( -1.0, 0.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f(  AREAXSIZE,  AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f(  AREAXSIZE,  AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, AREAZSIZE);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, m_wall[3] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);
    glNormal3f( 0.0, -1.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f(  AREAXSIZE, AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f( -AREAXSIZE, AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f( -AREAXSIZE, AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f(  AREAXSIZE, AREAYSIZE, AREAZSIZE);
  glEnd();

  if ( isTexture )
    glDisable(GL_TEXTURE_2D);

  glColor4f(0.1, 0.1, 0.1, 0.0);
  glBegin(GL_QUADS);			// 天井の描画
    glNormal3f( 0.0, 0.0, -1.0 );
    glVertex3f(  AREAXSIZE*2,  AREAYSIZE*2, AREAZSIZE);
    glVertex3f( -AREAXSIZE*2,  AREAYSIZE*2, AREAZSIZE);
    glVertex3f( -AREAXSIZE*2, -AREAYSIZE*2, AREAZSIZE);
    glVertex3f(  AREAXSIZE*2, -AREAYSIZE*2, AREAZSIZE);
  glEnd();

  if ( isLighting ) {
    glColor4f(0.0, 0.2, 0.0, 0.0);
    static GLfloat mat_board[] = { 0.0, 0.2, 0.0, 0.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_board);

    glBegin(GL_QUADS);			// 仕切り板の描画
      glNormal3f( 1.0, 0.0, 0.0 );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );

      glNormal3f( -1.0, 0.0, 0.0 );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );

      glNormal3f( 0.0, -1.0, 0.0 );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );

      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );
    glEnd();
  }

  glColor4f(0.4, 0.4, 0.4, 0.0);
  static GLfloat mat_leg[] = { 0.0, 0.4, 0.0, 0.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_leg);

  glBegin(GL_QUADS);			// 足の描画
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, 0.0);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.13, -TABLELENGTH/2+0.1, 0.0);
    glVertex3f(-TABLEWIDTH/2+0.13, -TABLELENGTH/2+0.1, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.1,  0.0);


    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, 0.0);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  0.0);


    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, 0.0);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  0.0);


    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, 0.0);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  0.0);
  glEnd();

  return true;
}

// 透明なものの描画
bool
BaseView::RedrawAlpha() {
  glColor4f(0.0, 0.0, 0.3, 0.7);
  static GLfloat mat_table[] = { 0.0, 0.0, 0.3, 1.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_table);

  glDepthMask(0);
  glBegin(GL_QUADS);			// Tableの描画
  if ( isLighting ) {
    glNormal3f( 0.0, 0.0, -1.0 );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );

    glNormal3f( 1.0, 0.0, 0.0 );
    glVertex3f( TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f( TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );

    glNormal3f( -1.0, 0.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );

    glNormal3f( 0.0, 1.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT );

    glNormal3f( 0.0, -1.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
  }

    glNormal3f( 0.0, 0.0, 1.0 );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f(  TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
  glEnd();
  glDepthMask(1);

  static GLfloat mat_default[] = { 0.0, 0.0, 0.0, 1.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);

  if ( isLighting ) {
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);		// 白線の描画
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( -TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );

      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( -TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );

      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );

      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );
    glEnd();

    glBegin(GL_LINES);
      glVertex3f( 0.0, -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( 0.0,  TABLELENGTH/2, TABLEHEIGHT );
    glEnd();
  }

  glColor4f(0.0, 0.2, 0.0, 0.9);

  static GLfloat mat_net[] = { 0.0, 0.2, 0.0, 1.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_net);
  glBegin(GL_QUADS);			// Netの描画
    glNormal3f( 0.0, 1.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
    glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
    glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT );
  glEnd();
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);

  if ( isLighting ) {
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);
      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT );
      glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
      glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
      glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT );
    glEnd();
    glColor4f( 0.2, 1.0, 0.2, 1.0 );	// Why? But my RIVA128 need it.
  }

//  glBlendFunc(GL_ONE, GL_SRC_ALPHA);

  return true;
}

bool
BaseView::SetViewPosition() {
  glLoadIdentity();
  SetLookAt();

  return true;
}

// 視線をボールの動きに合わせて滑らかに移動させる
void
BaseView::SetLookAt() {
  double x, y, z;

  x = y = z = 0.0;

  switch ( mode ){
  case MODE_PLAY:
  case MODE_DEMO:
  case MODE_TRAINING:
    x = thePlayer->GetX() + thePlayer->GetEyeX();
    y = thePlayer->GetY() + thePlayer->GetEyeY();
    z = thePlayer->GetZ() + thePlayer->GetEyeZ();
    break;
  case MODE_SELECT:
  case MODE_TRAININGSELECT:
    x = 0.0;
    y = -TABLELENGTH-1.2;
    z = 1.4;
    break;
  case MODE_TITLE:
    x = TABLELENGTH*2*cos( theTitle->GetCount()*3.14159265/720.0 )+m_centerX;
    y = TABLELENGTH*2*sin( theTitle->GetCount()*3.14159265/720.0 )+m_centerY;
    z = TABLEHEIGHT*4;
    break;
  case MODE_HOWTO:
    theHowto->LookAt( x, y, z );
  }

  if ( thePlayer ) {
    gluLookAt( x, y, z, m_centerX, m_centerY*thePlayer->GetSide(), m_centerZ,
	       0.0, 0.0, 0.5 );
    /* 視点, 視線設定. 視点x, y, z, 視点(視線ベクトルの通る点)x, y, z, 
       上向きベクトル(画面上の上に向かうベクトル)x, y, z */
  } else {
    gluLookAt( x, y, z, m_centerX, m_centerY, m_centerZ, 0.0, 0.0, 0.5 );
  }
}

bool
BaseView::AddView( View *view ) {
  view->m_next = m_View;
  m_View = view;

  return true;
}

bool
BaseView::RemoveView( View *view ) {
  View* _view = m_View;

  if ( _view == view ){
    m_View = _view->m_next;
    return true;
  }

  while ( _view ){
    if ( _view->m_next == view ){
      _view->m_next = view->m_next;
      return true;
    }
    _view = _view->m_next;
  }

  return false;
}
      
// 終了処理. 
void
BaseView::EndGame() {
  static char file[][30] = {"images/win.ppm", "images/lose.ppm"};
  GLubyte bmp[400*70/8];
#ifndef HAVE_LIBZ
  FILE *fp;
#else
  gzFile fp;
#endif
  int i, j;

  glColor4f(1.0, 1.0, 1.0, 0.0);

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLfloat)winWidth, 0.0, (GLfloat)winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if ( theBall.GetScore(thePlayer) > theBall.GetScore(comPlayer) ) {
#ifndef HAVE_LIBZ
    if ( (fp = fopen(&file[0][0], "r")) == NULL )
      return;
#else
    if ( (fp = gzopenx(&file[0][0], "rs")) == NULL )
      return;
#endif
  }  else {
#ifndef HAVE_LIBZ
    if ( (fp = fopen(&file[1][0], "r")) == NULL )
      return;
#else
    if ( (fp = gzopenx(&file[1][0], "rs")) == NULL )
      return;
#endif
  }

  for ( i = 69 ; i >= 0 ; i-- ) {
    for ( j = 0 ; j < 400/8 ; j++ ) {
      bmp[i*50+j] = strtol( getWord(fp), NULL, 16 );
    }
  }

#ifndef HAVE_LIBZ
  fclose(fp);
#else
  gzclose(fp);
#endif

  glRasterPos2i( 220, 330 );
  glBitmap( 400, 70, 0.0, 0.0, 0.0, 0, bmp );

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  glutSwapBuffers();

#ifdef WIN32
  Sleep(3000);
#else
  sleep(3);
#endif
}
