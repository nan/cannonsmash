/**
 * @file
 * @brief Definition of Sound class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2000-2004  ¿ÀÆî µÈ¹¨(Kanna Yoshihiro)
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

#include <vector>
#include "matrix"
typedef Vector<3, double> vector3d;

#define SOUND_QUEUESIZE 16

struct buffer {
  unsigned char const *start;
  unsigned long length;
};

/**
 * Sound class is a utility class to play sound effect and BGM. 
 */
class Sound {
public:
  virtual ~Sound();

  static Sound* TheSound();

  virtual bool Init( long sndMode );
  virtual void Clear();

  //bool Play( char *sndData, long count );
  bool Play( long soundID, vector3d x );
  bool PlayScore( long score1, long score2 );
  bool PlayNumber( long number );

  long GetSoundMode();

  long InitBGM( char *filename );
  long PlayBGM();
  long StopBGM();

#ifdef HAVE_LIBSDL_MIXER
  Mix_Chunk *m_sound[16];	///< Handler for sound effect wav file
  Mix_Chunk *m_score[31];	///< Handler for score reading wav file
  Mix_Music *m_opening;		///< Handler for opening BGM

  static void PlayFinishHandler( int channel );
#endif

  long m_soundMode;		///< Sound mode (SDL or none)
private:
  Sound();
  static Sound *m_theSound;	///< Singleton Sound object

#ifdef HAVE_LIBSDL_MIXER
  Mix_Chunk *m_chunkqueue[SOUND_QUEUESIZE];	///< Queue of handlers for playing in blocking mode
  int m_queuehead;		///< Head of the queue
  int m_queuetail;		///< Tail of the queue

  bool PlayBlocking( int channel, Mix_Chunk *chunk );
#endif
};

#endif // _Sound_
