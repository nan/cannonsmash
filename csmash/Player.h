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

#ifndef _Player_
#define _Player_
#include "PlayerView.h"

// m_playerType
#define PLAYER_PROTO		0	// $B%W%m%H%?%$%W(BPlayer
#define PLAYER_PENATTACK	1	// $BN"%Z%sB.96(B
#define PLAYER_SHAKECUT		2	// $B%+%C%H(B
#define PLAYER_PENDRIVE		3	// $B%Z%s%I%i%$%V(B


// m_swingType
#define SWING_NORMAL	0	// $B4pACBG$A(B
#define SWING_POKE	1	// $B%D%C%D%-(B($BK\Ev$O(Bpush$B$i$7$$$,(B...)
#define SWING_SMASH	2	// $B6/BG(B
#define SWING_DRIVE	3	// $B%I%i%$%V(B
#define SWING_CUT	4	// $B%+%C%H(B
#define SWING_BLOCK	5	// $B%V%m%C%/(B, $B%9%H%C%W(B

// Player Class$B$NDj5A(B

class PlayerView;
class HitMark;
class Ball;

class Player {
  friend class Howto;
  friend class Opening;
  friend class OpeningView;
public:
  Player();
  Player( long side );
  Player( long playerType, long side, double x, double y, double z, 
	  double vx, double vy, double vz,long status, long swing, 
	  long swingType, bool swingSide, long afterSwing, long swingError, 
	  double targetX, double targetY, double eyeX, double eyeY,
	  double eyeZ, long pow, double spin, double stamina );

  virtual ~Player();

  void operator=(Player&);

  static Player* Create( long player, long side, long type );

  virtual bool Init();

  //virtual bool Reset( struct PlayerData *p );
  virtual bool Reset( Player *p );

  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual bool AddStatus( long diff );

  virtual long   GetSide() { return m_side; }
  virtual long   GetPlayerType() { return m_playerType; }

  virtual double GetX() { return m_x; }
  virtual double GetY() { return m_y; }
  virtual double GetZ() { return m_z; }
  virtual double GetVX() { return m_vx; }
  virtual double GetVY() { return m_vy; }
  virtual double GetVZ() { return m_vz; }
  virtual long   GetPower() { return m_pow; }
  virtual double GetSpin() { return m_spin; }
  virtual double GetTargetX() { return m_targetX; }
  virtual double GetTargetY() { return m_targetY; }
  virtual double GetEyeX() { return m_eyeX; }
  virtual double GetEyeY() { return m_eyeY; }
  virtual double GetEyeZ() { return m_eyeZ; }
  virtual double GetLookAtX() { return m_lookAtX; }
  virtual double GetLookAtY() { return m_lookAtY; }
  virtual double GetLookAtZ() { return m_lookAtZ; }
  virtual double GetStamina() { return m_stamina; }
  virtual long   GetStatus() { return m_status; }
  virtual long   GetSwing() { return m_swing; }
  virtual long   GetSwingType() { return m_swingType; }
  virtual bool   GetSwingSide() { return m_swingSide; }
  virtual long   GetSwingError() { return m_swingError; }
  virtual long   GetAfterSwing() { return m_afterSwing; }

  virtual long   GetDragX() { return m_dragX; }
  virtual long   GetDragY() { return m_dragY; }

  virtual bool   GetShoulder( double &x, double &y, double &deg );
  virtual bool   GetElbow( double &degx, double& degy );
  virtual bool   GetHand( double &degx, double &degy, double &degz );

// Get{Shoulder|Elbow|Hand}$B$+$i8F$S=P$5$l$k(B. 
// $B%\!<%k$N0LCV$+$i%U%)%"$GBG$D$+%P%C%/$+$rA*Br(B. 
// true  -> $B%U%)%"(B
// false -> $B%P%C%/(B
  virtual bool ForeOrBack();

  virtual bool Warp( double x, double y, double z,
		     double vx, double vy, double vz );
  virtual bool ExternalSwing( long pow, double spin, long swingType, long swing );

  virtual bool Warp( char *buf );
  virtual bool ExternalSwing( char *buf );

  virtual char * SendSwing_forNODELAY( char *buf );
  virtual char * SendLocation_forNODELAY( char *buf );
  virtual bool SendLocation( int sd );
  virtual bool SendAll( int sd );

  virtual bool GetModifiedTarget( double &targetX, double &targetY );	// $BK\Mh$J$i(B pure virtual

  virtual void CalcLevel( Ball *ball, double &diff, double &level, double &maxVy );	// $BK\Mh$J$i(B pure virtual
protected:
  long m_playerType;	// Player$B$N<oN`(B

  long m_side;		// 1  --- $B<jA0B&(B( y < 0 )
			// -1 --- $B1|(B( y > 0 )

  double m_x;		// player$B$N0LCV(B($B;kE@$N0LCV(B)
  double m_y;
  double m_z;
  double m_vx;		// player$B$NB.EY(B
  double m_vy;
  double m_vz;

  long m_status;	// $BBN@*%2!<%8CM(B
  long m_swing;		// $B%9%$%s%0$N>uBV(B
  long m_swingType;	// $B%9%$%s%0$N<oN`(B
  bool m_swingSide;	// $B%U%)%"$+%P%C%/$+(B
  long m_afterSwing;	// $B%9%$%s%08e$N9ED>;~4V(B
  long m_swingError;	// $B%\!<%k$rBG$C$?$H$-$N8m:9(B. 
                        // 0 --- Perfect
                        // 1 --- Great
                        // 2 --- Good
                        // 3 --- Boo
                        // 4 --- Miss
  double m_targetX;	// $BBG5e$NL\I8Mn2<E@(B
  double m_targetY;	// $BBG5e$NL\I8Mn2<E@(B

  double m_eyeX;	// $B;kE@(B
  double m_eyeY;
  double m_eyeZ;

  double m_lookAtX;
  double m_lookAtY;
  double m_lookAtZ;

  long m_pow;		// $B<e(B, $BCf(B, $B6/(B
  double m_spin;	// $B%H%C%W(B/$B%P%C%/%9%T%s(B

  double m_stamina;	// $BBNNO(B

  long m_dragX;
  long m_dragY;		// $B%^%&%9$N%I%i%C%0NL(B

  PlayerView* m_View;

  HitMark *m_hitMark;

// Move$B$+$i8F$S=P$5$l$k(B
  virtual bool KeyCheck( unsigned long *KeyHistory, long *MouseXHistory,
			 long *MouseYHistory, unsigned long *MouseBHistory,
			 int Histptr );		// $B%-!<F~NO=hM}(B
  virtual bool Swing( long power );	// $BK\Mh$J$i(B pure virtual
  virtual bool StartSwing( long power );// $BK\Mh$J$i(B pure virtual

  virtual bool HitBall();		// $BK\Mh$J$i(B pure virtual

  virtual bool SwingError();
};

#endif // _Player_
