/* $Id$ */

// Copyright (C) 2000, 2001  神南 吉宏(Kanna Yoshihiro)
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

  static long GetWinWidth() { return m_winWidth; };
  static long GetWinHeight() { return m_winHeight; };

  virtual SDL_Surface *GetSurface() { return m_baseSurface; };
protected:
  BaseView();

  vector3d       m_centerX;
  View           *m_View;
  FieldView      *m_fieldView;

  static long    m_winWidth;
  static long    m_winHeight;

  //GLuint       m_offset;
  //GLuint       m_floor;
  GLuint       m_title;
  //GLuint       m_wall[4];

  SDL_Surface	*m_baseSurface;

  virtual void SetLookAt();

private:
  static BaseView* m_theView;
};

#endif // _BaseView
