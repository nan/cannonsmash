/***********************************************************************
	d:/users/wata/src/ese/src/std/GetArgs.cpp
	$Id$

	Copyright by ESESoft.
	You are granted the right to redistribute this file under
	the "BSD artistic license". See "ARTISTIC" for detail.
 ***********************************************************************/
#include "minwin32.h"
#include <stdlib.h>
#include <malloc.h>

#include "GetArgs.h"

static int get_escape_char()
{
    char s[1];
    if (GetEnvironmentVariable("SHLVL", s, 1) /* csh */
	|| GetEnvironmentVariable("_", s, 1)  /* sh  */ ) {
	return GETARGS_ESCAPE_UNIX;
    } else {
	return GETARGS_ESCAPE_CMD;
    }
}

static inline int isquote_char(int c)
{
    if ('"' == c /* || '\'' == c */)
	return 1;
    else
	return 0;
}

int GetArgs(int *pargc, char ***pargv, const char *cmdline_, int esc)
{
    if (NULL == cmdline_) cmdline_ = GetCommandLine();
    if (0 == esc) esc = get_escape_char();

    char *cmdline = (char *)calloc(strlen(cmdline_) + 2, sizeof(char));
    char **argv  = (char **)calloc(1024, sizeof(char*));
    char *cur = (char*)cmdline_, *p = cmdline, *last;
    int argc = 0, quote_char = 0, c;
    int first = 1;

    do {
	c = *cur++;
	if (!quote_char) {
	    if (esc == c) {
		if (first) {
		    if (GETARGS_ESCAPE_UNIX == esc) {
			*p++ = '/';
		    } else {
			*p++ = c;
		    }
		} else {
		    c = *cur++;
		    *p++ = c;
		}
	    }
	    else if (isquote_char(c)) {
		quote_char = c;
	    }
	    else if (' ' == c || '\t' == c) {
		first = 0;
		*p++ = '\0';
	    } else {
		*p++ = c;
	    }
	} else {
	    if (c == quote_char) {
		quote_char = 0;
	    } else {
		*p++ = c;
	    }
	}
    } while ('\0' != *cur);
    last = p;

// if (0 != quote_char), there is unterminated quote(s)

    cur = cmdline;
    do {
	for (;'\0' == *cur && last > cur; cur++);
	if (!(last > cur)) break;
	argv[argc++] = cur;

	for (;'\0' != *cur && last > cur; cur++);
    } while (last > cur);
    argv[argc] = NULL;
    *pargv = argv;
    *pargc = argc;

    if (quote_char) {
//	fprintf(stderr, "Unmached quote %c\n", quote_char);
	return quote_char;
    } else {
  	return 0;
    }
}
