/* $Id$ */

// Copyright (C) 2000, 2001  神南 吉宏(Kanna Yoshihiro)
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
#include "Sound.h"

#ifdef WIN32
#include <io.h>
#endif

extern long mode;
extern SDL_mutex *loadMutex;

Sound* Sound::m_theSound = NULL;

Sound::Sound() {
#ifdef HAVE_LIBSDL_MIXER
#else
  m_soundMode = SOUND_NONE;
#endif
}

Sound::~Sound() {
#ifdef HAVE_LIBSDL_MIXER
  for ( int i = 0 ; i < 16 ; i++ ) {
    if ( m_sound[i] != 0 ) {
      Mix_FreeChunk( m_sound[i] );
      m_sound[i] = 0;
    }
  }

  if ( m_opening != 0 ) {
    Mix_FreeMusic( m_opening );
    m_opening = 0;
  }

  Mix_CloseAudio();
#endif
}

Sound*
Sound::TheSound() {
  if ( Sound::m_theSound )
    return Sound::m_theSound;
  else
    return (Sound::m_theSound = new Sound());
}

bool
Sound::Init( long sndMode ) {
  m_soundMode = sndMode;

#ifdef HAVE_LIBSDL_MIXER
  for ( int i = 0 ; i < 16 ; i++ ) {
    m_sound[i] = 0;
  }
  m_opening = 0;
#endif

  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
#ifdef WIN32
  if ( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 4096 ) < 0 ) {
#else
  if ( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 128 ) < 0 ) {
#endif
    perror( "SDL Mix_OpenAudio failed\n" );
  }

  m_sound[SOUND_RACKET] = Mix_LoadWAV( "wav/racket.wav" );
  m_sound[SOUND_TABLE] = Mix_LoadWAV( "wav/table.wav" );
  m_sound[SOUND_CLICK] = Mix_LoadWAV( "wav/click.wav" );
#endif

  return true;
}

void
Sound::Clear() {
#ifdef HAVE_LIBSDL_MIXER
  for ( int i = 0 ; i < 16 ; i++ ) {
    if ( m_sound[i] != 0 ) {
      Mix_FreeChunk( m_sound[i] );
      m_sound[i] = 0;
    }
  }

  if ( m_opening != 0 ) {
    Mix_FreeMusic( m_opening );
    m_opening = 0;
  }

  Mix_CloseAudio();
#endif
}

bool
Sound::Play( long soundID ) {
  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
  Mix_PlayChannel( 0, m_sound[soundID], 0 );
#endif

  return true;
}

long
Sound::GetSoundMode() {
  return m_soundMode;
}

long
Sound::InitBGM( char *filename ) {
  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
  m_opening = Mix_LoadMUS( OPENINGFILENAME );

  if ( m_opening == 0 )
    printf( "%s\n", SDL_GetError() );
#endif

  return 0;
}

// It is better to move it to other thread
long
Sound::PlayBGM() {
  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
  Mix_PlayMusic( m_opening, 1 );
#endif

  return 0;
}

long
Sound::StopBGM() {
  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
  Mix_FadeOutMusic( 2000 );
#endif

  return 0;
}
