/* $Id$ */

// Copyright (C) 2001, 2002  神南 吉宏(Kanna Yoshihiro)
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
#include "Network.h"
#include "MultiPlay.h"
#include "Event.h"
#include "Ball.h"
#include "Player.h"
#include "NetPenAttack.h"
#include "NetPenDrive.h"
#include "NetShakeCut.h"
#include "RCFile.h"
#include "Control.h"

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

bool endian;
extern int theSocket;
extern Ball theBall;
extern RCFile *theRC;
extern long mode;

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

// Send time using "TM" protocol
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
  tb->millitm = (unsigned short)millitm;

#ifdef LOGGING
  struct timeb tbCurrent;

  getcurrenttime( &tbCurrent );

  Logging::GetLogging()->LogTime( LOG_COMMISC, &tbCurrent );
  Logging::GetLogging()->Log( LOG_COMMISC, "ReadTime " );
  Logging::GetLogging()->LogTime( LOG_COMMISC, tb );
  Logging::GetLogging()->Log( LOG_COMMISC, "\n" );
#endif
}

// Send PlayerData using "PI" protocol
void
SendPlayerData() {
  send( theSocket, "PI", 2, 0 );
  Control::TheControl()->GetThePlayer()->SendAll( theSocket );

#ifdef LOGGING
  Logging::GetLogging()->LogPlayer( LOG_COMTHEPLAYER,
				    Control::TheControl()->GetThePlayer() );
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
    if ( (len+=recv( theSocket, buf+len, 144-len, 0 )) == 144 )
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

  bool bSwingSide = bool(swingSide != 0);
  switch ( playerType ) {
  case PLAYER_PENATTACK:
    player = new NetPenAttack( playerType, side, x, y, z, vx, vy, vz, stat,
			       swing, swingType, bSwingSide, afterSwing,
			       swingError,
			       targetX, targetY, eyeX, eyeY, eyeZ, pow,
			       spin, stamina, statusMax );
    break;
  case PLAYER_SHAKECUT:
    player = new NetShakeCut( playerType, side, x, y, z, vx, vy, vz, stat,
			      swing, swingType, bSwingSide, afterSwing,
			      swingError,
			      targetX, targetY, eyeX, eyeY, eyeZ, pow,
			      spin, stamina, statusMax );
    break;
  case PLAYER_PENDRIVE:
    player = new NetPenDrive( playerType, side, x, y, z, vx, vy, vz, stat,
			      swing, swingType, bSwingSide, afterSwing,
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

// Send PlayerSwing using "PS" and "PV" protocol
void
SendSwing( Player *player ) {
  char buf[256];

  if ( Event::TheEvent()->IsBackTracking() || mode != MODE_MULTIPLAY )
    return;

  strncpy( buf, "PS", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[2]) );

  player->SendSwing( &(buf[7]) );

  // Player 位置情報も送信する
  strncpy( &(buf[31]), "PV", 2 );
  ((MultiPlay *)Control::TheControl())->SendTime( &(buf[33]) );

  player->SendLocation( &(buf[38]) );

  send( theSocket, buf, 31+55, 0 );
}

void
getcurrenttime( struct timeb *tb ) {
#ifdef WIN32
  ftime( tb );
#else
  struct timeval tv;
  struct timezone tz;

  gettimeofday( &tv, &tz );
  tb->time = tv.tv_sec;
  tb->millitm = tv.tv_usec/1000;
#endif
}

void
ReadBI() {
  long len;
  char buf[256];

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
  Logging::GetLogging()->LogTime( LOG_COMBALL );
  Logging::GetLogging()->Log( LOG_COMBALL, "recv BI: " );
#endif

  theBall.Warp( buf );

#ifdef LOGGING
  Logging::GetLogging()->LogBall( LOG_COMBALL, &theBall );
#endif
}

#ifdef ENABLE_IPV6
void
findhostname( struct addrinfo *saddr ) {
#else
void
findhostname( struct sockaddr_in *saddr ) {
#endif
  if (1 == theRC->serverName[0]) { // Broadcast mode
#ifdef ENABLE_IPV6
    printf( "Broadcast is not supported\n" );
    exit(1);
#else
    struct sockaddr_in sba;
    memset(&sba, 0, sizeof(sba));
    unsigned int sb;
    if (0 > (sb = socket(PF_INET, SOCK_DGRAM, 0))) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      exit(1);
    }

    int one = 1;
    setsockopt( sb, IPPROTO_TCP, TCP_NODELAY, (char*)&one, sizeof(int) );

    one = 1;
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
      sba.sin_port = htons(theRC->csmash_port);

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
    saddr->sin_addr.s_addr = sba.sin_addr.s_addr;
#endif
  } else {
#ifdef ENABLE_IPV6
    struct addrinfo hent, *res;

    hent.ai_family = PF_UNSPEC;
    hent.ai_socktype = SOCK_STREAM;

    getaddrinfo( theRC->serverName, NULL, &hent, &res );

    memcpy( saddr->ai_addr, res->ai_addr, res->ai_addrlen );
#else
    struct hostent *hent;
    hent = gethostbyname( theRC->serverName );
    memcpy( &saddr->sin_addr, hent->h_addr, hent->h_length );
#endif
  }
}

void
ClearSocket() {
  if ( theSocket >= 0 ) {
    send( theSocket, "QT", 2, 0 );
    closesocket( theSocket );
    theSocket = -1;
  }
}
