/**
 * @file
 * @brief Definition of Control class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _Control_
#define _Control_

#include <vector>
#include "matrix"
typedef Vector<3, double> vector3d;
typedef Vector<2, double> vector2d;

class View;
class Player;

/**
 * Base class of controller class. 
 * When game mode is changed, a subclass of Control class is created. 
 * The Control class manages all other classes to control the progress 
 * of the game. 
 */
class Control {
public:
  virtual ~Control();

  virtual bool Init() = 0;
  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr ) = 0;
  virtual bool LookAt( vector3d &srcX, vector3d &destX ) = 0;

  virtual bool IsPlaying() = 0;

  virtual View *GetView() = 0;

  static void ClearControl();

  /**
   * Getter method of singleton Control object. 
   * @return returns singleton control object. If Control is not created, returns null. 
   */
  static Control *TheControl() { return m_theControl; };

  /**
   * Getter method of Player object which is controlled by the game player. 
   * @return returns Player object. If Player is not created, returns null. 
   */
  static Player* GetThePlayer() { return m_thePlayer; };

  /**
   * Getter method of Player object which is controlled by CPU or opponent. 
   * @return returns Player object. If Player is not created, returns null. 
   */
  static Player* GetComPlayer() { return m_comPlayer; };
protected:
  Control();
  static Control *m_theControl;	///< Singleton Control object. 

  static Player* m_thePlayer;	///< Singleton Player object which is controlled by the game player. 
  static Player* m_comPlayer;	///< Singleton Player object which is not controlled by the game player. 
};

#endif	// _Control_
