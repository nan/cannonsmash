/* $Id$ */

// Copyright (C) 2003  神南 吉宏(Kanna Yoshihiro)
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
#include "MultiPlayerSelect.h"
#include "BaseView.h"
#include "Player.h"
#include "Sound.h"
#include "PracticeSelectView.h"
#include "Event.h"
#include "MultiPlay.h"
#include "Network.h"
#include "RCFile.h"

extern long mode;

extern bool isComm;

extern long wins;

extern RCFile *theRC;
extern SDL_mutex *networkMutex;
extern int theSocket;

MultiPlayerSelect::MultiPlayerSelect() {
  m_rotate = 0; m_opponentRotate = 0;
  m_View = NULL;
  m_selected = 0; m_opponentSelected = 0;
}

MultiPlayerSelect::~MultiPlayerSelect() {
}

bool
MultiPlayerSelect::Init() {
  m_View = (PracticeSelectView *)View::CreateView( VIEW_PRACTICESELECT );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  networkMutex = SDL_CreateMutex();
  SDL_CreateThread( MultiPlayerSelect::Connect, NULL );

  return true;
}

void
MultiPlayerSelect::Create() {
  Control::ClearControl();

  m_theControl = new MultiPlayerSelect();
  m_theControl->Init();

  SDL_ShowCursor(0);
}

bool
MultiPlayerSelect::Move( unsigned long *KeyHistory, long *MouseXHistory,
			 long *MouseYHistory, unsigned long *MouseBHistory,
			 int Histptr ) {
  static long lastRotate = 0;

  if ( m_selected > 500 && m_opponentSelected > 500 ) {
    mode = MODE_MULTIPLAY;
    return true;
  }

  if ( MouseBHistory[Histptr]&BUTTON_LEFT && 
       ( (Histptr > 0 && !(MouseBHistory[Histptr-1]&BUTTON_LEFT)) ||
	 (Histptr == 0 && !(MouseBHistory[MAX_HISTORY]&BUTTON_LEFT)) ) ) {
    if ( m_selected == 0 ) {
      m_selected = 1;
      Sound::TheSound()->Play( SOUND_CLICK, 0, 0 );
      SendPT(1);
    }
  }

  if ( m_selected > 0 ) {
    m_selected++;
    return true;
  }

  if ( m_opponentSelected > 0 )
    m_opponentSelected++;

  if ( lastRotate == 0 ) {
    if ( MouseXHistory[Histptr] - BaseView::GetWinWidth()/2 > 10 ) {
      lastRotate = 2;
    } else if ( MouseXHistory[Histptr] - BaseView::GetWinWidth()/2 < -10 ) {
      lastRotate = -2;
    }

    if ( lastRotate != 0 ) {
      m_rotate += lastRotate;
      if ( m_rotate < 0 )
	m_rotate += 360;
      else
	m_rotate %= 360;
    }
  } else {
    long nextRotate = m_rotate + lastRotate;

    if ( nextRotate < 0 )
      nextRotate += 360;
    else
      nextRotate %= 360;

    if ( (m_rotate)/(360/PLAYERS) != nextRotate/(360/PLAYERS) ) {
      m_rotate = (nextRotate+360/PLAYERS/2)/(360/PLAYERS)*(360/PLAYERS);
      lastRotate = 0;
      Sound::TheSound()->Play( SOUND_CLICK, 0, 0 );
      SendPT(0);
    } else
      m_rotate = nextRotate;
  }

  return true;
}

long
MultiPlayerSelect::GetOpponentNum() {
  if ( GetOpponentRotate() < 0 )
    return (360+(GetOpponentRotate()%360))/(360/PLAYERS);
  else
    return (GetOpponentRotate()%360)/(360/PLAYERS);
}

// Make connection with opponent
int
MultiPlayerSelect::Connect( void *dum ) {
  long side;

  if ( !(theRC->serverName[0]) )
    side = 1;	// server side
  else
    side = -1;	// client side

  if ( side == 1 ) {
    WaitForClient();
    ServerAdjustClock();
  } else {
    ConnectToServer();
    ClientAdjustClock();
  }

  SDL_CreateThread( MultiPlay::WaitForData, NULL );

  return 0;
}

void
MultiPlayerSelect::ReadPT( char *data ) {
  long type;

  // get player type
  ReadLong( &(data[1]), type );
  m_opponentRotate = type*(360/PLAYERS);

  if ( data[0] != 0 )
    m_opponentSelected = 1;
}

void
MultiPlayerSelect::SendPT( char fixed ) {
  // Send Player Type

  if ( theSocket < 0 )
    return;

  send( theSocket, "PT", 2, 0 );
  long len = 5;
  SendLong( theSocket, len );
  send( theSocket, &fixed, 1, 0 );
  len = GetPlayerNum();
  SendLong( theSocket, len );
}
