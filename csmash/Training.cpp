/**
 * @file
 * @brief Implementation of Training class. 
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

#include "ttinc.h"
#include "Training.h"
#include "TrainingView.h"
#include "BaseView.h"
#include "Player.h"
#include "Ball.h"
#include "Event.h"

extern Ball theBall;
extern long mode;

/**
 * Default constructor. 
 */
Training::Training() {
  m_View = NULL;
  m_trainingCount = 0;
  m_trainingMax = 0;
}

/**
 * Destructor. 
 * Detatch view and delete it. 
 */
Training::~Training() {
  if ( m_View ){
    BaseView::TheView()->RemoveView( m_View );
    delete m_View;
  }
}

/**
 * Initializer method. 
 * Create TrainingView and attach it to this object. 
 * 
 * @return returns true if succeeds. 
 */
bool
Training::Init() {
  m_View = (TrainingView *)View::CreateView( VIEW_TRAINING );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  return true;
}

/**
 * Training object creater. 
 * This method creates singleton Training object and two players. 
 * 
 * @param player type of the player controlled by this game player. 
 * @param com type of the player controlled by the opponent game player. 
 */
void
Training::Create( long player, long com ) {
  Control::ClearControl();

  m_theControl = new Training();
  m_theControl->Init();

  m_thePlayer = Player::Create( player, 1, 2 );
  m_comPlayer = Player::Create( com, -1, 3 );

  m_thePlayer->Init();
  m_comPlayer->Init();

  // Move it to view?
}

/**
 * Move valid objects. 
 * Call Move method of the ball and players. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
Training::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		long *MouseYHistory, unsigned long *MouseBHistory,
		int Histptr ) {
  bool reDraw = false;
  long ballStatus = theBall.GetStatus();

  if ( KeyHistory[Histptr].unicode == 'Q' ) {
    mode = MODE_TITLE;
    return true;
  }

  theBall.Move();
  reDraw |= m_thePlayer->Move( KeyHistory, MouseXHistory,
			     MouseYHistory, MouseBHistory, Histptr );
  reDraw |= m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

  if ( ballStatus != theBall.GetStatus() ) {
    if ( theBall.GetStatus() == 8 ) {
      if ( m_trainingCount > m_trainingMax )
	m_trainingMax = m_trainingCount;
      m_trainingCount = 0;
    }
  }

  return reDraw;
}

/**
 * Set camera position and direction. 
 * 
 * @param srcX camera position [out]
 * @param destX point where the camera is directed. [out]
 * @return returns true if succeeds. 
 */
bool
Training::LookAt( vector3d &srcX, vector3d &destX ) {
  if (m_thePlayer) {
    srcX = m_thePlayer->GetX() + m_thePlayer->GetEye();
    destX = m_thePlayer->GetLookAt();
  }

  return true;
}

/**
 * For each rally, training count is incremented. 
 */
void
Training::AddTrainingCount() {
  m_trainingCount++;
}
