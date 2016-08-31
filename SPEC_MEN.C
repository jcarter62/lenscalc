/* @(#) spec_menu.c 1.15@(#) 8/29/90 09:22:16 */
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

/* -- From ---> bif.c 5.6 11/29/88 */

#define next(string) {move(cur_row++,0);clrtoeol();printw("%s",string);}

#define LINES 6
static char	*menu_ary[LINES] = { 
	" Special Lens Menu.",
	"1 - Decentered O.Z.",
	"2 - Aspheric Lens.",
	"3 - Graph Lens.",
	"q - main menu", "\0" };


extern int	cur_row;

/*
* s p e c i a l _ m e n u  ( l ) 
*
* Parameters :
*   l : lens pointer.
*
* Purpose : Display special menu.  Allow user to pick one of the
*           menu items, and input some data for menu item selected.
*
* Globals : menu_ary[static], cur_row.
*
* Returns : n.a. 
* Date : Fri Apr 21, 1989; 08:47 am
*/
special_menu(l)
LENS *l;
{
	extern double	oz_lift(LENS *);
	extern double	outer_oz(LENS *);
	extern double	fabs(double);
	extern double	get_dbl(char *, double *);
	double	tmp;
	int	not_done = 1;
	int	c, i;

	clear();
	while ( not_done ) {
		i = menu(menu_ary);
		cur_row = LINES + 1;

		switch ( i ) {
		case 1 :
			if ( !(l->lens_type & LT_DECENTER))
				l->lens_type |= LT_DECENTER;

			next("");
			l->oz = l->diameter[l->rings-1];
			l->decenter = get_dbl("Decenter amount (mm)", &(l->decenter));
			l->seght = (l->diameter[0] / 2.0) -  ( l->oz / 2.0 -
			    l->decenter );
			l->trunc = 0.0;
			l->bal_prism = fabs(l->bal_prism);
			l->bal_prism = get_dbl("Ballast Prism", &(l->bal_prism));
			if ( l->bal_prism == 0.0 ) 
				goto no_prism;
			if ( l->bal_prism >= 0.64 ) /* 4 prism */
				l->bal_prism = l->bal_prism * 0.16;
			l->bal_prism = l->bal_prism * (-1.0);
			l->blk_prism = 0.0;
			/* add prism edge thickness difference to min et .12 */

			dbl_tmp = 0.12;

			dbl_tmp =  get_dbl("Minimum Edge Thickness", &(dbl_tmp));
			dbl_tmp += ((l->diameter[0] *  (l->bal_prism / (-1.0
			    *0.16))) / (2.0 * 49.0));
			dbl_tmp =  get_dbl("Edge Thickness + Ballast prism edge",
			     				&(dbl_tmp));

			if ( dbl_tmp > l->edge_thick )
				l->edge_thick = dbl_tmp;
			l->pref = ET_PREF;
			/* 
			* recalculate minimum et in case bal+min et is not
			* used for the reason above where the new bal+min
			* edge thickness is less than the current lens et.
			*/
			l->min_et = l->edge_thick -  ( l->diameter[0] * ((-l->bal_prism)
			    / 0.16) / (2.0 * 49));

			l->extra_flags |= EXRA_PRISM;
no_prism:
			l->dist_rad = l->radius[l->rings-1];
			l->near_rad = l->radius[l->rings-2];

			not_done = 0;
			break;
		case 2 :
			if ( l->rings > 2 ) {
				next("Error Aspheric lens Requires");
				next(" Base/Peripheral only (2 rings)");
				get_str(" <Press Return to continue>", "   ");
				return;
			}
			do {
				l->edge_lift = get_dbl("Desired Edge Lift",
				     					&(l->edge_lift));
				/*
				* Determine the proper o.z. to give the desired 
				* edge lift.
				*/
				status(WORKING, A_BLINK);
				tmp = oz_lift(l);
				status(NOT_WORKING, 0);

				if ( tmp <= 0.5 ) {
					next("Problem; Specified Edge lift produced");
					next("an o.z. of <= .5 mm. Please try again");
					refresh();
				}
			} while ( tmp <= 0.5 );

			l->diameter[1] = tmp;

			l->oz = get_dbl("Desired O.Z.", &(l->oz));
			l->a_rad = 20.0;
			l->a_rad = get_dbl("Radius of Tan. Perif.(mm)", &(l->a_rad));

			status(WORKING, A_BLINK);
			l->a_angle = asin((l->oz / 2.0) / l->radius[1]);
			l->a_offset = (l->a_rad - l->radius[1]) * sin(l->a_angle);
			l->ld_dif = (l->a_rad - l->radius[1]) * cos(l->a_angle);

			/*
			* Determine The outer o.z. based on the desired 
			* o.z. and the tangent periferal.
			*/
			l->a_outer_oz = outer_oz(l);

			 {
				double	tmp1, tmp2;
				double	big_ooz, big_ioz;

				/* edge dif with respect to base curve */
				tmp1 = sag(l->radius[1], l->a_outer_oz) -
				    sag(l->radius[1], l->oz);

				/* edge diff with respect to a_rad */
				big_ooz = (l->a_offset + (l->a_outer_oz /
				    2.0)) * 2.0;
				big_ioz = l->oz + (l->a_offset * 2.0);

				tmp2 = sag(l->a_rad, big_ooz) -  sag(l->a_rad,
				     big_ioz);

				l->a_el_tan = tmp1 - tmp2;

			}

			l->lens_type |= LT_ASPHERIC;
			status(NOT_WORKING, 0);
			not_done = 0;
			break;
		case 3 :
			graph_lens(l);
		case 4 : /* quit */
			not_done = 0;
			break;
		default :
			printf("Error");
			getstr(buffer);
			break;
		}
	}
	clear();
	refresh();
}



/*
* o z _ l i f t  ( l ) 
*
* Parameters :
*   l : lens data.
*
* Purpose : Determine the proper o.z. to result 
*  in an edge lift given (l->edge_lift).
*
* Globals : n.a. 
*
* Returns : oz, optic zone required to give l->edge_lift.
* Date : Wed Jan  4, 1989; 09:18 am
*/
static double	
oz_lift(l)
LENS *l;
{
	double	sag_base, sag_perif;
	double	base, oz, perif, diam;
	double	e_lift;
	int	done = 0;	/* not done */

	base = l->radius[1];	/* base curve */
	perif = l->radius[0];	/* perif curve */
	diam = l->diameter[0];	/* lens size */
	oz = l->diameter[1];	/* guess at correct oz */

	while ( !done ) {
		sag_base = sag(base, diam) - sag(base, oz);
		sag_perif = sag(perif, diam) - sag(perif, oz);
		e_lift = sag_base - sag_perif;

		if ( fabs(e_lift - l->edge_lift) < 0.001 )
			done = 1;	/* finished */
		else if ( e_lift > l->edge_lift )
			oz += fabs(e_lift - l->edge_lift);
		else
			oz -= fabs(e_lift - l->edge_lift);

		if ( oz <= 0.5 )	/* we have a problem */
			done = 1;
	}
	return oz;
}



/*
* o u t e r _ o z  ( l ) 
*
* Parameters :
*   l : lens data.
*
* Purpose : determine outer o.z. based on central oz (l->oz) given.
*  Calculated by changing outer o.z. until back surface sag at 
*  base/perif is == back surface sag at base/(aspheric perif).
*
* Globals : n.a. 
*
* Returns : ooz, outer o.z. 
* Date : Wed Jan  4, 1989; 08:56 am
*/
static double	
outer_oz(l)
LENS *l;
{
	int	done = 0;
	double	ooz = l->diameter[0];
	double	asph_sag, sph_sag, dif;

	while ( !done ) {
		asph_sag = sag(l->radius[1], l->oz) +  (sag(l->a_rad, ooz
		    + (2.0 * l->a_offset)) -  sag(l->a_rad, l->oz + (2.0 * l->a_offset)));
		sph_sag  = sag(l->radius[1], l->diameter[1]) +  (sag(l->radius[0],
		     ooz) -  sag(l->radius[0], l->diameter[1]));

		dif = fabs(asph_sag - sph_sag);
		if ( dif < 0.0001 )
			done = 1;
		else if ( asph_sag > sph_sag )
			ooz -= dif;
		else
			ooz += dif;
	}
	return ooz;
}


