/* $Id$ */

// Copyright (C) 2000  $B?@Fn(B $B5H9((B(Kanna Yoshihiro)
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

extern Ball theBall;
extern Player* thePlayer;
extern Player* comPlayer;
extern Event theEvent;
extern BaseView theView;
extern Control*      theControl;
extern long mode;

extern long timeAdj;

extern void Timer( int value );
struct timeb m_lastTime = {0, 0, 0, 0};	// $BD>A0$K(BTimerEvent$B$,8F$P$l$?$H$-$N;~9o(B

extern int theSocket;

extern bool isLighting;
extern bool isFog;
extern bool isTexture;
extern bool isPolygon;
extern long winWidth;
extern long winHeight;

extern long wins;

extern bool isComm;
extern char serverName[256];

long _perfCount;
long perfs;

void CopyPlayerData( struct PlayerData& dest, Player* src );
bool GetExternalData( ExternalData **ext, int sd, long side );

Event::Event() {
  m_KeyHistory[0] = 0;
  m_MouseXHistory[0] = winWidth/2;
  m_MouseYHistory[0] = winHeight/2;
  m_MouseBHistory[0] = 0;
  m_Histptr = 0;

  m_External = NULL;
}

Event::~Event() {
}

bool
Event::Init() {
  switch ( mode ){
  case MODE_PLAY:
    if (isComm) {
      theControl = MultiPlay::Create( 0, RAND(2) );
    } else {
      theControl = SoloPlay::Create( 0, RAND(2) );
    }
    break;
  case MODE_SELECT:
    theControl = PlayerSelect::Create();
    break;
  case MODE_TITLE:
    theControl = Title::Create();
    break;
  }

  theBall.Init();

  glutKeyboardFunc( Event::KeyboardFunc );

#if (GLUT_API_VERSION >= 4 || GLUT_XLIB_IMPLEMENTATION >= 13)
  glutKeyboardUpFunc( theEvent.KeyUpFunc );
#endif

  glutIdleFunc( Event::IdleFunc );
  glutMotionFunc( Event::MotionFunc );
  glutPassiveMotionFunc( Event::MotionFunc );
  glutMouseFunc( Event::ButtonFunc );

  return true;
}

void
Event::IdleFunc() {
  struct timeb tb;

  long diffsec, diffmsec;
  long diffcount = 0;
  bool reDraw = false;

#ifndef WIN32
  struct timeval tv;
  struct timezone tz;
#endif

#ifdef WIN32
  ftime( &tb );
#else
  gettimeofday( &tv, &tz );
  tb.time = tv.tv_sec;
  tb.millitm = tv.tv_usec/1000;
#endif

  if ( m_lastTime.time == 0 ){
    m_lastTime = tb;
    _perfCount = 0;
    perfs = 0;

    for ( int i = 0 ; i < MAX_HISTORY ; i++ ) {
      theEvent.m_MouseXHistory[i] = 0;
      theEvent.m_MouseYHistory[i] = 0;
      theEvent.m_MouseBHistory[i] = 0;
      theEvent.m_BacktrackBuffer[i].sec = m_lastTime.time;
      theEvent.m_BacktrackBuffer[i].count = m_lastTime.millitm;
      theEvent.m_BacktrackBuffer[i].theBall = theBall;
      CopyPlayerData( theEvent.m_BacktrackBuffer[i].thePlayer, thePlayer );
      CopyPlayerData( theEvent.m_BacktrackBuffer[i].comPlayer, comPlayer );
    }

    return;
  }

  diffsec = tb.time - m_lastTime.time;
  diffmsec = tb.millitm + diffsec*1000 - m_lastTime.millitm;

  diffcount = diffmsec / 10;
  if ( diffcount == 0 ) {
//    usleep( (10-diffmsec)*1000 );
    return;
  }

  perfs +=diffcount;
  _perfCount++;

  for ( int i = 0 ; i < diffcount ; i++ ) {
    reDraw |= theEvent.Move();
//    printf( "sec=%d count=%d status=%d\nx=%f y=%f z=%f spin = %f\n",
//	    m_lastTime.time, m_lastTime.millitm/10, theBall.GetStatus(),
//	    theBall.GetX(), theBall.GetY(), theBall.GetZ(), theBall.GetSpin() );
    theEvent.Record();

    m_lastTime.millitm += 10;
    if ( m_lastTime.millitm > 1000 ) {
      m_lastTime.time++;
      m_lastTime.millitm -= 1000;
    }
  }

  if ( theSocket >= 0 )
    theEvent.ReadData();

  if ( mode != MODE_TITLE )
    glutWarpPointer( theEvent.m_MouseXHistory[theEvent.m_Histptr],
		     theEvent.m_MouseYHistory[theEvent.m_Histptr] );

  if ( reDraw )
    glutPostRedisplay();
}

bool
Event::Move() {
  bool reDraw = false;
  long preMode = mode;

  reDraw |= theControl->Move( m_KeyHistory, m_MouseXHistory,
			      m_MouseYHistory, m_MouseBHistory, m_Histptr );

  if ( mode != preMode ){	// $B%b!<%IJQ99$"$j(B
    long p;

    switch ( mode ){
    case MODE_PLAY:
      p = ((PlayerSelect *)theControl)->GetPlayerNum();
      if (isComm) {
	theControl = MultiPlay::Create( p, 0 );
      } else {
	theControl = SoloPlay::Create( p, (p+wins+1)%PLAYERS );
      }
      break;
    case MODE_SELECT:
      theControl = PlayerSelect::Create();
      break;
    case MODE_TITLE:
      theControl = Title::Create();
      break;
    case MODE_HOWTO:
      theControl = Howto::Create();
      break;
    case MODE_TRAININGSELECT:
      theControl = TrainingSelect::Create();
      break;
    case MODE_TRAINING:
      p = ((PlayerSelect *)theControl)->GetPlayerNum();
      theControl = Training::Create( p, p );
      break;
    }

    preMode = mode;
    reDraw = true;
  }

  return reDraw;
}

void
Event::Record() {
  long x, y;
  long key;
  long btn;

  x = m_MouseXHistory[m_Histptr];
  y = m_MouseYHistory[m_Histptr];
  key = m_KeyHistory[m_Histptr];
  btn = m_MouseBHistory[m_Histptr];

  m_Histptr++;
  if ( m_Histptr == MAX_HISTORY )
    m_Histptr = 0;

  if ( mode == MODE_PLAY && theControl && !isComm &&
       ((SoloPlay *)theControl)->GetSmashPtr() >= 0 )
    return;

  m_KeyHistory[m_Histptr] = 0;
  if ( mode == MODE_TITLE ) {
    m_MouseXHistory[m_Histptr] = x;
    m_MouseYHistory[m_Histptr] = y;
  } else if ( mode == MODE_SELECT || mode == MODE_TRAININGSELECT ) {
    m_MouseXHistory[m_Histptr] = winWidth/2 + (x-winWidth/2)*15/16;
    m_MouseYHistory[m_Histptr] = y;
  } else {
    m_MouseXHistory[m_Histptr] = winWidth/2 + (x-winWidth/2)*15/16;
    m_MouseYHistory[m_Histptr] = winHeight/2 + (y-winHeight/2)*15/16;
  }
  m_MouseBHistory[m_Histptr] = btn;

  m_BacktrackBuffer[m_Histptr].sec = m_lastTime.time;
  m_BacktrackBuffer[m_Histptr].count = m_lastTime.millitm;
  while ( m_BacktrackBuffer[m_Histptr].count > 100 ) {
    m_BacktrackBuffer[m_Histptr].sec--;
    m_BacktrackBuffer[m_Histptr].count += 100;
  }
  m_BacktrackBuffer[m_Histptr].theBall = theBall;
  CopyPlayerData( m_BacktrackBuffer[m_Histptr].thePlayer, thePlayer );
  CopyPlayerData( m_BacktrackBuffer[m_Histptr].comPlayer, comPlayer );

  return;
}

void
Event::KeyboardFunc( unsigned char key, int x, int y ) {
#if (GLUT_API_VERSION < 4 && GLUT_XLIB_IMPLEMENTATION < 13)
  if ( key == 'Q' ) {
    printf( "Avg = %f\n", (double)perfs/_perfCount );
    exit(0);
  }
#endif
  theEvent.m_KeyHistory[theEvent.m_Histptr] = key;
}

void
Event::KeyUpFunc( unsigned char key, int x, int y ) {
  if ( key == 'Q' ) {
    printf( "Avg = %f\n", (double)perfs/_perfCount );
    exit(0);
  }
}

void
Event::MotionFunc( int x, int y ) {
  theEvent.m_MouseXHistory[theEvent.m_Histptr] = x;
  theEvent.m_MouseYHistory[theEvent.m_Histptr] = y;
}

void
Event::ButtonFunc( int button, int state, int x, int y ) {
  if ( state == GLUT_DOWN ){
    switch ( button ){
    case GLUT_LEFT_BUTTON:
      theEvent.m_MouseBHistory[theEvent.m_Histptr] |= BUTTON_LEFT;
      break;
    case GLUT_MIDDLE_BUTTON:
      theEvent.m_MouseBHistory[theEvent.m_Histptr] |= BUTTON_MIDDLE;
      break;
    case GLUT_RIGHT_BUTTON:
      theEvent.m_MouseBHistory[theEvent.m_Histptr] |= BUTTON_RIGHT;
      break;
    }
  } else {
    switch ( button ){
    case GLUT_LEFT_BUTTON:
      theEvent.m_MouseBHistory[theEvent.m_Histptr] &= ~BUTTON_LEFT;
      break;
    case GLUT_MIDDLE_BUTTON:
      theEvent.m_MouseBHistory[theEvent.m_Histptr] &= ~BUTTON_MIDDLE;
      break;
    case GLUT_RIGHT_BUTTON:
      theEvent.m_MouseBHistory[theEvent.m_Histptr] &= ~BUTTON_RIGHT;
      break;
    }
  }

  theEvent.m_MouseXHistory[theEvent.m_Histptr] = x;
  theEvent.m_MouseYHistory[theEvent.m_Histptr] = y;
}

void
CopyPlayerData( struct PlayerData& dest, Player* src ) {
  if ( !src )
    return;

  dest.playerType = src->GetPlayerType();
  dest.side = src->GetSide();
  dest.x = src->GetX();
  dest.y = src->GetY();
  dest.z = src->GetZ();
  dest.vx = src->GetVX();
  dest.vy = src->GetVY();
  dest.vz = src->GetVZ();
  dest.status = src->GetStatus();
  dest.swing = src->GetSwing();
  dest.swingType = src->GetSwingType();
  dest.swingError = src->GetSwingError();
  dest.targetX = src->GetTargetX();
  dest.targetY = src->GetTargetY();
  dest.eyeX = src->GetEyeX();
  dest.eyeY = src->GetEyeY();
  dest.eyeZ = src->GetEyeZ();
  dest.pow = src->GetPower();
  dest.spin = src->GetSpin();
  dest.stamina = src->GetStamina();
}

void
ReadTime( int sd, long *sec, char *count ) {
  char buf[256];
  long len = 0;
  long ctmp;

  while (1) {
    if ( (len+=recv( sd, buf+len, 5-len, 0 )) == 5 )
      break;
  }
  memcpy( sec, &buf[0], 4 );
  memcpy( count, &buf[4], 1 );

  ctmp = *count;
  ctmp -= timeAdj;

  while ( ctmp >= 100 ) {
    ctmp -= 100;
    (*sec)++;
  }
  while ( ctmp < 0 ) {
    ctmp += 100;
    (*sec)--;
  }
  *count = ctmp;
}

void SendTime( int sd ) {
  char v;
  long time, millitm;

  time = m_lastTime.time;
  millitm = m_lastTime.millitm/10;

  millitm += timeAdj;

  while ( millitm >= 100 ) {
    millitm -= 100;
    time++;
  }
  while ( millitm < 0 ) {
    millitm += 100;
    time--;
  }

  send( sd, (char *)&time, 4, 0 );
  v = (char)(millitm);
  send( sd, (char *)&v, 1, 0 );
}

bool
GetExternalData( ExternalData **ext, int sd, long side ) {
  char buf[256];
  ExternalData *extNow;
  long len;

  if ( recv( sd, buf, 2, 0 ) != 2 )
    return false;

#if 0
  extNow = new ExternalData();
  extNow->side = side;
  memset( extNow->data, 0, 256 );
  extNow->next = NULL;
#endif

  if ( !strncmp( buf, "PV", 2 ) ) {
#if 0
    extNow->dataType = DATA_PV;
    ReadTime( sd, &extNow->sec, &extNow->count );

    len = 0;
    while (1) {
      if ( (len+=recv( sd, extNow->data+len, 48-len, 0 )) == 48 )
	break;
    }
//    printf( "Get PV : %d %d\n", extNow->sec, extNow->count );
#else
    extNow = new ExternalPVData(side);
    extNow->Read( sd );
#endif
  } else if ( !strncmp( buf, "PS", 2 ) ) {
#if 0
    extNow->dataType = DATA_PS;
    ReadTime( sd, &extNow->sec, &extNow->count );

    len = 0;
    while (1) {
      if ( (len+=recv( sd, extNow->data+len, 20-len, 0 )) == 20 )
	break;
    }
#else
    extNow = new ExternalPSData(side);
    extNow->Read( sd );
#endif
  } else if ( !strncmp( buf, "BV", 2 ) ) {
#if 0
    extNow->dataType = DATA_BV;
    ReadTime( sd, &extNow->sec, &extNow->count );

    len = 0;
    while (1) {
      if ( (len+=recv( sd, extNow->data+len, 60-len, 0 )) == 60 )
	break;
    }
#else
    extNow = new ExternalBVData(side);
    extNow->Read( sd );
#endif
  } else
    return false;

  if ( !(*ext) ) {
    extNow->next = NULL;
    *ext = extNow;
  } else if ( (*ext)->sec > extNow->sec || 
	      ((*ext)->sec == extNow->sec && (*ext)->count > extNow->count) ) {
    extNow->next = *ext;
    *ext = extNow;
  } else {
    ExternalData *ext2 = *ext;

    while ( ext2->next ) {
      if ( ext2->next->sec > extNow->sec || 
	   (ext2->next->sec == extNow->sec &&
	    ext2->next->count > extNow->count ) ) {
	extNow->next = ext2->next;
	ext2->next = extNow;
	break;
      }
      ext2 = ext2->next;
    }
    if ( !ext2->next )
      ext2->next = extNow;
  }

  return true;
}

bool
Event::SendSwing( int sd, Player *player ) {
  if ( m_backtrack || mode != MODE_PLAY )
    return false;

  send( sd, "PS", 2, 0 );
  SendTime( sd );

  player->SendSwing( sd );

  return true;
}

bool
Event::SendPlayer( int sd, Player *player ) {
  if ( m_backtrack || mode != MODE_PLAY )
    return false;

  send( sd, "PV", 2, 0 );
  SendTime( sd );

  player->SendLocation( sd );

  return true;
}

bool
Event::SendBall( int sd ) {
  if ( m_backtrack || mode != MODE_PLAY )
    return false;

  send( sd, "BV", 2, 0 );
  SendTime( sd );

  theBall.Send( sd );

  return true;
}

void
Event::ClearObject() {
  if ( thePlayer && wins == 0 ) {
    delete thePlayer;
    thePlayer = NULL;
    if ( theSocket != -1 ) {
      send( theSocket, "QT", 2, 0 );
      closesocket( theSocket );
      theSocket = -1;
    }
  }
  if ( comPlayer ) {
    delete comPlayer;
    comPlayer = NULL;
  }
  if ( theControl ) {
    delete theControl;
    theControl = NULL;
  }
}

bool
Event::BackTrack( long Histptr ) {
  theBall = m_BacktrackBuffer[Histptr].theBall;
  thePlayer->Reset( &m_BacktrackBuffer[Histptr].thePlayer );
  comPlayer->Reset( &m_BacktrackBuffer[Histptr].comPlayer );

  m_Histptr = Histptr;
  return true;
}

void
Event::ReadData() {
  // $B>pJs<u?.(B
  fd_set rdfds;
  struct timeval to;

  while (1) {
    FD_ZERO( &rdfds );
    FD_SET( theSocket, &rdfds );

    to.tv_sec = to.tv_usec = 0;

    if ( select( theSocket+1, &rdfds, NULL, NULL, &to ) > 0 ) {
      GetExternalData( &m_External, theSocket, comPlayer->GetSide());
    } else
      break;
    //printf( "External\n" );
  }

  // externalData$B$N@hF,$^$G(Bbacktrack$B$9$k(B
  long btCount;
  ExternalData *externalOld;
  long btHistptr;
  while ( m_External ) {	// $B<N$F$k(B?
    //printf( "External2\n" );
    btCount = (m_lastTime.time-m_External->sec)*100 + 
      (m_lastTime.millitm/10-m_External->count);
    if ( btCount > MAX_HISTORY ) {
      externalOld = m_External;
      m_External = m_External->next;
      delete externalOld;
      continue;
    }
    if ( btCount <= MAX_HISTORY )
      break;
  }

  if ( m_External ) {
    if ( btCount >= 0 ) {
      btHistptr = m_Histptr - btCount;
      if ( btHistptr < 0 )
	btHistptr += MAX_HISTORY;

      long mtime = m_lastTime.millitm - btCount*10;
      while ( mtime < 0 ) {
	mtime += 1000;
	m_lastTime.time--;
      }
      m_lastTime.millitm = mtime;

      bool fTheBall, fThePlayer, fComPlayer;
      fTheBall = fThePlayer = fComPlayer = false;

      theBall = m_BacktrackBuffer[btHistptr].theBall;
      thePlayer->Reset( &m_BacktrackBuffer[btHistptr].thePlayer );
      comPlayer->Reset( &m_BacktrackBuffer[btHistptr].comPlayer );

      // $BE,MQ$9$k(B -> $B?J$a$k$r(BbtCount$B7+$jJV$9(B
      while (1) {
	if ( fTheBall )
	  theBall.Move();
	if ( fThePlayer ) {
	  thePlayer->Move( m_KeyHistory, m_MouseXHistory,
			   m_MouseYHistory, m_MouseBHistory, btHistptr );
	}
	if ( fComPlayer )
	  comPlayer->Move( NULL, NULL, NULL, NULL, 0 );

	while ( m_External &&
		m_External->sec == m_lastTime.time &&
		m_External->count == m_lastTime.millitm/10 ) {
	  Player *targetPlayer;
	  if ( m_External->side == thePlayer->GetSide() )
	    targetPlayer = thePlayer;
	  else if ( m_External->side == comPlayer->GetSide() )
	    targetPlayer = comPlayer;
	  else
	    exit(1);

#if 0&0
	  switch ( m_External->dataType ) {
	  case DATA_PV:
	    targetPlayer->Warp( m_External->data );
	    if ( targetPlayer == thePlayer )
	      fThePlayer = true;
	    else if ( targetPlayer == comPlayer )
	      fComPlayer = true;

//	    printf( "PV: sec = %d count = %d\n",
//		    m_External->sec, m_External->count);
//	    printf( "x=%f y=%f z=%f vx=%f vy=%f vz=%f\n",
//		    x, y, z, vx, vy, vz );
//	    fflush(0);
	    break;
	  case DATA_PS:
	    targetPlayer->ExternalSwing( m_External->data );
	    if ( targetPlayer == thePlayer )
	      fThePlayer = true;
	    else if ( targetPlayer == comPlayer )
	      fComPlayer = true;

//	    printf( "PS: sec = %d count = %d swing = %d\n",
//		    m_External->sec, m_External->count, targetPlayer->GetSwing());
//	    fflush(0);
	    break;
	  case DATA_BV:
	    theBall.Warp( m_External->data );
	    fTheBall = true;

//	    printf( "BV: sec = %d count = %d\n",
//		    m_External->sec, m_External->count);
//	    printf( "x=%f y=%f z=%f vx=%f vy=%f vz=%f\n",
//		    x, y, z, vx, vy, vz );
//	    fflush(0);
	    break;
	  }
#else
	  m_External->Apply( targetPlayer, fThePlayer, fComPlayer, fTheBall );
#endif
	  externalOld = m_External;
	  m_External = m_External->next;
	  delete externalOld;
	}

//	printf( "sec=%d count=%d status=%d\nx=%f y=%f z=%f spin = %f\n",
//		m_lastTime.time, m_lastTime.millitm/10,	theBall.GetStatus(),
//		theBall.GetX(), theBall.GetY(), theBall.GetZ(), theBall.GetSpin() );

	btHistptr++;
	if ( btHistptr == MAX_HISTORY )
	  btHistptr = 0;

	m_lastTime.millitm += 10;
	if ( m_lastTime.millitm >= 1000 ) {
	  m_lastTime.millitm -= 1000;
	  m_lastTime.time++;
	}

	btCount--;
	if ( btCount < 0 )
	  break;
      }
    }
  }
}
