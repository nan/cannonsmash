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
#endif
#ifdef WIN32
#include <io.h>
#endif

Sound::Sound() {
#ifdef HAVE_LIBESD
  m_fd[0] = m_fd[1] = m_fd[2] = m_fd[3] = m_fd[4] = -1;
  for ( int i = 0 ; i < 16 ; i++ ) {
    m_sound[i] = 0;
  }
  m_sndfd = 0;
#endif
}

Sound::~Sound() {
  for ( int i = 0 ; i < 16 ; i++ ) {
    if ( m_sound[i] == 0 ) {
#ifdef WIN32
      GlobalFree( m_sound[i] );
#else
      free( m_sound[i] );
#endif
    }
  }

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
#endif
}

bool
Sound::Init() {
#ifdef HAVE_LIBESD
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
#else
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
#endif

  return true;
}

bool
Sound::Play( char *sndData, long size ) {
#ifdef HAVE_LIBESD
  write( m_fd[m_sndfd++], sndData, size );

  if ( m_sndfd > 4 )
    m_sndfd = 0;
#endif

  return true;
}

bool
Sound::Play( long soundID ) {
#ifdef WIN32
  char *data;

  data = (char *)GlobalLock( m_sound[soundID] );
  PlaySound( data, NULL, SND_MEMORY|SND_ASYNC ); 
  GlobalUnlock( m_sound[soundID] );
#elif defined(HAVE_LIBESD)
  write( m_fd[m_sndfd++], m_sound[soundID], m_soundSize[soundID] );

  if ( m_sndfd > 4 )
    m_sndfd = 0;
#endif

  return true;
}
