/* @(#) bif.c 5.18@(#) 10/19/90 09:13:23 */
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

#define next(string) {move(cur_row++,0);clrtoeol();printw("%s",string);}

#define LINES 8
static char *menu_ary[LINES] = { " Bifocal Menu.",
		"1 - One Peice Target Decarle",
		"2 - One Peice Decentered Decarle",
		"3 - One Peice Crescent Prism Ballast",
		"4 - Double Fused (PMMA only)",
		"5 - Tri-Focal (Decentered Decarle)",
		"q - main menu", "\0" };

extern int cur_row;

/*
* b i f o c a l _ m e n u  ( l ) 
*
* Parameters :
*   l : pointer to LENS record(structure).
*
* Purpose : 
*   Display bifocal menu and allow user to select from menu.
*   Have user input selected data for each valid menu entry picked.
*   Also do simple calculations such as prism/et calculation.
*
* Globals : cur_row (screen current row position)
*
* Returns : n.a. 
* Date : Thu Jun 15, 1989; 03:51 pm
*/
int	/* dosn't really return anything. */
bifocal_menu(l)
LENS *l;
{
	extern double get_dbl(char *,double *);
	int not_done = 1;
	int c,i;

	clear();
	while ( not_done ) {
		i = menu(menu_ary);
        	cur_row = LINES + 1;

		switch( i ) {
		case 1 :                              /* 1 - One Peice Target Decarle */
			if ( l->lens_type & LT_DDECARLE ) {
				next("");
				next("Error: Can't have both Centered & Decentered");
				next("This lens will be a simple decarle bifocal.");
				l->lens_type &= ~LT_DDECARLE;
				l->lens_type |= LT_DECARLE; 
				next("Press Return to continue");
				getstr(buffer);
			}
			if ( l->lens_type & LT_DECARLE ) {
				l->rings--;
			} else {
				l->lens_type |= LT_DECARLE;
				l->trial_pow = l->power;
				l->trial_rad = l->radius[l->rings-1];
			}
			next("");
			l->add_pow = get_dbl("Add Power",&(l->add_pow) );
			simple_decarle(l);
			l->rings++;
			l->radius[l->rings -1] = l->dist_rad;
			l->power = l->dist_pow;

			l->diameter[l->rings -1] = 
			get_dbl("Distance O.Z. : ",&(l->diameter[l->rings -1]));
			noecho();
			not_done = 0;
      break;
		case 2 :                           /* 2 - One Peice Decentered Decarle */
			if ( l->lens_type & LT_DECARLE ) {
				next("");
				next("Error: Can't have both Centered & Decentered");
				next("This lens will be a decentered decarle bifocal.");
				l->lens_type &= ~LT_DECARLE;
				l->lens_type |= LT_DDECARLE; 
				next("Press Return to continue");
				getstr(buffer);
			}
			if ( l->lens_type & LT_DDECARLE ) {
				/* do nothing */
			} else {
				l->lens_type |= LT_DDECARLE;
				l->trial_pow = l->power;
				l->trial_rad = l->radius[l->rings-1];
			}
			next("");
			l->add_pow = get_dbl("Add Power",&(l->add_pow) );
			simple_decarle(l);
			l->power = l->near_pow;
			l->oz = get_dbl("Distance O.Z.",&(l->oz));
			l->seght = get_dbl("Seg Height",&(l->seght));
			l->trunc = get_dbl("Truncation",&(l->trunc));
			l->bal_prism = get_dbl("Ballast Prism",&(l->bal_prism));
			if ( l->bal_prism >= 0.64 ) /* 4 prism */
				l->bal_prism = l->bal_prism * 0.16;
			l->blk_prism = get_dbl("Blocking Prism",
				&(l->blk_prism));
			/* add prism edge thickness difference to min et .10 */
			
			l->pref = ET_PREF;
			l->min_et = 0.12;
			l->min_et = 
				get_dbl("Minimum Edge Thickness",
				&(l->min_et));
			l->edge_thick = l->min_et + ((l->diameter[0] * 
				(l->bal_prism/0.16))/(2.0*49.0));
			l->edge_thick = 
				get_dbl("Edge Thickness + Ballast prism edge",
				&(l->edge_thick));

			/* rotation of oz */
			l->rotang = get_dbl("Rotate oz degrees",&(l->rotang));
			if ( l->rotang != 0.0 ) {
				next("(R)ight or (L)eft lens ?");
				refresh();
				do {
					c = getch();
					if ( islower(c) ) c = toupper(c);
				} while ( c!='R' && c!='L' );
				l->rot_rl = c;
				next("Rotate (I)n or (O)ut");
				refresh();
				do {
					c = getch();
					if ( islower(c) ) c = toupper(c);
				} while ( c!='I' && c!='O' );
				l->rot_io = c;
			} /* end if rotang != 0 */
			
			not_done = 0;
      break;
		case 3 :                         /* 3 - One Peice Crescent Prism Ballast */
			next("");
			l->add_pow = get_dbl("Add Power",&(l->add_pow) );
			l->lens_type = LT_OP_CRES;
			not_done = 0;
			break;
		case 4 : /* quit */
			not_done = 0;
			break;
		case 5 : /* 5 - Tri-focal decentered decarle */
			if ( l->lens_type & (LT_DECARLE|LT_DDECARLE) ) {
				/* can't be centered decarle or decentered decarle and also */
				/* decentered tri-focal. */
				l->lens_type &= ~(LT_DECARLE|LT_DDECARLE);
				l->lens_type |= LT_TRI_DD;
			}
			if ( l->lens_type & LT_TRI_DD ) {
				/* do nothing */
			} else {
				l->lens_type |= LT_TRI_DD;
				l->itrm.trial_pow = l->power;
				l->itrm.trial_rad = l->radius[l->rings-1];
			}

			/*
			* Input Intermediate specific information.
			*/
			next("");
			l->itrm.add_pow = get_dbl("Intermediate Add Power",&(l->itrm.add_pow) );
			l->itrm.dist_oz = get_dbl("O.Z.                  ",&(l->itrm.dist_oz) );
			l->itrm.i_seght = get_dbl("Seg Height            ",&(l->itrm.i_seght) );
			next("");
			/*
			* Input Distance specific information.
			*/
			l->dist.add_pow = get_dbl("Distance Add Power    ",&(l->dist.add_pow) );
			l->dist.dist_oz = get_dbl("O.Z.                  ",&(l->dist.dist_oz) );
			l->dist.i_seght = get_dbl("Seg Height            ",&(l->dist.i_seght) );
			if ( l->dist.i_seght < l->itrm.i_seght ) {
				l->dist.i_seght = l->dist.i_seght + l->itrm.i_seght ;
				sprintf(buffer," (%8.2lf) ",l->dist.i_seght );
				next(buffer);
			}
			next("");

			/*
			* Input intermediate information, that pertains to both intermediate and
			* distance power curves.
			*/
			l->itrm.truncate= get_dbl("Truncation            ",&(l->itrm.truncate));
			l->itrm.ballast = get_dbl("Ballast Prism         ",&(l->itrm.ballast));
			if ( l->itrm.ballast >= 0.64 ) /* 4 prism */
				l->itrm.ballast = l->itrm.ballast * 0.16;

			/*
			* Edge thickness pertaining to prism/min-edge.
			*/
			l->pref = ET_PREF;
			l->min_et = 0.12;
			l->min_et = get_dbl("Minimum Edge Thickness", &(l->min_et));
			l->edge_thick = l->min_et + 
				((l->diameter[0] * (l->itrm.ballast/0.16))/(2.0*49.0));
			l->edge_thick = 
				get_dbl("Edge Thickness + Ballast prism edge", &(l->edge_thick));

			l->itrm.rotation = ROT_NONE;
			l->dist.rotation = ROT_NONE;

			l->itrm.near_oz  = l->diameter[0];

			l->dist.near_oz  = l->diameter[0]; /* l->itrm.dist_oz; */
			l->dist.truncate = l->itrm.truncate;
			l->dist.ballast  = 0.0;

			not_done = 0;
      break;
			/* end 5-trifocal */
		case 6 : /* quit */
			not_done = 0;
			break;
		default  :
			printf("Error");
			getstr(buffer);
			break;
		}
	}	
	clear();
	refresh();
}
