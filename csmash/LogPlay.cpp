/**
 * @file
 * @brief Implementation of LogPlay class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2007  神南 吉宏(Kanna Yoshihiro)
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
#include "LogPlay.h"
#include "Player.h"
#include "Ball.h"
#include "Event.h"
#include "BaseView.h"
#include "RCFile.h"

extern RCFile *theRC;

extern Ball theBall;
extern long mode;
extern long wins;

extern void CopyPlayerData( Player& dest, Player* src );

/**
 * Default constructor. 
 */
LogPlay::LogPlay() {
}

/**
 * Destructor. Do nothing. 
 */
LogPlay::~LogPlay() {
}

/**
 * Initializer method. 
 * Open log files and 
 * create PlayGameView and attach it to this object. 
 * 
 * @return returns true if succeeds. 
 */
bool
LogPlay::Init() {
  long sec, cnt;
  long score1, score2;

  m_View = (PlayGameView *)View::CreateView( VIEW_PLAYGAME );

  m_View->Init( this );

  BaseView::TheView()->AddView( m_View );

  fpBall = fopen("log/act_ball.log", "r");
  fpThePlayer = fopen("log/act_thePlayer.log", "r");
  fpComPlayer = fopen("log/act_comPlayer.log", "r");

  theBall.LoadBallLog(fpBall, sec, cnt, score1, score2);
  ((LogPlay *)m_theControl)->ChangeScore(score1, score2);

  Player *p1, *p2;
  p1 = new Player();
  p2 = new Player();

  p1->LoadPlayerLog(fpThePlayer, sec, cnt);
  p2->LoadPlayerLog(fpComPlayer, sec, cnt);
  m_thePlayer = Player::Create( p1->GetPlayerType()-1, 1, 0 );
  m_comPlayer = Player::Create( p2->GetPlayerType()-1, -1, 0 );
  delete p1;
  delete p2;

  Soundlog2wav();

  return true;
}

/**
 * LogPlay object creater. 
 * This method creates singleton LogPlay object, and two players. 
 * 
 * @param player type of the player controlled by this game player. 
 * @param com type of the player controlled by the opponent game player. 
 */
void
LogPlay::Create() {
  Control::ClearControl();

  m_theControl = new LogPlay();
  m_theControl->Init();

  m_thePlayer->Init();
  m_comPlayer->Init();
}

/**
 * Move valid objects. 
 * Call Move method of the ball and players. Or show smash replay. 
 * 
 * @param KeyHistory history of keyboard input
 * @param MouseXHistory history of mouse cursor move
 * @param MouseYHistory history of mouse cursor move
 * @param MouseBHistory history of mouse button push/release
 * @param Histptr current position of histories described above. 
 * @return returns true if it is neccesary to redraw. 
 */
bool
LogPlay::Move( SDL_keysym *KeyHistory, long *MouseXHistory,
	       long *MouseYHistory, unsigned long *MouseBHistory,
	       int Histptr ) {
  static int count = 0, redrawCount = 0;
  bool reDraw = false;
  long sec, cnt;
  long score1, score2;

  if (!theBall.LoadBallLog(fpBall, sec, cnt, score1, score2))
    exit(1);

  ChangeScore(score1, score2);

  m_thePlayer->LoadPlayerLog(fpThePlayer, sec, cnt);
  m_comPlayer->LoadPlayerLog(fpComPlayer, sec, cnt);

  count++;
  if (redrawCount*100/30 < count) {
    ScreenShot();
    reDraw = true;
    redrawCount++;
  } else {
    reDraw = false;
  }

  return reDraw;
}

/**
 * Set camera position and direction. 
 * 
 * @param srcX camera position [out]
 * @param destX point where the camera is directed. [out]
 * @return returns true if succeeds. 
 */
bool
LogPlay::LookAt( vector3d &srcX, vector3d &destX ) {
  if (m_thePlayer) {
    srcX = m_thePlayer->GetX() + m_thePlayer->GetEye();
    destX = m_thePlayer->GetLookAt();
  }

  return true;
}

void
LogPlay::ScreenShot() {
  static int count = 0;

  static SDL_Surface *image = NULL;
  static SDL_Surface *temp = NULL;
  int idx;

  long width = BaseView::GetWinWidth();
  long height = BaseView::GetWinHeight();

  if (image == NULL) {
    image = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
				 32, 0x000000FF, 0x0000FF00,
				 0x00FF0000, 0xFF000000);
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height,
				32, 0x000000FF, 0x0000FF00,
				0x00FF0000, 0xFF000000);
  }

  glReadPixels(0, 0, width, height, GL_RGBA,
	       GL_UNSIGNED_BYTE, image->pixels);

  for (idx = 0; idx < height; idx++) {
    memcpy((char *)temp->pixels + 4 * width * idx,
	   (char *)image->pixels + 4 * width*(height-1 - idx),
	   4*width);
  }

  char fname[64];
  sprintf( fname, "image%06d.bmp", count );
  SDL_SaveBMP(temp, fname);

  count++;
}

void
LogPlay::Soundlog2wav() {
  FILE *fp = fopen("log/sound.log", "r");
  FILE *gp = fopen("log/sound.wav", "w");
  int c;
  unsigned char sndData[256];
  long len=0;
  long longtmp;
  short shorttmp;
  char chartmp;

  while ((c = fgetc(fp))!=EOF) {
    if (c == 0) {
      len += 256;
    } else {
      fread(sndData, 256, 1, fp);
      len += 256;
    }
  }

  fprintf(gp, "RIFF");
  longtmp = len+36;
  fwrite(&longtmp, 4, 1, gp);
  fprintf(gp, "WAVEfmt ");
  longtmp = 16;
  fwrite(&longtmp, 4, 1, gp);
  shorttmp = 1;
  fwrite(&shorttmp, 2, 1, gp);
  shorttmp = 2;
  fwrite(&shorttmp, 2, 1, gp);
  longtmp = 44100;
  fwrite(&longtmp, 4, 1, gp);
  longtmp = 176400;
  fwrite(&longtmp, 4, 1, gp);
  shorttmp = 4;
  fwrite(&shorttmp, 2, 1, gp);
  shorttmp = 16;
  fwrite(&shorttmp, 2, 1, gp);
  fprintf(gp, "data");
  fwrite(&len, 4, 1, gp);

  rewind(fp);

  while ((c = fgetc(fp))!=EOF) {
    if (c == 0) {
      for (int i=0; i<256;i++) {
	fputc(0, gp);
      }
    } else {
      fread(sndData, 256, 1, fp);
      fwrite(sndData, 256, 1, gp);
    }
  }

  fflush(gp);

  fclose(fp);
  fclose(gp);
}
