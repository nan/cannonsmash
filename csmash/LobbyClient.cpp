/**
 * @file
 * @brief Implementation of LobbyClient class. 
 * @author KANNA Yoshihiro
 * $Id$
 * 
 * Copyright (C) 2001-2003  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ttinc.h"
#include "LobbyClient.h"
#include "LobbyClientView.h"
#include "MultiPlay.h"
#include "Network.h"
#include "RCFile.h"

#if !defined(WIN32)
#include <netinet/tcp.h>
#include <locale.h>
#endif

extern RCFile *theRC;

extern bool isComm;
extern long mode;

extern void StartGame();

extern int listenSocket[];
extern int one;

// dirty... used by "QP"
long score1 = 0;
long score2 = 0;

LobbyClient* LobbyClient::m_lobbyClient = NULL;

/**
 * Default constructor. 
 * Initialize member variables. 
 */
LobbyClient::LobbyClient() {
  m_playerNum = 0;
  m_canBeServer = false;
  m_selected = -1;
}

/**
 * Destructor. 
 */
LobbyClient::~LobbyClient() {
  if ( m_view )
    delete m_view;
  m_lobbyClient = NULL;
}

/**
 * Create method of LobbyClient. 
 * Since the constructor is private method, this method is the only method
 * to create LobbyClient. This method creates new LobbyClient and set it to
 * m_lobbyClient, the singleton holder. 
 * 
 * @return Created LobbyClient object. 
 */
LobbyClient*
LobbyClient::Create() {
  m_lobbyClient = new LobbyClient();
  return m_lobbyClient;
}

/**
 * Initializer method. 
 * This method opens socket for listening, get language code, and connects
 * to lobby server. 
 * 
 * @param nick nickname in lobby server
 * @param message join message
 * @param ping whether this client accepts ping or not. 
 * @return returns true if succeeds. 
 */
bool
LobbyClient::Init( char *nick, char *message, bool ping ) {
  char buf[1024];

#ifdef ENABLE_IPV6
  char port[10];

  sprintf( port, "%d", theRC->csmash_port );
#endif

  // open listening port
  if ( listenSocket[0] < 0 ) {
    if ( !::GetSocket() )
      return false;
  }

  // Get language code
  int i = 0;
#ifdef WIN32
  while ( table[i].langID != -1 ) {
    if ( PRIMARYLANGID(GetUserDefaultLangID()) == table[i].langID )
      break;
    i++;
  }
#else
  while ( table[i].langID != -1 ) {
    if ( strncmp( setlocale(LC_MESSAGES, NULL), table[i].code, 2 ) == 0 )
      break;
    i++;
  }
#endif
  m_lang = i;

  // connect to lobby server
#ifdef ENABLE_IPV6
  int error;
  struct addrinfo saddr, *res, *res0;

  memset(&saddr, 0, sizeof(saddr));

  if ( theRC->protocol == IPv6 )
    saddr.ai_family = PF_UNSPEC;
  else
    saddr.ai_family = PF_INET;

  saddr.ai_socktype = SOCK_STREAM;
  sprintf( port, "%d", LOBBYSERVER_PORT );
  error = getaddrinfo( LOBBYSERVER_NAME, port, &saddr, &res0 );
  if (error) {
    xerror("%s: %s(%d) getaddrinfo",
	   gai_strerror(error), __FILE__, __LINE__);
    return false;
  }

  m_socket = -1;

  for ( res = res0 ; res ; res = res->ai_next ) {
    if ( (m_socket = socket( res->ai_family, res->ai_socktype,
			     res->ai_protocol )) < 0 ) 
      continue;

    if ( connect( m_socket, res->ai_addr, res->ai_addrlen ) < 0 ) {
      close(m_socket);
      m_socket = -1;
      continue;
    }
    break;
  }

  freeaddrinfo(res0);

  if ( m_socket < 0 ) {
    xerror("%s(%d) connect", __FILE__, __LINE__);
    return false;
  }
#else
  struct hostent *hent;
  struct sockaddr_in saddr;

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
#endif

  // Connect to Lobby Server
  send( m_socket, "CN", 2, 0 );
  long len = 11 + 32 + 64;
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
  SendLong( m_socket, table[m_lang].langID );

  // send nick
  strncpy( buf, nick, 32 );
  send( m_socket, buf, 32, 0 );

  // send message
  strncpy( buf, message, 64 );
  send( m_socket, buf, 64, 0 );

  m_view = new LobbyClientView();
  m_view->Init( this );

  strncpy( m_nickname, nick, 32 );
  m_ping = ping;
  return true;
}

/**
 * Check messages from lobby server. 
 * This method is periodically called by GTK. 
 * 
 * @param data pointer to LobbyClient object. 
 * @param source source socket of the event which calls this function. 
 * @param condition the triggering condition. 
 */
void
LobbyClient::PollServerMessage( gpointer data,
				gint source,
				GdkInputCondition condition ) {
  LobbyClient *lobby = (LobbyClient *)data;

  int i = 0;
  bool listenIsSet = false;
  while ( listenSocket[i] >= 0 ) {
    if ( listenSocket[i] == source ) {
      int acSocket;
      acSocket = accept( listenSocket[i], NULL, NULL );

      closesocket( acSocket );
      lobby->m_canBeServer = true;
      listenIsSet = true;
      break;
    }
    i++;
  }

  if ( !listenIsSet && source == lobby->m_socket ) {
    char buf[1024];

    ReadHeader( lobby->m_socket, buf );

    if ( !strncmp( buf, "UI", 2 ) ) {
      lobby->ReadUI();
      lobby->m_view->UpdateTable();
    } else if ( !strncmp( buf, "PI", 2 ) ) {
      lobby->ReadPI();
    } else if ( !strncmp( buf, "OI", 2 ) ) {
      lobby->ReadOI();
    } else if ( !strncmp( buf, "AP", 2 ) ) {
      char *buffer;
      ReadEntireMessage( lobby->m_socket, &buffer );

      delete buffer;

      isComm = true;
      mode = MODE_MULTIPLAYSELECT;

      if ( lobby->m_canBeServer == true ) {	// I can be server
	theRC->serverName[0] = '\0';
      }

      lobby->m_view->SetSensitive( true );

      lobby->SendSP();

      ::StartGame();

      lobby->SendQP();
    } else if ( !strncmp( buf, "DP", 2 ) ) {
      char *buffer;
      ReadEntireMessage( lobby->m_socket, &buffer );

      delete buffer;

      lobby->m_view->SetSensitive( true );
    } else if ( !strncmp( buf, "OV", 2 ) ) {
      lobby->ReadOV();
    } else if ( !strncmp( buf, "MS", 2 ) ) {
      lobby->ReadMS();
    } else {
      xerror("%s(%d) read header", __FILE__, __LINE__);
      exit(1);
    }
  }
}

/**
 * Connect to server player. 
 * This method sends message to lobby server to get oppnent server information.
 * 
 * @param widget "connect" button widget. 
 * @param data pointer to LobbyClient object. 
 */
void
LobbyClient::Connect( GtkWidget *widget, gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;

  // Get selected player information
  send( lobby->m_socket, "OR", 2, 0 );
  long len = 4;
  SendLong( lobby->m_socket, len );
  SendLong( lobby->m_socket, lobby->m_selected );

  // Send Private IRC Message(Although it is not IRC at now)
  send( lobby->m_socket, "PI", 2, 0 );
  len = 4;
  SendLong( lobby->m_socket, len );
  SendLong( lobby->m_socket, lobby->m_selected );

  lobby->m_view->SetSensitive( false );
  //printf( "%d\n", lobby->m_selected );
}

/**
 * Get user information from the lobby server. 
 * This method reads message from the lobby server. Thw message shows the
 * number of players connecting to the lobby server, and their status. 
 */
void
LobbyClient::ReadUI() {
  // get length
  long len = 0;

  char *buffer;
  ReadEntireMessage( m_socket, &buffer );

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

    if ( buffer[len] )
      m_player[i].m_playing = true;
    else
      m_player[i].m_playing = false;
    len++;

    ReadLong( buffer+len, m_player[i].m_ID );
    len += 4;

    strncpy( m_player[i].m_nickname, &(buffer[len]), 32 );
    len += 32;

    strncpy( m_player[i].m_message, &(buffer[len]), 64 );
    len += 64;
  }	// add protocolLen check later

  delete buffer;
}

/**
 * Get opponent information from the lobby server. 
 * This method reads message from the lobby server. Thw message shows the
 * opponent who asked this player to play. And this method shows the dialog
 * to ask whether this player wants to play or not. 
 */
void
LobbyClient::ReadPI() {
  long len = 0;
  long uniqID;

  char *buffer;
  ReadEntireMessage( m_socket, &buffer );

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

  delete buffer;
}

/**
 * Get opponent machine information from the lobby server. 
 * This method reads message from the lobby server. Thw message shows the
 * opponent machine IP address and port number. 
 */
void
LobbyClient::ReadOI() {
  char *buffer;
  ReadEntireMessage( m_socket, &buffer );

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

  delete buffer;
}

/**
 * Get version information from the lobby server. 
 * This method reads message from the lobby server. Thw message shows the
 * version information. This method shows a dialog which ask the player to 
 * update to the latest version. 
 */
void
LobbyClient::ReadOV() {
  char *buffer;
  ReadEntireMessage( m_socket, &buffer );

  // get version number
  char version[16];
  sprintf( version, "%d.%d.%d",
	   (unsigned char)buffer[0], (unsigned char)buffer[1],
	   (unsigned char)buffer[2] );
  m_view->ShowUpdateDialog(version, &(buffer[3])); // second argument is URL. 

  delete buffer;
}

/**
 * Get chat message from the lobby server. 
 * This method reads message from the lobby server. The message shows the
 * chat message. This method add the message to chat window. 
 */
void
LobbyClient::ReadMS() {
  char *buffer;
  long len;
  len = ReadEntireMessage( m_socket, &buffer );

  long channelID;
  ReadLong( buffer, channelID );
  buffer[len] = 0;
  m_view->AddChatMessage( channelID, &(buffer[4]) );

  delete buffer;
}

/**
 * Send accept play message to the lobby server. 
 * This method sends message to the lobby server. The message shows that
 * this player accepts to play with the opponent. 
 */
void
LobbyClient::SendAP( long uniqID ) {
  send( m_socket, "AP", 2, 0 );
  long len = 4;
  SendLong( m_socket, len );
  SendLong( m_socket, uniqID );
}

/**
 * Send start play message to the lobby server. 
 * This method sends message to the lobby server. The message shows that
 * this player starts to play. 
 */
void
LobbyClient::SendSP() {
  send( m_socket, "SP", 2, 0 );
  long len = 0;
  SendLong( m_socket, len );
}

/**
 * Send quit play message to the lobby server. 
 * This method sends message to the lobby server. The message shows that
 * this player quits to play. 
 */
void
LobbyClient::SendQP() {
  send( m_socket, "QP", 2, 0 );
  long len = 8;
  SendLong( m_socket, len );

  SendLong( m_socket, score1 );	// Temp
  SendLong( m_socket, score2 );
}

/**
 * Send deny to play message to the lobby server. 
 * This method sends message to the lobby server. The message shows that
 * this player denies to play with the opponent. 
 */
void
LobbyClient::SendDP( long uniqID ) {
  send( m_socket, "DP", 2, 0 );
  long len = 4;
  SendLong( m_socket, len );
  SendLong( m_socket, uniqID );
}

/**
 * Send quit game message to the lobby server. 
 * This method sends message to the lobby server. The message shows that
 * this player quits the game. 
 */
void
LobbyClient::SendQT() {
  send( m_socket, "QT", 2, 0 );
  long len = 0;
  SendLong( m_socket, len );
  shutdown( m_socket, 2 );
}

/**
 * Send score to the lobby server. 
 * This method sends message to the lobby server. The message shows the
 * score of the game. 
 */
void
LobbyClient::SendSC( int score1, int score2 ) {
  send( m_socket, "SC", 2, 0 );
  long len = 8;
  SendLong( m_socket, len );

  SendLong( m_socket, score1 );	// Temp
  SendLong( m_socket, score2 );
}

/**
 * Send chat message to the lobby server. 
 * This method sends chat message to the lobby server. 
 */
void
LobbyClient::SendMS( char *message, long channel ) {
  send( m_socket, "MS", 2, 0 );
  long len = 4+strlen(message);
  SendLong( m_socket, len );

  SendLong( m_socket, channel );
  send( m_socket, message, strlen(message), 0 );
}


/**
 * Default constructor. 
 */
PlayerInfo::PlayerInfo() {
}

/**
 * Destructor. 
 * Do nothing. 
 */
PlayerInfo::~PlayerInfo() {
}
