/* $Id$ */

// Copyright (C) 2001  神南 吉宏(Kanna Yoshihiro)
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
#include "TitleView2D.h"
#include "Title.h"
#include "MenuItem.h"
#include "LoadImage.h"
#include "Sound.h"
#include "BaseView2D.h"

extern long gameLevel;
extern long gameMode;

extern Sound theSound;
extern BaseView *theView;

TitleView2D::TitleView2D() {
  m_View = NULL;
}

TitleView2D::~TitleView2D() {
}

bool
TitleView2D::Init( Title *title ) {
  int i, j, k;

  static char configTitle[][30] = {"images/LevelSelect.ppm",
				   "images/ModeSelect.ppm"
  };

  m_triangleBMP = SDL_LoadBMP( "images/Triangle.bmp" );
  SDL_SetColorKey( m_triangleBMP, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0 );

  m_title = title;

  return true;
}

bool
TitleView2D::Redraw() {
  return true;
}

bool
TitleView2D::RedrawAlpha() {
  View *view;
  SDL_Rect rect;

  switch ( m_title->GetSelectMode() ) {
  case MENU_MAIN:
    rect.x = m_title->GetSelected()->GetX()-80;
    rect.y = m_title->GetSelected()->GetY();
    rect.w = m_triangleBMP->w;
    rect.h = m_triangleBMP->h;

    SDL_BlitSurface(m_triangleBMP, NULL, theView->GetSurface(), &rect);
    break;
  case MENU_CONFIG:
    // Title
    if ( m_title->GetSelected()->GetHeight() == 70 ) {
      rect.x = m_title->GetSelected()->GetX()-80;
      rect.y = m_title->GetSelected()->GetY();
      rect.w = m_triangleBMP->w;
      rect.h = m_triangleBMP->h;

      SDL_BlitSurface(m_triangleBMP, NULL, theView->GetSurface(), &rect);
    } else {
      rect.x = m_title->GetSelected()->GetX()-40;
      rect.y = m_title->GetSelected()->GetY();
      rect.w = m_triangleBMP->w;
      rect.h = m_triangleBMP->h;

      SDL_BlitSurface(m_triangleBMP, NULL, theView->GetSurface(), &rect);
    }
    break;
  }

  /*
  view = m_View;
  while ( view ){
    view->RedrawAlpha();
    view = view->m_next;
  }
  */

  return true;
}

bool
TitleView2D::GetDamageRect() {
  static SDL_Rect rect = {0, 0, 0, 0};
  switch ( m_title->GetSelectMode() ) {
  case MENU_MAIN:
    if ( rect.x != m_title->GetSelected()->GetX()-80 ||
	 rect.y != m_title->GetSelected()->GetY() ||
	 rect.w != m_triangleBMP->w ||
	 rect.h != m_triangleBMP->h ) {
      ((BaseView2D *)theView)->AddUpdateRect( &rect );
      rect.x = m_title->GetSelected()->GetX()-80;
      rect.y = m_title->GetSelected()->GetY();
      rect.w = m_triangleBMP->w;
      rect.h = m_triangleBMP->h;
      ((BaseView2D *)theView)->AddUpdateRect( &rect );
    }
    break;
  case MENU_CONFIG:
    // Title
    if ( m_title->GetSelected()->GetHeight() == 70 ) {
      if ( rect.x != m_title->GetSelected()->GetX()-80 ||
	   rect.y != m_title->GetSelected()->GetY() ||
	   rect.w != m_triangleBMP->w ||
	   rect.h != m_triangleBMP->h ) {
	((BaseView2D *)theView)->AddUpdateRect( &rect );
	rect.x = m_title->GetSelected()->GetX()-80;
	rect.y = m_title->GetSelected()->GetY();
	rect.w = m_triangleBMP->w;
	rect.h = m_triangleBMP->h;
	((BaseView2D *)theView)->AddUpdateRect( &rect );
      }
    } else {
      if ( rect.x != m_title->GetSelected()->GetX()-40 ||
	   rect.y != m_title->GetSelected()->GetY() ||
	   rect.w != m_triangleBMP->w ||
	   rect.h != m_triangleBMP->h ) {
	((BaseView2D *)theView)->AddUpdateRect( &rect );
	rect.x = m_title->GetSelected()->GetX()-40;
	rect.y = m_title->GetSelected()->GetY();
	rect.w = m_triangleBMP->w;
	rect.h = m_triangleBMP->h;
	((BaseView2D *)theView)->AddUpdateRect( &rect );
      }
    }
    break;
  }

  return true;
}
