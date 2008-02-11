/**
 * @file
 * @brief Implementation of BaseView class. 
 * @author KANNA Yoshihiro
 * @version $Id$
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
#include "BaseView.h"
#include "BaseView2D.h"
#include "Player.h"
#include "Control.h"
#include "Ball.h"
#include "LoadImage.h"
#include "PlayGame.h"
#include "RCFile.h"
#include "glARB.h"
#include "LobbyClient.h"

extern RCFile *theRC;

extern long mode;

extern Ball theBall;

long BaseView::m_winWidth = WINXSIZE;
long BaseView::m_winHeight = WINYSIZE;

BaseView* BaseView::m_theView = NULL;

// Copyed from http://morihyphen.hp.infoseek.co.jp/files/fontlist.tar.gz
// by wo_atmark@s9.dion.ne.jp
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftsnames.h>
#include <freetype/ttnameid.h>

#ifdef WIN32
struct FontList {
	HANDLE hFind;
	BOOL first;
	char path[MAX_PATH];
};

BOOL
get_fontpath( char *path )
{
  BOOL res = GetWindowsDirectoryA( path, MAX_PATH ); /* strlen("\\fonts") = 6 */

  if ( res == FALSE )
    return FALSE;

  strcat( path, "\\fonts\\" );
  return TRUE;
}

FontList *
fontlist_open(void)
{
  FontList *fl = (FontList *)malloc( sizeof(*fl) );
  fl->first = TRUE;
  fl->hFind = INVALID_HANDLE_VALUE;

  return fl;
}

void
fontlist_close( FontList *fl )
{
  if ( fl->hFind != INVALID_HANDLE_VALUE )
    FindClose(fl->hFind);
  free( fl );
}

char *
fontlist_next_file( FontList *fl )
{
  WIN32_FIND_DATA fd;

  get_fontpath( fl->path );

  if ( fl->first ) {

    int len = strlen( fl->path );

    strcat( fl->path, "*.tt*" );
    fl->hFind = FindFirstFile( fl->path, &fd );
    if ( fl->hFind == INVALID_HANDLE_VALUE )
      return NULL;

    fl->path[len] = '\0';

    fl->first = FALSE;
  } else {
    if ( FindNextFile( fl->hFind, &fd )==0 )
      return NULL;
  }

  strcat( fl->path, fd.cFileName );

  return fl->path;
}
#else
#include <fontconfig/fontconfig.h>

struct FontList {
	FcFontSet *fs;
	int index;
};

FontList *
fontlist_open(void)
{
  FontList *fl = (FontList *)malloc( sizeof(*fl) );
  FcPattern *pat = FcPatternCreate();
  FcResult res;

  FcDefaultSubstitute( pat );

  fl->fs = FcFontSort( NULL, pat, FcTrue, NULL, &res );

  FcPatternDestroy( pat );

  fl->index = 0;
  return fl;
}

char *
fontlist_next_file( FontList *fl )
{
  FcChar8 *fname;
  FcResult res;

  if ( fl->index >= fl->fs->nfont ) 
    return NULL;

  res = FcPatternGetString( fl->fs->fonts[fl->index], FC_FILE, 0, &fname );
  fl->index++;
  return (char*)fname;
}

void
fontlist_close( FontList *fl )
{
	FcFontSetDestroy( fl->fs );
	free( fl );
}
#endif

int 
check_font( FT_Face face )
{
#ifdef WIN32
  HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
  LOGFONT logFont;
  GetObject(hFont, sizeof(LOGFONT), &logFont);
#endif
  unsigned int i, charmap_num, langID;
  FT_UInt snamec;

  if ( FT_IS_SFNT(face) == 0 )
    return -1;

  snamec = FT_Get_Sfnt_Name_Count(face);

  i=0;
  while ( table[i].langID != -1 ) {
#ifdef WIN32
    if ( PRIMARYLANGID(GetUserDefaultLangID()) == table[i].langID )
#else
    if ( strncmp( setlocale(LC_MESSAGES, NULL), table[i].code, 2 ) == 0 )
#endif
      break;
    i++;
  }
  langID = table[i].langID;

#ifdef WIN32
  for ( i=0; i<snamec; i++ ) {
    FT_SfntName name;
    FT_Get_Sfnt_Name( face, i, &name );

    if ( strlen(logFont.lfFaceName) == name.string_len &&
	 strncmp(logFont.lfFaceName, (char *)name.string, name.string_len) == 0 ) {
      break;
    }
  }
#else
  for ( i=0; i<snamec; i++ ) {
    FT_SfntName name;
    FT_Get_Sfnt_Name( face, i, &name );

    if ( (name.language_id&0xFF) == langID ) {
      break;
    }
  }
#endif

  if ( i == snamec )
    return -1;

  charmap_num = face->num_charmaps;

  for ( int j=0; j<charmap_num; j++ ) {
    FT_CharMap map = face->charmaps[j];

    if ( (map->encoding==FT_ENCODING_UNICODE) )
      return i;
  }

  return -1;
}

int
getDefaultFreetypeFont(char * &fontPath, char * &faceName) {
  FontList *fl = fontlist_open();
  FT_Library ftlib;
  int i, j;

  char *firstFontPath = NULL;

  if ( FT_Init_FreeType(&ftlib) != FT_Err_Ok )
    return NULL;

  while (fontPath == NULL) {
    FT_Face face;
    char *path = fontlist_next_file( fl );

    if ( path == NULL )
      break;
    if (firstFontPath == NULL)
      firstFontPath = strdup(path);

    i = 0;
    while (1) {
      if ( FT_New_Face(ftlib, path, i, &face) != FT_Err_Ok )
	break;

      if ((j = check_font(face)) >= 0) {
	FT_SfntName name;
	FT_Get_Sfnt_Name( face, j, &name );
	faceName = (char *)malloc(name.string_len+1);
	memcpy(faceName, name.string, name.string_len);
	faceName[name.string_len]=0;
	fontPath = strdup(path);
	FT_Done_Face(face);
	goto end;
      }

      FT_Done_Face(face);
      i++;
    }
  }

 end:
  FT_Done_FreeType( ftlib );
  fontlist_close( fl );

  if (fontPath == NULL) {
    fontPath = firstFontPath;
  }

  return 0;
}


// x --- x axis is the bottom line of the net. The plane x=0 represents
//       the vertical plain which includes center line. 
// y --- y axis is the center line. The plane y=0 includes the net. 
// z --- z axis is the vertical line which includes the center point of
//       the table. The plane z=0 is the floor. 

/**
 * Getter method of BaseView singleton object. 
 * 
 * @return returns singleton BaseView object. 
 */
BaseView *
BaseView::TheView() {
  if ( BaseView::m_theView )
    return BaseView::m_theView;

  if ( theRC->gmode == GMODE_2D )
    BaseView::m_theView = new BaseView2D();
  else
    BaseView::m_theView = new BaseView();

  return BaseView::m_theView;
}

/**
 * Default constructor. 
 */
BaseView::BaseView() {
  m_View = (View*)0;
  m_centerX = vector3d(0.0, TABLELENGTH/2, TABLEHEIGHT);
}

/**
 * Destructor. 
 * Do nothing. 
 */
BaseView::~BaseView() {
}

/**
 * Initializer method. 
 * This method should be called only once just after BaseView is created. 
 * This method initialize SDL and OpenGL settings, then create FieldView
 * object and initialize it. 
 * 
 * @return returns true if succeeds. 
 */
bool
BaseView::Init() {
#ifdef HAVE_LIBSDL_MIXER
  if ( theRC->sndMode != SOUND_NONE ) {
    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_NOPARACHUTE) < 0 ) {
      perror( _("SDL initialize failed\n") );
      exit(1);
    }
  } else {
    if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) {
      perror( _("SDL initialize failed\n") );
      exit(1);
    }
  }
#else
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE) < 0 ) {
    perror( _("SDL initialize failed\n") );
    exit(1);
  }
  theRC->sndMode = SOUND_NONE;
#endif

// Create and initialize Window

  if ( theRC->fullScreen )
    m_baseSurface = SDL_SetVideoMode( m_winWidth, m_winHeight, 0,
				      SDL_OPENGL|SDL_FULLSCREEN );
  else
    m_baseSurface = SDL_SetVideoMode( m_winWidth, m_winHeight, 0,
				      SDL_OPENGL );

  SDL_WM_SetCaption( _("CannonSmash"), NULL );

  // load GL_ARB_multitexture extensions
  initglARBmultitexture();

  //if (fullScreen)
  //SDL_WM_ToggleFullScreen( m_baseSurface );

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  //glOrtho(-2.0, 2.0, -2.0, 2.0, 2.0, -2.0);
  gluPerspective(60.0, 1.0, 0.1, 30.0);
  glMatrixMode(GL_MODELVIEW);

  //GLfloat light_position[] = { 0.0F, 0.0F, 3.0F, 1.0F };
  GLfloat light_position[] = { -TABLEWIDTH*2, TABLELENGTH/2, 3.0F, 1.0F };
  GLfloat light_intensity_non[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  GLfloat light_intensity_amb[] = { 0.2F, 0.2F, 0.2F, 1.0F };
  GLfloat light_intensity_dif[] = { 0.7F, 0.7F, 0.7F, 1.0F };
  GLfloat light_intensity_spc[] = { 1.0F, 1.0F, 1.0F, 1.0F };

  if ( theRC->gmode != GMODE_SIMPLE ) {
    glShadeModel (GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  }

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_intensity_amb);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_intensity_dif);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_intensity_spc);

  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.5);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.0);
  glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.005);
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  if ( theRC->gmode == GMODE_SIMPLE ) {
    glClear(GL_COLOR_BUFFER_BIT);
  } else {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  int i, j;
// Set textures. 
  ImageData image;

  glGenTextures( 1, &m_title );

  image.LoadPPM( "images/Title.ppm" );

  for ( i = 0 ; i < image.GetWidth() ; i++ ) {
    for ( j = 0 ; j < image.GetHeight() ; j++ ) {
      if ( image.GetPixel( i, j, 0 ) >= 5 ||
	   image.GetPixel( i, j, 1 ) >= 5 ||
	   image.GetPixel( i, j, 2 ) >= 5 )
	image.SetPixel( i, j, 3 , 255 );
      else
	image.SetPixel( i, j, 3 , 0 );
    }
  }

  glBindTexture( GL_TEXTURE_2D, m_title );
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, 4, image.GetWidth(), image.GetHeight(),
	       0, GL_RGBA, GL_UNSIGNED_BYTE, image.GetImage() );

  m_fieldView = (FieldView *)View::CreateView( VIEW_FIELD );
  m_fieldView->Init();

  CEGUI::OpenGLRenderer* renderer = new CEGUI::OpenGLRenderer(0,800,600);
  new CEGUI::System(renderer);

  CEGUI::WindowManager& wmgr = CEGUI::WindowManager::getSingleton();
  try {
    CEGUI::SchemeManager::getSingleton().loadScheme("TaharezLook.scheme");

    char *fontName = NULL;
    char *faceName = NULL;
    getDefaultFreetypeFont(fontName, faceName);
    if (fontName == NULL) {
      fprintf( stderr, _("No font.\n") );
      exit(1);
    }

    CEGUI::XMLAttributes attr;
    attr.add("Name", "default-24");
    attr.add("Filename", fontName);
    attr.add("Type", "FreeType");
    attr.add("Size", "24");
    attr.add("AutoScaled", "true");
    if (faceName != NULL)
      attr.add("FaceName", faceName);
    else
      attr.add("FaceName", "");
    attr.add("NativeHorzRes", "800");
    attr.add("NativeVertRes", "600");

    CEGUI::Font *f;
    f = CEGUI::FontManager::getSingleton().createFont("FreeType", attr);
    f->load();
    CEGUI::System::getSingleton().setDefaultFont (f);
    attr.add("Name", "default-48");
    attr.add("Size", "48");
    f = CEGUI::FontManager::getSingleton().createFont("FreeType", attr);
    f->load();

  } catch (CEGUI::InvalidRequestException e) {
    printf("%s\n", e.getMessage().c_str());
  } catch (CEGUI::FileIOException e) {
    printf("%s\n", e.getMessage().c_str());
  } catch (CEGUI::RendererException e) {
    printf("%s\n", e.getMessage().c_str());
  }

  return true;
}

/**
 * Redraw entire screen. 
 */
void
BaseView::DisplayFunc() {
  BaseView::TheView()->RedrawAll();
}

/**
 * Redraw entire screen. 
 * This method is called just after all models are updated
 * to refresh screen. (cf. Event::IdleFunc). 
 * This method refresh OpenGL settings, then call Redraw() method
 * of each view one after another. 
 * After that, this method call RedrawAll() of each view, and redraw title. 
 * 
 * @return returns true if succeeds. 
 */
bool
BaseView::RedrawAll() {

  View *view;
  GLfloat light_position[] = { 1.0F, -1.0F, 1.0F, 0.0F };

  SetViewPosition();

  if ( theRC->gmode == GMODE_SIMPLE ) {
    glClear(GL_COLOR_BUFFER_BIT);
  } else {
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
  }

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  glDisable(GL_BLEND);
  m_fieldView->Redraw();
  if ( theRC->gmode == GMODE_SIMPLE )
    m_fieldView->RedrawAlpha();

  view = m_View;
  while ( view ){
    if ( view->Redraw() == false ){
      View *_view = m_View;

      if ( _view == view ){
	m_View = view->m_next;
	delete view;
	view = m_View;
      }
      else{
	while ( _view->m_next != view )
	  _view = _view->m_next;

	_view->m_next = view->m_next;
	delete view;
	view = _view;
      }
    }

    view = view->m_next;
  }

  if ( theRC->gmode != GMODE_SIMPLE )
    glEnable(GL_BLEND);

//  glDisable(GL_CULL_FACE);
  if ( theRC->gmode != GMODE_SIMPLE )
    m_fieldView->RedrawAlpha();

  view = m_View;
  while ( view ){
    view->RedrawAlpha();
    view = view->m_next;
  }

  // Title
  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0F, (GLfloat)m_winWidth, 0.0F, (GLfloat)m_winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();


  if ( theRC->isTexture ) {
    glEnable(GL_TEXTURE_2D);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBindTexture(GL_TEXTURE_2D, m_title );
    glBegin(GL_QUADS);
    glTexCoord2f(0.0F, 1.0F);
    glVertex2i( m_winWidth-256, 0);
    glTexCoord2f(1.0F, 1.0F);
    glVertex2i( m_winWidth, 0 );
    glTexCoord2f(1.0F, 0.0F);
    glVertex2i( m_winWidth, 256 );
    glTexCoord2f(0.0F, 0.0F);
    glVertex2i( m_winWidth-256, 256 );
    glEnd();

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glDisable(GL_TEXTURE_2D);
  }

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  CEGUI::System::getSingleton().renderGUI();
  SDL_GL_SwapBuffers();

//  if ( glGetError() != GL_NO_ERROR )
//    printf( _("GL Error!\n") );

  return true;
}

/**
 * Initialize the location and direction of the camera. 
 * 
 * @return returns true if succeeds. 
 */
bool
BaseView::SetViewPosition() {
  glLoadIdentity();

  SetLookAt();

  return true;
}

/**
 * Move the location and direction of the camera as the player moves. 
 */
void
BaseView::SetLookAt() {
  vector3d srcX;
  vector3d destX;

  srcX = vector3d(0.0);
  destX = m_centerX;
  if (Control::TheControl()->GetThePlayer())
    destX[1] = m_centerX[1]*Control::TheControl()->GetThePlayer()->GetSide();

  Control::TheControl()->LookAt( srcX, destX );

  gluLookAt( srcX[0], srcX[1], srcX[2], destX[0], destX[1], destX[2],
	     0.0F, 0.0F, 0.5F );
  /* 視点, 視線設定. 視点x, y, z, 視点(視線ベクトルの通る点)x, y, z, 
     上向きベクトル(画面上の上に向かうベクトル)x, y, z */
}

/**
 * Add newly attached view class to the list of view classes. 
 * 
 * @param view a view class which should be added to the list of view classes. 
 * @return returns true if succeeds. 
 */
bool
BaseView::AddView( View *view ) {
  view->m_next = m_View;
  m_View = view;

  return true;
}

/**
 * Remove a view class from the list of view classes. 
 * 
 * @param view a view class which should be removeed from the list of view classes. 
 * @retval returns true if succeeds. 
 * @retval returns false if the view class is not found in the list of view classes. 
 */
bool
BaseView::RemoveView( View *view ) {
  View* _view = m_View;

  if ( _view == view ){
    m_View = _view->m_next;
    return true;
  }

  while ( _view ){
    if ( _view->m_next == view ){
      _view->m_next = view->m_next;
      return true;
    }
    _view = _view->m_next;
  }

  return false;
}
      
/**
 * Show game over message. 
 * This method shows game over message and sleep 3 seconds, then returns. 
 */
void
BaseView::EndGame() {
  static char file[][30] = {"images/win", "images/lose"};
  char filename[256];
  ImageData image;

  static GLfloat end_spc[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat end_dif[] = { 0.0F, 0.0F, 0.0F, 1.0F };
  static GLfloat end_amb[] = { 5.0F, 5.0F, 5.0F, 1.0F };
  static GLfloat end_shininess[] = { 5.0 };
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, end_spc);
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, end_dif);
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, end_amb);
  glMaterialfv(GL_FRONT, GL_SHININESS, end_shininess);

  glPushMatrix();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D( 0.0F, (GLfloat)m_winWidth, 0.0F, (GLfloat)m_winHeight );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  printf( _("EndGame %d : %d\n"),
	  ((PlayGame *)Control::TheControl())->GetScore(Control::TheControl()->GetThePlayer()), 
	  ((PlayGame *)Control::TheControl())->GetScore(Control::TheControl()->GetComPlayer()) );

  if ( Control::TheControl()->IsPlaying() && 
       ((PlayGame *)Control::TheControl())->GetScore(Control::TheControl()->GetThePlayer()) >
       ((PlayGame *)Control::TheControl())->GetScore(Control::TheControl()->GetComPlayer()) ) {
    sprintf( filename, _("%s.pbm"), file[0] );
  }  else {
    sprintf( filename, _("%s.pbm"), file[1] );
  }
  image.LoadFile( filename );

  glRasterPos2i( 220, 330 );
  glBitmap( 400, 70, 0.0F, 0.0F, 0.0F, 0, image.GetImage() );

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();

  SDL_GL_SwapBuffers();

#ifdef WIN32
  Sleep(3000);
#else
  sleep(3);
#endif
}

/**
 * Cleanup method for quitting the game. 
 * This method frees SDL objects and cleans them up. 
 */
void
BaseView::QuitGame() {
  //if (fullScreen)
  //SDL_WM_ToggleFullScreen( m_baseSurface );
  if (m_baseSurface) {
    SDL_FreeSurface( m_baseSurface );
    m_baseSurface = NULL;
  }

  if (m_fieldView) {
    delete m_fieldView;
    m_fieldView = NULL;
  }
}

void
BaseView::SetWindowMode(bool fullscreen) {
  if (m_baseSurface) {
    SDL_FreeSurface( m_baseSurface );
    m_baseSurface = NULL;
  }

  if (fullscreen)
    m_baseSurface = SDL_SetVideoMode( m_winWidth, m_winHeight, 0,
				      SDL_OPENGL|SDL_FULLSCREEN );
  else
    m_baseSurface = SDL_SetVideoMode( m_winWidth, m_winHeight, 0,
				      SDL_OPENGL );
}
