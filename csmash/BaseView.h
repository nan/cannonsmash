/**
 * @file
 * @brief Definition of BaseView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2001, 2007  Kanna Yoshihiro
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

#ifndef _BaseView_
#define _BaseView_

#include <vector>
#include "matrix"
typedef Vector<3, double> vector3d;
typedef Vector<2, double> vector2d;

#include "View.h"
#include "FieldView.h"


/**
 * View manager class. 
 * This class holds the list of all view classes, and call Redraw() and
 * RedrawAlpha() method of these view classes when the screen should be 
 * refreshed. 
 * Additionally, this class manages entire screen. In other words, this 
 * class holds screen size, initializes and cleans up the entire screen. 
 */
class BaseView {
public:
  virtual ~BaseView();

  static BaseView *TheView();

  virtual bool Init();

  static void DisplayFunc();

  virtual bool RedrawAll();
  virtual bool SetViewPosition();

  virtual bool AddView( View *view );
  virtual bool RemoveView( View *view );

  virtual void EndGame();
  virtual void QuitGame();

  virtual void SetWindowMode(bool fullscreen);

  /**
   * Returns the width of the screen. 
   */
  static long GetWinWidth() { return m_winWidth; };
  /**
   * Returns the height of the screen. 
   */
  static long GetWinHeight() { return m_winHeight; };

  /**
   * Returns the SDL_Surface object. 
   */
  virtual SDL_Surface *GetSurface() { return m_baseSurface; };
protected:
  BaseView();

  vector3d       m_centerX;	///< View destination. 
  View           *m_View;	///< List of views. 
  FieldView      *m_fieldView;	///< FieldView object. 

  static long    m_winWidth;	///< screen width
  static long    m_winHeight;	///< screen height

  //GLuint       m_offset;
  //GLuint       m_floor;
  GLuint         m_title;	///< Title texture handler. 
  //GLuint       m_wall[4];

  SDL_Surface	*m_baseSurface;	///< SDL_Surface object for the screen. 

  virtual void SetLookAt();

private:
  static BaseView* m_theView;	///< Singleton BaseView object. 
};

#endif // _BaseView
