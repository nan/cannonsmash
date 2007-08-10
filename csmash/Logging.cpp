/**
 * @file
 * @brief Implementation of Logging class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2001-2004, 2007  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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
#ifdef LOGGING
#include "MultiPlay.h"
#include "Event.h"
#include "Ball.h"
#include "Player.h"
#include "Network.h"
#include "Logging.h"

extern Ball theBall;
Logging* Logging::m_logging = NULL;

/**
 * Default constructor. 
 * Initialize member variables. 
 */
Logging::Logging() {
    memset(m_fp, 0, sizeof(m_fp));
}

/**
 * Destructor. 
 * Delete file pointers. 
 */
Logging::~Logging() {
  for ( int i = 0 ; i < LOG_FPSIZE ; i++ ) {
    if (m_fp[i]) fclose( m_fp[i] );
  }
}

/**
 * Get singleton Logging object. 
 * 
 * @return returns singleton Logging object. 
 */
Logging*
Logging::GetLogging() {
  if ( !Logging::m_logging )
    Logging::m_logging = new Logging();

  return Logging::m_logging;
}

/**
 * Initializer method. 
 * This method opens log file. 
 * 
 * @return returns true if succeeds. 
 */
bool
Logging::Init() {
  static const char* fname[] = {
      "log/com_ball.log",
      "log/com_thePlayer.log", 
      "log/com_comPlayer.log",
      "log/com_misc.log", 
      "log/act_ball.log",
      "log/act_thePlayer.log", 
      "log/act_comPlayer.log",
      "log/act_misc.log",
      "log/sound.log",
      "log/camera.log",
  };

  for ( int i = 0 ; i < LOG_FPSIZE; i++ ) {
    if ( (m_fp[i] = fopen( fname[i], "w" )) == 0 )
      return false;
  }

  return true;
}

/**
 * Write log message to specified file. 
 * This is the basic log writing API. 
 * 
 * @param logType type of log message. This specifies log file. 
 * @param logString string which is written to log file. 
 * 
 * @return returns true if succeeds. 
 */
bool
Logging::Log( long logType, char *logString ) {
    if (m_fp[logType]) {
        fputs( logString, m_fp[logType] );
        fflush( m_fp[logType] );
    }
  return true;
}

/**
 * Write startup message to each log file. 
 * 
 * @return returns true if succeeds. 
 */
bool
Logging::StartLog() {
  struct timeb tb;
  char buf[256];

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

  snprintf( buf, sizeof(buf), "--- START LOGGING %d.%3d ---\n",
            (int)tb.time, (int)tb.millitm );

  for ( int i = 0 ; i < 7 ; i++ )
    Log( i, buf );

  return true;
}

/**
 * Write time information to log file. 
 * 
 * @param logType type of log message
 * @param tb time information
 * 
 * @return returns true if succeeds. 
 */
bool
Logging::LogTime( long logType, struct timeb *tb ) {
  char buf[64];

  snprintf( buf, sizeof(buf), "%d.%3d: ", (int)tb->time, (int)tb->millitm );
  Log( logType, buf );

  return true;
}

/**
 * Write current time to log file. 
 * 
 * @param logType type of log message
 * @return returns true if succeeds. 
 */
bool
Logging::LogTime( long logType ) {
  char buf[64];
  long sec, count;

  sec = Event::m_lastTime.time;
  count = Event::m_lastTime.millitm/10;
  Event::GetAdjustedTime( sec, count );
  snprintf( buf, sizeof(buf), "%d.%3d: ", (int)sec, (int)count*10 );
  Log( logType, buf );

  return true;
}

/**
 * Write ball information to log file. 
 * Ball location, velocity, spin are logged to log file. 
 * 
 * @param logType type of log message. 
 * @param ball Ball object which is logged to log file. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogBall( long logType, Ball *ball ) {
  char buf[1024];

  snprintf( buf, sizeof(buf),
	    "x=%4.6f y=%4.6f z=%4.6f "
	    "vx=%4.6f vy=%4.6f vz=%4.6f "
	    "spinX=%3.6f spinY=%3.6f st=%d\n", 
	    ball->GetX()[0], ball->GetX()[1], ball->GetX()[2], 
	    ball->GetV()[0], ball->GetV()[1], ball->GetV()[2], 
	    ball->GetSpin()[0], ball->GetSpin()[1],
	    ball->GetStatus());

  Log( logType, buf );

  return true;
}

bool
Logging::LogScore( long logType, long score1, long score2 ) {
  char buf[1024];

  snprintf( buf, sizeof(buf),
	    "%d - %d  ", 
	    score1, score2 );
  Log( logType, buf );

  return true;
}

/**
 * Write player information to log file. 
 * Player type, side, location, velocity, status, etc. are logged to log file. 
 * 
 * @param logType type of log message. 
 * @param player Player object which is logged to log file. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogPlayer( long logType, Player *player ) {
  char buf[1024];

  snprintf( buf, sizeof(buf),
	    "playerType=%1d side=%2d x=%4.6f y=%4.6f z=%4.6f "
	    "vx=%4.6f vy=%4.6f vz=%4.6f status=%3d "
	    "swing=%2d swingType=%1d swingSide=%2d afterSwing=%2d "
	    "swingError=%1d targetX=%4.6f targetY=%4.6f "
	    "eyeX=%4.6f eyeY=%4.6f eyeZ=%4.6f "
	    "lookAtX=%4.6f lookAtY=%4.6f lookAtZ=%4.6f "
	    "pow=%1d spinX=%3.6f spinY=%3.6f stamina=%2.0f statusMax=%3d "
	    "dragX=%2d dragY=%2d\n",
	    (int)player->GetPlayerType(), (int)player->GetSide(), 
	    player->GetX()[0], player->GetX()[1], player->GetX()[2], 
	    player->GetV()[0], player->GetV()[1], player->GetV()[2], 
	    (int)player->GetStatus(), (int)player->GetSwing(),
	    (int)player->GetSwingType(), (int)player->GetSwingSide(),
	    (int)player->GetAfterSwing(), (int)player->GetSwingError(), 
	    player->GetTarget()[0], player->GetTarget()[1],
	    player->GetEye()[0], player->GetEye()[1], player->GetEye()[2],
	    player->GetLookAt()[0], player->GetLookAt()[1], player->GetLookAt()[2],
	    (int)player->GetPower(), player->GetSpin()[0], player->GetSpin()[1],
	    player->GetStamina(), player->GetStatusMax(), 
	    (int)player->GetDragX(), (int)player->GetDragY() );
  Log( logType, buf );

  return true;
}


/**
 * Write BV message to log file when it is sent. 
 * BV message represents ball location, velocity, spin and status. 
 * 
 * @param ball Ball object of which BV message is written. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogSendBVMessage( Ball *ball ) {
  char buf[256];

  LogTime( LOG_COMBALL );
  snprintf( buf, sizeof(buf),
            "x=%4.2f y=%4.2f z=%4.2f "
            "vx=%4.2f vy=%4.2f vz=%4.2f spinY=%3.2f status=%2d\n",
	   ball->GetX()[0], ball->GetX()[1], ball->GetX()[2], 
	   ball->GetV()[0], ball->GetV()[1], ball->GetV()[2], 
	   ball->GetSpin()[1], (int)ball->GetStatus() );
  Log( LOG_COMBALL, buf );

  return true;
}

/**
 * Write BV message to log file when it is received. 
 * BV message represents ball location, velocity, spin and status. 
 * 
 * @param bv BV message which is logged to log file. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogRecvBVMessage( ExternalBVData *bv ) {
  char buf[256];

  LogTime( LOG_COMBALL );
  snprintf( buf, sizeof(buf), "recv: %d.%3d ", (int)bv->sec, (int)bv->count );
  Log( LOG_COMBALL, buf );
  Ball *tmpBall = new Ball();
  tmpBall->Warp(bv->data);

  snprintf( buf, sizeof(buf),
            "x=%4.2f y=%4.2f z=%4.2f "
            "vx=%4.2f vy=%4.2f vz=%4.2f spinY=%3.2f status=%2d\n",
	   tmpBall->GetX()[0], tmpBall->GetX()[1], tmpBall->GetX()[2], 
	   tmpBall->GetV()[0], tmpBall->GetV()[1], tmpBall->GetV()[2], 
	   tmpBall->GetSpin()[1], (int)tmpBall->GetStatus() );
  Log( LOG_COMBALL, buf );

  return true;
}

/**
 * Write PV message to log file when it is sent. 
 * PV message represents player location and velocity. 
 * 
 * @param player Player object of which PV message is written. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogSendPVMessage( Player *player ) {
  char buf[256];

  LogTime( LOG_COMTHEPLAYER );
  snprintf( buf, sizeof(buf),
           "send PV: x=%4.2f y=%4.2f z=%4.2f vx=%4.2f vy=%4.2f vz=%4.2f\n",
	   player->GetX()[0], player->GetX()[1], player->GetX()[2], 
	   player->GetV()[0], player->GetV()[1], player->GetV()[2] );
  Log( LOG_COMTHEPLAYER, buf );

  return true;
}

/**
 * Write PV message to log file when it is received. 
 * PV message represents player location and velocity. 
 * 
 * @param pv PV message which is logged to log file. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogRecvPVMessage( ExternalPVData *pv ) {
  char buf[256];

  LogTime( LOG_COMCOMPLAYER );
  snprintf( buf, sizeof(buf),
            "recv PV: %d.%3d ", (int)pv->sec, (int)pv->count );
  Log( LOG_COMCOMPLAYER, buf );
  Player *tmpPlayer = new Player();
  tmpPlayer->Warp(pv->data);

  snprintf( buf, sizeof(buf),
            "x=%4.2f y=%4.2f z=%4.2f vx=%4.2f vy=%4.2f vz=%4.2f\n",
            tmpPlayer->GetX()[0], tmpPlayer->GetX()[1], tmpPlayer->GetX()[2], 
            tmpPlayer->GetV()[0], tmpPlayer->GetV()[1], tmpPlayer->GetV()[2] );
  Log( LOG_COMCOMPLAYER, buf );

  return true;
}

/**
 * Write PS message to log file when it is sent. 
 * PS message represents player swing information. 
 * 
 * @param player Player object of which PS message is written. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogSendPSMessage( Player *player ) {
  char buf[256];

  LogTime( LOG_COMTHEPLAYER );

  snprintf( buf, sizeof(buf),
            "send PS: pow=%2d spinY=%3.2f swingType=%1d swingSide=%2d swing=%2d\n",
	   (int)player->GetPower(), player->GetSpin()[1],
	   (int)player->GetSwingType(), player->GetSwingSide(),
	   (int)player->GetSwing() );
  Log( LOG_COMTHEPLAYER, buf );

  return true;
}

/**
 * Write PS message to log file when it is received. 
 * PS message represents player swing information. 
 * 
 * @param ps PS message which is logged to log file. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogRecvPSMessage( ExternalPSData *ps ) {
  char buf[256];

  LogTime( LOG_COMCOMPLAYER );
  snprintf( buf, sizeof(buf),
            "recv PS: %d.%3d ", (int)ps->sec, (int)ps->count );
  Log( LOG_COMCOMPLAYER, buf );
  Player *tmpPlayer = new Player();
  tmpPlayer->ExternalSwing(ps->data);

  snprintf( buf, sizeof(buf),
           "pow=%2d spinY=%3.2f swingType=%1d swingSide=%2d swing=%2d\n",
	   (int)tmpPlayer->GetPower(), tmpPlayer->GetSpin()[1],
	   (int)tmpPlayer->GetSwingType(), tmpPlayer->GetSwingSide(),
	   (int)tmpPlayer->GetSwing() );
  Log( LOG_COMCOMPLAYER, buf );

  return true;
}

/**
 * Write PT message to log file when it is received. 
 * PT message represents player type. 
 * 
 * @param pt PT message which is logged to log file. 
 * @return returns true if succeeds. 
 */
bool
Logging::LogRecvPTMessage( ExternalPTData *pt ) {
  char buf[256];

  LogTime( LOG_COMMISC );
  snprintf( buf, sizeof(buf),
            "recv PT: %d.%3d ", (int)pt->sec, (int)pt->count );
  Log( LOG_COMMISC, buf );

  long rotate;
  ReadLong( &(pt->data[1]), rotate );

  snprintf( buf, sizeof(buf), "fixed=%d type=%ld\n", pt->data[0],
	    rotate );
  Log( LOG_COMMISC, buf );

  return true;
}

bool
Logging::LogNullSound() {
  fputc(0, m_fp[LOG_SOUND]);
  fflush(m_fp[LOG_SOUND]);

  return true;
}

bool
Logging::LogSound(unsigned char *sndData) {
  fputc(-1, m_fp[LOG_SOUND]);
  fwrite(sndData, 256, 1, m_fp[LOG_SOUND]);
  fflush(m_fp[LOG_SOUND]);

  return true;
}

bool
isAllZero(void *stream, int len) {
  for (int i=0; i<len; i++) {
    if (((char *)stream)[i])
      return false;
  }

  return true;
}

void
Logging::LogSound(int channel, void *stream, int len, void *udata) {
  static unsigned char soundData[44100*2*2*300];
  static long length = 0;

  if (!Control::TheControl()->IsPlaying())
    return;

  if (length+len > 44100*2*2*300 ||
      (theBall.GetStatus() == 8 && length >= 256)) {
    int i;
    for (i=0; i<length-256; i+=256) {
      if (isAllZero(&(soundData[i]), 256)) {
	Logging::GetLogging()->LogNullSound();
      } else {
	Logging::GetLogging()->LogSound(&(soundData[i]));
      }
    }

    memcpy(soundData, &(soundData[i]), length-i);
    length = length-i;
  }

  memcpy(&(soundData[length]), stream, len);
  length += len;
}

bool
Logging::LogCamera(vector3d srcX, vector3d destX) {
  char buf[1024];

  snprintf( buf, sizeof(buf),
	    "srcX=%4.6f srcY=%4.6f srcZ=%4.6f "
	    "destX=%4.6f destY=%4.6f destZ=%4.6f\n", 
	    srcX[0], srcX[1], srcX[2], 
	    destX[0], destX[1], destX[2]);
  Log( LOG_CAMERA, buf );

  return true;
}
#endif
