/**
 * @file
 * @brief Definition of MenuItem class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

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

#ifndef _MenuItem_
#define _MenuItem_

class Title;
class MenuItemView;

/**
 * MenuItem class represents each menu shown in title. 
 */
class MenuItem {
public:
  MenuItem();
  virtual ~MenuItem();

  virtual bool Init( long x, long y, long width, long height, char *fileName,
		     Title *parent );

  bool GetSelected() { return m_selected; }	///< Returns whether this menu is selected or not. 
  long GetX() { return m_x; }			///< Returns x-location of this menu. 
  long GetY() { return m_y; }			///< Returns y-location of this menu. 
  long GetWidth() { return m_width; }		///< Returns width of this menu. 
  long GetHeight() { return m_height; }		///< Returns height of this menu. 
  bool SetSelected( bool select );
  MenuItemView *GetView() { return m_View; }	///< Returns corresponding MenuItemView object. 

protected:
  MenuItemView *m_View;		///< Corresponding MenuItemView object. 

  Title *m_parent;		///< Title object of which this MenuItem is attached. 

  bool m_selected;		///< True if this MenuItem is selected. 

  long m_x;			///< x-location of this MenuItem. 
  long m_y;			///< y-location of this MenuItem. 
  long m_width;			///< width of this MenuItem. 
  long m_height;		///< height of this MenuItem. 
};

#endif	// _MenuItem_
