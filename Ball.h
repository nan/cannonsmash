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

#ifndef _Ball_
#define _Ball_

// Ball Classの定義
class Player;
class BallView;

class Ball {
public:
  Ball();
  Ball( double _x, double _y, double _z, double _vx, double _vy, double _vz,
	double _spin, long _status );
  virtual ~Ball();

  virtual bool Init();

  double GetX();
  double GetY();
  double GetZ();
  double GetVX();
  double GetVY();
  double GetVZ();
  double GetSpin();

  long GetStatus();

  long GetService();
  long GetScore( Player *p );	// スコア取得. 

  bool Move();	// 1turn後(0.01秒)の状態に移行する

  bool Hit( double vx, double vy, double vz, double spin, Player *player );
  						// 打球
  bool Toss( Player *player, long power );	// トス

  void Warp( double x, double y, double z, double vx, double vy, double vz, 
	     double spin, long status );
  void Warp( char *buf );

  // 落下目標地点からvy, vzを求める
  bool TargetToV( double targetX, double targetY, double height, double spin, 
		  double &vx, double &vy, double &vz, double vMin = 0.1,
		  double vMax = 30.0 );
  // サーブ用
  bool TargetToVS( double targetX, double targetY, double height, double spin, 
		   double &vx, double &vy, double &vz );

  bool IsGameEnd();	// ゲーム終了チェック
  void EndGame();

  bool Send( int sd );

  long m_count;		// Training用カウンタ
protected:
  double m_x;		// ballの位置
  double m_y;
  double m_z;
  double m_vx;		// ballの速度
  double m_vy;
  double m_vz;

  double m_spin;	// 回転. 正 --- トップスピン   負 --- バックスピン

  long m_status;	// 0 --- 自分が打ってから１バウンドまで
  			// 1 --- コンピュータが打てる状態
  			// 2 --- コンピュータが打ってから１バウンドまで
  			// 3 --- 自分が打てる状態
  			// 4 --- 自分のサーブ後１バウンドまで
  			// 5 --- コンピュータのサーブ後１バウンドまで
  			// 6 --- 自分がトスした後
  			// 7 --- コンピュータがトスした後
  			// 8 --- playerがサーブする前
  			//-1 --- ボールデッド

  BallView* m_View;

  // スコア変更
  void ChangeScore();

  long m_Score1;	// 手前プレイヤーの得点
  long m_Score2;	// 奥プレイヤーの得点
};

#endif // _Ball_
