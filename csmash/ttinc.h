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

#ifndef _ttinc_
#define _ttinc_

#define CSMASH_PORT	3573	// TCP Port num. 

#ifdef _WIN32
#define WIN32
#endif

// ウィンドウサイズ
#define WINXSIZE	800
#define WINYSIZE	600
// 競技場の大きさ
#define AREAXSIZE	8.0
#define AREAYSIZE	12.0
#define AREAZSIZE	6.0

// Playerの大きさ
#define UPPERARM	0.25
#define FOREARM		0.30

// 卓球台の大きさ
#define TABLELENGTH	((double)(2.74))
#define TABLEWIDTH	((double)(1.525))
#define TABLEHEIGHT	((double)(0.76))
#define	TABLETHICK	((double)(0.1))
// ネットの高さ
#define NETHEIGHT	((double)(0.1525))
// ボール半径
#define BALL_R		((double)(0.019))

#define TABLE_E		((double)(0.8))
#define PHY		((double)(0.15))

// イメージの大きさ
#define IMAGE_WIDTH	256
#define IMAGE_HEIGHT	256

#define GRAV		(9.8+m_spin*5)
#define GRAVITY(spin)	(9.8+spin*5)

#define	TICK		(0.01)		// 1ターンの長さ(秒)

// キー入力
#define KEY_LEFT    (1<<0)
#define KEY_RIGHT   (1<<1)
#define KEY_FORWARD (1<<2)
#define KEY_BACK    (1<<3)
#define KEY_UP      (1<<4)
#define KEY_DOWN    (1<<5)
#define KEY_HIT     (1<<6)

// マウス入力
#define BUTTON_LEFT   (1<<0)
#define BUTTON_MIDDLE (1<<1)
#define BUTTON_RIGHT  (1<<2)

#define MAX_HISTORY (1024)

// モード
#define MODE_SOLOPLAY       (1)	// Play VS COM
#define MODE_MULTIPLAY      (2)	// Play VS MAN
#define MODE_SELECT         (3)	// Player Select
#define MODE_TITLE          (4)	// Title
#define MODE_HOWTO          (5)	// How to Play
#define MODE_TRAININGSELECT (6)	// Training Select
#define MODE_TRAINING       (7)	// Training

// サウンド
#define SOUND_RACKET (0)
#define SOUND_TABLE  (1)
#define SOUND_CLICK  (2)

// 通信対戦
#define DATA_PV      (0)
#define DATA_PS      (1)
#define DATA_BT      (2)
#define DATA_BV      (3)

// その他
#define RAND(N) ((int)((double)(N)*rand()/RAND_MAX))

// 強さ
enum level {LEVEL_EASY, LEVEL_NORMAL, LEVEL_HARD, LEVEL_TSUBORISH};
enum mode  {GAME_5PTS, GAME_11PTS, GAME_21PTS};

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <direct.h>
#define F_OK 0 /* if exist */
#else
#define closesocket(FD) close(FD)
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <fcntl.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <ctype.h>

#include <GL/gl.h>
#include <GL/glut.h>

#if HAVE_LIBPTHREAD
#include <pthread.h>
#endif

#ifdef HAVE_LIBZ
#include <sys/stat.h>
#include "z.h"
#endif

#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "LoadImage.h"
#include "Ball.h"
#include "Event.h"
#include "View.h"
#include "Control.h"
#include "BaseView.h"
#include "BallView.h"
#include "Player.h"
#include "comPlayer.h"
#include "PenAttack.h"
#include "ShakeCut.h"
#include "PenDrive.h"
#include "comPenAttack.h"
#include "comShakeCut.h"
#include "comPenDrive.h"
#include "ComTrainingPenAttack.h"
#include "TrainingPenAttack.h"
#include "ComTrainingPenDrive.h"
#include "TrainingPenDrive.h"
#include "PlayerView.h"
#include "parts.h"
#include "HitMark.h"
#include "PlayerSelect.h"
#include "PlayerSelectView.h"
#include "TrainingSelect.h"
#include "TrainingSelectView.h"
#include "SoloPlay.h"
#include "MultiPlay.h"
#include "Title.h"
#include "TitleView.h"
#include "Howto.h"
#include "HowtoView.h"
#include "Training.h"
#include "TrainingView.h"
#include "Sound.h"

void xerror(const char *str, ...);

#endif // _ttinc_
