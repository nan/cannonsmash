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

#ifndef _Event_
#define _Event_

// Event Handle Classの定義

class Event {
public:

  Event();
  ~Event();

  void IdleFunc();
  void KeyboardFunc( unsigned char key, int x, int y );
  void KeyUpFunc( unsigned char key, int x, int y );
  void MotionFunc( long x, long y );
  void ButtonFunc( long button, long state, long x, long y );

#ifdef PTHREAD
  void MoveCursor();
#endif
protected:
  unsigned long m_KeyHistory[MAX_HISTORY];		// キー入力履歴
  long m_MouseXHistory[MAX_HISTORY];			// マウス履歴
  long m_MouseYHistory[MAX_HISTORY];			// マウス履歴
  unsigned long m_MouseBHistory[MAX_HISTORY];		// マウスボタン履歴
  unsigned int  m_Histptr;
};

#endif	// _Event_
