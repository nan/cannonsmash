/**
 * @file
 * @brief Implementation of TrainingHumanController class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2007  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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

#include "ttinc.h"
#include "TrainingHumanController.h"
#include "Control.h"
#include "PlayGame.h"

/***********************************************************************
 *	Class  TrainingHumanController
 ***********************************************************************/

/**
 * Default constructor. 
 */
TrainingHumanController::TrainingHumanController() : HumanController() {
}

/**
 * Constructor. 
 * Set parent. 
 * 
 * @param side side of the player. 
 */
TrainingHumanController::TrainingHumanController( Player *parent ) : HumanController(parent){
  m_parent = parent;
}

/**
 * Destructor. 
 * Detach view class. 
 */
TrainingHumanController::~TrainingHumanController() {
}

/**
 * Move this player object. 
 * Move this player and change m_status. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
TrainingHumanController::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
			       long *MouseYHistory, unsigned long *MouseBHistory,
			       int Histptr ) {

  if ( Control::TheControl()->IsPlaying() &&
	 !((PlayGame *)Control::TheControl())->IsPause() )
    KeyCheck( KeyHistory, MouseXHistory, MouseYHistory, MouseBHistory,Histptr );

  vector3d v = m_parent->GetV();
  v[1] = v[2] = 0.0;
  m_parent->SetV(v);
  //m_x[1] = -(TABLELENGTH/2+0.3)*m_side;
  vector2d target = vector2d(-TABLEWIDTH/5*m_parent->GetSide(), TABLELENGTH/16*4*m_parent->GetSide());
  m_parent->SetTarget(target);

  return true;
}
