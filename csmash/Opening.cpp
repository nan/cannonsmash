/**
 * @file
 * @brief Implementation of Opening class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004  神南 吉宏(Kanna Yoshihiro)
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
#include "Opening.h"
#include "OpeningView.h"
#include "BaseView.h"
#include "Sound.h"
#include "Player.h"
#include "Ball.h"
#include "Event.h"
#include "HitMark.h"
#include "RCFile.h"

#include <signal.h>

extern long mode;

extern void Keyboard( unsigned char key, int x, int y );
extern void KeyUp( unsigned char key, int x, int y );

extern Ball theBall;

extern RCFile *theRC;

#define PHRASELENGTH (35.80)

/**
 * Constructor. 
 * Initialize member variables to 0 or NULL. 
 */
Opening::Opening() {
  m_View = NULL;
  m_count = 0;
  m_bgmCount = 0;
  m_pid = 0;
}

/**
 * Destructor. 
 * Detatch view class and delete it. 
 */
Opening::~Opening() {
  if ( m_View ){
    BaseView::TheView()->RemoveView( m_View );
    delete m_View;
  }

#ifndef WIN32
  if ( m_pid > 0 ) {
    kill ( m_pid, SIGKILL );
  }
#endif
}

/**
 * Initializer method. 
 * This method creates OpeningView object and attach it to this object. 
 * Additionally, creates m_thePlayer and m_comPlayer and their view classes. 
 * 
 * @return returns true if succeeds. 
 */
bool
Opening::Init() {
  m_View = (OpeningView *)View::CreateView( VIEW_OPENING );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  m_thePlayer = Player::Create( 1, 1, 0 );
  m_comPlayer = Player::Create( 2, -1, 0 );

  m_thePlayer->m_View = (PlayerView *)View::CreateView( VIEW_PLAYER );
  m_thePlayer->m_View->Init( m_thePlayer );

  m_comPlayer->m_View = (PlayerView *)View::CreateView( VIEW_PLAYER );
  m_comPlayer->m_View->Init( m_comPlayer );

  HitMark::Init();

  static char *telop[] = {"images/ArctanX" };

  m_telop[0] = new MenuItem();
  m_telop[0]->Init( 0, 0, 800, 600, telop[0], NULL );

  Sound::TheSound()->InitBGM( OPENINGFILENAME );

  return true;
}

/**
 * Opening object creater. 
 * This method creates singleton Opening object. 
 */
void
Opening::Create() {
  Control::ClearControl();

  m_theControl = new Opening();
  m_theControl->Init();
}

/**
 * Move valid objects. 
 * Referring count, modify the status of objects and call Move method 
 * of the ball and players. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
Opening::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
	       long *MouseYHistory, unsigned long *MouseBHistory,
	       int Histptr ) {
  long phrase, mod;

  GetPhrase( phrase, mod );

  // afterSwing should be disabled for opening. 
  m_thePlayer->m_afterSwing = 0;
  m_comPlayer->m_afterSwing = 0;

  if ( m_count == 0 ) {
    m_thePlayer->m_x[0] = -1.5;
    m_thePlayer->m_x[1] = -TABLELENGTH*3/2;

    m_comPlayer->m_x[0] = 1.5;
    m_comPlayer->m_x[1] = TABLELENGTH*3/2;

    //m_count = 5700;
  } else if ( phrase < 16 ) {	// なし
    m_thePlayer->m_swing = 0;
    m_comPlayer->m_swing = 0;
  } else if ( phrase < 36 ) {	// 素振り
    if ( m_count%200 < 60 ) {
      m_thePlayer->m_swing = (m_count%200)/2;
    } else if ( m_count%200 < 140 ) {
      m_thePlayer->m_swing = (m_count%200)/4+15;
    } else {
      m_thePlayer->m_swing = 0;
    }

    m_thePlayer->m_swingType = SWING_CUT;
    if ( m_count%400 < 200 )
      m_thePlayer->m_swingSide = true;
    else
      m_thePlayer->m_swingSide = false;

    if ( m_count%200 < 60 ) {
      m_comPlayer->m_swing = (m_count%200)/2;
    } else if ( m_count%200 < 140 ) {
      m_comPlayer->m_swing = (m_count%200)/4+15;
    } else {
      m_comPlayer->m_swing = 0;
    }

    m_comPlayer->m_swingType = SWING_DRIVE;
    if ( m_count%400 < 200 )
      m_comPlayer->m_swingSide = true;
    else
      m_comPlayer->m_swingSide = false;
  } else if ( phrase < 48 ) {	// 台前へ
    m_thePlayer->m_x[0] = -1.5+(m_count-1800)*1.5/600.0;
    m_thePlayer->m_x[1] = -TABLELENGTH*3/2+(m_count-1800)*TABLELENGTH/600.0;
    m_comPlayer->m_x[0] = 1.5-(m_count-1800)*1.5/600.0;
    m_comPlayer->m_x[1] = TABLELENGTH*3/2-(m_count-1800)*TABLELENGTH/600.0;

    theBall.m_x[2] = -1.0;
  } else if ( m_count == 2400 ) {	// 審判からボールを受け取る
    theBall.m_x[0] = -TABLEWIDTH/2;
    theBall.m_x[1] = 0;
    theBall.m_x[2] = TABLEHEIGHT+NETHEIGHT*3;
    theBall.m_v[0] = (TABLEWIDTH/2+0.3)/2.0;
    theBall.m_v[1] = -TABLELENGTH/2/2.0;
    theBall.m_v[2] = 1.0;
    theBall.m_status = -1;

    m_thePlayer->m_target[0] = -TABLEWIDTH/2+0.2;
    m_thePlayer->m_target[1] = TABLELENGTH/2-0.2;
  } else if ( phrase < 52 ) {	// ボール転がる
    theBall.Move();
    theBall.m_status = -1;
  } else if ( m_count == 2600 ) {	// ボール取る
    theBall.m_status = 8;
    theBall.m_x[0] = m_thePlayer->GetX()[0]+0.3;
    theBall.m_x[1] = m_thePlayer->GetX()[1];
    theBall.m_x[2] = TABLEHEIGHT + 0.15;
    theBall.m_v[0] = 0.0;
    theBall.m_v[1] = 0.0;
    theBall.m_v[2] = 0.0;
  } else if ( phrase < 56 ) {	// 構える
    m_thePlayer->m_v[1] = -0.1;
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    theBall.Move();
  } else if ( phrase < 68 ) {
  } else if ( phrase == 68 && mod == 0 ) {	// トス
    m_thePlayer->m_swing = 0;
    theBall.Toss( m_thePlayer, 3 );
    m_thePlayer->StartServe(3);
  } else if ( phrase < 72 ) {	// サーブ
    if ( mod == 0 ) {
      for ( int i = 0 ; i < 15 ; i++ ) {
	theBall.Move();
	m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
	m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
      }
    }
  } else if ( phrase < 76 ) {	// レシーブ
    if ( phrase == 72 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2;
    }

    if ( phrase == 73 && mod == 0 ) {
      vector3d v;
      theBall.Warp( vector3d((const double[]){TABLEWIDTH/4,
						-TABLELENGTH/4,
						TABLEHEIGHT+0.01}), 
		    vector3d(0.0), vector2d(0.0), 0 );
      theBall.TargetToV( vector2d((const double[]){-TABLEWIDTH/2+0.1,
						     TABLELENGTH/2-0.1}),
			 0.7, vector2d((const double[]){0.0, 0.0}),
			 v, 0.1, 20.0 );
      theBall.Warp( vector3d((const double[]){TABLEWIDTH/4,
						-TABLELENGTH/4,
						TABLEHEIGHT+0.01}),
		    v, vector2d(0.0), 0 );
    }
    if ( phrase >= 73 ) {
      m_comPlayer->m_v[0] = (-TABLEWIDTH+0.1)/0.7;
      m_comPlayer->m_v[1] = 0.5/0.7;
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_thePlayer->m_status = 200;
    m_comPlayer->m_status = 200;

    if ( phrase == 73 && mod == 26 ) {
      m_comPlayer->StartSwing( 0 );
    }
    if ( phrase == 74 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
    }
  } else if ( phrase < 80 ) {	// カット
    if ( phrase == 76 && mod == 0 ) {
      theBall.m_x[0] = theBall.m_x[1] = 0.0;
      theBall.m_x[2] = 3.0;
      theBall.m_v[0] = theBall.m_v[1] = 0.0;
      theBall.m_v[2] = 3.0;
      m_thePlayer->m_target[0] = TABLEWIDTH/2;
      m_thePlayer->m_swing = 0;
    }
    if ( phrase == 77 && mod == 20 ) {
      vector3d v;
      theBall.Warp( vector3d((const double[]){-TABLEWIDTH/2,
						TABLELENGTH/2,
						TABLEHEIGHT+NETHEIGHT}),
		    vector3d(0.0), vector2d(0.0), 1 );
      theBall.TargetToV( vector2d((const double[]){TABLEWIDTH/2-0.1,
						     -TABLELENGTH/2+0.3}),
			 0.8, vector2d((const double[]){0.0, 1.0}),
			 v, 0.1, 20.0 );
      theBall.Warp( vector3d((const double[]){-TABLEWIDTH/2,
						TABLELENGTH/2,
						TABLEHEIGHT+NETHEIGHT}),
		    v, vector2d((const double[]){0.0,1.0}), 2 );
    }
    if ( phrase >= 77 && phrase <= 78 ) {
      m_thePlayer->m_v[0] = (TABLEWIDTH-0.1)/0.7;
      m_thePlayer->m_v[1] = -TABLELENGTH/2/0.7;
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_thePlayer->m_status = 200;
    m_comPlayer->m_status = 200;

    if ( phrase == 77 && mod == 26 ) {
      m_thePlayer->StartSwing( 0 );
      m_thePlayer->m_swingType = SWING_CUT;
      m_comPlayer->m_swingSide = true;
    }
    if ( phrase == 78 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_thePlayer->Swing(3);
    }
  } else if ( phrase < 84 ) {	// バックへドライブ
    if ( phrase == 80 && mod == 0 ) {
      m_comPlayer->m_x[0] = -TABLEWIDTH/2;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.5;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;

      vector3d v;
      theBall.Warp( vector3d((const double[]){TABLEWIDTH/2+0.5,
						-TABLELENGTH,
						TABLEHEIGHT+NETHEIGHT}),
		    vector3d(0.0), vector2d(0.0), 0 );
      theBall.TargetToV( vector2d((const double[]){TABLEWIDTH/2-0.1,
						     TABLELENGTH/2-0.1}),
			 0.53, vector2d((const double[]){0.0, -1.0}),
			 v, 0.1, 20.0 );
      theBall.Warp( vector3d((const double[]){TABLEWIDTH/2+0.5,
						-TABLELENGTH,
						TABLEHEIGHT+NETHEIGHT}),
		    v, vector2d((const double[]){0.0, -1.0}), 0 );
    }
    if ( phrase >= 81 ) {
      m_comPlayer->m_v[0] = (TABLEWIDTH+0.1)/0.7;
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_thePlayer->m_status = 200;
    m_comPlayer->m_status = 200;

    if ( phrase == 81 && mod == 26 ) {
      m_comPlayer->StartSwing( 0 );
      m_comPlayer->m_swingType = SWING_DRIVE;
      m_comPlayer->m_swingSide = true;
    }
    if ( phrase == 82 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_comPlayer->Swing(3);
    }
  } else if ( phrase < 88 ) {	// バックカット
    if ( phrase == 84 && mod == 0 ) {
      theBall.m_x[0] = theBall.m_x[1] = 0.0;
      theBall.m_x[2] = 3.0;
      theBall.m_v[0] = theBall.m_v[1] = 0.0;
      theBall.m_v[2] = 3.0;
      m_thePlayer->m_x[0] = TABLEWIDTH/2;
      m_thePlayer->m_x[1] = -TABLELENGTH-0.5;
      m_thePlayer->m_v[0] = -(TABLEWIDTH+0.4)/0.7;
      m_thePlayer->m_v[1] = 0.0;
      m_thePlayer->m_target[0] = -TABLEWIDTH/2;
    }
    if ( phrase == 85 && mod == 20 ) {
      vector3d v;
      theBall.Warp( vector3d((const double[]){TABLEWIDTH/2-0.2,
						TABLELENGTH/2,
						TABLEHEIGHT+NETHEIGHT}),
		    vector3d(0.0), vector2d(0.0), 1 );
      theBall.TargetToV( vector2d((const double[]){-TABLEWIDTH/2+0.2,
						     -TABLELENGTH/2+0.3}),
			 0.8, vector2d((const double[]){0.0, 1.0}),
			 v, 0.1, 20.0 );
      theBall.Warp( vector3d((const double[]){TABLEWIDTH/2-0.2,
						TABLELENGTH/2,
						TABLEHEIGHT+NETHEIGHT}),
		    v, vector2d((const double[]){0.0, 1.0}), 2 );
    }
    if ( phrase >= 85 ) {
      theBall.Move();
      m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

      m_thePlayer->m_status = 200;
      m_comPlayer->m_status = 200;
    }

    if ( phrase == 85 && mod == 26 ) {
      m_thePlayer->StartSwing( 0 );
      m_thePlayer->m_swingType = SWING_CUT;
      m_thePlayer->m_swingSide = false;
    }
    if ( phrase == 86 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_thePlayer->Swing( 0 );
    }
    if ( phrase == 87 && mod == 0 ) {
      m_thePlayer->m_v[0] = 0.0;
    }
  } else if ( phrase < 92 ) {	// ストップ
    if ( phrase == 88 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH+0.5;
      m_comPlayer->m_v[0] = -(TABLEWIDTH/2+0.3)/1.05;
      m_comPlayer->m_v[1] = -TABLELENGTH/1.05;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.3;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+1.0;

      vector3d v;
      theBall.Warp( vector3d((const double[]){-TABLEWIDTH/2-0.4,
						-TABLELENGTH-0.5,
						TABLEHEIGHT+NETHEIGHT}),
		    vector3d(0.0), vector2d(0.0), 0 );
      theBall.TargetToV( vector2d((const double[]){-TABLEWIDTH/2+0.2,
						     TABLELENGTH/2-0.85}),
			 0.7, vector2d((const double[]){0.0, -1.0}),
			 v, 0.1, 20.0 );
      theBall.Warp( vector3d((const double[]){-TABLEWIDTH/2-0.4,
						-TABLELENGTH-0.5,
						TABLEHEIGHT+NETHEIGHT}),
		    v, vector2d((const double[]){0.0, -1.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_thePlayer->m_status = 200;
    m_comPlayer->m_status = 200;

    if ( phrase == 89 && mod == 26 ) {
      m_comPlayer->StartSwing( 0 );
    }
    if ( phrase == 90 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_comPlayer->Swing(3);
    }
    if ( phrase == 91 && mod == 0 ) {
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      theBall.m_v[2] += 2.0;
    }
  } else if ( phrase < 96 ) {	// クロスへスマッシュ
    if ( phrase == 92 && mod == 0 ) {
      m_thePlayer->m_x[0] = TABLEWIDTH/2+0.4;
      m_thePlayer->m_x[1] = -TABLELENGTH-0.5;
      m_thePlayer->m_v[0] = -(TABLEWIDTH/2+0.5)/0.7;
      m_thePlayer->m_v[1] = (TABLELENGTH/2+1.0)/0.7;
      m_thePlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_thePlayer->m_target[1] = TABLELENGTH/2-0.8;

      vector3d v;
      theBall.Warp( vector3d((const double[]){-0.5,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.05}),
		    vector3d(0.0), vector2d(0.0), 1 );
      theBall.TargetToV( vector2d((const double[]){-TABLEWIDTH/2+0.3,
						     -TABLELENGTH/2+0.8}),
			 0.5, vector2d((const double[]){0.0, -0.5}),
			 v, 0.1, 20.0 );
      theBall.Warp( vector3d((const double[]){-0.5,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.05}),
		    v, vector2d((const double[]){0.0, -0.5}), 2 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_thePlayer->m_status = 200;
    m_comPlayer->m_status = 200;

    if ( phrase == 93 && mod == 26 ) {
      m_thePlayer->StartSwing( 0 );
    }
    if ( phrase == 94 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_thePlayer->Swing( 0 );
    }
    if ( phrase == 94 && mod == 0 ) {
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;
    }
    if ( phrase == 95 && mod == 0 ) {	// 高速化
      theBall.TargetToV( m_thePlayer->m_target, 1.0, vector2d(0.0),
			 theBall.m_v, 0.1, 10.0 );
    }
  } else if ( phrase < 100 ) {	// リプレイ
    if ( phrase == 96 && mod == 0 ) {
      m_thePlayer->m_x[0] = TABLEWIDTH/2+0.4;
      m_thePlayer->m_x[1] = -TABLELENGTH-0.5;
      m_thePlayer->m_v[0] = -(TABLEWIDTH/2+0.5)/0.7;
      m_thePlayer->m_v[1] = (TABLELENGTH/2+1.0)/0.7;
      m_thePlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_thePlayer->m_target[1] = TABLELENGTH/2-0.8;

      vector3d v;
      theBall.Warp( vector3d((const double[]){-0.5,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.05}),
		    vector3d(0.0), vector2d(0.0), 1 );
      theBall.TargetToV( vector2d((const double[]){-TABLEWIDTH/2+0.3,
						     -TABLELENGTH/2+0.8}),
			 0.5, vector2d((const double[]){0.0, -0.5}),
			 v, 0.1, 20.0 );
      theBall.Warp( vector3d((const double[]){-0.5,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.05}),
		    v, vector2d((const double[]){0.0, -0.5}), 2 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_thePlayer->m_status = 200;
    m_comPlayer->m_status = 200;

    if ( phrase == 98 && mod == 1 ) {
      m_thePlayer->StartSwing( 0 );
    }
    if ( phrase == 98 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_thePlayer->Swing( 0 );
    }
    if ( phrase == 98 && mod == 0 ) {
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;
    }
    if ( phrase == 99 && mod == 0 ) {	// 高速化
      theBall.TargetToV( m_thePlayer->m_target, 1.0, vector2d(0.0),
			 theBall.m_v, 0.1, 10.0 );
    }
  } else if ( phrase < 104 ) {	// ボール転がる
    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
  } else if ( phrase < 106 ) {	// スマッシュ連打(1)
    if ( phrase == 104 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;
      m_thePlayer->m_swing = 0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){-1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 104 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 104 && mod == (int)(PHRASELENGTH-10) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 108 ) {	// スマッシュ連打(2)
    if ( phrase == 106 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 106 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 106 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 110 ) {	// スマッシュ連打(3)
    if ( phrase == 108 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){-1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 108 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 108 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 112 ) {	// スマッシュ連打(4)
    if ( phrase == 110 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 110 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 110 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 114 ) {	// スマッシュ連打(5)
    if ( phrase == 112 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){-1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 112 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 112 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 116 ) {	// スマッシュ連打(6)
    if ( phrase == 114 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 114 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 114 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 118 ) {	// スマッシュ連打(7)
    if ( phrase == 116 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 116 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 116 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 120 ) {	// スマッシュ連打(8)
    if ( phrase == 118 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 118 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 118 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 122 ) {	// スマッシュ連打(9)
    if ( phrase == 120 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){-1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 120 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 120 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 124 ) {	// スマッシュ連打(10)
    if ( phrase == 122 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 122 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 122 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 126 ) {	// スマッシュ連打(11)
    if ( phrase == 124 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){-1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 124 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 124 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 128 ) {	// スマッシュ連打(12)
    if ( phrase == 126 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 126 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 126 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 130 ) {	// スマッシュ連打(13)
    if ( phrase == 128 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){-1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 128 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 128 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 132 ) {	// スマッシュ連打(14)
    if ( phrase == 130 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = -TABLEWIDTH/2+0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 130 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 130 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 134 ) {	// スマッシュ連打(15)
    if ( phrase == 132 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH/2-0.3;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;

      m_comPlayer->m_x[0] = -TABLEWIDTH/2+0.5;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){-1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 132 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 132 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 136 ) {	// スマッシュ連打(16)
    if ( phrase == 134 && mod == 0 ) {
      m_comPlayer->m_x[0] = TABLEWIDTH/2+0.1;
      m_comPlayer->m_x[1] = TABLELENGTH/2+0.3;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_target[0] = TABLEWIDTH/2-0.1;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.1;

      theBall.Warp( vector3d((const double[]){0.0,
						TABLELENGTH/2-0.5,
						TABLEHEIGHT+0.01}),
		    vector3d((const double[]){1.6, 2.2, -5.0}),
		    vector2d((const double[]){0.0, 0.0}), 0 );
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

    m_comPlayer->m_status = 200;

    if ( phrase == 134 && mod == 1 ) {
      m_comPlayer->StartSwing( 0 );
    }

    if ( phrase == 134 && mod == (int)(PHRASELENGTH-9) ) {	// 打球0.1秒前
      m_comPlayer->Swing( 3 );
      m_comPlayer->m_pow = 10;
    }
  } else if ( phrase < 168 ) {	// スローモーション(1)
    static Ball ball1, ball2;
    if ( phrase == 136 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH*3/2;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;
      m_thePlayer->m_swing = 1;
      m_thePlayer->m_swingType = SWING_CUT;
      m_thePlayer->m_swingSide = false;

      m_comPlayer->m_x[0] = 0.0;
      m_comPlayer->m_x[1] = TABLELENGTH/2;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_swing = 1;
      m_comPlayer->m_swingSide = true;

      ball1.Warp( vector3d((const double[]){-0.3,
					      -TABLELENGTH/2-1.2,
					      TABLEHEIGHT+NETHEIGHT}),
		  vector3d((const double[]){0.0, -8.2, -1.0}),
		  vector2d((const double[]){0.0, 0.0}), 3 );
      ball2.Warp( vector3d((const double[]){0.0,
					      TABLELENGTH/4,
					      TABLEHEIGHT+0.01}),
		  vector3d((const double[]){-1.6, 3.5, -5.0}),
		  vector2d((const double[]){0.0, 0.0}), 0 );
    }

    if ( phrase%2 ) {
      theBall = ball1;
      if ( (mod%15) == 0 ) {
	if ( m_thePlayer->m_swing == 10 )
	  m_thePlayer->Swing(0);

	theBall.Move();
	m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      }
      ball1 = theBall;
    } else {
      theBall = ball2;
      if ( (mod%15) == 0 ) {
	if ( m_comPlayer->m_swing == 10 )
	  m_comPlayer->Swing(3);

	theBall.Move();
	m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
      }
      ball2 = theBall;
    }
  } else if ( phrase < 200 ) {	// スローモーション(2)
    static Ball ball1, ball2;
    if ( phrase == 168 && mod == 0 ) {
      m_thePlayer->m_x[0] = 0.0;
      m_thePlayer->m_x[1] = -TABLELENGTH*3/2;
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = 0.0;
      m_thePlayer->m_swing = 1;
      m_thePlayer->m_swingSide = true;

      m_comPlayer->m_x[0] = 0.0;
      m_comPlayer->m_x[1] = TABLELENGTH/2;
      m_comPlayer->m_v[0] = 0.0;
      m_comPlayer->m_v[1] = 0.0;
      m_comPlayer->m_swing = 1;
      m_comPlayer->m_swingSide = false;

      ball1.Warp( vector3d((const double[]){0.3,
					      -TABLELENGTH/2-1.2,
					      TABLEHEIGHT+NETHEIGHT}),
		  vector3d((const double[]){0.0, -8.2, -1.0}),
		  vector2d((const double[]){0.0, 0.0}), 3 );
      ball2.Warp( vector3d((const double[]){0.0,
					      TABLELENGTH/4,
					      TABLEHEIGHT+0.01}),
		  vector3d((const double[]){1.6, 3.5, -5.0}),
		  vector2d((const double[]){0.0, 0.0}), 0 );
    }

    if ( phrase%2 ) {
      theBall = ball1;
      if ( (mod%15) == 0 ) {
	if ( m_thePlayer->m_swing == 10 )
	  m_thePlayer->Swing( 3 );

	theBall.Move();
	m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      }
      ball1 = theBall;
    } else {
      theBall = ball2;
      if ( (mod%15) == 0 ) {
	if ( m_comPlayer->m_swing == 10 )
	  m_comPlayer->Swing(0);

	theBall.Move();
	m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
      }
      ball2 = theBall;
    }
  } else if ( phrase < 216 ) {	// 止め絵, 回り込み (1)
    if ( phrase == 200 && mod == 0 ) {
      m_thePlayer->m_x[0] = -1.2;
      m_thePlayer->m_x[1] = -TABLELENGTH*3/2;
      m_thePlayer->m_swing = 2;
      m_thePlayer->m_swingType = SWING_CUT;
      m_thePlayer->m_swingSide = true;

      m_comPlayer->m_x[0] = 0.0;
      m_comPlayer->m_x[1] = TABLELENGTH*3/4;
      m_comPlayer->m_swing = 30;
      m_comPlayer->m_swingType = SWING_DRIVE;
      m_comPlayer->m_swingSide = false;

      theBall.Warp( vector3d((const double[]){0.2,
						0.1,
						TABLEHEIGHT+NETHEIGHT*2}),
		    vector3d((const double[]){-0.1, -3.0, 1.0}),
		    vector2d((const double[]){0.0, 0.6}), 0 );
    }

    if ( (m_count%10) == 0 ) {
      theBall.Move();
      m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
    }
  } else if ( phrase < 232 ) {	// 止め絵, 回り込み (2)
    if ( phrase == 216 && mod == 0 ) {
      m_thePlayer->m_x[0] = 1.2;
      m_thePlayer->m_x[1] = -TABLELENGTH;
      m_thePlayer->m_swing = 28;
      m_thePlayer->m_swingType = SWING_CUT;
      m_thePlayer->m_swingSide = false;

      m_comPlayer->m_x[0] = -1.0;
      m_comPlayer->m_x[1] = TABLELENGTH/2;
      m_comPlayer->m_swing = 1;
      m_comPlayer->m_swingType = SWING_DRIVE;
      m_comPlayer->m_swingSide = true;

      theBall.Warp( vector3d((const double[]){0.6,
						-TABLELENGTH/2,
						TABLEHEIGHT+NETHEIGHT}),
		    vector3d((const double[]){-1.0, 2.0, 3.0}),
		    vector2d((const double[]){0.0, -0.8}), 0 );
    }

    if ( (m_count%10) == 0 ) {
      theBall.Move();
      m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
    }
  } else if ( phrase < 350 ) {	// デモ
    if ( phrase == 232 && mod == 0 ) {
      delete m_thePlayer;
      delete m_comPlayer;

      m_thePlayer = Player::Create( 1, 1, 1 );
      m_comPlayer = Player::Create( 2, -1, 1 );

      m_thePlayer->m_View = (PlayerView *)View::CreateView( VIEW_PLAYER );
      m_thePlayer->m_View->Init( m_thePlayer );

      m_comPlayer->m_View = (PlayerView *)View::CreateView( VIEW_PLAYER );
      m_comPlayer->m_View->Init( m_comPlayer );

      theRC->gameLevel = LEVEL_HARD;

      theBall.m_status = -1000;
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_thePlayer->m_target[1] = TABLELENGTH/12*5;
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->m_target[1] = -TABLELENGTH/12*5;
#if 0
  } else if ( phrase < 304 ) {	// 高速ラリー
    if ( phrase == 292 && mod == 0 ) {
      delete m_thePlayer;
      delete m_comPlayer;

      m_thePlayer = Player::Create( 0, 1, 0 );
      m_comPlayer = Player::Create( 0, -1, 0 );

      m_thePlayer->m_View = (PlayerView *)View::CreateView( VIEW_PLAYER );
      m_thePlayer->m_View->Init( m_thePlayer );

      m_comPlayer->m_View = (PlayerView *)View::CreateView( VIEW_PLAYER );
      m_comPlayer->m_View->Init( m_comPlayer );

      theBall.m_status = 8;
      m_thePlayer->m_target[0] = -0.3;
      m_thePlayer->m_target[1] = TABLELENGTH/2-0.7;
      m_comPlayer->m_target[0] = 0.3;
      m_comPlayer->m_target[1] = -TABLELENGTH/2+0.7;
    }
    m_thePlayer->m_x[1] = -TABLELENGTH/2-0.5;
    m_comPlayer->m_x[1] = TABLELENGTH/2+0.5;

    if ( phrase == 292 && mod == 10 ) { // 到達時刻が phrase=296 になるよう調整
      theBall.Toss( m_thePlayer, 3 );
      m_thePlayer->StartServe(0);
    }

    if ( phrase == 295 && mod == (int)(PHRASELENGTH-20) ) {
      m_comPlayer->StartSwing(0);
    }
    if ( phrase == 295 && mod == (int)(PHRASELENGTH-10) ) {
      m_comPlayer->Swing(3);
    }

    if ( phrase >= 296 && phrase <= 303 && mod == 0 && (phrase%2) == 0 ) {
      printf( "%f %f %f %f %f %f %d\n", theBall.GetX()[0], theBall.GetX()[1], theBall.GetX()[2], theBall.GetV()[0], theBall.GetV()[1], theBall.GetV()[2], theBall.GetStatus() );
      vector3d v;
      theBall.m_x[1] = m_comPlayer->m_x[1];
      theBall.TargetToV( m_comPlayer->m_target,
			 1.0, vector2d(0.0), v, 0.1, 10.0 );
      theBall.Warp( theBall.GetX(), v,
		    theBall.GetSpin(), theBall.GetStatus() );
    }
    if ( phrase >= 296 && phrase <= 303 && mod == 0 && (phrase%2) == 1 ) {
      printf( "%f %f %f %f %f %f %d\n", theBall.GetX()[0], theBall.GetX()[1], theBall.GetX()[2], theBall.GetV()[0], theBall.GetV()[1], theBall.GetV()[2], theBall.GetStatus() );
      vector3d v;
      theBall.m_x[1] = m_thePlayer->m_x[1];
      theBall.TargetToV( m_comPlayer->m_target, 1.0,
			 vector2d(0.0), v, 0.1, 10.0 );
      theBall.Warp( theBall.GetX(), v,
		    theBall.GetSpin(), theBall.GetStatus() );
    }

    if ( phrase == 296 && mod == (int)(PHRASELENGTH-20) ) {
      m_thePlayer->StartSwing(0);
    }
    if ( phrase == 296 && mod == (int)(PHRASELENGTH-10) ) {
      m_thePlayer->Swing(3);
    }

    if ( phrase == 297 && mod == (int)(PHRASELENGTH-20) ) {
      m_comPlayer->StartSwing(0);
    }
    if ( phrase == 297 && mod == (int)(PHRASELENGTH-10) ) {
      m_comPlayer->m_targetX = -0.3;
      m_comPlayer->Swing(3);
    }

    if ( phrase == 298 && mod == (int)(PHRASELENGTH-20) ) {
      m_thePlayer->StartSwing(0);
    }
    if ( phrase == 298 && mod == (int)(PHRASELENGTH-10) ) {
      m_thePlayer->Swing(0);
    }

    if ( phrase == 299 && mod == (int)(PHRASELENGTH-20) ) {
      m_comPlayer->StartSwing(0);
    }
    if ( phrase == 299 && mod == (int)(PHRASELENGTH-10) ) {
      m_comPlayer->m_target[0] = 0.3;
      m_comPlayer->Swing(3);
    }

    if ( phrase == 300 && mod == (int)(PHRASELENGTH-20) ) {
      m_thePlayer->StartSwing(0);
    }
    if ( phrase == 300 && mod == (int)(PHRASELENGTH-10) ) {
      m_thePlayer->m_target[0] = 0.3;
      m_thePlayer->Swing(3);
    }

    if ( phrase == 301 && mod == (int)(PHRASELENGTH-20) ) {
      m_comPlayer->StartSwing(0);
    }
    if ( phrase == 301 && mod == (int)(PHRASELENGTH-10) ) {
      m_comPlayer->Swing(0);
    }

    if ( phrase == 302 && mod == (int)(PHRASELENGTH-20) ) {
      m_thePlayer->StartSwing(0);
    }
    if ( phrase == 302 && mod == (int)(PHRASELENGTH-10) ) {
      m_thePlayer->m_target[0] = 0.3;
      m_thePlayer->Swing(3);
    }

    if ( phrase == 303 && mod == (int)(PHRASELENGTH-20) ) {
      m_comPlayer->StartSwing(0);
    }
    if ( phrase == 303 && mod == (int)(PHRASELENGTH-10) ) {
      m_comPlayer->Swing(0);
    }

    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
  } else if ( phrase < 320 ) {	// アウト
    if ( phrase == 304 && mod == 11 ) {
      theBall.m_v[0] = 1.0;
      theBall.m_v[1] = -1.0;
      theBall.m_v[2] = 3.0;
    }
    if ( (m_count%10) == 0 ) {
      theBall.Move();
      m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
      m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
    }
  }
#else
  }
#endif

  if ( m_count == 0 )
    Sound::TheSound()->PlayBGM();

  m_count++;

  if ( KeyHistory[Histptr].unicode == SDLK_ESCAPE || MouseBHistory[Histptr] 
       || phrase > 350 ) {
    mode = MODE_TITLE;
    Sound::TheSound()->StopBGM();
  }

  return true;
}

/**
 * Set camera position and direction. 
 * 
 * @param srcX camera position. [out]
 * @param destX point where the camera is directed. [out]
 * @return returns true if succeeds. 
 */
bool
Opening::LookAt( vector3d &srcX, vector3d &destX ) {
  long phrase, mod;
  GetPhrase( phrase, mod );

  if ( phrase < 16 ) {
    srcX[0] = TABLELENGTH*2*cos(m_count*3.14159265/720.0);
    srcX[1] = TABLELENGTH*2*sin(m_count*3.14159265/720.0) + TABLELENGTH/2;
    srcX[2] = TABLEHEIGHT*4;
  } else if ( phrase < 24 ) {
    srcX[0] = m_thePlayer->GetX()[0]-2.0*cos((m_count-800)*3.141592/800.0);
    srcX[1] = m_thePlayer->GetX()[1]+2.0*sin((m_count-800)*3.141592/800.0);
    srcX[2] = (double)(m_count-800)/400.0+0.5;
    destX[0] = m_thePlayer->GetX()[0];
    destX[1] = m_thePlayer->GetX()[1];
    destX[2] = 1.5;
  } else if ( phrase < 32 ) {
    srcX[0] = m_comPlayer->GetX()[0]+2.0*cos((m_count-800)*3.141592/800.0);
    srcX[1] = m_comPlayer->GetX()[1]-2.0*sin((m_count-800)*3.141592/800.0);
    srcX[2] = (double)(m_count-1200)/400.0+0.5;
    destX[0] = m_comPlayer->GetX()[0];
    destX[1] = m_comPlayer->GetX()[1];
    destX[2] = 1.5;
  } else if ( phrase < 48 ) {
    srcX[0] = m_thePlayer->GetX()[0];
    srcX[1] = m_thePlayer->GetX()[1]-1.5;
    srcX[2] = TABLEHEIGHT+1.15;
    destX[0] = m_thePlayer->GetX()[0]+0.3;
    destX[1] = m_thePlayer->GetX()[1];
    destX[2] = TABLEHEIGHT+0.15;
  } else if ( phrase < 56 ) {
    srcX[0] = m_thePlayer->GetX()[0]+(1.0+0.5*(2800-m_count)/400.0)*
      sin((m_count-2400)*3.141592/800.0);
    srcX[1] = m_thePlayer->GetX()[1]-(1.0+0.5*(2800-m_count)/400.0)*
      cos((m_count-2400)*3.141592/800.0);
    srcX[2] = TABLEHEIGHT+1.15-(m_count-2400)/400.0;
    destX[0] = m_thePlayer->GetX()[0]+0.3;
    destX[1] = m_thePlayer->GetX()[1];
    destX[2] = TABLEHEIGHT+0.15;
  } else if ( phrase < 72 ) {
    srcX[0] = m_thePlayer->GetX()[0]+1.0;
    srcX[1] = m_thePlayer->GetX()[1];
    srcX[2] = TABLEHEIGHT+0.15;
    destX[0] = m_thePlayer->GetX()[0]+0.3;
    destX[1] = m_thePlayer->GetX()[1];
    destX[2] = TABLEHEIGHT+0.15;
  } else if ( phrase < 76 ) {
    srcX[0] = m_comPlayer->GetX()[0]-1.3;
    srcX[1] = m_comPlayer->GetX()[1]-1.0;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX[0] = m_comPlayer->GetX()[0]-0.3;
    destX[1] = m_comPlayer->GetX()[1];
    destX[2] = TABLEHEIGHT+NETHEIGHT;
  } else if ( phrase < 80 ) {
    srcX[0] = m_thePlayer->GetX()[0]+1.3;
    srcX[1] = m_thePlayer->GetX()[1]+2.0;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX[0] = m_thePlayer->GetX()[0]+0.3;
    destX[1] = m_thePlayer->GetX()[1];
    destX[2] = TABLEHEIGHT+NETHEIGHT;
  } else if ( phrase < 84 ) {
    srcX[0] = theBall.GetX()[0];
    srcX[1] = theBall.GetX()[1]-1.0;
    srcX[2] = theBall.GetX()[2]+0.1;
    destX[0] = m_comPlayer->GetX()[0]-0.3;
    destX[1] = m_comPlayer->GetX()[1];
    destX[2] = theBall.GetX()[2];
  } else if ( phrase < 88 ) {
    srcX[0] = m_thePlayer->GetX()[0]+1.3;
    srcX[1] = m_thePlayer->GetX()[1]+2.0;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX[0] = m_thePlayer->GetX()[0]+0.3;
    destX[1] = m_thePlayer->GetX()[1];
    destX[2] = TABLEHEIGHT+NETHEIGHT;
  } else if ( phrase < 92 ) {
    srcX[0] = theBall.GetX()[0];
    srcX[1] = theBall.GetX()[1]-1.0;
    srcX[2] = theBall.GetX()[2]+0.1;
    destX[0] = m_comPlayer->GetX()[0]-0.3;
    destX[1] = m_comPlayer->GetX()[1];
    destX[2] = theBall.GetX()[2];
  } else if ( phrase < 96 ) {
    srcX[0] = m_thePlayer->GetX()[0];
    srcX[1] = m_thePlayer->GetX()[1]-0.3;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX[0] = 0.0;
    destX[1] = TABLELENGTH/2;
    destX[2] = TABLEHEIGHT;
  } else if ( phrase < 100 ) {
    srcX[0] = -TABLELENGTH/2-0.2;
    srcX[1] = 0.0;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX[0] = theBall.GetX()[0];
    destX[1] = theBall.GetX()[1];
    destX[2] = TABLEHEIGHT+NETHEIGHT;
  } else if ( phrase < 104 ) {
    srcX[0] = m_comPlayer->GetX()[0];
    srcX[1] = m_comPlayer->GetX()[1]+0.5;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX = theBall.GetX();
  } else if ( phrase < 108 ) {
    srcX[0] = -TABLEWIDTH/2-1.5;
    srcX[1] = -TABLELENGTH;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX = theBall.GetX();
  } else if ( phrase < 112 ) {
    srcX[0] = 0.0;
    srcX[1] = TABLELENGTH/2+1.0;
    srcX[2] = TABLEHEIGHT+4.0;
    destX[0] = 0.0;
    destX[1] = -TABLELENGTH/2;
    destX[2] = TABLEHEIGHT;
  } else if ( phrase < 116 ) {
    srcX[0] = 0.0;
    srcX[1] = -TABLELENGTH*2;
    srcX[2] = 2.5;
    destX = theBall.GetX();
  } else if ( phrase < 120 ) {
    srcX[0] = 0.0;
    srcX[1] = TABLELENGTH*2;
    srcX[2] = 0.3;
    destX = theBall.GetX();
  } else if ( phrase < 124 ) {
    srcX[0] = 0.0;
    srcX[1] = -TABLELENGTH/2;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.3;
    destX[0] = 0.0;
    destX[1] = TABLELENGTH/2;
    destX[2] = TABLEHEIGHT+NETHEIGHT;
  } else if ( phrase < 128 ) {
    srcX[0] = TABLELENGTH/2+1.5;
    srcX[1] = TABLELENGTH/2+0.3;
    srcX[2] = TABLEHEIGHT;
    destX = theBall.GetX();
  } else if ( phrase < 136 ) {
    srcX[0] = -TABLEWIDTH/2-1.5;
    srcX[1] = -TABLELENGTH;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX = theBall.GetX();
  } else if ( phrase < 168 ) {
    if ( phrase%2 ) {
      srcX[0] = m_thePlayer->GetX()[0] - 1.0;
      srcX[1] = m_thePlayer->GetX()[1] + 1.0;
      srcX[2] = TABLEHEIGHT+NETHEIGHT+0.3;
      destX[0] = m_thePlayer->GetX()[0];
      destX[1] = m_thePlayer->GetX()[1];
      destX[2] = 1.0;
    } else {
      srcX[0] = m_comPlayer->GetX()[0] - 1.0;
      srcX[1] = m_comPlayer->GetX()[1] - 1.0;
      srcX[2] = TABLEHEIGHT+NETHEIGHT+0.3;
      destX[0] = m_comPlayer->GetX()[0];
      destX[1] = m_comPlayer->GetX()[1];
      destX[2] = 1.0;
    }
  } else if ( phrase < 200 ) {
    if ( phrase%2 ) {
      srcX[0] = m_thePlayer->GetX()[0] + 1.0;
      srcX[1] = m_thePlayer->GetX()[1] + 1.0;
      srcX[2] = TABLEHEIGHT+NETHEIGHT+0.3;
      destX[0] = m_thePlayer->GetX()[0];
      destX[1] = m_thePlayer->GetX()[1];
      destX[2] = 1.0;
    } else {
      srcX[0] = m_comPlayer->GetX()[0] + 1.0;
      srcX[1] = m_comPlayer->GetX()[1] - 1.0;
      srcX[2] = TABLEHEIGHT+NETHEIGHT+0.3;
      destX[0] = m_comPlayer->GetX()[0];
      destX[1] = m_comPlayer->GetX()[1];
      destX[2] = 1.0;
    }
  } else if ( phrase < 216 ) {
    srcX[0] = TABLELENGTH*1.5*cos(((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[1] = TABLELENGTH*1.5*sin(((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[2] = TABLEHEIGHT*2;
    destX[0] = 0.0;
    destX[1] = 0.0;
    destX[2] = TABLEHEIGHT;
  } else if ( phrase < 232 ) {
    srcX[0] = TABLELENGTH*1.5*cos(3.14159265-((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[1] = TABLELENGTH*1.5*sin(3.14159265-((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[2] = TABLEHEIGHT*2;
    destX[0] = 0.0;
    destX[1] = 0.0;
    destX[2] = TABLEHEIGHT;
  } else if ( phrase < 248 ) {
    srcX[0] = TABLELENGTH*1.5*cos(((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[1] = TABLELENGTH*1.5*sin(((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[2] = TABLEHEIGHT*2;
    destX[0] = 0.0;
    destX[1] = 0.0;
    destX[2] = TABLEHEIGHT;
  } else if ( phrase < 264 ) {
    srcX[0] = 0.0;
    srcX[1] = -TABLELENGTH/2;
    srcX[2] = 5.0;
    destX[0] = 0.0;
    destX[1] = TABLELENGTH/2;
    destX[2] = TABLEHEIGHT;
  } else if ( phrase < 280 ) {
    srcX[0] = m_thePlayer->GetX()[0];
    srcX[1] = m_thePlayer->GetX()[1]+1.5;
    srcX[2] = 1.5;
    destX[0] = m_thePlayer->GetX()[0];
    destX[1] = m_thePlayer->GetX()[1];
    destX[2] = 1.0;
  } else if ( phrase < 292 ) {
    srcX[0] = m_comPlayer->GetX()[0];
    srcX[1] = m_comPlayer->GetX()[1]-1.5;
    srcX[2] = 1.5;
    destX[0] = m_comPlayer->GetX()[0];
    destX[1] = m_comPlayer->GetX()[1];
    destX[2] = 1.0;
  } else if ( phrase < 304 ) {
    srcX[0] = TABLELENGTH*1.5*cos(((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[1] = TABLELENGTH*1.5*sin(((phrase-200)*PHRASELENGTH+mod)*3.14159265/720.0);
    srcX[2] = TABLEHEIGHT*2;
    destX[0] = 0.0;
    destX[1] = 0.0;
    destX[2] = TABLEHEIGHT;
  } else {
    srcX[0] = -TABLEWIDTH/2-1.5;
    srcX[1] = -TABLELENGTH;
    srcX[2] = TABLEHEIGHT+NETHEIGHT+0.5;
    destX = theBall.GetX();
  }

  return true;
}

/**
 * Calculate phrase number and mod from m_count. 
 * Basically, phrase number increments for each PHRASELENGTH count. 
 * 
 * @param phrase phrase number [out]
 * @param mod mod [out]
 */
void
Opening::GetPhrase( long &phrase, long &mod ) {
  if ( m_count <= 3200 ) {
    phrase = m_count/50;
    mod = m_count%50;
  } else if ( m_count <= 3292 ) {
    phrase = 64;
    mod = 0;
  } else {
    phrase = 67 + (long)((m_count-3292)/PHRASELENGTH);
    mod = (m_count-3292) - (long)((phrase-67)*PHRASELENGTH);
    if ( 67 + (long)((m_count-3293)/PHRASELENGTH) != phrase )
      mod = 0;
  }
}
