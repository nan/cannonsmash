/**
 * @file
 * @brief Definition of HitMark class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

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

/**
 * Hit mark manager class. 
 * This class is used to show hit mark. Although HitMark is a subclass
 * of View class, HitMark is independent, not attached to model class. 
 * And an instance of HitMark is generated dynamically when the player
 * hit the ball. After the instance is created, it is attached to the 
 * list of View classes. Then, for 50msec, HitMark shows hit mark. 
 * After that, Redraw method of HitMark returns false, and BaseView class
 * deletes the instance of HitMark. 
 */
class HitMark : public View {
public:
  HitMark();
  static bool Init();
  bool Hit( vector3d x, vector3d v, long swingError );
  virtual ~HitMark();

  virtual bool Redraw();
  virtual bool RedrawAlpha();

  static GLuint       m_textures[2];	///< texture handler. 
private:
  vector3d m_x;				///< location of the hit mark. 
  vector3d m_v;				///< velocity of the hit mark. 
  long m_swingError;			///< indicates which type of hit mark should be shown. 

  struct timeb startTime;		///< time when the instance is created. 
  long m_time;				///< duration of the time from the instance is created. 
};

#endif	// _HitMark
