/* $Id$ */

// Copyright (C) 2000  $B?@Fn(B $B5H9((B(Kanna Yoshihiro)
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

#ifndef _PlayGame_
#define _PlayGame_

#include "Control.h"
class Player;

// Base class of SoloPlay, MultiPlay, Training
class PlayGame : public Control {
public:
  PlayGame();
  virtual ~PlayGame();

  virtual bool Init() = 0;

  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr ) = 0;
  virtual bool LookAt( double &srcX, double &srcY, double &srcZ,
		       double &destX, double &destY, double &destZ ) = 0;

  virtual bool IsPlaying() { return true; };

  long GetService();
  long GetScore( Player *p );
  long GetScore( long side );

  bool IsGameEnd();
  virtual void EndGame();
  void ChangeScore();
  void ChangeScore( long score1, long score2 );
protected:

  long m_Score1;	// Score of near side. Counter when training
  long m_Score2;	// Score of far side
};

#endif	// _PlayGame_
