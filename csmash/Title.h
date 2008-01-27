/**
 * @file
 * @brief Definition of Title class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004, 2007  Kanna Yoshihiro
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

#ifndef _Title_
#define _Title_
#include "Control.h"
#include "TitleView.h"

// major menu
#define MENU_MAIN	0
#define MENU_CONFIG	1
#define MENU_LOGIN	2
#define MENU_LOBBY	3

// minor menu
#define MENU_ALL		0	// number of all menus
#define MENU_CONFIG_LEVEL	1	// level menu(in config menu)
#define MENU_CONFIG_MODE	2	// mode(in config menu)
#define MENU_CONFIG_PLAYER	3	// player(in config menu)

/**
 * Title class is a controller class for showing title menu. 
 */
class Title : public Control {
public:
  Title();
  virtual ~Title();

  virtual bool Init();

  static void Create();

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  long GetSelectMode();
  void SetSelectMode(long selectMode);
  long GetCount();

  virtual bool LookAt( vector3d &srcX, vector3d &destX );

  virtual bool IsPlaying() { return false; }	///< Always returns false. 

  virtual View *GetView() { return m_View; }	///< Getter method fo m_View
protected:
  TitleView *m_View;	///< Reference to attached TitleView object
  long m_selectMode;	/**< If it is normal, m_selectMode == 0
			 *   If it is config, m_selectMode == 1
			 */
  long m_count;		///< TICKs from this title menu is started
};

#endif	// _Title_
