/* @(#) pr_lens.c 5.44@(#) 12/26/90 10:28:56 */
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

extern int	cur_row;
#define ON	(1)
#define OFF	(0)

#define PRINTING(filep) (filep != NULL)
#define underline(onoff,filepointer) { blink(onoff,filepointer); }

/* List of procedures/functions pr_lens.c
* pwr_fmt( d ) [static , returns char *]
* reset_output( fp ) [static void]
* finish_output( fp ) [static void]
* outline( s, fp ) [static void]
* nextline( fp ) [static void]
* print_lens( l, filep ) [public void]
* display_lens( l ) [public void]
* plens( l, fp ) [static void]
*/

/* forward declarations */
char	*pwr_fmt(double);
static void reset_output(FILE *);
static void finish_output(FILE *);
void outline(char *, FILE *);
void print_lens(LENS *, FILE *);
void display_lens(LENS *);
static void plens(LENS *, FILE *);
static void bold(int, FILE *);

static double	min_ct;
static double	min_edge;
static double	min_jt;
static double	min_lower_dial;
static double	min_front_cyl;

/* external definisions */
double	round(double);             /* math01.c */
double	round_001(double);         /* math01.c */
double	minimum_ct(char *, double); /* mod_mat.c */
double	minimum_et(char *, double); /* mod_mat.c */
double	minimum_jt(char *, double); /* mod_mat.c */


/*
* b l i n k ( on_off, fp ) [static void]
*
* Parameters :
*   on_off : true/false flag.
*   fp : FILE *
*
* Purpose : turn blink on/off
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 10, 1989; 04:05 pm
*/
static void blink(on_off, fp)
int	on_off;
FILE *fp;
{
	extern int	BLINK_ON;	/* defined in main.c */

	if ( !BLINK_ON ) 
		return;

	if ( fp == NULL ) {
		if (on_off)
			{attron(A_BLINK);}
		else
			{attroff(A_BLINK);}
	} else {
		if (on_off) {
			UNDERLINEON(fp);
		} else {
			UNDERLINEOFF(fp);
		}
	}
}


/*
* b o l d  ( on_off, fp ) [static void]
*
* Parameters :
*   on_off : true/false flag.
*   fp : FILE *
*
* Purpose : turn bold on/off
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 10, 1989; 04:05 pm
*/
static void bold(on_off, fp)
int	on_off;
FILE *fp;
{
	if ( fp == NULL ) {
		if (on_off)
			{attron(A_BOLD);}
		else
			{attroff(A_BOLD);}
	} else {
		if (on_off) {
			BOLDON(fp);
		} else {
			BOLDOFF(fp);
		}
	}
}


/*
* p w r _ f m t  ( d ) [static , returns char *]
*
* Parameters :
*   d : double value.
*
* Purpose : format double value in power format
*           'plano' +|-##.###.
*
* Globals : n.a. 
*
* Returns : pointer to static character buffer.
* Date : Mon Apr 10, 1989; 03:47 pm
*/
char	*pwr_fmt(d)
double	d;
{
	static char	buf[20];

	if ( d == 0.0 )
		sprintf(buf, "  Plano");
	else
		sprintf(buf, "%+7.3f", d);
	return(buf);
}


/*
* r e s e t _ o u t p u t  ( fp ) [static void]
*
* Parameters :
*   fp : file pointer or NULL if output to screen.
*
* Purpose : reset output device.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 10, 1989; 11:27 am
*/
static void reset_output(fp)
FILE *fp;
{
	if ( fp == NULL ) {
		cur_row = 5;
		move(cur_row, 0);
		clrtoeol();
	} else {
		if ( print_dest != to_file ) {
			RESET(fp);
			ELITE(fp);
			LPI8(fp);
		}
	}
}


/*
* f i n i s h _ o u t p u t  ( fp ) [static void]
*
* Parameters :
*   fp :  file pointer or NULL
*
* Purpose : cleanup stuff, finish up output to fp device.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 10, 1989; 03:32 pm
*/
static void finish_output(fp)
FILE *fp;
{
	if ( fp == NULL ) {
		while ( cur_row < 23 ) {
			move(cur_row++, 0);
			clrtoeol();
		}
		refresh();
	}
}


/*
* o u t l i n e  ( s, fp ) [static void]
*
* Parameters :
*   s : character string to output.
*   fp : where to output string.
*        fp = NULL, output to curses screen.
*        fp !=NULL, output to fp.
*
* Purpose : output string to output device/file.
*
* Globals : n.a. 
*
* Returns : n.a.
* Date : Mon Apr 10, 1989; 03:50 pm
*/
void outline(s, fp)
char	*s;
FILE *fp;
{
	if ( fp == NULL )	/* to curses screen */
		{addstr(s);}
	else
		fprintf(fp, "%s", s);
}


/*
* n e x t l i n e  ( fp ) [static void]
*
* Parameters :
*   fp : where to output string.
*        fp = NULL, output to curses screen.
*        fp !=NULL, output to fp.
*
* Purpose : output new-line.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 10, 1989; 03:52 pm
*/
void nextline(fp)
FILE *fp;
{
	if ( fp == NULL ) {	/* curses screen */
		move(cur_row++, 0);
		clrtoeol();
	} else
		fprintf(fp, "\n");
}



/*
* p r i n t _ l e n s  ( l, filep ) [public void]
*
* Parameters :
*   l : Lens structure pointer
*   filep : file pointer to output device/file.
*      if (NULL), then send to printer. use (p_open()/close()).
*      if (not NULL), then just send to filep.
*
* Purpose : output lens form to output device/file.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jun 15, 1989; 03:53 pm
*/
void print_lens(l, filep)
LENS *l;
FILE *filep;
{
	FILE * fp;

	if ( filep == NULL ) {
		if ( (fp = open_pfile()) == NULL )
			return;
	} else
		fp = filep;

	plens(l, fp);	/* print lens to fp */

	if ( filep == NULL )
		close_pfile();
}


/*
* d i s p l a y _ l e n s  ( l ) [public void]
*
* Parameters :
*   l : pointer to lens.
*
* Purpose : same as print_lens(l,fp) except, display 
* output on curses screen instead of a file.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 10, 1989; 03:55 pm
*/
void display_lens(l)
LENS *l;
{
	plens(l, NULL);
}



/*
* p l e n s  ( l, fp ) [static void]
*
* Parameters :
*   l : lens pointer to print.
*   fp : file pointer to output device.
*        if ( fp == NULL ) then output to curses screen.
*        if ( fp != NULL ) then output to fp.
*
* Purpose : print lens form.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 10, 1989; 03:42 pm
*/
static void plens(l, fp)
register LENS *l;
register FILE *fp;
{
	register int	i;
	extern char * lt_base(int);
	extern char * lt_front(LENS *);

	min_ct = minimum_ct(l->mat_type, l->power);
	min_edge = minimum_et(l->mat_type, l->power);
	min_jt = minimum_jt(l->mat_type, l->power);
	min_lower_dial = 0.30;
	min_front_cyl = 0.03;

	reset_output(fp);
	if ( print_dest == to_file ) {
		extern char *notes_line;
		if ( notes_line != NULL ) {
			outline("---------------------------------------------------",fp);
			nextline(fp);
			outline(notes_line,fp);
			nextline(fp);
			nextline(fp);
		}
	}
	/* nextline(fp); */
	outline(lt_base(l->lens_type), fp );
	nextline(fp);
	if ( l->lens_type & LT_TOR_SEC ) {
		if ( l->lens_type & LT_LENTIC ) {
			sprintf(buffer, "         Radius    O.Z./ P.OZ   L.E.T.   J.T.");
			outline(buffer, fp); 
			nextline(fp);
			sprintf(buffer, "  Steep   %5.2f   ", l->t.steep_radius);
			outline(buffer, fp);

			if ( l->t.ooz_steep < 0.0 )
				sprintf(buffer, "%5.2f/-----", l->t.oz_steep);
			else
				sprintf(buffer, "%5.2f/%5.2f", l->t.oz_steep,
				     l->t.ooz_steep);
			outline(buffer, fp);

			sprintf(buffer, "   %5.3f   ", l->t.et_steep);
			 {
				dbl_tmp = round_001(l->t.et_steep);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i ) 
					blink(ON, fp);
				outline(buffer, fp);
				if ( i ) 
					blink(OFF, fp);
			}
			sprintf(buffer, "%5.3f", l->t.jt_steep);
			 {
				dbl_tmp = round_001(l->t.jt_steep);
				i = ( dbl_tmp < min_jt ? (1) : (0) );
				if ( i ) 
					blink(ON, fp);
				outline(buffer, fp);
				if ( i ) 
					blink(OFF, fp);
			}
			nextline(fp);

			sprintf(buffer, "   Flat  -%5.2f   %5.2f/%5.2f   ",
			                       l->t.flat_radius, l->t.oz_flat,
			     l->t.ooz_flat);
			outline(buffer, fp);
			sprintf(buffer, "%5.3f   ", l->t.et_flat);
			 { /* see if et_flat is < min_edge */
				dbl_tmp = round_001(l->t.et_flat);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i ) 
					blink(ON, fp);
				outline(buffer, fp);
				if ( i ) 
					blink(OFF, fp);
			}

			sprintf(buffer, "%5.3f", l->t.jt_flat);
			 {
				dbl_tmp = round_001(l->t.jt_flat);
				i = ( dbl_tmp < min_jt ? (1) : (0) );
				if ( i ) 
					blink(ON, fp);
				outline(buffer, fp);
				if ( i ) 
					blink(OFF, fp);
			}
			nextline(fp);

		} else {
			sprintf(buffer, "         Radius     OZ / P.OZ   E.T.");
			outline(buffer, fp); 
			nextline(fp);
			sprintf(buffer, "  Steep   %5.2f   ", l->t.steep_radius);
			outline(buffer, fp);

			if ( l->t.ooz_steep < 0.0 )
				sprintf(buffer, "%5.2f/-----", l->t.oz_steep);
			else
				sprintf(buffer, "%5.2f/%5.2f", l->t.oz_steep,
				     l->t.ooz_steep);
			outline(buffer, fp);

			sprintf(buffer, "   %5.3f", l->t.et_steep);
			 {
				dbl_tmp = round_001(l->t.et_steep);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			nextline(fp);
			sprintf(buffer, "   Flat  -%5.2f   %5.2f/%5.2f   ",
			            l->t.flat_radius, l->t.oz_flat, l->t.ooz_flat,
			     l->t.et_flat);
			outline(buffer, fp);
			sprintf(buffer, "%5.3f", l->t.et_flat);
			 {
				dbl_tmp = round_001(l->t.et_flat);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i ) 
					blink(ON, fp);
				outline(buffer, fp);
				if ( i ) 
					blink(OFF, fp);
			}
			nextline(fp);
		}
		outline("         ------  ", fp);
		bold(ON, fp);
		outline("Toric Secondary,", fp); 
		bold(OFF, fp);
		nextline(fp);
		sprintf(buffer, "        %7.2f  ", l->t.steep_radius - l->t.flat_radius ) ;
		outline(buffer, fp);
		bold(ON, fp);
		outline("Polish Base Only", fp);
		bold(OFF, fp);
		nextline(fp);
		nextline(fp);
	} else if ( l->lens_type & LT_TOR_OOZ ) {
		if ( l->lens_type & LT_LENTIC ) {
			outline("         Radius    O.Z.   L.E.T.   J.T.",
			     fp);
			nextline(fp);
			sprintf(buffer, "  Steep   %5.2f   %5.2f   ", l->t.steep_radius,
			     						  l->t.oz_steep);
			outline(buffer, fp);

			sprintf(buffer, "%5.3f   ", l->t.et_steep);
			 {
				dbl_tmp = round_001(l->t.et_steep);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i ) 
					blink(ON, fp);
				outline(buffer, fp);
				if ( i ) 
					blink(OFF, fp);
			}

			sprintf(buffer, "%5.3f", l->t.jt_steep);
			 {
				dbl_tmp = round_001(l->t.jt_steep);
				i = ( dbl_tmp < min_jt ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			nextline(fp);

			sprintf(buffer, "   Flat  -%5.2f   %5.2f   ", l->t.flat_radius,
			     						  l->t.oz_flat);
			outline(buffer, fp);

			sprintf(buffer, "%5.3f   ", l->t.et_flat);
			 {
				dbl_tmp = round_001(l->t.et_flat);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}

			sprintf(buffer, "%5.3f", l->t.jt_flat);
			 {
				dbl_tmp = round_001(l->t.jt_flat);
				i = ( dbl_tmp < min_jt ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			nextline(fp);
		} else {
			outline("         Radius    O.Z.    E.T.", fp);
			nextline(fp);
			sprintf(buffer, "  Steep   %5.2f   %5.2f   ", l->t.steep_radius,
			     l->t.oz_steep);
			outline(buffer, fp);

			sprintf(buffer, "%5.3f", l->t.et_steep);
			 {
				dbl_tmp = round_001(l->t.et_steep);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			nextline(fp);

			sprintf(buffer, "   Flat  -%5.2f   %5.2f   ", l->t.flat_radius,
			     l->t.oz_flat);
			outline(buffer, fp);
			sprintf(buffer, "%5.3f", l->t.et_flat);
			 {
				dbl_tmp = round_001(l->t.et_flat);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			nextline(fp);
		}
		outline("         ------  Oval O.Z.", fp);
		nextline(fp);
		sprintf(buffer, "        %7.2f  >>> Cut No Polish <<<" ,
		    l->t.steep_radius - l->t.flat_radius ) ;
		outline(buffer, fp);
		nextline(fp);
		nextline(fp);
	} else if ( l->lens_type & LT_TOR_ROZ ) {
		outline("  Steep", fp);
		for (i = l->rings - 1; i > 0; i--) {
			sprintf(buffer, "%7.2f", l->radius[i]);
			outline(buffer, fp);
		}
		if ( l->lens_type & LT_LENTIC ) {
			sprintf(buffer, "| %5.3f ", l->t.et_steep);
			 {
				dbl_tmp = round_001(l->t.et_steep);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			sprintf(buffer, "%5.3f", l->t.jt_steep);
			 {
				dbl_tmp = round_001(l->t.jt_steep);
				i = ( dbl_tmp < min_jt ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
		} else {
			sprintf(buffer, "| %5.3f", l->t.et_steep);
			 {
				dbl_tmp = round_001(l->t.et_steep);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
		}
		nextline(fp);

		outline("   Flat", fp);
		for (i = l->round.rings - 1; i >= 0; i--) {
			sprintf(buffer, "%7.2f", l->round.rad[i]);
			outline(buffer, fp);
		}
		if ( l->lens_type & LT_LENTIC ) {
			sprintf(buffer, "| %5.3f ", l->t.et_flat);
			 {
				dbl_tmp = round_001(l->t.et_flat);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			sprintf(buffer, "%5.3f", l->t.jt_flat);
			 {
				dbl_tmp = round_001(l->t.jt_flat);
				i = ( dbl_tmp < min_jt ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
		} else {
			sprintf(buffer, "| %5.3f", l->t.et_flat);
			 {
				dbl_tmp = round_001(l->t.et_flat);
				i = ( dbl_tmp < min_edge ? (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
		}
		nextline(fp);

		sprintf(buffer, "    -  ");
		outline(buffer, fp);
		for (i = l->round.rings - 1; i >= 0; i--)
			outline(" ------", fp);

		if ( l->lens_type & LT_LENTIC )
			outline("| ^^^^^ ^^^^^", fp);
		else
			outline("| ^^^^^", fp);
		nextline(fp);

		outline("       ", fp);
		for (i = l->round.rings - 1; i >= 0; i--) {
			sprintf(buffer, "%7.2f", l->radius[i+1] - l->round.rad[i]);
			outline(buffer, fp);
		}
		if ( l->lens_type & LT_LENTIC )
			outline("| L.E.T.  J.T.", fp);
		else
			outline("| E.T.", fp);
		nextline(fp);

		outline(" --- Toric Round O.Z.", fp); 
		nextline(fp);
		outline(" >>> Cut No Polish <<<", fp); 
		nextline(fp);
	}

	/* 
	* ---------------------
	* Normal print of rings 
	* ---------------------
	*/
	if ( l->lens_type & LT_TRI_DD ) {
		double angle;
		double sum_value;

		angle = asin(l->itrm.ballast/l->itrm.near_rad);
/*           123456789012345678901234567890123456789012345678901234567890 */
/*           xxxxxx xxxxxx xxxxxx xxxxxx xxxxxx xxxxxx xxxxxx xxxxxx 7890 */
		outline("  Diam   Rad   Touch   Add   Total   SUM    Offset",fp);
		nextline(fp);

		for (i = 0; i < l->rings; i++) {
			int	width;
			int	d1, d2;
	
			if ( i ) {
				sprintf(buffer, "%6.3lf %6.3lf %6.3lf ", l->diameter[i],
				     	l->radius[i], l->rad_dif[i]);
				outline(buffer, fp);
				sprintf(buffer, "%6.3lf %6.3lf %6.3lf", l->sag_dif[i],
				     	l->sag_plus_rad[i], l->sag_sum[i]);
				outline(buffer, fp);
			} else {
				sprintf(buffer, "%6.3lf %6.3lf        %6.3lf <%4.2lfcm>     ",
				            l->diameter[i], l->radius[i], l->sag_dif[i],
				     l->min_bt);
				outline(buffer, fp);
			}
			/* 
	    * display width of current curve in the form of /##.
	    * at far right of printout of lens information.
	    */
			if ( i != (l->rings - 1) ) {
				register int	j;
	
				dbl_tmp = round(l->diameter[i] - l->diameter[i+1]) /
				    2.0 ;
				/* width = (int)(dbl_tmp * 100.0); */
				sprintf(buffer, "/%2.2lf", dbl_tmp);
				/* get rid of 0. in /0.##. */
				j = 1;
				while ( buffer[j] ) {
					buffer[j] = buffer[j+2];
					j++;
				}
				outline(buffer, fp);
			} else {
				/*       /00      */
				outline("   ",fp);
			}
			/* xxxxxx Here put offset for each radius. */
			sprintf(buffer,"(%4.2lf)",l->radius[i] * sin(angle) );
			outline(buffer,fp);

			nextline(fp);
		}

		/* store sag_sum value */
		sum_value = l->sag_sum[l->rings - 1];

		/* intermediate curves */
		outline("--------------------------------------------------",fp);
		nextline(fp);

		sprintf(buffer, "%6.3lf %6.3lf %6.3lf ", 
			l->itrm.dist_oz,l->itrm.dist_rad,l->itrm.touch);
		outline(buffer, fp);
		sum_value = sum_value + (l->itrm.touch + l->itrm.tch_add);
		sprintf(buffer, "%6.3lf %6.3lf %6.3lf   (%4.2lf)",  /* includes /00 */
			l->itrm.tch_add,l->itrm.touch + l->itrm.tch_add, sum_value,
			l->itrm.prism+l->itrm.ballast );
		outline(buffer, fp);
		nextline(fp);

		/* distance powers */
		sprintf(buffer, "%6.3lf %6.3lf %6.3lf ", 
			l->dist.dist_oz,l->dist.dist_rad,l->dist.touch);
		outline(buffer, fp);
		sum_value = sum_value + (l->dist.touch + l->dist.tch_add);
		sprintf(buffer, "%6.3lf %6.3lf %6.3lf   (%4.2lf)",  /* includes /00 */
			l->dist.tch_add,l->dist.touch + l->dist.tch_add, sum_value,
			l->dist.prism + (l->itrm.prism + l->itrm.ballast) );
		outline(buffer, fp);
		nextline(fp);

	} 
	else 
	{ /* Normal Lenses */
		for (i = 0; i < l->rings; i++) {
			int	width;
			int	d1, d2;
	
			if ( i ) {
				sprintf(buffer, "%7.3lf %7.3lf %7.3lf ", l->diameter[i],
				     	l->radius[i], l->rad_dif[i]);
				outline(buffer, fp);
				sprintf(buffer, "%7.3lf %7.3lf %7.3lf", l->sag_dif[i],
				     	l->sag_plus_rad[i], l->sag_sum[i]);
				outline(buffer, fp);
			} else {
				sprintf(buffer, "%7.3lf %7.3lf         %7.3lf  <%4.2lfcm>      ",
				            l->diameter[i], l->radius[i], l->sag_dif[i],
				     l->min_bt);
				outline(buffer, fp);
			}
			/* 
	    * display width of current curve in the form of /##.
	    * at far right of printout of lens information.
	    */
			if ( i != (l->rings - 1) ) {
				register int	j;
	
				dbl_tmp = round(l->diameter[i] - l->diameter[i+1]) /
				    2.0 ;
				/* width = (int)(dbl_tmp * 100.0); */
				sprintf(buffer, "/%2.2lf", dbl_tmp);
				/* get rid of 0. in /0.##. */
				j = 1;
				while ( buffer[j] ) {
					buffer[j] = buffer[j+2];
					j++;
				}
				outline(buffer, fp);
			}
			nextline(fp);
		}
	}

	sprintf(buffer, "%-30s%18s", lt_front(l), l->mat_type);
	outline(buffer, fp); 
	nextline(fp);
	sprintf(buffer, "%7.3lf ", l->front_radius);
	outline(buffer, fp);
	if ( fp != NULL || l->pref == CT_PREF )
		bold(ON, fp);

	sprintf(buffer, "%7.3lf ", l->center_thick);
	 {
		dbl_tmp = round_001(l->center_thick);
		i = ( dbl_tmp < min_ct ? (1) : (0) );
		if ( i ) 
			blink(ON, fp);
		outline(buffer, fp);
		if ( i ) 
			blink(OFF, fp);
	}

	if ( fp != NULL || l->pref == CT_PREF )
		bold(OFF, fp);

	/*
* edge thickness
*/
	if ( l->lens_type & LT_LENTIC ) {
		if ( l->lens_type & LT_TOR_SEC )
			dbl_tmp = l->t.lt_et;
		else
			dbl_tmp = l->lt_et;
	} else
		dbl_tmp = l->edge_thick;

	/* center thickness, always bold on paper */
	if ( fp == NULL && l->pref == ET_PREF )
		bold(ON, fp);

	dbl_tmp1 = round_001(dbl_tmp);

	if ( dbl_tmp1 < min_edge ) /* if printing et is < minimum edge thickness */
		blink(ON, fp);
	sprintf(buffer, "%6.3lf ", dbl_tmp);
	outline(" ", fp);
	outline(buffer, fp);

	if ( dbl_tmp1 < min_edge ) 
		blink(OFF, fp);

	if ( fp == NULL && l->pref == ET_PREF )
		bold(OFF, fp);

	/*
* power output.
*/
	if ( l->lens_type & LT_TOR_OOZ || l->lens_type & LT_TOR_ROZ ) {
		sprintf(buffer, "%s/", pwr_fmt(l->t.pow_flat));
		outline(buffer, fp);
		sprintf(buffer, "%s   ", pwr_fmt(l->t.pow_steep));
		outline(buffer, fp);
	} else if ( l->lens_type & LT_TORIC_FRONT ) {
		sprintf(buffer, "%s/", pwr_fmt(l->power));
		outline(buffer, fp);
		sprintf(buffer, "%s   ", pwr_fmt(l->power + l->t.cyl));
		outline(buffer, fp);
	} else {
		sprintf(buffer, "%s           ", pwr_fmt(l->power));
		outline(buffer, fp);
	}

	sprintf(buffer, "%5.1f", ( (l->index - 1.0) * 1000.0 ) );
	outline(buffer, fp);
	nextline(fp);

	if ( (l->extra_flags & EXRA_PRISM) || 
			 (l->lens_type & (LT_DDECARLE|LT_TRI_DD)) ) {
		if ( l->lens_type & LT_TRI_DD ) {
			if ( l->itrm.ballast > 0 ) {
			sprintf(buffer, ">>> %4.2f prism", l->itrm.ballast / 0.16);
			outline(buffer, fp); 

			sprintf(buffer, " %5.3f minimum e.t.", 
			l->edge_thick - ( l->diameter[0] * (l->itrm.ballast/0.16)/(2.0 * 49)));

			outline(buffer, fp);

			outline(" <<<", fp); 
			nextline(fp);
			}
		} 
		else 
		{
			sprintf(buffer, ">>> %4.2f prism", l->bal_prism / 0.16);
			bold(ON, fp); 
			outline(buffer, fp); 
			bold(OFF, fp);
	
			sprintf(buffer, " %5.3f minimum e.t.", l->min_et);
			outline(buffer, fp);

			bold(ON, fp); 
			outline(" <<<", fp); 
			bold(OFF, fp);
			nextline(fp);
		}
	}

	if ( l->lens_type & LT_LENTIC ) {
		if ( l->lens_type & LT_TOR_SEC ) {
			if ( l->t.lt_rad < l->front_radius ) {
				outline(" LT-Rad   LT-OZ  Junc-T  Tk-off",
				     fp);
				nextline(fp);
				sprintf(buffer, "%7.3lf %7.3lf ", l->t.lt_rad,
				     l->t.lt_oz);
				outline(buffer, fp);
				/* blink jt if < min_jt */
				sprintf(buffer, "%7.3lf ", l->t.lt_jt);
				 {
					dbl_tmp = round_001(l->t.lt_jt);
					i = ( dbl_tmp < min_jt ? (1) : (0) );
					if ( i )
						blink(ON, fp);
					outline(buffer, fp);
					if ( i )
						blink(OFF, fp);
				}

				outline(" ", fp);
				bold(ON, fp);
				sprintf(buffer, "%6.3lf", l->t.lt_tkoff);
				outline(buffer, fp);
				bold(OFF, fp);
				nextline(fp);
			} else {
				outline(" LT-Rad   LT-OZ  Junc-T   Set  ",
				     fp);
				nextline(fp);
				sprintf(buffer, "%7.3lf %7.3lf ", l->t.lt_rad,
				     l->t.lt_oz);
				outline(buffer, fp);
				sprintf(buffer, "%7.3lf ", l->t.lt_jt);
				 {
					dbl_tmp = round_001(l->t.lt_jt);
					i = ( dbl_tmp < min_jt ? (1) : (0) );
					if ( i )
						blink(ON, fp);
					outline(buffer, fp);
					if ( i )
						blink(OFF, fp);
				}
				outline(" ", fp);
				bold(ON, fp);
				sprintf(buffer, "%6.3lf", l->t.lt_set);
				outline(buffer, fp);
				bold(OFF, fp);
				nextline(fp);
			}
			sprintf(buffer, "Cap < =%6.2f, Lower Dial =", l->t.lt_ca);
			outline(buffer, fp);
			sprintf(buffer, "%6.2f", l->t.lt_ld);
			 {
				i = ( fabs(l->t.lt_ld) < min_lower_dial ?
				    (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			nextline(fp);
		} else {
			if ( l->lt_rad < l->front_radius ) {
				outline(" LT-Rad   LT-OZ  Junc-T  Tk-off",
				     fp);
				nextline(fp);
				sprintf(buffer, "%7.3lf %7.3lf ", l->lt_rad,
				     l->lt_oz);
				outline(buffer, fp);

				sprintf(buffer, "%7.3lf ", l->lt_jt);
				 {
					dbl_tmp = round_001(l->lt_jt);
					i = ( dbl_tmp < min_jt ? (1) : (0) );
					if ( i )
						blink(ON, fp);
					outline(buffer, fp);
					if ( i )
						blink(OFF, fp);
				}

				outline(" ", fp);
				bold(ON, fp);
				sprintf(buffer, "%6.3lf", l->lt_tkoff);
				outline(buffer, fp); 
				bold(OFF, fp);
				nextline(fp);
			} else {
				outline(" LT-Rad   LT-OZ  Junc-T   Set  ",
				     fp);
				nextline(fp);
				sprintf(buffer, "%7.3lf %7.3lf ", l->lt_rad,
				     l->lt_oz);
				outline(buffer, fp);

				sprintf(buffer, "%7.3lf ", l->lt_jt);
				 {
					dbl_tmp = round_001(l->lt_jt);
					i = ( dbl_tmp < min_jt ? (1) : (0) );
					if ( i )
						blink(ON, fp);
					outline(buffer, fp);
					if ( i )
						blink(OFF, fp);
				}

				outline(" ", fp);
				bold(ON, fp);
				sprintf(buffer, "%6.3lf", l->lt_set);
				outline(buffer, fp); 
				bold(OFF, fp);
				nextline(fp);
			}
			sprintf(buffer, "Cap < =%6.2f, Lower Dial =", l->cap_ang);
			outline(buffer, fp);
			sprintf(buffer, "%6.2f", l->lower_dial);
			 {
				i = ( fabs(l->lower_dial) < min_lower_dial ?
				    (1) : (0) );
				if ( i )
					blink(ON, fp);
				outline(buffer, fp);
				if ( i )
					blink(OFF, fp);
			}
			nextline(fp);
		}
	}

	if ( l->lens_type & LT_PLATEAU ) {
		outline("  Ang    LowDial   Touch  Cut-to", fp);
		nextline(fp);
		sprintf(buffer, "%7.3f %7.3f %7.3f %7.3f", l->cap_ang_p,
		    l->lower_dial_p, l->extra1l, l->extra1l + l->extra2l);
		outline(buffer, fp); 
		nextline(fp);
	}

	if ( l->lens_type & LT_DECARLE ) {
		sprintf(buffer, "Decarle Bif: Near%s ", pwr_fmt(l->near_pow));
		outline(buffer, fp);
		sprintf(buffer, "Add%s ", pwr_fmt(l->add_pow));
		outline(buffer, fp);
		sprintf(buffer, "Trial%s", pwr_fmt(l->trial_pow));
		outline(buffer, fp);
		nextline(fp);
	}

	if ( l->lens_type & LT_OP_CRES ) {
		outline("One piece Bifocal : Add Power ", fp);
		nextline(fp);
		sprintf(buffer, "%7.3f %7.3f %7.3f %s", l->extra3l, l->center_thick,
		     l->extra4l, pwr_fmt(l->power + l->add_pow));
		outline(buffer, fp); 
		nextline(fp);
	}

	if ( l->lens_type & LT_TOR_OOZ || l->lens_type & LT_TOR_ROZ ) {
		switch (l->t.tor_type) {
		case TOR_SPH_F :
			outline("-Toric Base, Spherical Front", fp); 
			nextline(fp);
			sprintf(buffer, " No Mark (%g x %g)", l->t.cyl, l->t.axis);
			outline(buffer, fp); 
			nextline(fp);
			break;
		case TOR_CYL_F1:
			if ( l->extra2l > 0.0 ) {
				outline("-Bitoric - Mark at Screw.", fp);	
				nextline(fp);
				print_cyl(l, l->extra1l, l->t.cyl, l->extra2l, 999.0,
			     	l->extra3l, fp);

			} else {
				outline("-Bitoric ", fp);	
				nextline(fp);
			  outline("-Lay Flat Meridian at 180 degrees.",fp);
				nextline(fp);
			  outline(" Short Mark at 180 degrees.", fp); 
				nextline(fp);
				print_cyl(l, l->extra1l, l->t.cyl, l->extra2l, 999.0,
			     	l->extra3l, fp);
			}
			break;
		case TOR_CYL_F2:
			sprintf(buffer, "-Toric Base, Spherical Front(%g x %g)",
			            l->t.cyl, l->t.axis);
			outline(buffer, fp); 
			nextline(fp);
			sprintf(buffer, "-Lay Flat meridian at %lg degrees,",
			     l->t.axis);
			outline(buffer, fp); 
			nextline(fp);
			sprintf(buffer, " mark at 90 degrees (Top of Lens)");
			outline(buffer, fp); 
			nextline(fp);
			break;
		case TOR_CYL_F3:
		case TOR_CYL_F4:
			outline("-Bitoric.", fp); 
			nextline(fp);
			sprintf(buffer, " Lensometer Power %s", pwr_fmt(l->t.v3.sph));
			outline(buffer, fp);
			sprintf(buffer, "%s", pwr_fmt(l->t.v3.cyl));
			outline(buffer, fp);
			sprintf(buffer, "(%s)x%4.0f", pwr_fmt((l->t.v3.cyl)
			    + (l->t.v3.sph)), l->t.v3.axis);
			outline(buffer, fp); 
			nextline(fp);

			print_cyl(l, l->extra1l, l->t.cyl, l->extra2l, l->t.axis,
			     l->extra3l, fp);
			sprintf(buffer, "-Lay Flat Meridian at %g degrees.",
			     l->t.axis_fm);
			outline(buffer, fp); 
			nextline(fp);
			sprintf(buffer, " Short Mark %g degrees.",  (l->t.axis +
			    90.0 > 180 ? l->t.axis - 90 : l->t.axis + 90 ));
			outline(buffer, fp); 
			nextline(fp);

		} /* end switch */
		if ( l->t.tor_type == TOR_CYL_F4 ) {
			outline("-Long mark at 90 degrees (top)", fp);
			nextline(fp);
		}
	}

	if (l->lens_type & LT_TORIC_FRONT) {
		double	dif;
		int	flag;
		extern double	fabs(double);

		if ( ! (l->lens_type & (LT_TRI_DD|LT_DDECARLE) ) )
			print_trunc(l,l->trunc,fp);

		dif = fabs((l->t.res_cyl) - (l->front_radius));
		print_cyl(l, l->t.res_cyl, l->t.cyl, dif, l->t.axis, l->extra3l, fp);
	}

	if ( l->lens_type & LT_ASPHERIC ) {
		outline("- Aspheric Lens (T. = Tangent)", fp); 
		nextline(fp);
		outline("T.Angle   T.oz     E.L.   T.E.L.   Dif", fp); 
		nextline(fp);
		sprintf(buffer, "%7.3f %7.3f %7.3f %7.3f %7.3f", (180.0 *
		    l->a_angle) / 3.14159, l->oz, l->edge_lift, l->a_el_tan,
		    (l->edge_lift - l->a_el_tan));
		outline(buffer, fp); 
		nextline(fp);
		outline("LowDial  Offset  T.Rad  OuterOZ", fp); 
		nextline(fp);
		sprintf(buffer, "%7.3f %7.3f %7.3f %7.3f", l->ld_dif, l->a_offset,
		     l->a_rad, l->a_outer_oz);
		outline(buffer, fp); 
		nextline(fp);
	}
  if ( l->lens_type & LT_TRI_DD ) 
   	decar_p_tri(l,fp);

	if ( l->lens_type & (LT_DDECARLE | LT_DECENTER) ) {
		if ( l->lens_type & LT_DDECARLE ) {
			outline("Decentered Decarle Bif: ", fp); 
			nextline(fp);
			sprintf(buffer, "--- Dist%s ", pwr_fmt(l->dist_pow));
			outline(buffer, fp);
			sprintf(buffer, "Add%s ", pwr_fmt(l->add_pow));
			outline(buffer, fp);
			sprintf(buffer, "Trial%s ---", pwr_fmt(l->trial_pow));
			outline(buffer, fp); 
			nextline(fp);
			if ( l->rotang != 0.0 ) {
				sprintf(buffer, "  (%s lens, Rotated %s %lg degrees)",
				     					(l->rot_rl == 'R' ? "Right" : "Left"),
				     					(l->rot_io == 'I' ? "In" : "Out"),
				    l->rotang);
				bold(ON, fp);
				outline(buffer, fp);
				bold(OFF, fp);
				nextline(fp);
			}
		} else {
			outline("Decentered O.Z.", fp);
			nextline(fp);
		}

		outline(" Steep R  Flat R    OZ    Seght   Trunc  Decentern",
		     fp);
		nextline(fp);
		if ( l->rotang == 0.0 ) {
			sprintf(buffer, "%7.3f %7.3f %7.3f %7.3f %7.3f %7.3f",
			            l->dist_rad, l->near_rad, l->oz, l->seght,
			     l->trunc, l->decenter);
		} else {
			sprintf(buffer, "%7.3f %7.3f %7.3f %3.1f/%3.1f %7.3f %7.3f",
			            l->dist_rad, l->near_rad, l->oz, l->seght,
			     l->rot_sht, l->trunc, l->decenter);
		}
		outline(buffer, fp); 
		nextline(fp);

		outline(" Ballast    x2    Dec.      x2    Total     x2",
		     fp);
		nextline(fp);
		if ( l->rotang == 0.0 ) {
			sprintf(buffer, "%7.3f %7.3f %7.3f %7.3f %7.3f %7.3f",
			            l->bal_prism, l->bal_prism * 2.0,  l->prism,
			     l->prism * 2.0, l->bal_prism + l->prism, (l->bal_prism
			    + l->prism) * 2.0);
		} else {
			char	*tmps = "  ";
			if ( l->rot_rl == 'R' ) {
				if ( l->rot_io == 'I' )
					strcpy(tmps, "<-");
				else
					strcpy(tmps, "->");
			} else {
				if ( l->rot_io == 'I' )
					strcpy(tmps, "->");
				else
					strcpy(tmps, "<-");
			}
			sprintf(buffer, "%7.3f %3.2f/%3.2f%2s%6.3f %7.3f %7.3f %7.3f",
			            l->bal_prism, l->extra1l * 2.0, l->extra2l
			    *2.0, tmps, l->prism, l->prism * 2.0, l->bal_prism
			    + l->prism, (l->bal_prism + l->prism - (l->bal_prism -
			    l->extra1l)) * 2.0 );
		}
		outline(buffer, fp); 
		nextline(fp);

		if ( l->bel_val < 0.0 ) {
			outline("  Touch    Add    Total    Above  LensSZ  Blk Dec",
			     fp);
			nextline(fp);
			l->bel_val = -1.0 * l->bel_val;
		} else {
			outline("  Touch    Add    Total    Below  LensSZ  Blk Dec",
			     fp);
			nextline(fp);
		}

		sprintf(buffer, "%7.3f %7.3f %7.3f %7.3f %7.3f %7.3f", l->touch,
		     l->tch_add, l->touch + l->tch_add,  l->bel_val, l->diameter[0],
		     l->blk_prism);
		outline(buffer, fp); 
		nextline(fp);

		/* skip one line */
		nextline(fp);
		sprintf(buffer, "(%5.3lf + Button Thk)*", l->near_rad);
		outline(buffer, fp);
		sprintf(buffer, "%5.3lf ", l->tch_angle);
		outline(buffer, fp);
		sprintf(buffer, "%+4.2lf = Drill Dist", l->bal_prism);
		outline(buffer, fp); 
		nextline(fp);

		 {
			register int	j;
			double	angle;

			angle = asin(l->bal_prism / l->near_rad);

			j = l->rings;
			if ( l->lens_type & LT_DECENTER )
				j--;

			for ( i = 0; i < j; i++) {
				sprintf(buffer, "%5.2lf Rad(%4.2lf Off),",
				     					l->radius[i], l->radius[i] * sin(angle));
				outline(buffer, fp);
			}
			nextline(fp);
		}

		if ( l->lens_type & LT_DDECARLE  && fp != NULL ) {
			char	ansr[5];

			strcpy(ansr, "n");
			get_string("Print Instructions <y/n>", ansr);
			if ( ansr[0] == 'y' )
				ddecar_inst(l, fp);
		}
	}
	nextline(fp);

	if ( print_dest == to_file ) {
		extern char *notes_line;
		if ( notes_line != NULL ) {
			bold(ON, fp);
			outline("- Manufacturing Record. - ",fp);
			bold(OFF, fp);
			nextline(fp);
			outline("          Cut              Polish       ",fp);
			nextline(fp);
			outline("      Name     Date     Name      Date   Measure",fp);
			nextline(fp);
			outline("Base  ______ __/__/__   _______ __/__/__ _______",fp);
			nextline(fp);
			outline("Front ______ __/__/__   _______ __/__/__",fp);
			nextline(fp);
			outline("Edging                  _______ __/__/__",fp);
			nextline(fp);
			nextline(fp);
			bold(ON, fp);
			outline("- Quality Inspection. -",fp);
			bold(OFF, fp);
			nextline(fp);
			/*                 1         2         3         4         5 */
			/*       012345678901234567890123456789012345678901234567890 */
outline( "\332------\302-------\302-------\302------\302------------\277",fp);
nextline(fp);
outline( "\263 B.C. \263 Power \263 Diam. \263 C.T. \263 By / Date  \263",fp);
nextline(fp);
outline( "\303------\305-------\305-------\305------\305------------\264",fp);
nextline(fp);
outline( "\263      \263       \263       \263      \263            \263",fp);
nextline(fp);
outline( "\300------\301-------\301-------\301------\301------------\331",fp);
nextline(fp);

		}
	}
	finish_output(fp);
}


ddecar_inst(l, fp)
LENS *l;
FILE *fp;
{
	MICROFONT(fp);
	fprintf(fp, "\nInstructions to cut lens\n");
	fprintf(fp, "1 - Put Blocked button into prism collet, mark\n");
	fprintf(fp, "    at screw and remove any side to side prism.\n");
	if ( l->rotang == 0.0 ) {
		fprintf(fp, "2 - Put %lg prism (%lg dif) into button and cut \n"
		    , l->bal_prism, (l->bal_prism * 2.0));
	} else {
		static char	*tmps = "  ";

		fprintf(fp, "2 - Put %lg prism (%lg dif) down, Looking at base of lens,\n"
		    , l->extra1l, (l->extra1l * 2.0));
		if ( l->rot_rl == 'R' ) {
			if ( l->rot_io == 'I' )
				strcpy(tmps, "<-");
			else
				strcpy(tmps, "->");
		} else {
			if ( l->rot_io == 'I' )
				strcpy(tmps, "->");
			else
				strcpy(tmps, "<-");
		}

		fprintf(fp, "    put %lg prism (%lg diff) %s and cut \n"
		    , l->extra2l, (l->extra2l * 2.0), tmps);
	}
	if ( l->bal_prism > .32 )
		fprintf(fp, "    Intermediate curve also.(NOT Peripheral)\n");
	else
		fprintf(fp, "    Peripheral & Intermediate curves also.\n");
	fprintf(fp, "3 - Cut Near Power Curve (%lg) and zero bottom \n" ,
	     l->near_rad);
	fprintf(fp, "    dial on final cut.\n");
	fprintf(fp, "4 - Back top dial out to Distance Power Curve \n");
	fprintf(fp, "    (%lg) and stop lathe.\n", l->dist_rad);
	if ( l->rotang == 0.0 ) {
		fprintf(fp, "5 - Put additional %lg prism (%lg total dif) \n"
		    , l->prism, (l->prism + l->bal_prism) * 2.0);
	} else {
		fprintf(fp, "5 - Put additional %lg prism (%lg total dif) \n"
		    , l->prism, (l->prism + l->extra1l) * 2.0);
	}
	fprintf(fp, "    into button.\n");
	fprintf(fp, "6 - Start lathe and touch at %lg(bottom dial) \n" ,
	    l->touch);
	fprintf(fp, "    and cut additional %lg (cut to %lg).\n" , l->tch_add,
	     l->touch + l->tch_add);
	fprintf(fp, "7 - All Done. Now Drill Hole in back\n");
	fprintf(fp, "    Measure Button thickness and complete formula\n");
	fprintf(fp, "    above. Put button in \"turn down lathe\" and\n");
	fprintf(fp, "    cut a mote, (drill distance) from center of back\n");
	fprintf(fp, "    of button.  Drill a hole by hand opposite the\n");
	fprintf(fp, "    Distance Power Curve (%lg)\n", l->dist_rad);
}


/*
* p r i n t _ trunc  ( l , fp ) 
*
* Parameters :
*   l : lens pointer of cutting lens.
*   fp : file pointer.
*
* Purpose : print out truncation line.
*
* Returns : n.a. 
* Date : Mon Jul 17, 1989; 08:10 am
*/
print_trunc(l, trunc_amt, fp)
LENS *l;
double trunc_amt;
FILE *fp;
{
	extern double	fabs(double);

	if ( trunc_amt <= 0 )
		return;

	outline("-Truncate ",fp);

	bold(ON,fp);
	sprintf(buffer, "%5.3f",trunc_amt);
	outline(buffer, fp);
	bold(OFF,fp);
	outline(" To ", fp);

	bold(ON, fp);
	sprintf(buffer, "(%7.2f/%7.2f)",l->diameter[0], l->diameter[0] - trunc_amt );
	outline(buffer, fp);
	bold(OFF, fp);

	nextline(fp);
}

/*
* p r i n t _ c y l  ( l, res_cyl, cyl_value, dif, padcut_val, fp ) 
*
* Parameters :
*   l : lens pointer of cutting lens.
*   res_cyl : front surface radius value.
*   cyl_value : amount of cyl requested.
*   dif : difference between front radius values.
*   axis : axis requested.
*   padcut_val : value used to cut polish pad.
*   fp : file pointer.
*
* Purpose : print out cylinder line for various lens types.
*
* Globals : min_front_cyl
*
* Returns : n.a. 
* Date : Mon Jul 17, 1989; 08:10 am
*/
print_cyl(l, res_cyl, cyl_value, dif, axis, padcut_val, fp)
LENS *l;
double	res_cyl, cyl_value, dif, axis, padcut_val;
FILE *fp;
{
	register int	i;
	extern double	fabs(double);

	if ( axis > 360.0 )
		sprintf(buffer, "%7.3f=%7.3f Cyl ", res_cyl, cyl_value);
	else
		sprintf(buffer, "%7.3f=%7.3f x%5.1f Cyl ", res_cyl, cyl_value,
		     axis);

	outline(buffer, fp);

	if ( PRINTING(fp) ) {
		bold(ON, fp);
		underline(ON, fp);
	} else {
		i = (fabs(dif) < min_front_cyl);
		if ( i ) 
			blink(ON, fp);
	}

	sprintf(buffer, "%7.3f = Dif", dif);
	outline(buffer, fp);

	if ( PRINTING(fp) ) {
		underline(OFF, fp);
		bold(OFF, fp);
	} else {
		if ( i ) 
			blink(OFF, fp);
	}

	sprintf(buffer, " %7.3f", padcut_val );
	outline(buffer, fp);
	nextline(fp);

/* Print Rough cut information */
  if ( PRINTING(fp) ) {
    bold(ON,fp);blink(ON,fp);
  }
  sprintf(buffer, "-Rough Cut >>> Front=%6.3f, CT=%5.3f <<<",
    (l->front_radius < res_cyl ? l->front_radius : res_cyl ) + 0.20 ,
    l->center_thick + 0.20 );
  outline(buffer, fp);
  if ( PRINTING(fp) ) {
    bold(OFF,fp);blink(OFF,fp);
  }
  nextline(fp);
}

/*
* l t _ b a s e  ( tp ) 
*
* Parameters :
*   tp : value of lens->lens_type flag.
*
* Purpose : pass back string that identifies type of base surface.
*
* Globals : n.a. 
*
* Returns : static string.
* Date : Wed Aug 23, 1989; 08:15 am
*/
char *
lt_base(tp)
int tp;
{
	static char type_buf[100];

	type_buf[0] = '\0';

	if ( tp & LT_PLATEAU )
		strcat(type_buf,"Plateau ");
	if ( tp & LT_DECARLE )
		strcat(type_buf,"Decarle (centered) ");
	if ( tp & LT_DDECARLE )
		strcat(type_buf,"Decentered Decarle ");
	if ( tp & LT_DECENTER )
		strcat(type_buf,"Decentered OZ ");
	if ( tp & LT_ASPHERIC )
		strcat(type_buf,"Aspheric P.C. ");
	if ( tp & LT_TOR_SEC )
		strcat(type_buf,"Toric Secondary ");
	if ( tp & LT_TOR_OOZ )
		strcat(type_buf,"Toric Base, Oval ");
	if ( tp & LT_TOR_ROZ )
		strcat(type_buf,"Toric Base, Round ");
	if ( tp & LT_TRI_DD )
		strcat(type_buf,"TriFocal (Decentered Decarle) ");
	if ( strlen(type_buf) == 0 )
		strcat(type_buf,"Spherical Base ");

quit_lt_base:
	return type_buf;
}

/*
* l t _ f r o n t  ( tp, tor, extra ) 
*
* Parameters :
*   tp :  l->lens_type flag
*   tor : l->t.tor_type
*   extra : l->extra_flags
*
* Purpose : pass back string that identifies type of base surface.
*
* Globals : n.a. 
*
* Returns : static string.
* Date : Wed Aug 23, 1989; 08:37 am
*/
char *
lt_front(l)
LENS *l;
{
	static char buf[100];

	buf[0] = '\0';

	if ( l->lens_type == 0 ) /* regular lens */
	{
		if ( l->extra_flags & EXRA_PRISM )
			strcat(buf,"Prism + ");
		strcat(buf,"Spherical Front ");
		goto quit_lt_front;
	}
	if ( l->lens_type & LT_LENTIC )
	{
		if ( l->extra_flags & EXRA_PRISM )
			strcat(buf,"Prism + ");
		strcat(buf,"Lenticular ");
		goto quit_lt_front;
	}
	if ( l->lens_type & LT_TORIC_FRONT )
	{
		if ( l->extra_flags & EXRA_PRISM )
			strcat(buf,"Prism + ");
		strcat(buf,"Toric Front ");
		goto quit_lt_front;
	}
	if ( l->lens_type & LT_TORIC_FRONT )
	{
		if ( l->extra_flags & EXRA_PRISM )
			strcat(buf,"Prism + ");
		strcat(buf,"Toric Front ");
		goto quit_lt_front;
	}
	if ( l->lens_type & LT_OP_CRES )
	{
		strcat(buf,"One Piece Cresent Bifocal ");
		goto quit_lt_front;
	}
	if ( l->lens_type & (LT_TOR_OOZ|LT_TOR_ROZ) )
	{
		if ( l->t.tor_type & (TOR_CYL_F1|TOR_CYL_F2|TOR_CYL_F3|TOR_CYL_F4) )
		{ 
			strcat(buf,"Toric Front");
			if ((l->t.tor_type & (TOR_CYL_F2|TOR_CYL_F4) ) && (l->bal_prism > 0.001))
				strcat(buf," /with Prism");
		} else
			strcat(buf,"Spherical Front");
	}
	if ( strlen(buf) == 0 )
	{
		if ( l->extra_flags & EXRA_PRISM )
			strcat(buf,"Prism + ");
		strcat(buf,"Spherical Front ");
	}

quit_lt_front:
	return buf;
}
