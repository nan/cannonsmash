/* $Id$ */

// Copyright (C) 2000  神南 吉宏(Kanna Yoshihiro)
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

extern BaseView theView;
extern long mode;

extern long winWidth;
extern long winHeight;
extern Sound theSound;

extern long gameLevel;
extern long gameMode;

extern Sound theSound;

extern void Keyboard( unsigned char key, int x, int y );
extern void KeyUp( unsigned char key, int x, int y );

extern Ball theBall;

Title::Title() {
  m_View = NULL;
  m_selected = 0;
  m_selectMode = 0;
  m_count = 0;
}

Title::~Title() {
  if ( m_View ){
    theView.RemoveView( m_View );
    delete m_View;
  }
}

bool
Title::Init() {
  m_View = new TitleView();
  m_View->Init( this );

  theView.AddView( m_View );

  return true;
}

bool
Title::Move( unsigned long *KeyHistory, long *MouseXHistory,
		    long *MouseYHistory, unsigned long *MouseBHistory,
		    int Histptr ) {
  switch ( m_selectMode ) {
  case MENU_MAIN:
    m_selected = MouseYHistory[Histptr]*GetMenuNum( MENU_MAIN )/winHeight;
    break;
  case MENU_CONFIG:
    if ( MouseXHistory[Histptr] < winWidth/2 ) {
      m_selected = MouseYHistory[Histptr]*
	GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )/(winHeight-100);
      if ( m_selected >= GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )-1 )
	m_selected = GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )-2;
    } else {
      m_selected = MouseYHistory[Histptr]*
	(GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE )
	 +GetMenuNum( MENU_CONFIG, MENU_CONFIG_SOUND ))/(winHeight-100)
	+GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL );
      if ( m_selected >= GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE )+
	                 GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )+
	                 GetMenuNum( MENU_CONFIG, MENU_CONFIG_SOUND ) )
	m_selected = GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE )+
	  GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )+
	  GetMenuNum( MENU_CONFIG, MENU_CONFIG_SOUND )-1;
    }

    if ( MouseYHistory[Histptr] > winHeight-100 )
      m_selected = GetMenuNum( MENU_CONFIG, MENU_ALL )-1;
  }

  if ( m_selectMode == MENU_CONFIG && KeyHistory[Histptr] == 27 ) {	// ESC
    m_selectMode = MENU_MAIN;
    return true;
  }

  if ( m_selected < 0 )
    m_selected = 0;
  else {
    if ( m_selectMode == MENU_MAIN && m_selected >= GetMenuNum( MENU_MAIN ) )
      m_selected = GetMenuNum( MENU_MAIN )-1;
  }

  long last = Histptr-1;
  if ( last < 0 )
    last = MAX_HISTORY-1;
  if ( (MouseBHistory[Histptr]&BUTTON_LEFT) && 
       !(MouseBHistory[last]&BUTTON_LEFT) ) {
    switch ( m_selectMode ) {
    case MENU_MAIN:
      switch ( m_selected ) {
      case 0:	// Start
	theBall.EndGame();
	mode = MODE_SELECT;
	break;
      case 1:	// Training
	theBall.EndGame();
	mode = MODE_TRAINING;
	break;
      case 2:	// Howto
	theBall.EndGame();
	mode = MODE_HOWTO;
	break;
      case 3:	// Config...
	m_selectMode = MENU_CONFIG;
	break;
      case 4:	// Quit
	Keyboard( 'Q', 0, 0 );
	KeyUp( 'Q', 0, 0 );
	break;
      }
      break;
    case MENU_CONFIG:
      if ( m_selected < GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL ) )
	gameLevel = m_selected;
      else if ( m_selected < GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )+
		             GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE ) )
	gameMode = m_selected-GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL );
      else if ( m_selected < GetMenuNum( MENU_CONFIG, MENU_CONFIG_LEVEL )+
		             GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE )+
		             GetMenuNum( MENU_CONFIG, MENU_CONFIG_SOUND ) )
	theSound.SetSoundMode( m_selected-GetMenuNum( MENU_CONFIG,
						      MENU_CONFIG_LEVEL )
			       -GetMenuNum( MENU_CONFIG, MENU_CONFIG_MODE ) );
      else if ( m_selected == GetMenuNum( MENU_CONFIG, MENU_ALL )-1 )
	m_selectMode = MENU_MAIN;
    }
  }

  m_count++;

  return true;
}

long
Title::GetSelected() {
  return m_selected;
}

long
Title::GetSelectMode() {
  return m_selectMode;
}

long
Title::GetCount() {
  return m_count;
}

long
Title::GetMenuNum( long major, long minor ) {
  switch ( major ) {
  case MENU_MAIN:
    return 5;
  case MENU_CONFIG:
    switch ( minor ) {
    case MENU_ALL:
      return 10;
    case MENU_CONFIG_LEVEL:
      return 4;
    case MENU_CONFIG_MODE:
      return 3;
    case MENU_CONFIG_SOUND:
#ifdef HAVE_LIBESD
      return 2;
#else
      return 0;
#endif
    }
  }

  return -1;
}
