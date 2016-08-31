/* @(#) tor02.c 1.10@(#) 6/16/89 15:29:33 */
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

extern double tor_ooz_cylpow(double,double,double,double,double);
extern double flatOZ(double,double,double,double);

/* global routines used in this module */
extern LENS *alloc_lens();			/* tor01.c */
extern double backsag(LENS *);			/* tor01.c */
extern void debug_display(LENS *,char *);	/* help.c */
extern void oval_flat_oz(LENS *,LENS *);	/* here */

extern double square(double);			/* math01.c */
extern double sqrt(double);			/* math library */

/*
* function :  t o r _ r o u n d (l) 
*
* At Beginning of function we can assume the following :
* 1 - l->lens_type & LT_TOR_ROZ is true.
* 2 - Flat base curve is stored in l->t.flat_radius.
* 3 - Steep base curve is stored in l->t.steep_radius.
* 4 - Cylinder power is stored in l->t.cyl.
* 5 - Current lens stored in l->diameter/radius/... is the 
*     Flat lens and Peripheral with incorrect power.
*
* Purpose : The point of this routine is to calculate the following :
* 1 - round oz curves radius[l->rings-2] ... radius[0].
* 2 - power in flat meridian : Save in l->t.pow_flat
* 3 - edge thickness in steep/flat : save in l->t.et_[flat&steep]
* 4 - Junction thickness in steep/flat : save in l->t.jt_[flat&steep]
* 5 - Front surface cyl if needed : 
*       cyl, front_radius in : l->extra1l
*       difference in        : l->extra2l
*       pad cutting value in : l->extra3l
*
*/
void
tor_round(l)
LENS *l;
{
	LENS l_copy;
	register int i,oldrings;
	double tmp;

	/* make copy of lens in l_copy */
	bcopy(l,&l_copy,sizeof(LENS));

	/*
	* steep lens must have 15mm as outside peripheral radius
	*/
	l->diameter[0] = l->round.diam[0];	/* lens size */
	l->radius[0] = 15.0;			/* outside radius */

	/*
	* copy the flat lens diameter info to the steep lens
	*/
	for (i=0;i<l->rings;i++)
		l->diameter[i+1] = l->round.diam[i];
	l->rings = l->round.rings + 1;	/* one more */
	l->radius[l->rings-1] = l->t.steep_radius;

	/* 
	* determine the round/steep components of the lens.
	*/
	for (i=l->rings-1;i>1;i--) {
		double s1,s2,s3,s4,dif;

		s1 = sag(l->round.rad[i-1],l->round.diam[i-1]);
		s2 = sag(l->round.rad[i-2],l->round.diam[i-1]);
		dif = s1 - s2;
		s3 = sag(l->radius[i],l->diameter[i]);
		s4 = s3 - dif;
		l->radius[i-1] = (pow(s4,2.0) + 
			pow(l->diameter[i]/2.0,2.0))/(2.0*s4);
	}

	/* set up steep meridian lens. */
	/* determine steep power */
	l->t.pow_steep = tor_ooz_cylpow(l->t.cyl,l->t.pow_flat,
			l->t.flat_radius,l->t.steep_radius,l->index);
	/* use this power to calculate lens in steep meridian */
	l->power = l->t.pow_steep;

	/* set up flat meridian lens. */
	l_copy.rings = l->round.rings;
	for (i=0;i<l_copy.rings;i++) {
		l_copy.diameter[i] = l->round.diam[i];
		l_copy.radius[i] = l->round.rad[i];
	}
	/* use flat_pow to calculate lens in flat meridian */
	l_copy.power = l->t.pow_flat;	
		
	/* determine edge thickness in flat meridian */
	if ( l->pref == JT_PREF ) {
		junct_thick(&l_copy,l->t.jt_flat);
		l->pref = l_copy.pref = CT_PREF;
	}
	if ( l_copy.lens_type & LT_LENTIC )
	  l_copy.lt_et = l_copy.t.save_lt_et;
	main_calc(&l_copy);	/* calculate lens in flat */
	l->pref = CT_PREF;
	l->center_thick = l_copy.center_thick;
	main_calc(l);	/* calculate lens in steep meridian */

	if ( l->lens_type & LT_LENTIC ) {
	  l->lt_et = l_copy.lt_et + ( l->edge_thick - l_copy.edge_thick );
	  main_calc(l); /* re-calculate for new lenticular parameters */
	}

	l->pref = l_copy.pref;

	if ( l->t.cyl != 0.0 ) {
		l->extra1l = l_copy.front_radius;
		l->extra2l = l_copy.front_radius - l->front_radius;
		l->extra3l = l->front_radius - sqrt(pow(l->front_radius,2.0) -
				16.0) - 0.477 + 17.0 - l->front_radius;
	}
	/* save important data we want to printout */
	l->t.et_flat = l_copy.edge_thick;
	l->t.et_steep = l->edge_thick;
	if ( l->lens_type & LT_LENTIC ) {
		l->t.jt_flat = l_copy.lt_jt;
		l->t.jt_steep = l->lt_jt;
		l->t.et_flat = l_copy.lt_et;
		l->t.et_steep = l->lt_et;
	}
}

/*
* t o r _ o v a l  ( l ) 
*
* Parameters :
*   l : lens structure pointer.
*
* Purpose : Calculate required information for toric base or
*  bi-toric oval o.z. 
*
* Globals : n.a. 
*
* Returns : Modified input lens structure data.
* Date : Tue May 16, 1989; 02:18 pm
*
* Notes :
* At Beginning of function we can assume the following :
* 1 - l->lens_type & LT_TOR_OOZ is true.
* 2 - Flat base curve is stored in l->t.flat_radius.
* 3 - Steep base curve is stored in l->t.steep_radius.
* 4 - Cylinder power is stored in l->t.cyl.
* 5 - Current lens stored in l->diameter/radius/... is the 
*     Steep lens and Peripheral with incorrect power.
*
* Purpose : The point of this routine is to calculate the following :
* 1 - power in steep meridian : Save in l->t.pow_steep
* 2 - Oz in flat meridian : Save in l->t.oz_flat
* 3 - edge thickness in steep/flat : save in l->t.et_[flat&steep]
* 4 - Junction thickness in steep/flat : save in l->t.jt_[flat&steep]
* 5 - Front surface cyl if needed : 
*       cyl, front_radius in : l->extra1l
*       difference in        : l->extra2l
*       pad cutting value in : l->extra3l
*/
void
tor_oval(l)
LENS *l;
{
  LENS *ls,  /* steep meridian lens */
       *lf,  /* flat meridian lens */
       *lc;  /* Copy of input lens, & output data */
  register int i,j;
  double flat_sag,  /* used in calculation of lenticular */
	 steep_sag; /* edge thickness in flat meridian */

  /*
  * Allocate temporary lens data.
  */
  ls = alloc_lens();
  lf = alloc_lens();
  lc = alloc_lens();
  /*
  * Make 3 copies of input lens.
  */
  *ls = *l;  /* more efficent than calling a bcopy() routine */
  *lf = *l;
  *lc = *l;
  /* 
  * -- Setup steep lens. --
  * make the steep meridian lens a non oval o.z. lens.
  * so when debug_display(), it will not be toric.
  */
  ls -> lens_type = (ls -> lens_type & ~LT_TOR_OOZ );
  ls -> radius[ls->rings-1] = lc ->t.steep_radius;
  /* 
  * calculate power based on cyl power, flat base & index 
  */
  ls -> power = tor_ooz_cylpow( lc->t.cyl, lc->t.pow_flat, 
		lc->t.flat_radius, lc->t.steep_radius, l->index);
  main_calc(ls);
  if ( DEBUG )
    debug_display(ls,"Steep Lens");

  /*
  * -- Setup flat lens. --
  * make the flat meridian lens a non oval o.z. lens.
  * so when debug_display(), it will not be toric.
  */
  lf -> lens_type = (lf -> lens_type & ~LT_TOR_OOZ );
  lf -> power = lc -> t.pow_flat;
  /*
  * Don't allow center thickness to change.
  * Lens thickness must be constant between flat/steep 
  * meridians.  
  */
  lf -> pref = CT_PREF;
  lf -> center_thick = ls -> center_thick;
  lf -> radius[lf->rings-1] = lc -> t.flat_radius;
  /*
  * Determine the dimensions of flat meridian.  
  * oval_flat_oz() calculates where the flat meridian intersects
  * the steep lens.  I.E. where the flat lens intersects the steep
  * lens peripheral/secondary lens system.  
  */
  oval_flat_oz(ls,lf);	
  /*
  * In the lenticular case, it is desired to have a constant
  * lenticular radius in the steep/flat meridian.  Thus it is
  * nessesary for the following calculation.  In most cases the
  * lenticular edge thickness is constant.  In the case where
  * the intersection of flat base to the steep peripheral occurs
  * at a value > lens size, the lenticular edge thickness in the
  * flat meridian will be < steep meridian.
  */
  if ( lf -> lens_type & LT_LENTIC ) {
    flat_sag = backsag(lf);
    steep_sag= backsag(ls);
    if ( flat_sag != steep_sag )
      lf -> lt_et = ls -> lt_et - ( steep_sag - flat_sag );
  }
  main_calc(lf);
  if ( DEBUG )
    debug_display(lf,"Flat Lens");
  
  /*
  * Calculate Cutting lens.
  */
  lc -> power        = ls -> power;
  lc -> center_thick = ls -> center_thick;
  lc -> pref         = CT_PREF;		/* don't let ct change */
  main_calc(lc);
  debug_display(lc,"Cutting Lens");
  lc -> pref = ls -> pref;	/* restore ct/et pref */

  /*
  * All done, now update lens COPY to send back to parent 
  * subroutine.
  */
  lc -> t.pow_steep = ls -> power;
  lc -> t.pow_flat  = lf -> power;
  lc -> t.oz_steep  = ls -> diameter[ls->rings -1];
  lc -> t.oz_flat   = lf -> diameter[lf->rings -1];
  if ( lc -> lens_type & LT_LENTIC ) {
    lc -> t.jt_flat  = lf -> lt_jt;
    lc -> t.jt_steep = ls -> lt_jt;
    lc -> t.et_flat  = lf -> lt_et;
    lc -> t.et_steep = ls -> lt_et;
  } else {
    lc -> t.et_flat  = lf -> edge_thick;
    lc -> t.et_steep = ls -> edge_thick;
  }
  if ( l->t.cyl != 0.0 ) {
    lc -> extra1l = lf -> front_radius;
    lc -> extra2l = lf -> front_radius - ls -> front_radius;
    lc -> extra3l = 
      ls -> front_radius - sqrt(square(ls -> front_radius) - 16.0) 
      - 0.477 + 17.0 - ls -> front_radius;
  }
  debug_display(lc,"Return Lens");
  *l = *lc;
  free(lc);	/* get rid of malloc() lenses */
  free(ls);
  free(lf);
}

/*
* o v a l _ f l a t _ o z  ( s, f ) 
*
* Parameters :
*   s : steep lens record pointer.
*   f : flat lens record pointer.
*
* Purpose : Determine the proper flat oz for a given 
*  steep lens/radius.  This routine is used by tor_oval().
*  It assumes that we have an oval oz toric base with similar
*  peripheral/secondary curves.
*
* Globals : n.a. 
*
* Returns : modified flat lens record.
* Date : Mon May 15, 1989; 10:57 am
*/
void oval_flat_oz(s,f)
LENS *s,*f;
{
  double sd,oz;
  int not_done;
  int i,j;
  extern double sag_dif(LENS *,int );
  extern double sec_oz(double,double,double);

  i = (s->rings) - 1;
  not_done = 1;	/* true, we are not done yet */

  while ( not_done && i > 0 ) {
    sd = sag_dif(s,i);  /* determine sag difference at ring i */
    oz = sec_oz(s->radius[i-1],s->t.flat_radius,sd);
    if ( oz > s->diameter[i-1] ) 
      i--;
    else {
      f -> rings = i + 1;
      for ( j=0;j<i-1;j++ ) {
        f -> diameter[i] = s -> diameter[i];
        f -> radius[i] = s -> radius[i];
      }
      f -> diameter[i] = oz;
      f -> radius[i] = s -> t.flat_radius;
      not_done = 0;	/* false, we are not not done (!!done = done) */
    } /* oz > diameter */
  } /* while */
  if ( i <= 0 ) {
    f -> rings = 1;
    f -> diameter[0] = s -> diameter[0];
    f -> radius[0] = s -> t.flat_radius;
  }
} /* end if oval_flat_oz() */

/*
* s a g _ d i f  ( l, ring ) 
*
* Parameters :
*   l : input lens structure.  
*   ring : ring number to base sag_diference on.
*
* Purpose : This subroutine will determine the difference between
*  two spherical curves of contact lens defined by (l).  One curve
*  is defined as the base curve.  The other curve is identified as 
*  l->radius[rings].  In effect, this subroutine calculates the
*  entire back surface sag value upto and including the indicated 
*  radius[ring].  
*
* Globals : n.a. 
*
* Returns : sagital difference.
* Date : Fri May 12, 1989; 10:27 am
*/
double sag_dif(l,ring)
LENS *l;
int ring;
{
  double sag_total, sag_part;
  int i;

  sag_total = 0.0;
  for ( i=0;i<l->rings;i++ ) {
    sag_total += sag(l->radius[i],l->diameter[i]);
    if ( i ) 
      sag_total -= sag(l->radius[i-1],l->diameter[i]);
  }

  sag_part = 0.0;
  for ( i=0;i<ring;i++ ) {
    sag_part += sag(l->radius[i],l->diameter[i]);
    if ( i )
      sag_part -= sag(l->radius[i-1],l->diameter[i]);
  }

  return(sag_total-sag_part);
}

static double
tor_ooz_cylpow(cyl,flat_pow,flat_base,steep_base,index)
double cyl,flat_pow,flat_base,steep_base;
double index;
{
	double rot_factor;
	double flat_diopt,steep_diopt;
	double tmp;

	if ( cyl < 0.0 )
		return ( cyl+flat_pow );

	flat_diopt = mm_to_diopt(TEAR_INDEX,flat_base);
	steep_diopt= mm_to_diopt(TEAR_INDEX,steep_base);
	rot_factor = 2.9629624 * index - 3.962622;

	tmp = (flat_diopt - steep_diopt) * rot_factor +
		cyl + ( flat_diopt-steep_diopt ) + flat_pow; 
	return tmp;
}

/*
* function :  f l a t O Z (secR,flatR,steepR,steepOZ)
*
* Purpose : The point of this routine is to calculate the following :
*   - The oz dimension of the flat meridian relative to the
*     steepOZ/secRadius.
*
* Method :
* To determine the oz in the flat meridian, we use the fact that 
* the sag difference between steep/secondary is equal to the sag
* difference between flat/secondary.  From this we can solve the
* problem by subtracting one circle from the other.
*
	D = secR + flatR - sagdiff.

	1 =>	x^2     + y^2 = secR^2
	2 =>	(x-D)^2 + y^2 = flatR^2
	1-2 =>	(x-D)^2 - x^2 = flatR^2 - secR^2
	=>	2xD - D^2     = flatR^2 - secR^2
	=>	        2xD   = flatR^2 - secR^2 + D^2
	=>	        x     = (flatR^2 - secR^2 + D^2)/2D

	1 =>	x^2 + y^2     = secR^2
	substitute x from above.
	=>	y^2           = secR^2 - x^2
	=>	y^2           = secR^2 - (flatR^2 - secR^2 + D^2)/2D
	=>	y = sqrt(above)
*/
static double
flatOZ(secR,flatR,steepR,steepOZ)
double secR,flatR,steepR,steepOZ;
{
	double sd,	/* sag difference between steepR,secR @ steepOZ */
	       D,	/* distance between origins of the two circles */
			/* secR & flatR */
	       D2,	/* D^2 */
	       x,	/* depth coordinate (sag value) */
	       y,	/* oz coordinate */
	       secR2,	/* secR^2 */
	       flatR2;	/* flatR^2 */

	sd = sag(steepR,steepOZ) - sag(secR,steepOZ);
	D = secR + flatR - sd; 
	secR2 = secR*secR;
	flatR2 = flatR*flatR;
	D2 = D*D;
	x = (secR2 - flatR2 + D2)/(2.0*D);
	y = sqrt(secR2 - (x*x));
	return 2.0*y;
}

/*
* function :  b c o p y (a,b,n) 
*
* byte copy of a (to) b, of n bytes.
*/
static
bcopy(a,b,n)
register unsigned short *a,*b;
register int n;
{
	register int i;

	n = n / sizeof(unsigned short);

	for (i=0;i<n;i++) {
		b[i] = a[i];
	}
}

