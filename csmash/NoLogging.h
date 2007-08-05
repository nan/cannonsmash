/**
 * @file
 * @brief Definition of Logging class. 
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

#ifndef _NoLogging_
#define _NoLogging_

#include "Logging.h"

/**
 * Logger class. 
 * This class writes logg message, especially for internet play log. 
 */
class NoLogging : public Logging {
public:
  virtual ~NoLogging();
  static Logging* GetLogging();
  virtual bool Init();

  virtual bool Log( long logType, char *logString );
  virtual bool StartLog();
  virtual bool LogTime( long logType, struct timeb *tb );
  virtual bool LogTime( long logType );
  virtual bool LogScore( long logType, long score1, long score2 );
  virtual bool LogBall( long logType, Ball *ball );
  virtual bool LogPlayer( long logType, Player *player );
  virtual bool LogSendBVMessage( Ball *ball );
  virtual bool LogRecvBVMessage( ExternalBVData *bv );
  virtual bool LogSendPVMessage( Player *player );
  virtual bool LogRecvPVMessage( ExternalPVData *pv );
  virtual bool LogSendPSMessage( Player *player );
  virtual bool LogRecvPSMessage( ExternalPSData *ps );
  virtual bool LogRecvPTMessage( ExternalPTData *pt );
  virtual bool LogNullSound();
  virtual bool LogSound(unsigned char *sndData);

  static void LogSound(int channel, void *stream, int len, void *udata);
private:
  NoLogging();
};
#endif
