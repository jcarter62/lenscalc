/* @(#) tor01.c 1.3@(#) 6/16/89 15:27:54 */
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

/* List of procedures/functions 
* alloc_lens(  ) 
* backsag( l ) 
* tor_secondary( l ) 
* sec_oz( flat_r, steep_r, dif ) 
* rad_at_axis( l ) 
* ellipse_xy( theta, a, b, xret, yret ) 
*/

/*
* Forward declarations 
*/
static void   rad_at_axis(LENS *);		/* here */
       double sec_oz(double,double,double);	/* here */
       double square(double);			/* math01 */
static void ellipse_xy(double,double,double,double *,double *); /* here */
       double backsag(LENS *);			/* here */
       LENS *alloc_lens();			/* here */


/*
* a l l o c _ l e n s  (  ) 
*
* NO-Parameters :
* Purpose : allocate from heap one lens structure, return byte pointer.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Apr  6, 1989; 04:27 pm
*/
LENS *alloc_lens()
{
	LENS *ptr;

	ptr = (LENS *)malloc(sizeof(LENS));
	if ( ptr == NULL ) {
		perror("alloc_lens()");
		exit(-1);
	}
	return ptr;
}

/*
* b a c k s a g  ( l ) 
*
* Parameters :
*   l : LENS pointer.
*
* Purpose : calculate back surface sag value.
*
* Globals : n.a. 
*
* Returns : see purpose.
* Date : Thu Apr  6, 1989; 11:18 am
*/
double backsag(l)
register LENS *l;
{
	register int i;
	register double ret;

	ret = 0.0;
	for (i=0;i<l->rings-1;i++) {
		ret += sag(l->radius[i],l->diameter[i]) -
		       sag(l->radius[i],l->diameter[i+1]);
	}
	ret += sag(l->radius[l->rings-1],l->diameter[l->rings-1]);
	return(ret);
}

/*
* t o r _ s e c o n d a r y  ( l ) 
*
* Parameters :
*   l : Lens structure.
*
* Purpose : Calculate the nessesary values for toric secondary
*
* Globals : n.a. 
*
* Returns : modified lens data.
* Date : Thu Mar 30, 1989; 03:22 pm
*
* ---- Notes ----
At entry, we assume the following :
 l->lens_type & LT_TOR_SEC == TRUE, we don't check here.
 l->t.save_et == flat meridian edge thickness if lens is not a 
		 lenticular lens.
* ---End Notes--- 
*/
void tor_secondary(l)
LENS *l;
{
	int i,im1,im2,r;
	double dif1,dif2,dif;/* sag difference between 2 spherical radius */
	double base;	/* base curve value, just a temp for speed */
	double sec_out_oz;	/* outer oz value */
	LENS *ls,	/* steep secondary + base & periph */
	     *lf,	/* flat secondary + base & periph */
	     *lc,	/* copy of input lens. */
	     *l1,*l2;	/* pointers used to select flat/steep et pref */

	ls = alloc_lens();	/* never returns on error */
	lf = alloc_lens();
	lc = alloc_lens();

	main_calc(l);	/* recalculate lens that will be cut */

	/* make copies of (l) */
	*lc = *l;
	*ls = *l;
	*lf = *l;

	base = lc->radius[lc->rings-1];		/* base curve */
	/* 
	* determine inside oz in steep meridian 
	* (steep secondary/base curve) dimension
	*/
	dif = sag(base,lc->t.oz_flat) - 
	      sag(lc->t.flat_radius,lc->t.oz_flat);
	lc->t.oz_steep = sec_oz(lc->t.steep_radius,base,dif);

	/* 
	* setup flat secondary lens 
	*/
	i = lc->rings;
	/*
	* determine outer oz with respect to peripheral curve,
	* and flat secondary curve 
	*/
	dif1 = sag(base,lc->diameter[1]) - sag(lc->radius[0],lc->diameter[1]);
	dif2 = sag(base,lc->t.oz_flat) - sag(lc->t.flat_radius,lc->t.oz_flat);
	dif = dif1 - dif2;
	sec_out_oz = sec_oz(lc->radius[i-2],lc->t.flat_radius,dif);
	lc->t.ooz_flat = sec_out_oz;
	/*
	* determine if (sec_out_oz) should be used.
	*/
	if ( sec_out_oz >= lc->diameter[i-2] ) 
	{
		lf->radius[i-2] = lc->t.flat_radius;
		lf->diameter[i-1] = lc->t.oz_flat;
	} else {
		lf->radius[i]   = lf->radius[i-1];		/* base rad */
		lf->diameter[i] = lc->t.oz_flat;		/* base diam */
		lf->radius[i-1] = lc->t.flat_radius;		/* sec rad */
		lf->diameter[i-1] = sec_out_oz;			/* sec diam */
		lf->rings++;
	}

	/* 
	* setup steep secondary lens 
	*/
	/*
	* if oz in steep meridian is > cutting oz, then lens is just
	* a copy of cutting lens.  This occures if the difference between
	* flat/steep lens is relatively large and steep curve doesn't 
	* actually touch steep meridian.
	*/
	if ( lc->t.oz_steep > lc->diameter[lc->rings-1] ) {
		/* 
		* indicate that values in steep meridian are not valid 
		*/
		lc->t.ooz_steep = -1.0;
	} else {
		i = ls->rings;
		im1 = i - 1;
		im2 = i - 2;
		ls->radius[i]   = ls->radius[im1];		/* base rad */
		ls->diameter[i] = lc->t.oz_steep;			/* base diam */
		ls->radius[im1] = lc->t.steep_radius;		/* sec rad */
		/*
		* determine outer oz with respect to peripheral curve,
		* and flat secondary curve 
		*/
		dif1 = sag(base,lc->diameter[1]) - 
		       sag(lc->radius[0],lc->diameter[1]);
		dif2 = sag(base,lc->t.oz_steep) - 
		       sag(lc->t.steep_radius,lc->t.oz_steep);
		dif = dif1 - dif2;
		sec_out_oz = sec_oz(ls->radius[im2],ls->radius[im1],dif);
		lc->t.ooz_steep = sec_out_oz;
		/*
		* determine if (sec_out_oz) should be used.
		*/
		if ( sec_out_oz > ls->diameter[im2] ) 
			ls->diameter[im1] = ls->diameter[im2];
		else
			ls->diameter[im1] = sec_out_oz;
		ls->rings++;
	}

	/*
	* Determine flat/steep axis takes precidence in et/jt 
	* calculations.  
	*/
	if ( lc->t.axis > 0 ) {
		l1 = ls;
		l2 = lf;
	} else {
		l1 = lf;
		l2 = ls;
	}

	/*
	* Setup is now complete.
	* Now calculate the lenses, in steep & flat meridian.
	*/
	ls->pref = lf->pref = CT_PREF;
	if ( lc->lens_type & LT_LENTIC ) {
		/*
		* flat/steep lenses are considered separately just
		* regular non-toric-secondary lenses.
		*/
		l1->lens_type = (l1->lens_type & ~LT_TOR_SEC );
		l2->lens_type = (l2->lens_type & ~LT_TOR_SEC );
		/* 
		* calculate first lens 
		*/
		l1->lt_et = lc->t.save_lt_et;	/* saved lentic et */
		main_calc(l1);
		/*
		* Determine junction thickness if needed.
		*/
		if ( lc->pref == JT_PREF ) {
			junct_thick(l1,lc->t.save_jt);
			main_calc(l1);
		}
		/* 
		* copy center thickness for new other lens , both flat/steep
		* lenses must have a constant center thickness.
		*/
		l2->center_thick = l1->center_thick;
		/*
		* Determine Lenticular Edge thickness for second lens. :
		* Purpose of this is to force the front surfaces of l1 & l2
		* to be identical.  I.E. same lenticular radius,...
		*  Assumption : (min_bt used, minimum button thickness)
		*  l->min_bt is based on the back surface thickness and
		*  center thickness.  Thus the difference between flat/steep
		*  (min_bt) == difference between flat/steep edge thickness,
		*  given a constant center thickness and also constant 
		*  front surface (spherical front).
		*/
		l2->pref = CT_PREF;
		main_calc(l2);

		dif = l1->min_bt - l2->min_bt;
		l2->lt_et = l1->lt_et - dif;

		main_calc(l2);	/* recalculate with new lenticular e.t. */
		/*
		* At this point we have know flat/steep lenses for this system.
		* Now save results in (lc). {l1&l2 front surfaces are 
		* identical Thus use either l1 or l2 or lf or ls }
		*/
		lc->t.lt_ca	= l1->cap_ang;
		lc->t.lt_ld	= l1->lower_dial;
		lc->t.lt_et	= l1->lt_et;
		lc->t.lt_oz	= l1->lt_oz;
		lc->t.lt_rad	= l1->lt_rad;
		lc->t.lt_set	= l1->lt_set;
		lc->t.lt_tkoff	= l1->lt_tkoff;
		lc->t.lt_jt	= l1->lt_jt;

		lc->t.jt_flat	= lf->lt_jt;
		lc->t.jt_steep	= ls->lt_jt;
		lc->t.et_flat	= lf->lt_et;
		lc->t.et_steep	= ls->lt_et;

		lc->center_thick	= lf->center_thick;
		lc->lens_type = (lc->lens_type & ~LT_TOR_SEC );
		lc->pref = CT_PREF;
		main_calc(lc);
		/*
		* Determine Lenticular Edge thickness as above,
		* for the lens which has neither flat/steep secondary.
		* Make this modification relative to first lens (l1).
		*/
		dif = l1->min_bt - lc->min_bt;
		lc->lt_et = l1->lt_et - dif;

		main_calc(lc);	/* recalculate with new lenticular e.t. */
		lc->pref = l->pref;
		lc->lens_type = l->lens_type;
		*l = *lc;
		/* end of lenticular - toric secondary */
	} else { /* not lenticular */
		if ( lc->pref == ET_PREF ) {
			l1->edge_thick = lc->t.save_et;
			l1->pref = ET_PREF;
		} else
			l1->pref = CT_PREF;
		l1->lens_type = (l1->lens_type & ~LT_TOR_SEC );
		/* 
		* calculate flat lens 
		*/
		main_calc(l1);

		/*
		* Determine steep lens center thickness
		*/
		l2->center_thick = l1->center_thick;
		l2->pref = CT_PREF;
		l2->lens_type = (l2->lens_type & ~LT_TOR_SEC );
		/* 
		* calculate second lens 
		*/
		main_calc(l2);
		/* ----------- Done with second lens ------------- */
		/*
		* Store edge thicknesses in both meridians 
		* in the return data.
		*/
		lc->t.et_flat = lf->edge_thick;
		lc->t.et_steep = ls->edge_thick;
		/*
		* now calculate the cutting lens (no flat/steep secondary)
		* return with cutting lens in (l).
		*/
		lc->center_thick = ls->center_thick;
		*l = *lc;
		l->pref = CT_PREF;
		main_calc(l);
		l->pref = lc->pref;
	}
	free(ls);
	free(lf);
	free(lc);
}

/*
* s e c _ o z  ( flat_r, steep_r, dif ) 
*
* Parameters :
*   flat_r : flatter radius of these two.
*   steep_r : steeper radius of these two.
*   dif : distance difference at y=0 and x>0.
*
* Purpose : Determine diameter created by flat/steep given
*           radius and also given the constraints of (dif) value.
*           Thus determine oz for given two curves with a sag
*           difference of dif.
*
* Globals : n.a. 
*
* Returns : optic zone dimension of flat_r/steep_r combination.
* Date : Thu Mar 30, 1989; 03:09 pm
*/
double sec_oz(flat_r,steep_r,dif)
double flat_r,steep_r,dif;
{
	double x,y,a;

	a = (flat_r - steep_r) + dif;
	x = (square(flat_r) - square(steep_r) + square(a))/(2.0*a);
	y = sqrt(square(flat_r)-square(x));
	return (2.0 * y);
}

#ifdef PRISM_AT_AXIS
/*
* r a d _ a t _ a x i s  ( l ) 
*
* Parameters :
*   l : lens structure pointer.
*
* Purpose : Determine the demensions of lens at l->t.axis, 
*  based on flat/steep axis of lens.
*
* Globals : n.a. 
*
* Returns : n.a. 
----- Notes -----
The basic idea of this routine is to describe the toric secondary curve
as an elipsoid.  Given an angle, Determine what the cross section 2-D lens
looks like at the given axis.  I.E. Determine the oz and width along with
the curvature of radius based on the values deposited in lens parameter (l).

First we determine the oz and width, (oz + width = outer oz).  Then we treat
the cross-section at axis as a spherical lens.  At that point we can
determine the radius of curvature.
--- End Notes ---
* Date : Mon Apr  3, 1989; 01:38 pm
*/
static void rad_at_axis(l)
LENS *l;
{
	double a,b;
	double x,y,x1,y1;
	double oz1,oz2,axis,rad;

	axis = deg_to_rad(l->t.axis);
	/*
	* Determine (x,y) of inner oz at given axis.
	*/
	ellipse_xy(axis,(l->t.oz_flat/2.0),(l->t.oz_steep/2.0),
		&x,&y);
	/*
	* Determine (x1,y1) of outer oz at given axis.
	*/
	ellipse_xy(axis,(l->t.ooz_flat/2.0),(l->t.ooz_steep/2.0),
		&x1,&y1);

	/*
	* Determine inner/outer oz dimensions at given axis.
	*/
	oz1 = 2.0 * y;
	oz2 = 2.0 * y1;

	rad = sqrt((square(x) + square(y) + square(x1) + square(y1)) / 2.0 );

	/* 
	* We now have completed mission, now store values in lens structure
	*/
	l->t.ioz_axis = oz1;		/* inner oz at axis */
	l->t.ooz_axis = oz2;		/* outer oz at axis */
	l->t.rad_axis = rad;		/* radius of curvature */
}

/*
* e l l i p s e _ x y  ( theta, a, b, xret, yret ) 
*
* Parameters :
*   theta : angle we want to match.
*   a : x axis dimension of ellipse
*   b : y axis dimension of ellipse
*   xret : return value x
*   yret : return value y
*
* Purpose : Determine point(x,y) for which atan(y/x) = theta.
*
* Globals : n.a. 
*
* Returns : modified xret & yret.
* Date : Tue Apr  4, 1989; 04:22 pm
*/
static void ellipse_xy(theta,a,b,xret,yret)
double theta,a,b;
double *xret,*yret;
{
	double a2,b2;
	double thta_dif;
	double thta,x,y;
	double value;		/* temp value for add/sub to x value */
	int not_done = 1;

	a2 = a * a;
	b2 = b * b;
	x = a;
	y = 0.0;
	while ( not_done ) {
		y        = sqrt(a2 - (a2*square(x)/b2));
		thta     = atan(y/x);
		thta_dif = theta - thta;
		if ( fabs(thta_dif) < 0.0001 )
			not_done    = 0;	/* done */
		else {
			value = 1.0/(M_PI/2.0 * a);
			if ( thta_dif < 0 ) value    = -(value);
			x += value;
		}
	}
	*xret = x;
	*yret = y;
}
#endif
