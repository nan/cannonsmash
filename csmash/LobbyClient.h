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

#include <gtk/gtk.h>

//#define LOBBYSERVER_NAME	"nan.p.utmc.or.jp"
#define LOBBYSERVER_NAME	"192.168.1.184"
#define LOBBYSERVER_PORT	(5733)

class PlayerInfo;

class LobbyClient {
public:
  LobbyClient();
  ~LobbyClient();

  void Init( char *nickname, char *message );

  int GetSocket() { return m_socket; };
  PlayerInfo *GetPlayerInfo() { return m_player; };
  long GetPlayerNum() { return m_playerNum; };

protected:
  static gint PollServerMessage( gpointer data );
  static void SelectRow( GtkCList *clist, gint row, gint column,
			 GdkEventButton *event, gpointer data );
  static void Connect( GtkWidget *widget, gpointer data );
  static void Quit( GtkWidget *widget, gpointer data );
  guint m_timeout;

  void ReadHeader( char *buf );

  void ReadUI();
  void UpdateTable();

  void ReadPI();
  void ReadOI();

  GtkWidget *m_window;
  GtkWidget *m_table;
  GtkWidget *m_connectButton;

  int m_socket;

  long m_playerNum;
  PlayerInfo *m_player;

  long m_selected;		// テーブルの何番目が選択されているか
};

class PIDialog {
public:
  PIDialog();
  PIDialog( LobbyClient *parent );
  ~PIDialog();

  void PopupDialog( long uniqID );

  GtkWidget *m_window;
  LobbyClient *m_parent;

  long m_uniqID;

  static void PIOK( GtkWidget *widget, gpointer data );
  static void PINo( GtkWidget *widget, gpointer data );
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
