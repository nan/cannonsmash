#include "ttinc.h"

extern bool isSimple;
extern bool isTexture;
extern bool is2D;
extern bool fullScreen;
extern char serverName[256];

extern char nickname[32];
extern char message[64];

FILE * OpenRCFile();

bool
ReadRCFile() {
  FILE *fp = OpenRCFile();
  char buf[1024];

  while ( fgets( buf, 1024, fp ) ) {
    if ( strncmp( buf, "fullscreen=", 11 ) == 0 ) {
      if ( buf[11] == '1' )
	fullScreen = true;
      else
	fullScreen = false;
    } else if ( strncmp( buf, "graphics=", 9 ) == 0 ) {
      if ( strncmp( &buf[9], "simple", 6 ) == 0 ) {
	isSimple = true;
	isTexture = false;
      } else if ( strncmp( &buf[9], "2D", 2 ) == 0 ) {
	is2D = true;
      }
    } else if ( strncmp( buf, "server=", 7 ) == 0 ) {
      strncpy( serverName , &buf[7], 256 );
    } else if ( strncmp( buf, "nickname=", 9 ) == 0 ) {
      strncpy( nickname , &buf[9], 32 );
    } else if ( strncmp( buf, "message=", 8 ) == 0 ) {
      strncpy( message , &buf[8], 64 );
    }
  }

  fclose( fp );

  return true;
}

bool
WriteRCFile() {
  FILE *fp = OpenRCFile();

  fprintf( fp, "fullscreen=%d\n", fullScreen ? 1 : 0 );

  fprintf( fp, "graphics=" );
  if ( isSimple )
    fprintf( fp, "simple\n" );
  else if ( is2D )
    fprintf( fp, "2D\n" );
  else
    fprintf( fp, "full\n" );

  fprintf( fp, "server=%s\n", serverName );

  fprintf( fp, "nickname=%s\n", nickname );

  fprintf( fp, "message=%s\n", message );

  fclose( fp );

  return true;
}

FILE *
OpenRCFile() {
  char *csmashrc;

  csmashrc = getenv( "CSMASH_RC" );

  if ( csmashrc == NULL ) {
    if ( getenv( "HOME" ) == NULL ) {
#ifdef WIN32
      csmashrc = new char[256+12];
      GetWindowsDirectory( csmashrc, 256 );
      strcat( csmashrc, "\\csmash.ini" );
#else
      fprintf( stderr, "No home directory.\n" );
      exit(1);
#endif
    } else {
#ifdef WIN32
      csmashrc = new char[strlen(getenv("HOME"))+12];
      sprintf( csmashrc, "%s\\csmash.ini", getenv("HOME") );
#else
      csmashrc = new char[strlen(getenv("HOME"))+12];
      sprintf( csmashrc, "%s/.csmashrc", getenv("HOME") );
#endif
    }
  }

  FILE *fp = fopen( csmashrc, "r+" );
  if ( fp == NULL ) {	// file doesn't exist
    fp = fopen( csmashrc, "w" );
  }

  if ( fp == NULL ) {
    fprintf( stderr, "Cannot open rc file %s.\n", csmashrc );
    exit(1);
  }

  return fp;
}
