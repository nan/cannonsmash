/**
 * @file
 * @brief Definition of HowtoView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2002  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _HowtoView_
#define _HowtoView_
#include "View.h"
#include "LoadImage.h"

class Howto;

/**
 * View class for Howto. 
 */
class HowtoView : public View {
public:
  HowtoView();
  virtual ~HowtoView();

  bool Init( Howto * );

  virtual bool Redraw();
  virtual bool RedrawAlpha();

  static GLuint m_textures[4];
protected:
  Howto       *m_howto;			///< Reference to Howto object. 
  ImageData    m_mouse[4];		///< Mouse texture handler. 
  ImageData    m_howtoText[8];		///< Explanation text texture handler. 
  ImageData    m_arrow[4];		///< Arrow mark texture handler. 
  GLuint       m_keyboard[1];		///< Keyboard texture handler.  
};

#endif	// _HowtoView_
