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

#ifdef HAVE_LIBESD
#include <esd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#endif
#ifdef WIN32
#include <io.h>
#endif

extern long mode;

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
  if ( Mix_OpenAudio( MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 128 ) < 0 ) {
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
Sound::InitBGM( char *filename ) {
#if 0
#ifdef HAVE_LIBESD
  int filedes[2];
  long pid;

  pipe( filedes );
  if ( (pid = fork()) == 0 ) {
    dup2( filedes[1], 1 );
    execl( "/usr/bin/mpg123", "mpg123", "-q", "-s", filename, NULL );
    return -1;
  }
  m_bgminfd = filedes[0];
  m_bgmoutfd = esd_play_stream( ESD_BITS16|ESD_STEREO|ESD_SAMPLE, 44100, NULL,
				NULL );

  return pid;
#endif

#ifdef WIN32
  int fd;

  fd = open( SOUND_OPENING, O_RDONLY );
  m_bgmSound = (char *)GlobalAlloc( GMEM_SHARE, 4*1024*1024 );
  read( fd, m_bgmSound, 4*1024*1024 );
  close( fd );
#endif
#endif

  return 0;
}

// It is better to move it to other thread
long
Sound::PlayBGM() {
#if 0
#ifdef WIN32
  char *data;

  data = (char *)GlobalLock( m_bgmSound );
  PlaySound( data, NULL, SND_MEMORY|SND_ASYNC ); 
  GlobalUnlock( m_bgmSound );
#elif defined(HAVE_LIBESD)
  switch ( m_soundMode ) {
  case SOUND_ESD:
    char buf[44100*2+2];
    static long bytes = 0;

    fd_set rdfds;
    struct timeval to;

    FD_ZERO( &rdfds );
    FD_SET( m_bgminfd, &rdfds );
    to.tv_sec = to.tv_usec = 0;

    if ( select( m_bgminfd+1, &rdfds, NULL, NULL, &to ) > 0 &&
	 FD_ISSET( m_bgminfd, &rdfds ) ) {
      bytes = read( m_bgminfd, buf, 44100*2*2 );
      if ( bytes > 0 ) {
	write( m_bgmoutfd, buf, bytes );
	return bytes;
      }
    }
  }
#endif
#endif

  return 0;
}

long
Sound::SkipBGM() {
#if 0
#ifdef HAVE_LIBESD
  switch ( m_soundMode ) {
  case SOUND_ESD:
    char buf[4410*2+2];

    long bytes = 0;

    fd_set rdfds;
    struct timeval to;

    FD_ZERO( &rdfds );
    FD_SET( m_bgminfd, &rdfds );
    to.tv_sec = to.tv_usec = 0;

    if ( select( m_bgminfd+1, &rdfds, NULL, NULL, &to ) > 0 &&
	 FD_ISSET( m_bgminfd, &rdfds ) ) {
      bytes = read( m_bgminfd, buf, 4410*2*2 );
      return bytes;
    }
  }
#endif
#endif

  return 0;
}
