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

extern long winHeight;
extern Sound theSound;

extern void Keyboard( unsigned char key, int x, int y );
extern void KeyUp( unsigned char key, int x, int y );

extern Ball theBall;

Title::Title() {
  m_View = NULL;
  m_selected = 0;
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
  m_selected = MouseYHistory[Histptr]*GetMenuNum()/winHeight;

  if ( m_selected < 0 )
    m_selected = 0;
  else if ( m_selected >= GetMenuNum() )
    m_selected = GetMenuNum()-1;


  if ( MouseBHistory[Histptr]&BUTTON_LEFT ){
    switch ( m_selected ) {
    case 0:	// Start
      theBall.EndGame();
      mode = MODE_SELECT;
      break;
    case 1:	// Quit
      theBall.EndGame();
      mode = MODE_HOWTO;
      break;
    case 2:	// Quit
      Keyboard( 'Q', 0, 0 );
      KeyUp( 'Q', 0, 0 );
      break;
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
Title::GetCount() {
  return m_count;
}
