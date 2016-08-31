/* @(#) lenscalc.c 5.22@(#) 8/29/90 09:21:08 */
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

double sag_at_i[MAX_RINGS];
extern void junct_thick(LENS *,double);

/*
* found in decarle.c 
*/
extern void decar_pow(double,DECARLE_DATA *);
extern void decar_dec(DECARLE_DATA *);

#define SMALL   .001
#define EBSILON .0005

/* List of procedures/functions lenscalc.c
* lenscalc( l ) 
* main_calc( l ) 
* figure_lentic( l ) 
* plateau( l ) 
* junct_thick( l, jt ) 
* simple_decarle( l ) 
* ddecarle( l ) 
*/

/*
* l e n s c a l c  ( l ) 
*
* Parameters :
*   l : LENS structure.
*
* Purpose : main math calculation entry point.  If the data
* indicates a regular lens, simply call main_calc() otherwise
* call the appropriate calculation function for toric/...etc.
* - Also, indicate to user the status working/notworking while
*   time consuming math calculations are being done.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Fri Mar 31, 1989; 09:30 am
*/
lenscalc(l)
register LENS *l;
{
  status(WORKING,A_BLINK);

	if ( l->lens_type & LT_TRI_DD ) {
		l->itrm.trial_rad = l->radius[l->rings-1];

		decar_pow(l->index,&(l->itrm));
		decar_dec(&(l->itrm));

		/* this next step is important. */
		l->dist.trial_pow = l->itrm.dist_pow;
		l->dist.trial_rad = l->itrm.dist_rad;
		l->dist.ballast   = 0.0;

		decar_pow(l->index,&(l->dist));
		decar_dec(&(l->dist));

		l->power = l->itrm.near_pow + l->dist.add_pow ;
		main_calc(l);
 	} 

  if ( l->lens_type & LT_TOR_OOZ )
    tor_oval(l);
  else 
		if ( l->lens_type & LT_TOR_ROZ )
    	tor_round(l);
  else 
		if ( l->lens_type & LT_TOR_SEC ) 
    	tor_secondary(l);
	else
    main_calc(l);

  status(NOT_WORKING,0);
}

/*
* m a i n _ c a l c  ( l ) 
*
* Parameters :
*   l : Lens data.
*
* Purpose : Calculate regular/lenticular lens as well as other
*  types (bifocals/plateau/...)
*
* Globals : sag_at_i[]. This array is used in lenticular subroutines.
*
* Returns : Modified lens data.(l).
* Date : Fri Apr  7, 1989; 10:44 am
*/
main_calc(l)
register LENS *l;
{
  extern void figure_lentic(LENS *);
  register int i;         /* counter */
  double sag_total;       /* total back surface sagvalue */
  double tmp_index;       /* 505 instead of 1.505 */
  double diopt_index;     /* diopt value based on base curve and index */
  double radius_est;      /* front radius estimate (tmp) */
  double diopt_front;     /* diopters of front curve */
  double tmp_edge;        /* edge thickness tmp value */
  char buf[80];

  /* 
  * re-calculate the needed data for decarle bifocals. 
  * Which is needed for most other calculations.
  * the Distance curves/powers will change if index or
  * trial power, etc. is changed.
  */
  if ( l->lens_type & (LT_DECARLE | LT_DDECARLE) ) {
    simple_decarle(l);
  if ( l->lens_type & LT_DECARLE ) {
    l->radius[l->rings-1] = l->dist_rad;
    l->power = l->dist_pow;
  } else 
    l->power = l->near_pow;
  }

  for (i=0;i<l->rings;i++) {
    if ( i ) {
      l->rad_dif[i] = l->radius[i-1] - l->radius[i];
    }
    sag_at_i[i]   = sag(l->radius[i],l->diameter[i]);
  }

  for (i=0;i<l->rings;i++) {
    if ( i ) {
      l->sag_dif[i]      = sag_at_i[i] - 
                           sag(l->radius[i-1],l->diameter[i]);
      l->sag_plus_rad[i] = l->rad_dif[i] + l->sag_dif[i];
      l->sag_sum[i]      = l->sag_sum[i-1] + l->sag_plus_rad[i];
    } else {
      l->sag_dif[i]      = sag_at_i[i];
      l->sag_plus_rad[i] =
      l->sag_sum[i]      = 0.0;
    }
  }

  sag_total       = 0.0;
  for (i=1;i<l->rings;i++)
    sag_total += l->sag_dif[i];
  tmp_index       = (l->index - 1.0) * 1000;
  diopt_index     = tmp_index / l->radius[l->rings - 1];
  diopt_front     = diopt_index + l->power;
  radius_est      = tmp_index / diopt_front;
loop:
  l->front_radius = radius_est +
    ( l->center_thick * (l->index - 1.0)/(l->index) );
  /*
  * For the degenerate case where the front radius is so small
  * a lens size required is impossible, we must just rule out
  * this case.
  */
  if ( l->front_radius * 2.0 < l->diameter[0] ) {
    l->sag_front = l->front_radius;
    l->pref = CT_PREF;
  } else
    l->sag_front       = sag(l->front_radius,l->diameter[0]);

  tmp_edge = sag_total + l->center_thick +
             sag(l->radius[0],l->diameter[0]) - l->sag_front;

  if ( l->pref & CT_PREF )
    l->edge_thick = tmp_edge;
  else
  if ( l->pref & ET_PREF ) {
    double diff;
    diff = fabs(tmp_edge - (l->edge_thick));
    if ( diff > EBSILON ) {
      if ( tmp_edge < (l->edge_thick) )
        l->center_thick += diff;
      else
        l->center_thick -= diff;
      goto loop;
    } /* ebs */
    else
      l->edge_thick = tmp_edge;
  } /* pref */


        if ( l->lens_type & LT_LENTIC ) {
                figure_lentic(l);
        }

        if ( l->rad_dif[l->rings - 1] < 0.0 ) {
           plateau(l);
           l->lens_type |= LT_PLATEAU;
        } else
           l->lens_type &= ~LT_PLATEAU;

  if ( l->lens_type & LT_OP_CRES ) {
    double re,rf,sf,ed;

    re = tmp_index / ( diopt_index + l->power + l->add_pow) ;
    rf = re + (l->center_thick*(l->index-1.0)/l->index);
    sf = sag(rf,l->diameter[0]);

    ed = sag_total + l->center_thick + 
      sag( l->radius[0],l->diameter[0]) - sf;
    l->extra3l = rf;
    l->extra4l = ed;
  }

  if ( l->lens_type & LT_TORIC_FRONT ) {
    l->t.res_cyl = (tmp_index/
                   ((tmp_index/l->front_radius)+l->t.cyl));
    l->extra3l = sag(l->front_radius,8.0)
            + (double)16.533 - l->front_radius;
  }
  if ( l->lens_type & (LT_DDECARLE|LT_DECENTER) ) 
    ddecarle(l);

  l->min_bt = l->sag_dif[0] + sag_total + l->center_thick;
}

/*
* f i g u r e _ l e n t i c  ( l ) 
*
* Parameters :
*   l : Lens data.
*
* Purpose : Calculate lenticular specific Data in (l).
*
* Globals : sag_at_i[], created in main_calc().
*
* Returns : Modified lens data.
* Date : Fri Apr  7, 1989; 10:46 am
*/
void
figure_lentic(l)
LENS *l;
{
  register int i;         /* counter */
  double a,b,c, tmp;      /* temp variables used in calculation of lenticular radius */
  double sage,bt_sag;     /* sag estimate & back surface sag value */

  /* difference between non-lentic edge and finished lentic edge */
  l->lt_tkoff = l->edge_thick - l->lt_et;
  /* Sag value difference of front radius at lens-size & lent oz */
  sage = l->sag_front - sag(l->front_radius,l->lt_oz);

  a = sage + l->lt_tkoff;
  b = l->diameter[0]/2.0; b *= b;
  c = l->lt_oz/2.0; c *= c;
  tmp = ((pow(a,2.0)*2.0*b) + pow(b,2.0) - (2.0*b*c) +
          pow(c,2.0) + (pow(a,2.0)*2.0*c) + pow(a,4.0)) /
          (pow(a,2.0)*4.0);
  l -> lt_rad = sqrt(tmp);

  /* determine back surface sag at lenticular oz */
  bt_sag = 0.0;
  i = l->rings - 1;
  while ( l->diameter[i] < l->lt_oz ) {   /* no check for i < 0 */
  	if ( i == l->rings-1 )
     	bt_sag = sag_at_i[i];
  	else
     	bt_sag += sag(l->radius[i],l->diameter[i]) -
            sag(l->radius[i],l->diameter[i+1]);
  	i--;
  }
  bt_sag += sag(l->radius[i],l->lt_oz);
  if ( i < l->rings-1 )
    bt_sag -= sag(l->radius[i],l->diameter[i+1]);

  /* determine junction thickness and set */
  l->lt_jt = bt_sag + l->center_thick - sag(l->front_radius,l->lt_oz);
  l->lt_set= l->lt_jt - bt_sag + sag(l->lt_rad,l->lt_oz);
  {
    /* determine cap angle & lower dial for chase lathe */
    double x_f_rad, x_l_rad;
    x_f_rad = sqrt(pow(l->front_radius,2.0) - pow((l->lt_oz/2.0),2.0) );
    x_l_rad = sqrt(pow(l->lt_rad,2.0) - pow((l->lt_oz/2.0),2.0) );
    l->cap_ang = 57.29578 * atan(l->lt_oz/(2.0*x_f_rad));
    l->lower_dial = x_l_rad - x_f_rad;
  }
}

/*
* p l a t e a u  ( l ) 
*
* Parameters :
*   l : Lens Data.
*
* Purpose : Calculate plateau data.  Plateau lens defined by negative
*  radius difference base & secondary.
*
* Globals : n.a. 
*
* Returns : Lens Data modified.
* Date : Fri Apr  7, 1989; 10:47 am
*/
plateau(l)
LENS *l;
{
        int     n;              /* temp counter */
        double  tmp;            /* temp double just used to reduce typing */
        double  s1,s2;          /* temp sag values */

        n = l->rings-1;
        tmp = sqrt(pow(l->radius[n],2.0) - pow((l->diameter[n]/2.0),2.0) );
        l->cap_ang_p = 57.29578 * atan(l->diameter[n]/(2.0*tmp));
        l->lower_dial_p = sqrt(pow(l->radius[n-1],2.0) -
                        pow((l->diameter[n]/2.0),2.0)) - tmp;
        l->extra1l = l->radius[n] - l->radius[n-2] + l->lower_dial_p;
        l->extra1l = fabs(l->extra1l);
        s1 = sag(l->radius[n-1],l->diameter[n])
           - sag(l->radius[n],l->diameter[n]);
        s2 = sag(l->radius[n-1],l->diameter[n-1])
           - sag(l->radius[n-2],l->diameter[n-1]);
        l->extra2l = fabs(s2-s1);
}

/*
* j u n c t _ t h i c k  ( l, jt ) 
*
* Parameters :
*   l :  Lens Data.
*   jt : Desired junction thickness for Lens (l).
*
* Purpose : Change center thickness of given lens (l), to give
*  a lenticular junction thickness desired.
*
* Globals : n.a. 
*
* Returns : Modified lens data.
* Date : Fri Apr  7, 1989; 10:49 am
*/
void
junct_thick(l,jt)
LENS *l;
double jt;
{
  double dif;
  double save1,save2;

  while ( fabs(jt - l->lt_jt) > .0005 ) {
    dif = fabs(jt - l->lt_jt);
    if ( jt > l->lt_jt )
      l->center_thick += dif;
    else
      l->center_thick -= dif;
    main_calc(l);
  }
}

/*
* s i m p l e _ d e c a r l e  ( l ) 
*
* Parameters :
*   l : Lens data.
*
* Purpose : Determine simple decarle lens data based on a given lens
*  and an add power.
*
* Globals : n.a. 
*
* Returns : Modifed Lens data :
*   l->near_rad, l->near_pow, l->dist_rad, l->dist_pow
*
* Date : Fri Apr  7, 1989; 10:52 am
*/
simple_decarle(l)
LENS *l;
{
  double new_base, new_base_diopt, induced_pow, dist_power;
  double near_power;
  double tmpi;

  tmpi = (l->index - 1.0)*1000.0;

  {
    /* determine diopt index difference with material & tear */
    double m1_diopt,m2_diopt,m_diopt;
    double e1_diopt,e2_diopt,e_diopt;
    double rot_factor;

    m1_diopt = mm_to_diopt(tmpi,(double)10.0);
    m2_diopt = mm_to_diopt(tmpi,(double)8.0);
    m_diopt = m2_diopt - m1_diopt;
    e1_diopt = mm_to_diopt(TEAR_INDEX,(double)10.0);
    e2_diopt = mm_to_diopt(TEAR_INDEX,(double)8.0);
    e_diopt = e2_diopt - e1_diopt;

    induced_pow = m_diopt - e_diopt;
    rot_factor = induced_pow / e_diopt;
    induced_pow = l->add_pow/rot_factor;
  }
  if ( l->trial_rad < MAX_MM ) /* in diopters already */
    l->trial_rad = mm_to_diopt(TEAR_INDEX,l->trial_rad);

  new_base_diopt = l->trial_rad + induced_pow;
  new_base = diopt_to_mm(TEAR_INDEX,new_base_diopt);

  dist_power = l->trial_pow + l->trial_rad - new_base_diopt;
  near_power = l->add_pow + l->trial_pow;

  l->near_rad = diopt_to_mm(TEAR_INDEX,l->trial_rad);
  l->near_pow = near_power;
  l->dist_rad = new_base;
  l->dist_pow = dist_power;
}


/* from decar.c 4.1 8/4/88 */

/*
* d d e c a r l e  ( l ) 
*
* Parameters :
*   l : Lens data
*
* Purpose : Determine decentered part of decarle bifocal.  simple_decarle()
*  Should be called before this routine is called.  
*
* Globals : n.a. 
*
* Returns : Modified lens data :
*   l->decenter, l->prism, l->bel_val, l->touch, 
*   l->tch_add, l->tch_angle 
*
* Date : Thu Jun 15, 1989; 03:51 pm
*/
#include "circle.h"
ddecarle( l )
LENS *l;
{
  extern double rad_to_deg(double);
  extern double deg_to_rad(double);
  CIRCLE flat,steep,trak;
  POINT lens_center, oz_center;
  double theta_1, theta_2;
  double center_dist;
  double diff;
  int  not_done;
  double one_ang_mm;
  double tmp;

  if ( l->rotang != 0.0 ) {
    double a,c;
    /* if this lens requires rotation of oz in/out relative to
    * ballast prism, calculate the required seg height and use 
    * in the calculation of l->decenter bellow. 
    */

    a = l->seght + l->trunc + (l->oz/2.0) - (l->diameter[0]/2.0);
    c = a / cos(deg_to_rad(l->rotang));
    l->rot_sht = l->diameter[0]/2.0 + (c - l->oz/2.0) - l->trunc;
		
		/*
		* Determine side/down prism relative to balast prism input.
		* used in pr_lens, decarle stuff.
		*/
		a = deg_to_rad(l->rotang);
		c = l->bal_prism * 2.0;
		l->extra1l = (cos(a) * c) / 2.0;/* down prism*/
		l->extra2l = (sin(a) * c) / 2.0;/* side prism*/
  }

  tmp = l->diameter[0] / 2.0;
  tmp = tmp - l->trunc;
  tmp = tmp - (l->rotang != 0.0 ? l->rot_sht : l->seght );
  tmp = tmp + l->blk_prism + l->bal_prism;
  l->decenter = (l->oz / 2.0) - tmp;

  flat.r = l->near_rad;
  flat.x =
  flat.y = 0.0;

  steep.r = l->dist_rad;
  steep.y = 0.0;
  steep.x = flat.r - steep.r + 
            ( sag(steep.r,l->oz) - sag(flat.r,l->oz) );

  trak.r = steep.x;
  trak.x = 0.0;
  trak.y = 0.0;

  lens_center.x = 0.0;
  lens_center.y = 0.0;

  one_ang_mm = asin(1.0 / flat.r);
  theta_1 = asin( (l->oz / 2.0) / (flat.r) );
  theta_2 = 0.0;
  not_done = (1);

  do {
    double add_amount;

    steep.x = trak.r * cos(theta_2);
    steep.y = trak.r * sin(theta_2);

    oz_center.x = 0.0;
    oz_center.y = flat.r*sin(theta_2);

    center_dist = sqrt(pow(lens_center.x-oz_center.x,2.0) +
                  pow(lens_center.y-oz_center.y,2.0));

    diff = fabs(center_dist - l->decenter);
    if ( diff < SMALL )
            not_done = (0);
    else {
      add_amount = one_ang_mm * diff;
      if ( center_dist > l->decenter )
        theta_2 = theta_2 - add_amount;
      else
        theta_2 = theta_2 + add_amount;
    }
  } while ( not_done );

  {
    POINT p1,p2;
    double a,b;

    p1.x = steep.x;
    p1.y = steep.y;
    p2.x = trak.r;
    p2.y = 0.0;
    l->prism = sqrt(pow(p1.x-p2.x,2.0) + pow(p1.y-p2.y,2.0));

    l->bel_val = flat.r*sin(theta_1 - theta_2) - l->bal_prism;
    l->touch = flat.r * cos(theta_2) - steep.r * cos(theta_2);
    a = flat.r * cos(theta_2);
    b = steep.r * cos(theta_2) + steep.x;
    l->tch_add = b - a;
    l->tch_angle = steep.y/steep.x;
  }
}
