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
#include "Event.h"
#include "Control.h"
#include "BaseView.h"
#include "BaseView2D.h"
#include "SoloPlay.h"
#include "PracticePlay.h"
#include "MultiPlay.h"
#include "PlayerSelect.h"
#include "Title.h"
#include "Opening.h"
#include "Howto.h"
#include "TrainingSelect.h"
#include "Training.h"
#include "PracticeSelect.h"
#include "RCFile.h"

extern RCFile *theRC;

extern Ball theBall;
extern Player* thePlayer;
extern Player* comPlayer;
extern long mode;

extern void Timer( int value );
struct timeb Event::m_lastTime = {0, 0, 0, 0};	// 直前にTimerEventが呼ばれたときの時刻

Event* Event::m_theEvent = NULL;

extern int theSocket;

extern long wins;

extern bool isComm;

extern long score1;
extern long score2;

long _perfCount;
long perfs;

long _backTrackCount = 0;
double backTracks = 0;

void CopyPlayerData( Player& dest, Player* src );

void QuitGame();

Event::Event() {
  m_KeyHistory[0] = 0;
  m_MouseXHistory[0] = BaseView::GetWinWidth()/2;
  m_MouseYHistory[0] = BaseView::GetWinHeight()/2;
  m_MouseBHistory[0] = 0;
  m_Histptr = 0;

  m_External = NULL;

  m_mouseButton = 0;
}

Event::~Event() {
}

Event*
Event::TheEvent() {
  if ( !m_theEvent )
    m_theEvent = new Event();
  return m_theEvent;
}

bool
Event::Init() {
  switch ( mode ){
  case MODE_SOLOPLAY:
    SoloPlay::Create( 0, RAND(2) );
    break;
  case MODE_MULTIPLAY:
    MultiPlay::Create( 0, RAND(2) );
    break;
  case MODE_SELECT:
    PlayerSelect::Create();
    break;
  case MODE_TITLE:
    Title::Create();
    break;
  case MODE_OPENING:
    Opening::Create();
    break;
  }

  theBall.Init();

  if (isComm)
    m_External = new ExternalNullData();

  return true;
}

void
Event::IdleFunc() {
  struct timeb tb;

  long diffsec, diffmsec;
  long diffcount = 0;
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

    ClearBacktrack();

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

  for ( int i = 0 ; i < diffcount ; i++ ) {
    reDraw |= Event::TheEvent()->Move();
    Event::TheEvent()->Record();

    m_lastTime.millitm += 10;
    if ( m_lastTime.millitm > 1000 ) {
      m_lastTime.time++;
      m_lastTime.millitm -= 1000;
    }
  }

  if ( mode == MODE_MULTIPLAY ) {
    long preMode = mode;
    Event::TheEvent()->ReadData();
    Event::TheEvent()->IsModeChanged( preMode );
  }

  if ( mode != MODE_OPENING && mode != MODE_TITLE &&
       SDL_WM_GrabInput( SDL_GRAB_QUERY ) == SDL_GRAB_ON )
    SDL_WarpMouse((unsigned short)Event::TheEvent()->m_MouseXHistory[Event::TheEvent()->m_Histptr],
		  (unsigned short)Event::TheEvent()->m_MouseYHistory[Event::TheEvent()->m_Histptr] );

  if ( reDraw )
    BaseView::TheView()->RedrawAll();
}

bool
Event::Move() {
  long preMode = mode;
  bool reDraw = false;

  reDraw |= Control::TheControl()->Move( m_KeyHistory, m_MouseXHistory,
					 m_MouseYHistory, m_MouseBHistory,
					 m_Histptr );

  return reDraw | IsModeChanged( preMode );
}

bool
Event::IsModeChanged( long preMode ) {
  if ( mode != preMode ){	// モード変更あり
    long p, q;

    switch ( mode ) {
    case MODE_SOLOPLAY:
      p = ((PlayerSelect *)Control::TheControl())->GetPlayerNum();
      q = ((PlayerSelect *)Control::TheControl())->GetOpponentNum();

      SoloPlay::Create( p, q );
      break;
    case MODE_MULTIPLAY:
      p = ((PlayerSelect *)Control::TheControl())->GetPlayerNum();
      MultiPlay::Create( p, 0 );
      break;
    case MODE_SELECT:
      PlayerSelect::Create();
      break;
    case MODE_TITLE:
      Title::Create();
      break;
    case MODE_OPENING:
      Opening::Create();
      break;
    case MODE_HOWTO:
      Howto::Create();
      break;
    case MODE_TRAININGSELECT:
      TrainingSelect::Create();
      break;
    case MODE_TRAINING:
      p = ((PlayerSelect *)Control::TheControl())->GetPlayerNum();
      Training::Create( p, p );
      break;
    case MODE_PRACTICESELECT:
      PracticeSelect::Create();
      break;
    case MODE_PRACTICE:
      p = ((PlayerSelect *)Control::TheControl())->GetPlayerNum();
      q = ((PlayerSelect *)Control::TheControl())->GetOpponentNum();
      PracticePlay::Create( p, q );
      break;
    }

    ClearBacktrack();

    return true;
  }

  return false;
}

void
Event::Record() {
  long x, y;
  long key;
  long btn;

  x = m_MouseXHistory[m_Histptr];
  y = m_MouseYHistory[m_Histptr];
  key = m_KeyHistory[m_Histptr];
  btn = m_MouseBHistory[m_Histptr];

  m_Histptr++;

  if ( m_Histptr == MAX_HISTORY )
    m_Histptr = 0;

  if ( (mode == MODE_SOLOPLAY || mode == MODE_PRACTICE) &&
       Control::TheControl() &&
       ((SoloPlay *)Control::TheControl())->GetSmashPtr() >= 0 )
    return;

  // Warp Mouse Pointer
  SetNextMousePointer( x, y );

  m_KeyHistory[m_Histptr] = 0;
  m_MouseXHistory[m_Histptr] = x;
  m_MouseYHistory[m_Histptr] = y;
  m_MouseBHistory[m_Histptr] = btn;

  m_BacktrackBuffer[m_Histptr].sec = m_lastTime.time;
  m_BacktrackBuffer[m_Histptr].count = m_lastTime.millitm/10;

  m_BacktrackBuffer[m_Histptr].theBall = theBall;
  CopyPlayerData( m_BacktrackBuffer[m_Histptr].thePlayer, thePlayer );
  CopyPlayerData( m_BacktrackBuffer[m_Histptr].comPlayer, comPlayer );

  if ( mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY ||
       mode == MODE_PRACTICE ) {
    m_BacktrackBuffer[m_Histptr].score1 =
      ((PlayGame *)Control::TheControl())->GetScore(1);
    m_BacktrackBuffer[m_Histptr].score2 =
      ((PlayGame *)Control::TheControl())->GetScore(-1);
  }

  return;
}

void HotKey_ToggleFullScreen(void)
{
  SDL_Surface *screen = SDL_GetVideoSurface();
  if ( SDL_WM_ToggleFullScreen(screen) ) {
    theRC->fullScreen = (screen->flags & SDL_FULLSCREEN) ? 1 : 0;
    fprintf(stderr, "Toggled fullscreen mode - now %s\n",
            theRC->fullScreen  ? "fullscreen" : "windowed");
  } else {
    fprintf(stderr, "Unable to toggle fullscreen mode\n");
  }
}

void
Event::KeyboardFunc( SDL_Event key, int x, int y ) {
  if ( (key.key.keysym.sym == SDLK_RETURN) &&
       (key.key.keysym.mod & KMOD_ALT) ) {
    HotKey_ToggleFullScreen();
    return;
  }
  if ( key.key.keysym.unicode == 'Q' ) {
    if ( mode == MODE_SOLOPLAY || mode == MODE_PRACTICE ||
	 mode == MODE_SELECT || mode == MODE_HOWTO ||
	 mode == MODE_TRAININGSELECT || mode == MODE_TRAINING ||
	 mode == MODE_PRACTICESELECT ) {
    } else
      QuitGame();
  }
  Event::TheEvent()->m_KeyHistory[Event::TheEvent()->m_Histptr] = key.key.keysym.unicode;
}

void
Event::KeyUpFunc( SDL_Event key, int x, int y ) {
  // 廃止してもよい
#if 0
  if ( key.key.keysym.unicode == 'Q' ) {
    QuitGame();
  }
#endif
}

void
Event::MotionFunc( int x, int y ) {
  Event *e = Event::TheEvent();
  e->m_MouseXHistory[e->m_Histptr] = x;
  e->m_MouseYHistory[e->m_Histptr] = y;
}

void
Event::ButtonFunc( int button, int state, int x, int y ) {
  Event *e = Event::TheEvent();
  if ( state == SDL_MOUSEBUTTONDOWN ) {
    switch ( button ) {
    case 1:
      e->m_MouseBHistory[e->m_Histptr] |= BUTTON_LEFT;
      e->m_mouseButton |= BUTTON_LEFT;
      break;
    case 2:
      e->m_MouseBHistory[e->m_Histptr] |= BUTTON_MIDDLE;
      e->m_mouseButton |= BUTTON_MIDDLE;
      break;
    case 3:
      e->m_MouseBHistory[e->m_Histptr] |= BUTTON_RIGHT;
      e->m_mouseButton |= BUTTON_RIGHT;
      break;
    }
  } else {	// Relase Button
    switch ( button ) {
    case 1:
      e->m_MouseBHistory[e->m_Histptr] &= ~BUTTON_LEFT;
      e->m_mouseButton &= ~BUTTON_LEFT;
      break;
    case 2:
      e->m_MouseBHistory[e->m_Histptr] &= ~BUTTON_MIDDLE;
      e->m_mouseButton &= ~BUTTON_MIDDLE;
      break;
    case 3:
      e->m_MouseBHistory[e->m_Histptr] &= ~BUTTON_RIGHT;
      e->m_mouseButton &= ~BUTTON_RIGHT;
      break;
    }
  }

  e->m_MouseXHistory[e->m_Histptr] = x;
  e->m_MouseYHistory[e->m_Histptr] = y;
}

void
CopyPlayerData( Player& dest, Player* src ) {
  if ( !src )
    return;

  dest = *src;
}

bool
Event::GetExternalData( ExternalData *&ext, long side ) {
  ExternalData *extNow;

  if ( !(extNow = ExternalData::ReadData( side )) )
    return false;

  if ( ext->isNull() || ext->sec > extNow->sec || 
	      (ext->sec == extNow->sec && ext->count > extNow->count) ) {
    extNow->next = ext;
    ext = extNow;
  } else {
    ExternalData *extTmp = ext;
    while ( !extTmp->next->isNull() &&
	    (extTmp->next->sec < extNow->sec || 
	    (extTmp->next->sec == extNow->sec &&
	     extTmp->next->count < extNow->count) ) ) {
      extTmp = extTmp->next;
    }

    extNow->next = extTmp->next;
    extTmp->next = extNow;
  }

  return true;
}

bool
Event::SendSwing( Player *player ) {
  char buf[256];

  if ( m_backtrack || mode != MODE_MULTIPLAY )
    return false;

  strncpy( buf, "PS", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[2]) );

  player->SendSwing( &(buf[7]) );

  // Player 位置情報も送信する
  strncpy( &(buf[31]), "PV", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[33]) );

  player->SendLocation( &(buf[38]) );

  send( theSocket, buf, 31+55, 0 );
  return true;
}

bool
Event::SendPlayer( Player *player ) {
  char buf[256];

  if ( m_backtrack || mode != MODE_MULTIPLAY )
    return false;

  strncpy( buf, "PV", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[2]) );

  player->SendLocation( &(buf[7]) );

  send( theSocket, buf, 55, 0 );

  return true;
}

bool
Event::SendBall() {
  char buf[256];

  if ( m_backtrack || mode != MODE_MULTIPLAY )
    return false;

  strncpy( buf, "BV", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[2]) );

  theBall.Send( &(buf[7]) );

  send( theSocket, buf, 67, 0 );

  return true;
}

bool
Event::SendPlayerAndBall( Player *player ) {
  char buf[256];

  if ( m_backtrack || mode != MODE_MULTIPLAY )
    return false;

  strncpy( buf, "PV", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[2]) );

  player->SendLocation( &(buf[7]) );

  strncpy( &(buf[55]), "BV", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[57]) );

  theBall.Send( &(buf[62]) );

  send( theSocket, buf, 122, 0 );

  return true;
}

void
Event::ClearObject() {
  if ( thePlayer && wins == 0 ) {
    delete thePlayer;
    thePlayer = NULL;
    if ( theSocket >= 0 ) {
      send( theSocket, "QT", 2, 0 );
      closesocket( theSocket );
      theSocket = -1;
    }
  }
  if ( comPlayer ) {
    delete comPlayer;
    comPlayer = NULL;
  }
  if ( Control::TheControl() ) {
    if ( Control::TheControl()->IsPlaying() ) {
      score1 = ((PlayGame *)Control::TheControl())->GetScore(1);
      score2 = ((PlayGame *)Control::TheControl())->GetScore(-1);
    }
    delete Control::TheControl();
  }
}

bool
Event::BackTrack( long Histptr ) {
  theBall = m_BacktrackBuffer[Histptr].theBall;
  thePlayer->Reset( &m_BacktrackBuffer[Histptr].thePlayer );
  comPlayer->Reset( &m_BacktrackBuffer[Histptr].comPlayer );

  if ( mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY ||
       mode == MODE_PRACTICE) {
    ((PlayGame *)Control::TheControl())->
      ChangeScore( m_BacktrackBuffer[Histptr].score1,
		   m_BacktrackBuffer[Histptr].score2 );
  }

  m_Histptr = Histptr;
  return true;
}

void
Event::ReadData() {
  fd_set rdfds;
  struct timeval to;

  while (1) {
    FD_ZERO( &rdfds );
    FD_SET( (unsigned int)theSocket, &rdfds );

    to.tv_sec = to.tv_usec = 0;

    if ( select( theSocket+1, &rdfds, NULL, NULL, &to ) > 0 ) {
      if ( !GetExternalData( m_External, comPlayer->GetSide() ) )
	break;
    } else
      break;
  }

  // externalDataの先頭までbacktrackする
  long btCount = 0;
  ExternalData *externalOld;
  long btHistptr;
  while ( !(m_External->isNull()) ) {	// 古すぎる情報を捨てる
    btCount = (m_lastTime.time-m_External->sec)*100 + 
      (m_lastTime.millitm/10-m_External->count);
    if ( btCount > MAX_HISTORY ) {
      externalOld = m_External;
      m_External = m_External->next;
      delete externalOld;
      continue;
    }
    if ( btCount <= MAX_HISTORY )
      break;
  }

  if ( !(m_External->isNull()) && btCount > 0 ) {
    backTracks += btCount;
    _backTrackCount++;

    btHistptr = m_Histptr - btCount;
    if ( btHistptr < 0 )
      btHistptr += MAX_HISTORY;

    long mtime = m_lastTime.millitm - btCount*10;
    while ( mtime < 0 ) {
      mtime += 1000;
      m_lastTime.time--;
    }
    m_lastTime.millitm = mtime;

    bool fTheBall, fThePlayer, fComPlayer;
    fTheBall = fThePlayer = fComPlayer = false;

    BackTrack( btHistptr );

    // 適用する -> 進めるをbtCount繰り返す
    while (1) {
      if ( fTheBall )
	theBall.Move();

      if ( fThePlayer )
	thePlayer->Move( m_KeyHistory, m_MouseXHistory,
			 m_MouseYHistory, m_MouseBHistory, m_Histptr );

      if ( fComPlayer )
	comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

      while ( !(m_External->isNull()) &&
	      m_External->sec == m_lastTime.time &&
	      m_External->count == m_lastTime.millitm/10 ) {
	Player *targetPlayer;
	if ( m_External->side == thePlayer->GetSide() )
	  targetPlayer = thePlayer;
	else if ( m_External->side == comPlayer->GetSide() )
	  targetPlayer = comPlayer;
	else {
	  xerror("%s(%d) ExternalData", __FILE__, __LINE__);
	  exit(1);
	}

	m_External->Apply( targetPlayer, fThePlayer, fComPlayer, fTheBall );

	externalOld = m_External;
	m_External = m_External->next;
	delete externalOld;
      }

      m_Histptr++;
      if ( m_Histptr == MAX_HISTORY )
	m_Histptr = 0;

      if ( fTheBall ) {
	m_BacktrackBuffer[m_Histptr].theBall = theBall;
	if ( mode == MODE_SOLOPLAY || mode == MODE_MULTIPLAY ||
	     mode == MODE_PRACTICE ) {
	  m_BacktrackBuffer[m_Histptr].score1 =
	    ((PlayGame *)Control::TheControl())->GetScore(1);
	  m_BacktrackBuffer[m_Histptr].score2 =
	    ((PlayGame *)Control::TheControl())->GetScore(-1);
	}
      }
      if ( fThePlayer )
	CopyPlayerData( m_BacktrackBuffer[m_Histptr].thePlayer, thePlayer );
      if ( fComPlayer )
	CopyPlayerData( m_BacktrackBuffer[m_Histptr].comPlayer, comPlayer );

      m_lastTime.millitm += 10;
      if ( m_lastTime.millitm >= 1000 ) {
	m_lastTime.millitm -= 1000;
	m_lastTime.time++;
      }

      btCount--;
      if ( btCount <= 0 )
	break;
    }

    if (!fTheBall) {
      theBall = m_BacktrackBuffer[m_Histptr].theBall;
      ((PlayGame *)Control::TheControl())->
	ChangeScore( m_BacktrackBuffer[m_Histptr].score1,
		     m_BacktrackBuffer[m_Histptr].score2 );
    }
    if (!fThePlayer)
      thePlayer->Reset( &m_BacktrackBuffer[m_Histptr].thePlayer );
    if (!fComPlayer)
      comPlayer->Reset( &m_BacktrackBuffer[m_Histptr].comPlayer );
  }
}

void
Event::ClearBacktrack() {
  for ( int i = 0 ; i < MAX_HISTORY ; i++ ) {
    Event::TheEvent()->m_MouseXHistory[i] = 0;
    Event::TheEvent()->m_MouseYHistory[i] = 0;
    Event::TheEvent()->m_MouseBHistory[i] = 0;
    Event::TheEvent()->m_BacktrackBuffer[i].sec = m_lastTime.time;
    Event::TheEvent()->m_BacktrackBuffer[i].count = m_lastTime.millitm/10;
    Event::TheEvent()->m_BacktrackBuffer[i].theBall = theBall;
    CopyPlayerData( Event::TheEvent()->m_BacktrackBuffer[i].thePlayer, thePlayer );
    CopyPlayerData( Event::TheEvent()->m_BacktrackBuffer[i].comPlayer, comPlayer );

    Event::TheEvent()->m_BacktrackBuffer[i].score1 = 0;
    Event::TheEvent()->m_BacktrackBuffer[i].score2 = 0;
  }
}

void
QuitGame() {
  printf( "Avg = %f\n", (double)perfs/_perfCount );
  if (_backTrackCount) printf( "BackTrack = %f\n", backTracks/_backTrackCount);

  Event::TheEvent()->m_lastTime.time = 0;
  _perfCount = 0;
  perfs = 0;
  backTracks = 0;
  _backTrackCount = 0;

  SDL_WM_GrabInput( SDL_GRAB_OFF );

  SDL_Event e;
  e.type = SDL_QUIT;
  SDL_PushEvent( &e );
}

void
Event::SetNextMousePointer( long &x, long &y ) {
  if ( mode == MODE_TITLE ) {
    return;
  } else if ( mode == MODE_SELECT || mode == MODE_TRAININGSELECT ||
	      mode == MODE_PRACTICESELECT ) {
    x = BaseView::GetWinWidth()/2 + (x-BaseView::GetWinWidth()/2)*15/16;
  } else {
    x = BaseView::GetWinWidth()/2 + (x-BaseView::GetWinWidth()/2)*15/16;
    y = BaseView::GetWinHeight()/2 + (y-BaseView::GetWinHeight()/2)*15/16;
  }
}
