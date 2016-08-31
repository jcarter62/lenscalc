/* menu.c 5.7 8/29/90 */
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
#include <ctype.h>

#define next(string) {move(cur_row++,center(string,80));clrtoeol();\
		      printw("%s",string);}

/* returns menu entry picked , where first entry = 1 */
int
menu(ary)
char **ary;
{
	int hi_line;
        int cur_row;
        register int c;
	register int i,n;

	noecho();	/* turn echo off */
	keypad(stdscr, TRUE);

	clear();
	hi_line = 0;
	n = 0;
	i = 0;
	while ( *ary[i] ) { i++; n++;}

	while (1) {
		display_menu_array(ary,n,hi_line);

                c = getch();
		if ( isupper(c) ) c = tolower(c);

		switch (c) {
		case KEY_DOWN :
		case '+':
			hi_line++;
			if ( hi_line >= n ) hi_line = 0;
			break;
		case KEY_UP :
		case '-':
			hi_line--;
			if ( hi_line < 0 ) hi_line = n-1;
			break;
		case '\n' :
		case '\r' :
			if ( hi_line == 0 ) {
				beep();
			} else 	
				c = *ary[hi_line];
		default  :
			for (i=1;i<n;i++) 
				if ( *ary[i] == c ) {
					display_menu_array(ary,n,i);
					return i;
				}
		}
	}	
	clear();
	refresh();
	echo();	/* turn echo on */
	keypad(stdscr, FALSE);
}

static int
center(s,columns)
char *s;	/* string to center */
int columns;	/* number of columns to center s in */
{
	register int i;
	int pad,sl;

	sl = strlen(s);
	if ( sl > columns ) return 0;
	pad = columns/2 - sl/2 - 1;
	return pad;
}

static 
display_menu_array(a,n,h)
char **a;	/* menu strings. */
int n;		/* number of strings. */
int h;		/* highlight string h */
{
	int cur_row;
	int i;

        cur_row = 0;
	for ( i=0;i<n;i++ ) {
		if ( h == i ) attron(A_BOLD);
		next(a[i]);
		if ( h == i ) attroff(A_BOLD);
		if ( !i ) {
			next("");
		}
	}
        refresh();
}
