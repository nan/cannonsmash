/**
 * @file
 * @brief Definition of LogPlay class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2007  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _LogPlay_
#define _LogPlay_
#include "PlayGame.h"

/**
 * LogPlay class is the controller class for log play (replay log file) mode. 
 */
class LogPlay : public PlayGame {
public:
  LogPlay();
  virtual ~LogPlay();

  virtual bool Init();
  static void Create();

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual bool LookAt( vector3d &srcX, vector3d &destX );

protected:
  FILE *fpBall;
  FILE *fpThePlayer;
  FILE *fpComPlayer;

  void ScreenShot();
};

#endif	// _LogPlay_
