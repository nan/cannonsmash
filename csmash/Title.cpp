/**
 * @file
 * @brief Implementation of Title class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2000-2004, 2007  Kanna Yoshihiro
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
#include "Title.h"
#include "BaseView.h"
#include "BaseView2D.h"
#include "Player.h"
#include "Ball.h"
#include "TitleView.h"
#include "TitleView2D.h"
#include "Event.h"
#include "RCFile.h"
#include "LobbyClient.h"
#include <time.h>

extern RCFile *theRC;

extern long mode;

extern void QuitGame();

extern Ball theBall;

/**
 * Constructor. 
 * Initialize member variables to 0 or NULL. 
 */
Title::Title() {
  m_View = NULL;
  m_selectMode = 0;
  m_count = 0;
}

/**
 * Destructor. 
 * Delete menu items. Detach attached view class and delete it. 
 */
Title::~Title() {
  if ( m_View ){
    BaseView::TheView()->RemoveView( m_View );
    delete m_View;
  }
}

/**
 * Initializer method. 
 * Create TitleView object, two players and initialize them. 
 * 
 * @return returns true if succeeds. 
 */
bool
Title::Init() {
  m_View = (TitleView *)View::CreateView( VIEW_TITLE );
  m_View->Init( this );
  SetSelectMode(MENU_MAIN);

  BaseView::TheView()->AddView( m_View );

  m_thePlayer = Player::Create( RAND(3)+1, 1, 1 );
  m_comPlayer = Player::Create( RAND(3)+1, -1, 1 );

  m_thePlayer->Init();
  m_comPlayer->Init();

  return true;
}

/**
 * Title object creater. 
 * This method creates singleton Title object. 
 */
void
Title::Create() {
  Control::ClearControl();

  m_theControl = new Title();
  m_theControl->Init();
}

/**
 * Move valid objects. 
 * Call Move method of the ball and players. 
 * Additionally, this method handles the selection of menu. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
Title::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
	     long *MouseYHistory, unsigned long *MouseBHistory,
	     int Histptr ) {
  long last = Histptr-1;
  if ( last < 0 )
    last = MAX_HISTORY-1;

  theBall.Move();
  m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
  m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

  if ( (m_selectMode == MENU_CONFIG || m_selectMode == MENU_LOGIN) &&
       KeyHistory[Histptr].unicode == SDLK_ESCAPE ) {
    m_selectMode = MENU_MAIN;

    return true;
  }

  if (m_View->GetNextMode() != MODE_TITLE) {
    theBall.Warp( vector3d(0.0), vector3d(0.0), vector2d(0.0), -1000 );
    mode = m_View->GetNextMode();
  }

  m_count++;

  if (LobbyClient::TheLobbyClient()) {
    LobbyClient::TheLobbyClient()->PollServerMessage();
  }

  return true;
}

/**
 * Return whether main menu is shown or config menu is shown. 
 * 
 * @returns returns type of menu. 
 */
long
Title::GetSelectMode() {
  return m_selectMode;
}

/**
 * Return whether main menu is shown or config menu is shown. 
 * 
 * @returns returns type of menu. 
 */
void
Title::SetSelectMode(long selectMode) {
  m_selectMode = selectMode;
  if (m_View)
    m_View->CreateMenu(m_selectMode);
}

/**
 * Getter method of m_count. 
 */
long
Title::GetCount() {
  return m_count;
}

/**
 * Set camera position and direction. 
 * 
 * @param srcX camera position [out]
 * @param destX point where the camera is directed. 
 * @return returns true if succeeds. 
 */
bool
Title::LookAt( vector3d &srcX, vector3d &destX ) {
  srcX[0] = TABLELENGTH*2*cos(GetCount()*3.14159265/720.0);
  srcX[1] = TABLELENGTH*2*sin(GetCount()*3.14159265/720.0) + TABLELENGTH/2;
  srcX[2] = TABLEHEIGHT*4;

  return true;
}
