/**
 * @file
 * @brief Definition of PracticeSelect class. 
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

#ifndef _PracticeSelect_
#define _PracticeSelect_

#include "PlayerSelect.h"
#include "PracticeSelectView.h"

/**
 * PracticeSelect class is a controller classes for selecting player on practice play. 
 */
class PracticeSelect : public PlayerSelect {
public:
  PracticeSelect();
  virtual ~PracticeSelect();

  virtual bool Init();

  static void Create();

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual long GetOpponentRotate() { return m_opponentRotate; }	///< Getter method of m_opponentRotate. 
  virtual long GetOpponentNum();
  virtual long GetOpponentSelected() { return m_opponentSelected; }	///< Getter method of m_opponentSelected. 

protected:
  long m_opponentRotate;	///< Rotation of opponent player in degree
  long m_opponentSelected;	///< If the opponent player is selected, m_selected > 0
};

#endif	// _PracticeSelect_
