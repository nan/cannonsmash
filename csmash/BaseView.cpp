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
#include "BaseView.h"
#include "BaseView2D.h"
#include "Player.h"
#include "Control.h"
#include "Event.h"
#include "BaseView.h"
#include "Ball.h"
#include "LoadImage.h"
#include "PlayGame.h"

extern Player* thePlayer;
extern Player* comPlayer;
extern long mode;
extern Control*      theControl;

extern Event theEvent;

extern BaseView* theView;
extern Ball theBall;

extern bool isLighting;
extern bool isTexture;
extern bool isPolygon;
extern bool isSimple;
extern bool fullScreen;
extern bool is2D;

long BaseView::m_winWidth = WINXSIZE;
long BaseView::m_winHeight = WINYSIZE;

// x --- x axis is the bottom line of the net. The plane x=0 represents
//       the vertical plain which includes center line. 
// y --- y axis is the center line. The plane y=0 includes the net. 
// z --- z axis is the vertical line which includes the center point of
//       the table. The plane z=0 is the floor. 

BaseView *
BaseView::Create() {
  if ( is2D )
    return new BaseView2D();
  else
    return new BaseView();
}

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
// Create and initialize Window

  if ( fullScreen )
    m_baseSurface = SDL_SetVideoMode( m_winWidth, m_winHeight, 0,
				      SDL_OPENGL|SDL_FULLSCREEN );
  else
    m_baseSurface = SDL_SetVideoMode( m_winWidth, m_winHeight, 0,
				      SDL_OPENGL );

  SDL_WM_SetCaption( "CannonSmash", NULL );

  //if (fullScreen)
  //SDL_WM_ToggleFullScreen( m_baseSurface );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //glOrtho(-2.0, 2.0, -2.0, 2.0, 2.0, -2.0);
  gluPerspective(60.0, 1.0, 0.1, 30.0);
  glMatrixMode(GL_MODELVIEW);

  //GLfloat light_position[] = { 1.0F, 1.0F, 1.0F, 0.0F };
  GLfloat light_intensity_amb[] = { 0.6F, 0.6F, 0.6F, 1.0F };
  GLfloat light_intensity_dif[] = { 1.0F, 1.0F, 1.0F, 1.0F };
  GLfloat light_intensity_none[] = { 0.0F, 0.0F, 0.0F, 0.0F };

  if (!isSimple) {
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_SRC_ALPHA);
  }

  glEnable(GL_COLOR_MATERIAL);
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);


  //glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_intensity_amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_intensity_dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_intensity_dif);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  if (!isSimple) {
    glEnable(GL_FOG);
    GLfloat fogcolor[] = {0.2F, 0.2F, 0.2F, 1.0F};
    glFogi(GL_FOG_MODE, GL_EXP);
    glFogfv(GL_FOG_COLOR, fogcolor);
    glFogf(GL_FOG_DENSITY, 0.05F);
    glHint(GL_FOG_HINT, GL_DONT_CARE);
    glFogf(GL_FOG_START, 1.0F);
    glFogf(GL_FOG_END, AREAYSIZE*2);
  }

  glClearColor(0.2F, 0.2F, 0.2F, 1.0F);
  if (isSimple) {
    glClear(GL_COLOR_BUFFER_BIT);
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  int i, j;
// Set textures. 
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

  return true;
}

void
BaseView::DisplayFunc() {
  theView->RedrawAll();
}

bool
BaseView::RedrawAll() {
  View *view;
  GLfloat light_position[] = { 1.0F, -1.0F, 1.0F, 0.0F };

  SetViewPosition();

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);

  if (isSimple) {
    glClear(GL_COLOR_BUFFER_BIT);
  } else {
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  if ( mode == MODE_OPENING )	// Must be improved
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glColor4f(0.4F, 0.2F, 0.0F, 0.0F);
//  glEnable(GL_CULL_FACE);

  glDisable(GL_BLEND);
  m_fieldView->Redraw();
  if (isSimple)
    m_fieldView->RedrawAlpha();

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
  if (!isSimple)
    m_fieldView->RedrawAlpha();

  view = m_View;
  while ( view ){
    view->RedrawAlpha();
    view = view->m_next;
  }

  // Title
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0F, (GLfloat)m_winWidth, 0.0F, (GLfloat)m_winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glColor4f( 0.0F, 0.0F, 0.0F, 1.0F );

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  if ( isTexture ) {
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, m_title );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0F, 1.0F);
    glVertex2i( m_winWidth-256, 0);
    glTexCoord2f(1.0F, 1.0F);
    glVertex2i( m_winWidth, 0 );
    glTexCoord2f(1.0F, 0.0F);
    glVertex2i( m_winWidth, 256 );
    glTexCoord2f(0.0F, 0.0F);
    glVertex2i( m_winWidth-256, 256 );
    glEnd();

    glDisable(GL_TEXTURE_2D);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  SDL_GL_SwapBuffers();

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

// Move the viewpoint as the player moves
void
BaseView::SetLookAt() {
  double srcX, srcY, srcZ;
  double destX, destY, destZ;

  srcX = srcY = srcZ = 0.0F;
  destX = m_centerX;
  if (thePlayer)
    destY = m_centerY*thePlayer->GetSide();
  else
    destY = m_centerY;
  destZ = m_centerZ;

  theControl->LookAt( srcX, srcY, srcZ, destX, destY, destZ );

  gluLookAt( srcX, srcY, srcZ, destX, destY, destZ, 0.0F, 0.0F, 0.5F );
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

  glColor4f(1.0F, 1.0F, 1.0F, 0.0F);

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0F, (GLfloat)m_winWidth, 0.0F, (GLfloat)m_winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  printf( "EndGame %d : %d\n", ((PlayGame *)theControl)->GetScore(thePlayer), 
    ((PlayGame *)theControl)->GetScore(comPlayer) );

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
      bmp[i*50+j] = (unsigned char)strtol( getWord(fp), NULL, 16 );
    }
  }

#ifndef HAVE_LIBZ
  fclose(fp);
#else
  gzclose(fp);
#endif

  glRasterPos2i( 220, 330 );
  glBitmap( 400, 70, 0.0F, 0.0F, 0.0F, 0, bmp );

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  SDL_GL_SwapBuffers();

#ifdef WIN32
  Sleep(3000);
#else
  sleep(3);
#endif
}

void
BaseView::QuitGame() {
  //if (fullScreen)
  //SDL_WM_ToggleFullScreen( m_baseSurface );
  SDL_FreeSurface( m_baseSurface );
  delete m_fieldView;
}
