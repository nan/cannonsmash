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

#ifndef _Event_
#define _Event_

// Event Handle Classの定義

struct PlayerData {
  long playerType;
  long side;
  double x;
  double y;
  double z;
  double vx;
  double vy;
  double vz;
  long status;
  long swing;
  long swingType;
  long afterSwing;
  long swingError;
  double targetX;
  double targetY;
  double eyeX;
  double eyeY;
  double eyeZ;
  long pow;
  double spin;
  double stamina;
};

struct Backtrack {
  long sec;		// 秒
  char count;		// 1/100秒単位
  Ball theBall;
  PlayerData thePlayer;
  PlayerData comPlayer;
};

class ExternalData;

class Event {
public:
  Event();
  ~Event();

  bool Init();

  static void IdleFunc();
  static void KeyboardFunc( unsigned char key, int x, int y );
  static void KeyUpFunc( unsigned char key, int x, int y );
  static void MotionFunc( int x, int y );
  static void ButtonFunc( int button, int state, int x, int y );

  //bool SendSwing( int sd, Player *player );
  bool SendSwing( Player *player );
  //bool SendPlayer( int sd, Player *player );
  bool SendPlayer( Player *player );
  //bool SendBall( int sd );
  bool SendBall();

  static void ClearObject();

  bool BackTrack( long Histptr );
protected:
  bool Move();
  void Record();
  void ReadData();

  void PlayInit( long player, long com );
  void DemoInit();
  void SelectInit();
  void TrainingInit( long player, long com );
  void TrainingSelectInit();

  unsigned long m_KeyHistory[MAX_HISTORY];		// キー入力履歴
  long m_MouseXHistory[MAX_HISTORY];			// マウス履歴
  long m_MouseYHistory[MAX_HISTORY];			// マウス履歴
  unsigned long m_MouseBHistory[MAX_HISTORY];		// マウスボタン履歴
  int  m_Histptr;

  ExternalData *m_External;
  struct Backtrack m_BacktrackBuffer[MAX_HISTORY];
  bool m_backtrack;		// 現在backtrack中か否か
};

#endif	// _Event_
