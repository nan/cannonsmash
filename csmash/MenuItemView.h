/**
 * @file
 * @brief Definition of MenuItemView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2001, 2002  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _MenuItemView_
#define _MenuItemView_
#include "View.h"

class MenuItem;
class ImageData;

/**
 * MenuItemView is a View class for MenuItem. 
 */
class MenuItemView : public View {
public:
  MenuItemView();
  virtual ~MenuItemView();

  virtual bool Init( MenuItem *menu, char *fileName );

  virtual bool Redraw();
  virtual bool RedrawAlpha();

protected:
  MenuItem    *m_menuItem;	///< MenuItem object of which this MenuItemView is attached. 
  ImageData   *m_image;		///< Texture image of this MenuItemView. 
};

#endif	// _MenuItemView_
