/**
 * @file
 * @brief Implementation of Title class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2000-2004  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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
#include "MenuItem.h"
#include "RCFile.h"

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
  m_selected = 0;
  m_selectMode = 0;
  m_count = 0;

  for ( int i = 0 ; i < 16 ; i++ )
    m_menuItem[i] = NULL;
}

/**
 * Destructor. 
 * Delete menu items. Detach attached view class and delete it. 
 */
Title::~Title() {
  for ( int i = 0 ; i < 16 ; i++ ) {
    if ( m_menuItem[i] )
      delete m_menuItem[i];
  }

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

  BaseView::TheView()->AddView( m_View );

  m_thePlayer = Player::Create( RAND(3), 1, 1 );
  m_comPlayer = Player::Create( RAND(3), -1, 1 );

  m_thePlayer->Init();
  m_comPlayer->Init();

  CreateMenu( MENU_MAIN );

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

  switch ( m_selectMode ) {
  case MENU_MAIN:
    // Get key input
    if ( KeyHistory[Histptr].unicode != KeyHistory[last].unicode ) {
      switch ( KeyHistory[Histptr].unicode ) {
      case '8':
      case SDLK_UP:
	SetSelected( m_selected-1 );
	break;
      case '2':
      case SDLK_DOWN:
	SetSelected( m_selected+1 );
	break;
      }

      MouseYHistory[Histptr] = BaseView::GetWinHeight() -
	(m_menuItem[m_selected]->GetY()+m_menuItem[m_selected]->GetHeight()/2);
    } else {
      SetSelected( HitTest( MouseXHistory[Histptr], MouseYHistory[Histptr] ) );
    }
    break;
  case MENU_CONFIG:
    // Get key input
    if ( KeyHistory[Histptr].unicode != KeyHistory[last].unicode ) {
      switch ( KeyHistory[Histptr].unicode ) {
      case '8':
      case SDLK_UP:
	SetSelected( m_selected-1 );
	break;
      case '6':
      case SDLK_RIGHT:
	MouseXHistory[Histptr] = BaseView::GetWinWidth()/4*3;
	SetSelected( HitTest( MouseXHistory[Histptr], MouseYHistory[Histptr] ) );
	break;
      case '4':
      case SDLK_LEFT:
	MouseXHistory[Histptr] = BaseView::GetWinWidth()/4;
	SetSelected( HitTest( MouseXHistory[Histptr], MouseYHistory[Histptr] ) );
	break;
      case '2':
      case SDLK_DOWN:
	SetSelected( m_selected+1 );
	break;
      }
      MouseYHistory[Histptr] = BaseView::GetWinHeight() -
	(m_menuItem[m_selected]->GetY()+m_menuItem[m_selected]->GetHeight()/2);
      MouseXHistory[Histptr] = 
	(m_menuItem[m_selected]->GetX()+m_menuItem[m_selected]->GetWidth()/2);
    } else {
      SetSelected( HitTest( MouseXHistory[Histptr], MouseYHistory[Histptr] ) );
    }
  }

  if ( m_selectMode == MENU_CONFIG &&
       KeyHistory[Histptr].unicode == SDLK_ESCAPE ) {
    m_selectMode = MENU_MAIN;
    CreateMenu( m_selectMode );
    m_selected = 0;

    return true;
  }

  if ( m_selected < 0 )
    SetSelected( 0 );
  else {
    if ( m_selectMode == MENU_MAIN && m_selected >= GetMenuNum( MENU_MAIN ) )
      SetSelected( GetMenuNum( MENU_MAIN )-1 );
  }

  if ( ((MouseBHistory[Histptr]&BUTTON_LEFT) && 
       !(MouseBHistory[last]&BUTTON_LEFT)) ||
       (KeyHistory[Histptr].unicode == SDLK_RETURN &&
	KeyHistory[last].unicode != SDLK_RETURN) ) {
    switch ( m_selectMode ) {
    case MENU_MAIN:
      switch ( m_selected ) {
      case 0:	// Start
	//theBall.EndGame();
	theBall.Warp( vector3d(0.0), vector3d(0.0), vector2d(0.0), -1000 );
	mode = MODE_SELECT;
	break;
      case 1:	// Practice
	//theBall.EndGame();
	theBall.Warp( vector3d(0.0), vector3d(0.0), vector2d(0.0), -1000 );
	mode = MODE_PRACTICESELECT;
	break;
      case 2:	// Training
	//theBall.EndGame();
	theBall.Warp( vector3d(0.0), vector3d(0.0), vector2d(0.0), -1000 );
	mode = MODE_TRAININGSELECT;
	break;
      case 3:	// Howto
	//theBall.EndGame();
	theBall.Warp( vector3d(0.0), vector3d(0.0), vector2d(0.0), -1000 );
	mode = MODE_HOWTO;
	break;
      case 4:	// Config...
	m_selectMode = MENU_CONFIG;
	break;
      case 5:	// Quit
	QuitGame();
	break;
      }
      break;
    case MENU_CONFIG:
      if ( m_selected < GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL ) )
	theRC->gameLevel = m_selected;
      else if ( m_selected < GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )+
		             GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE ) )
	theRC->gameMode = m_selected-GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL );
      else if ( m_selected < GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )+
		GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE )+
		GetMenuNum( MENU_CONFIG, MENU_CONFIG_PLAYER ) ) {
	theRC->myModel = m_selected-GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)
	  -GetMenuNum(MENU_CONFIG, MENU_CONFIG_MODE);
      } else if ( m_selected == GetMenuNum( MENU_CONFIG, MENU_ALL )-1 )
	m_selectMode = MENU_MAIN;
    }

    CreateMenu( m_selectMode );
    m_selected = 0;
  }

  m_count++;

  return true;
}

/**
 * Getter method of selected MenuItem. 
 * 
 * @return returns selected MenuItem object. 
 */
MenuItem *
Title::GetSelected() {
  return m_menuItem[m_selected];
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
 * Getter method of m_count. 
 */
long
Title::GetCount() {
  return m_count;
}

/**
 * Getter method of the number of menu shown currently. 
 * 
 * @param major major menu ID (main or config)
 * @param minor minor menu ID (all menu, level menu, mode menu, etc. )
 * @return returns the number of menus
 */
long
Title::GetMenuNum( long major, long minor ) {
  switch ( major ) {
  case MENU_MAIN:
    return 6;
  case MENU_CONFIG:
    switch ( minor ) {
    case MENU_ALL:
      return 11;
    case MENU_CONFIG_LEVEL:
      return 4;
    case MENU_CONFIG_MODE:
      return 3;
    case MENU_CONFIG_PLAYER:
      return 3;
    }
  }

  return -1;
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

/**
 * Create the list of MenuItem objects. 
 * 
 * @param menuMajorNum major menu ID (main or config)
 */
void
Title::CreateMenu( long menuMajorNum ) {
  static char menu[][30] = {"images/StartGame", "images/Practice", 
			    "images/Training",  "images/Howto",
			    "images/Config", "images/Quit"};
  static char configMenu[][30] = {"images/Easy", "images/Normal",
				  "images/Hard", "images/Tsuborish", 
				  "images/5point", "images/11point",
				  "images/21point",
				  "images/Transparent", "images/WireFrame", 
				  "images/ArmOnly" };

  int i, j = 0;

  if ( m_menuItem[0] ) {
    for ( i = 0 ; i < 16 ; i++ ) {
      if ( m_menuItem[i] ) {
	delete m_menuItem[i];
	m_menuItem[i] = NULL;
      }
    }
  }

  for ( i = 0 ; i < GetMenuNum( menuMajorNum ) ; i++ )
    m_menuItem[i] = new MenuItem();

  switch ( menuMajorNum ) {
  case MENU_MAIN:
    for ( i = 0 ; i < GetMenuNum( MENU_MAIN ) ; i++ )
      m_menuItem[i]->Init( 200, 500-i*90, 400, 70, &menu[i][0], this );
    break;
  case MENU_CONFIG:
    for ( i = 0 ; i < GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL ) ; i++ )
      m_menuItem[i]->Init( 100, 450-i*60, 200, 35, &configMenu[i][0], this );
    for ( i = 0 ; i < GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE ) ; i++ ) {
      j = i+GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL );
      m_menuItem[j]->Init( 500, 450-i*60, 200, 35, &configMenu[j][0], this );
    }
    for ( i = 0 ; i < GetMenuNum( MENU_CONFIG, MENU_CONFIG_PLAYER ) ; i++ ) {
      j = i+GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL ) +
	GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE );
      m_menuItem[j]->Init( 500, 190-i*60, 200, 35, &configMenu[j][0], this );
    }
    m_menuItem[j+1]->Init( 300, 20, 400, 70, &menu[5][0], this );

    m_menuItem[theRC->gameLevel]->SetSelected( true );

    m_menuItem[GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)+theRC->gameMode]
      ->SetSelected( true );

    m_menuItem[GetMenuNum(MENU_CONFIG, MENU_CONFIG_LEVEL)+
	      GetMenuNum(MENU_CONFIG, MENU_CONFIG_MODE)+theRC->myModel]
      ->SetSelected( true );

    break;
  }
}

/**
 * Make specified menu item selected. 
 * 
 * @param selected ID of selected menu item
 * @return returns selected menu ID
 */
long
Title::SetSelected( long selected ) {
  if ( selected < 0 || selected >= GetMenuNum( m_selectMode ) )
    return m_selected;

  switch ( m_selectMode ) {
  case MENU_MAIN:
    m_menuItem[m_selected]->SetSelected( false );
    m_menuItem[selected]->SetSelected( true );
    break;
  case MENU_CONFIG:
    /*
    if ( selected == 3 )
      return m_selected;
    */
    break;
  }

  m_selected = selected;
  return m_selected;
}

/**
 * Check whether the mouse pointer is on some menu item or not. 
 * 
 * @param x x location of mouse pointer
 * @param y y location of mouse pointer
 * @return returns menu ID of which mouse pointer is hit. If mouse pointer is on no menu item, returns -1. 
 */
long 
Title::HitTest( long x, long y ) {
  if ( theRC->gmode != GMODE_2D )
    y = BaseView::GetWinHeight()-y;
  for ( int i = 0 ; i < GetMenuNum( m_selectMode ) ; i++ ) {
    if ( x > m_menuItem[i]->GetX() &&
	 x < m_menuItem[i]->GetX()+m_menuItem[i]->GetWidth() &&
	 y > m_menuItem[i]->GetY() &&
	 y < m_menuItem[i]->GetY()+m_menuItem[i]->GetHeight() )
      return i;
  }

  return -1;
}
