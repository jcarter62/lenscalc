/* @(#) get.c 1.8@(#) 7/27/89 16:15:34 */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/

#include <stdio.h>
#include <curses.h>
#include <ctype.h>

/* List of procedures/functions get.c
* local_getch(  ) [public]
* local_getstr( s ) [public]
* toprintable( s ) [static]
* ss_display( line_#{l} , input_string{s} ) [static]
* disp_clock( ) [static]
*/

/* global variables defined in main.c */
extern int	scripti;		/* input script flag */
extern FILE *fp_iscript;	/* file pointer for input script file */
extern char	*fn_iscript, *fn_oscript;
extern int	scripto;		/* output script flag */
extern FILE *fp_oscript;	/* file pointer for output script file */
extern int	DEBUG;
extern FILE *dbg;
extern int	CLOCK;         /* clock flag, on/off */

static int	__line = 0;		/* used in ss_display() call */

#ifndef dos
static int	single_step = 0;
#endif

/*
* l o c a l _ g e t c h  (  ) [public]
*
* NO-Parameters :
* Purpose :  This is the getch() replacement for all other source
*  files.  Basically, the routine allows script input/output, and as
*  as result it is just another layer of input/output.  I.E.  The 
*  application will call local_getch() and in turn calls getch() under
*  normal opperation.  However, script in/out is also available.
*
* Globals : n.a. 
*
* Returns : input character, either from keyboard, or scriptfile.
* Date : Fri Apr  7, 1989; 03:15 pm
*/
int	local_getch()
{
	static char	buf[80];	/* save in ds */
	register int	c;
	extern char	*toprintable(char *);

	if ( scripti ) {
		__line++;
		if ( fgets(buf, 80, fp_iscript) == NULL )
			c = EOF;
		else
			c = buf[0];
		if ( c == EOF ) {
#ifndef dos
			single_step = 0;
			nodelay(stdscr, FALSE);
#endif
			scripti = 0;/* no more input script file */
			return local_getch();
#ifndef dos
		} else {
			addch(c);
			if ( single_step ) {
				ss_display(__line, buf);
				if ( getch() != 's' )
					single_step = 0;
			} else {
				nodelay(stdscr, TRUE);
				if ( getch() == 's' ) {
					single_step = 1;
					nodelay(stdscr, FALSE);
				}
			}
#endif
		}
	} else {
		if ( CLOCK ) {
			nodelay(stdscr, TRUE);
			do {
				disp_clock();
				c = getch();
			} while ( c == -1 );
			nodelay(stdscr, FALSE);
		} else
			c = getch();
	}

	if ( scripto ) {
		putc(c, fp_oscript);
		putc('\n', fp_oscript);
	}
	if (DEBUG) {
		if ( !scripti ) {
			buf[0] = c;
			buf[1] = '\0';
		}
		fprintf(dbg, "lgetch() = %s\n", toprintable(buf));
	}
	return c;
}



/*
* l o c a l _ g e t s t r  ( s ) [public]
*
* Parameters :
*   s : buffer, where to put input string.
*
* Purpose :  As above in local_getch(), this too allows input/output 
*  script files.  This is the getstr() replacement.
*
* Globals : n.a. 
*
* Returns : pointer to input string if not at end of file.
* Date : Fri Apr  7, 1989; 03:18 pm
*/
char	*local_getstr(s)
char	*s;
{
	extern char	*toprintable(char *);

	if ( scripti ) {
		__line++;
		if ( fgets(s, 80, fp_iscript) == NULL ) {
			scripti = 0;/* no more input script file */
#ifndef dos
			nodelay(stdscr, FALSE);
#endif
			return local_getstr(s);
#ifndef dos
		} else {
			addstr(s);
			if ( single_step ) {
				ss_display(__line, s);
				if ( getch() != 's' )
					single_step = 0;
			} else {
				nodelay(stdscr, TRUE);
				if ( getch() == 's' ) {
					single_step = 1;
					nodelay(stdscr, FALSE);
				}
			}
#endif
		}
		s[strlen(s)-1] = '\0';	/* get rid of new-line */
	} else {
		if ( CLOCK ) 
			disp_clock();
		getstr(s);
	}

	if ( scripto ) {
		fputs(s, fp_oscript);
		putc('\n', fp_oscript);
	}
	if (DEBUG)
		fprintf(dbg, "lgetstr()= %s\n", toprintable(s));
	return s;
}



/*
* t o p r i n t a b l e  ( s ) [static]
*
* Parameters :
*   s : string to convert to printable characters.
*
* Purpose : convert string (s) to printable characters,
*  (s) is not modified, result is stored in static buffer.
*  return string has \### for unprintable characters.
*
* Globals : n.a. 
*
* Returns : pointer to static buffer.
* Date : Fri Apr  7, 1989; 03:22 pm
*/
static char	*toprintable(s)
char	*s;
{
	static char	buf[50]; /* save in ds */
	int	i = 0;

	buf[0] = '\0';
	while (*s) {
		if ( isprint(*s) ) {
			buf[i++] = *s++;
			buf[i] = '\0';
		} else {
			sprintf(&(buf[i]), "\\%03o", *s++);
			i += 4;
		}
	}
	return buf;
}



/*
* s s _ d i s p l a y  ( line_#{l} , input_string{s} ) [static]
*
* Parameters :
*   l : line number of script input file.
*   s : input string at line (l) of input script file.
*
* Purpose : Display message on lower half of screen to show
*  user the progress of input script file.
*
* Globals : fn_iscript, input script file name.
*
* Returns : n.a. 
* Date : Fri Apr  7, 1989; 03:25 pm
*/
static ss_display(l, s)
int	l;
char	*s;
{
	short	x, y;

	getyx(stdscr, y, x);	/* get current position */
	move(LINES - 3, 0);
	printw("Single Step mode input=%s,line#=%d,string=%s", fn_iscript,
	     l, s);
	move(y, x);		/* back to old position */
	refresh();
}



/*
* d i s p _ c l o c k  (  ) [static]
*
* NO-Parameters :
* Purpose : Display clock at upper right corner of screen.
*
* Globals : save_time , used to determine if clock should be updated.
*
* Returns : n.a. 
* Date : Tue Jul 25, 1989; 06:22 am
*/
#include <time.h>
static long	save_time = (long)
0;
static disp_clock()
{
	long	tloc;
	struct tm *ptr;
	short	old_col, old_row;
	int	hour;
	char	ampm;

	(void)time(&tloc);
#ifdef MSDOS
	if ( save_time + (long)1 <= tloc ) 
#else
	if ( save_time + (long)10 <= tloc ) 
#endif
	{
		save_time = tloc;
		ptr = localtime(&tloc);

		hour = ptr->tm_hour;
		if ( hour >= 12 ) {
			ampm = 'P';
			hour -= 12;
		} else
			ampm = 'A';

		getyx(stdscr, old_row, old_col);
		standout();
		move(0, COLS - 12);
		printw("%2d:%02d:%02d %cM", hour, ptr->tm_min, ptr->tm_sec,
		     ampm);
		standend();
		move(old_row, old_col);
		refresh();
	}
}
