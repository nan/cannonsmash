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
#endif

void Draw(void);
void Keyboard( unsigned char key, int x, int y );
void KeyUp( unsigned char key, int x, int y );
void MouseMove( int x, int y );
void MouseButton( int button, int state, int x, int y );
#if HAVE_LIBPTHREAD & USETHREAD
void *ThreadTimer( void *dum );
#endif
void Timer();
void PlayInit( long player, long com );
void DemoInit();
void SelectInit();
void TitleInit();
void HowtoInit();
void *LoadData( void *dum );

Ball theBall;

Sound theSound;

BaseView theView;
Player *thePlayer = NULL;
Player *comPlayer = NULL;
Event theEvent;

bool isLighting	= true;
bool isFog	= true;
bool isTexture	= true;
bool isPolygon	= true;
long winWidth	= WINXSIZE;
long winHeight	= WINYSIZE;

long wins	= 0;		// 勝ち抜き数
long gameLevel  = LEVEL_NORMAL;	// 強さ

PlayerSelect* theSelect = NULL;
Title*        theTitle  = NULL;
Howto*        theHowto  = NULL;

long mode = MODE_TITLE;

char *dataDir = NULL;

#if HAVE_LIBPTHREAD & USETHREAD
pthread_mutex_t drawMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
#if HAVE_LIBPTHREAD
pthread_mutex_t loadMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
MotionData *motion_Fnormal;
MotionData *motion_Bnormal;
MotionData *motion_Fdrive;
MotionData *motion_Fcut;
MotionData *motion_Bcut;
MotionData *motion_Fpeck;
MotionData *motion_Bpeck;
MotionData *motion_Fsmash;

#ifdef WIN32
int WINAPI WinMain_(HINSTANCE, HINSTANCE, LPSTR, int);

int main( int argc, char *argv[] )
{
  HINSTANCE hInstance = GetModuleHandle(NULL);
  return WinMain_(hInstance, NULL, argv[0], SW_SHOW);
}

int WINAPI WinMain_(HINSTANCE hInstance, HINSTANCE hPrevInstance,
      LPSTR lpCmdLine, int nCmdShow )
{
  int argc = 1;
  char **argv;

  argv = (char**)malloc(sizeof(char*) * 2);
  argv[0] = (char*)malloc(32);
  argv[1] = NULL;
  strncpy(argv[0], "CannonSmash" ,32);

#else
int main(int argc, char** argv) {
#endif

#ifndef HAVE_LIBZ
  if ( access( "Motion/Fnormal1.bin", F_OK ) == 0 )
#else
  if ( access( "Motion/Fnormal1.bin.gz", F_OK ) == 0 )
#endif
    dataDir = ".";
  else {
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

  glutInit(&argc, argv);

#if HAVE_LIBPTHREAD
  pthread_t ptid;

  pthread_create( &ptid, NULL, LoadData, NULL );
#else
  LoadData( NULL );
#endif

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

#if HAVE_LIBPTHREAD & USETHREAD
  pthread_t tid;

  pthread_create( &tid, NULL, ThreadTimer, NULL );
#else
  glutIdleFunc( Timer );
#endif
  glutMotionFunc( MouseMove );
  glutPassiveMotionFunc( MouseMove );
  glutMouseFunc( MouseButton );

//  glutFullScreen();
//  glutSetCursor( GLUT_CURSOR_NONE );
  glutWarpPointer( winWidth/2, winHeight/2 );

  glutMainLoop();
  return 0;
}

void
Draw() {
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_lock( &drawMutex );
#endif
  theView.RedrawAll();
#if HAVE_LIBPTHREAD & USETHREAD
  pthread_mutex_unlock( &drawMutex );
#endif
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

#if HAVE_LIBPTHREAD & USETHREAD
void *
ThreadTimer( void *dum ) {
  while (1) 
    theEvent.IdleFunc();
}
#else
void
Timer() {
  theEvent.IdleFunc();
}
#endif

void
ClearObject() {
  if ( thePlayer && wins == 0 ) {
    delete thePlayer;
    thePlayer = NULL;
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
  ClearObject();

  if ( thePlayer == NULL ) {
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

    thePlayer->Init();
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

void *
LoadData( void *dum ) {
#if HAVE_LIBPTHREAD
  pthread_mutex_lock( &loadMutex );
#endif

  motion_Fnormal = new MotionData();
  motion_Bnormal = new MotionData();
  motion_Fdrive = new MotionData();
  motion_Fcut = new MotionData();
  motion_Bcut = new MotionData();
  motion_Fpeck = new MotionData();
  motion_Bpeck = new MotionData();
  motion_Fsmash = new MotionData();
#ifdef HAVE_LIBZ
  motion_Fnormal->LoadData( "Motion/Fnormal%d.bin.gz", 706 );
  motion_Bnormal->LoadData( "Motion/Bnormal%d.bin.gz", 706 );
  motion_Fdrive->LoadData( "Motion/Fdrive%d.bin.gz", 706 );
  motion_Fcut->LoadData( "Motion/Fcut%d.bin.gz", 706 );
  motion_Bcut->LoadData( "Motion/Bcut%d.bin.gz", 706 );
  motion_Fpeck->LoadData( "Motion/Fpeck%d.bin.gz", 706 );
  motion_Bpeck->LoadData( "Motion/Bpeck%d.bin.gz", 664 );
  motion_Fsmash->LoadData( "Motion/Fsmash%d.bin.gz", 664 );
#else
  motion_Fnormal->LoadData( "Motion/Fnormal%d.bin", 706 );
  motion_Bnormal->LoadData( "Motion/Bnormal%d.bin", 706 );
  motion_Fdrive->LoadData( "Motion/Fdrive%d.bin", 706 );
  motion_Fcut->LoadData( "Motion/Fcut%d.bin", 706 );
  motion_Bcut->LoadData( "Motion/Bcut%d.bin", 706 );
  motion_Fpeck->LoadData( "Motion/Fpeck%d.bin", 706 );
  motion_Bpeck->LoadData( "Motion/Bpeck%d.bin", 664 );
  motion_Fsmash->LoadData( "Motion/Fsmash%d.bin", 664 );
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
