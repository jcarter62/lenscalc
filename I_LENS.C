/* @(#) i_lens.c 5.19@(#) 8/29/90 09:21:51 */
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

/* List of procedures/functions i_lens.c
* init_lens( l ) [global]
* wipe_lens( l ) [global void]
* input_lens( l ) [global void]
* parse_rad( s, r, w ) [static ]
* round1_10( d ) [static double]
*/

char *hlp[6]= {
/* 0 */ "MM value: '.'=back-up, 'cr'=accept, or number #[.#]",
/* 1 */ "MM or Diopter value: '.' to back-up, 'cr'=accept, or number #[.[#][.W]] W=width",
/* 2 */ "Power in Diopters. 'cr'=accept, or number [+|-]#[.#]",
/* 3 */ "Center Thickness in MM. 'cr'=accept, - number=E.T., or number .# ",
/* 4 */ "Edge Thickness in MM. 'cr'=accept, - number=C.T., or number .# ",
/* 5 */ "",
};

/*
* i n i t _ l e n s  ( l ) [global]
*
* Parameters :
*   l : lens pointer.
*
* Purpose : initialize lens data to ground zero state.
*
* Globals : n.a. 
*
* Returns : modified lens data.
* Date : Mon Apr 17, 1989; 03:30 pm 
*/
init_lens(l)
register LENS *l;
{
  register int i;

  l->rings        = 0;
  l->power        = 0.0;
  l->center_thick = 0.0;
  l->edge_thick   = 0.0;
  l->min_et       = 0.1;
  l->front_radius = 0.0;
  l->index        = 1.485;
  l->pref         = CT_PREF;
  l->lens_type    = LT_REG;
  l->extra_flags  = EXRA_NONE;
  /* decarle */
  l->trial_pow    = 0.0;
  l->trial_rad    = 0.0;
  l->add_pow      = 2.0;
  l->near_rad     = 0.0;
  l->near_pow     = 0.0;
  l->dist_rad     = 0.0;
  l->dist_pow     = 0.0;
  l->oz           = 4.0;  /* mm */
  l->seght        = 0.0;
  l->trunc        = 0.0;
  l->bal_prism    = 0.32; /* 2 prism */
  l->blk_prism    = 0.0;  /* mm */

  /* lenticular parameters */
  l->lt_oz        = 7.6;
  l->lt_et        = .1;
  l->lt_jt        = -1000.0;

  l->t.tor_type   = 0;
  l->t.pow_flat   =
  l->t.pow_steep  = 0.0;

  l->t.cyl        = 
  l->t.res_cyl    = 
  l->t.axis       = 0.0;

  l->t.steep_radius= 
  l->t.flat_radius = 
  l->t.oz_flat     =
  l->t.oz_steep    =
  l->t.jt_flat     =
  l->t.jt_steep    =
  l->t.et_flat     =
  l->t.et_steep    = 0.0;

  l->mat_type[0] = '\0';

  for (i=0;i<MAX_RINGS;i++)  {
    l->diameter[i]     =
    l->radius[i]       =
    l->rad_dif[i]      =
    l->sag_dif[i]      =
    l->sag_plus_rad[i] =
    l->sag_sum[i]      = 0.0;
  }
}

/*
* w i p e _ l e n s  ( l ) [global void]
*
* Parameters :
*   l : lens pointer.
*
* Purpose : wipe lens data, usually after lens has been input
*           and user presses new lens function (8) from main menu.
*
* Globals : n.a. 
*
* Returns : lens pointer (l) modified.
* Date : Mon Apr 17, 1989; 03:31 pm
*/
void
wipe_lens(l)
LENS *l;
{
  int i;

  for (i=0;i<MAX_RINGS;i++ )
    l->rad_dif[i] =
    l->sag_dif[i] =
    l->sag_plus_rad[i] =
    l->sag_sum[i] = 0.0;
  l->front_radius = 0.0;

  if ( l->lens_type & LT_TOR_ROZ ) {
    l->t.steep_radius = -(l->t.steep_radius);
    l->rings = l->round.rings;
    for (i=0;i<l->rings;i++) {
      l->diameter[i] = l->round.diam[i];
      l->radius[i]   = l->round.rad[i];
    }
    l->power = l->t.pow_flat;
  }

  if ( l->lens_type & LT_TOR_OOZ ) {
    l->t.steep_radius = -(l->t.steep_radius);
    l->radius[l->rings-1] = l->t.flat_radius;
    l->power = l->t.pow_flat;
  }

  if ( l->lens_type & LT_DDECARLE ) {
    l->power = l->trial_pow;
  }

  if ( l->lens_type & LT_DECARLE ) {
    l->power = l->trial_pow;
    l->rings --;
  }

	if ( l->lens_type & LT_TRI_DD ) {
		l->power = l->itrm.trial_pow;
	}
}

/*
* i n p u t _ l e n s  ( l ) [global void]
*
* Parameters :
*   l : lens pointer
*
* Purpose : prompt user for lens data, rings/radius/diameter/power/ct,et.
*
* Globals : n.a. 
*
* Returns : modified lens pointer (l).
* Date : Fri Dec 22, 1989; 07:25 am
*/
void
input_lens(l)
LENS *l;
{
  int ct_et_pref;
  int i;
  extern int BLINK_ON;
  extern int get_int(char *, int );
  extern double fabs(double );
  extern double round(double);

  BLINK_ON = 0;  /* turn blinking off for now. */

  do {
    sprintf(help_buf,"Number of rings in range 1 to %d",MAX_RINGS);
    help_msg(help_buf);
    l->rings = get_int("Rings ",l->rings);
  } while ( l->rings <= 0 );

  ct_et_pref = l->pref;
  l->pref = 0;

  display_lens(l);
  for (i=0;i<l->rings;i++) 
  {
    double width;

in_diam:
    help_msg(hlp[0]);
    move(5+i,0);
    standout();printw("%7.3f",l->diameter[i]);standend();
    sprintf(buffer,"Diameter[%d] ",i+1);
    sprintf(buffer1,"%7.3lf",l->diameter[i]);
    get_string(buffer,buffer1);
    if ( strcmp(buffer1,".") == 0 ) { 
      if ( i > 0 ) {
        i--;
        display_lens(l);
      } else {
        beep();
        goto in_diam;
      }
    } else {
      l->diameter[i] = atof(buffer1);
      display_lens(l);
    }

    help_msg(hlp[1]);
    move(5+i,8);
    standout();printw("%7.3f",l->radius[i]);standend();
    sprintf(buffer,"Radius[%d] ",i+1);
    sprintf(buffer1,"%7.3lf",l->radius[i]);
    get_string(buffer,buffer1);
    if ( strcmp(buffer1,".") == 0 ) { 
      display_lens(l);
      goto in_diam; 
    }
    parse_rad(buffer1,&(l->radius[i]),&width);
    if ( width > 0.0 )
      l->diameter[i+1] = l->diameter[i] - (2.0 * width);
    if ( l->radius[i] >= MAX_MM ) {
       l->radius[i] = (double)diopt_to_mm(TEAR_INDEX,l->radius[i]);
       l->radius[i] = round(l->radius[i]);
    }

    if ( i < (l->rings-1) ) {
      extern int NOROUND;
      extern double round1_10(double);
      double tmp;

      tmp = round1_10(l->radius[i]);
      if ( !NOROUND && (tmp != l->radius[i]) ) {
        move(2,0);
        beep();
        printw("Can I use %7.3f instead of %7.3f (y/n/enter=y)",
          tmp,l->radius[i]);
        refresh();
        getstr(buffer);
        move(2,0);clrtoeol();
        if (strlen(buffer) == 0 || buffer[0] == 'y' || buffer[0] == 'Y' )
          l->radius[i] = tmp;
      }
    }

    display_lens(l);
  }

  help_msg(hlp[2]);
  move(6+l->rings,24);
  standout();printw("%7.3f",l->power);standend();
  l->power = get_float("power : ",&(l->power));
  display_lens(l);

  if ( ct_et_pref & CT_PREF ) 
  {
    help_msg(hlp[3]);
    move(6+l->rings,8);standout();
    printw("%7.3f",l->center_thick);standend();
    l->center_thick = get_float("Center Thickness : ",&(l->center_thick));
    if ( l->center_thick < 0.0 ) {
      l->center_thick = fabs(l->center_thick);
      l->edge_thick   = l->center_thick;
      ct_et_pref= ET_PREF;
    }
  } 
  else 
  {
    help_msg(hlp[4]);
    move(6+l->rings,16);standout();
    printw("%7.3f",l->edge_thick);standend();
    l->edge_thick = get_float("Edge Thickness : ", &(l->edge_thick));
    if ( l->edge_thick < 0.0 ) {
      l->edge_thick   = fabs(l->edge_thick);
      l->center_thick = l->edge_thick;
      ct_et_pref= CT_PREF;
    }
  }

  help_msg(hlp[5]);
  l->pref = ct_et_pref;
  lenscalc(l);

  BLINK_ON = 1;  /* turn blinking on */
  display_lens(l);
}

/*
* p a r s e _ r a d  ( s, r, w ) [static ]
*
* Parameters :
*   s : input string of possible radius with width.
*   r : return radius value.
*   w : return width value.
*
* Purpose : parse radius input value.  In the form of :
*           [radius] followed by [width]
*           legal forms [#.[#]][.#]
*           i.e. (2) (2..2) (.2) (2.2) 
*
* Globals : n.a. 
*
* Returns : s, not changed.  r & w possibly modified.
* Date : Mon Apr 17, 1989; 03:33 pm
*/
static
parse_rad(s,r,w)
char *s;
double *r,*w;
{
  char *p;
  int np=0;
  int ns=0;

  p = s;
  while ( *p ) {
    if ( *p == '/' )
      ns++;
    else
    if ( *p == '.' )
      np++;
    p++;
  }
  if ( ns ) {
    if ( np == 2 )                 /* diam[.fract]/width */
      sscanf(s,"%lf/%lf",r,w);
    else
    if ( *s == '/' )               /* /width */
      sscanf(s,"/%lf",w);
    else {                         /* diam[.fract]/ */
      *w = 0.0;
      *r = atof(s);
    }
  } else {
    if ( np == 2 )                 /* diam.fract.width */
      sscanf(s,"%lf%lf",r,w);
    else
    if ( *s == '.' )               /* .width */
      *w = atof(s);
    else {                         /* diam[.fract] */
      *w = 0.0;
      *r = atof(s);
    }
  }
}

/*
* r o u n d 1 _ 1 0  ( d ) [static double]
*
* Parameters :
*   d : input double value.
*
* Purpose : round input value (d) to 1 position after decimal.
*           1.123 ==> 1.1.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 17, 1989; 03:37 pm
*/
static double
round1_10(d)
double d;
{
  double frac,x;

  x = floor(d*(double)10.0);
  frac = (d*(double)10.0 - x);
  if ( frac > (double)0.4999 )
    x = x + (double)1.0;
  x = x/(double)10.0;
  return x;
}

