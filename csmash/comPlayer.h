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

#ifndef _comPlayer_
#define _comPlayer_

#define LEVELMARGIN	(0.20/(gameLevel+1))
//#define LEVELMARGIN	(0.15-gameLevel*0.05)

class ComPlayer {
public:
  ComPlayer();
  virtual ~ComPlayer();

// 1turn後(0.01秒)の状態に移行する
//  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
//		     long *MouseYHistory, unsigned long *MouseBHistory,
//		     int Histptr );

protected:
// 思考ルーチン
  virtual bool Think() = 0;

// 思考ルーチン用変数
  int _prevBallstatus;		// 以前のボールの状態
  double _hitX;			// 打球点
  double _hitY;

// 打球点を計算する
  virtual bool Hitarea( double &hitX, double &hitY) = 0;
};

#endif // _comPlayer_
