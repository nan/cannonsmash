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
#include "PlayerView2D.h"
#include "BaseView2D.h"
#include "Player.h"

extern BaseView *theView;

extern long mode;

PlayerView2D::PlayerView2D() {
  m_player = NULL;
}

PlayerView2D::~PlayerView2D() {
}

bool
PlayerView2D::Init( Player *player ) {
  m_player = player;

  m_playerBMP = SDL_LoadBMP( "images/PenAttack.bmp" );
  SDL_SetColorKey( m_playerBMP, SDL_SRCCOLORKEY|SDL_RLEACCEL, 0 );

  return true;
}

bool
PlayerView2D::Redraw() {
  SDL_SetAlpha( m_playerBMP, 0, 0 );
  return SubRedraw();

  return true;
}

bool
PlayerView2D::RedrawAlpha() {
  SDL_SetAlpha( m_playerBMP, SDL_SRCALPHA|SDL_RLEACCEL, 100 );
  return SubRedraw();

  return true;
}

bool
PlayerView2D::SubRedraw() {
  static SDL_Rect rect = {0, 0, 0, 0};
  if ( m_player->GetY() > -3.5 ) {
    int x, y;

    RenderPoint( m_player->GetX(), m_player->GetY(), 1.7, &x, &y );

    rect.x = x-m_playerBMP->w/2;
    rect.y = y;
    rect.w = m_playerBMP->w;
    rect.h = m_playerBMP->h;

    if ( rect.x > BaseView::GetWinWidth() )
      rect.x = BaseView::GetWinWidth();
    if ( rect.x+rect.w > BaseView::GetWinWidth() )
      rect.w = BaseView::GetWinWidth()-rect.x;
    if ( rect.y > BaseView::GetWinHeight() )
      rect.y = BaseView::GetWinHeight();
    if ( rect.y+rect.h > BaseView::GetWinHeight() )
      rect.h = BaseView::GetWinHeight()-rect.y;

    SDL_BlitSurface(m_playerBMP, NULL, theView->GetSurface(), &rect);
  }

  return true;
}

bool
PlayerView2D::GetDamageRect() {
  static SDL_Rect rect = {0, 0, 0, 0};
  if ( m_player->GetY() > -3.5 ) {
    int x, y;
    SDL_Rect _rect;

    GetDrawRect( &_rect );

    if ( rect.x != _rect.x || rect.y != _rect.y || rect.w != _rect.w ||
	 rect.h != _rect.h ) {
      ((BaseView2D *)theView)->AddUpdateRect( &rect );

      rect = _rect;

      ((BaseView2D *)theView)->AddUpdateRect( &rect );
    }
  }

  return true;
}

bool
PlayerView2D::GetDrawRect( SDL_Rect *drawRect ) {
  drawRect->x = drawRect->y = drawRect->w = drawRect->h = 0;

  if ( m_player->GetY() > -3.5 ) {
    int x, y;

    RenderPoint( m_player->GetX(), m_player->GetY(), 1.7, &x, &y );

    drawRect->x = x-m_playerBMP->w/2;
    drawRect->y = y;
    drawRect->w = m_playerBMP->w;
    drawRect->h = m_playerBMP->h;

    if ( drawRect->x > BaseView::GetWinWidth() )
      drawRect->x = BaseView::GetWinWidth();
    if ( drawRect->x+drawRect->w > BaseView::GetWinWidth() )
      drawRect->w = BaseView::GetWinWidth()-drawRect->x;
    if ( drawRect->y > BaseView::GetWinHeight() )
      drawRect->y = BaseView::GetWinHeight();
    if ( drawRect->y+drawRect->h > BaseView::GetWinHeight() )
      drawRect->h = BaseView::GetWinHeight()-drawRect->y;
  }
}
