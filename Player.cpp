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

extern Ball   theBall;
extern BaseView theView;

extern long winWidth;
extern long winHeight;

Player::Player() {
  m_side = 1;
  m_playerType = PLAYER_PROTO;

  m_x = 0;
  m_y = -TABLELENGTH/2-0.5;
  m_z = 1.4;
  m_vx = m_vy = m_vz = 0.0;

  m_status = 200;
  m_swing = 0;
  m_afterSwing = 0;
  m_swingType = SWING_NORMAL;
  m_swingError = SWING_PERFECT;
  m_targetX = 0.0;
  m_targetY = TABLELENGTH/16*5;

  m_eyeX = 0.0;
  m_eyeY = -0.8;
  m_eyeZ = 0.2;

  m_pow = 0;
  m_spin = 0.0;

  m_stamina = 80.0;

  m_View = NULL;
}

Player::~Player() {
  if ( m_View ){
    theView.RemoveView( m_View );
    delete m_View;
  }
}

bool
Player::Init() {
  m_View = new PlayerView();
  m_View->Init( this );

  theView.AddView( m_View );

  HitMark::Init();

  return true;
}

bool
Player::Move( unsigned long *KeyHistory, long *MouseXHistory,
	      long *MouseYHistory, unsigned long *MouseBHistory,
	      int Histptr ) {
  KeyCheck( KeyHistory, MouseXHistory, MouseYHistory, MouseBHistory,Histptr );

  if ( m_swing == 0 ) {
    Ball *tmpBall;

    tmpBall = new Ball( theBall.GetX(), theBall.GetY(), theBall.GetZ(),
			theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(),
			theBall.GetSpin(), theBall.GetStatus() );

    for ( int i = 0 ; i < 20 ; i++ ) {
      tmpBall->Move();

      if ( ((tmpBall->GetStatus() == 3 && m_side == 1) ||
	    (tmpBall->GetStatus() == 1 && m_side == -1)) &&
	   (m_y-tmpBall->GetY())*m_side < 0.3 &&
	   (m_y-tmpBall->GetY())*m_side > -0.05 ){
	StartSwing( 3, m_spin );
	break;
      }
    }
  }

// playerの移動
  if ( m_x+m_vx*TICK < -AREAXSIZE/2 ){
    m_x = -AREAXSIZE/2;
    m_vx = 0.0;
  }
  else if ( m_x+m_vx*TICK > AREAXSIZE/2 ){
    m_x = AREAXSIZE/2;
    m_vx = 0.0;
  }
  else if ( m_x <= -TABLEWIDTH/2 && m_x+m_vx*TICK >= -TABLEWIDTH/2 &&
	    m_y > -TABLELENGTH/2 && m_y < TABLELENGTH/2 ){
    m_x = -TABLEWIDTH/2;
    m_vx = 0.0;
  }
  else if ( m_x >= TABLEWIDTH/2 && m_x+m_vx*TICK <= TABLEWIDTH/2 &&
	    m_y > -TABLELENGTH/2 && m_y < TABLELENGTH/2 ){
    m_x = TABLEWIDTH/2;
    m_vx = 0.0;
  }
  else
    m_x += m_vx*TICK;

  if ( m_y+m_vy*TICK < -AREAYSIZE/2 ){
    m_y = -AREAYSIZE/2;
    m_vy = 0.0;
  }
  else if ( m_y+m_vy*TICK > AREAYSIZE/2 ){
    m_y = AREAYSIZE/2;
    m_vy = 0.0;
  }
  else if ( m_y <= -TABLELENGTH/2+0.5 && m_y+m_vy*TICK >= -TABLELENGTH/2+0.5 &&
	    m_x > -TABLEWIDTH/2 && m_x < TABLEWIDTH/2 ){
    m_y = -TABLELENGTH/2+0.5;
    m_vy = 0.0;
  }
  else if ( m_y >= TABLELENGTH/2-0.5 && m_y+m_vy*TICK <= TABLELENGTH/2-0.5 &&
	    m_x > -TABLEWIDTH/2 && m_x < TABLEWIDTH/2 ){
    m_y = TABLELENGTH/2-0.5;
    m_vy = 0.0;
  }
  else
    m_y += m_vy*TICK;

// swing
  if ( m_swing > 0 ){
    if ( m_swing == 30 && m_afterSwing > 0 )
      m_afterSwing--;
    else {
      if ( theBall.GetStatus() == 6 || theBall.GetStatus() == 7 ) {
	if ( theBall.GetVZ() < 0 )
	  m_swing++;
      } else {
	if ( m_swing == 10 ) {
	  if ( theBall.GetStatus() == -1 )
	    m_swing = 0;
	} else
	  m_swing++;
      }
    }
  }

// status 計算
  else if ( hypot( m_vx, m_vy ) > 2.0 )
    AddStatus( -1 );

  if ( m_swing > 0 )
    AddStatus( -1 );

  if ( theBall.GetStatus() == 8 || theBall.GetStatus() == -1 )
    AddStatus( 200 );

// backswing と inpact
  if ( m_swing == 20 ){
    HitBall();

    if ( m_side > 0 ) {
      HitMark *hit;

      hit = new HitMark();
      hit->Hit( theBall.GetX(), m_y, theBall.GetZ(), 
		theBall.GetVX(), theBall.GetVY(), theBall.GetVZ(), 
		GetSwingError() );

      theView.AddView( hit );
    }
    m_spin = 0.0;
  }
  else if ( m_swing == 50 ){
    m_swing = 0;
    m_swingType = SWING_NORMAL;
  }

  return true;
}

bool
Player::KeyCheck( unsigned long *KeyHistory, long *MouseXHistory,
		  long *MouseYHistory, unsigned long *MouseBHistory,
		  int Histptr ) {
  long mouse, lastmouse;

// COM側の場合
  if ( !KeyHistory || !MouseXHistory || !MouseYHistory || !MouseBHistory )
    return true;

// 入力の反映
  switch ( KeyHistory[Histptr] ){
  case '1':
  case 'q':
  case 'a':
  case 'z':
    m_targetX = -TABLEWIDTH/2*0.9;
    break;
  case '2':
  case 'w':
  case 's':
  case 'x':
    m_targetX = -TABLEWIDTH/2*0.7;
    break;
  case '3':
  case 'e':
  case 'd':
  case 'c':
    m_targetX = -TABLEWIDTH/2*0.5;
    break;
  case '4':
  case 'r':
  case 'f':
  case 'v':
    m_targetX = -TABLEWIDTH/2*0.3;
    break;
  case '5':
  case 't':
  case 'g':
  case 'b':
    m_targetX = -TABLEWIDTH/2*0.1;
    break;
  case '6':
  case 'y':
  case 'h':
  case 'n':
    m_targetX = 0;
    break;
  case '7':
  case 'u':
  case 'j':
  case 'm':
    m_targetX = TABLEWIDTH/2*0.1;
    break;
  case '8':
  case 'i':
  case 'k':
  case ',':
    m_targetX = TABLEWIDTH/2*0.3;
    break;
  case '9':
  case 'o':
  case 'l':
  case '.':
    m_targetX = TABLEWIDTH/2*0.5;
    break;
  case '0':
  case 'p':
  case ';':
  case '/':
    m_targetX = TABLEWIDTH/2*0.7;
    break;
  case '-':
  case '@':
  case ':':
  case '\\':
  case '^':
  case '[':
  case ']':
    m_targetX = TABLEWIDTH/2*0.9;
    break;
  }

  switch ( KeyHistory[Histptr] ){
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
  case '0':
  case '-':
  case '^':
    m_targetY = TABLELENGTH/16*7;
    break;
  case 'q':
  case 'w':
  case 'e':
  case 'r':
  case 't':
  case 'y':
  case 'u':
  case 'i':
  case 'o':
  case 'p':
  case '@':
  case '[':
    m_targetY = TABLELENGTH/16*5;
    break;
  case 'a':
  case 's':
  case 'd':
  case 'f':
  case 'g':
  case 'h':
  case 'j':
  case 'k':
  case 'l':
  case ';':
  case ':':
  case ']':
    m_targetY = TABLELENGTH/16*3;
    break;
  case 'z':
  case 'x':
  case 'c':
  case 'v':
  case 'b':
  case 'n':
  case 'm':
  case ',':
  case '.':
  case '/':
  case '\\':
    m_targetY = TABLELENGTH/16*1;
    break;
  }

  if ( (Histptr == 0 && KeyHistory[Histptr] != KeyHistory[MAX_HISTORY-1]) ||
       (Histptr != 0 && KeyHistory[Histptr] != KeyHistory[Histptr-1]) ) {
    switch ( KeyHistory[Histptr] ){
    case 'H':
      m_eyeX -= 0.05;
      break;
    case 'J':
      m_eyeZ -= 0.05;
      break;
    case 'K':
      m_eyeZ += 0.05;
      break;
    case 'L':
      m_eyeX += 0.05;
      break;
    case '<':
      m_eyeY -= 0.05;
      break;
    case '>':
      m_eyeY += 0.05;
      break;
    }
  }

  m_vx = (MouseXHistory[Histptr] - winWidth/2) / (winWidth/40);
  m_vy = -(MouseYHistory[Histptr] - winHeight/2) / (winHeight/40);
  m_vx /= 4;
  m_vy /= 4;	// 0.25刻み

  mouse = MouseBHistory[Histptr];
  if ( Histptr-1 < 0 )
    lastmouse = MouseBHistory[MAX_HISTORY-1];
  else
    lastmouse = MouseBHistory[Histptr-1];

  if ( (mouse & BUTTON_RIGHT) && !(lastmouse & BUTTON_RIGHT) ){
    if ( theBall.GetStatus() == 8 && theBall.GetService() == GetSide() ) {
      theBall.Toss( this, m_pow );
      StartSwing( 3, m_spin );
    } else {
      AddStatus( (m_swing-10)*10 );
      Swing( 3, m_spin );
    }
  }
  else if ( (mouse & BUTTON_MIDDLE) && !(lastmouse & BUTTON_MIDDLE) ){
    if ( theBall.GetStatus() == 8 && theBall.GetService() == GetSide() ) {
      theBall.Toss( this, m_pow );
      StartSwing( 2, m_spin );
    } else {
      AddStatus( (m_swing-10)*10 );
      Swing( 2, m_spin );
    }
  }
  else if ( (mouse & BUTTON_LEFT) && !(lastmouse & BUTTON_LEFT) ){
    if ( theBall.GetStatus() == 8 && theBall.GetService() == GetSide() ) {
      theBall.Toss( this, m_pow );
      StartSwing( 1, m_spin );
    } else {
      AddStatus( (m_swing-10)*10 );
      Swing( 1, m_spin );
    }
  }

  return true;
}

bool
Player::AddStatus( long diff ) {
  m_status += diff;
  if ( m_status > 200 )
    m_status = 200;

  if ( m_status < 1 ){
    m_stamina += (m_status-1) / 10.0;
    m_status = 1;
  }

  return true;
}

long
Player::GetSide() {
  return m_side;
}

long
Player::GetPlayerType() {
  return m_playerType;
}

double
Player::GetX() {
  return m_x;
}

double
Player::GetY() {
  return m_y;
}

double
Player::GetZ() {
  return m_z;
}

double
Player::GetVX() {
  return m_vx;
}

double
Player::GetVY() {
  return m_vy;
}

double
Player::GetVZ() {
  return m_vz;
}

long
Player::GetPower() {
  return m_pow;
}

double
Player::GetSpin() {
  return m_spin;
}

double
Player::GetTargetX() {
  return m_targetX;
}

double
Player::GetTargetY() {
  return m_targetY;
}

double
Player::GetEyeX() {
  return m_eyeX;
}

double
Player::GetEyeY() {
  return m_eyeY;
}

double
Player::GetEyeZ() {
  return m_eyeZ;
}

double
Player::GetStamina() {
  return m_stamina;
}

long
Player::GetStatus() {
  return m_status;
}

long
Player::GetSwing() {
  return m_swing;
}

long
Player::GetSwingType() {
  return m_swingType;
}

long
Player::GetSwingError() {
  return m_swingError;
}

// 利き腕の肩の位置を計算する. 
// x   --- x座標値
// y   --- y座標値
// deg --- z軸周りの体の回転角度
bool
Player::GetShoulder( double &x, double &y, double &deg ) {
  double px, py, bx, by, bvx, bvy;
  double t, btx;

  // 奥側ならば180度回転
  if ( m_side < 0 ){
    px = -m_x;
    py = -m_y;
    bx = -theBall.GetX();
    by = -theBall.GetY();
    bvx = -theBall.GetVX();
    bvy = -theBall.GetVY();
  }
  else{
    px = m_x;
    py = m_y;
    bx = theBall.GetX();
    by = theBall.GetY();
    bvx = theBall.GetVX();
    bvy = theBall.GetVY();
  }

  // ボールの予想到達地点
  if ( bvy == 0.0 || m_swing == 0 ){
    btx = bx;
    t = -1;
  }
  else{
    t = (py - by) / bvy;
    btx = bx + bvx*t;
  }

  // フォアかバックか
  if ( btx - px > 0 ){	// フォア
    switch ( m_swingType ){
    case SWING_NORMAL:
    case SWING_DRIVE:
      if ( m_swing < 10 )
	deg = -(double)m_swing/10*15-30;
      else if ( m_swing < 30 )
	deg = -45+(double)(m_swing-10)/20*45;
      else
	deg = -(double)(m_swing-30)/20*30;
      break;
    case SWING_POKE:
      if ( m_swing < 10 )
	deg = -(double)m_swing/10*10-30;
      else if ( m_swing < 30 )
	deg = -40+(double)(m_swing-10)/20*20;
      else
	deg = -20-(double)(m_swing-30)/20*10;
      break;
    case SWING_CUT:
      if ( m_swing < 10 )
	deg = -(double)m_swing/10*60-30;
      else if ( m_swing < 30 )
	deg = -60+(double)(m_swing-10)/20*60;
      else
	deg = -(double)(m_swing-30)/20*30;
      break;
    case SWING_SMASH:
      if ( m_swing < 10 )
	deg = -(double)m_swing/10*30-30;
      else if ( m_swing < 30 )
	deg = -60+(double)(m_swing-10)/20*90;
      else
	deg = 30-(double)(m_swing-30)/20*60;
      break;
    }

    if ( m_swing == 0 ){
      x = y = 0.0;
      return true;
    }

    // 理想的な打球位置から, ボールがどれくらいずれているかを計算する
    if ( btx - px < 0.6 ){
      x = btx - px - 0.3;
      y = 0.0;
    }
    else {
      x = 0.3;
      y = 0.0;
    }
  }
  else{		// バック
    switch ( m_swingType ){
    case SWING_NORMAL:
    case SWING_SMASH:
    case SWING_DRIVE:
      if ( m_swing < 10 )
	deg = (double)m_swing/10*30-30;
      else if ( m_swing < 30 )
	deg = -(double)(m_swing-10)/20*45;
      else
	deg = -45+(double)(m_swing-30)/20*15;
      break;
    case SWING_POKE:
      if ( m_swing < 30 )
	deg = (double)m_swing/30*60-30;
      else
	deg = 30-(double)(m_swing-30)/20*60;
      break;
    case SWING_CUT:
      if ( m_swing < 10 )
	deg = (double)m_swing/10*120-30;
      else if ( m_swing < 30 )
	deg = 90-(double)(m_swing-10)/20*90;
      else
	deg = -(double)(m_swing-30)/20*30;
      break;
    }

    if ( m_swing == 0 ){
      x = y = 0.0;
      return true;
    }

    // 理想的な打球位置から, ボールがどれくらいずれているかを計算する
    if ( btx - px > -0.6 ){
      x = btx - px + 0.3;
      y = 0.0;
    }
    else{
      x = -0.3;
      y = 0.0;
    }
  }

  if ( m_swing < 30 ) {
    x *= (double)m_swing / 30.0;
    y *= (double)m_swing / 30.0;
  } else {
    x *= (double)(50-m_swing) / 20.0;
    y *= (double)(50-m_swing) / 20.0;
  }

  return true;
}

// 利き腕の肘の位置を計算する. 
// degx --- x軸周りの上腕の回転角度, 腕を前後にどれくらい振るか
// degy --- y軸周りの上腕の回転角度, 肘をどれくらい上に挙げるか, default=-15
bool
Player::GetElbow( double &degx, double &degy ) {
  if ( ForeOrBack() ){
    switch ( m_swingType ){
    case SWING_NORMAL:
    case SWING_SMASH:
      if ( m_swing < 10 )
	degx = 0.0;
      else if ( m_swing < 30 )
	degx = (double)(m_swing-10)/20*15;
      else
	degx = 15-(double)(m_swing-30)/20*15;
      degy = -15.0;
      break;
    case SWING_POKE:
      degx = 0.0;
      degy = -15.0;
      break;
    case SWING_CUT:
      if ( m_swing < 10 ){
	degx = 0.0;
	degy = -15-(double)m_swing/10*75;
      } else if ( m_swing < 30 ){
	degx = 0.0;
	degy = -90+(double)(m_swing-10)/20*90;
      } else {
	degx = 0.0;
	degy = -(double)(m_swing-30)/20*15;
      }
      break;
    case SWING_DRIVE:
      if ( m_swing < 10 ){
	degx = -(double)m_swing/10*10;
	degy = -15.0;
      } else if ( m_swing < 30 ){
	degx = -10+(double)(m_swing-10)/20*160;
	degy = -15+(double)(m_swing-10)/20*45;
      } else{
	degx = 150-(double)(m_swing-30)/20*150;
	degy = 30-(double)(m_swing-30)/20*45;
      }
      break;
    }
  }
  else{
    switch ( m_swingType ){
    case SWING_NORMAL:
    case SWING_SMASH:
    case SWING_DRIVE:
      degx = 0.0;
      degy = -15.0;
      break;
    case SWING_POKE:
      if ( m_swing < 10 ){
	degx = (double)m_swing/10*15;
	degy = -15.0;
      }
      else if ( m_swing < 30 ){
	degx = 15 - (double)(m_swing-10)/20*15;
	degy = -15.0;
      } else{
	degx = 0.0;
	degy = -15.0;
      }
      break;
    case SWING_CUT:
      if ( m_swing < 10 ){
	degx = (double)m_swing/10*10;
	degy = (double)m_swing/10*45;
      } else if ( m_swing < 30 ){
	degx = 10-(double)(m_swing-10)/20*10;
	degy = 45-(double)(m_swing-10)/20*45;
      } else{
	degx = 0.0;
	degy = -(double)(m_swing-30)/20*15;
      }
      break;
    }
  }
  return true;
}

// 利き腕の手の位置を計算する. 
// degx --- x軸周りの肘の回転角度, 肘の曲がり具合
// degy --- y軸周りの肘の回転角度, ラケットの角度
// degz --- z軸周りの肘の回転角度, 腕の開き具合
bool
Player::GetHand( double &degx, double &degy, double &degz ) {

  if ( ForeOrBack() ){
    switch ( m_swingType ){
    case SWING_NORMAL:
    case SWING_SMASH:
      if ( m_swing < 10 ){
	degz = -(double)m_swing/10*90;
	degx = degy = 0.0;
      }
      else if ( m_swing < 30 ){
	degz = -90+(double)(m_swing-10)/20*120;
	degx = (double)(m_swing-10)/20*75;
	degy = 0.0;
      }
      else{
	degz = 30-(double)(m_swing-30)/20*30;
	degx = 75-(double)(m_swing-30)/20*75;
	degy = 0.0;
      }
      break;
    case SWING_POKE:
      if ( m_swing < 10 ){
	degz = -(double)m_swing/10*90;
	degx = (double)m_swing/10*45;
	degy = (double)m_swing/10*45;
      }
      else if ( m_swing < 30 ){
	degz = -90+(double)(m_swing-10)/20*150;
	degx = 45 - (double)(m_swing-10)/20*45;
	degy = 45.0;
      }
      else{
	degz = 60-(double)(m_swing-30)/20*30;
	degx = 0.0;
	degy = 45.0 - (double)(m_swing-30)/20*45;
      }
      break;
    case SWING_CUT:
      if ( m_swing < 10 ){
	degz = -(double)m_swing/10*90;
	degx = 0.0;
	degy = (double)m_swing/10*45;
      }
      else if ( m_swing < 30 ){
	degz = -90+(double)(m_swing-10)/20*150;
	degx = -(double)(m_swing-10)/20*60;
	degy = 45.0;
      }
      else{
	degz = 60-(double)(m_swing-30)/20*30;
	degx = -60+(double)(m_swing-30)/20*60;
	degy = 45.0 - (double)(m_swing-30)/20*45;
      }
      break;
    case SWING_DRIVE:
      if ( m_swing < 10 ){
	degz = -(double)m_swing/10*90;
	degx = -(double)m_swing/10*90;
	degy = 0.0;
      }
      else if ( m_swing < 30 ){
	degz = -90+(double)(m_swing-10)/20*120;
	degx = -90;
	degy = 0.0;
      }
      else{
	degz = 30-(double)(m_swing-30)/20*30;
	degx = -90+(double)(m_swing-30)/20*90;
	degy = 0.0;
      }
      break;
    }
  }
  else{
    switch ( m_swingType ){
    case SWING_NORMAL:
    case SWING_SMASH:
    case SWING_DRIVE:
      if ( m_swing < 10 ){
	degz = (double)m_swing/10*90;
	degx = degy = 0.0;
      }
      else if ( m_swing < 30 ){
	degz = 90-(double)(m_swing-10)/20*120;
	degx = (double)(m_swing-10)/20*75;
	degy = 0.0;
      }
      else{
	degz = -30+(double)(m_swing-30)/20*30;
	degx = 75-(double)(m_swing-30)/20*75;
	degy = 0.0;
      }
      break;
    case SWING_POKE:
      if ( m_swing < 10 ){
	degz = (double)m_swing/10*90;
	degx = 0.0;
	degy = -(double)m_swing/10*45;
      }
      else if ( m_swing < 30 ){
	degz = 90-(double)(m_swing-10)/20*120;
	degx = 0.0;
	degy = -45.0;
      }
      else{
	degz = -30+(double)(m_swing-30)/20*30;
	degx = 0.0;
	degy = -45.0 + (double)(m_swing-30)/20*45;
      }
      break;
    case SWING_CUT:
      if ( m_swing < 10 ){
	degz = (double)m_swing/10*90;
	degx = (double)m_swing/10*30;
	degy = -(double)m_swing/10*45;
      }
      else if ( m_swing < 30 ){
	degz = 90-(double)(m_swing-10)/20*120;
	degx = 30-(double)(m_swing-10)/20*90;
	degy = -45.0;
      }
      else{
	degz = -30+(double)(m_swing-30)/20*30;
	degx = -60+(double)(m_swing-30)/20*60;
	degy = -45.0 + (double)(m_swing-30)/20*45;
      }
      break;
    }
  }

  return true;
}

bool
Player::ForeOrBack() {
  double px, py, bx, by, bvx, bvy;
  double t, btx;

  // 奥側ならば180度回転
  if ( m_side < 0 ){
    px = -m_x;
    py = -m_y;
    bx = -theBall.GetX();
    by = -theBall.GetY();
    bvx = -theBall.GetVX();
    bvy = -theBall.GetVY();
  }
  else{
    px = m_x;
    py = m_y;
    bx = theBall.GetX();
    by = theBall.GetY();
    bvx = theBall.GetVX();
    bvy = theBall.GetVY();
  }

  // ボールの予想到達地点
  if ( bvy == 0.0 )
    btx = bx;
  else{
    t = (py - by) / bvy;
    btx = bx + bvx*t;
  }

  // フォアかバックか
  if ( btx - px > 0.0 )		// フォア
    return true;
  else
    return false;
}

bool
Player::SwingError() {
  double diff;

  if ( (m_y-theBall.GetY())*m_side < 0 )
    diff = fabs(m_y-theBall.GetY())/2;
  else
    diff = fabs(m_y-theBall.GetY());

  if ( diff < 0.03 )
    m_swingError = SWING_PERFECT;
  else if ( diff < 0.1 )
    m_swingError = SWING_GREAT;
  else if ( diff < 0.2 )
    m_swingError = SWING_GOOD;
  else
    m_swingError = SWING_BOO;

  return true;
}
