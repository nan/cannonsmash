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

#ifndef _Title_
#define _Title_

// major menu
#define MENU_MAIN	0
#define MENU_CONFIG	1

// minor menu
#define MENU_ALL		0	// 画面内の全メニュー数
#define MENU_CONFIG_LEVEL	1	// レベル(Configメニュー内)
#define MENU_CONFIG_MODE	2	// モード(Configメニュー内)
#define MENU_CONFIG_SOUND	3	// サウンド(Configメニュー内)

class TitleView;

//class Title : public Control {
class Title : public Control {
public:
  Title();
  virtual ~Title();

  virtual bool Init();

  static Title* Create();

  virtual bool Move( unsigned long *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  long GetSelected();
  long GetSelectMode();
  long GetCount();
  long GetMenuNum( long major, long minor=0 );

  virtual bool LookAt( double &srcX, double &srcY, double &srcZ,
		       double &destX, double &destY, double &destZ );

  virtual bool IsPlaying() { return false; };
protected:
  TitleView *m_View;
  long m_selected;	// 選択された : m_selected > 0
  long m_selectMode;	// 基本選択   : 0
			// config選択  : 1
  long m_count;
};

#endif	// _Title_
