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
#include "Howto.h"
#include "HowtoView.h"
#include "BaseView.h"
#include "Player.h"
#include "Ball.h"
#include "PenAttack.h"
#include "ShakeCut.h"
#include "Event.h"

extern BaseView theView;

extern long mode;

extern Player* thePlayer;
extern Player* comPlayer;

extern Ball theBall;

Howto::Howto() {
  m_View = NULL;
  m_mode = 0;
  m_count = 0;

  m_mouseX = 25;
  m_mouseY = 25;
  m_mouseB = 0;
}

Howto::~Howto() {
  if ( m_View ){
    theView.RemoveView( m_View );
    delete m_View;
  }
}

bool
Howto::Init() {
  m_View = new HowtoView();
  m_View->Init( this );

  theView.AddView( m_View );

  thePlayer = new PenAttack(1);
  comPlayer = new ShakeCut(-1);

  thePlayer->Init();
  comPlayer->Init();

  return true;
}

Howto*
Howto::Create() {
  Howto *newHowto;

  Event::ClearObject();

  newHowto = new Howto();
  newHowto->Init();

  return newHowto;
}

bool
Howto::Move( unsigned long *KeyHistory, long *MouseXHistory,
	     long *MouseYHistory, unsigned long *MouseBHistory,
	     int Histptr ) {
  if ( IsMove() ) {
    theBall.Move();
    thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
  }

  if ( KeyHistory[Histptr] == 27 ) {	// ESC
    mode = MODE_TITLE;
    return true;
  }

  switch ( m_mode ) {
  case 0:	// 移動方法解説
    if ( m_count == 0 ) {
      thePlayer->m_x = -TABLEWIDTH/4;
    }

    switch ( (m_count%400)/100 ) {
    case 0:
      thePlayer->m_vx = (double)(50-abs((m_count%100)-50))/25.0;
      thePlayer->m_vy = 0.0;
      m_mouseX = (m_count%100)/2;
      m_mouseY = 50;
      break;
    case 1:
      thePlayer->m_vx = 0.0;
      thePlayer->m_vy = -(double)(50-abs((m_count%100)-50))/25.0;
      m_mouseX = 50;
      m_mouseY = 50-(m_count%100)/2;
      break;
    case 2:
      thePlayer->m_vx = -(double)(50-abs((m_count%100)-50))/25.0;
      thePlayer->m_vy = 0.0;
      m_mouseX = 50-(m_count%100)/2;
      m_mouseY = 0;
      break;
    case 3:
      thePlayer->m_vx = 0.0;
      thePlayer->m_vy = (double)(50-abs((m_count%100)-50))/25.0;
      m_mouseX = 0;
      m_mouseY = (m_count%100)/2;
      break;
    }
    m_mouseB = 0;
    break;
  case 1:	// サーブ
    thePlayer->m_x = -TABLEWIDTH/2;
    thePlayer->m_y = -TABLELENGTH/2-0.5;
    thePlayer->m_vx = 0.0;
    thePlayer->m_vy = 0.0;
    m_mouseX = 25;
    m_mouseY = 25;
    if ( m_count%350 == 50 ) {
      theBall.Toss( thePlayer, 1 );
      thePlayer->StartSwing( m_count/350+1 );
    }
    if ( (m_count%350) >= 50 && (m_count%350) < 100 )
      m_mouseB = m_count/350+1;
    else
      m_mouseB = 0;
    break;
  case 2:	// target移動
    if ( m_count == 200 ) {
      thePlayer->m_targetX = -TABLEWIDTH/2*0.9;
      thePlayer->m_targetY = TABLELENGTH/12*5;
      m_mouseX = 0;
      m_mouseY = 0;
    } else if ( m_count == 300 ) {
      thePlayer->m_targetX = -TABLEWIDTH/2*0.9;
      thePlayer->m_targetY = TABLELENGTH/12*2;
      m_mouseX = 0;
      m_mouseY = 3;
    } else if ( m_count == 400 ) {
      thePlayer->m_targetX = TABLEWIDTH/2*0.9;
      thePlayer->m_targetY = TABLELENGTH/12*5;
      m_mouseX = 12;
      m_mouseY = 0;
    } else if ( m_count == 500 ) {
      thePlayer->m_targetX = TABLEWIDTH/2*0.9;
      thePlayer->m_targetY = TABLELENGTH/12*2;
      m_mouseX = 12;
      m_mouseY = 3;
    } else if ( m_count == 600 ) {
      thePlayer->m_targetX = -TABLEWIDTH/2*0.6;
      thePlayer->m_targetY = TABLELENGTH/12*3;
      m_mouseX = 2;
      m_mouseY = 2;
    } else if ( m_count == 700 ) {
      thePlayer->m_targetX = TABLEWIDTH/2*0.15;
      thePlayer->m_targetY = TABLELENGTH/12*4;
      m_mouseX = 7;
      m_mouseY = 1;
    } else if ( m_count == 800 ) {
      thePlayer->m_targetX = TABLEWIDTH/2*0.6;
      thePlayer->m_targetY = TABLELENGTH/12*4;
      m_mouseX = 10;
      m_mouseY = 1;
    } else if ( m_count == 900 ) {
      thePlayer->m_targetX = TABLEWIDTH/2*0.75;
      thePlayer->m_targetY = TABLELENGTH/12*3;
      m_mouseX = 11;
      m_mouseY = 2;
    } else if ( m_count == 1150 ) {
      theBall.Toss( thePlayer, 1 );
      thePlayer->StartSwing( 1 );
    } else if ( m_count == 1450 ) {
      thePlayer->m_targetX = -TABLEWIDTH/2*0.9;
      thePlayer->m_targetY = TABLELENGTH/12*5;
      m_mouseX = 0;
      m_mouseY = 0;
    } else if ( m_count == 1550 ) {
      theBall.Toss( thePlayer, 1 );
      thePlayer->StartSwing( 1 );
    }

    break;
  case 3:	// 打球1
    if ( m_count == 0 ) {
      thePlayer->m_targetX = TABLEWIDTH/2*0.3;
      thePlayer->m_targetY = TABLELENGTH/16*5;
      comPlayer->m_x = 0.221131;
      comPlayer->m_y = 1.855788;
      m_mouseX = 0;
      m_mouseY = 0;
      m_mouseB = 0;
    } else if ( m_count == 50 ) {
      theBall.Toss( thePlayer, 1 );
      thePlayer->StartSwing( 3 );
      m_mouseB = 1;
    } else if ( m_count > 50 && m_count < 100 ) {
      m_mouseB = 1;
    } else if ( m_count == 180 ) {
      comPlayer->m_targetX = -TABLEWIDTH/2*0.15;
      comPlayer->Swing( 1 );
      comPlayer->m_pow = 10;
      m_mouseB = 0;
    } else {
      m_mouseB = 0;
    }
    break;
  case 4:	// 打球2
    if ( m_count == 570 ) {
//      thePlayer->Swing( 1 );
      thePlayer->Swing( 3 );
      thePlayer->m_pow = 10;
      m_mouseB = 1;
    } else if ( m_count > 570 && m_count < 700 ) {
      m_mouseB = 1;
    } else if ( m_count == 1000 ) {
      thePlayer->m_x = 0.2;
      theBall.Warp( 0.243705, 0.673854, 1.017764,
		    -1.042663, -4.457266, 0.108404, -1.0, 2 );
      thePlayer->m_x = 0.0;
      m_mouseB = 0;
    } else if ( m_count == 1570 ) {
      thePlayer->Swing( 1 );
      thePlayer->m_pow = 10;
      m_mouseB = 3;
    } else if ( m_count > 1570 && m_count < 1700 ) {
      m_mouseB = 3;
    } else {
      m_mouseB = 0;
    }
  }

  m_count++;

  switch ( m_mode ) {
  case 0:
    if ( m_count >= 800 ) {
      m_mode = 1;
      m_count = 0;
    }
    break;
  case 1:
    if ( m_count >= 1050 ) {
      m_mode = 2;
      m_count = 0;
    }
    break;
  case 2:
    if ( m_count >= 1900 ) {
      m_mode = 3;
      m_count = 0;
    }
    break;
  case 3:
    if ( m_count > 440 ) {
      m_mode = 4;
      m_count = 0;
    }
    break;
  case 4:
    if ( m_count > 2200 ) {
      mode = MODE_TITLE;
      return true;
    }
  }

  return true;
}

bool
Howto::LookAt( double &srcX, double &srcY, double &srcZ,
	       double &destX, double &destY, double &destZ ) {
  srcX = thePlayer->GetX() + thePlayer->GetEyeX();
  srcY = thePlayer->GetY() + thePlayer->GetEyeY();
  srcZ = thePlayer->GetZ() + thePlayer->GetEyeZ();

  switch ( m_mode ) {
  case 2:
    if ( m_count < 100 ) {
      srcX *= (double)(100-m_count)/100;
      srcY += (TABLELENGTH/3 - srcY)*(double)(m_count)/100;
      srcZ += (double)(m_count)*2.0/100;
    } else if ( m_count < 1000 ) {
      srcX = 0.0;
      srcY = TABLELENGTH/3;
      srcZ += 2.0;
    } else if ( m_count < 1100 ) {
      srcX *= (double)(m_count-1000)/100;
      srcY += (TABLELENGTH/3 - srcY)*(double)(100-(m_count-1000))/100;
      srcZ += (double)(100-(m_count-1000))*2.0/100;
    }
    break;
  }

  return true;
}

/* ball, playerを移動させるか否か */
bool
Howto::IsMove() {
  switch ( m_mode ) {
  case 0:
  case 1:
  case 2:
    return true;
  case 3:
    if ( m_count > 215 )
      return false;
    else
      return true;
  case 4:
    if ( m_count < 570 ) {
      if ( m_count%10 == 0 )
	return true;
      else
	return false;
    } else if ( m_count < 670 ) {
      return false;
    } else if ( m_count < 1000 ) {
      return true;
    } else if ( m_count < 1570 ) {
      if ( m_count%10 == 0 )
	return true;
      else
	return false;
    } else if ( m_count < 1670 ) {
      return false;
    } else 
      return true;
  }

  return true;
}
