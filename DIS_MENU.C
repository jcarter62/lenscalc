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
#include "table.h"
#ifdef dos
#include <errno.h>
#else
#include <sys/errno.h>
#endif
#include <signal.h>

/* dis_menu.c 5.12 8/23/89 */

#define LAST_MENU 2 		/* 0,1,2 */

static char	*menu_str[15] = {
	"     Main Menu : Press space for next menu.",
	"1 - Center Thickness   4 - Index       7 - print",
	"2 - Edge Thickness     5 - Base curve  8 - new lens",
	"3 - Power              6 - Lenticular  9 - junction",
	"",
	"     Menu 2 : Press space for next menu.",
	"p - Prism,edge thick  q - Exit           s - Special, Unusual Lenses",
	"t - Toric Menu        V - Version.       x - Exit & save",
	"m - Pick Material     b - bifocal menu   g - gripe to author of program",
	"",
	"     Menu 3 : Press space for next menu.",
	"Cntr-L - Re-draw screen",
	"Esc    - Exit & save",
	"S      - Save Current Lens",
	"" };


static menu_num = 0;
extern int	CLOCK;

int
dis_menu()
{
	extern int	input_table[];
	int	cur_row;
	int	c;

loop:
	cur_row = 0;
	move(cur_row++, 0);
	clrtoeol();
	printw(menu_str[menu_num*5]);
	move(cur_row++, 0);
	clrtoeol();
	printw(menu_str[menu_num*5+1]);
	move(cur_row++, 0);
	clrtoeol();
	printw(menu_str[menu_num*5+2]);
	move(cur_row++, 0);
	clrtoeol();
	printw(menu_str[menu_num*5+3]);
	refresh();
	noecho();	/* turn echo off */
	do {
		c = getch();

		if ( c == ' ' ) {
			menu_num++;
			if ( menu_num > LAST_MENU )
				menu_num = 0;
			goto loop;
		}
	} while ( input_table[c] == NO_OP );

	for (cur_row = 0; cur_row < 4; cur_row++) {
		move(cur_row, 0);
		clrtoeol();
	}
	refresh();
	echo();
	return input_table[c];
}

