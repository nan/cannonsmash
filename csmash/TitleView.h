/**
 * @file
 * @brief Definition of Title class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2002, 2007  Kanna Yoshihiro
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

#ifndef _TitleView_
#define _TitleView_
#include "View.h"
#include "LoadImage.h"

class Title;
class LobbyClient;

/**
 * TitleView class is a view class which corresponds to Title object. 
 */
class TitleView : public View {
public:
  TitleView();
  virtual ~TitleView();

  virtual bool Init( Title * );

  virtual bool Redraw();
  virtual bool RedrawAlpha();

  virtual long GetNextMode() { return m_nextMode; }		///< Getter method of m_nextMode

  virtual void popupPIDialog( long uniqID );

  virtual void CreateMenu( long menuMajorNum );
protected:
  Title       *m_title;		///< Attached Title object. 
  long         m_nextMode;

  long m_popupUniqID;		///< player ID of the player who wants to play with this player

  virtual void CreateMainMenu();
  bool handleMainStartGame(const CEGUI::EventArgs& e);
  bool handleMainLoginLobby(const CEGUI::EventArgs& e);
  bool handleMainPractice(const CEGUI::EventArgs& e);
  bool handleMainTraining(const CEGUI::EventArgs& e);
  bool handleMainHowtoPlay(const CEGUI::EventArgs& e);
  bool handleMainConfig(const CEGUI::EventArgs& e);
  bool handleMainQuit(const CEGUI::EventArgs& e);

  virtual void CreateConfigMenu();
  bool handleConfigScreenMenuSelected(const CEGUI::EventArgs& e);
  bool handleConfigGraphicsMenuSelected(const CEGUI::EventArgs& e);
  bool handleConfigGraphicsTypeMenuSelected(const CEGUI::EventArgs& e);

  bool handleConfigSoundSliderChanged(const CEGUI::EventArgs& e);

  bool handleConfigInvertMouseChanged(const CEGUI::EventArgs& e);

  bool handleConfigGameModeLevelMenuSelected(const CEGUI::EventArgs& e);
  bool handleConfigGameModeModeMenuSelected(const CEGUI::EventArgs& e);

  bool handleConfigBack(const CEGUI::EventArgs& e);

  virtual void CreateLoginMenu();
  bool handleLoginLogin(const CEGUI::EventArgs& e);
  bool handleLoginCancel(const CEGUI::EventArgs& e);

  virtual void CreateLobbyMenu();
  bool handleLobbyCheckSelection(const CEGUI::EventArgs& e);
  bool handleLobbyChatInput(const CEGUI::EventArgs& e);
  bool handleLobbyConnect(const CEGUI::EventArgs& e);
  bool handleLobbyLogout(const CEGUI::EventArgs& e);

  bool handlePIDialogOK(const CEGUI::EventArgs& e);
  bool handlePIDialogNo(const CEGUI::EventArgs& e);
};

#endif	// _TitleView_
