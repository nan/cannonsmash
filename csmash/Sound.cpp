/**
 * @file
 * @brief Implementation of Sound class. 
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

#include "ttinc.h"
#include "Sound.h"
#include "Control.h"

#ifdef WIN32
#include <io.h>
#endif

extern long mode;
extern SDL_mutex *loadMutex;

Sound* Sound::m_theSound = NULL;

/**
 * Default constructor. 
 */
Sound::Sound() {
#ifdef HAVE_LIBSDL_MIXER
  m_queuehead = m_queuetail = 0;
#else
  m_soundMode = SOUND_NONE;
#endif
}

/**
 * Destructor. 
 * Free registered sounds. 
 */
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

/**
 * Getter method of singleton Sound object. 
 */
Sound*
Sound::TheSound() {
  if ( Sound::m_theSound )
    return Sound::m_theSound;
  else
    return (Sound::m_theSound = new Sound());
}

/**
 * Initializer method. 
 * Load sound files. 
 * 
 * @param sndMode sound mode (SDL or none). 
 * @return returns true if succeeds. 
 */
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
    perror( _("SDL Mix_OpenAudio failed\n") );
  }

  m_sound[SOUND_RACKET] = Mix_LoadWAV( "wav/racket.wav" );
  m_sound[SOUND_TABLE] = Mix_LoadWAV( "wav/table.wav" );
  m_sound[SOUND_CLICK] = Mix_LoadWAV( "wav/click.wav" );
  m_sound[SOUND_LOVEALL] = Mix_LoadWAV( "wav/loveall.wav" );
  m_sound[SOUND_ALL] = Mix_LoadWAV( "wav/all.wav" );

  m_score[0] = Mix_LoadWAV( "wav/zero.wav" );
  m_score[1] = Mix_LoadWAV( "wav/one.wav" );
  m_score[2] = Mix_LoadWAV( "wav/two.wav" );
  m_score[3] = Mix_LoadWAV( "wav/three.wav" );
  m_score[4] = Mix_LoadWAV( "wav/four.wav" );
  m_score[5] = Mix_LoadWAV( "wav/five.wav" );
  m_score[6] = Mix_LoadWAV( "wav/six.wav" );
  m_score[7] = Mix_LoadWAV( "wav/seven.wav" );
  m_score[8] = Mix_LoadWAV( "wav/eight.wav" );
  m_score[9] = Mix_LoadWAV( "wav/nine.wav" );
  m_score[10] = Mix_LoadWAV( "wav/ten.wav" );
  m_score[11] = Mix_LoadWAV( "wav/eleven.wav" );
  m_score[12] = Mix_LoadWAV( "wav/twelve.wav" );
  m_score[13] = Mix_LoadWAV( "wav/thirteen.wav" );
  m_score[14] = Mix_LoadWAV( "wav/fourteen.wav" );
  m_score[15] = Mix_LoadWAV( "wav/fifteen.wav" );
  m_score[16] = Mix_LoadWAV( "wav/sixteen.wav" );
  m_score[17] = Mix_LoadWAV( "wav/seventeen.wav" );
  m_score[18] = Mix_LoadWAV( "wav/eighteen.wav" );
  m_score[19] = Mix_LoadWAV( "wav/nineteen.wav" );
  m_score[20] = Mix_LoadWAV( "wav/twenty.wav" );
  m_score[30] = Mix_LoadWAV( "wav/thirty.wav" );

  Mix_ChannelFinished( Sound::PlayFinishHandler );
#endif

  return true;
}

/**
 * Cleanup registered sound handlers. 
 */
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

  Mix_ChannelFinished( NULL );
  Mix_CloseAudio();
#endif
}

/**
 * Play sound. 
 * 
 * @param soundID ID of sound handler to be played. 
 * @param x location of the source of sound. 
 */
bool
Sound::Play( long soundID, vector3d x ) {
  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
  vector3d srcX, destX;
  double angle, destAngle;

  Control::TheControl()->LookAt( srcX, destX );

  destX -= srcX;
  x -= srcX;

  destAngle = acos( destX[0]/destX.len() );
  if ( destX[1] < 0 )
    destAngle += 3.14159265;

  angle = acos( x[0]/x.len() );
  if ( x[1] < 0 )
    angle += 3.14159265;

  angle -= destAngle;

  Mix_SetPosition( 0, (int)(angle*180.0/3.14159265), (unsigned char)x.len()*8 );
  Mix_PlayChannel( 0, m_sound[soundID], 0 );
#endif

  return true;
}

/**
 * Play score reading voice. 
 * 
 * @param score1 score of near side
 * @param score2 score of far side
 * @return returns true if succeeds. 
 */
bool
Sound::PlayScore( long score1, long score2 ) {
  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
  vector3d x = vector3d((const double[]){-TABLEWIDTH/2-0.5, 0, 0});
  vector3d srcX, destX;
  double angle, destAngle;

  Control::TheControl()->LookAt( srcX, destX );

  destX -= srcX;
  x -= srcX;

  destAngle = acos( destX[0]/destX.len() );
  if ( destX[1] < 0 )
    destAngle += 3.14159265;

  angle = acos( x[0]/x.len() );
  if ( x[1] < 0 )
    angle += 3.14159265;

  angle -= destAngle;

  Mix_SetPosition( 0, (int)(angle*180.0/3.14159265), x.len()*8 );
  PlayNumber( score1 );
  if ( score1 == score2 )
    PlayBlocking( 1, m_sound[SOUND_ALL] );
  else
    PlayNumber( score2 );
#endif

  return true;
}

/**
 * Play number voice. 
 * 
 * @param number number to be played. 
 * @return returns true if succees. 
 */
bool
Sound::PlayNumber( long number ) {
  if ( m_soundMode == SOUND_NONE )
    return true;

#ifdef HAVE_LIBSDL_MIXER
  number %= 40;
  if ( number > 30 ) {
    PlayBlocking( 1, m_score[30] );
    PlayBlocking( 1, m_score[number-30] );
  } else if ( number > 20 ) {
    PlayBlocking( 1, m_score[20] );
    PlayBlocking( 1, m_score[number-20] );
  } else
    PlayBlocking( 1, m_score[number] );
#endif

  return true;
}

#ifdef HAVE_LIBSDL_MIXER
/**
 * Play the sound on blocking mode
 * 
 * @param channel sound channel ID
 * @param chunk sound handler to be played
 * @return returns true if succeeds. 
 */
bool
Sound::PlayBlocking( int channel, Mix_Chunk *chunk ) {
  if ( !Mix_Playing( channel ) ) {
    Mix_PlayChannel( channel, chunk, 0 );
  } else {
    m_chunkqueue[m_queuetail%SOUND_QUEUESIZE] = chunk;
    m_queuetail++;
  }

  return true;
}

/**
 * Callback method to play queued sound. 
 * This method checks the queue of sound handler. If a sound handler is queued,
 * it is played. 
 * 
 * @param channel sound channel ID
 */
void
Sound::PlayFinishHandler( int channel ) {
  if ( channel != 1 )
    return;

  if ( Sound::TheSound()->m_queuehead != Sound::TheSound()->m_queuetail ) {
    Mix_PlayChannel( channel,
		     Sound::TheSound()->
		     m_chunkqueue[Sound::TheSound()->
				 m_queuehead%SOUND_QUEUESIZE],
		     0 );
    Sound::TheSound()->m_queuehead++;
  }
}
#endif

/**
 * Getter method of m_soundMode
 */
long
Sound::GetSoundMode() {
  return m_soundMode;
}

/**
 * Load BGM file. 
 */
long
Sound::InitBGM( char *filename ) {
  if ( m_soundMode == SOUND_NONE )
    return 0;

#ifdef HAVE_LIBSDL_MIXER
  m_opening = Mix_LoadMUS( OPENINGFILENAME );

  if ( m_opening == 0 )
    printf( "%s\n", SDL_GetError() );
#endif

  return 0;
}

/**
 * Play BGM. 
 */
long
Sound::PlayBGM() {
  if ( m_soundMode == SOUND_NONE )
    return 0;

#ifdef HAVE_LIBSDL_MIXER
  Mix_PlayMusic( m_opening, 1 );
#endif

  return 0;
}

/**
 * Fade out BGM when it should be stopped. 
 */
long
Sound::StopBGM() {
  if ( m_soundMode == SOUND_NONE )
    return 0;

#ifdef HAVE_LIBSDL_MIXER
  Mix_FadeOutMusic( 2000 );
#endif

  return 0;
}
