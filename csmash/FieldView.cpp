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

#include "ttinc.h"

#ifdef HAVE_LIBZ
#include "z.h"
#endif

extern bool isLighting;
extern bool isTexture;

// x --- x座標軸はネットの底辺とその延長. x=0は, センターラインを含み
//       台に垂直な平面. 
// y --- y座標軸はセンターラインとその延長. y=0はネットを含む平面. 
// z --- z座標軸は台の中心を通る鉛直線. z=0は床面. 
FieldView::FieldView() {
}

FieldView::~FieldView() {
}

bool
FieldView::Init() {
  int i, j;
// テクスチャの設定. 
  ImageData image;

  image.LoadFile( "images/Floor.jpg" );

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glGenTextures( 1, &m_floor );
  glBindTexture( GL_TEXTURE_2D, m_floor );
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

  glTexImage2D(GL_TEXTURE_2D, 0, 3, image.GetWidth(), image.GetHeight(),
	       0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );

  static char pname[][30] = {"images/Left.jpg", "images/Front.jpg", 
			     "images/Right.jpg", "images/Back.jpg" };

  glGenTextures( 4, m_wall );
  for ( i = 0 ; i < 4 ; i++ ) {
    image.LoadFile( &(pname[i][0]) );

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture( GL_TEXTURE_2D, m_wall[i] );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    glTexImage2D(GL_TEXTURE_2D, 0, 3, image.GetWidth(), image.GetHeight(),
		 0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );
  }

  m_offset = glGenLists(1);

  glNewList( m_offset, GL_COMPILE );

  glBindTexture( GL_TEXTURE_2D, m_floor );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

  glBegin(GL_QUADS);			// 床の描画
    glNormal3f( 0.0, 0.0, 1.0 );
    glTexCoord2f(0.0, 0.0); glVertex3f( -AREAXSIZE*2,  AREAYSIZE*2, 0.0 );
    glTexCoord2f(4.0, 0.0); glVertex3f( -AREAXSIZE*2, -AREAYSIZE*2, 0.0 );
    glTexCoord2f(4.0, 4.0); glVertex3f(  AREAXSIZE*2, -AREAYSIZE*2, 0.0 );
    glTexCoord2f(0.0, 4.0); glVertex3f(  AREAXSIZE*2,  AREAYSIZE*2, 0.0 );
  glEnd();

  glEndList();

  return true;
}

bool
FieldView::Redraw() {
  if ( isTexture )
    glEnable(GL_TEXTURE_2D);

  glCallList( m_offset );

  glColor4f(0.8, 0.8, 0.8, 0.0);

  glBindTexture( GL_TEXTURE_2D, m_wall[0] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);			// 壁の描画
    glNormal3f( 1.0, 0.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f( -AREAXSIZE,  AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f( -AREAXSIZE,  AREAYSIZE, AREAZSIZE);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, m_wall[1] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);
    glNormal3f( 0.0, 1.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f( -AREAXSIZE, -AREAYSIZE, AREAZSIZE);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, m_wall[2] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);
    glNormal3f( -1.0, 0.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f(  AREAXSIZE,  AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f(  AREAXSIZE,  AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f(  AREAXSIZE, -AREAYSIZE, AREAZSIZE);
  glEnd();

  glBindTexture( GL_TEXTURE_2D, m_wall[3] );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glBegin(GL_QUADS);
    glNormal3f( 0.0, -1.0, 0.0 );
    glTexCoord2f(0.0, 1.0); glVertex3f(  AREAXSIZE, AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 1.0); glVertex3f( -AREAXSIZE, AREAYSIZE, 0.0 );
    glTexCoord2f(1.0, 0.0); glVertex3f( -AREAXSIZE, AREAYSIZE, AREAZSIZE);
    glTexCoord2f(0.0, 0.0); glVertex3f(  AREAXSIZE, AREAYSIZE, AREAZSIZE);
  glEnd();

  if ( isTexture )
    glDisable(GL_TEXTURE_2D);

  glColor4f(0.1, 0.1, 0.1, 0.0);
  glBegin(GL_QUADS);			// 天井の描画
    glNormal3f( 0.0, 0.0, -1.0 );
    glVertex3f(  AREAXSIZE*2,  AREAYSIZE*2, AREAZSIZE);
    glVertex3f( -AREAXSIZE*2,  AREAYSIZE*2, AREAZSIZE);
    glVertex3f( -AREAXSIZE*2, -AREAYSIZE*2, AREAZSIZE);
    glVertex3f(  AREAXSIZE*2, -AREAYSIZE*2, AREAZSIZE);
  glEnd();

  if ( isLighting ) {
    glColor4f(0.0, 0.2, 0.0, 0.0);
    static GLfloat mat_board[] = { 0.0, 0.2, 0.0, 0.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_board);

    glBegin(GL_QUADS);			// 仕切り板の描画
      glNormal3f( 1.0, 0.0, 0.0 );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );

      glNormal3f( -1.0, 0.0, 0.0 );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );

      glNormal3f( 0.0, -1.0, 0.0 );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, 0.0 );
      glVertex3f(  AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f( -AREAXSIZE/2,  AREAYSIZE/2, TABLEHEIGHT );

      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, 0.0 );
      glVertex3f( -AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );
      glVertex3f(  AREAXSIZE/2, -AREAYSIZE/2, TABLEHEIGHT );
    glEnd();
  }

  glColor4f(0.4, 0.4, 0.4, 0.0);
  static GLfloat mat_leg[] = { 0.0, 0.4, 0.0, 0.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_leg);

  glBegin(GL_QUADS);			// 足の描画
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, 0.0);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.13, -TABLELENGTH/2+0.1, 0.0);
    glVertex3f(-TABLEWIDTH/2+0.13, -TABLELENGTH/2+0.1, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13,-TABLELENGTH/2+0.1,  0.0);


    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, 0.0);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f(-TABLEWIDTH/2+0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f(-TABLEWIDTH/2+0.13, TABLELENGTH/2-0.1,  0.0);


    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, 0.0);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1, -TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13,-TABLELENGTH/2+0.1,  0.0);


    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, 0.0);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.13, 0.0);

    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  0.0);
    glVertex3f( TABLEWIDTH/2-0.1,  TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  TABLEHEIGHT-TABLETHICK);
    glVertex3f( TABLEWIDTH/2-0.13, TABLELENGTH/2-0.1,  0.0);
  glEnd();

  return true;
}

// 透明なものの描画
bool
FieldView::RedrawAlpha() {
  glColor4f(0.0, 0.0, 0.3, 0.7);
  static GLfloat mat_table[] = { 0.0, 0.0, 0.3, 1.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_table);

  glDepthMask(0);
  glBegin(GL_QUADS);			// Tableの描画
  if ( isLighting ) {
    glNormal3f( 0.0, 0.0, -1.0 );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );

    glNormal3f( 1.0, 0.0, 0.0 );
    glVertex3f( TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f( TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );

    glNormal3f( -1.0, 0.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );

    glNormal3f( 0.0, 1.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, TABLELENGTH/2, TABLEHEIGHT );

    glNormal3f( 0.0, -1.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT-TABLETHICK );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
  }

    glNormal3f( 0.0, 0.0, 1.0 );
    glVertex3f( -TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f(  TABLEWIDTH/2, -TABLELENGTH/2, TABLEHEIGHT );
    glVertex3f(  TABLEWIDTH/2,  TABLELENGTH/2, TABLEHEIGHT );
  glEnd();
  glDepthMask(1);

  static GLfloat mat_default[] = { 0.0, 0.0, 0.0, 1.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);

  if ( isLighting ) {
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);		// 白線の描画
      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( -TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );

      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f( -TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );

      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  TABLEWIDTH/2,      -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02, -TABLELENGTH/2+0.02, TABLEHEIGHT );

      glNormal3f( 0.0, 0.0, 1.0 );
      glVertex3f(  TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2,       TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( -TABLEWIDTH/2+0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );
      glVertex3f(  TABLEWIDTH/2-0.02,  TABLELENGTH/2-0.02, TABLEHEIGHT );
    glEnd();

    glBegin(GL_LINES);
      glVertex3f( 0.0, -TABLELENGTH/2, TABLEHEIGHT );
      glVertex3f( 0.0,  TABLELENGTH/2, TABLEHEIGHT );
    glEnd();
  }

  glColor4f(0.0, 0.2, 0.0, 0.9);

  static GLfloat mat_net[] = { 0.0, 0.2, 0.0, 1.0 };
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_net);
  glBegin(GL_QUADS);			// Netの描画
    glNormal3f( 0.0, 1.0, 0.0 );
    glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT );
    glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
    glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
    glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT );
  glEnd();
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_default);

  if ( isLighting ) {
    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);
      glNormal3f( 0.0, 1.0, 0.0 );
      glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT );
      glVertex3f( -TABLEWIDTH/2-NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
      glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT-0.01 );
      glVertex3f(  TABLEWIDTH/2+NETHEIGHT, 0.0, TABLEHEIGHT+NETHEIGHT );
    glEnd();
    glColor4f( 0.2, 1.0, 0.2, 1.0 );	// Why? But my RIVA128 need it.
  }

//  glBlendFunc(GL_ONE, GL_SRC_ALPHA);

  return true;
}
