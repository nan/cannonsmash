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

void EndianCheck();

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
  long side;
  long dataType;
  long sec;
  char count;
  char data[256];
  ExternalData *next;
};

class MultiPlay : public Control {
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
protected:
  //bool m_smash;
  //long m_smashCount;
  //long m_smashPtr;
};

#endif	// _MultiPlayer_
