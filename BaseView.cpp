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
extern Control*      theControl;

extern Event theEvent;

extern BaseView theView;
extern Ball theBall;

extern bool isLighting;
extern bool isFog;
extern bool isTexture;
extern bool isPolygon;
extern bool isSimple;

long BaseView::m_winWidth = WINXSIZE;
long BaseView::m_winHeight = WINYSIZE;

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
// Windowの生成, 初期化
  glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH );
//  glutInitDisplayMode( GLUT_RGBA | GLUT_DEPTH );
  glutInitWindowSize(m_winWidth, m_winHeight);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("CannonSmash");

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-2.0, 2.0, -2.0, 2.0, 2.0, -2.0);
  /* 有効領域設定. 左, 右, 下, 上, 奥, 手前 */
  gluPerspective(30.0, 1.0, 0.1, 20.0);
  /* 遠近法設定. 視野角, x/y比, 有効距離(手前), 有効距離(奥) */
  glMatrixMode(GL_MODELVIEW);

  GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
  GLfloat light_intensity_amb[] = { 0.6, 0.6, 0.6, 1.0 };
  GLfloat light_intensity_dif[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat light_intensity_none[] = { 0.0, 0.0, 0.0, 0.0 };

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

  int i, j;
// テクスチャの設定. 
  ImageData image;

  glGenTextures( 1, &m_title );

  image.LoadPPM( "images/Title.ppm" );

  for ( i = 0 ; i < image.GetWidth() ; i++ ) {
    for ( j = 0 ; j < image.GetHeight() ; j++ ) {
      if ( image.GetPixel( i, j, 0 ) >= 5 ||
	   image.GetPixel( i, j, 1 ) >= 5 ||
	   image.GetPixel( i, j, 2 ) >= 5 )
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

  m_fieldView = new FieldView();
  m_fieldView->Init();

  glutDisplayFunc( theView.DisplayFunc );

  return true;
}

void
BaseView::DisplayFunc() {
  theView.RedrawAll();
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

  if ( mode == MODE_OPENING )	// もうちょっとまともにする
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glColor4f(0.4, 0.2, 0.0, 0.0);
//  glEnable(GL_CULL_FACE);

  glDisable(GL_BLEND);
  m_fieldView->Redraw();

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

  if ( !isSimple )
    glEnable(GL_BLEND);

//  glDisable(GL_CULL_FACE);
  m_fieldView->RedrawAlpha();

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
  gluOrtho2D( 0.0, (GLfloat)m_winWidth, 0.0, (GLfloat)m_winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f( 0.0, 0.0, 0.0, 1.0 );

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
  if ( isTexture ) {
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, m_title );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0);
    glVertex2i( m_winWidth-256, 0 );
    glTexCoord2f(1.0, 1.0);
    glVertex2i( m_winWidth, 0 );
    glTexCoord2f(1.0, 0.0);
    glVertex2i( m_winWidth, 256 );
    glTexCoord2f(0.0, 0.0);
    glVertex2i( m_winWidth-256, 256 );
    glEnd();

    glDisable(GL_TEXTURE_2D);
  }

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
BaseView::SetViewPosition() {
  glLoadIdentity();
  SetLookAt();

  return true;
}

// 視線をボールの動きに合わせて滑らかに移動させる
void
BaseView::SetLookAt() {
  double srcX, srcY, srcZ;
  double destX, destY, destZ;

  srcX = srcY = srcZ = 0.0;
  destX = m_centerX;
  if (thePlayer)
    destY = m_centerY*thePlayer->GetSide();
  else
    destY = m_centerY;
  destZ = m_centerZ;

  theControl->LookAt( srcX, srcY, srcZ, destX, destY, destZ );

  gluLookAt( srcX, srcY, srcZ, destX, destY, destZ, 0.0, 0.0, 0.5 );
  /* 視点, 視線設定. 視点x, y, z, 視点(視線ベクトルの通る点)x, y, z, 
     上向きベクトル(画面上の上に向かうベクトル)x, y, z */
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
  gluOrtho2D( 0.0, (GLfloat)m_winWidth, 0.0, (GLfloat)m_winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if ( theControl->IsPlaying() && 
       ((PlayGame *)theControl)->GetScore(thePlayer) >
       ((PlayGame *)theControl)->GetScore(comPlayer) ) {
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

void
BaseView::ReshapeFunc( int width, int height ) {
  m_winWidth = width;
  m_winHeight = height;

  glViewport( 0, 0, m_winWidth, m_winHeight );
}
