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
#include "Ball.h"
#include "Sound.h"
#include "BaseView.h"
#include "Player.h"
#include "PlayerView.h"
#include "MultiPlay.h"
#include "Event.h"
#include "Control.h"
#include "Launcher.h"

int LoadData( void *dum );

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
bool isTexture	= true;
bool isPolygon	= true;
bool isSimple	= false;
bool isWireFrame = true;
bool fullScreen = false;

long wins	= 0;		// 勝ち抜き数
long gameLevel  = LEVEL_EASY;	// 強さ
long gameMode   = GAME_21PTS;	// ゲームの長さ

Control*      theControl = NULL;

long mode = MODE_OPENING;

long sndMode;			// あとで引数化

SDL_mutex *loadMutex;

extern void QuitGame();
void StartGame();

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

#ifdef main
# undef main
#endif
int main(int argc, char **argv) {

#endif
#else /* WIN32 */
int main(int argc, char** argv) {
#endif

    int c;
    while(EOF != (c = getopt(argc, argv, "schfSOp:"))) {
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
	}
    }

    if (!isComm && argc > optind) {
      // Client mode
      isComm = true;
      mode = MODE_SELECT;
      strncpy(serverName, argv[optind], sizeof(serverName));
    }

  Launcher *launcher = new Launcher();
  launcher->Init();
}

void
StartGame() {
  if ( isSimple )
    isTexture = false;

#ifdef HAVE_LIBSDL_MIXER
  sndMode = SOUND_SDL;

  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE) < 0 ) {
    perror( "SDL initialize failed\n" );
    exit(1);
  }
#else
  if ( SDL_Init(SDL_INIT_VIDEO) < 0 ) {
    perror( "SDL initialize failed\n" );
    exit(1);
  }
  sndMode = SOUND_NONE;
#endif

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

  theSound.Init( sndMode );

  loadMutex = SDL_CreateMutex();

  SDL_CreateThread( LoadData, NULL );

  EndianCheck();

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

  SDL_Event event;

  SDL_EnableUNICODE(1);

  while (1) {
    while ( SDL_PollEvent(&event) ) {
      // このあたり, GLUT 風になっているので SDL 風に直す
      switch ( event.type ) {
      case SDL_KEYDOWN:
	Event::KeyboardFunc( event, 0, 0 );
	break;
      case SDL_KEYUP:
	Event::KeyUpFunc( event, 0, 0 );
	break;
      case SDL_MOUSEMOTION:
	Event::MotionFunc( event.motion.x, event.motion.y );
	break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
	Event::ButtonFunc( event.button.button, event.button.type, 
			   event.motion.x, event.motion.y );
	break;
      case SDL_QUIT:
	return;
      case SDL_SYSWMEVENT:
	break;
      }
    }

    Event::IdleFunc();
  }

  return;
}

int
LoadData( void *dum ) {
  SDL_mutexP( loadMutex );
  PlayerView::LoadData(NULL);
  SDL_mutexV( loadMutex );

  return 0;
}
