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

extern Ball theBall;
extern Player* thePlayer;
extern Player* comPlayer;
extern BaseView theView;
extern PlayerSelect* theSelect;
extern Title* theTitle;
extern Howto* theHowto;
extern long mode;
extern long gameLevel;

extern void Timer( int value );
struct timeb m_lastTime = {0, 0, 0, 0};	// 直前にTimerEventが呼ばれたときの時刻

extern void PlayInit( long player, long com );
extern void DemoInit();
extern void SelectInit();
extern void TitleInit();
extern void HowtoInit();

extern bool isLighting;
extern bool isFog;
extern bool isTexture;
extern bool isPolygon;
extern long winWidth;
extern long winHeight;

extern long wins;

#if HAVE_LIBPTHREAD & USETHREAD
extern pthread_mutex_t drawMutex;
pthread_mutex_t inputMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

long _perfCount;
long perfs;

Event::Event() {
  m_KeyHistory[0] = 0;
  m_MouseXHistory[0] = winWidth/2;
  m_MouseYHistory[0] = winHeight/2;
  m_MouseBHistory[0] = 0;
  m_Histptr = 0;
}

Event::~Event() {
}

void
Event::IdleFunc() {
  struct timeb tb;

  int i;
  long diffsec, diffmsec;
  long millisec;
  long diffcount = 0;
  long key;
  long x, y;
  long btn;
  long preMode = mode;
  bool reDraw = false;

#ifndef WIN32
  struct timeval tv;
  struct timezone tz;
#endif

#ifdef WIN32
  ftime( &tb );
#else
  gettimeofday( &tv, &tz );
  tb.time = tv.tv_sec;
  tb.millitm = tv.tv_usec/1000;
#endif

  if ( m_lastTime.time == 0 ){
    m_lastTime = tb;
    _perfCount = 0;
    perfs = 0;
    return;
  }

  diffsec = tb.time - m_lastTime.time;
  diffmsec = tb.millitm + diffsec*1000 - m_lastTime.millitm;

  diffcount = diffmsec / 10;
  if ( diffcount == 0 ) {
//    usleep( (10-diffmsec)*1000 );
    return;
  }

  perfs +=diffcount;
  _perfCount++;

#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_lock( &inputMutex );
#endif
  for ( i = 0 ; i < diffcount ; i++ ){
    switch ( mode ){
    case MODE_PLAY:
      theBall.Move();
      reDraw |= thePlayer->Move( m_KeyHistory, m_MouseXHistory,
				 m_MouseYHistory, m_MouseBHistory, m_Histptr );

      reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
      break;
    case MODE_SELECT:
      reDraw |= theSelect->Move( m_KeyHistory, m_MouseXHistory,
				 m_MouseYHistory, m_MouseBHistory, m_Histptr );
      break;
    case MODE_DEMO:
      theBall.Move();
      reDraw |= thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

      if ( (m_MouseXHistory[m_Histptr] - winWidth/2) / (winWidth/40) ||
	   (m_MouseYHistory[m_Histptr] - winHeight/2) / (winHeight/40) ||
	   m_MouseBHistory[m_Histptr] || m_KeyHistory[m_Histptr] ) {
	theBall.EndGame();
      }

      reDraw = true;
      break;
    case MODE_TITLE:
      theBall.Move();
      reDraw |= thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

      reDraw |= theTitle->Move( m_KeyHistory, m_MouseXHistory,
				m_MouseYHistory, m_MouseBHistory, m_Histptr );
      break;
    case MODE_HOWTO:
      if ( theHowto->IsMove() ) {
	theBall.Move();
	reDraw |= thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
	reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
      }

      reDraw |= theHowto->Move( m_KeyHistory, m_MouseXHistory,
				m_MouseYHistory, m_MouseBHistory, m_Histptr );
      break;
    }

    if ( mode != preMode ){	// モード変更あり
#if HAVE_LIBPTHREAD & USETHREAD
      pthread_mutex_lock( &drawMutex );
#endif
      long p;

      switch ( mode ){
      case MODE_PLAY:
	if ( theSelect->GetRotate() < 0 )
	  p = (360+(theSelect->GetRotate()%360))/(360/PLAYERS);
	else
	  p = (theSelect->GetRotate()%360)/(360/PLAYERS);

	gameLevel = theSelect->GetLevel();
	PlayInit( p, (p+wins+1)%PLAYERS );
	break;
      case MODE_SELECT:
	SelectInit();
	break;
      case MODE_DEMO:
	DemoInit();
	break;
      case MODE_TITLE:
	TitleInit();
	break;
      case MODE_HOWTO:
	HowtoInit();
	break;
      }

      preMode = mode;
      reDraw = true;

#if HAVE_LIBPTHREAD & USETHREAD
      pthread_mutex_unlock( &drawMutex );
#endif
    }

    x = m_MouseXHistory[m_Histptr];
    y = m_MouseYHistory[m_Histptr];
    key = m_KeyHistory[m_Histptr];
    btn = m_MouseBHistory[m_Histptr];

    m_Histptr++;
    if ( m_Histptr == MAX_HISTORY )
      m_Histptr = 0;

    m_KeyHistory[m_Histptr] = 0;
//    m_MouseXHistory[m_Histptr] = winWidth/2 + (x-winWidth/2)*7/8;
//    m_MouseYHistory[m_Histptr] = winHeight/2 + (y-winHeight/2)*7/8;
    if ( mode == MODE_TITLE ) {
      m_MouseXHistory[m_Histptr] = x;
      m_MouseYHistory[m_Histptr] = y;
    } else if ( mode == MODE_SELECT ) {
      m_MouseXHistory[m_Histptr] = winWidth/2 + (x-winWidth/2)*15/16;
      m_MouseYHistory[m_Histptr] = y;
    } else {
      m_MouseXHistory[m_Histptr] = winWidth/2 + (x-winWidth/2)*15/16;
      m_MouseYHistory[m_Histptr] = winHeight/2 + (y-winHeight/2)*15/16;
    }
    m_MouseBHistory[m_Histptr] = btn;
  }

  millisec = (long)tb.millitm - (diffmsec-diffcount*10);

  while ( millisec < 0 ){
    tb.time--;
    millisec += 1000;
  }

  tb.millitm = millisec;
  m_lastTime = tb;

#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_lock( &drawMutex );
#endif
  if ( mode != MODE_TITLE )
    glutWarpPointer( m_MouseXHistory[m_Histptr], m_MouseYHistory[m_Histptr] );

  if ( reDraw )
    glutPostRedisplay();
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_unlock( &drawMutex );
#endif
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_unlock( &inputMutex );
#endif
}

void
Event::KeyboardFunc( unsigned char key, int x, int y ) {
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_lock( &inputMutex );
#endif
#if (GLUT_API_VERSION < 4 && GLUT_XLIB_IMPLEMENTATION < 13)
  if ( key == 'Q' ) {
    printf( "Avg = %f\n", (double)perfs/_perfCount );
    exit(0);
  }
#endif
  m_KeyHistory[m_Histptr] = key;
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_unlock( &inputMutex );
#endif
}

void
Event::KeyUpFunc( unsigned char key, int x, int y ) {
  if ( key == 'Q' ) {
    printf( "Avg = %f\n", (double)perfs/_perfCount );
    exit(0);
  }
}

void
Event::MotionFunc( long x, long y ) {
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_lock( &inputMutex );
#endif
  m_MouseXHistory[m_Histptr] = x;
  m_MouseYHistory[m_Histptr] = y;

#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_unlock( &inputMutex );
#endif
}

void
Event::ButtonFunc( long button, long state, long x, long y ) {
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_lock( &inputMutex );
#endif
  if ( state == GLUT_DOWN ){
    switch ( button ){
    case GLUT_LEFT_BUTTON:
      m_MouseBHistory[m_Histptr] |= BUTTON_LEFT;
      break;
    case GLUT_MIDDLE_BUTTON:
      m_MouseBHistory[m_Histptr] |= BUTTON_MIDDLE;
      break;
    case GLUT_RIGHT_BUTTON:
      m_MouseBHistory[m_Histptr] |= BUTTON_RIGHT;
      break;
    }
  }
  else{
    switch ( button ){
    case GLUT_LEFT_BUTTON:
      m_MouseBHistory[m_Histptr] &= ~BUTTON_LEFT;
      break;
    case GLUT_MIDDLE_BUTTON:
      m_MouseBHistory[m_Histptr] &= ~BUTTON_MIDDLE;
      break;
    case GLUT_RIGHT_BUTTON:
      m_MouseBHistory[m_Histptr] &= ~BUTTON_RIGHT;
      break;
    }
  }

  m_MouseXHistory[m_Histptr] = x;
  m_MouseYHistory[m_Histptr] = y;
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_unlock( &inputMutex );
#endif
}
