/**
 * @file
 * @brief Definition of MultiPlay and ExternalData classes. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _MultiPlay_
#define _MultiPlay_

#include "PlayGame.h"

/**
 * ExternalData class represents a message from the lobby server or the opponent machine. 
 * ExternalData class is a base class. For each message type, subclass of 
 * ExternalData is defined. 
 */
class ExternalData {
public:
  ExternalData();
  ExternalData( long side );

  virtual ~ExternalData();

  long side;		///< side of the player who send this message
  long dataType;	///< type of the message
  long sec;		///< time in second when this message is sent
  char count;		///< time in 1/100 second when this message is sent
  char data[256];	///< payload
  ExternalData *next;	///< next ExternalData object

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall ) = 0;
  virtual bool Read( long sock ) = 0;

  static ExternalData* ReadData( long s );

  /**
   * Whether this ExternalData object is ExternalNullData or not. 
   */
  virtual bool isNull() { return false; };

protected:
  void ReadTime( int sd, long *sec, char *count );
};

/**
 * ExternalPVData class represents a PV message from the opponent machine. 
 */
class ExternalPVData : public ExternalData {
public:
  ExternalPVData();
  ExternalPVData( long side );

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall );
  virtual bool Read( long sock );
};

/**
 * ExternalPSData class represents a PS message from the opponent machine. 
 */
class ExternalPSData : public ExternalData {
public:
  ExternalPSData();
  ExternalPSData( long side );

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall );
  virtual bool Read( long sock );
};

/**
 * ExternalBVData class represents a BV message from the opponent machine. 
 */
class ExternalBVData : public ExternalData {
public:
  ExternalBVData();
  ExternalBVData( long side );

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall );
  virtual bool Read( long sock );
};

/**
 * ExternalPTData class represents a PT message from the opponent machine. 
 */
class ExternalPTData : public ExternalData {
public:
  ExternalPTData();
  ExternalPTData( long side );

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall );
  virtual bool Read( long sock );
};

/**
 * ExternalNullData class represents a null message. 
 * This object is used for the anchor of ExternalData list. 
 */
class ExternalNullData : public ExternalData {
public:
  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall ) { return false; };
  /**
   * Always returns false. 
   */
  virtual bool Read( long sock ) { return false; };

  /**
   * Whether this object is ExternalNullData or not. 
   */
  virtual bool isNull() { return true; };
};


/**
 * MultiPlay class is the controller class for multiplayer mode. 
 */
class MultiPlay : public PlayGame {
public:
  MultiPlay();
  virtual ~MultiPlay();

  virtual bool Init();

  static void Create( long player, long com );

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual bool LookAt( vector3d &srcX, vector3d &destX );

  void SendTime( char *buf );

  virtual void EndGame();

  void StartServer();
  void StartClient();

  static int WaitForData( void * );
};

#endif	// _MultiPlay_
