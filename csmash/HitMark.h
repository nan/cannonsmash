/* $Id$ */

// Copyright (C) 2000, 2004  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _HitMark_
#define _HitMark_
#include "View.h"

#include <vector>
#include "matrix"
typedef Vector<3, double> vector3d;
typedef Vector<2, double> vector2d;

class HitMark : public View {
public:
  HitMark();
  static bool Init();
  bool Hit( vector3d x, vector3d v, long swingError );
  virtual ~HitMark();

  virtual bool Redraw();
  virtual bool RedrawAlpha();

  static GLuint       m_textures[2];
private:
  vector3d m_x;
  vector3d m_v;
  long m_swingError;

  struct timeb startTime;
  long m_time;
};

#endif	// _HitMark
