/* $Id$ */

// Copyright (C) 2001-2003  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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
#include "RCFile.h"
#include <time.h>

extern RCFile *theRC;

extern bool isComm;
extern long mode;

extern void InitGame();
extern void StartGame();
extern void EndGame();
extern bool PollEvent();

extern void QuitGame();

bool isWaiting = false;		// waiting for opponent player on the internet

void
getcurrenttimestr( char *buf ) {
  struct tm *ltime;
  time_t t;

  time(&t);
  ltime = localtime( &t );

  strftime( buf, 32, "%H:%M:%S ", ltime );
}  

LobbyClientView::LobbyClientView() {
  m_timeout = 0;
  m_idle = 0;
  m_chatChannel = 0;
}

LobbyClientView::~LobbyClientView() {
  if ( m_timeout > 0 )
    gtk_timeout_remove( m_timeout );
  if ( m_idle > 0 )
    gtk_idle_remove( m_idle );
}

void
LobbyClientView::Init( LobbyClient *lobby ) {
  m_parent = lobby;

  gchar *channels[16] = {_("English"), _("Japanese"),
			 _("Germany"), _("Korean") };

  m_timeout = gtk_timeout_add( 1000, LobbyClient::PollServerMessage,
			       m_parent );

  // display
  m_window = gtk_dialog_new();
  gtk_container_border_width (GTK_CONTAINER (m_window), 10);

  gtk_window_set_title( GTK_WINDOW(m_window), _("Cannon Smash"));
  gtk_widget_show(m_window);
  gtk_window_set_modal( (GtkWindow *)m_window, true );
  gtk_widget_set_usize( m_window, 300, 400 );

  GtkWidget *scrolled_window;
  GtkWidget *button;

  scrolled_window = gtk_scrolled_window_new( NULL, NULL );
  gtk_container_border_width (GTK_CONTAINER (scrolled_window), 10);
  gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scrolled_window),
				  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_window)->vbox), scrolled_window,
		      TRUE, TRUE, 0 );
  gtk_widget_show (scrolled_window);

  gchar *titles[3] = { _("Nickname"), _("Message") };
  m_table = gtk_clist_new_with_titles( 2, titles );

  gtk_clist_set_shadow_type( GTK_CLIST(m_table), GTK_SHADOW_OUT );
  gtk_clist_set_column_width( GTK_CLIST(m_table), 0, 150 );

  gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW(scrolled_window),
					  m_table );
  gtk_signal_connect (GTK_OBJECT (m_table), "select-row",
		      GTK_SIGNAL_FUNC (LobbyClientView::SelectRow), this);
  gtk_widget_show(m_table);

  UpdateTable();


  GtkWidget *notebook = gtk_notebook_new();
  for ( int i = 0 ; i < 4 ; i++ ) {
    GtkWidget *label;
    GtkWidget *table = gtk_table_new( 1, 2, FALSE );

    m_chat[i] = gtk_text_new( NULL, NULL );
    GtkWidget *vscrollbar = gtk_vscrollbar_new( GTK_TEXT(m_chat[i])->vadj );

    gtk_table_attach(GTK_TABLE(table), m_chat[i], 0, 1, 0, 1, 
		     (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL), 
		     (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL),
		     0, 0 );
    gtk_table_attach(GTK_TABLE(table), vscrollbar, 1, 2, 0, 1, 
		     (GtkAttachOptions)(GTK_FILL), 
		     (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL),
		     0, 0 );

    gtk_widget_show(m_chat[i]);
    gtk_widget_show(vscrollbar);
    gtk_widget_show(table);

    label = gtk_label_new( channels[i] );
    gtk_notebook_append_page( GTK_NOTEBOOK(notebook), table, label);
  }

  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_window)->vbox), notebook,
		      TRUE, TRUE, 0 );
  gtk_signal_connect (GTK_OBJECT (notebook), "switch-page",
		      GTK_SIGNAL_FUNC (LobbyClientView::SwitchChatPage), this);
  gtk_widget_show(notebook);

  m_chatinput = gtk_entry_new();
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(m_window)->vbox), m_chatinput,
		      TRUE, TRUE, 10 );
  gtk_signal_connect (GTK_OBJECT (m_chatinput), "key-press-event",
		      GTK_SIGNAL_FUNC (LobbyClientView::KeyPress), this);
  gtk_widget_show(m_chatinput);


  m_connectButton = gtk_button_new_with_label (_("connect"));
  gtk_signal_connect (GTK_OBJECT (m_connectButton), "clicked",
		      GTK_SIGNAL_FUNC (LobbyClient::Connect), m_parent);
  gtk_widget_show (m_connectButton);
  gtk_widget_set_sensitive (m_connectButton, false);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      m_connectButton, TRUE, TRUE, 0);

  m_warmUpButton = gtk_button_new_with_label (_("warm up"));
  gtk_signal_connect (GTK_OBJECT (m_warmUpButton), "clicked",
		      GTK_SIGNAL_FUNC (LobbyClientView::WarmUp), this);
  gtk_widget_show (m_warmUpButton);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      m_warmUpButton, TRUE, TRUE, 0);

  button = gtk_button_new_with_label (_("close"));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (LobbyClientView::Quit), this);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      button, TRUE, TRUE, 0);

  gtk_widget_grab_default (button);
  gtk_widget_show (button);
  
  gtk_widget_show (m_window);                     
}

void
LobbyClientView::UpdateTable() {
  gchar *row[3];

  gtk_clist_clear( GTK_CLIST(m_table) );

  for ( int i = 0 ; i < m_parent->m_playerNum ; i++ ) {
    row[0] = m_parent->m_player[i].m_nickname;
    row[1] = m_parent->m_player[i].m_message;
    gtk_clist_append( GTK_CLIST(m_table), row );
  }

  gtk_clist_columns_autosize( GTK_CLIST(m_table) );
}

void
LobbyClientView::SelectRow( GtkCList *clist, gint row, gint column,
			GdkEventButton *event, gpointer data ) {
  LobbyClientView *lobby = (LobbyClientView *)data;

  lobby->m_parent->m_selected = row;
  gtk_widget_set_sensitive (lobby->m_connectButton, true);
}

void
LobbyClientView::WarmUp( GtkWidget *widget, gpointer data ) {
  LobbyClientView *lobby = (LobbyClientView *)data;

  isWaiting = true;
  ::InitGame();

  lobby->m_idle = gtk_idle_add( LobbyClientView::IdleFunc, data );
}

void
LobbyClientView::KeyPress( GtkWidget *widget,
			   GdkEventKey *event,
			   gpointer data) {
  LobbyClientView *lobby = (LobbyClientView *)data;

  if ( event->keyval == GDK_Return &&
       strlen(gtk_entry_get_text(GTK_ENTRY(lobby->m_chatinput))) > 0 ) {
    lobby->m_parent->SendMS( gtk_entry_get_text(GTK_ENTRY(lobby->m_chatinput)),
			     lobby->m_chatChannel );

    char buf[32];
    getcurrenttimestr(buf);


    gtk_text_insert (GTK_TEXT(lobby->m_chat[lobby->m_chatChannel]),
		     NULL, NULL, NULL, buf, -1);
    gtk_text_insert (GTK_TEXT(lobby->m_chat[lobby->m_chatChannel]),
		     NULL, NULL, NULL, ">", -1);
    gtk_text_insert (GTK_TEXT(lobby->m_chat[lobby->m_chatChannel]),
		     NULL, NULL, NULL, 
		     lobby->m_parent->m_nickname, -1);
    gtk_text_insert (GTK_TEXT(lobby->m_chat[lobby->m_chatChannel]),
		     NULL, NULL, NULL, "< ", -1);
    gtk_text_insert (GTK_TEXT(lobby->m_chat[lobby->m_chatChannel]),
		     NULL, NULL, NULL,
		     gtk_entry_get_text(GTK_ENTRY(lobby->m_chatinput)), -1);
    gtk_text_insert (GTK_TEXT(lobby->m_chat[lobby->m_chatChannel]),
		     NULL, NULL, NULL, "\n", -1);
    gtk_entry_set_text(GTK_ENTRY(lobby->m_chatinput), "");
  }
}

void
LobbyClientView::SwitchChatPage( GtkNotebook *notebook,
				 GtkNotebookPage *page,
				 gint page_num,
				 gpointer data ) {
  LobbyClientView *lobby = (LobbyClientView *)data;

  lobby->m_chatChannel = page_num;
}


gint
LobbyClientView::IdleFunc( gpointer data ) {
  if ( !PollEvent() ) {
    ::EndGame();
    return 0;
  }

  return 1;
}

void
LobbyClientView::Quit( GtkWidget *widget, gpointer data ) {
  LobbyClientView *lobby = (LobbyClientView *)data;

  lobby->m_parent->SendQT();

  gtk_widget_destroy( lobby->m_window );
  gtk_timeout_remove( lobby->m_timeout );

  delete lobby->m_parent;
}

void
LobbyClientView::SetSensitive( bool sensitive ) {
  gtk_widget_set_sensitive (m_connectButton, sensitive);
  gtk_widget_set_sensitive (m_warmUpButton, sensitive);
  gtk_widget_set_sensitive (m_table, sensitive);
}

void
LobbyClientView::ShowUpdateDialog( char *version, char *URL ) {
  GtkWidget *dialog = gtk_dialog_new();

  char buf[256];
  sprintf( buf, _("Latest Version %s is already released. \nPlease download the latest version from \n%s\n"), version, URL );

  GtkWidget *label = gtk_label_new( buf );
  GtkWidget *button = gtk_button_new_with_label( "OK" );

  gtk_window_set_modal( (GtkWindow *)dialog, true );

  gtk_label_set_line_wrap( GTK_LABEL(label), true );
  gtk_widget_show( label );

  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dialog)->vbox),
		      label, TRUE, TRUE, 0 );

  gtk_widget_show (dialog);

  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->action_area),
		      button, TRUE, TRUE, 0);
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC(LobbyClientView::Quit), this);
  gtk_signal_connect_object(GTK_OBJECT(button), "clicked",
			    GTK_SIGNAL_FUNC(gtk_widget_destroy), 
			    GTK_OBJECT(dialog));
  gtk_widget_show (button);
}

void
LobbyClientView::AddChatMessage( long channelID, char *message ) {
  char buf[32];
  getcurrenttimestr( buf );
  gtk_text_insert (GTK_TEXT(m_chat[channelID]), NULL, NULL, NULL, buf, -1);
  gtk_text_insert (GTK_TEXT(m_chat[channelID]), NULL, NULL, NULL, message, -1);
  gtk_text_insert (GTK_TEXT(m_chat[channelID]), NULL, NULL, NULL, "\n", -1);
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
  gtk_window_set_title( GTK_WINDOW(m_window), _("Cannon Smash"));
  gtk_container_border_width (GTK_CONTAINER (m_window), 10);
  gtk_window_set_modal( (GtkWindow *)m_window, true );

  PlayerInfo *p = m_parent->GetPlayerInfo();

  int i;
  for ( i = 0 ; i < m_parent->GetPlayerNum() ; i++ ) {
    if ( p[i].m_ID == m_uniqID ) {
      sprintf( buf, _("\"%s\" (message: %s)want to play with you. OK?\n"),
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

  button = gtk_button_new_with_label (_("OK!"));
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (PIDialog::PIOK), this);

  GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX (GTK_DIALOG (m_window)->action_area),
		      button, TRUE, TRUE, 0);

  gtk_widget_grab_default (button);
  gtk_widget_show (button);

  button = gtk_button_new_with_label (_("No!"));
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

  piDialog->m_parent->SendAP( piDialog->m_uniqID );
  piDialog->m_parent->SendSP();

  // If oppenent cannot be a server, I become server. 
  for ( int i = 0 ; i < piDialog->m_parent->m_playerNum ; i++ ) {
    if ( piDialog->m_parent->m_player[i].m_ID == piDialog->m_uniqID ) {
      if ( piDialog->m_parent->m_player[i].m_canBeServer == false ) {	// Opponent cannot be server
	theRC->serverName[0] = '\0';
      }
      break;
    }
  }

  ::StartGame();

  piDialog->m_parent->SendQP();
}

void
PIDialog::PINo( GtkWidget *widget, gpointer data ) {
  PIDialog *piDialog = (PIDialog *)data;
  gtk_widget_destroy( GTK_WIDGET(piDialog->m_window) );

  piDialog->m_parent->SendDP( piDialog->m_uniqID );
}

