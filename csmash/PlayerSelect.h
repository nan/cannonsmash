/**
 * @file
 * @brief Definition of PlayerSelect class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2001, 2004  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _PlayerSelect_
#define _PlayerSelect_

#include "Control.h"
#include "PlayerSelectView.h"

/**
 * PlayerSelect class is a controller classes for selecting player. 
 */
class PlayerSelect : public Control {
public:
  PlayerSelect();
  virtual ~PlayerSelect();

  virtual bool Init();

  static void Create();

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual long GetRotate() { return m_rotate; }		///< Getter method of m_rotate. 
  virtual long GetPlayerNum();
  virtual long GetOpponentNum();
  virtual long GetSelected() { return m_selected; }	///< Getter method of m_selected. 

  virtual bool LookAt( vector3d &srcX, vector3d &destX );

  virtual bool IsPlaying() { return false; }		///< Always returns false. 

    virtual View *GetView() { return m_View; }		///< Getter method of m_View. 
protected:
  PlayerSelectView *m_View;	///< Reference to PlayerSelectView object attached to this object. 
  long m_rotate;	///< Rotation of players in degree. 
  long m_selected;	///< If the player is selected, m_selected > 0. 
};

#endif	// _PlayerSelect_
