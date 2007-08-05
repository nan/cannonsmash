/**
 * @file
 * @brief Implementation of Logging class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2007  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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
#include "NoLogging.h"

/**
 * Default constructor. 
 * Initialize member variables. 
 */
NoLogging::NoLogging() {
}

/**
 * Destructor. 
 * Delete file pointers. 
 */
NoLogging::~NoLogging() {
}

/**
 * Get singleton Logging object. 
 * 
 * @return returns singleton Logging object. 
 */
Logging*
NoLogging::GetLogging() {
  if ( !Logging::m_logging )
    Logging::m_logging = new NoLogging();

  return Logging::m_logging;
}

/**
 * Initializer method. 
 * 
 * @return returns true if succeeds. 
 */
bool
NoLogging::Init() {
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
NoLogging::Log( long logType, char *logString ) {
  return true;
}

/**
 * Write startup message to each log file. 
 * 
 * @return returns true if succeeds. 
 */
bool
NoLogging::StartLog() {
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
NoLogging::LogTime( long logType, struct timeb *tb ) {
  return true;
}

/**
 * Write current time to log file. 
 * 
 * @param logType type of log message
 * @return returns true if succeeds. 
 */
bool
NoLogging::LogTime( long logType ) {
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
NoLogging::LogBall( long logType, Ball *ball ) {
  return true;
}

bool
NoLogging::LogScore( long logType, long score1, long score2 ) {
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
NoLogging::LogPlayer( long logType, Player *player ) {
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
NoLogging::LogSendBVMessage( Ball *ball ) {
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
NoLogging::LogRecvBVMessage( ExternalBVData *bv ) {
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
NoLogging::LogSendPVMessage( Player *player ) {
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
NoLogging::LogRecvPVMessage( ExternalPVData *pv ) {
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
NoLogging::LogSendPSMessage( Player *player ) {
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
NoLogging::LogRecvPSMessage( ExternalPSData *ps ) {
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
NoLogging::LogRecvPTMessage( ExternalPTData *pt ) {
  return true;
}

bool
NoLogging::LogNullSound() {
  return true;
}

bool
NoLogging::LogSound(unsigned char *sndData) {
  return true;
}

void
NoLogging::LogSound(int channel, void *stream, int len, void *udata) {
}
#endif
