/**
 * @file
 * @brief Implementation of MultiPlay and ExternalData classes. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004  神南 吉宏(Kanna Yoshihiro)
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
#include "MultiPlay.h"
#include "Ball.h"
#include "Player.h"
#include "Event.h"
#include "BaseView.h"
#include "RCFile.h"
#include "Network.h"
#include "MultiPlayerSelect.h"
#include "NetPenAttack.h"
#include "NetPenDrive.h"
#include "NetShakeCut.h"

#ifdef LOGGING
#include "Logging.h"
#endif

#if !defined(WIN32)
#include <netinet/tcp.h>
#endif

#if defined(WIN32) || defined(__FreeBSD__)

typedef int socklen_t;		/* mimic Penguin's typedef */

#endif

extern long mode;
extern Ball theBall;
extern RCFile *theRC;
extern int theSocket;
extern long timeAdj;

extern void QuitGame();

int one=1;

SDL_mutex *networkMutex;

/**
 * Setver setup method. 
 * Currently this method do nothing. All setup are moved to MultiPlayerSelect. 
 */
void
MultiPlay::StartServer() {
}

/**
 * Client setup method. 
 * Currently this method do nothing. All setup are moved to MultiPlayerSelect. 
 */
void
MultiPlay::StartClient() {
  //Event::TheEvent()->SendBall();
}

/**
 * Default constructor. 
 */
MultiPlay::MultiPlay() {
}

/**
 * Destructor. Do nothing. 
 */
MultiPlay::~MultiPlay() {
}

/**
 * Initializer method. 
 * Setup member variable and PlayGameView. 
 * 
 * @return returns true if succeeds. 
 */
bool
MultiPlay::Init() {
#ifdef LOGGING
  Logging::GetLogging()->StartLog();
#endif

  theRC->gameLevel = LEVEL_HARD;
  theRC->gameMode = GAME_11PTS;

  // Init timer again
  struct timeb tb;
  getcurrenttime(&tb);

  Event::m_lastTime = tb;

  m_View = (PlayGameView *)View::CreateView( VIEW_PLAYGAME );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  return true;
}

/**
 * MultiPlay object creater. 
 * This method creates singleton MultiPlay object, and two players. 
 * 
 * @param player type of the player controlled by this game player. 
 * @param com type of the player controlled by the opponent game player. 
 */
void
MultiPlay::Create( long player, long com ) {
  long side;

  Control::ClearControl();

  m_theControl = new MultiPlay();
  m_theControl->Init();

  theBall.Warp( vector3d(0.0), vector3d(0.0), vector2d(0.0), -1000 );

  if ( !(theRC->serverName[0]) )
    side = 1;	// server side
  else
    side = -1;	// client side

  m_thePlayer = Player::Create( player, side, 0 );
  m_comPlayer = Player::Create( com, -side, 0 );

  networkMutex = SDL_CreateMutex();

  if ( side == 1 ) {
    ((MultiPlay *)m_theControl)->StartServer();
  } else {
    ((MultiPlay *)m_theControl)->StartClient();
  }

  m_thePlayer->Init();
  m_comPlayer->Init();
}

/**
 * Move valid objects. 
 * Call Move method of the ball and players. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
MultiPlay::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		 long *MouseYHistory, unsigned long *MouseBHistory,
		 int Histptr ) {
  bool reDraw = false;

  theBall.Move();
  reDraw |= m_thePlayer->Move( KeyHistory, MouseXHistory,
			       MouseYHistory, MouseBHistory, Histptr );
  reDraw |= m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

  return reDraw;
}

/**
 * Set camera position and direction. 
 * 
 * @param srcX camera position [out]
 * @param destX point where the camera is directed. 
 * @return returns true if succeeds. 
 */
bool
MultiPlay::LookAt( vector3d &srcX, vector3d &destX ) {
  if (m_thePlayer) {
    srcX = m_thePlayer->GetX() + m_thePlayer->GetEye();
    destX = m_thePlayer->GetLookAt();
  }

  return true;
}

/**
 * Send adjusted time information to the opponent machine. 
 * Calculate adjusted current time and set it to buf. 
 * 
 * @param buf buffer of which current time is set. Buf must be the pointer to allocated memory of more than 5 bytes long. 
 */
void
MultiPlay::SendTime( char *buf ) {
  char v;
  long sec, count;

  sec = Event::m_lastTime.time;
  count = Event::m_lastTime.millitm/10;

  count += timeAdj;

  while ( count >= 100 ) {
    count -= 100;
    sec++;
  }
  while ( count < 0 ) {
    count += 100;
    sec--;
  }

  memcpy( buf, (char *)&sec, 4 );
  v = (char)(count);
  memcpy( &(buf[4]), (char *)&v, 1 );
}

/**
 * A method to quit game. 
 */
void
MultiPlay::EndGame() {
  QuitGame();
  //mode = MODE_TITLE;
}

/**
 * Network message handler. 
 * This method is invoked as a thread. This method waits for a message from
 * the opponent machine. When a message arrives, this method calls
 * Event::GetExternalData to fetch the message. 
 * 
 * @param dum not used. 
 * @return returns 0. 
 */
int
MultiPlay::WaitForData( void *dum ) {
  fd_set rdfds;

  while (1) {
    FD_ZERO( &rdfds );
    FD_SET( (unsigned int)theSocket, &rdfds );

    if ( select( theSocket+1, &rdfds, NULL, NULL, NULL ) > 0 ) {
      bool ret;
      long side;
      if ( !(theRC->serverName[0]) )
	side = -1;
      else
	side = 1;

      if ( m_comPlayer )
	side = m_comPlayer->GetSide();

      SDL_mutexP( networkMutex );
      ret = Event::TheEvent()->GetExternalData( side );
      SDL_mutexV( networkMutex );

      if ( !ret ) {
	break;
      }
    } else {
      printf( "Select failed\n" );
      break;
    }
  }

  return 0;
}


/**
 * Default constructor. 
 * Initialise member variables to default. 
 */
ExternalData::ExternalData() {
  side = 1;
  dataType = 0;
  sec = 0;
  count = 0;
  memset( data, 0, 256 );
  next = NULL;
}

/**
 * Constructor. 
 * Initialise member variables to default. 
 * 
 * @param s side of the player who sent this message. 
 */
ExternalData::ExternalData( long s ) {
  side = s;
  dataType = 0;
  sec = 0;
  count = 0;
  memset( data, 0, 256 );
  next = NULL;
}

/**
 * Destructor. Do nothing. 
 */
ExternalData::~ExternalData() {
}

/**
 * Read time information. 
 * This method reads the header of incoming message to extract time
 * information.
 * 
 * @param sd socket
 * @param sec time in second [out]
 * @param count time in 1/100 second [out]
 */
void
ExternalData::ReadTime( int sd, long *sec, char *count ) {
  char buf[256];
  long len = 0;
  long ctmp;

  while (1) {
    if ( (len+=recv( sd, buf+len, 5-len, 0 )) == 5 )
      break;
  }
  memcpy( sec, &buf[0], 4 );
  memcpy( count, &buf[4], 1 );

  ctmp = *count;
  ctmp -= timeAdj;

  while ( ctmp >= 100 ) {
    ctmp -= 100;
    (*sec)++;
  }
  while ( ctmp < 0 ) {
    ctmp += 100;
    (*sec)--;
  }
  *count = (char)ctmp;
}

/**
 * Read message type header and message payload. 
 * This method first checks the message header. Referring the header, this
 * method creates appropriate subclass object of ExternalData. Then, this
 * method calls Read() to read message payload. 
 * 
 * @param s side of the player who sent this message. 
 * @return returns created subclass object of ExternalData. If the message header is not valid, returns NULL. 
 */
ExternalData *
ExternalData::ReadData( long s ) {
  char buf[256];
  ExternalData *extNow;

  ReadHeader( theSocket, buf );

  if ( !strncmp( buf, "PV", 2 ) ) {
    extNow = new ExternalPVData(s);
    extNow->Read( theSocket );
  } else if ( !strncmp( buf, "PS", 2 ) ) {
    extNow = new ExternalPSData(s);
    extNow->Read( theSocket );
  } else if ( !strncmp( buf, "BV", 2 ) ) {
    extNow = new ExternalBVData(s);
    extNow->Read( theSocket );
  } else if ( !strncmp( buf, "PT", 2 ) ) {
    extNow = new ExternalPTData(s);
    extNow->Read( theSocket );
  } else if ( !strncmp( buf, "QT", 2 ) ) {
    QuitGame();
    //mode = MODE_TITLE;
    return NULL;
  } else {
    printf( "Comm Error!!\n" );
    return NULL;
  }

  return extNow;
}


/**
 * Default constructor. 
 */
ExternalPVData::ExternalPVData() : ExternalData() {
  dataType = DATA_PV;
}

/**
 * Constructor. 
 */
ExternalPVData::ExternalPVData( long s ) : ExternalData(s) {
  dataType = DATA_PV;
}

/**
 * Apply this ExternalData object to the ball and players. 
 * Referring the PV message, this method moves the player. 
 * 
 * @param targetPlayer the Player object of which this ExternalPVData should be applied. 
 * @param fThePlayer if this method modifies Control::ThePlayer singleton object, this is set to true. [out]
 * @param fComPlayer if this method modifies Control::ComPlayer singleton object, this is set to true. [out]
 * @param fTheBall if this method modifies TheBall singleton object, this is set to true. [out]
 * @return returns true if succeeds. 
 */
bool
ExternalPVData::Apply( Player *targetPlayer, bool &fThePlayer,
		       bool &fComPlayer, bool &fTheBall ) {
  targetPlayer->Warp( data );
  if ( targetPlayer == Control::GetThePlayer() )
    fThePlayer = true;
  else if ( targetPlayer == Control::GetComPlayer() )
    fComPlayer = true;

#ifdef LOGGING
  Logging::GetLogging()->LogRecvPVMessage( this );
#endif

  return true;
}

/**
 * Read incoming message payload. 
 * This method reads payload of incoming PV message and set it to 
 * internal buffer. 
 * 
 * @param sock socket
 * @return returns true if succeeds. 
 */
bool
ExternalPVData::Read( long sock ) {
  ReadTime( sock, &sec, &count );

  long len = 0;
  while (1) {
    if ( (len+=recv( sock, data+len, 48-len, 0 )) == 48 )
      break;
  }

#ifdef LOGGING
  char buf[256];
  Logging::GetLogging()->LogTime( LOG_COMCOMPLAYER );
  sprintf( buf, "Recv PV: %d.%3d\n", (int)sec, (int)count );
  Logging::GetLogging()->Log( LOG_COMCOMPLAYER, buf );
#endif

  return true;
}


/**
 * Default constructor. 
 */
ExternalPSData::ExternalPSData() : ExternalData() {
  dataType = DATA_PS;
}

/**
 * Constructor. 
 */
ExternalPSData::ExternalPSData( long s ) : ExternalData(s) {
  dataType = DATA_PS;
}

/**
 * Apply this ExternalData object to the ball and players. 
 * Referring the PS message, this method changes swing status of the player. 
 * 
 * @param targetPlayer the Player object of which this ExternalPSData should be applied. 
 * @param fThePlayer if this method modifies Control::ThePlayer singleton object, this is set to true. [out]
 * @param fComPlayer if this method modifies Control::ComPlayer singleton object, this is set to true. [out]
 * @param fTheBall if this method modifies TheBall singleton object, this is set to true. [out]
 * @return returns true if succeeds. 
 */
bool
ExternalPSData::Apply( Player *targetPlayer, bool &fThePlayer,
		       bool &fComPlayer, bool &fTheBall ) {
  targetPlayer->ExternalSwing( data );
  if ( targetPlayer == Control::GetThePlayer() )
    fThePlayer = true;
  else if ( targetPlayer == Control::GetComPlayer() )
    fComPlayer = true;

#ifdef LOGGING
  Logging::GetLogging()->LogRecvPSMessage( this );
#endif

  return true;
}

/**
 * Read incoming message payload. 
 * This method reads payload of incoming PS message and set it to 
 * internal buffer. 
 * 
 * @param sock socket
 * @return returns true if succeeds. 
 */
bool
ExternalPSData::Read( long sock ) {
  ReadTime( sock, &sec, &count );

  long len = 0;
  while (1) {
    if ( (len+=recv( sock, data+len, 32-len, 0 )) == 24 )
      break;
  }

#ifdef LOGGING
  char buf[256];
  Logging::GetLogging()->LogTime( LOG_COMCOMPLAYER );
  sprintf( buf, "Recv PS: %d.%3d\n", (int)sec, (int)count );
  Logging::GetLogging()->Log( LOG_COMCOMPLAYER, buf );
#endif

  return true;
}


/**
 * Default constructor. 
 */
ExternalBVData::ExternalBVData() : ExternalData() {
  dataType = DATA_BV;
}

/**
 * Constructor. 
 */
ExternalBVData::ExternalBVData( long s ) : ExternalData(s) {
  dataType = DATA_BV;
}

/**
 * Apply this ExternalData object to the ball and players. 
 * Referring the BV message, this method moves the ball. 
 * 
 * @param targetPlayer the Player object of which this ExternalBVData should be applied. 
 * @param fThePlayer if this method modifies Control::ThePlayer singleton object, this is set to true. [out]
 * @param fComPlayer if this method modifies Control::ComPlayer singleton object, this is set to true. [out]
 * @param fTheBall if this method modifies TheBall singleton object, this is set to true. [out]
 * @return returns true if succeeds. 
 */
bool
ExternalBVData::Apply( Player *targetPlayer, bool &fThePlayer,
		       bool &fComPlayer, bool &fTheBall ) {
  theBall.Warp( data );
  fTheBall = true;

#ifdef LOGGING
  Logging::GetLogging()->LogRecvBVMessage( this );
#endif

  return true;
}

/**
 * Read incoming message payload. 
 * This method reads payload of incoming BV message and set it to 
 * internal buffer. 
 * 
 * @param sock socket
 * @return returns true if succeeds. 
 */
bool
ExternalBVData::Read( long sock ) {
  ReadTime( sock, &sec, &count );

  long len = 0;
  while (1) {
    if ( (len+=recv( sock, data+len, 68-len, 0 )) == 68 )
      break;
  }

#ifdef LOGGING
  char buf[256];
  Logging::GetLogging()->LogTime( LOG_COMBALL );
  sprintf( buf, "Recv BV: %d.%3d\n", (int)sec, (int)count );
  Logging::GetLogging()->Log( LOG_COMBALL, buf );
#endif

  return true;
}


/**
 * Default constructor. 
 */
ExternalPTData::ExternalPTData() : ExternalData() {
  dataType = DATA_PT;
}

/**
 * Constructor. 
 */
ExternalPTData::ExternalPTData( long s ) : ExternalData(s) {
  dataType = DATA_PT;
}

/**
 * Apply this ExternalData object to the ball and players. 
 * Referring the PT message, this method changes point. 
 * 
 * @param targetPlayer the Player object of which this ExternalPTData should be applied. 
 * @param fThePlayer if this method modifies Control::ThePlayer singleton object, this is set to true. [out]
 * @param fComPlayer if this method modifies Control::ComPlayer singleton object, this is set to true. [out]
 * @param fTheBall if this method modifies TheBall singleton object, this is set to true. [out]
 * @return returns true if succeeds. 
 */
bool
ExternalPTData::Apply( Player *targetPlayer, bool &fThePlayer,
		       bool &fComPlayer, bool &fTheBall ) {
  ((MultiPlayerSelect *)Control::TheControl())->ReadPT( data );

#ifdef LOGGING
  Logging::GetLogging()->LogRecvPTMessage( this );
#endif

  return true;
}

/**
 * Read incoming message payload. 
 * This method reads payload of incoming PT message and set it to 
 * internal buffer. 
 * 
 * @param sock socket
 * @return returns true if succeeds. 
 */
bool
ExternalPTData::Read( long sock ) {
  //ReadTime( sock, &sec, &count );

  long len = 0;
  while (1) {
    if ( (len+=recv( sock, data+len, 5-len, 0 )) == 5 )
      break;
  }

#ifdef LOGGING
  char buf[256];
  Logging::GetLogging()->LogTime( LOG_COMMISC );
  sprintf( buf, "Recv PT: \n" );
  Logging::GetLogging()->Log( LOG_COMMISC, buf );
#endif

  return true;
}
