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

#ifndef _MultiPlyaer_
#define _MultiPlayer_

#include "PlayGame.h"

void EndianCheck();

double SwapDbl( double d );
long SwapLong( long l );

bool SendDouble( int sd, double d );
bool SendLong( int sd, long l );
char *ReadDouble( char *buf, double& d );
char *ReadLong( char *buf, long& l );

void WritePlayerData();
Player *ReadPlayerData();
bool AcceptClient();

void StartServer();
void StartClient();

class ExternalData {
public:
  ExternalData();
  ExternalData( long side );

  virtual ~ExternalData();

  long side;
  long dataType;
  long sec;
  char count;
  char data[256];
  ExternalData *next;

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall ) = 0;
  virtual bool Read( long sock ) = 0;

  static ExternalData* ReadData( long s );

  virtual bool isNull() { return false; };
protected:
  void ReadTime( int sd, long *sec, char *count );
};

class ExternalPVData : public ExternalData {
public:
  ExternalPVData();
  ExternalPVData( long side );

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall );
  virtual bool Read( long sock );
};

class ExternalPSData : public ExternalData {
public:
  ExternalPSData();
  ExternalPSData( long side );

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall );
  virtual bool Read( long sock );
};

class ExternalBVData : public ExternalData {
public:
  ExternalBVData();
  ExternalBVData( long side );

  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall );
  virtual bool Read( long sock );
};

class ExternalNullData : public ExternalData {
public:
  virtual bool Apply( Player *targetPlayer, bool &fThePlayer, bool &fComPlayer,
		      bool &fTheBall ) { return false; };
  virtual bool Read( long sock ) { return false; };

  virtual bool isNull() { return true; };
};


class MultiPlay : public PlayGame {
public:
  MultiPlay();
  virtual ~MultiPlay();

  virtual bool Init();

  static MultiPlay* Create( long player, long com );

  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  //long GetSmashPtr() { return m_smashPtr; };

  virtual bool LookAt( double &srcX, double &srcY, double &srcZ,
		       double &destX, double &destY, double &destZ );


  void SendTime_forNODELAY( char *buf );
  void SendTime();

  virtual void EndGame();
protected:
  //bool m_smash;
  //long m_smashCount;
  //long m_smashPtr;
};

#endif	// _MultiPlayer_
