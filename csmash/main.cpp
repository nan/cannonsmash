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

void *LoadData( void *dum );

Ball theBall;

Sound theSound;

BaseView theView;
Player *thePlayer = NULL;
Player *comPlayer = NULL;
Event theEvent;

short csmash_port = CSMASH_PORT;
int theSocket = -1;
bool isComm = false;		// 通信対戦か
char serverName[256];

long timeAdj = 0;

bool isLighting	= true;
bool isFog	= true;
bool isTexture	= true;
bool isPolygon	= true;
bool isSimple	= false;
bool isWireFrame = true;

long wins	= 0;		// 勝ち抜き数
long gameLevel  = LEVEL_NORMAL;	// 強さ
long gameMode   = GAME_21PTS;	// ゲームの長さ

Control*      theControl = NULL;

long mode = MODE_OPENING;

long sndMode;			// あとで引数化

#if HAVE_LIBPTHREAD
pthread_mutex_t loadMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

#ifdef WIN32
#include "win32/getopt.h"

#ifndef WIN32CONSOLE
#include "win32/GetArgs.h"

int WINAPI WinMain_(HINSTANCE, HINSTANCE, LPSTR, int);

int WINAPI WinMain(HINSTANCE hI, HINSTANCE hP, LPSTR, int nCmdShow)
{
  return WinMain_(hI, hP, GetCommandLine(), nCmdShow);
}

int WINAPI WinMain_(HINSTANCE hInstance, HINSTANCE hPrevInstance,
      LPSTR lpCmdLine, int nCmdShow )
{
  int argc;
  char **argv;

  GetArgs(&argc, &argv, lpCmdLine);
#else
int main(int argc, char **argv) {

#endif
#else /* WIN32 */
int main(int argc, char** argv) {
#endif
  bool fullScreen = false;

#ifdef HAVE_LIBESD
  sndMode = SOUND_ESD;
#elif defined(WIN32)
  sndMode = SOUND_WIN32;
#elif 1
  sndMode = SOUND_NONE;
#endif

    int c;
    while(EOF != (c = getopt(argc, argv, "scphfSO"))) {
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
	case 'p':
	    // set the csmash_port
	    csmash_port = (short)atoi(optarg);
	    if (0 == csmash_port) csmash_port = CSMASH_PORT;
	    break;
	case 'f':
	    // Fullscreen mode
	    fullScreen = true;
	    break;
	case 'S':
	    // Simple mode
	    isSimple = true;
	    isTexture = false;
	    break;
	case 'O':
	    // OSS sound mode
	    sndMode = SOUND_OSS;
	    break;
	}
    }

    if (!isComm && argc > optind) {
      // Client mode
      isComm = true;
      mode = MODE_SELECT;
      strncpy(serverName, argv[optind], sizeof(serverName));
    }

#define PROBE_FILE "Parts/Fnormal/Fnormal-head01.dat"
  char *dataDir = NULL;

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

  if ( mode == MODE_OPENING && (access( SOUND_OPENING, F_OK ) != 0) ) {
    mode = MODE_TITLE;
  }

#if HAVE_LIBPTHREAD
  pthread_t ptid1, ptid2;
//  pthread_mutex_lock( &loadMutex );

//  pthread_create( &ptid1, NULL, PlayerView::LoadData, NULL );
  pthread_create( &ptid2, NULL, LoadData, NULL );

//  pthread_mutex_unlock( &loadMutex );
#else
  PlayerView::LoadData(NULL);
  LoadData( NULL );
#endif

  EndianCheck();

  glutInit(&argc, argv);

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

  theView.Init();
  theEvent.Init();

  glutReshapeFunc(BaseView::ReshapeFunc);

  if (fullScreen) {
    glutFullScreen();
    glutSetCursor( GLUT_CURSOR_NONE );
  }

  //glutWarpPointer( winWidth/2, winHeight/2 );
  glutWarpPointer( BaseView::GetWinWidth()/2, BaseView::GetWinHeight()/2 );

  glutMainLoop();
  return 0;
}

// 後でSoundもここから追い出して廃止する. 
void *
LoadData( void *dum ) {
#if HAVE_LIBPTHREAD
  pthread_mutex_lock( &loadMutex );
#endif

  PlayerView::LoadData(NULL);
  theSound.Init( sndMode );

#if HAVE_LIBPTHREAD
  pthread_mutex_unlock( &loadMutex );
#endif

#if HAVE_LIBPTHREAD
  pthread_detach(pthread_self());
  pthread_exit(NULL);
  return NULL;
#else
  return NULL;
#endif
}
