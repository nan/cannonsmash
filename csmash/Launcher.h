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

// Upper half of initial window
class LauncherHeader {
public:
  LauncherHeader();
  ~LauncherHeader();

  void Init( GtkBox *box );

protected:
  GtkWidget * FullScreenFrame();
  GtkWidget * GraphicsFrame();

  static void Toggle( GtkWidget *widget, gpointer data );
};


// Lower half of initial window
class ModeNote {
public:
  ModeNote();
  ~ModeNote();

  void Init( GtkBox *box );

protected:
  GtkWidget * InitSoloPlayPanel();
  GtkWidget * InitLANPlayPanel();
  GtkWidget * InitInternetPlayPanel();

  static void Toggle( GtkWidget *widget, gpointer data );
  static void StartGame( GtkWidget *widget, gpointer data );
  static void LANStartGame( GtkWidget *widget, gpointer data );
  static void InternetStartGame( GtkWidget *widget, gpointer data );
};

class Launcher {
public:
  Launcher();
  ~Launcher();

  void Init();

  static void Destroy(GtkWidget *widget, gpointer data);
protected:
  GtkWidget *m_window;		// main window
  LauncherHeader *m_header;	// upper area
  ModeNote *m_note;		// mode notebook(solo, LAN, Internet)

  GtkWidget *m_quit;		// quit button
};
