/**
 * @file
 * @brief Definition of LauncherView and related classes. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

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

/**
 * LauncherHeader class manages upper half (toggle button of fullscreen, 
 * sound and graphics settings) of the initial dialog. 
 */
class LauncherHeader {
public:
  LauncherHeader();
  ~LauncherHeader();

  void Init( GtkBox *box );

protected:
  GtkWidget * FullScreenFrame();
  GtkWidget * SoundFrame();
  GtkWidget * GraphicsFrame();

  static void Toggle( GtkWidget *widget, gpointer data );
  static void ToggleFullScreen( GtkWidget *widget, gpointer data );
  static void ToggleSound( GtkWidget *widget, gpointer data );

#ifdef ENABLE_IPV6
  GtkWidget * ProtocolFrame();
  static void ToggleProtocol( GtkWidget *widget, gpointer data );
#endif
};


// Lower half of initial window
/**
 * ModeNote class manages lower half (panel for solo play, LAN play and
 * the internet play) of the initial dialog. 
 */
class ModeNote {
public:
  ModeNote();
  ~ModeNote();

  void Init( GtkBox *box );

  GtkWidget *m_serverName;		///< Server name input area. (For LAN)
  GtkWidget *m_lobbyEdit[2];		///< Name and message input area (For internet). 
protected:
  GtkWidget * InitSoloPlayPanel();
  GtkWidget * InitLANPlayPanel();
  GtkWidget * InitInternetPlayPanel();

  static void Toggle( GtkWidget *widget, gpointer data );
  static void StartGame( GtkWidget *widget, gpointer data );
  static void LANStartGame( GtkWidget *widget, gpointer data );
  static void InternetStartGame( GtkWidget *widget, gpointer data );

#ifdef WIN32
  static LRESULT CALLBACK EditWindowProc( HWND hwnd, UINT msg,
					  WPARAM wparam, LPARAM lparam);
  static LRESULT CALLBACK ParentWindowProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam);
  static LONG pEditWndProc;		///< Edit window message handler. 
  static LONG pParentWndProc;		///< Parent window message handler. 
  static HWND pChildHWnd;		///< Child window handler. 
#endif
};

/**
 * LauncherView is a view class for the initial dialog. 
 */
class LauncherView {
public:
  LauncherView();
  ~LauncherView();

  void Init();

  static void Destroy(GtkWidget *widget, gpointer data);
  static void ConnectionFailedDialog();

#ifdef WIN32
  static HWND hWnd;		///< Window handler of the initial dialog. 
#endif
protected:
  GtkWidget *m_window;		///< main window control
  LauncherHeader *m_header;	///< upper area manager
  ModeNote *m_note;		///< lower area manager

  GtkWidget *m_quit;		///< quit button control
};
