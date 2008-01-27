/**
 * @file
 * @brief Implementation of TitleView class. 
 * @author KANNA Yoshihiro
 * $Id$
 */

// Copyright (C) 2000-2004, 2007  Kanna Yoshihiro
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
#include "TitleView.h"
#include "Title.h"
#include "LobbyClient.h"
#include "Sound.h"
#include "BaseView.h"
#include "RCFile.h"

extern RCFile *theRC;

extern long mode;

extern void StartGame();

extern void QuitGame();

/**
 * Get current time as string. 
 * This method returns the current local time in "HH:MM:SS" format. 
 * 
 * @param buf buffer of which current time is stored. The length of the buffer must be greater than 32. 
 */
void
getCurrentTimeStr( char *buf ) {
  struct tm *ltime;
  time_t t;

  time(&t);
  ltime = localtime( &t );

  strftime( buf, 32, "%H:%M:%S ", ltime );
}  


/**
 * Default constructor. 
 */
TitleView::TitleView() {
  m_nextMode = MODE_TITLE;
}

/**
 * Destructor. 
 * Do nothing. 
 */
TitleView::~TitleView() {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  winMgr.destroyAllWindows();
}

/**
 * Initializer method. 
 * Load config title image texture. 
 * 
 * @param title attached Title object. 
 * @return returns true if succeeds. 
 */
bool
TitleView::Init( Title *title ) {
  m_title = title;

  SDL_ShowCursor(SDL_ENABLE);
  SDL_WM_GrabInput( SDL_GRAB_OFF );

  return true;
}

/**
 * Redraw valid objects. 
 * 
 * @return returns true if succeeds. 
 */
bool
TitleView::Redraw() {
  return true;
}

/**
 * Redraw transparent objects. 
 * 
 * @return returns true if succeeds. 
 */
bool
TitleView::RedrawAlpha() {
  glColor4f( 1.0F, 1.0F, 1.0F, 1.0F );

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0, (GLfloat)BaseView::GetWinWidth(),
	      0.0, (GLfloat)BaseView::GetWinHeight() );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  GLboolean depthtestenabled = glIsEnabled(GL_DEPTH_TEST);
  glDisable(GL_DEPTH_TEST);
  glDepthMask(0);

  glDepthMask(1);
  if (depthtestenabled) glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  return true;
}

/**
 * Create the list of MenuItem objects. 
 * 
 * @param menuMajorNum major menu ID (main or config)
 */
void
TitleView::CreateMenu( long menuMajorNum ) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  winMgr.destroyAllWindows();

  try {
    switch ( menuMajorNum ) {
    case MENU_MAIN:
      CreateMainMenu();
      break;
    case MENU_CONFIG:
      CreateConfigMenu();
      break;
    case MENU_LOGIN:
      CreateLoginMenu();
      break;
    case MENU_LOBBY:
      CreateLobbyMenu();
      break;
    }
  } catch (CEGUI::InvalidRequestException e) {
    printf("%s\n", e.getMessage().c_str());
  } catch (CEGUI::FileIOException e) {
    printf("%s\n", e.getMessage().c_str());
  } catch (CEGUI::RendererException e) {
    printf("%s\n", e.getMessage().c_str());
  }
}


/** 
 * Main menu
 */
void
TitleView::CreateMainMenu() {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* myRoot = winMgr.loadWindowLayout(_("mainMenu.layout"));
  CEGUI::System::getSingleton().setGUISheet(myRoot);

  winMgr.getWindow("Start Game")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleMainStartGame, this));
  winMgr.getWindow("Login Lobby Server")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleMainLoginLobby, this));
  winMgr.getWindow("Practice")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleMainPractice, this));
  winMgr.getWindow("Training")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleMainTraining, this));
  winMgr.getWindow("How to Play")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleMainHowtoPlay, this));
  winMgr.getWindow("Config")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleMainConfig, this));
  winMgr.getWindow("Quit")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleMainQuit, this));
}

bool 
TitleView::handleMainStartGame(const CEGUI::EventArgs& e) {
  m_nextMode = MODE_SELECT;
  return true;
}

bool 
TitleView::handleMainLoginLobby(const CEGUI::EventArgs& e) {
  m_title->SetSelectMode(MENU_LOGIN);
  return true;
}

bool
TitleView::handleMainPractice(const CEGUI::EventArgs& e) {
  m_nextMode = MODE_PRACTICESELECT;
  return true;
}

bool
TitleView::handleMainTraining(const CEGUI::EventArgs& e) {
  m_nextMode = MODE_TRAININGSELECT;
  return true;
}

bool
TitleView::handleMainHowtoPlay(const CEGUI::EventArgs& e) {
  m_nextMode = MODE_HOWTO;
  return true;
}

bool
TitleView::handleMainConfig(const CEGUI::EventArgs& e) {
  m_title->SetSelectMode(MENU_CONFIG);
  return true;
}

bool
TitleView::handleMainQuit(const CEGUI::EventArgs& e) {
  QuitGame();
  return true;
}


/** 
 * Config menu
 */
void
TitleView::CreateConfigMenu() {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* myRoot = winMgr.loadWindowLayout(_("configMenu.layout"));

  CEGUI::System::getSingleton().setGUISheet(myRoot);

  // Setup items
  CEGUI::Combobox *cbox;
  CEGUI::ListboxTextItem *item[16];
  char volBuf[16];

  cbox = (CEGUI::Combobox *)winMgr.getWindow("Config/System/ScreenMenu");
  item[0] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("FullScreen"));
  item[1] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Window"));
  cbox->addItem(item[0]);
  cbox->addItem(item[1]);
  item[0]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[1]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");

  if (theRC->fullScreen)
    cbox->setText((CEGUI::utf8 *)_("FullScreen"));
  else
    cbox->setText((CEGUI::utf8 *)_("Window"));

  cbox = (CEGUI::Combobox *)winMgr.getWindow("Config/System/GraphicsMenu");

  item[0] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Normal"));
  item[1] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Simple"));
  item[2] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Toon"));
  cbox->addItem(item[0]);
  cbox->addItem(item[1]);
  cbox->addItem(item[2]);
  item[0]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[1]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[2]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");

  switch (theRC->gmode) {
  case GMODE_SIMPLE:
    cbox->setText((CEGUI::utf8 *)_("Simple"));
    break;
  case GMODE_FULL:
    cbox->setText((CEGUI::utf8 *)_("Normal"));
    break;
  case GMODE_TOON:
    cbox->setText((CEGUI::utf8 *)_("Toon"));
    break;
  }

  cbox = (CEGUI::Combobox *)winMgr.getWindow("Config/System/GraphicsTypeMenu");

  item[0] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Transparent"));
  item[1] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("WireFrame"));
  item[2] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("ArmOnly"));
  cbox->addItem(item[0]);
  cbox->addItem(item[1]);
  cbox->addItem(item[2]);
  item[0]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[1]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[2]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");

  switch (theRC->myModel) {
  case MODEL_TRANSPARENT:
    cbox->setText((CEGUI::utf8 *)_("Transparent"));
    break;
  case MODEL_WIREFRAME:
    cbox->setText((CEGUI::utf8 *)_("WireFrame"));
    break;
  case MODEL_ARMONLY:
    cbox->setText((CEGUI::utf8 *)_("ArmOnly"));
    break;
  }

  //TODO: Sound on/off 機能を削除 (ボリュームup/down に変更した)
  /*
    if (theRC->sndMode == SOUND_SDL)
    ((CEGUI::RadioButton *)winMgr.getWindow("Config/System/SoundOn"))->setSelected(true);
    else
    ((CEGUI::RadioButton *)winMgr.getWindow("Config/System/SoundOff"))->setSelected(true);
  */
  sprintf(volBuf, "%3d%%", (int)theRC->sndVolume);
  winMgr.getWindow("Config/System/Volume")->setText(volBuf);
  ((CEGUI::Scrollbar *)winMgr.getWindow("Config/System/SoundSlider"))->setScrollPosition(theRC->sndVolume);


  cbox = (CEGUI::Combobox *)winMgr.getWindow("Config/GameMode/LevelMenu");
  item[0] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Easy"));
  item[1] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Normal"));
  item[2] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Hard"));
  item[3] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("Tsuborish"));
  cbox->addItem(item[0]);
  cbox->addItem(item[1]);
  cbox->addItem(item[2]);
  cbox->addItem(item[3]);
  item[0]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[1]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[2]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[3]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");

  switch (theRC->gameLevel) {
  case LEVEL_EASY:
    cbox->setText((CEGUI::utf8 *)_("Easy"));
    break;
  case LEVEL_NORMAL:
    cbox->setText((CEGUI::utf8 *)_("Normal"));
    break;
  case LEVEL_HARD:
    cbox->setText((CEGUI::utf8 *)_("Hard"));
    break;
  case LEVEL_TSUBORISH:
    cbox->setText((CEGUI::utf8 *)_("Tsuborish"));
    break;
  }

  cbox = (CEGUI::Combobox *)winMgr.getWindow("Config/GameMode/ModeMenu");
  item[0] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("5point"));
  item[1] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("11point"));
  item[2] = new CEGUI::ListboxTextItem((CEGUI::utf8 *)_("21point"));
  cbox->addItem(item[0]);
  cbox->addItem(item[1]);
  cbox->addItem(item[2]);
  item[0]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[1]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");
  item[2]->setSelectionBrushImage("TaharezLook", "MultiListSelectionBrush");

  switch (theRC->gameMode) {
  case GAME_5PTS:
    cbox->setText((CEGUI::utf8 *)_("5point"));
    break;
  case GAME_11PTS:
    cbox->setText((CEGUI::utf8 *)_("11point"));
    break;
  case GAME_21PTS:
    cbox->setText((CEGUI::utf8 *)_("21point"));
    break;
  }

  // Set callbacks
  winMgr.getWindow("Config/System/ScreenMenu")->
    subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&TitleView::handleConfigScreenMenuSelected, this));

  winMgr.getWindow("Config/System/GraphicsMenu")->
	subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&TitleView::handleConfigGraphicsMenuSelected, this));

  winMgr.getWindow("Config/System/GraphicsTypeMenu")->
    subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&TitleView::handleConfigGraphicsTypeMenuSelected, this));


  winMgr.getWindow("Config/System/SoundSlider")->
    subscribeEvent (CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber(&TitleView::handleConfigSoundSliderChanged, this));

  winMgr.getWindow("Config/GameMode/LevelMenu")->
    subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&TitleView::handleConfigGameModeLevelMenuSelected, this));


  winMgr.getWindow("Config/GameMode/ModeMenu")->
    subscribeEvent(CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber(&TitleView::handleConfigGameModeModeMenuSelected, this));

  winMgr.getWindow("Config/GameMode/InvertMouse")->
    subscribeEvent (CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber(&TitleView::handleConfigInvertMouseChanged, this));


  winMgr.getWindow("Back")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleConfigBack, this));
}

bool TitleView::handleConfigScreenMenuSelected(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* selectedGraphics = winMgr.getWindow("Config/System/ScreenMenu");
  bool fullScreen = theRC->fullScreen;

  if (selectedGraphics->getText().compare(_("FullScreen")) == 0)
    fullScreen = true;
  else if (selectedGraphics->getText().compare(_("Window")) == 0)
    fullScreen = false;

  if (fullScreen != theRC->fullScreen) {
    theRC->fullScreen = fullScreen;
    BaseView::TheView()->SetWindowMode(fullScreen);
  }

  return true;
}


bool TitleView::handleConfigGraphicsMenuSelected(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* selectedGraphics = winMgr.getWindow("Config/System/GraphicsMenu");

  if (selectedGraphics->getText().compare(_("Simple")) == 0)
    theRC->gmode = GMODE_SIMPLE;
  else if (selectedGraphics->getText().compare(_("Normal")) == 0)
    theRC->gmode = GMODE_FULL;
  else if (selectedGraphics->getText().compare(_("Toon")) == 0)
    theRC->gmode = GMODE_TOON;

  return true;
}

bool
TitleView::handleConfigGraphicsTypeMenuSelected(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* selectedGraphics = winMgr.getWindow("Config/System/GraphicsTypeMenu");

  if (selectedGraphics->getText().compare(_("Transparent")) == 0)
    theRC->myModel = MODEL_TRANSPARENT;
  else if (selectedGraphics->getText().compare(_("WireFrame")) == 0)
    theRC->myModel = MODEL_WIREFRAME;
  else if (selectedGraphics->getText().compare(_("ArmOnly")) == 0)
    theRC->myModel = MODEL_ARMONLY;

  return true;
}


bool
TitleView::handleConfigGameModeLevelMenuSelected(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* selectedGraphics = winMgr.getWindow("Config/GameMode/LevelMenu");

  if (selectedGraphics->getText().compare(_("Easy")) == 0)
    theRC->gameLevel = LEVEL_EASY;
  else if (selectedGraphics->getText().compare(_("Normal")) == 0)
    theRC->gameLevel = LEVEL_NORMAL;
  else if (selectedGraphics->getText().compare(_("Hard")) == 0)
    theRC->gameLevel = LEVEL_HARD;
  else if (selectedGraphics->getText().compare(_("Tsuborish")) == 0)
    theRC->gameLevel = LEVEL_TSUBORISH;

  return true;
}

bool
TitleView::handleConfigGameModeModeMenuSelected(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* selectedGraphics = winMgr.getWindow("Config/GameMode/ModeMenu");

  if (selectedGraphics->getText().compare(_("5point")) == 0)
    theRC->gameMode = GAME_5PTS;
  else if (selectedGraphics->getText().compare(_("11point")) == 0)
    theRC->gameMode = GAME_11PTS;
  else if (selectedGraphics->getText().compare(_("21point")) == 0)
    theRC->gameMode = GAME_21PTS;

  return true;
}

bool TitleView::handleConfigBack(const CEGUI::EventArgs& e) {
  m_title->SetSelectMode(MENU_MAIN);

  return true;
}

bool
TitleView::handleConfigSoundSliderChanged(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  float f;
  char buf[16];

  f = ((CEGUI::Scrollbar *)winMgr.getWindow("Config/System/SoundSlider"))->getScrollPosition();
  sprintf(buf, "%3d%%", (int)f);
  winMgr.getWindow("Config/System/Volume")->setText(buf);

  Sound::TheSound()->SetVolume(f);

  return true;
}

bool
TitleView::handleConfigInvertMouseChanged(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

  bool invert = ((CEGUI::Checkbox *)winMgr.getWindow("Config/GameMode/InvertMouse"))->isSelected();
  theRC->switchButtons = invert;
}


/** 
 * Login menu
 */
void
TitleView::CreateLoginMenu() {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* myRoot = winMgr.loadWindowLayout(_("loginMenu.layout"));
  CEGUI::System::getSingleton().setGUISheet(myRoot);

  winMgr.getWindow("Login")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleLoginLogin, this));
  winMgr.getWindow("Cancel")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleLoginCancel, this));
}

bool TitleView::handleLoginLogin(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();

  LobbyClient *lb;
  lb = LobbyClient::Create();

  char *nickname = (char *) winMgr.getWindow("NicknameEdit")->getText().c_str();
  char *password = (char *) winMgr.getWindow("PasswordEdit")->getText().c_str();
  bool ping      = ((CEGUI::Checkbox *)winMgr.getWindow("AcceptPing"))->isSelected();

  if ( strncmp( theRC->message, password, 64 ) ) {
    //TODO: encrypt password. At that time, password buffer should not be overwritten. 
  }

  if ( lb->Init(nickname, password, ping) == false ) {
    //TODO:
    //LauncherView::ConnectionFailedDialog();
    return true;
  }

  m_title->SetSelectMode(MENU_LOBBY);

  return true;
}

bool TitleView::handleLoginCancel(const CEGUI::EventArgs& e) {
  m_title->SetSelectMode(MENU_MAIN);

  return true;
}


/** 
 * Lobby menu
 */
void
TitleView::CreateLobbyMenu() {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Window* myRoot = winMgr.loadWindowLayout(_("lobbyMenu.layout"));
  CEGUI::System::getSingleton().setGUISheet(myRoot);

  winMgr.getWindow("PlayerInfoList")->
    subscribeEvent(CEGUI::MultiColumnList::EventSelectionChanged, CEGUI::Event::Subscriber(&TitleView::handleLobbyCheckSelection, this));
  winMgr.getWindow("ChatEdit")->
    subscribeEvent(CEGUI::Editbox::EventTextAccepted, CEGUI::Event::Subscriber(&TitleView::handleLobbyChatInput, this));

  winMgr.getWindow("Connect")->disable();

  LobbyClient *lb = LobbyClient::TheLobbyClient();
  long nativeLangID = table[lb->GetLang()].langID;
  if (nativeLangID != 0x09) {
    winMgr.getWindow("NativeLanguage")->setText(table[lb->GetLang()].langname);
    winMgr.getWindow("NativeChatLog")->setID(nativeLangID);
  } else  {
    winMgr.getWindow("NativeLanguage")->destroy();
  }
  winMgr.getWindow("EnglishChatLog")->setID(0x09);

  winMgr.getWindow("Connect")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleLobbyConnect, this));
  winMgr.getWindow("Logout")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handleLobbyLogout, this));
  // TODO: LobbyClientView::ShowUpdateDialog()
  // TODO: PIDialog
}

bool
TitleView::handleLobbyCheckSelection(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::MultiColumnList *mcl = (CEGUI::MultiColumnList *)winMgr.getWindow("PlayerInfoList");
  LobbyClient *lb = LobbyClient::TheLobbyClient();
  unsigned int rowIdx;

  CEGUI::ListboxItem *li = mcl->getFirstSelectedItem();

  winMgr.getWindow("Connect")->disable();

  if (li != NULL) {
    try {
      rowIdx = mcl->getItemRowIndex(li);
    } catch (CEGUI::InvalidRequestException e) {
      return false;
    }
  } else {
    return false;
  }

  for ( int i = 0 ; i < lb->m_playerNum ; i++ ) {
    if (lb->m_player[i].m_ID == mcl->getRowID(rowIdx)) {
      if (li->isDisabled() == false) {
	lb->m_selected = lb->m_player[i].m_ID;

	winMgr.getWindow("Connect")->enable();
	return true;
      } else {
	lb->m_selected = -1;
	mcl->setItemSelectState(li, false);
	return false;
      }
    }
  }

  return false;
}

bool
TitleView::handleLobbyChatInput(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  CEGUI::Editbox *chatEditBox = (CEGUI::Editbox *)winMgr.getWindow("ChatEdit");
  CEGUI::TabControl *chatlog = (CEGUI::TabControl *)winMgr.getWindow("TabControl");
  LobbyClient *lb = LobbyClient::TheLobbyClient();
  CEGUI::String chatInput = chatEditBox->getText();

  if (chatInput.length() > 0) {
    CEGUI::Window *selectedTab = chatlog->getTabContentsAtIndex(chatlog->getSelectedTabIndex());
    CEGUI::Window *selectedChatlog = selectedTab->isChild("NativeChatLog") ? winMgr.getWindow("NativeChatLog") : winMgr.getWindow("EnglishChatLog");
    int selectedChannel = selectedChatlog->getID();
    lb->SendMS( (char *)chatInput.c_str(), selectedChannel );

    char buf[32];
    getCurrentTimeStr(buf);

    CEGUI::String chatText = selectedChatlog->getText();
    chatText += buf;
    chatText += ">";
    chatText += lb->m_nickname;
    chatText += "<";
    chatText += chatInput;
    chatText += "\n";
    selectedChatlog->setText(chatText);

    CEGUI::Scrollbar* vert = ((CEGUI::MultiLineEditbox *)selectedChatlog)->getVertScrollbar();
    vert->setScrollPosition(vert->getDocumentSize()-vert->getPageSize());

    chatEditBox->setText("");
  }

  return true;
}

bool
TitleView::handleLobbyConnect(const CEGUI::EventArgs& e) {
  LobbyClient::TheLobbyClient()->Connect();

  return true;
}

bool
TitleView::handleLobbyLogout(const CEGUI::EventArgs& e) {
  LobbyClient::TheLobbyClient()->SendQT();
  delete LobbyClient::TheLobbyClient();

  m_title->SetSelectMode(MENU_MAIN);

  return true;
}


/**
 * Show popup dialog showing that a opponent wants to play with the player. 
 * This method is called when a opponent sends a message to play with the 
 * player. 
 * 
 * @param uniqID ID of the opponent player. 
 */
void
TitleView::popupPIDialog( long uniqID ) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  char buf[256];

  m_popupUniqID = uniqID;

  CEGUI::FrameWindow *window = (CEGUI::FrameWindow *)winMgr.loadWindowLayout(_("piDialog.layout"));
  winMgr.getWindow("root")->addChildWindow(window);

  LobbyClient *lb = LobbyClient::TheLobbyClient();
  PlayerInfo *p = lb->GetPlayerInfo();

  int i;
  for ( i = 0 ; i < lb->GetPlayerNum() ; i++ ) {
    if ( p[i].m_ID == m_popupUniqID ) {
      sprintf( buf, _("\"%s\" (message: %s)want to play with you. OK?\n"),
	       p[i].m_nickname, p[i].m_message );
      break;
    }
  }

  winMgr.getWindow("PIDialog/Text")->setText(buf);

  winMgr.getWindow("PIDialog/OK")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handlePIDialogOK, this));
  winMgr.getWindow("PIDialog/NO")->
    subscribeEvent(CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TitleView::handlePIDialogNo, this));
}

/**
 * Callback method of "OK" button of the popup dialog. 
 * This method sends message of accepting to play with the opponent. 
 * If the opponent cannot be the server, the client starts the game in server
 * mode. Otherwise the game is started in client mode. 
 * 
 * @param widget not used. 
 * @param data pointer of popup dialog object. 
 */
bool
TitleView::handlePIDialogOK(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  winMgr.destroyWindow("PIDialog");

  m_nextMode = MODE_MULTIPLAYSELECT;

  LobbyClient *lb = LobbyClient::TheLobbyClient();
  lb->SendAP(m_popupUniqID);
  lb->SendSP();

  // If oppenent cannot be a server, I become server. 
  for ( int i = 0 ; i < lb->m_playerNum ; i++ ) {
    if ( lb->m_player[i].m_ID == m_popupUniqID ) {
      if ( lb->m_player[i].m_canBeServer == false ) {	// Opponent cannot be server
	theRC->serverName[0] = '\0';
      }
      break;
    }
  }

  //TODO:
  // After playing, QP message should be sent. 
  //lb->SendQP();

  return true;
}

/**
 * Callback method of "No" button of the popup dialog. 
 * This method sends message of denying to play with the opponent. 
 * 
 * @param widget not used. 
 * @param data pointer of popup dialog object. 
 */
bool
TitleView::handlePIDialogNo(const CEGUI::EventArgs& e) {
  CEGUI::WindowManager& winMgr = CEGUI::WindowManager::getSingleton();
  winMgr.destroyWindow("PIDialog");

  LobbyClient *lb = LobbyClient::TheLobbyClient();
  lb->SendDP( m_popupUniqID );

  return true;
}
