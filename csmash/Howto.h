/**
 * @file
 * @brief Definition of Howto class. 
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

#ifndef _Howto_
#define _Howto_
#include "Control.h"
#include "HowtoView.h"

/**
 * Controller class for showing howto message. 
 * Howto is created when the game player selects to see tutorial. 
 * During tutorial, Howto works as controller class. 
 */
class Howto : public Control {
public:
  Howto();
  virtual ~Howto();

  bool Init();

  static void Create();

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  long GetMode() {return m_mode;}	///< Returns m_mode. 
  long GetCount() {return m_count;}	///< Returns m_count. 

  long GetMouseX() {return m_mouseX;}	///< Returns m_mouseX. 
  long GetMouseY() {return m_mouseY;}	///< Returns m_mouseY. 
  long GetMouseB() {return m_mouseB;}	///< Returns m_mouseB. 

  virtual bool LookAt( vector3d &srcX, vector3d &destX );

  virtual bool IsPlaying() { return false; }	///< Always returns false. 

  virtual View *GetView() { return m_View; }	///< Returns m_View. 
protected:
  HowtoView *m_View;	///< HowtoView object related with Howto. 
  long m_mode;		///< indicates what type of tutorial is shown now. 
  long m_count;		///< duration of time from m_mode has changed. 

  long m_mouseX;	///< x-coordinate of the mouse shown in tutorial. 
  long m_mouseY;	///< y-coordinate of the mouse shown in tutorial. 
  long m_mouseB;	///< button status of the mouse shown in tutorial. 

  bool IsMove();
};

#endif	// _Howto_
