/* $Id$ */

// Copyright (C) 2001  �_�� �g�G(Kanna Yoshihiro)
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
#include "BallView2D.h"
#include "Ball.h"
#include "Player.h"
#include "Control.h"
#include "LoadImage.h"
#include "PlayGame.h"
#include "BaseView.h"

extern Ball   theBall;
extern Player* thePlayer;
extern Player *comPlayer;
extern long mode;
extern BaseView* theView;

extern Control* theControl;

extern bool isLighting;
extern bool isTexture;

extern bool RenderRect( double x1, double y1, double z1, 
			double x2, double y2, double z2, 
			SDL_Rect *rect );

BallView2D::BallView2D() {
}

BallView2D::~BallView2D() {
}

bool
BallView2D::Init() {
  static char num[][20] = {"images/zero.ppm", "images/one.ppm",
			   "images/two.ppm", "images/three.ppm",
			   "images/four.ppm", "images/five.ppm",
			   "images/six.ppm", "images/seven.ppm",
			   "images/eight.ppm", "images/nine.ppm"};

  return true;
}
 
bool
BallView2D::Redraw() {
  double rad;
  Ball* tmpBall;
  const static GLfloat mat_yel[] = { 1.0F, 0.8F, 0.0F, 0.0F };

  // Draw the Ball itself
  if ( theBall.GetY() > -3.5 ) {
    SDL_Rect rect;
    RenderRect( theBall.GetX()-BALL_R/2, theBall.GetY()-BALL_R/2, theBall.GetZ()-BALL_R/2, 
		theBall.GetX()+BALL_R/2, theBall.GetY()+BALL_R/2, theBall.GetZ()+BALL_R/2, 
		&rect );

    SDL_FillRect( theView->GetSurface(), &rect, 0 );
  }

  // Draw the ball shadow

  // Draw the ball location in the future

  return true;
}

bool
BallView2D::RedrawAlpha() {
  // Score

  return true;
}
