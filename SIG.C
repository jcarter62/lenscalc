#include "lens.h"
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
#ifndef dos
#  include <signal.h>
#endif

/* sig.c 5.12 6/16/89 */

/*
* s i g _ e x i t  ( sign ) 
*
* Parameters :
*   sign : signal number sent from o.s./compiler error routine.
*
* Purpose : Catch signal if possible and reset terminal to
*  The best possible state before bale-out.
*
* Globals : 
*  DEBUG : debug indicator,
*  dbg : FILE * if DEBUG = TRUE.
*
* Returns : n.a. 
* Date : Tue Mar 28, 1989; 02:41 pm
*/
sig_exit(sign)
int sign;
{
	if ( DEBUG ) fclose(dbg);
	endwin();
	fprintf(stderr,"\nbibi...\n\n");
#ifdef dos
}
#else
	nodelay(stdscr,FALSE);
	if ( sign != SIGFPE ) 
		exit();
}
#endif
