/* $Id$ */

// Copyright (C) 2001  _“ì ‹gG(Kanna Yoshihiro)
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
  if ( m_player->GetY() > -3.5 ) {
    SDL_Rect rect;
    int x, y;

    RenderPoint( m_player->GetX(), m_player->GetY(), 1.7, &x, &y );

    rect.x = x;
    rect.y = y;
    rect.w = m_playerBMP->w;
    rect.h = m_playerBMP->h;

    SDL_BlitSurface(m_playerBMP, NULL, theView->GetSurface(), &rect);
  }

  return true;
}
