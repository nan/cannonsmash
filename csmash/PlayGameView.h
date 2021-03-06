/**
 * @file
 * @brief Definition of PlayGameView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2001, 2002  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _PlayGameView_
#define _PlayGameView_
#include "View.h"

class PlayGame;
class ImageData;

/**
 * PlayGameView class is a view class which corresponds to PlayGame object. 
 */
class PlayGameView : public View {
public:
  PlayGameView();
  virtual ~PlayGameView();

  virtual bool Init( PlayGame * );

  virtual bool Redraw();
  virtual bool RedrawAlpha();

protected:
  PlayGame    *m_playGame;	///< Attached PlayGame object. 

  ImageData    *m_image;	///< Handler for "Pause" image. 
};

#endif	// _PlayGameView_
