/**
 * @file
 * @brief Implementation of MultiPlayerSelect class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2003, 2004  神南 吉宏(Kanna Yoshihiro)
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

#ifdef LOGGING
#include "Logging.h"
#endif

extern long mode;

extern bool isComm;

extern long wins;

extern RCFile *theRC;
extern SDL_mutex *networkMutex;
extern int theSocket;

/**
 * Default constructor. 
 * Initialize member variables. 
 */
MultiPlayerSelect::MultiPlayerSelect() {
  m_rotate = 0; m_opponentRotate = 0;
  m_View = NULL;
  m_selected = 0; m_opponentSelected = 0;

  m_lastRotate = m_lastOpponentRotate = 0;

  m_isConnected = false;
}

/**
 * Destructor. Do nothing. 
 */
MultiPlayerSelect::~MultiPlayerSelect() {
}

/**
 * Initializer method. 
 * This method creates PracticeSelectView singleton object and create
 * thread to connect to the opponent machine. 
 * 
 * @return returns true if succeeds. 
 */
bool
MultiPlayerSelect::Init() {
  m_View = (PracticeSelectView *)View::CreateView( VIEW_PRACTICESELECT );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  networkMutex = SDL_CreateMutex();
  m_connectThread = SDL_CreateThread( MultiPlayerSelect::Connect, NULL );

  return true;
}

/**
 * Singleton creator method. 
 * This method deletes existing singleton objects and create MultiPlayerSelect
 * singleton object. 
 */
void
MultiPlayerSelect::Create() {
  Control::ClearControl();

  m_theControl = new MultiPlayerSelect();
  m_theControl->Init();
}

/**
 * Move valid objects. 
 * This method moves player panel as the game player moves mouse. 
 * Additionally, this method moves player panel of the opponent. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
MultiPlayerSelect::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
			 long *MouseYHistory, unsigned long *MouseBHistory,
			 int Histptr ) {
  if ( m_selected > 500 && m_opponentSelected > 500 ) {
    mode = MODE_MULTIPLAY;
    return true;
  }

  if ( MouseBHistory[Histptr]&BUTTON_LEFT && 
       ( (Histptr > 0 && !(MouseBHistory[Histptr-1]&BUTTON_LEFT)) ||
	 (Histptr == 0 && !(MouseBHistory[MAX_HISTORY]&BUTTON_LEFT)) ) ) {
    if ( m_selected == 0 ) {
      m_selected = 1;
      Sound::TheSound()->Play( SOUND_CLICK, vector3d(0.0) );
      SendPT(1);
    }
  }

  if ( m_selected > 0 ) {
    if (m_selected > 500 && m_connectThread) {
      int status;
      SDL_WaitThread(m_connectThread, &status);

      if ( status != 0 )
	throw NetworkError();

      m_connectThread = NULL;
    }

    m_selected++;
  } else {
    if ( m_lastRotate == 0 ) {
      if ( MouseXHistory[Histptr] - BaseView::GetWinWidth()/2 > 10 ) {
	m_lastRotate = 2;
      } else if ( MouseXHistory[Histptr] - BaseView::GetWinWidth()/2 < -10 ) {
	m_lastRotate = -2;
      }

      if ( m_lastRotate != 0 ) {
	m_rotate += m_lastRotate;
	if ( m_rotate < 0 )
	  m_rotate += 360;
	else
	  m_rotate %= 360;
	SendPT(0);
      }
    } else {
      long nextRotate = m_rotate + m_lastRotate;

      if ( nextRotate < 0 )
	nextRotate += 360;
      else
	nextRotate %= 360;

      if ( (m_rotate)/(360/PLAYERS) != nextRotate/(360/PLAYERS) ) {
	m_rotate = (nextRotate+360/PLAYERS/2)/(360/PLAYERS)*(360/PLAYERS);
	m_lastRotate = 0;
	Sound::TheSound()->Play( SOUND_CLICK, vector3d(0.0) );
	SendPT(0);
      } else
	m_rotate = nextRotate;
    }
  }

  if ( m_opponentSelected > 0 ) {
    m_opponentSelected++;
  } else {
    if ( m_lastOpponentRotate != 0 ) {
      long nextRotate = m_opponentRotate + m_lastOpponentRotate;

      if ( nextRotate < 0 )
	nextRotate += 360;
      else
	nextRotate %= 360;

      if ( (m_opponentRotate)/(360/PLAYERS) != nextRotate/(360/PLAYERS) ) {
	m_opponentRotate = (nextRotate+360/PLAYERS/2)/(360/PLAYERS)*(360/PLAYERS);
	m_lastOpponentRotate = 0;
      } else
	m_opponentRotate = nextRotate;
    }
  }

  return true;
}

/**
 * Get player ID of opponent which is selected. 
 * 
 * @return return player ID. 
 */
long
MultiPlayerSelect::GetOpponentNum() {
  if ( GetOpponentRotate() < 0 )
    return (360+(GetOpponentRotate()%360))/(360/PLAYERS);
  else
    return (GetOpponentRotate()%360)/(360/PLAYERS);
}

/**
 * Make connection with opponent machine. 
 * If this machine is the server, this method wait for connection from the 
 * client and adjust clock. Otherwise, this method tries to connect to the 
 * server and adjust clock. After that, this method starts a thread to 
 * listen to the message from the opponent machine. 
 * 
 * @param dum not used. 
 * @return returns 0 if succeeds. 
 */
int
MultiPlayerSelect::Connect( void *dum ) {
  long side;

  if ( !(theRC->serverName[0]) )
    side = 1;	// server side
  else
    side = -1;	// client side

  if ( side == 1 ) {
    try {
      WaitForClient();
    } catch ( NetworkError ) {
      xerror("%s(%d) WaitForClient", __FILE__, __LINE__);
      return 1;
    }
    ServerAdjustClock();
  } else {
    try {
      ConnectToServer();
    } catch ( NetworkError ) {
      xerror("%s(%d) ConnectToServer", __FILE__, __LINE__);
      return 1;
    }

    ClientAdjustClock();
  }

  ((MultiPlayerSelect *)m_theControl)->m_isConnected = true;

  if ( ((MultiPlayerSelect *)m_theControl)->m_selected > 0 )
    ((MultiPlayerSelect *)m_theControl)->SendPT(1);
  else
    ((MultiPlayerSelect *)m_theControl)->SendPT(0);

  SDL_CreateThread( MultiPlay::WaitForData, NULL );

  return 0;
}

/**
 * Read PT(player type) message from the opponent machine. 
 * PT message is received when the opponent game player changes player type
 * or fix player type. 
 * Referring PT message, this method sets m_lastOpponentRotate, 
 * m_opponentRotate and m_opponentSelected. 
 * 
 * @param data message payload
 */
void
MultiPlayerSelect::ReadPT( char *data ) {
  long rotate;

  // get player type
  ReadLong( &(data[1]), rotate );

  m_lastOpponentRotate = rotate%(360/PLAYERS);

  while ( m_lastOpponentRotate > 10 )
    m_lastOpponentRotate -= (360/PLAYERS);

  while ( m_lastOpponentRotate < -10 )
    m_lastOpponentRotate += (360/PLAYERS);

  m_opponentRotate = rotate;

  if ( data[0] != 0 )
    m_opponentSelected = 1;
}

/**
 * Send PT(player type) message to the opponent machine. 
 * PT message is sent when the game player changes player type or fix player
 * type. 
 * 
 * @param fixed if player type is fixed, this parameter is 1. Otherwise it is 0. 
 */
void
MultiPlayerSelect::SendPT( char fixed ) {
  long rotate;

  if ( theSocket < 0 || !m_isConnected )
    return;

  send( theSocket, "PT", 2, 0 );
  send( theSocket, &fixed, 1, 0 );
  rotate = GetRotate();
  SendLong( theSocket, rotate );

#ifdef LOGGING
  char buf[256];

  sprintf( buf, "fixed=%d rotate=%d\n", fixed, rotate );
  Logging::GetLogging()->LogTime( LOG_COMMISC );
  Logging::GetLogging()->Log( LOG_COMMISC, buf );
#endif
}
