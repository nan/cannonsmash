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

#ifdef HAVE_LIBMAD
# include <sys/mman.h>
extern "C"{
#include <mad.h>
};
struct mad_pcm {
  unsigned int samplerate;		/* sampling frequency (Hz) */
  unsigned short channels;		/* number of channels */
  unsigned short length;		/* number of samples per channel */
  mad_fixed_t samples[2][1152];	/* PCM output samples */
};
#endif

#ifdef WIN32
#include <io.h>
#endif

extern long mode;
extern Sound theSound;

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


// From here, copied from minimad.c
#ifdef HAVE_LIBMAD
static
enum mad_flow input(void *data,
		    struct mad_stream *stream)
{
  struct buffer *buffer = (struct buffer *)data;

  if (!buffer->length)
    return MAD_FLOW_STOP;

  mad_stream_buffer(stream, buffer->start, buffer->length);

  buffer->length = 0;

  return MAD_FLOW_CONTINUE;
}

/* utility to scale and round samples to 16 bits */

static inline
signed int scale(mad_fixed_t sample)
{
  /* round */
  sample += (1L << (MAD_F_FRACBITS - 16));

  /* clip */
  if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
  else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

  /* quantize */
  return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/* 3. called to process output */

static
enum mad_flow output(void *data,
		     struct mad_header const *header,
		     struct mad_pcm *pcm)
{
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  static int i=44;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  if ( i > 32*1024*1024-4 )
    return MAD_FLOW_CONTINUE;

  while (nsamples--) {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */

    sample = scale(*left_ch++);
    theSound.m_bgmSound[i++] = (sample >> 0) & 0xff;
    theSound.m_bgmSound[i++] = (sample >> 8) & 0xff;

    if (nchannels == 2) {
      sample = scale(*right_ch++);
      theSound.m_bgmSound[i++] = (sample >> 0) & 0xff;
      theSound.m_bgmSound[i++] = (sample >> 8) & 0xff;
    }
  }

  return MAD_FLOW_CONTINUE;
}
#endif

long
Sound::InitBGM( char *filename ) {
#ifdef HAVE_LIBMAD
  static char wavHeader[] = { 0x52, 0x49, 0x46, 0x46, 0x24, 0xbc, 0x5d, 0x01,
			      0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
			      0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00,
			      0x44, 0xac, 0x00, 0x00, 0x10, 0xb1, 0x02, 0x00,
			      0x04, 0x00, 0x10, 0x00, 0x64, 0x61, 0x74, 0x61,
			      0x00, 0xbc, 0x5d, 0x01 };
  int fd;
  void *mp3;

  fd = open( OPENINGFILENAME, O_RDONLY );
  mp3 = malloc( 779656 );
  read( fd, mp3, 779656 );

  m_bgmSound = (char *)malloc( 32*1024*1024 );
  memcpy( m_bgmSound, wavHeader, 44 );

  struct buffer buffer;
  struct mad_decoder decoder;

  buffer.start  = (unsigned char *)mp3;
  buffer.length = 779656;

  mad_decoder_init(&decoder, &buffer,
		   input, 0, 0, output, 0, 0);

  mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
  mad_decoder_finish(&decoder);
#ifdef HAVE_LIBSDL_MIXER
  m_sound[SOUND_OPENING] =
    //Mix_LoadWAV( "danslatristesse2-48.wav" );
    Mix_LoadWAV_RW( SDL_RWFromMem( m_bgmSound, 32*1024*1024 ), 1);
  if ( m_sound[SOUND_OPENING] == 0 )
    printf( "%s\n", SDL_GetError() );
#endif
#endif

#ifdef WIN32
  int fd;

  fd = open( OPENINGFILENAME, O_RDONLY );
  m_bgmSound = (char *)GlobalAlloc( GMEM_SHARE, 4*1024*1024 );
  read( fd, m_bgmSound, 4*1024*1024 );
  close( fd );
#endif

  return 0;
}

// It is better to move it to other thread
long
Sound::PlayBGM() {
#ifdef HAVE_LIBMAD
#ifdef WIN32
  char *data;

  data = (char *)GlobalLock( m_bgmSound );
  PlaySound( data, NULL, SND_MEMORY|SND_ASYNC ); 
  GlobalUnlock( m_bgmSound );
#elif defined(HAVE_LIBSDL_MIXER)
  Mix_PlayChannel( 1, m_sound[SOUND_OPENING], 0 );
#endif
#elif defined(WIN32)
  char *data;

  data = (char *)GlobalLock( m_bgmSound );
  PlaySound( data, NULL, SND_MEMORY|SND_ASYNC ); 
  GlobalUnlock( m_bgmSound );
#endif

  return 0;
}

long
Sound::SkipBGM() {
#ifdef HAVE_LIBMAD
#endif

  return 0;
}
