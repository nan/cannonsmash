/**
 * @file
 * @brief Definition of MultiPlayerSelect class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2003  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _MultiPlayerSelect_
#define _MultiPlayerSelect_

#include "PlayerSelect.h"
#include "PracticeSelectView.h"

/**
 * MultiPlayerSelect class is the controller class for player select mode
 * of multiplayer. 
 */
class MultiPlayerSelect : public PlayerSelect {
public:
  MultiPlayerSelect();
  virtual ~MultiPlayerSelect();

  virtual bool Init();

  static void Create();

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual long GetOpponentNum();
  /** Getter method of m_opponentRotate */
  virtual long GetOpponentRotate() { return m_opponentRotate; };
  /** Getter method of m_opponentSelected */
  virtual long GetOpponentSelected() { return m_opponentSelected; };

  static int Connect( void *dum );

  void ReadPT( char *data );
  void SendPT( char fixed );

protected:
  long m_opponentRotate;	///< Rotation angle of opponent player
  long m_opponentSelected;	///< If selected, m_opponentSelected > 0


  long m_lastRotate;		///< Last rotation angle
  long m_lastOpponentRotate;	///< Last rotation angle of opponent player

  bool m_isConnected;		///< If this machine is already connected to the opponent machine, m_isConnected is true. 

  SDL_Thread *m_connectThread;	///< Thread for connecting to the opponent machine. 
};

#endif	// _MultiPlayerSelect_
