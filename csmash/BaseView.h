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

#ifndef _BaseView_
#define _BaseView_

// 背景部分(台, ネット)の描画およびWindow担当

class BaseView {
public:
  BaseView();
  ~BaseView();

  bool Init();

  static void DisplayFunc();
  static void ReshapeFunc( int width, int height );

  bool RedrawAll();
  bool Redraw();
  bool RedrawAlpha();
  bool SetViewPosition();

  bool AddView( View *view );
  bool RemoveView( View *view );

  void EndGame();

  static long GetWinWidth() { return m_winWidth; };
  static long GetWinHeight() { return m_winHeight; };
protected:
  double         m_centerX;
  double         m_centerY;
  double         m_centerZ;
  View           *m_View;

  static long    m_winWidth;
  static long    m_winHeight;

  GLuint       m_offset;
  GLuint       m_floor;
  GLuint       m_title;
  GLuint       m_wall[4];

  void SetLookAt();
};

#endif // _BaseView
