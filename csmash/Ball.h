/* $Id$ */

// Copyright (C) 2000, 2003, 2004  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _Ball_
#define _Ball_

#include "BallView.h"
#include "BallView2D.h"
#include "Player.h"

#include <vector>
#include "matrix"
typedef Vector<3, double> vector3d;
typedef Vector<2, double> vector2d;

class Ball {
  friend class Opening;
public:
  Ball();
  Ball( const vector3d _x, const vector3d _vx, const vector2d _spin, long _status );
  Ball(Ball *ball);

  virtual ~Ball();

  virtual bool Init();

  vector3d GetX() { return m_x; }
  vector3d GetV() { return m_v; }
  vector2d GetSpin() { return m_spin; }

  long GetStatus() { return m_status; }

  bool Move();	// move to 1turn(0.01 sec) later

  bool Hit( const vector3d v, const vector2d spin, Player *player );	// hit ball
  bool Toss( Player *player, long power );				// toss

  void Warp( const vector3d x, const vector3d v, const vector2d spin, long status );
  void Warp( char *buf );

  // Calc vx, vy, vz from bound location
  bool TargetToV( vector2d target, double level, const vector2d spin, 
		  vector3d &v, double vMin = 0.1, double vMax = 30.0 );
  // For serve
  bool TargetToVS( vector2d target, double level, 
		   const vector2d spin, vector3d &v );

  char * Send( char *buf );

  BallView *GetView() { return m_View; };
protected:
  vector3d m_x;		// ball location
  vector3d m_v;		// ball velocity
  vector2d m_spin;	// spin[0] plus  --- ball moves to right
			//         minus --- ball moves to left
			// spin[1] plus  --- top spin
			//         minus --- back spin

  long m_status;	// 0 --- From the time side=1 hit to bound
			// 1 --- During side=-1 can hit
			// 2 --- From the time side=-1 hit to bound
			// 3 --- During side=1 can hit
			// 4 --- From the time side=1 serve to bound
			// 5 --- From the time side=-1 serve to bound
			// 6 --- From the time side=1 toss to hit
			// 7 --- From the time side=-1 toss to hit
			// 8 --- Until player serve
			//-1 --- Ball dead

  BallView* m_View;

  long m_lastSendCount;
private:
  void BallDead();
};

#endif // _Ball_
