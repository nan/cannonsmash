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

#ifndef _Control_
#define _Control_

class Control {
public:
  Control();
  virtual ~Control();

  virtual bool Init() = 0;
  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr ) = 0;
  virtual bool LookAt( double &srcX, double &srcY, double &srcZ,
		       double &destX, double &destY, double &destZ ) = 0;

  virtual bool IsPlaying() = 0;
};

#endif	// _Control_
