/* @(#) graph.c 1.4@(#) 7/27/89 16:16:11 */
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
#ifndef dos

#include "gp.h"

#define LINES 5
static char	*menu_ary[LINES] = { 
	" Graph Lens Menu.",
	"1 - Base View",
	"2 - Side View",
	"q - main menu", "\0" };


/* graphics specific definitions */

typedef struct {
	double	xmin, ymin;
	double	xmax, ymax;
} PORT;

static double	cur_row;

#define Home() { cur_row = tw.ymax - 1.0; }

#define next_line(str) { \
	cur_row -= 1.0; \
	MoveTo(0.0,cur_row);\
	DrawText(str);\
}

static baseview(l)
LENS *l;
{
	PORT port1, port2, port3, port4;
	PORT tp, gp, tw;
	double	tmp;
	double	a, b, difs, difr;
	int	i;
	int	c;

	/* graphics port Left part of screen */
	gp.xmin = 1.0; 
	gp.ymin = 1.0;
	gp.xmax = 778.0; 
	gp.ymax = 778.0;
	/* Text port Right part of screen */
	tp.xmin = 800.0; 
	tp.ymin = 1.0;
	tp.xmax = 1020.0; 
	tp.ymax = 778.0;
	/* Text window coordinates */
	tw.xmin = -2.0; 
	tw.ymin = -2.0;
	tw.xmax = 80.0; 
	tw.ymax = 20.0;

	graph_mode();

	ClearScreen();
	/* first port Looking at Base curve */
	SetViewport(gp.xmin, gp.ymin, gp.xmax, gp.ymax);
	/* SetViewport(port1.xmin,port1.ymin,port1.xmax,port1.ymax); */

	tmp = l->diameter[0] / 2.0;
	SetWindow(-tmp, -tmp, tmp, tmp);
	Border();
	for ( i = 0; i < l->rings; i++) {
		tmp = l->diameter[i] / 2.0;
		circle(tmp, 0.0, 0.0);
	}
	ScreenUpdate();

	SetViewport(tp.xmin, tp.ymin, tp.xmax, tp.ymax);
	SetWindow(tw.xmin, tw.ymin, tw.xmax, tw.ymax);
	Home();
	next_line("Base View");
	next_line(" -------");
	next_line("Dimensions : ");
	for (i = 0; i < l->rings; i++) {
		sprintf(buffer, "%7.2lf", l->diameter[i]);
		next_line(buffer);
	}
	next_line(" -------");
	next_line("Press <cr>");
	next_line("- to continue");
	ScreenUpdate();

	getcrnl();
	text_mode();
}


static sideview(l)
LENS *l;
{
	PORT port1, port2, port3, port4;
	PORT tp, gp, tw;
	double	tmp;
	double	a, b, difs, difr;
	int	i;
	int	c;

	/* graphics port Left part of screen */
	gp.xmin = 1.0; 
	gp.ymin = 1.0;
	gp.xmax = 778.0; 
	gp.ymax = 778.0;
	/* Text port Right part of screen */
	tp.xmin = 800.0; 
	tp.ymin = 1.0;
	tp.xmax = 1020.0; 
	tp.ymax = 778.0;
	/* Text window coordinates */
	tw.xmin = -2.0; 
	tw.ymin = -2.0;
	tw.xmax = 80.0; 
	tw.ymax = 20.0;

	graph_mode();
	/* second port, looking at side view */
	 {
		double	tenpc;
		tmp = l->diameter[0] / 2.0;
		tenpc = tmp * 0.10;

		SetWindow(tmp - tenpc, 0.0, tmp * 2.0 + tenpc, tmp + tenpc);
	}

	SetViewport(gp.xmin, gp.ymin, gp.xmax, gp.ymax);
	ClearScreen();
	Border();

	a = 0.0;
	b = 0.0;
	l->diameter[l->rings] = 0.0;

	for ( i = l->rings - 1; i >= 0; i--) {
		arc(l->radius[i], l->diameter[i+1] / 2.0, l->diameter[i]
		    / 2.0, a, b);
		LineTo(a, b);

		if ( i > 0 ) {
			difs = sag(l->radius[i], l->diameter[i]) -  sag(l->radius[i-1],
			     l->diameter[i]);
			difr = l->radius[i-1] - l->radius[i];
			a = a - difr - difs;
		}
	}
	a = 0.0;
	a = a - ( l->front_radius - l->radius[l->rings-1] ) +  l->center_thick;
	if ( l->lens_type & LT_LENTIC ) {
		double	sag_dif;

		arc(l->front_radius, 0.0, l->lt_oz / 2.0, a, b);
		LineTo(l->radius[l->rings-1], l->diameter[0] / 2.0);
		DrawText("(Junct) ");
		sag_dif = sag(l->front_radius, l->lt_oz) -  sag(l->lt_rad,
		     l->lt_oz);
		a = a - (l->lt_rad - l->front_radius) - sag_dif;
		/*
	  a = a - ( l->lt_rad - l->front_radius ) +
	     ( l->edge_thick - l->lt_et );
	  */
		arc(l->lt_rad, l->lt_oz / 2.0, l->diameter[0] / 2.0, a, b);
	} else
		arc(l->front_radius, 0.0, l->diameter[0] / 2.0, a, b);
	if ( l->lens_type & LT_ASPHERIC )
		asp_side(l);
	ScreenUpdate();

	SetViewport(tp.xmin, tp.ymin, tp.xmax, tp.ymax);
	SetWindow(tw.xmin, tw.ymin, tw.xmax, tw.ymax);
	Home();
	next_line("Side View");
	next_line(" -------");
	next_line("Dimensions : ");
	for (i = 0; i < l->rings; i++) {
		sprintf(buffer, "Diam %7.2lf", l->diameter[i]);
		next_line(buffer);
	}
	sprintf(buffer, "C.T. %4.2lf", l->center_thick);
	next_line(buffer);
	if ( l->lens_type & LT_LENTIC ) {
		sprintf(buffer, "E.T. %4.2lf", l->lt_et);
		next_line(buffer);
		sprintf(buffer, "J.T. %4.2lf", l->lt_jt);
		next_line(buffer);
		sprintf(buffer, "L.Rad %5.2lf", l->lt_rad);
		next_line(buffer);
	} else {
		sprintf(buffer, "E.T. %4.2lf", l->edge_thick);
		next_line(buffer);
	}
	next_line("Front Radius :");
	sprintf(buffer, "     %5.2lf", l->front_radius);
	next_line(buffer);
	next_line(" -------");
	next_line("Press <print>");
	next_line("or ^P for hardcopy");
	next_line(" -------");
	next_line("Press <cr>");
	next_line("- to continue");
	ScreenUpdate();

	 {
		int	c;

		do {
			c = getchar();
		} while ( c != '\r' && c != '\n' && c != '\' );
		if ( c == '\' )
			hardcopy();
	}

	text_mode();
}


static asp_side(l)
LENS *l;
{
	double	a, b;
	double	x, y;
	double	x1, y1;
	double	r;

	a = -l->ld_dif;
	b = -l->a_offset;
	r = l->a_rad;

	arc(r, l->oz / 2.0, l->a_outer_oz / 2.0, a, b);
	sprintf(buffer, "%4.1lfmm(%5.2lf/%5.2lf)", r, l->a_outer_oz, l->oz);

	y = l->oz / 2.0;
	x = r * cos(asin((y - b) / r)) + a;
	MoveTo(x, y);
	LineTo(l->radius[l->rings-1], y);
	x = l->radius[l->rings-1] * 1.03;	/* add 7 % */
	y = l->oz / 2.0 + (l->a_outer_oz / 2.0 - l->oz / 2.0) / 2.0;
	LineTo(x, y);
	DrawText(buffer);
	MoveTo(x, y);
	y = l->a_outer_oz / 2.0;
	x = r * cos(asin((y - b) / r)) + a;
	LineTo(l->radius[l->rings-1], y);
	LineTo(x, y);
}


graph_lens(l)
LENS *l;
{
	int	not_done = 1;

	while ( not_done ) {
		switch (menu(menu_ary)) {
		case 1 :
			baseview(l); 
			break;
		case 2 :
			sideview(l); 
			break;
		case 3 :
			not_done = 0;
			break;
		}
	}
}


static getcrnl()
{
	int	c;

	do {
		c = getchar();
	} while ( c != '\r' && c != '\n' );
}


#else
graph_lens(l)
LENS *l;
{
}


#endif
