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

#ifdef WIN32
#include <io.h>
#include <direct.h>
#define F_OK 0 /* if exist */
#else
#define closesocket(FD) close(FD)
#endif

void Draw(void);
void Keyboard( unsigned char key, int x, int y );
void KeyUp( unsigned char key, int x, int y );
void MouseMove( int x, int y );
void MouseButton( int button, int state, int x, int y );
void Timer();
void Reshape( int width, int height );

void PlayInit( long player, long com );
void DemoInit();
void SelectInit();
void TitleInit();
void HowtoInit();
void TrainingInit( long player, long com );
void *LoadData( void *dum );

Ball theBall;

Sound theSound;

BaseView theView;
Player *thePlayer = NULL;
Player *comPlayer = NULL;
Event theEvent;

int theSocket = -1;
bool isComm = false;		// 通信対戦か
char serverName[256];

bool fullScreen = false;

long timeAdj = 0;

bool isLighting	= true;
bool isFog	= true;
bool isTexture	= true;
bool isPolygon	= true;
long winWidth	= WINXSIZE;
long winHeight	= WINYSIZE;

long wins	= 0;		// 勝ち抜き数
long gameLevel  = LEVEL_NORMAL;	// 強さ
long gameMode   = GAME_21PTS;	// ゲームの長さ

PlayerSelect* theSelect = NULL;
Title*        theTitle  = NULL;
Howto*        theHowto  = NULL;

long mode = MODE_TITLE;

char *dataDir = NULL;

#if HAVE_LIBPTHREAD
pthread_mutex_t loadMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#if 0
MotionData *motion_Fnormal;
MotionData *motion_Bnormal;
MotionData *motion_Fdrive;
MotionData *motion_Fcut;
MotionData *motion_Bcut;
MotionData *motion_Fpeck;
MotionData *motion_Bpeck;
MotionData *motion_Fsmash;
#else
partsmotion *motion_Fnormal = NULL;
partsmotion *motion_Bnormal = NULL;
partsmotion *motion_Fdrive = NULL;
partsmotion *motion_Fcut = NULL;
partsmotion *motion_Bcut = NULL;
partsmotion *motion_Fpeck = NULL;
partsmotion *motion_Bpeck = NULL;
partsmotion *motion_Fsmash = NULL;
#endif

#ifdef WIN32
#include "win32/GetArgs.h"
#include "win32/getopt.h"

int WINAPI WinMain_(HINSTANCE, HINSTANCE, LPSTR, int);

int main( int argc, char *argv[] )
{
  HINSTANCE hInstance = GetModuleHandle(NULL);
  return WinMain_(hInstance, NULL, GetCommandLine(), SW_SHOW);
}

int WINAPI WinMain_(HINSTANCE hInstance, HINSTANCE hPrevInstance,
      LPSTR lpCmdLine, int nCmdShow )
{
  int argc;
  char **argv;

  GetArgs(&argc, &argv, lpCmdLine);

#else
int main(int argc, char** argv) {
#endif

    int c;
    while(EOF != (c = getopt(argc, argv, "schf"))) {
        switch (c) {
        case 'h':
	    // brief help
	    printf("csmash [-s] [ip-address]");
	    return 0;
	case 's':
	    // Server mode
	    isComm = true;
	    mode = MODE_SELECT;
	    serverName[0] = '\0';
	    break;
	case 'c':
	    // Client mode
	    isComm = true;
	    mode = MODE_SELECT;
	    serverName[0] = 1;	// :-p
	    break;
	case 'f':
	    // Fullscreen mode
	    fullScreen = true;
	}
    }

    if (!isComm && argc > optind) {
      // Client mode
      isComm = true;
      mode = MODE_SELECT;
      strncpy(serverName, argv[optind], sizeof(serverName));
    }

#if 0
#define PROBE_FILE "Motion/Fnormal1.bin"
#else
#define PROBE_FILE "Parts/Fnormal/Fnormal-head01.dat"
#endif

  if ( (access( PROBE_FILE, F_OK ) == 0) ) {
    dataDir = ".";
  } else {
#ifdef CANNONSMASH_DATADIR
    dataDir = CANNONSMASH_DATADIR;
#else
    fprintf( stderr, "No datafile directory.\n" );
    exit(1);
#endif
  }

  if ( chdir( dataDir ) == -1 ) {
    fprintf( stderr, "No datafile directory.\n" );
    exit(1);
  }

  printf( dataDir );

#if HAVE_LIBPTHREAD
  pthread_t ptid;

  pthread_create( &ptid, NULL, LoadData, NULL );
#else
  LoadData( NULL );
#endif

  EndianCheck();
  glutInit(&argc, argv);
  theView.Init();

  struct timeb tb;
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

  srand(tb.millitm);

  switch ( mode ){
  case MODE_PLAY:
    PlayInit( 0, RAND(2) );
    break;
  case MODE_SELECT:
    SelectInit();
    break;
  case MODE_DEMO:
    DemoInit();
    break;
  case MODE_TITLE:
    TitleInit();
    break;
  }
  theBall.Init();

  glutDisplayFunc(Draw);
  glutKeyboardFunc( Keyboard );

#if (GLUT_API_VERSION >= 4 || GLUT_XLIB_IMPLEMENTATION >= 13)
  glutKeyboardUpFunc( KeyUp );
#endif

  glutIdleFunc( Timer );
  glutMotionFunc( MouseMove );
  glutPassiveMotionFunc( MouseMove );
  glutMouseFunc( MouseButton );
  glutReshapeFunc(Reshape);

  if (fullScreen) {
    glutFullScreen();
    glutSetCursor( GLUT_CURSOR_NONE );
  }

  glutWarpPointer( winWidth/2, winHeight/2 );

  glutMainLoop();
  return 0;
}

void
Draw() {
  theView.RedrawAll();
}

void
Keyboard( unsigned char key, int x, int y ) {
  theEvent.KeyboardFunc( key, x, y );
}

void
KeyUp( unsigned char key, int x, int y ) {
  theEvent.KeyUpFunc( key, x, y );
}

void
MouseMove( int x, int y ) {
  theEvent.MotionFunc( x, y );
}

void
MouseButton( int button, int state, int x, int y ) {
  theEvent.ButtonFunc( button, state, x, y );
}

void
Timer() {
  theEvent.IdleFunc();
}

void
Reshape( int width, int height ) {
  winWidth = width;
  winHeight = height;

  glViewport( 0, 0, winWidth, winHeight );
}

void
ClearObject() {
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
  if ( theSelect ) {
    delete theSelect;
    theSelect = NULL;
  }
  if ( theTitle ) {
    delete theTitle;
    theTitle = NULL;
  }
  if ( theHowto ) {
    delete theHowto;
    theHowto = NULL;
  }
}

void
PlayInit( long player, long com ) {
  long side;

  ClearObject();

  if (isComm) {
    if ( !(serverName[0]) )
      side = 1;		// server側
    else
      side = -1;	// client側

    if ( thePlayer == NULL ) {
      switch (player) {
      case 0:
	thePlayer = new PenAttack(side);
	break;
      case 1:
	thePlayer = new ShakeCut(side);
	break;
      case 2:
	thePlayer = new PenDrive(side);
	break;
      default:
	printf( "no player %ld\n", player );
	exit(1);
      }
    }

    if ( side == 1 ) {
      StartServer();
    } else {
      StartClient();
    }
  } else {
    switch (player) {
    case 0:
      thePlayer = new PenAttack(1);
      break;
    case 1:
      thePlayer = new ShakeCut(1);
      break;
    case 2:
      thePlayer = new PenDrive(1);
      break;
    default:
      printf( "no player %ld\n", player );
    }

    switch(com) {
    case 0:
      comPlayer = new ComPenAttack(-1);
      break;
    case 1:
      comPlayer = new ComShakeCut(-1);
      break;
    case 2:
      comPlayer = new ComPenDrive(-1);
      break;
    default:
      comPlayer = new ComPenAttack(-1);
      break;
    }
  }

  thePlayer->Init();
  comPlayer->Init();

  glutSetCursor( GLUT_CURSOR_NONE );
}

void
DemoInit() {
  ClearObject();

  // 後でSelectに移動?
  switch  ( RAND(3) ) {
  case 0:
    thePlayer = new ComPenAttack(1);
    break;
  case 1:
    thePlayer = new ComShakeCut(1);
    break;
  case 2:
    thePlayer = new ComPenDrive(1);
    break;
  }

  switch ( RAND(3) ) {
  case 0:
    comPlayer = new ComPenAttack(-1);
    break;
  case 1:
    comPlayer = new ComShakeCut(-1);
    break;
  case 2:
    comPlayer = new ComPenDrive(-1);
    break;
  }

  thePlayer->Init();
  comPlayer->Init();

  glutSetCursor( GLUT_CURSOR_NONE );
}

void
SelectInit() {
  ClearObject();

  theSelect = new PlayerSelect();

  theSelect->Init();

  glutSetCursor( GLUT_CURSOR_NONE );
}

void
TitleInit() {
  ClearObject();

  theTitle = new Title();

  theTitle->Init();

  // 後でSelectに移動?
  switch ( RAND(3) ) {
  case 0:
    thePlayer = new ComPenAttack(1);
    break;
  case 1:
    thePlayer = new ComShakeCut(1);
    break;
  case 2:
    thePlayer = new ComPenDrive(1);
    break;
  }

  switch ( RAND(3) ) {
  case 0:
    comPlayer = new ComPenAttack(-1);
    break;
  case 1:
    comPlayer = new ComShakeCut(-1);
    break;
  case 2:
    comPlayer = new ComPenDrive(-1);
    break;
  }

  thePlayer->Init();
  comPlayer->Init();

  glutSetCursor( GLUT_CURSOR_INHERIT );
}

void
HowtoInit() {
  ClearObject();

  theHowto = new Howto();

  theHowto->Init();

  // 後でSelectに移動?
  thePlayer = new PenAttack(1);
  comPlayer = new ShakeCut(-1);

  thePlayer->Init();
  comPlayer->Init();
}

void
TrainingInit( long player, long com ) {
  long side;

  ClearObject();

  switch (player) {
  case 0:
    thePlayer = new TrainingPenAttack(1);
    break;
  case 1:
    thePlayer = new ShakeCut(1);
    break;
  case 2:
    thePlayer = new TrainingPenDrive(1);
    break;
  default:
    printf( "no player %ld\n", player );
  }

  switch(com) {
  case 0:
    comPlayer = new ComTrainingPenAttack(-1);
    break;
  case 1:
    comPlayer = new ComShakeCut(-1);
    break;
  case 2:
    comPlayer = new ComTrainingPenDrive(-1);
    break;
  default:
    comPlayer = new ComPenAttack(-1);
    break;
  }

  thePlayer->Init();
  comPlayer->Init();

  glutSetCursor( GLUT_CURSOR_NONE );
}

void *
LoadData( void *dum ) {
#if HAVE_LIBPTHREAD
  pthread_mutex_lock( &loadMutex );
#endif

#if 0
  motion_Fnormal = new MotionData();
  motion_Bnormal = new MotionData();
  motion_Fdrive = new MotionData();
  motion_Fcut = new MotionData();
  motion_Bcut = new MotionData();
  motion_Fpeck = new MotionData();
  motion_Bpeck = new MotionData();
  motion_Fsmash = new MotionData();
  motion_Fnormal->LoadData( "Motion/Fnormal%d.bin", 706 );
  motion_Bnormal->LoadData( "Motion/Bnormal%d.bin", 706 );
  motion_Fdrive->LoadData( "Motion/Fdrive%d.bin", 706 );
  motion_Fcut->LoadData( "Motion/Fcut%d.bin", 706 );
  motion_Bcut->LoadData( "Motion/Bcut%d.bin", 706 );
  motion_Fpeck->LoadData( "Motion/Fpeck%d.bin", 706 );
  motion_Bpeck->LoadData( "Motion/Bpeck%d.bin", 664 );
  motion_Fsmash->LoadData( "Motion/Fsmash%d.bin", 664 );
#else
  motion_Fnormal = new partsmotion("Parts/Fnormal/Fnormal");
  motion_Bnormal = new partsmotion("Parts/Bnormal/Bnormal");
  motion_Fdrive = new partsmotion("Parts/Fdrive/Fdrive");
  motion_Fcut = new partsmotion("Parts/Fcut/Fcut");
  motion_Bcut = new partsmotion("Parts/Bcut/Bcut");
  motion_Fpeck = new partsmotion("Parts/Fpeck/Fpeck");
  motion_Bpeck = new partsmotion("Parts/Bpeck/Bpeck");
  motion_Fsmash = new partsmotion("Parts/Fsmash/Fsmash");
#endif
  theSound.Init();

#if HAVE_LIBPTHREAD
  pthread_mutex_unlock( &loadMutex );
  pthread_detach(pthread_self());
  pthread_exit(NULL);
  return NULL;
#else
  return NULL;
#endif
}
