/**
 * @file
 * @brief Definition of PlayerSelectView class. 
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

#ifndef _PlayerSelectView_
#define _PlayerSelectView_
#include "View.h"

class PlayerSelect;
class ImageData;

/**
 * PlayerSelectView class is a view class which corresponds to PlayerSelect object. 
 */
class PlayerSelectView : public View {
public:
  PlayerSelectView();
  virtual ~PlayerSelectView();

  virtual bool Init( PlayerSelect * );

  virtual bool Redraw();
  virtual bool RedrawAlpha();
protected:
  PlayerSelect *m_playerSelect;		///< Attached PlayerSelect object. 
  GLuint       m_textures[PLAYERS+1];	///< texture handler of player panels. 
  ImageData    *m_selectPlayer;		///< Image of "Select player". 
};

#endif	// _PlayerSelectView_
