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

static int	flag = 0;
char	scrsem[20];	/* file name for semaphore */

/* init.c 5.13 7/27/89 */

void
init()
{
	extern char	*ttyname(int);
	extern int	CLOCK, DEBUG;
	register int	i = 0;
	char	number[80];
	char	*ptr, *p;

	if ( DEBUG ) {
		dbg = fopen("DEBUG.OUT", "w");
		if ( dbg == NULL ) {
			printf("Error in open DEBUG.OUT\n");
			exit(0);
		}
	}

	initscr();
	clear();
	if (flag == 0 ) {
		flag = 1;
		savetty();
	}
#ifdef dos
	nonl();
	cbreak();
	echo();
#else
	if ( isatty(1) == 1 ) {
		nonl();
		cbreak();
		echo();
	}
#endif
}


