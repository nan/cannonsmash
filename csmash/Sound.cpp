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

#include "ttinc.h"
#include "Sound.h"

#ifdef WIN32
#include <io.h>
#endif

extern long mode;
extern Sound theSound;
extern SDL_mutex *loadMutex;

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
  Mix_CloseAudio();
#endif
}

bool
Sound::Init( long sndMode ) {
#ifdef HAVE_LIBSDL_MIXER
#ifdef WIN32
  if ( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 4096 ) < 0 ) {
#else
  if ( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 128 ) < 0 ) {
  //if ( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 128 ) < 0 ) {
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
  Mix_CloseAudio();
#endif
}

/*
bool
Sound::Play( char *sndData, long size ) {
  if ( mode != MODE_MULTIPLAY ) {
  }

  return true;
}
*/

bool
Sound::Play( long soundID ) {
  if ( mode != MODE_MULTIPLAY ) {
#ifdef HAVE_LIBSDL_MIXER
    //Mix_PlayChannel( -1, m_sound[soundID], 0 );
    Mix_PlayChannel( 0, m_sound[soundID], 0 );
#endif
  }

  return true;
}

long
Sound::GetSoundMode() {
  return m_soundMode;
}

// Unnessesary?
bool
Sound::SetSoundMode( long mode ) {
  if ( mode == m_soundMode )
    return true;

  return true;
}

long
Sound::LoadBGM( char *filename ) {
#ifdef HAVE_LIBVORBIS
  OggVorbis_File vf;
  int is_big_endian = 0;
  int current_section = -1, ret;
  int old_section = -1;
  int total = 0;

  static char wavHeader[] = { 0x52, 0x49, 0x46, 0x46, 0x24, 0xbc, 0x5d, 0x01,
			      0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
			      0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
			      0x44, 0xac, 0x00, 0x00, 0x10, 0xb1, 0x02, 0x00,
			      0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61,
			      0x00, 0xbc, 0x5d, 0x01 };
  FILE *fp;

  if ( (fp = fopen( OPENINGFILENAME, "rb" )) == NULL ) {
    perror( "BGM file open failed\n" );
  }

#ifdef WIN32
  m_bgmSound = (char *)GlobalAlloc( GMEM_SHARE, 32*1024*1024 );
#else
  m_bgmSound = (char *)malloc( 32*1024*1024 );
#endif

  memcpy( m_bgmSound, wavHeader, 44 );

  if ( ov_open( fp, &vf, NULL, 0 ) < 0 ) {
    perror( "ov_open() failed\n" );
  }

  while (1) {
    old_section = current_section;
    ret = ov_read(&vf, m_bgmSound+44+total, 32*1024*1024, is_big_endian,
		  2, 1, &current_section);
    if ( ret <= 0 )
      break;

    total += ret;
  }
#endif

  return 0;
}

long
Sound::InitBGM( char *filename ) {
#ifdef HAVE_LIBVORBIS
  SDL_mutexP( loadMutex );
#ifdef HAVE_LIBSDL_MIXER
  m_sound[SOUND_OPENING] = Mix_QuickLoad_WAV( (Uint8 *)m_bgmSound );
  if ( m_sound[SOUND_OPENING] == 0 )
    printf( "%s\n", SDL_GetError() );
#endif
  SDL_mutexV( loadMutex );
#endif

  return 0;
}

// It is better to move it to other thread
long
Sound::PlayBGM() {
#ifdef HAVE_LIBVORBIS
#ifdef WIN32
  char *data;

  data = (char *)GlobalLock( m_bgmSound );
  PlaySound( data, NULL, SND_MEMORY|SND_ASYNC ); 
  GlobalUnlock( m_bgmSound );
#elif defined(HAVE_LIBSDL_MIXER)
  Mix_PlayChannel( 1, m_sound[SOUND_OPENING], 0 );
#endif
#endif

  return 0;
}

long
Sound::StopBGM() {
#ifdef HAVE_LIBVORBIS
#ifdef WIN32
  char *data;

  data = (char *)GlobalLock( m_bgmSound );
  PlaySound( NULL, NULL, SND_NODEFAULT ); 
  GlobalUnlock( m_bgmSound );
#elif defined(HAVE_LIBSDL_MIXER)
  Mix_FadeOutChannel( 1, 2000 );
#endif
#endif

  return 0;
}

long
Sound::SkipBGM() {
#ifdef HAVE_LIBVORBIS
#endif

  return 0;
}
