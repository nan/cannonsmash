/**
 * @file
 * @brief Definition of Training class. 
 * @author KANNA Yoshihiro
 * @version $Id$
 */

// Copyright (C) 2000, 2001, 2004  神南 吉宏(Kanna Yoshihiro)
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

#ifndef _Training_
#define _Training_
#include "PlayGame.h"
#include "TrainingView.h"

/**
 * Training class is the controller class for training play mode. 
 */
class Training : public PlayGame {
public:
  Training();
  virtual ~Training();

  virtual bool Init();

  static void Create( long player, long com );

  virtual bool Move( SDL_keysym *KeyHistory, long *MouseXHistory,
		     long *MouseYHistory, unsigned long *MouseBHistory,
		     int Histptr );

  long GetTrainingCount() { return m_trainingCount; }	///< Getter method of m_trainingCount. 
  long GetTrainingMax() { return m_trainingMax; }	///< Getter method of m_trainingMax. 
  void AddTrainingCount();

  virtual bool LookAt( vector3d &srcX, vector3d &destX );

  virtual View *GetView() { return m_View; }		///< Getter method of m_View. 
protected:
  TrainingView *m_View;			///< Attached view class. 
  long          m_trainingCount;	///< Number of rally count
  long          m_trainingMax;		///< Max number of rally count
};

#endif	// _Training_
