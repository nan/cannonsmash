/* $Id$ */

// Copyright (C) 2000, 2001, 2002  神南 吉宏(Kanna Yoshihiro)
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

#ifdef LOGGING
#include "Logging.h"
#endif

#if !defined(WIN32)
#include <netinet/tcp.h>
#endif

#if defined(WIN32) || defined(__FreeBSD__)

typedef int socklen_t;		/* mimic Penguin's typedef */

#else	/* ! WIN32 */

#define closesocket(FD) close(FD)

#endif

extern long mode;

extern Ball theBall;

extern RCFile *theRC;

extern int theSocket;

unsigned int listenSocket = 0;

extern void QuitGame();

int one=1;

SDL_mutex *networkMutex;

void
MultiPlay::StartServer() {

  WaitForClient();
  AdjustClock();
  ReadBI();

  SendPlayerData();

  if ( !(m_comPlayer = ReadPlayerData()) ) {
    xerror("%s(%d) ReadPlayerData", __FILE__, __LINE__);
    exit(1);
  }
}

void
MultiPlay::StartClient() {
  char buf[128];

  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));

  findhostname(&saddr);

  printf("server is %s\n", inet_ntoa(saddr.sin_addr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(theRC->csmash_port);

  // connect
  if ( (theSocket = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {
    xerror("%s(%d) socket", __FILE__, __LINE__);
    exit(1);
  }
  setsockopt( theSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int) );

  int i;
  for ( i = 0 ; i < 10 ; i++ ) {
    if ( !connect( theSocket, (struct sockaddr *)&saddr, sizeof(saddr) ) )
      break;
#ifdef WIN32
    Sleep(3000);
#else
    sleep(3);
#endif
  }

  if ( i == 10 ) {
    xerror("%s(%d) connect", __FILE__, __LINE__);
    exit(1);
  }

  // Respond server's AdjustClock()
  for ( i = 0 ; i < 16 ; i++ ) {
    struct timeb tb;

    ReadTime( theSocket, &tb );	// Dispose
    getcurrenttime( &tb );
    ::SendTime( theSocket, &tb );
  }

  // Send Ball Data
#ifdef LOGGING
  Logging::GetLogging()->LogTime( LOG_COMBALL );
  Logging::GetLogging()->Log( LOG_COMBALL, "send BI\n" );
#endif

  send( theSocket, "BI", 2, 0 );
  theBall.Send( buf );
  send( theSocket, buf, 60, 0 );

  // exchange player data
  if ( !(m_comPlayer = ReadPlayerData()) ) {
    xerror("%s(%d) ReadPlayerData", __FILE__, __LINE__);
    exit(1);
  }
  SendPlayerData();
}

MultiPlay::MultiPlay() {
  m_timeAdj = 0;
}

MultiPlay::~MultiPlay() {
}

bool
MultiPlay::Init() {
#ifdef LOGGING
  Logging::GetLogging()->StartLog();
#endif
  m_View = (PlayGameView *)View::CreateView( VIEW_PLAYGAME );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  return true;
}

void
MultiPlay::Create( long player, long com ) {
  long side;

  Control::ClearControl();

  m_theControl = new MultiPlay();
  m_theControl->Init();

  if ( !(theRC->serverName[0]) )
    side = 1;		// server side
  else
    side = -1;	// client side

  if ( m_thePlayer == NULL ) {
    m_thePlayer = Player::Create( player, side, 0 );
  }

  if ( side == 1 ) {
    ((MultiPlay *)m_theControl)->StartServer();
  } else {
    ((MultiPlay *)m_theControl)->StartClient();
  }

  m_thePlayer->Init();
  m_comPlayer->Init();

  SDL_ShowCursor(SDL_DISABLE);
  SDL_WM_GrabInput( SDL_GRAB_ON );

  theRC->gameLevel = LEVEL_HARD;
  theRC->gameMode = GAME_11PTS;

  // Init timer again
  struct timeb tb;

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

  Event::m_lastTime = tb;

  networkMutex = SDL_CreateMutex();
  SDL_CreateThread( MultiPlay::WaitForData, NULL );
}

bool
MultiPlay::Move( unsigned long *KeyHistory, long *MouseXHistory,
		 long *MouseYHistory, unsigned long *MouseBHistory,
		 int Histptr ) {
  bool reDraw = false;

  theBall.Move();
  reDraw |= m_thePlayer->Move( KeyHistory, MouseXHistory,
			     MouseYHistory, MouseBHistory, Histptr );
  reDraw |= m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

  return reDraw;
}

bool
MultiPlay::LookAt( double &srcX, double &srcY, double &srcZ,
		   double &destX, double &destY, double &destZ ) {
  if (m_thePlayer) {
    srcX = m_thePlayer->GetX() + m_thePlayer->GetEyeX();
    srcY = m_thePlayer->GetY() + m_thePlayer->GetEyeY();
    srcZ = m_thePlayer->GetZ() + m_thePlayer->GetEyeZ();
    destX = m_thePlayer->GetLookAtX();
    destY = m_thePlayer->GetLookAtY();
    destZ = m_thePlayer->GetLookAtZ();
  }

  return true;
}


void
MultiPlay::SendTime( char *buf ) {
  char v;
  long sec, count;

  sec = Event::m_lastTime.time;
  count = Event::m_lastTime.millitm/10;

  count += m_timeAdj;

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

void
MultiPlay::EndGame() {
  QuitGame();
  //mode = MODE_TITLE;
}

bool
MultiPlay::WaitForClient() {
  socklen_t fromlen;
  struct sockaddr_in saddr, faddr;

  if ( listenSocket == 0 ) {
    if ( (listenSocket = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      exit(1);
    }

    setsockopt( listenSocket, IPPROTO_TCP, TCP_NODELAY,
		(char*)&one, sizeof(int) );

    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(theRC->csmash_port);
    if ( bind( listenSocket, (struct sockaddr *)&saddr, sizeof(saddr) ) < 0 ) {
      xerror("%s(%d) bind", __FILE__, __LINE__);
      exit(1);
    }

    if ( listen( listenSocket, 1 ) < 0 ) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      exit(1);
    }
  }

  unsigned int sb;
  struct sockaddr_in sba;
  if (0 > (sb = socket(PF_INET, SOCK_DGRAM, 0))) {
    xerror("%s(%d) socket", __FILE__, __LINE__);
    exit(1);
  }
  sba.sin_addr.s_addr = INADDR_ANY;
  sba.sin_family = AF_INET;
  sba.sin_port = htons(theRC->csmash_port);
  if (0 > bind(sb, (struct sockaddr*)&sba, sizeof(sba))) {
    xerror("%s(%d) bind", __FILE__, __LINE__);
    exit(1);
  }

  // wait for connection / broadcast packet
  printf("server selecting\n");
  do {
    int max;

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(listenSocket, &fd);
    FD_SET(sb, &fd);

    max = listenSocket > sb ? listenSocket : sb;
    if (0 <= select(max+1, &fd, NULL, NULL, NULL)) {
      if (FD_ISSET(sb, &fd)) {
	// datagram to udp port
	printf("server recived broadcast packet");
	char buf[1];
	sockaddr_in client;
	socklen_t l = sizeof(client);

	if ( 0 <= recvfrom(sb, (char*)&buf, sizeof(buf), 0,
			  (sockaddr*)&client, &l)) {
	  printf("%s\n", inet_ntoa(client.sin_addr));
	  struct hostent *h;
	  char buf[128];
	  if (0 == gethostname(buf, sizeof(buf))) {
	    h = gethostbyname(buf);
	    if (h) {
	      sockaddr_in a;
	      memcpy(&a.sin_addr, h->h_addr, h->h_length);
	      printf("send %s\n", inet_ntoa(a.sin_addr));
	      l = sizeof(client);
	      sendto(sb, (char*)&a.sin_addr.s_addr, sizeof(a.sin_addr.s_addr),
		     0, (sockaddr*)&client, l);
	    }
	  }
	}
      }
      else {
	// connection to tcp port
	break;
      }
    } else {
      // select error... No, Timeout
      xerror("%s(%d): select");
    }
  } while (1);
  closesocket(sb);

  fromlen = sizeof(faddr);
  theSocket = accept( listenSocket, (struct sockaddr *)&faddr, &fromlen );
  setsockopt( theSocket, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int) );

  if (0 > theSocket) {
    xerror("%s(%d) accept", __FILE__, __LINE__);
    exit(1);
  }

  return true;
}

void
MultiPlay::AdjustClock() {
  long adjLog[16];
  int i, j;

  for ( i = 0 ; i < 16 ; i++ ) {
    struct timeb tb1, tb2, tb3;
    long diff;

    getcurrenttime( &tb1 );

    ::SendTime( theSocket, &tb1 );
    ReadTime( theSocket, &tb3 );

    getcurrenttime( &tb2 );

    diff = ((long)(tb2.time-tb1.time)*1000 + tb2.millitm-tb1.millitm)/2;
    long mtm = tb1.millitm+diff;
    while ( mtm >= 1000 ) {
      mtm -= 1000;
      tb1.time++;
    }
    while ( mtm < 1000 ) {
      mtm += 1000;
      tb1.time--;
    }
    tb1.millitm = (unsigned short)mtm;

    adjLog[i] = (tb3.time-tb1.time)*1000 + tb3.millitm-tb1.millitm;
  }

  // Bubble sort
  i = 0;
  while ( i == 0 ) {
    i = 1;
    for ( j = 0 ; j < 15 ; j++ ) {
      if ( adjLog[j] > adjLog[j+1] ) {
	long k = adjLog[j];
	adjLog[j] = adjLog[j+1];
	adjLog[j+1] = k;
	i = 0;
      }
    }
  }

  for ( i = 0 ; i < 16 ; i++ ) {
    printf( "%d ", adjLog[i] );
  }
  printf( "\n" );

  // Use 8 medium value
  m_timeAdj = 0;
  for ( i = 4 ; i < 12 ; i++ ) {
    m_timeAdj += adjLog[i];
  }

  m_timeAdj /= 80;	/* 8*10 */

  printf( "%d\n", m_timeAdj );
}

int
MultiPlay::WaitForData( void *dum ) {
  fd_set rdfds;

  while (1) {
    FD_ZERO( &rdfds );
    FD_SET( (unsigned int)theSocket, &rdfds );

    if ( select( theSocket+1, &rdfds, NULL, NULL, NULL ) > 0 ) {
      bool ret;

      SDL_mutexP( networkMutex );
      ret = Event::TheEvent()->GetExternalData( m_comPlayer->GetSide() );
      SDL_mutexV( networkMutex );

      if ( !ret ) {
	printf( "GetExternalData failed\n" );
	break;
      } else {
	printf( "GetExternalData succeeded\n" );
      }
    } else {
      printf( "Select failed\n" );
      break;
    }
  }

  return 0;
}


ExternalData::ExternalData() {
  side = 1;
  dataType = 0;
  sec = 0;
  count = 0;
  memset( data, 0, 256 );
  next = NULL;
}

ExternalData::ExternalData( long s ) {
  side = s;
  dataType = 0;
  sec = 0;
  count = 0;
  memset( data, 0, 256 );
  next = NULL;
}

ExternalData::~ExternalData() {
}

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
  ctmp -= ((MultiPlay *)Control::TheControl())->GetTimeAdj();

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

ExternalData *
ExternalData::ReadData( long s ) {
  char buf[256];
  ExternalData *extNow;

  if ( recv( theSocket, buf, 2, 0 ) != 2 ) {
    return NULL;
  }

  if ( !strncmp( buf, "PV", 2 ) ) {
    extNow = new ExternalPVData(s);
    extNow->Read( theSocket );
  } else if ( !strncmp( buf, "PS", 2 ) ) {
    extNow = new ExternalPSData(s);
    extNow->Read( theSocket );
  } else if ( !strncmp( buf, "BV", 2 ) ) {
    extNow = new ExternalBVData(s);
    extNow->Read( theSocket );
  } else if ( !strncmp( buf, "QT", 2 ) ) {
    QuitGame();
    //mode = MODE_TITLE;
    return NULL;
  } else
    return NULL;

  return extNow;
}


ExternalPVData::ExternalPVData() : ExternalData() {
  dataType = DATA_PV;
}

ExternalPVData::ExternalPVData( long s ) : ExternalData(s) {
  dataType = DATA_PV;
}

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


ExternalPSData::ExternalPSData() : ExternalData() {
  dataType = DATA_PS;
}

ExternalPSData::ExternalPSData( long s ) : ExternalData(s) {
  dataType = DATA_PS;
}

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

bool
ExternalPSData::Read( long sock ) {
  ReadTime( sock, &sec, &count );

  long len = 0;
  while (1) {
    if ( (len+=recv( sock, data+len, 24-len, 0 )) == 24 )
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


ExternalBVData::ExternalBVData() : ExternalData() {
  dataType = DATA_BV;
}

ExternalBVData::ExternalBVData( long s ) : ExternalData(s) {
  dataType = DATA_BV;
}

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

bool
ExternalBVData::Read( long sock ) {
  ReadTime( sock, &sec, &count );

  long len = 0;
  while (1) {
    if ( (len+=recv( sock, data+len, 60-len, 0 )) == 60 )
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
