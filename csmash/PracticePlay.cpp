/**
 * @file
 * @brief Implementation of PracticePlay class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2001, 2002, 2004  神南 吉宏(Kanna Yoshihiro)
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
#include "Event.h"
#include "PracticePlay.h"
#include "Player.h"
#include "Ball.h"

extern Ball theBall;
extern long mode;

/**
 * Default constructor. 
 */
PracticePlay::PracticePlay() : SoloPlay() {
}

/**
 * Destructor. 
 * Do nothing. 
 */
PracticePlay::~PracticePlay() {
}

/**
 * PracticePlay object creater. 
 * This method creates singleton PracticePlay object. 
 * 
 * @param player type of the player controlled by this game player. 
 * @param com type of the player controlled by the opponent game player. 
 */
void
PracticePlay::Create( long player, long com ) {
  Control::ClearControl();

  m_theControl = new PracticePlay();
  m_theControl->Init();

  m_thePlayer = Player::Create( player, 1, 0 );
  m_comPlayer = Player::Create( com, -1, 1 );

  m_thePlayer->Init();
  m_comPlayer->Init();

  // Move it to view?
}

/**
 * A method to quit game. 
 */
void
PracticePlay::EndGame() {
  mode = MODE_TITLE;
}
