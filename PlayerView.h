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

#ifndef _PlayerView_
#define _PlayerView_

class MotionData {
public:
  MotionData();
  virtual ~MotionData();

  bool LoadData( char *fname, int points );

  GLfloat   m_points[50][1024][3];
  GLshort   m_poly[50][4096][4];
  GLfloat   m_normal[50][4096][3];	// 法線ベクトル
};

class PlayerView : public View {
public:
  PlayerView();
  virtual ~PlayerView();

  bool Init( Player *player );

  virtual bool Redraw();
  virtual bool RedrawAlpha();

protected:
  GLuint       m_textures[1];

private:
  Player  *m_player;	// Model

  MotionData *m_Fnormal;
  MotionData *m_Bnormal;
  MotionData *m_Fdrive;
  MotionData *m_Bdrive;
  MotionData *m_Fcut;
  MotionData *m_Bcut;
  MotionData *m_Fpeck;
  MotionData *m_Bpeck;
  MotionData *m_Fsmash;
  MotionData *m_Bsmash;

  bool SubRedraw();
};

#endif	// _PlayerView
