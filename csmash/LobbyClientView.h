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

#ifndef _LobbyClientView_
#define _LobbyClientView_

#include <gtk/gtk.h>

class LobbyClient;

class LobbyClientView {
public:
  LobbyClientView();
  ~LobbyClientView();

  void Init( LobbyClient *lobby );

  void SetSensitive( bool sensitive );
  void UpdateTable();
  void ShowUpdateDialog( char *version );
protected:
  static gint IdleFunc( gpointer data );
  static void SelectRow( GtkCList *clist, gint row, gint column,
			 GdkEventButton *event, gpointer data );
  static void Connect( GtkWidget *widget, gpointer data );
  static void WarmUp( GtkWidget *widget, gpointer data );
  static void Quit( GtkWidget *widget, gpointer data );

  guint m_timeout;
  guint m_idle;
  LobbyClient *m_parent;

  void ReadHeader( char *buf );

  void ReadUI();
  void ReadPI();
  void ReadOI();

  GtkWidget *m_window;
  GtkWidget *m_table;
  GtkWidget *m_connectButton;
  GtkWidget *m_warmUpButton;

  long m_selected;		// Selected row of the table
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

#endif // _LobbyClientView_
