/**
 * @file
 * @brief Implementation of Howto class. 
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
#include "Howto.h"
#include "HowtoView.h"
#include "BaseView.h"
#include "Player.h"
#include "Ball.h"
#include "PenAttack.h"
#include "ShakeCut.h"
#include "Event.h"

extern long mode;

extern Ball theBall;

/**
 * Default constructor. 
 * initialize member variable. 
 */
Howto::Howto() {
  m_View = NULL;
  m_mode = 0;
  m_count = 0;

  m_mouseX = 25;
  m_mouseY = 25;
  m_mouseB = 0;
}

/**
 * Destructor. 
 * Detatch m_View and delete it. 
 */
Howto::~Howto() {
  if ( m_View ){
    BaseView::TheView()->RemoveView( m_View );
    delete m_View;
  }
}

/**
 * Initialize member variables. 
 * Create HowtoView, Player objects and initialize them. 
 * 
 * @return returns true if succeeds. 
 */
bool
Howto::Init() {
  m_View = (HowtoView *)View::CreateView( VIEW_HOWTO );
  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  m_thePlayer = new PenAttack(1);
  m_comPlayer = new ShakeCut(-1);

  m_thePlayer->Init();
  m_comPlayer->Init();

  return true;
}

/**
 * Create Howto and initialize it. 
 * This method clean up existing Control object and related object, then
 * create Howto object and initialize it. 
 */
void
Howto::Create() {
  Control::ClearControl();

  m_theControl = new Howto();
  m_theControl->Init();
}

/**
 * Move objects managed by Howto. 
 * This method calls Move method of  theBall, thePlayer, comPlayer
 * to move them. And referring the duration of the time, this method
 * changes m_mode and m_count. Additionally, this method manages the
 * location and status of the mouse shown in the left bottom of the screen. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
Howto::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
	     long *MouseYHistory, unsigned long *MouseBHistory,
	     int Histptr ) {
  if ( IsMove() ) {
    theBall.Move();
    m_thePlayer->Move( NULL, NULL, NULL, NULL, 0 );
    m_comPlayer->Move( NULL, NULL, NULL, NULL, 0 );
  }

  if ( KeyHistory[Histptr].unicode == SDLK_ESCAPE ) {
    mode = MODE_TITLE;
    return true;
  }

  switch ( m_mode ) {
  case 0:	// How to move
    if ( m_count == 0 ) {
      m_thePlayer->m_x[0] = -TABLEWIDTH/4;
    }

    switch ( (m_count%400)/100 ) {
    case 0:
      m_thePlayer->m_v[0] = (double)(50-abs((m_count%100)-50))/25.0;
      m_thePlayer->m_v[1] = 0.0;
      m_mouseX = (m_count%100)/2;
      m_mouseY = 50;
      break;
    case 1:
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = -(double)(50-abs((m_count%100)-50))/25.0;
      m_mouseX = 50;
      m_mouseY = 50-(m_count%100)/2;
      break;
    case 2:
      m_thePlayer->m_v[0] = -(double)(50-abs((m_count%100)-50))/25.0;
      m_thePlayer->m_v[1] = 0.0;
      m_mouseX = 50-(m_count%100)/2;
      m_mouseY = 0;
      break;
    case 3:
      m_thePlayer->m_v[0] = 0.0;
      m_thePlayer->m_v[1] = (double)(50-abs((m_count%100)-50))/25.0;
      m_mouseX = 0;
      m_mouseY = (m_count%100)/2;
      break;
    }
    m_mouseB = 0;
    break;
  case 1:	// Serve
    m_thePlayer->m_x[0] = -TABLEWIDTH/2;
    m_thePlayer->m_x[1] = -TABLELENGTH/2-0.5;
    m_thePlayer->m_v[0] = 0.0;
    m_thePlayer->m_v[1] = 0.0;
    m_mouseX = 25;
    m_mouseY = 25;
    if ( m_count%350 == 50 ) {
      theBall.Toss( m_thePlayer, 1 );
      m_thePlayer->StartServe( m_count/350+1 );
    }
    if ( (m_count%350) >= 50 && (m_count%350) < 100 )
      m_mouseB = m_count/350+1;
    else
      m_mouseB = 0;
    break;
  case 2:	// How to move target
    if ( m_count == 200 ) {
      m_thePlayer->m_target[0] = -TABLEWIDTH/2*0.9;
      m_thePlayer->m_target[1] = TABLELENGTH/12*5;
      m_mouseX = 0;
      m_mouseY = 0;
    } else if ( m_count == 300 ) {
      m_thePlayer->m_target[0] = -TABLEWIDTH/2*0.9;
      m_thePlayer->m_target[1] = TABLELENGTH/12*2;
      m_mouseX = 0;
      m_mouseY = 3;
    } else if ( m_count == 400 ) {
      m_thePlayer->m_target[0] = TABLEWIDTH/2*0.9;
      m_thePlayer->m_target[1] = TABLELENGTH/12*5;
      m_mouseX = 12;
      m_mouseY = 0;
    } else if ( m_count == 500 ) {
      m_thePlayer->m_target[0] = TABLEWIDTH/2*0.9;
      m_thePlayer->m_target[1] = TABLELENGTH/12*2;
      m_mouseX = 12;
      m_mouseY = 3;
    } else if ( m_count == 600 ) {
      m_thePlayer->m_target[0] = -TABLEWIDTH/2*0.6;
      m_thePlayer->m_target[1] = TABLELENGTH/12*3;
      m_mouseX = 2;
      m_mouseY = 2;
    } else if ( m_count == 700 ) {
      m_thePlayer->m_target[0] = TABLEWIDTH/2*0.15;
      m_thePlayer->m_target[1] = TABLELENGTH/12*4;
      m_mouseX = 7;
      m_mouseY = 1;
    } else if ( m_count == 800 ) {
      m_thePlayer->m_target[0] = TABLEWIDTH/2*0.6;
      m_thePlayer->m_target[1] = TABLELENGTH/12*4;
      m_mouseX = 10;
      m_mouseY = 1;
    } else if ( m_count == 900 ) {
      m_thePlayer->m_target[0] = TABLEWIDTH/2*0.75;
      m_thePlayer->m_target[1] = TABLELENGTH/12*3;
      m_mouseX = 11;
      m_mouseY = 2;
    } else if ( m_count == 1150 ) {
      theBall.Toss( m_thePlayer, 1 );
      m_thePlayer->StartServe(1);
    } else if ( m_count == 1450 ) {
      m_thePlayer->m_target[0] = -TABLEWIDTH/2*0.9;
      m_thePlayer->m_target[1] = TABLELENGTH/12*5;
      m_mouseX = 0;
      m_mouseY = 0;
    } else if ( m_count == 1550 ) {
      theBall.Toss( m_thePlayer, 1 );
      m_thePlayer->StartServe(1);
    }

    break;
  case 3:	// Hit ball (1)
    if ( m_count == 0 ) {
      m_thePlayer->m_target[0] = TABLEWIDTH/2*0.3;
      m_thePlayer->m_target[1] = TABLELENGTH/16*5;
      m_thePlayer->m_x[0] = -TABLEWIDTH/2+0.1;

      m_comPlayer->m_x[0] = 0.221131;
      m_comPlayer->m_x[1] = 1.855788;
      m_mouseX = 0;
      m_mouseY = 0;
      m_mouseB = 0;
    } else if ( m_count == 50 ) {
      theBall.Toss( m_thePlayer, 1 );
      m_thePlayer->StartServe(3);
      m_mouseB = 1;
    } else if ( m_count > 50 && m_count < 100 ) {
      m_mouseB = 1;
    } else if ( m_count == 180 ) {
      m_comPlayer->m_target[0] = -TABLEWIDTH/2*0.15;
      m_comPlayer->Swing( 1 );
      m_comPlayer->m_pow = 10;
      m_mouseB = 0;
    } else {
      m_mouseB = 0;
    }
    break;
  case 4:	// Hit ball (2)
  case 5:
    if ( m_count > 0 && m_count < 500 ) {
      m_thePlayer->m_x[0] = -TABLEWIDTH/2+0.1 +
	(500-m_count)/1000.0*sin( m_count*2*3.141592/500 );
    } else if ( m_count == 570 ) {
      m_thePlayer->Swing( 3 );
      m_thePlayer->m_pow = 10;
      m_mouseB = 1;
    } else if ( m_count > 570 && m_count < 700 ) {
      m_mouseB = 1;
    } else if ( m_count == 1000 ) {
      m_thePlayer->m_x[0] = 0.2;
      theBall.Warp( vector3d((const double[]){0.243705, 0.673854, 1.017764}), 
		    vector3d((const double[]){-1.042663, -4.457266, 0.108404}),
		    vector2d((const double[]){0.0, -1.0}), 2 );
      m_thePlayer->m_x[0] = -0.1;
      m_mouseB = 0;
    } else if ( m_count > 1000 && m_count < 1500 ) {
      m_thePlayer->m_x[0] = -0.1 +
	(1500-m_count)/1000.0*sin( (m_count-1000)*2*3.141592/500 );
    } else if ( m_count == 1570 ) {
      m_thePlayer->Swing( 1 );
      m_thePlayer->m_pow = 10;
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
    if ( m_count > 500 ) {
      m_mode = 5;
    }
  case 5:
    if ( m_count > 2200 ) {
      mode = MODE_TITLE;
      return true;
    }
  }

  return true;
}

/**
 * Initialize the location and direction of the camera. 
 * 
 * @param srcX location of the camera
 * @param destX direction of the camera
 * @return returns true if succeeds. 
 */
bool
Howto::LookAt( vector3d &srcX, vector3d &destX ) {
  srcX = m_thePlayer->GetX() + m_thePlayer->GetEye();

  switch ( m_mode ) {
  case 2:
    if ( m_count < 100 ) {
      srcX[0] *= (double)(100-m_count)/100;
      srcX[1] += (TABLELENGTH/3 - srcX[1])*(double)(m_count)/100;
      srcX[2] += (double)(m_count)*2.0/100;
    } else if ( m_count < 1000 ) {
      srcX[0] = 0.0;
      srcX[1] = TABLELENGTH/3;
      srcX[2] += 2.0;
    } else if ( m_count < 1100 ) {
      srcX[0] *= (double)(m_count-1000)/100;
      srcX[1] += (TABLELENGTH/3 - srcX[1])*(double)(100-(m_count-1000))/100;
      srcX[2] += (double)(100-(m_count-1000))*2.0/100;
    }
    break;
  }

  return true;
}

/**
 * Check wheather ball and player should be moved or not. 
 * This method is used for slow motion, etc. 
 * 
 * @return returns false if objects should not be moved. 
 */
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
  case 5:
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
