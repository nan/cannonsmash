/* $Id$ */

// Copyright (C) 2001, 2002  $B?@Fn(B $B5H9((B(Kanna Yoshihiro)
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

#ifndef _LobbyClient_
#define _LobbyClient_

#include <gtk/gtk.h>

#define LOBBYSERVER_NAME	"nan.p.utmc.or.jp"
#define LOBBYSERVER_PORT	(5733)

class PlayerInfo;
class LobbyClientView;

class LobbyClient {
public:
  LobbyClient();
  ~LobbyClient();

  bool Init( char *nickname, char *message );

  int GetSocket() { return m_socket; };
  PlayerInfo *GetPlayerInfo() { return m_player; };
  long GetPlayerNum() { return m_playerNum; };

  static gint PollServerMessage( gpointer data );
  static void Connect( GtkWidget *widget, gpointer data );

  void SendAP( long uniqID );
  void SendSP();
  void SendQP();
  void SendDP( long uniqID);
  void SendQT();

  long m_playerNum;
  PlayerInfo *m_player;

  long m_selected;		// Selected row of the table

protected:
  void ReadHeader( char *buf );

  void ReadUI();
  void UpdateTable();

  void ReadPI();
  void ReadOI();

  LobbyClientView *m_view;

  int m_socket;
  bool m_canBeServer;
};

class PlayerInfo {
public:
  PlayerInfo();
  ~PlayerInfo();

  bool m_canBeServer;
  long m_ID;
  char m_nickname[32];
  char m_message[64];
};

#endif // _LobbyClient_
