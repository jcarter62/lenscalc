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

/* iw.c 5.9 8/29/90 */

#ifdef __TURBOC__
	typedef int WINDOW;
	WINDOW iw = 0;
#else
	WINDOW *iw = (WINDOW *)0; /* null */
#endif

static char stat_buf[80];

static void
init_iw()
{
        iw = stdscr;
}

double
get_float(msg,last_val)
char *msg;
double *last_val;
{
        extern void init_iw(void);
        int sl;

        if ( iw == (WINDOW *)NULL ) init_iw();  /* null */

        wmove(iw,0,0);
        clrtoeol();
        if ( last_val == (double *)NULL )
                printw(msg);
        else
                printw("%s (%7.3f)",msg,*last_val);
        refresh();

        getstr(stat_buf);
	bs_check(stat_buf);
        sl = strlen(stat_buf);
        if ( sl <= 0 )
                return *last_val;
        else
                return atof(stat_buf);

}

int
get_int(msg,retval)
char *msg;
int retval;
{
        int val = 0;
        int sl;

        if ( iw == (WINDOW *)0 ) init_iw();

        wmove(iw,0,0);
        clrtoeol();
        printw("%s <%d> ",msg,retval);
        refresh();

        getstr(stat_buf);
	bs_check(stat_buf);
        sl = strlen(stat_buf);
        if (sl <= 0)
                return retval;
        else {
                val = atoi(stat_buf);
                return val;
        }
}

get_string(msg,buf)
char *msg;
char *buf;
{
        if ( iw == (WINDOW *)NULL ) init_iw();  /* null */

        wmove(iw,0,0);
        clrtoeol();
        printw("%s (%s)",msg,buf);
        refresh();

        getstr(stat_buf);
	bs_check(stat_buf);
        if ( strlen(stat_buf) <= 0 )
                return;
        strcpy(buf,stat_buf);
        return;
}

#define next(string) {move(cur_row++,0);clrtoeol();printw("%s",string);}

double get_dbl(s,d)
char *s;
double *d;
{
	extern int cur_row;

	sprintf(stat_buf,"%s (%g) ",s,*d);
	next(stat_buf);
	refresh();
	echo();
	getstr(stat_buf);
	noecho();
	bs_check(stat_buf);
	if ( strlen(stat_buf) < 1 )
		return *d;
	else
		return atof(stat_buf);
}

double get_str(msg,str)
char *msg;
double *str;
{
	extern int cur_row;

	sprintf(stat_buf,"%s (%s) ",msg,str);
	next(stat_buf);
	refresh();
	echo();
	getstr(stat_buf);
	noecho();
	bs_check(stat_buf);
	if ( strlen(stat_buf) < 1 )
		return;
	else
		strcpy(str,stat_buf);
}

bs_check(s)
char *s;
{
	char *p;

	p = s;
	while ( *p ) {
		if ( *p == 8 ) {
			s--; p++;
		} else {
			*s++ = *p++;
		}
	}
	*s++ = *p++;
}

dis_er_msg(s)
char *s;
{
        move(2,0);
        clrtoeol();
	standout();
        printw("Error: %s",s);
	standend();
        refresh();
}

ers_er_msg()
{
        move(2,0);
        clrtoeol();
}
