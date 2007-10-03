/**
 * @file
 * @brief Definition of TrainingHumanController class. 
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

#ifndef _TrainingHumanController_
#define _TrainingHumanController_

#include "HumanController.h"

/**
 * Training Human controller class. 
 * This class translates the input (Mouse, keyboard)
 * into the behavior of the player. 
 */
class TrainingHumanController : public HumanController {
public:
  TrainingHumanController();
  TrainingHumanController(Player *parent);
  virtual ~TrainingHumanController();

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );
};

#endif	// _TrainingHumanController_
