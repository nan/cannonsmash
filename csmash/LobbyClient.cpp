/* $Id$ */

// Copyright (C) 2001  神南 吉宏(Kanna Yoshihiro)
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
#include "MultiPlay.h"
#include "RCFile.h"

#if !defined(WIN32)
#include <netinet/tcp.h>
#endif

#if defined(WIN32) || defined(__FreeBSD__)

typedef int socklen_t;		/* mimic Penguin's typedef */

#else	/* ! WIN32 */

#define closesocket(FD) close(FD)

#endif

extern RCFile *theRC;

extern bool isComm;
extern long mode;

extern bool isWaiting;

extern void StartGame();
extern void EventLoop();
extern bool PollEvent();

extern void QuitGame();

extern unsigned int listenSocket;
extern int one;

bool isWaiting = false;		// waiting for opponent player on the internet

// dirty... used by "QP"
long score1 = 0;
long score2 = 0;

LobbyClient::LobbyClient() {
  m_playerNum = 0;
  m_timeout = 0;
  m_idle = 0;
}

LobbyClient::~LobbyClient() {
  if ( m_timeout > 0 )
    gtk_timeout_remove( m_timeout );
  if ( m_idle > 0 )
    gtk_idle_remove( m_idle );
}

void
LobbyClient::Init( char *nick, char *message ) {
  char buf[1024];

  // open listening port
  struct sockaddr_in saddr;

  if ( listenSocket == 0 ) {
    if ( (listenSocket = socket( PF_INET, SOCK_STREAM, 0 )) < 0 ) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      gtk_main_quit();
      exit(1);
    }

    setsockopt( listenSocket, IPPROTO_TCP, TCP_NODELAY,
		(char*)&one, sizeof(int) );

    saddr.sin_addr.s_addr = htonl(INADDR_ANY);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(theRC->csmash_port);
    if ( bind( listenSocket, (struct sockaddr *)&saddr, sizeof(saddr) ) < 0 ) {
      xerror("%s(%d) bind", __FILE__, __LINE__);
      gtk_main_quit();
      exit(1);
    }

    if ( listen( listenSocket, 1 ) < 0 ) {
      xerror("%s(%d) socket", __FILE__, __LINE__);
      gtk_main_quit();
      exit(1);
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
    gtk_main_quit();
    exit(1);
  }
  if ( connect( m_socket, (struct sockaddr *)&saddr, sizeof(saddr) ) ) {
    xerror("%s(%d) connect", __FILE__, __LINE__);
    gtk_main_quit();
    exit(1);
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

  m_timeout = gtk_timeout_add( 1000, LobbyClient::PollServerMessage, this );

  // display
  m_window = gtk_dialog_new();
  gtk_container_border_width (GTK_CONTAINER (m_window), 10);

  gtk_window_set_title( GTK_WINDOW(m_window), "Cannon Smash");
  gtk_widget_show(m_window);
  gtk_window_set_modal( (GtkWindow *)m_window, true );
  gtk_widget_set_usize( m_window, 300, 200 );

  GtkWidget *scrolled_window;
  GtkWidget *button;

  scrolled_window = gtk_scrolled_window_new( NULL, NULL );
  gtk_container_border_width (GTK_CONTAINER (scrolled_window), 10);
  gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_window)->vbox), scrolled_window, 
		      TRUE, TRUE, 0 );
  gtk_widget_show (scrolled_window);

  gchar *titles[3] = { "Nickname", "Message" };
  m_table = gtk_clist_new_with_titles( 2, titles );

  gtk_clist_set_shadow_type( GTK_CLIST(m_table), GTK_SHADOW_OUT );
  gtk_clist_set_column_width( GTK_CLIST(m_table), 0, 150 );

  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW(scrolled_window),
					  m_table );
  gtk_signal_connect (GTK_OBJECT (m_table), "select-row",
		      GTK_SIGNAL_FUNC (LobbyClient::SelectRow), this);
  gtk_widget_show(m_table);

  UpdateTable();

  m_connectButton = gtk_button_new_with_label ("connect");
  gtk_signal_connect (GTK_OBJECT (m_connectButton), "clicked",
		      GTK_SIGNAL_FUNC (LobbyClient::Connect), this);
  gtk_widget_show (m_connectButton);
  gtk_widget_set_sensitive (m_connectButton, false);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      m_connectButton, TRUE, TRUE, 0);

  m_warmUpButton = gtk_button_new_with_label ("warm up");
  gtk_signal_connect (GTK_OBJECT (m_warmUpButton), "clicked",
		      GTK_SIGNAL_FUNC (LobbyClient::WarmUp), this);
  gtk_widget_show (m_warmUpButton);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      m_warmUpButton, TRUE, TRUE, 0);

  button = gtk_button_new_with_label ("close");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (LobbyClient::Quit), this);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      button, TRUE, TRUE, 0);

  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  
  gtk_widget_show (m_window);                     
}

gint
LobbyClient::PollServerMessage( gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;
  fd_set rdfds;
  struct timeval to;

  while (1) {
    int max;
    FD_ZERO( &rdfds );
    FD_SET( lobby->m_socket, &rdfds );
    if ( listenSocket )
      FD_SET( listenSocket, &rdfds );

    if ( lobby->m_socket > listenSocket )
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
    } else {
      char buf[1024];

      lobby->ReadHeader( buf );

      if ( !strncmp( buf, "UI", 2 ) ) {
	lobby->ReadUI();
	lobby->UpdateTable();
      } else if ( !strncmp( buf, "PI", 2 ) ) {
	lobby->ReadPI();
	//printf( "PI!\n" );
      } else if ( !strncmp( buf, "OI", 2 ) ) {
	lobby->ReadOI();
      } else if ( !strncmp( buf, "AP", 2 ) ) {
	isComm = true;
	mode = MODE_SELECT;
	theRC->serverName[0] = '\0';
	gtk_widget_set_sensitive (lobby->m_connectButton, true);
	gtk_widget_set_sensitive (lobby->m_warmUpButton, true);
	gtk_widget_set_sensitive (lobby->m_table, true);

	send( lobby->m_socket, "SP", 2, 0 );
	long len = 0;
	SendLong( lobby->m_socket, len );

	StartGame();
	EventLoop();

	send( lobby->m_socket, "QP", 2, 0 );
	len = 8;
	SendLong( lobby->m_socket, len );

	SendLong( lobby->m_socket, score1 );	// Temp
	SendLong( lobby->m_socket, score2 );
      } else if ( !strncmp( buf, "DP", 2 ) ) {
	gtk_widget_set_sensitive (lobby->m_connectButton, true);
	gtk_widget_set_sensitive (lobby->m_warmUpButton, true);
	gtk_widget_set_sensitive (lobby->m_table, true);
      } else {
	xerror("%s(%d) read header", __FILE__, __LINE__);
	gtk_main_quit();
	exit(1);
      }
    }
  }
}

void
LobbyClient::UpdateTable() {
  gchar *row[3];

  gtk_clist_clear( GTK_CLIST(m_table) );

  for ( int i = 0 ; i < m_playerNum ; i++ ) {
    row[0] = m_player[i].m_nickname;
    row[1] = m_player[i].m_message;
    gtk_clist_append( GTK_CLIST(m_table), row );
  }

  gtk_clist_columns_autosize( GTK_CLIST(m_table) );
}

void
LobbyClient::SelectRow( GtkCList *clist, gint row, gint column,
			GdkEventButton *event, gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;

  lobby->m_selected = row;
  gtk_widget_set_sensitive (lobby->m_connectButton, true);
}

void
LobbyClient::Connect( GtkWidget *widget, gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;

  // Send Private IRC Message(Although it is not IRC at now)
  send( lobby->m_socket, "PI", 2, 0 );
  long len = 4;
  SendLong( lobby->m_socket, len );
  SendLong( lobby->m_socket, lobby->m_player[lobby->m_selected].m_ID );

  gtk_widget_set_sensitive (lobby->m_connectButton, false);
  gtk_widget_set_sensitive (lobby->m_warmUpButton, false);
  gtk_widget_set_sensitive (lobby->m_table, false);
  //printf( "%d\n", lobby->m_selected );
}

void
LobbyClient::WarmUp( GtkWidget *widget, gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;

  isWaiting = true;
  StartGame();

  lobby->m_idle = gtk_idle_add( LobbyClient::IdleFunc, data );
}

gint
LobbyClient::IdleFunc( gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;

  if ( !PollEvent() ) {
    return 0;
    //gtk_idle_remove( lobby->m_idle );
  }

  return 1;
}

void
LobbyClient::Quit( GtkWidget *widget, gpointer data ) {
  LobbyClient *lobby = (LobbyClient *)data;

  // Send Quit Message
  send( lobby->m_socket, "QT", 2, 0 );
  shutdown( lobby->m_socket, 2 );
  
  gtk_widget_destroy( lobby->m_window );
  gtk_timeout_remove( lobby->m_timeout );

  delete lobby;
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

  //printf( "%s %d\n", serverName, theRC->csmash_port );
}


PIDialog::PIDialog() {
}

PIDialog::PIDialog( LobbyClient *parent ) {
  m_parent = parent;
}

PIDialog::~PIDialog() {
}

void
PIDialog::PopupDialog( long uniqID ) {
  GtkWidget *label, *button;
  char buf[256];

  if ( isWaiting ) {
    QuitGame();
  }

  m_uniqID = uniqID;

  m_window = gtk_dialog_new();
  gtk_window_set_title( GTK_WINDOW(m_window), "Cannon Smash");
  gtk_container_border_width (GTK_CONTAINER (m_window), 10);
  gtk_window_set_modal( (GtkWindow *)m_window, true );

  PlayerInfo *p = m_parent->GetPlayerInfo();

  int i;
  for ( i = 0 ; i < m_parent->GetPlayerNum() ; i++ ) {
    if ( p[i].m_ID == m_uniqID ) {
      sprintf( buf, "\"%s\" (message: %s)want to play with you. OK?\n",
	       p[i].m_nickname, p[i].m_message );
      break;
    }
  }

  label = gtk_label_new( buf );
  gtk_label_set_line_wrap( GTK_LABEL(label), true );
  gtk_widget_show( label );

  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_window)->vbox), label, 
		      TRUE, TRUE, 0 );

  gtk_widget_show (m_window);

  button = gtk_button_new_with_label ("OK!");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (PIDialog::PIOK), this);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      button, TRUE, TRUE, 0);

  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label ("No!");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (PIDialog::PINo), this);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      button, TRUE, TRUE, 0);
  gtk_widget_show (button);
}

void
PIDialog::PIOK( GtkWidget *widget, gpointer data ) {
  PIDialog *piDialog = (PIDialog *)data;
  gtk_widget_destroy( GTK_WIDGET(piDialog->m_window) );

  isComm = true;
  mode = MODE_SELECT;

  send( piDialog->m_parent->GetSocket(), "AP", 2, 0 );
  long len = 4;
  SendLong( piDialog->m_parent->GetSocket(), len );
  SendLong( piDialog->m_parent->GetSocket(), piDialog->m_uniqID );

  send( piDialog->m_parent->GetSocket(), "SP", 2, 0 );
  len = 0;
  SendLong( piDialog->m_parent->GetSocket(), len );

  StartGame();
  EventLoop();

  send( piDialog->m_parent->GetSocket(), "QP", 2, 0 );
  len = 8;
  SendLong( piDialog->m_parent->GetSocket(), len );

  SendLong( piDialog->m_parent->GetSocket(), len );	// Temp
  SendLong( piDialog->m_parent->GetSocket(), len );
}

void
PIDialog::PINo( GtkWidget *widget, gpointer data ) {
  PIDialog *piDialog = (PIDialog *)data;
  gtk_widget_destroy( GTK_WIDGET(piDialog->m_window) );

  send( piDialog->m_parent->GetSocket(), "DP", 2, 0 );
  long len = 4;
  SendLong( piDialog->m_parent->GetSocket(), len );
  SendLong( piDialog->m_parent->GetSocket(), piDialog->m_uniqID );
}


PlayerInfo::PlayerInfo() {
}

PlayerInfo::~PlayerInfo() {
}
