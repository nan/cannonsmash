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

#ifndef _Player_
#define _Player_

// m_playerType
#define PLAYER_PROTO		0	// プロトタイプPlayer
#define PLAYER_PENATTACK	1	// 裏ペン速攻
#define PLAYER_SHAKECUT		2	// カット
#define PLAYER_PENDRIVE		3	// ペンドライブ


// m_swingType
#define SWING_NORMAL	0	// 基礎打ち
#define SWING_POKE	1	// ツッツキ(本当はpushらしいが...)
#define SWING_SMASH	2	// 強打
#define SWING_DRIVE	3	// ドライブ
#define SWING_CUT	4	// カット
#define SWING_BLOCK	5	// ブロック, ストップ

// m_swingError
#define SWING_PERFECT	0
#define SWING_GREAT	1
#define SWING_GOOD	2
#define SWING_BOO	3
#define SWING_MISS	4

// Player Classの定義

class PlayerView;
class HitMark;

class Player {
  friend class Howto;
public:
  Player();
  Player( long side );
  Player( long playerType, long side, double x, double y, double z, 
	  double vx, double vy, double vz,long status, long swing, 
	  long swingType, long afterSwing, long swingError, 
	  double targetX, double targetY, double eyeX, double eyeY,
	  double eyeZ, long pow, double spin, double stamina );

  virtual ~Player();

  virtual bool Init();

  virtual bool Reset( struct PlayerData *p );

  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual bool AddStatus( long diff );

  virtual long   GetSide();
  virtual long   GetPlayerType();

  virtual double GetX();
  virtual double GetY();
  virtual double GetZ();
  virtual double GetVX();
  virtual double GetVY();
  virtual double GetVZ();
  virtual long   GetPower();
  virtual double GetSpin();
  virtual double GetTargetX();
  virtual double GetTargetY();
  virtual double GetEyeX();
  virtual double GetEyeY();
  virtual double GetEyeZ();
  virtual double GetStamina();
  virtual long   GetStatus();
  virtual long   GetSwing();
  virtual long   GetSwingType();
  virtual long   GetSwingError();
  virtual long   GetAfterSwing();

  virtual bool   GetShoulder( double &x, double &y, double &deg );
  virtual bool   GetElbow( double &degx, double& degy );
  virtual bool   GetHand( double &degx, double &degy, double &degz );

// Get{Shoulder|Elbow|Hand}から呼び出される. 
// ボールの位置からフォアで打つかバックかを選択. 
// true  -> フォア
// false -> バック
  virtual bool ForeOrBack();

  virtual bool Warp( double x, double y, double z,
		     double vx, double vy, double vz );
  virtual bool ExternalSwing( long pow, double spin, long swingType, long swing );

  virtual bool Warp( char *buf );
  virtual bool ExternalSwing( char *buf );

  virtual bool SendSwing( int sd );
  virtual bool SendLocation( int sd );
  virtual bool SendAll( int sd );

protected:
  long m_playerType;	// Playerの種類

  long m_side;		// 1  --- 手前側( y < 0 )
			// -1 --- 奥( y > 0 )

  double m_x;		// playerの位置(視点の位置)
  double m_y;
  double m_z;
  double m_vx;		// playerの速度
  double m_vy;
  double m_vz;

  long m_status;	// 体勢ゲージ値
  long m_swing;		// スイングの状態
  long m_swingType;	// スイングの種類
  long m_afterSwing;	// スイング後の硬直時間
  long m_swingError;	// ボールを打ったときの誤差. 
                        // 0 --- Perfect
                        // 1 --- Great
                        // 2 --- Good
                        // 3 --- Boo
                        // 4 --- Miss
  double m_targetX;	// 打球の目標落下点
  double m_targetY;	// 打球の目標落下点

  double m_eyeX;	// 視点
  double m_eyeY;
  double m_eyeZ;

  long m_pow;		// 弱, 中, 強
  double m_spin;	// トップ/バックスピン

  double m_stamina;	// 体力

  PlayerView* m_View;

  HitMark *m_hitMark;

// Moveから呼び出される
  virtual bool KeyCheck( unsigned long *KeyHistory, long *MouseXHistory,
			 long *MouseYHistory, unsigned long *MouseBHistory,
			 int Histptr );		// キー入力処理
  virtual bool Swing( long power, double spin ) = 0;
  virtual bool StartSwing( long power, double spin ) = 0;

  virtual bool HitBall() = 0;

  virtual bool SwingError();
};

#endif // _Player_
