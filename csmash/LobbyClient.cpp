/* $Id$ */

// Copyright (C) 2001, 2002  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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
#include "LobbyClient.h"
#include "LobbyClientView.h"
#include "MultiPlay.h"
#include "Network.h"
#include "RCFile.h"

#if !defined(WIN32)
#include <netinet/tcp.h>
#endif

extern RCFile *theRC;

extern bool isComm;
extern long mode;

extern void StartGame();

extern unsigned int listenSocket;
extern int one;

// dirty... used by "QP"
long score1 = 0;
long score2 = 0;

LobbyClient* LobbyClient::m_lobbyClient = NULL;

LobbyClient::LobbyClient() {
  m_playerNum = 0;
  m_canBeServer = false;
}

LobbyClient::~LobbyClient() {
  if ( m_view )
    delete m_view;
  m_lobbyClient = NULL;
}

LobbyClient*
LobbyClient::Create() {
  m_lobbyClient = new LobbyClient();
  return m_lobbyClient;
}

bool
LobbyClient::Init( char *nick, char *message ) {
  char buf[1024];

  // open listening port
  struct sockaddr_in saddr;

  if ( listenSocket == 0 ) {
    if ( (listenSocket = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      return false;
    }

    setsockopt( listenSocket, IPPROTO_TCP, TCP_NODELAY,
		(char*)&one, sizeof(int) );

    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(theRC->csmash_port);
    if ( bind( listenSocket, (struct sockaddr *)&saddr, sizeof(saddr) ) < 0 ) {
      xerror("%s(%d) bind", __FILE__, __LINE__);
      return false;
    }

    if ( listen( listenSocket, 1 ) < 0 ) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      return false;
    }
  }

  // connect to lobby server
  struct hostent *hent;

  memset(&saddr, 0, sizeof(saddr));

  hent = gethostbyname( LOBBYSERVER_NAME );
  memcpy( &saddr.sin_addr, hent->h_addr, hent->h_length );

  saddr.sin_family = AF_INET;
  saddr.sin_port = htons(LOBBYSERVER_PORT);

  // connect
  if ( (m_socket = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {
    xerror("%s(%d) socket", __FILE__, __LINE__);
    return false;
  }
  if ( connect( m_socket, (struct sockaddr *)&saddr, sizeof(saddr) ) ) {
    xerror("%s(%d) connect", __FILE__, __LINE__);
    return false;
  }

  // Connect to Lobby Server
  send( m_socket, "CN", 2, 0 );
  long len = 7 + 32 + 64;
  SendLong( m_socket, len );

  // Send version number(Must be changed)
  char ver;
  ver = MAJOR_VERSION;
  send( m_socket, &ver, 1, 0 );
  ver = MIDDLE_VERSION;
  send( m_socket, &ver, 1, 0 );
  ver = MINOR_VERSION;
  send( m_socket, &ver, 1, 0 );

  // Send port number(Must be changed, too)
  SendLong( m_socket, theRC->csmash_port );

  // send nick
  strncpy( buf, nick, 32 );
  send( m_socket, buf, 32, 0 );

  // send message
  strncpy( buf, message, 64 );
  send( m_socket, buf, 64, 0 );

  m_view = new LobbyClientView();
  m_view->Init( this );

  return true;
}

gint
LobbyClient::PollServerMessage( gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;
  fd_set rdfds;
  struct timeval to;

  while (1) {
    int max;
    FD_ZERO( &rdfds );
    FD_SET( (SOCKET)lobby->m_socket, &rdfds );
    if ( listenSocket )
      FD_SET( listenSocket, &rdfds );

    if ( (SOCKET)lobby->m_socket > listenSocket )
      max = lobby->m_socket;
    else
      max = listenSocket;

    to.tv_sec = to.tv_usec = 0;

    if ( select( max+1, &rdfds, NULL, NULL, &to ) <= 0 ) {
      return 1;
    } else if ( FD_ISSET( listenSocket, &rdfds ) ) {
      int acSocket;
      acSocket = accept( listenSocket, NULL, NULL );

      closesocket( acSocket );
      lobby->m_canBeServer = true;
    } else {
      char buf[1024];

      lobby->ReadHeader( buf );

      if ( !strncmp( buf, "UI", 2 ) ) {
	lobby->ReadUI();
	lobby->m_view->UpdateTable();
      } else if ( !strncmp( buf, "PI", 2 ) ) {
	lobby->ReadPI();
      } else if ( !strncmp( buf, "OI", 2 ) ) {
	lobby->ReadOI();
      } else if ( !strncmp( buf, "AP", 2 ) ) {
	isComm = true;
	mode = MODE_SELECT;

	if ( lobby->m_canBeServer == true ) {	// I can be server
	  theRC->serverName[0] = '\0';
	}

	lobby->m_view->SetSensitive( true );

	lobby->SendSP();

	::StartGame();

	lobby->SendQP();
      } else if ( !strncmp( buf, "DP", 2 ) ) {
	lobby->m_view->SetSensitive( true );
      } else if ( !strncmp( buf, "OV", 2 ) ) {
	lobby->ReadOV();
      } else {
	xerror("%s(%d) read header", __FILE__, __LINE__);
	exit(1);
      }
    }
  }
}

void
LobbyClient::Connect( GtkWidget *widget, gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;

  // Get selected player information
  send( lobby->m_socket, "OR", 2, 0 );
  long len = 4;
  SendLong( lobby->m_socket, len );
  SendLong( lobby->m_socket, lobby->m_player[lobby->m_selected].m_ID );

  // Send Private IRC Message(Although it is not IRC at now)
  send( lobby->m_socket, "PI", 2, 0 );
  len = 4;
  SendLong( lobby->m_socket, len );
  SendLong( lobby->m_socket, lobby->m_player[lobby->m_selected].m_ID );

  lobby->m_view->SetSensitive( false );
  //printf( "%d\n", lobby->m_selected );
}

void
LobbyClient::ReadHeader( char *buf ) {
  long len = 0;
  while (1) {
    if ( (len+=recv( m_socket, buf+len, 2-len, 0 )) == 2 )
      break;
  }
}

void
LobbyClient::ReadUI() {
  // get length
  long protocolLength;
  long len = 0;
  char buf[1024];

  while (1) {
    if ( (len+=recv( m_socket, buf+len, 4-len, 0 )) == 4 )
      break;
  }
  ReadLong( buf, protocolLength );

  // First, read all
  char *buffer = new char[protocolLength];
  len = 0;
  while (1) {
    if ( (len+=recv( m_socket, buffer+len, protocolLength-len, 0 )) == protocolLength )
      break;
  }

  // Get player number
  ReadLong( buffer, m_playerNum );
  len = 4;

  m_player = new PlayerInfo[m_playerNum];

  // Analyse data
  int i;
  for ( i = 0 ; i < m_playerNum ; i++ ) {
    if ( buffer[len] )
      m_player[i].m_canBeServer = true;
    else
      m_player[i].m_canBeServer = false;
    len++;

    ReadLong( buffer+len, m_player[i].m_ID );
    len += 4;

    strncpy( m_player[i].m_nickname, &(buffer[len]), 32 );
    len += 32;

    strncpy( m_player[i].m_message, &(buffer[len]), 64 );
    len += 64;
  }	// add protocolLen check later

}

void
LobbyClient::ReadPI() {
  // get length
  long protocolLength;
  long len = 0;
  char buf[1024];
  long uniqID;

  while (1) {
    if ( (len+=recv( m_socket, buf+len, 4-len, 0 )) == 4 )
      break;
  }
  ReadLong( buf, protocolLength );

  // First, read all
  char *buffer = new char[protocolLength];
  len = 0;
  while (1) {
    if ( (len+=recv( m_socket, buffer+len, protocolLength-len, 0 ))
	 == protocolLength )
      break;
  }

  // get uniq ID
  ReadLong( buffer, uniqID );

  // Get opponent info(Not so good. Must be modified later. )
  send( m_socket, "OR", 2, 0 );
  len = 4;
  SendLong( m_socket, len );
  SendLong( m_socket, uniqID );

  // Is it OK to do this here? 
  PIDialog *piDialog = new PIDialog( this );
  piDialog->PopupDialog( uniqID );
}

void
LobbyClient::ReadOI() {
  // get length
  long protocolLength;
  long len = 0;
  char buf[1024];

  while (1) {
    if ( (len+=recv( m_socket, buf+len, 4-len, 0 )) == 4 )
      break;
  }
  ReadLong( buf, protocolLength );

  // First, read all
  char *buffer = new char[protocolLength];
  len = 0;
  while (1) {
    if ( (len+=recv( m_socket, buffer+len, protocolLength-len, 0 ))
	 == protocolLength )
      break;
  }

  // get IP address
  sprintf( theRC->serverName, "%d.%d.%d.%d",
	   (unsigned char)buffer[0], (unsigned char)buffer[1],
	   (unsigned char)buffer[2], (unsigned char)buffer[3] );
  // get port number
  long tmpPort;
  ReadLong( &(buffer[4]), tmpPort );
  theRC->csmash_port = tmpPort;

  // At now, Ignore server or client. 

  //printf( "%s %d\n", theRC->serverName, theRC->csmash_port );
}

void
LobbyClient::ReadOV() {
  // get length
  long protocolLength;
  long len = 0;
  char buf[1024];

  while (1) {
    if ( (len+=recv( m_socket, buf+len, 4-len, 0 )) == 4 )
      break;
  }
  ReadLong( buf, protocolLength );

  // First, read all
  char *buffer = new char[protocolLength];
  len = 0;
  while (1) {
    if ( (len+=recv( m_socket, buffer+len, protocolLength-len, 0 ))
	 == protocolLength )
      break;
  }

  // get version number
  char version[16];
  sprintf( version, "%d.%d.%d",
	   (unsigned char)buffer[0], (unsigned char)buffer[1],
	   (unsigned char)buffer[2] );
  m_view->ShowUpdateDialog(version, &(buffer[3])); // second argument is URL. 
}

void
LobbyClient::SendAP( long uniqID ) {
  send( m_socket, "AP", 2, 0 );
  long len = 4;
  SendLong( m_socket, len );
  SendLong( m_socket, uniqID );
}

void
LobbyClient::SendSP() {
  send( m_socket, "SP", 2, 0 );
  long len = 0;
  SendLong( m_socket, len );
}

void
LobbyClient::SendQP() {
  send( m_socket, "QP", 2, 0 );
  long len = 8;
  SendLong( m_socket, len );

  SendLong( m_socket, score1 );	// Temp
  SendLong( m_socket, score2 );
}

void
LobbyClient::SendDP( long uniqID ) {
  send( m_socket, "DP", 2, 0 );
  long len = 4;
  SendLong( m_socket, len );
  SendLong( m_socket, uniqID );
}

void
LobbyClient::SendQT() {
  send( m_socket, "QT", 2, 0 );
  shutdown( m_socket, 2 );
}

void
LobbyClient::SendSC( int score1, int score2 ) {
  send( m_socket, "SC", 2, 0 );
  long len = 8;
  SendLong( m_socket, len );

  SendLong( m_socket, score1 );	// Temp
  SendLong( m_socket, score2 );
}


PlayerInfo::PlayerInfo() {
}

PlayerInfo::~PlayerInfo() {
}
