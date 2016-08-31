/* @(#) help.c 1.6@(#) 6/16/89 15:28:07 */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
#include "lens.h"
#include "pfile.h"
#include <ctype.h>

/* List of procedures/functions help.c
* help_msg( message ) 
* status( str, attribute ) 
* debug_display( l, message ) 
*/

/* global data used in other modules. */
char help_buf[100];
char *WORKING = "- - - Working - - -";
char *NOT_WORKING = NULL;

/*
* h e l p _ m s g  ( message ) 
*
* Parameters :
*   message : string
*
* Purpose : Display help information on bottom line.
*
* Globals : n.a. 
*   help_buf : string
*
* Returns : n.a. 
* Date : Tue Mar  7, 1989; 04:22 pm
*/
help_msg(message)
char *message;
{
	short row,col;	/* used to save old row,col */

	getyx(stdscr,row,col);	/* get current cursor position */
	move(LINES-1,0);		/* move to bottom line */
	clrtoeol();
	attron(A_UNDERLINE);
	printw(message);
	attroff(A_UNDERLINE);
	move(row,col);		/* move back to prev. position */
}

/*
* s t a t u s  ( str, attribute ) 
*
* Parameters :
*   str : String to display. == NULL to undo display status 
*   attribute : attribute to use.
*
* Purpose : 
*   Display status message (str) in attribute.
*   Display only non-alphanumeric characters in attribute.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Wed Apr 26, 1989; 08:20 am
*/
status(str,attribute)
char *str;
int attribute;
{
	register int cur_attr;
	int savex,savey;
	int x,y;

	getyx(stdscr,savey,savex);

	if ( str != NULL ) {
		y = LINES-3;
		x = (COLS-1)/2 - (strlen(str)/2);
		move(y,x);
		cur_attr = 0;
		while (*str) {
		  if ( isalnum(*str) ) {
		    if ( cur_attr ) {
		      attroff(attribute);
		      cur_attr = 0;
		    }
		  } else {
		    if ( !cur_attr)  {
		      attron(attribute);
		      cur_attr = 1;
		    }
		  }
		  printw("%c",*str);
		  str++;
		}
		attroff(attribute);
	} else {
		move(LINES-3,0);
		clrtoeol();
	}

	move(savey,savex);
	refresh();	
}

/*
* d e b u g _ d i s p l a y  ( l, message ) 
*
* Parameters :
*   l : Lens pointer to display.
*   message : string to display on aprox top line.
*
* Purpose : Display lens data in a form of normal lens.
* Use this routine to display intermediate lens data.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Wed Apr  5, 1989; 10:08 am
*/
debug_display(l,message)
LENS *l;
char *message;
{
	int c;

	if ( DEBUG ) {
	clear();
	display_lens(l);
	move(0,0);
	printw("debug message (%s)",message);
	move(2,0);
	addstr("-- Press Return to continue --");
	refresh();
	do {
		c = getch();
		if ( c == '7' ) {
			print_dest = to_paper;
                        print_lens(l,NULL);
		}
	} while ( c != '\n' && c != '\r' );
	clear();
	refresh();
	}
}
