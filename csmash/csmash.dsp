# Microsoft Developer Studio Project File - Name="csmash" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** 編集しないでください **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=csmash - Win32 Debug
!MESSAGE これは有効なﾒｲｸﾌｧｲﾙではありません。 このﾌﾟﾛｼﾞｪｸﾄをﾋﾞﾙﾄﾞするためには NMAKE を使用してください。
!MESSAGE [ﾒｲｸﾌｧｲﾙのｴｸｽﾎﾟｰﾄ] ｺﾏﾝﾄﾞを使用して実行してください
!MESSAGE 
!MESSAGE NMAKE /f "csmash.mak".
!MESSAGE 
!MESSAGE NMAKE の実行時に構成を指定できます
!MESSAGE ｺﾏﾝﾄﾞ ﾗｲﾝ上でﾏｸﾛの設定を定義します。例:
!MESSAGE 
!MESSAGE NMAKE /f "csmash.mak" CFG="csmash - Win32 Debug"
!MESSAGE 
!MESSAGE 選択可能なﾋﾞﾙﾄﾞ ﾓｰﾄﾞ:
!MESSAGE 
!MESSAGE "csmash - Win32 Release" ("Win32 (x86) Application" 用)
!MESSAGE "csmash - Win32 Debug" ("Win32 (x86) Application" 用)
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "csmash - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /Ox /Ot /Ow /Og /Oi /Oy /Ob2 /I "./include" /I "./include/glib" /I "./include/gdk" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "HAVE_LIBZ" /D "HAVE_LIBSDL_MIXER" /D "HAVE_LIBVORBIS" /D "USE_GETTEXT_DLL" /Yu"ttinc.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDL_mixer.lib SDL_image.lib gtk-1.3.lib glib-1.3.lib gdk-1.3.lib iconv-1.3.lib zlib.lib libjpeg.lib gnu_gettext.lib opengl32.lib glu32.lib winmm.lib ws2_32.lib /nologo /subsystem:windows /machine:I386 /libpath:"./lib"

!ELSEIF  "$(CFG)" == "csmash - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MD /W3 /Gm /Gi /GX /ZI /Od /I "./include" /I "./include/glib" /I "./include/gdk" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "_MBCS" /D "HAVE_LIBZ" /D "HAVE_LIBSDL_MIXER" /D "HAVE_LIBVORBIS" /D "USE_GETTEXT_DLL" /D "WIN32CONSOLE" /Fr /Yu"ttinc.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib SDL.lib SDL_mixer.lib SDL_image.lib gtk-1.3.lib glib-1.3.lib gdk-1.3.lib iconv-1.3.lib zlib.lib libjpeg.lib gnu_gettext.lib opengl32.lib glu32.lib winmm.lib ws2_32.lib /nologo /debug /machine:I386 /pdbtype:sept /libpath:"./lib"
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "csmash - Win32 Release"
# Name "csmash - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Ball.cpp
# End Source File
# Begin Source File

SOURCE=.\BallView.cpp
# End Source File
# Begin Source File

SOURCE=.\BallView2D.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseView.cpp
# End Source File
# Begin Source File

SOURCE=.\BaseView2D.cpp
# End Source File
# Begin Source File

SOURCE=.\comPenAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\comPenDrive.cpp
# End Source File
# Begin Source File

SOURCE=.\comPlayer.cpp
# End Source File
# Begin Source File

SOURCE=.\comShakeCut.cpp
# End Source File
# Begin Source File

SOURCE=.\ComTrainingPenAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\ComTrainingPenDrive.cpp
# End Source File
# Begin Source File

SOURCE=.\Control.cpp
# End Source File
# Begin Source File

SOURCE=.\Event.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldView.cpp
# End Source File
# Begin Source File

SOURCE=.\FieldView2D.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\GetArgs.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\win32\getopt.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\HitMark.cpp
# End Source File
# Begin Source File

SOURCE=.\Howto.cpp
# End Source File
# Begin Source File

SOURCE=.\HowtoView.cpp
# End Source File
# Begin Source File

SOURCE=.\Launcher.cpp
# End Source File
# Begin Source File

SOURCE=.\LoadImage.cpp
# End Source File
# Begin Source File

SOURCE=.\loadparts.cpp
# End Source File
# Begin Source File

SOURCE=.\LobbyClient.cpp
# End Source File
# Begin Source File

SOURCE=.\Logging.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuItem.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuItemView.cpp
# End Source File
# Begin Source File

SOURCE=.\MenuItemView2D.cpp
# End Source File
# Begin Source File

SOURCE=.\MultiPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPenAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\NetPenDrive.cpp
# End Source File
# Begin Source File

SOURCE=.\NetShakeCut.cpp
# End Source File
# Begin Source File

SOURCE=.\Network.cpp
# End Source File
# Begin Source File

SOURCE=.\Opening.cpp
# End Source File
# Begin Source File

SOURCE=.\OpeningView.cpp
# End Source File
# Begin Source File

SOURCE=.\parts.cpp
# End Source File
# Begin Source File

SOURCE=.\PenAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\PenDrive.cpp
# End Source File
# Begin Source File

SOURCE=.\Player.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerSelectView.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerSelectView2D.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerView.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayerView2D.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayGame.cpp
# End Source File
# Begin Source File

SOURCE=.\PlayGameView.cpp
# End Source File
# Begin Source File

SOURCE=.\PracticePlay.cpp
# End Source File
# Begin Source File

SOURCE=.\PracticeSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\PracticeSelectView.cpp
# End Source File
# Begin Source File

SOURCE=.\RCFile.cpp
# End Source File
# Begin Source File

SOURCE=.\ShakeCut.cpp
# End Source File
# Begin Source File

SOURCE=.\SoloPlay.cpp
# End Source File
# Begin Source File

SOURCE=.\Sound.cpp
# End Source File
# Begin Source File

SOURCE=.\Title.cpp
# End Source File
# Begin Source File

SOURCE=.\TitleView.cpp
# End Source File
# Begin Source File

SOURCE=.\TitleView2D.cpp
# End Source File
# Begin Source File

SOURCE=.\Training.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainingPenAttack.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainingPenDrive.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainingSelect.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainingSelectView.cpp
# End Source File
# Begin Source File

SOURCE=.\TrainingView.cpp
# End Source File
# Begin Source File

SOURCE=.\ttinc.cpp
# ADD CPP /Yc"ttinc.h"
# End Source File
# Begin Source File

SOURCE=.\View.cpp
# End Source File
# Begin Source File

SOURCE=.\win32\wsaerror.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\win32\wsainit.cpp
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=.\xerror.cpp
# End Source File
# Begin Source File

SOURCE=.\z.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\affine
# End Source File
# Begin Source File

SOURCE=.\Ball.h
# End Source File
# Begin Source File

SOURCE=.\BallView.h
# End Source File
# Begin Source File

SOURCE=.\BallView2D.h
# End Source File
# Begin Source File

SOURCE=.\BaseView.h
# End Source File
# Begin Source File

SOURCE=.\BaseView2D.h
# End Source File
# Begin Source File

SOURCE=.\comPenAttack.h
# End Source File
# Begin Source File

SOURCE=.\comPenDrive.h
# End Source File
# Begin Source File

SOURCE=.\comPlayer.h
# End Source File
# Begin Source File

SOURCE=.\comShakeCut.h
# End Source File
# Begin Source File

SOURCE=.\ComTrainingPenAttack.h
# End Source File
# Begin Source File

SOURCE=.\ComTrainingPenDrive.h
# End Source File
# Begin Source File

SOURCE=.\Control.h
# End Source File
# Begin Source File

SOURCE=.\Event.h
# End Source File
# Begin Source File

SOURCE=.\FieldView.h
# End Source File
# Begin Source File

SOURCE=.\FieldView2D.h
# End Source File
# Begin Source File

SOURCE=.\float
# End Source File
# Begin Source File

SOURCE=.\win32\GetArgs.h
# End Source File
# Begin Source File

SOURCE=.\win32\getopt.h
# End Source File
# Begin Source File

SOURCE=.\HitMark.h
# End Source File
# Begin Source File

SOURCE=.\Howto.h
# End Source File
# Begin Source File

SOURCE=.\HowtoView.h
# End Source File
# Begin Source File

SOURCE=.\Launcher.h
# End Source File
# Begin Source File

SOURCE=.\LoadImage.h
# End Source File
# Begin Source File

SOURCE=.\loadparts.h
# End Source File
# Begin Source File

SOURCE=.\LobbyClient.h
# End Source File
# Begin Source File

SOURCE=.\Logging.h
# End Source File
# Begin Source File

SOURCE=.\matrix
# End Source File
# Begin Source File

SOURCE=.\MenuItem.h
# End Source File
# Begin Source File

SOURCE=.\MenuItemView.h
# End Source File
# Begin Source File

SOURCE=.\MenuItemView2D.h
# End Source File
# Begin Source File

SOURCE=.\win32\minwin32.h
# End Source File
# Begin Source File

SOURCE=.\MultiPlay.h
# End Source File
# Begin Source File

SOURCE=.\NetPenAttack.h
# End Source File
# Begin Source File

SOURCE=.\NetPenDrive.h
# End Source File
# Begin Source File

SOURCE=.\NetShakeCut.h
# End Source File
# Begin Source File

SOURCE=.\Network.h
# End Source File
# Begin Source File

SOURCE=.\Opening.h
# End Source File
# Begin Source File

SOURCE=.\OpeningView.h
# End Source File
# Begin Source File

SOURCE=.\parts.h
# End Source File
# Begin Source File

SOURCE=.\PenAttack.h
# End Source File
# Begin Source File

SOURCE=.\PenDrive.h
# End Source File
# Begin Source File

SOURCE=.\Player.h
# End Source File
# Begin Source File

SOURCE=.\PlayerSelect.h
# End Source File
# Begin Source File

SOURCE=.\PlayerSelectView.h
# End Source File
# Begin Source File

SOURCE=.\PlayerSelectView2D.h
# End Source File
# Begin Source File

SOURCE=.\PlayerView.h
# End Source File
# Begin Source File

SOURCE=.\PlayerView2D.h
# End Source File
# Begin Source File

SOURCE=.\PlayGame.h
# End Source File
# Begin Source File

SOURCE=.\PlayGameView.h
# End Source File
# Begin Source File

SOURCE=.\PracticePlay.h
# End Source File
# Begin Source File

SOURCE=.\PracticeSelect.h
# End Source File
# Begin Source File

SOURCE=.\PracticeSelectView.h
# End Source File
# Begin Source File

SOURCE=.\RCFile.h
# End Source File
# Begin Source File

SOURCE=.\win32\resource.h
# End Source File
# Begin Source File

SOURCE=.\ShakeCut.h
# End Source File
# Begin Source File

SOURCE=.\SoloPlay.h
# End Source File
# Begin Source File

SOURCE=.\Sound.h
# End Source File
# Begin Source File

SOURCE=.\Title.h
# End Source File
# Begin Source File

SOURCE=.\TitleView.h
# End Source File
# Begin Source File

SOURCE=.\TitleView2D.h
# End Source File
# Begin Source File

SOURCE=.\Training.h
# End Source File
# Begin Source File

SOURCE=.\TrainingPenAttack.h
# End Source File
# Begin Source File

SOURCE=.\TrainingPenDrive.h
# End Source File
# Begin Source File

SOURCE=.\TrainingSelect.h
# End Source File
# Begin Source File

SOURCE=.\TrainingSelectView.h
# End Source File
# Begin Source File

SOURCE=.\TrainingView.h
# End Source File
# Begin Source File

SOURCE=.\ttinc.h
# End Source File
# Begin Source File

SOURCE=.\View.h
# End Source File
# Begin Source File

SOURCE=.\win32\wsaerror.h
# End Source File
# Begin Source File

SOURCE=.\z.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\win32\csmash.rc
# End Source File
# Begin Source File

SOURCE=.\win32\orange.ico
# End Source File
# Begin Source File

SOURCE=.\win32\scream.ico
# End Source File
# End Group
# End Target
# End Project
