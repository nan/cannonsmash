/***********************************************************************
 * n:/Werk/src/csmash-0.3.3/z.cpp
 * $Id$
 *
 * This source is distributed under "AS-IS" policy.
 * No copyright is claimed.
 ***********************************************************************/
#ifdef HAVE_LIBZ

#ifdef _WIN32
#include <io.h>
#endif

#include "ttinc.h"

gzFile gzopenx(const char *filename, const char *mode) {
  int fd, m = 0;

#ifdef _WIN32
  m |= O_BINARY;
  if (NULL != strchr(mode, 's')) {
    m |= O_SEQUENTIAL;
  }
#endif
  if (NULL != strchr(mode, 'a')) {
    m |= O_APPEND;
  }

  bool r = false, w = false, plus = false; 
  if (NULL != strchr(mode, 'r')) {
    r = true;
  }
  if (NULL != strchr(mode, 'w')) {
    w = true;
  }
  if (NULL != strchr(mode, '+')) {
    plus = true;
  }

  if (plus) {
    if (!w) {
      m |= O_RDWR;
    } else {
      m |= O_RDWR | O_CREAT | O_TRUNC;
    }
  } else if (r) {
    m |= O_RDONLY;
  } else if (w) {
    m |= O_WRONLY | O_CREAT | O_TRUNC;
  }

  char buf[512];
  strncpy(buf, filename, 512);
  fd = open(buf, m, S_IREAD|S_IWRITE);
  if (0 > fd) {
    strncat(buf, ".gz", 512);
    fd = open(buf, m, S_IREAD|S_IWRITE);
  }
  gzFile zfp = gzdopen(fd, mode);
  return zfp;
}
#endif
/***********************************************************************
 * END OF z.cpp
 ***********************************************************************/
