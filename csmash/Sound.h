/* $Id$ */

// Copyright (C) 2000  $B?@Fn(B $B5H9((B(Kanna Yoshihiro)
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

#ifndef _Sound_
#define _Sound_

#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

class Sound {
public:
  Sound();
  virtual ~Sound();

  virtual bool Init( long sndMode );
  virtual void Clear();

  //bool Play( char *sndData, long count );
  bool Play( long soundID );

  long GetSoundMode();
  bool SetSoundMode( long mode );

  long InitBGM( char *filename );
  long LoadBGM( char *filename );
  long PlayBGM();
  long StopBGM();
  long SkipBGM();

  // For BGM
  char *m_bgmSound;

#ifdef HAVE_LIBSDL_MIXER
  Mix_Chunk *m_sound[16];
#endif

  long m_soundMode;
};

#endif // _Sound_
