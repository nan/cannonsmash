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

extern double maglate;
extern Player* thePlayer;
extern Player* comPlayer;
extern Ball theBall;
extern BaseView theView;
extern long mode;
extern Sound theSound;

extern long wins;

Ball::Ball() {
  m_x = m_y = m_vx = m_vy = m_spin = 0.0;
  m_status = -1000;
  m_Score1 = m_Score2 = 0;

  m_View = NULL;
}

Ball::Ball( double x, double y, double z, double vx, double vy, double vz,
	    double spin, long status ) {
  m_x = x;
  m_y = y;
  m_z = z;
  m_vx = vx;
  m_vy = vy;
  m_vz = vz;
  m_spin = spin;
  m_status = status;

  m_Score1 = m_Score2 = 0;
  m_View = NULL;
}

Ball::~Ball() {
  if ( m_View ){
    theView.RemoveView( m_View );
    delete m_View;
  }
}

bool
Ball::Init() {
  m_View = new BallView();

  m_View->Init();

  theView.AddView( m_View );

  return true;
}

double
Ball::GetX() {
  return m_x;
}

double
Ball::GetY() {
  return m_y;
}

double
Ball::GetZ() {
  return m_z;
}

double
Ball::GetVX() {
  return m_vx;
}

double
Ball::GetVY() {
  return m_vy;
}

double
Ball::GetVZ() {
  return m_vz;
}

double
Ball::GetSpin() {
  return m_spin;
}

long
Ball::GetStatus() {
  return m_status;
}

long
Ball::GetService() {
  if ( m_Score1 > 19 && m_Score2 > 19 ) {	// Deuce
    return ((m_Score1+m_Score2) & 1 ? -1 : 1);
  } else {
    if ( (m_Score1 + m_Score2)%10 >= 5 )
      return -1;
    else
      return 1;
  }
}

long
Ball::GetScore( Player *p ) {
  if ( p->GetSide() > 0 )
    return m_Score1;
  else
    return m_Score2;
}

bool
Ball::Move() {
  double netT , tableT;         /* ネット、テーブルの衝突判定フラグ */
  double x , y, z;              /* ボールのx,y座標の保存用 */
  double tableY;                /* テーブルバウンド時のx座標 */

// ボールデッドの時は速やかに元に戻す
  if ( m_status < 0 )
    m_status--;

  if ( m_status < -100 || m_status == 8 ){
    if ( GetService() > 0 ) {
      m_x = thePlayer->GetX()+0.3;
      m_y = thePlayer->GetY();
    } else {
      m_x = comPlayer->GetX()-0.3;
      m_y = comPlayer->GetY();
    }

    m_z = TABLEHEIGHT + 0.15;
    m_vx = 0.0;
    m_vy = 0.0;
    m_vz = 0.0;

    m_status = 8;

    if ( IsGameEnd() == true ){
      theView.EndGame();
      EndGame();
    }

    return true;
  }

/* 速度の加算 */ 
  x = m_x;
  y = m_y;
  z = m_z;
  m_x += (m_vx*2-PHY*m_vx*fabs(m_vx)*TICK)/2*TICK;
  m_y += (m_vy*2-PHY*m_vy*TICK)/2*TICK;
  m_z += (m_vz*2-GRAV*TICK-PHY*m_vz*TICK)/2*TICK;

/* 衝突判定 */
  if ( y*m_y <= 0.0 ){
    netT = fabs( y/((m_y-y)/TICK) );
    if ( z+(m_z-z)*netT/TICK < TABLEHEIGHT ||
	 z+(m_z-z)*netT/TICK > TABLEHEIGHT+NETHEIGHT ||
	 x+(m_x-x)*netT/TICK < -TABLEWIDTH/2-NETHEIGHT ||
	 x+(m_x-x)*netT/TICK > TABLEWIDTH/2+NETHEIGHT )
      netT = TICK*100;
  } else
    netT = TICK*100;

  if ( (z-TABLEHEIGHT)*(m_z-TABLEHEIGHT) <= 0.0 ){
    tableT = fabs( (z-TABLEHEIGHT)/((m_z-z)/TICK) );
    if ( tableT <= 0.0 || y+(m_y-y)*tableT/TICK < -TABLELENGTH/2 ||
	 y+(m_y-y)*tableT/TICK > TABLELENGTH/2 ||
	 x+(m_x-x)*tableT/TICK < -TABLEWIDTH/2 ||
	 x+(m_x-x)*tableT/TICK > TABLEWIDTH/2 )
      tableT = TICK*100;
  } else
    tableT = TICK*100;

  if ( netT < tableT ){	// ネットに接触
    m_vy = -m_vy*0.2;
    m_y = m_vy*(TICK-netT);
  }

  if ( tableT < netT ){	// テーブルに接触
    if ( this == &theBall ) {
      theSound.Play( SOUND_TABLE );
    }
    tableY = y+m_vy*tableT;
    if ( tableY < 0 ){		// player側のテーブル
      switch( m_status ){
      case 2:
	m_status = 3;
	break;
      case 4:
	m_status = 0;
	break;
      default:
	if ( m_status >= 0 ) {
	  if ( &theBall == this )
	    ChangeScore();
	  m_status = -1;
	}
      }
    } else{			// COM側のテーブル
      switch( m_status ){
      case 0:
	m_status = 1;
	break;
      case 5:
	m_status = 2;
	break;
      default:
	if ( m_status >= 0 ) {
	  if ( &theBall == this )
	    ChangeScore();
	  m_status = -1;
	}
      }
    }

    m_vz -= GRAV*tableT;
    m_vz += -PHY*m_vz*tableT;
    m_vz *= -TABLE_E;
    m_z = TABLEHEIGHT + (TICK-tableT)*m_vz;
    m_vz -= GRAV*(TICK-tableT);
    m_vz += -PHY*m_vz*(TICK-tableT);

    m_vy += -PHY*m_vy*tableT;

    if ( m_vy > 0 )
      m_vy += m_spin*0.8;
    else
      m_vy -= m_spin*0.8;

    m_vy += -PHY*m_vy*(TICK-tableT);
    m_vx += -PHY*m_vx*TICK;

    m_spin *= 0.8;

    return true;
  }

/* 周りの壁との衝突判定 */
  if ( m_x < -AREAXSIZE/2 ){
    m_x = -AREAXSIZE/2;
    m_vx = -m_vx*TABLE_E/2;
    if ( this == &theBall ) {
      theSound.Play( SOUND_TABLE );
    }
    if ( m_status >= 0 ) {
      if ( &theBall == this )
	ChangeScore();
      m_status = -1;
    }
  }
  else if ( m_x > AREAXSIZE/2 ){
    m_x = AREAXSIZE/2;
    m_vx = -m_vx*TABLE_E/2;
    if ( this == &theBall ) {
      theSound.Play( SOUND_TABLE );
    }
    if ( m_status >= 0 ) {
      if ( &theBall == this )
	ChangeScore();
      m_status = -1;
    }
  }

  if ( m_y < -AREAYSIZE/2 ){
    m_y = -AREAYSIZE/2;
    m_vy = -m_vy*TABLE_E/2;
    if ( this == &theBall ) {
      theSound.Play( SOUND_TABLE );
    }
    if ( m_status >= 0 ) {
      if ( &theBall == this )
	ChangeScore();
      m_status = -1;
    }
  }
  else if ( m_y > AREAYSIZE/2 ){
    m_y = AREAYSIZE/2;
    m_vy = -m_vy*TABLE_E/2;
    if ( this == &theBall ) {
      theSound.Play( SOUND_TABLE );
    }
    if ( m_status >= 0 ) {
      if ( &theBall == this )
	ChangeScore();
      m_status = -1;
    }
  }

  if ( m_z < 0 ){
    m_z = 0;
    m_vz = -m_vz*TABLE_E;
    if ( m_vy > 0 )
      m_vy += m_spin*0.8;
    else
      m_vy -= m_spin*0.8;

    if ( this == &theBall ) {
      theSound.Play( SOUND_TABLE );
    }
    if ( m_status >= 0 ) {
      if ( &theBall == this )
	ChangeScore();
      m_status = -1;
    }
  }
  else if ( m_z > AREAZSIZE ){
    m_z = AREAZSIZE;
    m_vz = -m_vz*TABLE_E;
    if ( this == &theBall ) {
      theSound.Play( SOUND_TABLE );
    }
    if ( m_status >= 0 ) {
      if ( &theBall == this )
	ChangeScore();
      m_status = -1;
    }
  }

// 重力
  m_vz -= GRAV*TICK;


// 空気抵抗
  m_vx += -PHY*m_vx*TICK;
  m_vy += -PHY*m_vy*TICK;
  m_vz += -PHY*m_vz*TICK;

  return true;
}

// ボールのインパクト処理
bool
Ball::Hit( double vx, double vy, double vz, double spin ) {

// 一般の打球

  if ( this == &theBall ) {
      theSound.Play( SOUND_RACKET );
  }

  m_spin = spin;

  if ( m_status == 6 )
    m_status = 4;
  else if ( m_status == 7 )
    m_status = 5;
  else if ( m_status == 3 )
    m_status = 0;
  else if ( m_status == 1 )
    m_status = 2;

  m_vx = vx;
  m_vy = vy;
  m_vz = vz;

  return true;
}

bool
Ball::Toss( Player *player , long power ) {
  m_vz = 2.5;
  m_spin = 0.0;
  if ( player->GetSide() > 0 )
    m_status = 6;
  else
    m_status = 7;

  return true;
}

// 空気抵抗あり版
// 現在のボールの位置, target, heightから, ボールの初速を割り出す
// target --- 目標落下地点
// level --- 全力の何%で打球するか
bool
Ball::TargetToV( double targetX, double targetY, double level, double spin, 
		 double &vx, double &vy, double &vz, double vMin, 
		 double vMax ) {
  double y;
  double vyMin = 0.1, vyMax;
  double t1, t2, z1;

  vyMax = fabs(targetY-m_y)/hypot(targetX-m_x, targetY-m_y)*vMax;

  if ( targetY < m_y ){
    y = -m_y;
    targetY = -targetY;
  } else
    y = m_y;


  while (vyMax-vyMin > 0.001) {
    vy = (vyMin+vyMax)/2;

    // まず, ボールがtargetYに到達するまでの時間t2を求める. 
    t2 = -log( 1- PHY*(targetY-y)/vy ) / PHY;
    // ボールがネット上に到達するまでの時間t1を求める. 
    t1 = -log( 1- PHY*(-y)/vy ) / PHY;

    // t2時に, z=TABLEHEIGHTとなるvzを求める. 
    vz = (PHY*(TABLEHEIGHT-m_z)+GRAVITY(spin)*t2)/(1-exp(-PHY*t2)) -
      GRAVITY(spin)/PHY;

    // t1時のボールの高さを求める. 
    z1 = -(vz+GRAVITY(spin)/PHY)*exp(-PHY*t1)/PHY - GRAVITY(spin)*t1/PHY +
      (vz+GRAVITY(spin)/PHY)/PHY;

    if ( z1 < TABLEHEIGHT+NETHEIGHT-m_z )
      vyMax = vy;
    else
      vyMin = vy;
  }

  vy *= level;

  t2 = -log( 1- PHY*(targetY-y)/vy ) / PHY;

  vz = (PHY*(TABLEHEIGHT-m_z)+GRAVITY(spin)*t2)/(1-exp(-PHY*t2)) -
    GRAVITY(spin)/PHY;

  if ( y != m_y )
    vy = -vy;

  vx = PHY*(targetX-m_x) / (1-exp(-PHY*t2));

  return true;
}

bool
Ball::TargetToVS( double targetX, double targetY, double level, double spin, 
		  double &vx, double &vy, double &vz ) {
  double boundY = -TABLELENGTH/2;
  double y;
  double tmpVX = 0.0, tmpVY = 0.0, tmpVZ = 0.0;

  if ( targetY < m_y ){
    y = -m_y;
    targetY = -targetY;
  } else
    y = m_y;

  for ( boundY = -TABLELENGTH/2 ; boundY < 0 ; boundY += TICK ) {
    double vyMin = 0.1;
    double vyMax = 30.0;
    double vyCurrent, vzCurrent;
    double t1, t2, t3;
    double z;

    while (vyMax-vyMin > 0.001) {
      vy = (vyMin+vyMax)/2;

      // まず, ボールがboundYに到達するまでの時間t2を求める. 
      t2 = -log( 1- PHY*(boundY-y)/vy ) / PHY;

      // t2時に, z=TABLEHEIGHTとなるvzを求める. 
      vz = (PHY*(TABLEHEIGHT-m_z)+GRAVITY(spin)*t2)/(1-exp(-PHY*t2)) -
	    GRAVITY(spin)/PHY;

      // バウンド
      vyCurrent = vy*exp(-PHY*t2);
      vzCurrent = (vz+GRAVITY(spin)/PHY)*exp(-PHY*t2) - GRAVITY(spin)/PHY;

      vyCurrent += spin*0.8;
      vzCurrent *= -TABLE_E;

      t1 = -log( 1- PHY*(targetY-boundY)/vyCurrent ) / PHY;

      z = -( vzCurrent+GRAVITY(spin*0.8)/PHY)*exp(-PHY*t1)/PHY
	 - GRAVITY(spin*0.8)/PHY*t1
	 + (vzCurrent+GRAVITY(spin*0.8)/PHY)/PHY;

      if ( z > 0 )
	vyMax = vy;
      else
	vyMin = vy;
    }

    if ( fabs(z) < TICK ) {
      t3 = -log( 1- PHY*(-boundY)/vyCurrent ) / PHY;
      z = -( vzCurrent+GRAVITY(spin*0.8)/PHY)*exp(-PHY*t3)/PHY
	 - GRAVITY(spin*0.8)/PHY*t3
	 + (vzCurrent+GRAVITY(spin*0.8)/PHY)/PHY;
      if ( z > NETHEIGHT+(1.0-level)*0.1 ) {	// temporary
        if ( vy > tmpVY ) {
	  tmpVX = PHY*(targetX-m_x) / (1-exp(-PHY*(t1+t2)));
	  tmpVY = vy;
	  tmpVZ = vz;
	}
      }
    }
  }
  vx = tmpVX;
  vy = tmpVY;

  if ( y != m_y )
    vy = -vy;

  vz = tmpVZ;

  return true;
}

void
Ball::EndGame() {
  if ( mode == MODE_DEMO || mode == MODE_TITLE || mode == MODE_HOWTO ) {
    mode = MODE_TITLE;
  } else {
    // 再初期化する
    if ( theBall.GetScore(thePlayer) > theBall.GetScore(comPlayer) )
      wins++;
    else
      wins = 0;

    if ( wins > 0 )
      mode = MODE_SELECT;
    else
      mode = MODE_TITLE;
  }

  m_Score1 = m_Score2 = 0;
  m_status = -1000;
}

void
Ball::ChangeScore() {
  if ( mode == MODE_PLAY || mode == MODE_DEMO || mode == MODE_TITLE ){
    if ( m_status == 0 || m_status == 3 || m_status == 4 || m_status == 6 ) {
      m_Score2++;
    } else
      m_Score1++;
  }
}

bool
Ball::IsGameEnd() {
  if ( (m_Score1 > 20 || m_Score2 > 20) && abs( m_Score1-m_Score2 ) > 1 &&
       this == &theBall ){
    return true;
  } else
    return false;
}
