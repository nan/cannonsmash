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

#ifdef HAVE_LIBESD
#include <esd.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#endif
#ifdef WIN32
#include <io.h>
#endif

#if HAVE_LIBPTHREAD
pthread_mutex_t bgmMutex = PTHREAD_MUTEX_INITIALIZER;
#endif

extern long mode;

Sound::Sound() {
#ifdef HAVE_LIBESD
  m_fd[0] = m_fd[1] = m_fd[2] = m_fd[3] = m_fd[4] = -1;
  for ( int i = 0 ; i < 16 ; i++ ) {
    m_sound[i] = 0;
  }
  m_sndfd = 0;

  m_ossfd = -1;

  m_soundMode = SOUND_ESD;
#elif defined(WIN32)
  m_soundMode = SOUND_WIN32;
#else
  m_soundMode = SOUND_NONE;
#endif
}

Sound::~Sound() {
#ifdef HAVE_LIBSDL_MIXER
  for ( int i = 0 ; i < 16 ; i++ ) {
    if ( m_sound[i] != 0 ) {
      Mix_FreeChunk( m_sound[i] );
    }
  }
#else
  for ( int i = 0 ; i < 16 ; i++ ) {
    if ( m_sound[i] != 0 ) {
#ifdef WIN32
      GlobalFree( m_sound[i] );
#else
      free( m_sound[i] );
#endif
    }
  }
#endif

#ifdef HAVE_LIBESD
  if ( m_fd[0] >= 0 )
    esd_close( m_fd[0] );
  if ( m_fd[1] >= 0 )
    esd_close( m_fd[1] );
  if ( m_fd[2] >= 0 )
    esd_close( m_fd[2] );
  if ( m_fd[3] >= 0 )
    esd_close( m_fd[3] );
  if ( m_fd[4] >= 0 )
    esd_close( m_fd[4] );
  m_fd[0] = m_fd[1] = m_fd[2] = m_fd[3] = m_fd[4] = -1;

  if ( m_ossfd >= 0 )
    close( m_ossfd );
#endif
}

bool
Sound::Init() {
#ifdef HAVE_LIBSDL_MIXER
  if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
    perror( "SDL initialize failed\n" );
    return false;
  }

  if ( Mix_OpenAudio( 44100, AUDIO_S16SYS, 2, 4096 ) < 0 ) {
    perror( "SDL Mix_OpenAudio failed\n" );
  }
#elif defined(HAVE_LIBESD)
  int rate, fmt, stereo;

  switch ( m_soundMode ) {
  case SOUND_OSS:
    for ( int i = 0 ; i < 10 ; i++ ) {	// Mmm, what's wrong with it?
      if ( (m_ossfd = open( "/dev/dsp", O_RDWR )) < 0 )
	sleep(1);
      else
	break;
    }

    if ( m_ossfd < 0 ) {
      perror( "open" );
      return false;
    }

    fmt = AFMT_S16_LE;
    if ( ioctl( m_ossfd, SNDCTL_DSP_SETFMT, &fmt ) < 0 ) {
      perror( "SNDCTL_DSP_SETFMT" );
      return false;
    }

    rate = 44100;
    if ( ioctl( m_ossfd, SNDCTL_DSP_SPEED, &rate ) < 0 ) {
      perror( "SNDCTL_DSP_SPEED" );
      return false;
    }

    stereo = 1;
    if ( ioctl( m_ossfd, SNDCTL_DSP_STEREO, &stereo ) < 0 ) {
      perror( "SNDCTL_DSP_STEREO" );
      return false;
    }
    break;
  case SOUND_ESD:
    // ESD対応. とりあえず. 
    m_fd[0] = esd_play_stream( ESD_BITS16|ESD_STEREO|ESD_SAMPLE, 44100, NULL,
			       NULL );
    m_fd[1] = esd_play_stream( ESD_BITS16|ESD_STEREO|ESD_SAMPLE, 44100, NULL,
			       NULL );
    m_fd[2] = esd_play_stream( ESD_BITS16|ESD_STEREO|ESD_SAMPLE, 44100, NULL,
			       NULL );
    m_fd[3] = esd_play_stream( ESD_BITS16|ESD_STEREO|ESD_SAMPLE, 44100, NULL,
			       NULL );
    m_fd[4] = esd_play_stream( ESD_BITS16|ESD_STEREO|ESD_SAMPLE, 44100, NULL,
			       NULL );
    break;
  }
#endif

  int fd;

#ifdef WIN32
  fd = open( "wav/racket.wav", O_RDONLY );
  if ( m_sound[SOUND_RACKET] == 0 ) {
    m_sound[SOUND_RACKET] = (char *)GlobalAlloc( GMEM_SHARE, 65536 );
  }
  m_soundSize[SOUND_RACKET] = read( fd, m_sound[SOUND_RACKET], 65536 );
  close( fd );

  fd = open( "wav/table.wav", O_RDONLY );
  if ( m_sound[SOUND_TABLE] == 0 ) {
    m_sound[SOUND_TABLE] = (char *)GlobalAlloc( GMEM_SHARE, 65536 );
  }
  m_soundSize[SOUND_TABLE] = read( fd, m_sound[SOUND_TABLE], 65536 );
  close( fd );

  fd = open( "wav/click.wav", O_RDONLY );
  if ( m_sound[SOUND_CLICK] == 0 ) {
    m_sound[SOUND_CLICK] = (char *)GlobalAlloc( GMEM_SHARE, 65536 );
  }
  m_soundSize[SOUND_CLICK] = read( fd, m_sound[SOUND_CLICK], 65536 );
  close( fd );
#elif defined(HAVE_LIBESD)
  fd = open( "wav/racket.wav", O_RDONLY );
  if ( m_sound[SOUND_RACKET] == 0 ) {
    m_sound[SOUND_RACKET] = (char *)malloc( 65536 );
  }
  m_soundSize[SOUND_RACKET] = read( fd, m_sound[SOUND_RACKET], 624 );
  m_soundSize[SOUND_RACKET] = read( fd, m_sound[SOUND_RACKET], 65536 );
  close( fd );

  fd = open( "wav/table.wav", O_RDONLY );
  if ( m_sound[SOUND_TABLE] == 0 ) {
    m_sound[SOUND_TABLE] = (char *)malloc( 65536 );
  }
  m_soundSize[SOUND_TABLE] = read( fd, m_sound[SOUND_TABLE], 624 );
  m_soundSize[SOUND_TABLE] = read( fd, m_sound[SOUND_TABLE], 65536 );
  close( fd );

  fd = open( "wav/click.wav", O_RDONLY );
  if ( m_sound[SOUND_CLICK] == 0 ) {
    m_sound[SOUND_CLICK] = (char *)malloc( 65536 );
  }
  m_soundSize[SOUND_CLICK] = read( fd, m_sound[SOUND_CLICK], 624 );
  m_soundSize[SOUND_CLICK] = read( fd, m_sound[SOUND_CLICK], 65536 );
  close( fd );
#elif defined(HAVE_LIBSDL_MIXER)
  m_sound[SOUND_RACKET] = Mix_LoadWAV( "wav/racket.wav" );
  m_sound[SOUND_TABLE] = Mix_LoadWAV( "wav/table.wav" );
  m_sound[SOUND_CLICK] = Mix_LoadWAV( "wav/click.wav" );
#endif

  return true;
}

bool
Sound::Play( char *sndData, long size ) {
  if ( mode != MODE_MULTIPLAY ) {
#ifdef HAVE_LIBESD
    switch ( m_soundMode ) {
    case SOUND_ESD:
      write( m_fd[m_sndfd++], sndData, size );

      if ( m_sndfd > 4 )
	m_sndfd = 0;
      break;
    case SOUND_OSS:
      write( m_ossfd, sndData, size );
      break;
    }
#endif
  }

  return true;
}

bool
Sound::Play( long soundID ) {
  if ( mode != MODE_MULTIPLAY ) {
#ifdef WIN32
    char *data;

    data = (char *)GlobalLock( m_sound[soundID] );
    PlaySound( data, NULL, SND_MEMORY|SND_ASYNC ); 
    GlobalUnlock( m_sound[soundID] );
#elif defined(HAVE_LIBESD)
    switch ( m_soundMode ) {
    case SOUND_ESD:
      write( m_fd[m_sndfd++], m_sound[soundID], m_soundSize[soundID] );

      if ( m_sndfd > 4 )
	m_sndfd = 0;
      break;
    case SOUND_OSS:
      write( m_ossfd, m_sound[soundID], m_soundSize[soundID] );
      break;
    }
#elif defined(HAVE_LIBSDL_MIXER)
    Mix_PlayChannel( -1, m_sound[soundID], 0 );
#endif
  }

  return true;
}

long
Sound::GetSoundMode() {
  return m_soundMode;
}

bool
Sound::SetSoundMode( long mode ) {
  if ( mode == m_soundMode )
    return true;

#ifdef HAVE_LIBESD
  switch ( m_soundMode ) {
  case SOUND_ESD:
    if ( m_fd[0] >= 0 )
      esd_close( m_fd[0] );
    if ( m_fd[1] >= 0 )
      esd_close( m_fd[1] );
    if ( m_fd[2] >= 0 )
      esd_close( m_fd[2] );
    if ( m_fd[3] >= 0 )
      esd_close( m_fd[3] );
    if ( m_fd[4] >= 0 )
      esd_close( m_fd[4] );
    m_fd[0] = m_fd[1] = m_fd[2] = m_fd[3] = m_fd[4] = -1;
    break;
  case SOUND_OSS:
    if ( m_ossfd >= 0 )
      close( m_ossfd );
  }

  m_soundMode = mode;
  Init();
#endif

  return true;
}

long
Sound::InitBGM( char *filename ) {
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

  return 0;
}

// 別スレッド化したい
long
Sound::PlayBGM() {
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

  return 0;
}

long
Sound::SkipBGM() {
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

  return 0;
}
