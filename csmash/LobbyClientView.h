/**
 * @file
 * @brief Definition of LobbyClientView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2001-2003  神南 吉宏(Kanna Yoshihiro)
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
#include <gdk/gdkkeysyms.h>

class LobbyClient;

/**
 * LobbyClientView class is a view class for lobby client dialog. 
 */
class LobbyClientView {
public:
  LobbyClientView();
  ~LobbyClientView();

  void Init( LobbyClient *lobby );

  void SetSensitive( bool sensitive );
  void UpdateTable();
  void ShowUpdateDialog( char *version, char *URL );
  void AddChatMessage( long channelID, char *message );

protected:
  static gint IdleFunc( gpointer data );
  static gboolean checkSelection( GtkTreeSelection *selection,
				  GtkTreeModel *model,
				  GtkTreePath *path,
				  gboolean path_currently_selected,
				  gpointer data);

  static void WarmUp( GtkWidget *widget, gpointer data );
  static gboolean KeyPress( GtkWidget *widget, GdkEventKey *event, gpointer data );
  static void SwitchChatPage( GtkNotebook *notebook, GtkNotebookPage *page,
			      gint page_num, gpointer data );
  static void Quit( GtkWidget *widget, gpointer data );

#ifdef WIN32
  static LRESULT CALLBACK EditWindowProc( HWND hwnd, UINT msg,
					  WPARAM wparam, LPARAM lparam);
#endif

  guint m_timeout;		///< wait timer handler for lobby server message
  guint m_idle;			///< idle callback handler

  gint  m_chatChannel;		///< current chat channel ID
  int   m_langID[2];		///< language ID of each chat channel

  LobbyClient *m_parent;	///< reference to LobbyClient object

  GtkWidget *m_window;		///< lobby client dialog
  GtkWidget *m_table;		///< table view of members connecting to the lobby server
  GtkWidget *m_chat[2];		///< text view of each chat message
  GtkWidget *m_chatinput;	///< edit control to input chat message
  GtkWidget *m_connectButton;	///< "connect" button
  GtkWidget *m_warmUpButton;	///< "warm up" button

  long m_selected;		///< Selected row of the table
};

/**
 * PIDialog is opened when a opponent wants to play with this player. 
 */
class PIDialog {
public:
  PIDialog();
  PIDialog( LobbyClient *parent );
  ~PIDialog();

  void PopupDialog( long uniqID );

  GtkWidget *m_window;		///< PIDialog window. 
  LobbyClient *m_parent;	///< LobbyClient object. 

  long m_uniqID;		///< player ID of the player who wants to play with this player

  static void PIOK( GtkWidget *widget, gpointer data );
  static void PINo( GtkWidget *widget, gpointer data );
};

#endif // _LobbyClientView_
