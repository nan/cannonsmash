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
#include "Launcher.h"

extern bool fullScreen;
extern bool isComm;
extern char serverName[256];
extern long mode;
extern long gmode;

extern void StartGame();
extern void EventLoop();

extern bool WriteRCFile();

char nickname[32] = {'\0'};
char message[64] = {'\0'};

LauncherHeader::LauncherHeader() {
}

LauncherHeader::~LauncherHeader() {
}

void
LauncherHeader::Init( GtkBox *box ) {
  GtkWidget *frame;

  frame = FullScreenFrame();
  gtk_box_pack_start( box, frame, FALSE, FALSE, 10 );
  gtk_widget_show (frame);
  frame = GraphicsFrame();
  gtk_box_pack_start( box, frame, FALSE, FALSE, 10 );
  gtk_widget_show (frame);
}

GtkWidget *
LauncherHeader::FullScreenFrame() {
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *button;
  GSList *list;

  frame = gtk_frame_new( "FullScreen" );

  box = gtk_hbox_new( FALSE, 10 );
  gtk_container_border_width (GTK_CONTAINER (box), 5);

  button = gtk_radio_button_new_with_label ((GSList *)NULL, "On");
  list = gtk_radio_button_group( GTK_RADIO_BUTTON(button) );
  gtk_box_pack_start( GTK_BOX(box), button, FALSE, FALSE, 10 );
  if (fullScreen)
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(button), TRUE );
  gtk_widget_show (button);

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (LauncherHeader::ToggleFullScreen),
		      &fullScreen);

  button = gtk_radio_button_new_with_label (list, "Off");
  list = gtk_radio_button_group( GTK_RADIO_BUTTON(button) );
  gtk_box_pack_start( GTK_BOX(box), button, FALSE, FALSE, 10 );
  if (!fullScreen)
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(button), TRUE );
  gtk_widget_show (button);

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (LauncherHeader::ToggleFullScreen),
		      &fullScreen);

  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (frame), box);

  return frame;
}

GtkWidget *
LauncherHeader::GraphicsFrame() {
  GtkWidget *frame;
  GtkWidget *box;
  GtkWidget *twoDButton, *simpleButton, *normalButton;
  GSList *list;

  frame = gtk_frame_new( "Graphics" );

  box = gtk_hbox_new( FALSE, 10 );
  gtk_container_border_width (GTK_CONTAINER (box), 5);

  twoDButton = gtk_radio_button_new_with_label ( (GSList *)NULL, "2D");
  list = gtk_radio_button_group( GTK_RADIO_BUTTON(twoDButton) );
  gtk_box_pack_start( GTK_BOX(box), twoDButton, FALSE, FALSE, 10 );
  gtk_widget_show (twoDButton);
  if ( gmode == GMODE_2D ) {
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(twoDButton), TRUE );
  }

  simpleButton = gtk_radio_button_new_with_label ( list, "Simple");
  list = gtk_radio_button_group( GTK_RADIO_BUTTON(simpleButton) );
  gtk_box_pack_start( GTK_BOX(box), simpleButton, FALSE, FALSE, 10 );
  gtk_widget_show (simpleButton);
  if ( gmode == GMODE_SIMPLE ) {
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(simpleButton), TRUE );
  }

  normalButton = gtk_radio_button_new_with_label (list, "Normal");
  list = gtk_radio_button_group( GTK_RADIO_BUTTON(normalButton) );
  gtk_box_pack_start( GTK_BOX(box), normalButton, FALSE, FALSE, 10 );
  gtk_widget_show (normalButton);
  if ( gmode == GMODE_FULL ) {
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(normalButton), TRUE );
  }

  gtk_signal_connect (GTK_OBJECT (twoDButton), "clicked",
		      GTK_SIGNAL_FUNC (LauncherHeader::Toggle), &gmode);
  gtk_signal_connect (GTK_OBJECT (simpleButton), "clicked",
		      GTK_SIGNAL_FUNC (LauncherHeader::Toggle), &gmode);
  gtk_signal_connect (GTK_OBJECT (normalButton), "clicked",
		      GTK_SIGNAL_FUNC (LauncherHeader::Toggle), &gmode);

  gtk_widget_show (box);
  gtk_container_add (GTK_CONTAINER (frame), box);

  return frame;
}

void
LauncherHeader::ToggleFullScreen( GtkWidget *widget, gpointer data ) {
  GSList *list = gtk_radio_button_group( (GtkRadioButton *)widget );

  if ( gtk_toggle_button_get_active
       ( (GtkToggleButton *)g_slist_nth_data( list, 0 ) ) ) {
    fullScreen = false;
  } else {
    fullScreen = true;
  }
}

void
LauncherHeader::Toggle( GtkWidget *widget, gpointer data ) {
  GSList *list = gtk_radio_button_group( (GtkRadioButton *)widget );

  if ( gtk_toggle_button_get_active
       ( (GtkToggleButton *)g_slist_nth_data( list, 0 ) ) ) {
    gmode = GMODE_FULL;
  } else if ( gtk_toggle_button_get_active
	      ( (GtkToggleButton *)g_slist_nth_data( list, 1 ) ) ) {
    gmode = GMODE_SIMPLE;
  } else if ( gtk_toggle_button_get_active
	      ( (GtkToggleButton *)g_slist_nth_data( list, 2 ) ) ) {
    gmode = GMODE_2D;
  }
}

ModeNote::ModeNote() {
}

ModeNote::~ModeNote() {
}

void
ModeNote::Init( GtkBox *box ) {
  GtkWidget *notebook;
  GtkWidget *noteBox;
  GtkWidget *label;

  notebook = gtk_notebook_new();

  // Solo Play
  noteBox = InitSoloPlayPanel();
  label = gtk_label_new( "Solo" );
  gtk_notebook_append_page( GTK_NOTEBOOK(notebook), noteBox, label);

  // LAN Play
  noteBox = InitLANPlayPanel();
  label = gtk_label_new( "LAN" );
  gtk_notebook_append_page( GTK_NOTEBOOK(notebook), noteBox, label);

  // Internet Play
  noteBox = InitInternetPlayPanel();
  label = gtk_label_new( "Internet(Experimental)" );
  gtk_notebook_append_page( GTK_NOTEBOOK(notebook), noteBox, label);

  gtk_box_pack_start( box, notebook, FALSE, FALSE, 10 );
  gtk_widget_show (notebook);
}

GtkWidget *
ModeNote::InitSoloPlayPanel() {
  GtkWidget *box;
  GtkWidget *button;

  box = gtk_vbox_new( FALSE, 10 );
  gtk_container_border_width (GTK_CONTAINER (box), 5);

  button = gtk_button_new_with_label ("Game Start!");
  gtk_box_pack_start( GTK_BOX(box), button, FALSE, FALSE, 10 );
  gtk_widget_show (button);

  gtk_signal_connect( GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (ModeNote::StartGame), NULL );

  gtk_widget_show (box);

  return box;
}

GtkWidget *
ModeNote::InitLANPlayPanel() {
  GtkWidget *box, *toggleBox, *editBox;
  GtkWidget *button, *label;
  GtkWidget *toggleButton[2];
  GSList *list;

  box = gtk_vbox_new( FALSE, 5 );
  gtk_container_border_width (GTK_CONTAINER (box), 5);

  // Upper toggle button
  toggleBox = gtk_hbox_new( FALSE, 10 );
  gtk_container_border_width (GTK_CONTAINER (toggleBox), 5);

  toggleButton[0] = gtk_radio_button_new_with_label ((GSList *)NULL, "Server");
  list = gtk_radio_button_group( GTK_RADIO_BUTTON(toggleButton[0]) );
  gtk_box_pack_start( GTK_BOX(toggleBox), toggleButton[0], FALSE, FALSE, 5 );
  gtk_widget_show (toggleButton[0]);
  toggleButton[1] = gtk_radio_button_new_with_label (list, "Client");
  list = gtk_radio_button_group( GTK_RADIO_BUTTON(toggleButton[1]) );
  gtk_box_pack_start( GTK_BOX(toggleBox), toggleButton[1], FALSE, FALSE, 5 );
  gtk_widget_show (toggleButton[1]);

  gtk_widget_show (toggleBox);
  gtk_box_pack_start( GTK_BOX(box), toggleBox, FALSE, FALSE, 5 );

  if (serverName[0])
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(toggleButton[1]), TRUE );

  // Lower input area for server name
  editBox = gtk_table_new( 2, 2, FALSE );

  label = gtk_label_new( "Server name:" );
  gtk_table_attach( GTK_TABLE(editBox), label, 0, 1, 1, 2,
		    GTK_FILL, GTK_EXPAND, 0, 0 );
  gtk_widget_show (label);
  m_serverName = gtk_entry_new ();
  gtk_table_attach( GTK_TABLE(editBox), m_serverName, 1, 2, 1, 2,
		    GTK_FILL, GTK_EXPAND, 0, 0 );
  if ( serverName[0] > 1 ) {
    gtk_entry_set_text( GTK_ENTRY(m_serverName), serverName );
    gtk_widget_show ( GTK_WIDGET(m_serverName) );
  }

  gtk_widget_show (editBox);
  gtk_box_pack_start( GTK_BOX(box), editBox, FALSE, FALSE, 5 );

  gtk_signal_connect (GTK_OBJECT (toggleButton[0]), "pressed",
		      GTK_SIGNAL_FUNC (ModeNote::Toggle), m_serverName);
  gtk_signal_connect (GTK_OBJECT (toggleButton[1]), "pressed",
		      GTK_SIGNAL_FUNC (ModeNote::Toggle), m_serverName);

  // lowest area(start button)
  button = gtk_button_new_with_label ("Game Start!");
  gtk_box_pack_start( GTK_BOX(box), button, FALSE, FALSE, 5 );
  gtk_widget_show (button);

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
		      GTK_SIGNAL_FUNC (ModeNote::LANStartGame), m_serverName);

  gtk_widget_show (box);

  return box;
}

GtkWidget *
ModeNote::InitInternetPlayPanel() {
  GtkWidget *box, *editBox;
  GtkWidget *button, *label;

  box = gtk_vbox_new( FALSE, 10 );
  gtk_container_border_width (GTK_CONTAINER (box), 5);

  editBox = gtk_table_new( 2, 2, FALSE );

  label = gtk_label_new( "Nickname:" );
  gtk_table_attach( GTK_TABLE(editBox), label, 0, 1, 0, 1,
		    GTK_FILL, GTK_EXPAND, 0, 0 );
  gtk_widget_show (label);
  m_lobbyEdit[0] = gtk_entry_new ();
  gtk_table_attach( GTK_TABLE(editBox), m_lobbyEdit[0], 1, 2, 0, 1,
		    GTK_FILL, GTK_EXPAND, 0, 0 );
  gtk_widget_show (m_lobbyEdit[0]);
  gtk_entry_set_text( GTK_ENTRY(m_lobbyEdit[0]), nickname );

  label = gtk_label_new( "Message:" );
  gtk_table_attach( GTK_TABLE(editBox), label, 0, 1, 1, 2,
		    GTK_FILL, GTK_EXPAND, 0, 0 );
  gtk_widget_show (label);
  m_lobbyEdit[1] = gtk_entry_new ();
  gtk_table_attach( GTK_TABLE(editBox), m_lobbyEdit[1], 1, 2, 1, 2,
		    GTK_FILL, GTK_EXPAND, 0, 0 );
  gtk_widget_show (m_lobbyEdit[1]);
  gtk_entry_set_text( GTK_ENTRY(m_lobbyEdit[1]), message );

  gtk_widget_show (editBox);
  gtk_box_pack_start( GTK_BOX(box), editBox, FALSE, FALSE, 5 );

  button = gtk_button_new_with_label("Connect to Lobby Server");
  gtk_box_pack_start( GTK_BOX(box), button, FALSE, FALSE, 10 );
  gtk_widget_show(button);

  gtk_signal_connect( GTK_OBJECT(button), "clicked",
		      GTK_SIGNAL_FUNC(ModeNote::InternetStartGame), m_lobbyEdit);

  gtk_widget_show (box);

  return box;
}

void
ModeNote::Toggle( GtkWidget *widget, gpointer data ) {
  GSList *list = gtk_radio_button_group( (GtkRadioButton *)widget );

  if ( g_slist_index( list, widget ) == 0 ) {
    serverName[0] = 1;	// :-p
    gtk_widget_show ( GTK_WIDGET(data) );
  } else {
    serverName[0] = '\0';
    gtk_widget_hide ( GTK_WIDGET(data) );
  }
}

void
ModeNote::StartGame( GtkWidget *widget, gpointer data ) {
  if ( gmode == GMODE_2D )
    mode = MODE_TITLE;
  ::StartGame();
  ::EventLoop();
}

void
ModeNote::LANStartGame( GtkWidget *widget, gpointer data ) {
  if ( serverName[0] == 1 &&
       strlen(gtk_entry_get_text( GTK_ENTRY(data) )) > 0 )
    strncpy( serverName, gtk_entry_get_text( GTK_ENTRY(data) ), 256 );
  isComm = true;
  mode = MODE_SELECT;
  ::StartGame();
  ::EventLoop();
}

void
ModeNote::InternetStartGame( GtkWidget *widget, gpointer data ) {
  LobbyClient *lb;
  lb = new LobbyClient();
  lb->Init( gtk_entry_get_text( GTK_ENTRY(((GtkWidget **)data)[0]) ),
	    gtk_entry_get_text( GTK_ENTRY(((GtkWidget **)data)[1]) ) );
}


Launcher::Launcher() {
}

Launcher::~Launcher() {
}

void
Launcher::Init() {
  GtkWidget *frame;
  GtkWidget *edit;
  GtkWidget *button;

  GtkWidget *label;

  GtkWidget *allbox, *mainbox, *quitBox;

  /* 初期化 */
  gtk_init( (int *)NULL, (char ***)NULL );

  /* Window 生成 */
  m_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_border_width (GTK_CONTAINER (m_window), 5);
  gtk_window_set_title( GTK_WINDOW(m_window), "Cannon Smash");

  allbox = gtk_hbox_new( FALSE, 5 );
  mainbox = gtk_vbox_new( FALSE, 5 );

  m_header = new LauncherHeader();
  m_header->Init( GTK_BOX(mainbox) );
  m_note = new ModeNote();
  m_note->Init( GTK_BOX(mainbox) );

  label = gtk_label_new( "http://CannonSmash.SourceForge.net\nmailto: nan@utmc.or.jp" );
  gtk_label_set_justify( GTK_LABEL(label), GTK_JUSTIFY_LEFT );
  gtk_widget_show(label);
  gtk_box_pack_start( GTK_BOX(mainbox), label, FALSE, TRUE, 0 );

  quitBox = gtk_vbox_new( FALSE, 5 );

  m_quit = gtk_button_new_with_label ("Quit");
  gtk_box_pack_start( GTK_BOX(quitBox), m_quit, FALSE, TRUE, 10 );

  gtk_box_pack_start( GTK_BOX(allbox), mainbox, FALSE, FALSE, 10 );
  gtk_box_pack_start( GTK_BOX(allbox), quitBox, FALSE, TRUE, 10 );

  gtk_signal_connect( GTK_OBJECT (m_window), "destroy",
		      GTK_SIGNAL_FUNC (Launcher::Destroy), this );
  gtk_signal_connect( GTK_OBJECT (m_quit), "clicked",
		      GTK_SIGNAL_FUNC (Launcher::Destroy),
		      this );

  gtk_container_add (GTK_CONTAINER (m_window), allbox);

  gtk_widget_show(m_quit);
  gtk_widget_show(mainbox);
  gtk_widget_show(quitBox);
  gtk_widget_show(allbox);
  gtk_widget_show(m_window);

  gtk_main();
}

void
Launcher::Destroy(GtkWidget *widget, gpointer data) {
  ModeNote *note = ((Launcher *)data)->m_note;

  strncpy( serverName,
	   gtk_entry_get_text( GTK_ENTRY(note->m_serverName) ), 256 );
  strncpy( nickname,
	   gtk_entry_get_text( GTK_ENTRY(note->m_lobbyEdit[0]) ), 32 );
  strncpy( message,
	   gtk_entry_get_text( GTK_ENTRY(note->m_lobbyEdit[1]) ), 64 );

  WriteRCFile();

  gtk_main_quit();

  gtk_exit(2);
}

