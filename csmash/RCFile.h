/**
 * @file
 * @brief Definition of RCFile class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2001, 2003, 2007  Kanna Yoshihiro
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

#ifndef _RCFile_
#define _RCFile_

#include "ttinc.h"

/**
 * RCFile class is for loading/saving .csmashrc (csmash.rc) file. 
 */
class RCFile {
public:
  static RCFile *GetRCFile();

  bool isTexture;	///< Texture is enabled/disabled
  bool fullScreen;	///< window mode/fullscreen mode
  long myModel;		///< Model is arm only/transparent/wire frame
  long gmode;		///< Graphic mode is simple/normal
  long gameLevel;	///< Game level is easy/normal/hard/tsuborish
  long gameMode;	///< Game mode is 5pts/11pts/21pts
  bool switchButtons;	///< Whether mouse buttons are switched or not
  long sndMode;		///< Sound mode is none/SDL
  float sndVolume;	///< Sound volume(0..100)
  long protocol;	///< Network protocol is IPV4/IPV6

  char serverName[256];	///< Default server name
  char nickname[32];	///< Default nickname
  char message[64];	///< Default message

  short csmash_port;	///< Default port number

  bool ReadRCFile();
  bool WriteRCFile();

private:
  RCFile();

  static RCFile* m_rcFile;	///< Reference to singleton RCFile object. 

  FILE * OpenRCFile( char *mode );
};

#endif	// _RCFile_
