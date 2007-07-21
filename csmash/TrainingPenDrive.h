/**
 * @file
 * @brief Definition of TrainingPenDrive class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2004, 2007  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _TrainingPenDrive_
#define _TrainingPenDrive_
#include "PenDrive.h"

/**
 * TrainingPenDrive class is a Player class of pen drive type player. 
 * This player is the opponent player in training mode. 
 */
class TrainingPenDrive : public PenDrive {
public:
  TrainingPenDrive();
  TrainingPenDrive(long side);
  TrainingPenDrive( long playerType, long side, const vector3d x,
		    const vector3d v, long status, long swing, long swingType,
		    bool swingSide, long afterSwing, long swingError,
		    const vector2d target, const vector3d eye,
		    const vector3d lookAt, long pow, const vector2d spin,
		    double stamina, long statusMax, long dragX, long dragY );
  virtual ~TrainingPenDrive();

  virtual bool AddStatus( long diff );

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual bool HitBall();
};

#endif // _PenDrive__
