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

#ifndef _PenAttack_
#define _PenAttack_

// PenAttack (Pen, right)$B$NDj5A(B

class PenAttack : public Player {
public:
  PenAttack();
  PenAttack(long side);
  PenAttack( long playerType, long side, double x, double y, double z, 
	     double vx, double vy, double vz,long status, long swing, 
	     long swingType, long afterSwing, long swingError, 
	     double targetX, double targetY, double eyeX, double eyeY,
	     double eyeZ, long pow, double spin, double stamina );
  virtual ~PenAttack();

  virtual bool AddStatus( long diff );

  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  virtual bool GetModifiedTarget( double &targetX, double &targetY );
protected:
  virtual bool Swing( long spin );
  virtual bool StartSwing( long spin );

  virtual bool HitBall();

private:
  bool SwingType( Ball *ball, long spin );
};

#endif // _PenAttack__
