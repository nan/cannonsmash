/**
 * @file
 * @brief Definition of Player class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000-2004  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _Player_
#define _Player_
#include "PlayerView.h"
#include "PlayerView2D.h"

#include <vector>
#include "matrix"
typedef Vector<3, double> vector3d;
typedef Vector<2, double> vector2d;

// m_playerType
#define PLAYER_PROTO		0	// Prototype player
#define PLAYER_PENATTACK	1	// Pen Attack
#define PLAYER_SHAKECUT		2	// Cut
#define PLAYER_PENDRIVE		3	// Pen Drive


// m_swingType
#define SWING_NORMAL	0	// 
#define SWING_POKE	1	// push?
#define SWING_SMASH	2	// 
#define SWING_DRIVE	3	// 
#define SWING_CUT	4	// 
#define SWING_BLOCK	5	// 


class PlayerView;
class HitMark;
class Ball;

/**
 * Player class is a base class of player classes (PenAttack, PenDrive, etc.). 
 */
class Player {
  friend class Howto;
  friend class Opening;
  friend class OpeningView;
public:
  Player();
  Player( long side );
  Player( long playerType, long side, const vector3d x, const vector3d v,
	  long status, long swing, long swingType, bool swingSide, long afterSwing,
	  long swingError, const vector2d target, const vector3d eye,
	  long pow, const vector2d spin, double stamina, long statusMax );

  virtual ~Player();

  void operator=(Player&);

  static Player* Create( long player, long side, long type );

  virtual bool Init();

  //virtual bool Reset( struct PlayerData *p );
  virtual bool Reset( Player *p );

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual bool AddStatus( long diff );

  virtual View *GetView() { return m_View; }		///< Getter method of m_View

  virtual long   GetSide() { return m_side; }		///< Getter method of m_side
  virtual long   GetPlayerType() { return m_playerType; }///< Getter method of m_playerType

  virtual vector3d GetX() { return m_x; }		///< Getter method of m_x
  virtual vector3d GetV() { return m_v; }		///< Getter method of m_v
  virtual long   GetPower() { return m_pow; }		///< Getter method of m_pow
  virtual vector2d GetSpin() { return m_spin; }		///< Getter method of m_spin
  virtual vector2d GetTarget() { return m_target; }	///< Getter method of m_target
  virtual vector3d GetEye() { return m_eye; }		///< Getter method of m_eye
  virtual vector3d GetLookAt() { return m_lookAt; }	///< Getter method of m_lookAt
  virtual double GetStamina() { return m_stamina; }	///< Getter method of m_stamina
  virtual long   GetStatus() { return m_status; }	///< Getter method of m_status
  virtual long   GetSwing() { return m_swing; }		///< Getter method of m_swing
  virtual long   GetSwingType() { return m_swingType; }	///< Getter method of m_swingType
  virtual bool   GetSwingSide() { return m_swingSide; }	///< Getter method of m_swingSide
  virtual long   GetSwingError() { return m_swingError; }///< Getter method of m_swingError
  virtual long   GetAfterSwing() { return m_afterSwing; }///< Getter method of m_afterSwing

  virtual long   GetDragX() { return m_dragX; }		///< Getter method of m_dragX
  virtual long   GetDragY() { return m_dragY; }		///< Getter method of m_dragY

  // true  -> forehand
  // false -> backhand
  virtual bool ForeOrBack();

  virtual bool Warp( const vector3d &x, const vector3d &v );
  virtual bool ExternalSwing( long pow, const vector2d &spin,
			      long swingType, long swing );

  virtual bool Warp( char *buf );
  virtual bool ExternalSwing( char *buf );

  virtual char * SendSwing( char *buf );
  virtual char * SendLocation( char *buf );
  virtual bool SendAll( int sd );

  virtual bool GetModifiedTarget( vector2d &target );

  virtual void CalcLevel( Ball *ball, double &diff, double &level, double &maxVy );

  long StatusBorder();
protected:
  long m_playerType;	///< Player type

  long m_side;		/**< Player side
			 * <ul>
			 *  <li> 1  --- ( y < 0 )
			 *  <li> -1 --- ( y > 0 )
			 * </ul>
			 */

  vector3d m_x;		///< player location
  vector3d m_v;		///< player velocity

  long m_status;	///< status gauge
  long m_swing;		///< swing status
  long m_swingType;	///< swing type
  bool m_swingSide;	///< forehand or backhand
  long m_afterSwing;	///< after swing stop penalty
  long m_swingError;	/**< Error when hitting
			 * <ul>
			 *  <li> 0 --- Perfect
			 *  <li> 1 --- Great
			 *  <li> 2 --- Good
			 *  <li> 3 --- Boo
			 *  <li> 4 --- Miss
			 * </ul>
			 */
  vector2d m_target;	///< location of target circle

  vector3d m_eye;	///< camera location

  vector3d m_lookAt;	///< target of camera

  long m_pow;		///< power
  vector2d m_spin;	///< {sidespin, topspin/backspin}

  double m_stamina;	///< Not used

  long m_statusMax;	///< Max status value

  long m_dragX;		///< Mouse drag
  long m_dragY;		///< Mouse drag

  PlayerView* m_View;	///< Reference to view class

  vector3d m_lastSendX;	///< m_x which is sent to the opponent recently. 
  vector3d m_lastSendV;	///< m_v which is sent to the opponent recently. 
  long m_lastSendCount;	///< TICKs from when something is sent to the opponent recently. 

  virtual bool KeyCheck( SDL_keysym *KeyHistory, long *MouseXHistory,
			 long *MouseYHistory, unsigned long *MouseBHistory,
			 int Histptr );
  virtual bool Swing( long power );
  virtual bool StartSwing( long power );

  virtual bool HitBall();

  virtual bool SwingError();

  void UpdateLastSend();

  void AddError( vector3d &v );
};

#endif // _Player_
