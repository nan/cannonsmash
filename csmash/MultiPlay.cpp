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
#include "MultiPlay.h"
#include "Ball.h"
#include "Player.h"
#include "PenAttack.h"
#include "PenDrive.h"
#include "ShakeCut.h"
#include "Event.h"
#include "BaseView.h"
#include "RCFile.h"

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
extern Player *thePlayer;
extern Player *comPlayer;
extern BaseView* theView;

extern RCFile *theRC;

extern long timeAdj;

extern short csmash_port;
extern int theSocket;
bool endian;

int listenSocket = 0;

extern void QuitGame();

int one=1;

// convert endian
double
SwapDbl( double d ) {
  if ( endian ) {
    return d;
  } else {
    double swp;
    char *c1 = (char *)&d, *c2 = (char *)&swp;

    for ( int i = 0 ; i < 8 ; i++ )
      *(c2+7-i) = *(c1+i);

    return swp;
  }
}

long
SwapLong( long l ) {
  if ( endian ) {
    return l;
  } else {
    long swp;
    char *c1 = (char *)&l, *c2 = (char *)&swp;

    for ( int i = 0 ; i < 4 ; i++ )
      *(c2+3-i) = *(c1+i);

    return swp;
  }
}

// test endian
void
EndianCheck() {
  long n = 1;
  if ( *((char *)&n) == 1 )
    endian = false;
  else
    endian = true;
}

bool
SendDouble( int sd, double d ) {
  d = SwapDbl(d);

  if ( send( sd, (char *)&d, 8, 0 ) == 8 )
    return true;
  else
    return false;
}

bool
SendLong( int sd, long l ) {
  l = SwapLong(l);

  if ( send( sd, (char *)&l, 4, 0 ) == 4 )
    return true;
  else
    return false;
}

// It seems to be strange...
char *
ReadDouble( char *buf, double& d ) {
  memcpy( &d, buf, 8 );
  d = SwapDbl(d);

  return buf+8;
}

char *
ReadLong( char *buf, long& l ) {
  memcpy( &l, buf, 4 );
  l = SwapLong(l);

  return buf+4;
}

void
SendTime( int sd, struct timeb* tb ) {
  long millitm = tb->millitm;
  send( sd, "TM", 2, 0 );
  SendLong( sd, tb->time );
  SendLong( sd, millitm );
#ifdef LOGGING
  Logging::GetLogging()->LogTime( LOG_COMMISC, tb );
  Logging::GetLogging()->Log( LOG_COMMISC, "SendTime\n" );
#endif
}

void
ReadTime( int sd, struct timeb* tb ) {
  char buf[256];
  long len;

  if ( recv( sd, buf, 2, 0 ) != 2 ) {
    xerror("%s(%d) recv", __FILE__, __LINE__);
    exit(1);
  }
  len = 0;
  while (1) {
    if ( (len+=recv( sd, buf+len, 8-len, 0 )) == 8 )
      break;
  }

  char *b = buf;
  long millitm;
  b = ReadLong( b, tb->time );
  b = ReadLong( b, millitm );
  tb->millitm = millitm;

#ifdef LOGGING
  struct timeb tbCurrent;

#ifndef WIN32
  struct timeval tv;
  struct timezone tz;
#endif

#ifdef WIN32
  ftime( &tbCurrent );
#else
  gettimeofday( &tv, &tz );
  tbCurrent.time = tv.tv_sec;
  tbCurrent.millitm = tv.tv_usec/1000;
#endif

  Logging::GetLogging()->LogTime( LOG_COMMISC, &tbCurrent );
  Logging::GetLogging()->Log( LOG_COMMISC, "ReadTime " );
  Logging::GetLogging()->LogTime( LOG_COMMISC, tb );
  Logging::GetLogging()->Log( LOG_COMMISC, "\n" );
#endif
}

// Send PlayerData
void
SendPlayerData() {
  send( theSocket, "PI", 2, 0 );
  thePlayer->SendAll( theSocket );

#ifdef LOGGING
  Logging::GetLogging()->LogPlayer( LOG_COMTHEPLAYER, thePlayer );
#endif

}

// Recv PlayerData
Player *
ReadPlayerData() {
  double x, y, z, vx, vy, vz, spin;
  long playerType, side, swing, swingType, swingSide, afterSwing, swingError, pow, statusMax;
  double targetX, targetY, eyeX, eyeY, eyeZ, stamina;
  long stat;
  char buf[256];
  long len;
  Player *player;

  if ( recv( theSocket, buf, 2, 0 ) != 2 )
    return 0;

  if ( strncmp( buf, "PI", 2 ) )
    return 0;

  len = 0;
  while (1) {
    if ( (len+=recv( theSocket, buf+len, 136-len, 0 )) == 136 )
      break;
  }

  char *b = buf;

  b = ReadLong( b, playerType );
  b = ReadLong( b, side );

  b = ReadDouble( b, x );
  b = ReadDouble( b, y );
  b = ReadDouble( b, z );
  b = ReadDouble( b, vx );
  b = ReadDouble( b, vy );
  b = ReadDouble( b, vz );

  b = ReadLong( b, stat );
  b = ReadLong( b, swing );
  b = ReadLong( b, swingType );
  b = ReadLong( b, swingSide );
  b = ReadLong( b, afterSwing );
  b = ReadLong( b, swingError );

  b = ReadDouble( b, targetX );
  b = ReadDouble( b, targetY );
  b = ReadDouble( b, eyeX );
  b = ReadDouble( b, eyeY );
  b = ReadDouble( b, eyeZ );

  b = ReadLong( b, pow );
  b = ReadDouble( b, spin );
  b = ReadDouble( b, stamina );
  b = ReadLong( b, statusMax );

  switch ( playerType ) {
  case PLAYER_PENATTACK:
    player = new PenAttack( playerType, side, x, y, z, vx, vy, vz, stat,
			    swing, swingType, (bool)swingSide, afterSwing,
			    swingError,
			    targetX, targetY, eyeX, eyeY, eyeZ, pow,
			    spin, stamina, statusMax );
    break;
  case PLAYER_SHAKECUT:
    player = new ShakeCut( playerType, side, x, y, z, vx, vy, vz, stat,
			   swing, swingType, (bool)swingSide, afterSwing,
			   swingError,
			   targetX, targetY, eyeX, eyeY, eyeZ, pow,
			   spin, stamina, statusMax );
    break;
  case PLAYER_PENDRIVE:
    player = new PenDrive( playerType, side, x, y, z, vx, vy, vz, stat,
			   swing, swingType, (bool)swingSide, afterSwing,
			   swingError,
			   targetX, targetY, eyeX, eyeY, eyeZ, pow,
			   spin, stamina, statusMax );
    break;
  default:
    return 0;
  }

#ifdef LOGGING
  Logging::GetLogging()->LogPlayer( LOG_COMCOMPLAYER, player );
#endif

  return player;
}

bool
AcceptClient() {
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
    saddr.sin_port = htons(csmash_port);
    if ( bind( listenSocket, (struct sockaddr *)&saddr, sizeof(saddr) ) < 0 ) {
      xerror("%s(%d) bind", __FILE__, __LINE__);
      exit(1);
    }

    if ( listen( listenSocket, 1 ) < 0 ) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      exit(1);
    }
  }

  int sb;
  struct sockaddr_in sba;
  if (0 > (sb = socket(PF_INET, SOCK_DGRAM, 0))) {
    xerror("%s(%d) socket", __FILE__, __LINE__);
    exit(1);
  }
  sba.sin_addr.s_addr = INADDR_ANY;
  sba.sin_family = AF_INET;
  sba.sin_port = htons(csmash_port);
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
StartServer() {
  int i, j;
  char buf[256];
  long len;

  AcceptClient();

  // Adjust timer
  long adjLog[16];
  for ( i = 0 ; i < 16 ; i++ ) {
#ifndef WIN32
    struct timeval tv;
    struct timezone tz;
#endif
    struct timeb tb1, tb2, tb3;
    long diff;

#ifdef WIN32
    ftime( &tb1 );
#else
    gettimeofday( &tv, &tz );
    tb1.time = tv.tv_sec;
    tb1.millitm = tv.tv_usec/1000;
#endif

    SendTime( theSocket, &tb1 );
    ReadTime( theSocket, &tb3 );

#ifdef WIN32
    ftime( &tb2 );
#else
    gettimeofday( &tv, &tz );
    tb2.time = tv.tv_sec;
    tb2.millitm = tv.tv_usec/1000;
#endif

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
    tb1.millitm = mtm;

    //timeAdj += (tb3.time-tb1.time)*1000 + tb3.millitm-tb1.millitm;
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
  timeAdj = 0;
  for ( i = 4 ; i < 12 ; i++ ) {
    timeAdj += adjLog[i];
  }

  timeAdj /= 80;	/* 8*10 */

  printf( "%d\n", timeAdj );

  // Read Ball Data
  if ( recv( theSocket, buf, 2, 0 ) != 2 ) {
    xerror("%s(%d) recv", __FILE__, __LINE__);
    exit(1);
  }

  if ( strncmp( buf, "BI", 2 ) ) {
    xerror("%s(%d) recv BI", __FILE__, __LINE__);
    exit(1);
  }

  len = 0;
  while (1) {
    if ( (len+=recv( theSocket, buf+len, 60-len, 0 )) == 60 )
      break;
  }

#ifdef LOGGING
  char logBuf[256];
  Logging::GetLogging()->LogTime( LOG_COMBALL );
  sprintf( logBuf, "recv BI: " );
  Logging::GetLogging()->Log( LOG_COMBALL, logBuf );
#endif

  theBall.Warp( buf );

#ifdef LOGGING
  sprintf( buf, "x=%4.2f y=%4.2f z=%4.2f vx=%4.2f vy=%4.2f vz=%4.2f spin=%3.2f status=%2d\n",
	   theBall.GetX(), theBall.GetY(), theBall.GetZ(), 
	   theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(), 
	   theBall.GetSpin(), (int)theBall.GetStatus() );
  Logging::GetLogging()->Log( LOG_COMBALL, buf );
#endif

  SendPlayerData();

  if ( !(comPlayer = ReadPlayerData()) ) {
    xerror("%s(%d) ReadPlayerData", __FILE__, __LINE__);
    exit(1);
  }
}

void
StartClient() {
  char buf[128];

  struct sockaddr_in saddr;
  memset(&saddr, 0, sizeof(saddr));

  if (1 == theRC->serverName[0]) { // Broadcast mode
    struct sockaddr_in sba;
    memset(&sba, 0, sizeof(sba));
    int sb;
    if (0 > (sb = socket(PF_INET, SOCK_DGRAM, 0))) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      exit(1);
    }
    setsockopt( sb, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int) );

    int one = 1;
    setsockopt(sb, SOL_SOCKET, SO_BROADCAST, (char*)&one, sizeof(one));
    sba.sin_family = AF_INET;
    sba.sin_addr.s_addr = INADDR_ANY;
    sba.sin_port = 0;
    if (0 != bind(sb, (sockaddr*)&sba, sizeof(sba))) {
      xerror("%s(%d) bind", __FILE__, __LINE__);
      exit(1);
    }

    int x = 0;
    do {
      char buf[8];
      memset(&sba, 0, sizeof(sba));
      sba.sin_family = AF_INET;
      sba.sin_addr.s_addr = INADDR_BROADCAST;
      sba.sin_port = htons(csmash_port);

      sendto(sb, buf, 0, 0, (sockaddr*)&sba, sizeof(sba));
      fd_set fd;
      FD_ZERO(&fd);
      FD_SET(sb, &fd);
      timeval tv;
      memset(&tv, 0, sizeof(tv));
      tv.tv_sec = 1;
      if (0 < select(sb+1, &fd, 0, 0, &tv)) {
	socklen_t l = sizeof(sba);
	int i;
	i = recvfrom(sb, buf, sizeof(buf), 0, (sockaddr*)&sba, &l);
	printf("recvfrom %d %s\n", i, inet_ntoa(sba.sin_addr));      
	break;
      } else {
	printf("%d ", x);
      }
    } while (x++ < 30);
    closesocket(sb);
    if (30 <= x) {
      // notfound
      printf("timeout\n");
      exit(1);
    }
    saddr.sin_addr.s_addr = sba.sin_addr.s_addr;
  } else {
    struct hostent *hent;
    hent = gethostbyname( theRC->serverName );
    memcpy( &saddr.sin_addr, hent->h_addr, hent->h_length );
  }

  printf("server is %s\n", inet_ntoa(saddr.sin_addr));
  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(csmash_port);

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

  // Adjust timer
  struct timeb tb;
#ifndef WIN32
  struct timeval tv;
  struct timezone tz;
#endif

  for ( i = 0 ; i < 16 ; i++ ) {
    ReadTime( theSocket, &tb );	// Dispose

#ifdef WIN32
    ftime( &tb );
#else
    gettimeofday( &tv, &tz );
    tb.time = tv.tv_sec;
    tb.millitm = tv.tv_usec/1000;
#endif

    SendTime( theSocket, &tb );
  }

  // Send Ball Data
#ifdef LOGGING
  char logBuf[256];
  Logging::GetLogging()->LogTime( LOG_COMBALL );
  sprintf( logBuf, "send BI\n" );
  Logging::GetLogging()->Log( LOG_COMBALL, logBuf );
#endif
  send( theSocket, "BI", 2, 0 );
  theBall.Send( buf );
  send( theSocket, buf, 60, 0 );

//      printf( "read\n" );
  if ( !(comPlayer = ReadPlayerData()) ) {
    xerror("%s(%d) ReadPlayerData", __FILE__, __LINE__);
    exit(1);
  }
//      printf( "write\n" );
  SendPlayerData();
}

MultiPlay::MultiPlay() {
}

MultiPlay::~MultiPlay() {
}

bool
MultiPlay::Init() {
#ifdef LOGGING
  Logging::GetLogging()->StartLog();
#endif
  m_View = new PlayGameView();

  m_View->Init( this );

  theView->AddView( m_View );

  return true;
}

MultiPlay*
MultiPlay::Create( long player, long com ) {
  MultiPlay *newMultiPlay;
  long side;

  Event::ClearObject();

  newMultiPlay = new MultiPlay();
  newMultiPlay->Init();

  if ( !(theRC->serverName[0]) )
    side = 1;		// server side
  else
    side = -1;	// client side

  if ( thePlayer == NULL ) {
    thePlayer = Player::Create( player, side, 0 );
  }

  if ( side == 1 ) {
    StartServer();
  } else {
    StartClient();
  }

  thePlayer->Init();
  comPlayer->Init();

  SDL_ShowCursor(0);
  SDL_WM_GrabInput( SDL_GRAB_ON );

  theRC->gameLevel = LEVEL_HARD;
  theRC->gameMode = GAME_21PTS;

  return newMultiPlay;
}

bool
MultiPlay::Move( unsigned long *KeyHistory, long *MouseXHistory,
		 long *MouseYHistory, unsigned long *MouseBHistory,
		 int Histptr ) {
  bool reDraw = false;

  if ( KeyHistory[Histptr] == SDLK_ESCAPE ) {
    if ( SDL_WM_GrabInput( SDL_GRAB_QUERY ) == SDL_GRAB_ON )
      SDL_WM_GrabInput( SDL_GRAB_OFF );
    else
      SDL_WM_GrabInput( SDL_GRAB_ON );
  }

#ifdef LOGGING
  long prevStatus = theBall.GetStatus();
  theBall.Move();

  double prevVx = thePlayer->GetVX();
  double prevVy = thePlayer->GetVY();
  long   prevSwing = thePlayer->GetSwing();
  reDraw |= thePlayer->Move( KeyHistory, MouseXHistory,
			     MouseYHistory, MouseBHistory, Histptr );
  if ( prevVx != thePlayer->GetVX() || prevVy != thePlayer->GetVY() ||
       prevSwing != thePlayer->GetSwing() ) {
    Logging::GetLogging()->LogPlayer( LOG_ACTTHEPLAYER, thePlayer );
  }

  prevVx = comPlayer->GetVX();
  prevVy = comPlayer->GetVY();
  prevSwing = comPlayer->GetSwing();
  reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
  if ( prevVx != comPlayer->GetVX() || prevVy != comPlayer->GetVY() ||
       prevSwing != comPlayer->GetSwing() ) {
    Logging::GetLogging()->LogPlayer( LOG_ACTCOMPLAYER, comPlayer );
  }

  if ( prevStatus != theBall.GetStatus() && prevStatus >= -1 ) {
    Logging::GetLogging()->LogBall( LOG_ACTBALL, &theBall );
  }
#else
  theBall.Move();
  reDraw |= thePlayer->Move( KeyHistory, MouseXHistory,
			     MouseYHistory, MouseBHistory, Histptr );
  reDraw |= comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
#endif

  return reDraw;
}

bool
MultiPlay::LookAt( double &srcX, double &srcY, double &srcZ,
		   double &destX, double &destY, double &destZ ) {
  if (thePlayer) {
    srcX = thePlayer->GetX() + thePlayer->GetEyeX();
    srcY = thePlayer->GetY() + thePlayer->GetEyeY();
    srcZ = thePlayer->GetZ() + thePlayer->GetEyeZ();
    destX = thePlayer->GetLookAtX();
    destY = thePlayer->GetLookAtY();
    destZ = thePlayer->GetLookAtZ();
  }

  return true;
}


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

void
MultiPlay::EndGame() {
  QuitGame();
  mode = MODE_TITLE;
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
    mode = MODE_TITLE;
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
  if ( targetPlayer == thePlayer )
    fThePlayer = true;
  else if ( targetPlayer == comPlayer )
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
  if ( targetPlayer == thePlayer )
    fThePlayer = true;
  else if ( targetPlayer == comPlayer )
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
