/* @(#) toric.c 5.22@(#) 8/29/90 09:20:53 */
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

#ifdef dos
# ifndef M_PI
#  define M_PI  3.14159265358979323846
# endif
#endif

#define next(string) {move(cur_row++,0);clrtoeol();printw("%s",string);}

#define M_LINES 7

char *menu_ary[M_LINES]= { " Toric Menu.","1 - Toric Front","2 - Toric Secondary",
  "3 - Toric Base, Round O.Z.","4 - Toric Base, Oval O.Z.",
  "q - main menu" ,"\0" };

/* List of procedures/functions toric.c
* toric_menu( l ) [public void]
* parse_cyl( s, cyl, axis )  [static void]
* get_cyl_etc( l ) [static void]
* vadd( v1, v2, v3 ) [static void]
* get_stp_flt( l ) [static void]
* swapd( a, b ) [static void]
*/

/* forward declarations */
static void parse_cyl(char *,double *,double *);
static void get_cyl_etc(LENS *);
static void vadd(VECTOR *, VECTOR *, VECTOR *);
static void get_stp_flt(LENS *);


/*
* t o r i c _ m e n u  ( l ) [public void]
*
* Parameters :
*   l : lens pointer.
*
* Purpose : main toric menu entry.
*
* Globals : n.a. 
*
* Returns : posible modification to (l).
* Date : Tue Apr 11, 1989; 11:12 am
*/
void toric_menu(l)
LENS *l;
{
  extern void get_cyl_etc(LENS *);
  extern double get_dbl(char *,double *);
  extern int cur_row;
  extern void swapd(double *,double *);
  char buf[80];
  int not_done = 1;
  register int i;
  TORIC_DATA *tor;
  double tmp;
  extern double round(double);
  char cyl_ax_buf[100];

  clear();
  while ( not_done ) {
    i = menu(menu_ary);
    cur_row = M_LINES+1;
    switch( i ) {
    case 1 : /* toric front */
      next("");
      strcpy(cyl_ax_buf,"");
      help_msg("Front Cylinder Power in Diopters, Plus Axis.");
      get_str("Toric Front (Cylinder x Axis) ",cyl_ax_buf);
      parse_cyl(cyl_ax_buf,&(l->t.cyl),&(l->t.axis));
      if ( l->t.cyl == 0 ) {
        l->lens_type = l->lens_type & (~LT_TORIC_FRONT);
      } else {
        /*
        * Now get prism value.
        */
        help_msg("A value <= .64 = MM offset, > .64 = Prism amount");
        l->bal_prism = l->bal_prism/0.16;
        l->bal_prism = get_dbl("Ballast Prism",&l->bal_prism);
        /* 4 prism */
        if ( l->bal_prism > 0.0 ) {
          if ( l->bal_prism > .64 ) l->bal_prism = l->bal_prism * 0.16;
          if ( l->min_et < .05 || l->min_et > 1.0 ) l->min_et = l->edge_thick;
          tmp = l->min_et;
          help_msg(
"This value is used to calculate the required thickness based on above prism");
          tmp = get_dbl("Minimum Edge Thickness",&tmp);
          l->min_et = tmp;
          tmp +=(l->diameter[0]*(l->bal_prism/0.16)/(2.0*49.0));
          sprintf(buf,
"This is the e.t. required to produce a finished e.t. of %4.2lf with %g prism",
          l->min_et,(l->bal_prism/0.16));
          help_msg(buf);
          tmp = get_dbl("Edge Thickness + Ballast prism edge", &tmp);
          help_msg("");
          if ( tmp > l->edge_thick )
            l->edge_thick = tmp;
          l->pref = ET_PREF;
        
          /* 
          * recalculate minimum et in case bal+min et is not
          * used for the reason above where the new bal+min
          * edge thickness is less than the current lens et.
          */
          l->min_et = l->edge_thick - 
            ( l->diameter[0]*(l->bal_prism/0.16)/(2.0*49));
  
          /* set prism bit/flag */
          if ( l->bal_prism != 0.0 )
            l->extra_flags |= EXRA_PRISM;
          else
            l->extra_flags &= ~EXRA_PRISM;

        }

        l->trunc = 0.0;
        l->trunc = get_dbl("Truncation",&(l->trunc));
        if ( l->trunc > 3.0 )
          l->trunc = l->diameter[0] - l->trunc;

        l->lens_type = l->lens_type | LT_TORIC_FRONT;
				move( LINES - 1 , 5 );
				printw("Lens Type = %x",l->lens_type);
				refresh();
      }
      not_done = 0;
      break;
    case 2 : /* toric secondary */
      if ( l->rings > 2 ) {
        next("Only 2 rings allowed on Toric Secondary");
        next("Press <Enter> to continue");
        getstr("buffer");
        return;
      }
      next("");
      tor = &(l->t);

      sprintf(help_buf,
      "FLAT MM Value somwhere between %7.2lf and %7.2lf",
        l->radius[1],l->radius[0]);
      help_msg(help_buf);
      tor->flat_radius = get_dbl("Flat meridian Radius",
        &(tor->flat_radius));
      tor->steep_radius = (round(l->radius[l->rings-1]/10.0)
        *10.0) + 1.0;

      sprintf(help_buf,
      "STEEP MM Value somwhere between %7.2lf and %7.2lf",
        l->radius[1],tor->flat_radius);
      help_msg(help_buf);
      tor->steep_radius = get_dbl("Steep meridian Radius",
        &(tor->steep_radius));

      sprintf(help_buf,
      "Smallest O.Z., Probably less than %7.2lf",
        l->diameter[1]);
      help_msg(help_buf);
      tor->oz_flat = get_dbl("Narrow OZ",&(tor->oz_flat));
      if ( !(l->lens_type & LT_TOR_SEC )) {
        l->lens_type = l->lens_type | LT_TOR_SEC;
        tor->save_et = l->edge_thick;
        tor->save_jt = l->lt_jt;
        tor->save_lt_et = l->lt_et;  
      }
      
      /* get axis preference */
      do {
      help_msg("Enter 0 for flat,90 for steep meridian");
      tor->axis = 
      get_dbl("Toric-Sec prism Axis, 0=flat/90=steep",
                  &(tor->axis));
      } while ( tor->axis != 0.0 && tor->axis != 90.0 );

      help_msg("");
      not_done = 0;
      break;
    case 3 : /* round oz */
      next("");
      get_stp_flt(l);
      l->radius[l->rings-1] = l->t.flat_radius;
      l->round.rad[l->round.rings-1] = l->t.flat_radius;
      if ( !(l->lens_type & LT_TOR_ROZ )) {
        for (i=0;i<l->rings;i++) {
          l->round.diam[i] = l->diameter[i];
          l->round.rad[i] = l->radius[i];
        }
        l->round.rings = l->rings;
        l->lens_type = l->lens_type | LT_TOR_ROZ;
        l->t.pow_flat = l->power;
        l->t.oz_steep = l->diameter[l->rings-1];
      }
      get_cyl_etc(l);
      not_done = 0;
      break;
    case 4 : /* oval oz */
      next("");
      get_stp_flt(l);
      l->radius[l->rings-1] = l->t.steep_radius;
      if ( !(l->lens_type & LT_TOR_OOZ )) {
        l->lens_type = l->lens_type | LT_TOR_OOZ;
        l->t.pow_flat = l->power;
        l->t.oz_steep = l->diameter[l->rings-1];
      }
      get_cyl_etc(l);
      not_done = 0;
      break;
    case 5 :
      not_done = 0;
      break;
    default  :
      next("");
      next("Error");
      refresh();
      getstr(buffer);
      break;
    }
  }  
  clear();
  refresh();
}

/*
* p a r s e _ c y l  ( s, cyl, axis )  [static void]
*
* Parameters :
*   s : char * (input in form [ # [x #] ], can be empty)
*   cyl : double *, result cylinder value.
*   axis : double *, result axis value.
*
* Purpose : parse input string and put results in cyl,axis 
*  variables.  input string can be empty, or just contain cylinder
*  value.
*
* Globals : n.a. 
*
* Returns : modified cyl & axis.
* Date : Tue Apr 11, 1989; 11:08 am
*/
static void
parse_cyl(s,cyl,axis)
char *s;
double *cyl,*axis;
{
        extern int cur_row;
  register char *p1,*p2;
  char buf[80];

  p1 = s;
  p2 = NULL;
  while (*s) {
    if (*s == 'x' || *s == 'X')  {
      p2 = s+1;
      break;
    } else
      s++;
  }

  *cyl = atof(p1);
  if ( p2 != NULL ) 
    *axis = atof(p2);
  else
    *axis = 0.0;

  sprintf(buf," %g x %g ",*cyl,*axis);
  next(buf)
  refresh();
}

/*
* g e t _ c y l _ e t c  ( l ) [static void]
*
* Parameters :
*   l : lens structure pointer.
*
* Purpose : get cylinder + [axis] as well as other information
*  as required by the situation at hand.
*
* Globals : n.a. 
*
* Returns : modified lens structure.
* Date : Tue Apr 11, 1989; 11:02 am
*/
static void
get_cyl_etc(l)
LENS *l;
{
  extern double get_dbl(char *,double *);
  extern int cur_row;
#ifdef dos
  static char buf[100];
  static double tmp;
#else
  char buf[100];
  double tmp;
#endif
  double tmp_i;

  /* reset prism flag bit */
  l->extra_flags &= ~EXRA_PRISM;

  next("Cylinder Power [+|-]cyl [x Axis]");
  next(" Axis = Flat meridian Axis of Base");
  sprintf(buf,"%g x %g",l->t.cyl,l->t.axis);
help_msg(
"Cylinder Power in form [+-]# [x #], 0 = Sphere Front, s = Sphere Equivalent");
  get_str("Cylinder",buf);
  help_msg("");
  /*
  * If we just input what is already input
  */
  if ( strlen(buf) == 0 )
    return;
  /*
  * If we specify Sphere Equivalent.
  */
  if (( buf[0] == 's' || buf[0] == 'S' ) && strlen(buf) < 2 ) {
    l->t.cyl = 
      mm_to_diopt(TEAR_INDEX,l->t.flat_radius) -
      mm_to_diopt(TEAR_INDEX,l->t.steep_radius);  
    l->t.axis = 0.0;
  } else {
    extern void parse_cyl(char *, double *, double *);

    parse_cyl(buf,&(l->t.cyl),&(l->t.axis));
  }
  /*
  * Cyl Power and Axis is known at this point.
  */
  if ( l->t.axis == 0.0 && l->t.cyl == 0.0 ) {
    /*
    * No mark, Spherical Front.
    */
    l->t.tor_type = TOR_SPH_F;
    return;
  } else if ( l->t.axis == 0.0 && l->t.cyl != 0.0 ) {
    /*
    * Mark at screw. Done
    */
    l->t.tor_type = TOR_CYL_F1;  /* simple bitoric. */
    return;
  } 

  /* Input prism value. */
  /* convert to prism, not mm offset */
  help_msg("A value <= .64 = MM offset, > .64 = Prism amount");
  l->bal_prism = l->bal_prism/0.16;
  l->bal_prism = get_dbl("Ballast Prism",&l->bal_prism);
  /* 4 prism */
  if ( l->bal_prism > 0.0 ) {
  if ( l->bal_prism > .64 ) l->bal_prism = l->bal_prism * 0.16;
  if ( l->min_et < .05 || l->min_et > 1.0 ) l->min_et = l->edge_thick;
  tmp = l->min_et;
  help_msg(
"This value is used to calculate the required thickness based on above prism");
  tmp = get_dbl("Minimum Edge Thickness",&tmp);
  l->min_et = tmp;
  tmp +=(l->diameter[0]*(l->bal_prism/0.16)/(2.0*49.0));
  sprintf(buf,
"This is the e.t. required to produce a finished e.t. of %4.2lf with %g prism",
l->min_et,(l->bal_prism/0.16));
  help_msg(buf);
  tmp = get_dbl("Edge Thickness + Ballast prism edge", &tmp);
  help_msg("");
  if ( tmp > l->edge_thick )
    l->edge_thick = tmp;
  l->pref = ET_PREF;

  /* 
  * recalculate minimum et in case bal+min et is not
  * used for the reason above where the new bal+min
  * edge thickness is less than the current lens et.
  */
  l->min_et = l->edge_thick - 
    ( l->diameter[0]*(l->bal_prism/0.16)/(2.0*49));

  /* set prism bit/flag */
  if ( l->bal_prism != 0.0 )
    l->extra_flags |= EXRA_PRISM;

  }
  /* -----------
  * At this point we have eliminated the spheric front
  * and also the +/-cyl x 0 case.
  */
  if ( l->t.cyl == 0.0 && l->t.axis != 0.0 ) {
    /* 
    * Lay flat meridian at angle l->t.axis and then
    * mark at 90 (top of lens)
    */
    l->t.tor_type = TOR_CYL_F2;
    return;
  }  

  
  if ( l->bal_prism == 0.0 ) 
    l->t.tor_type = TOR_CYL_F3;
  else
    l->t.tor_type = TOR_CYL_F4;
    
  help_msg("You may find this value in the K.READINGS area");
  l->t.axis_fm = get_dbl("Axis of Flat Meridian",&(l->t.axis_fm));
  help_msg("");

  /* set up vector 1 (l->t.v1) */
  l->t.v1.sph = l->t.pow_flat;
  tmp_i = (l->index - 1.0)*1000.0;
  l->t.v1.cyl = mm_to_diopt(tmp_i,l->t.flat_radius) -
                mm_to_diopt(tmp_i,l->t.steep_radius);
  l->t.v1.axis = l->t.axis_fm;

  /* set up vector 2 (l->t.v2) */
  l->t.v2.sph = 0.0;
  l->t.v2.cyl = l->t.cyl;
  l->t.v2.axis = l->t.axis;

  /* determine result powers */
  vadd(&(l->t.v1),&(l->t.v2),&(l->t.v3));
  return;
}


/* 
* defines needed for vadd function.
*/
#define sign(a) (a < 0.0 ? -1 : 1)
#define deg_to_rad(x) ((x)*M_PI / 180.0)
#define rad_to_deg(x) ((x)*180.0 / M_PI)
/*
* v a d d  ( v1, v2, v3 ) [static void]
*
* Parameters :
*   v1 : input vector
*   v2 : input vector
*   v3 : result vector
*
* Purpose : add vectors v1+v2 == vector v3.
*  input vectors must be supplied in polar form, 
*  (sph,cyl,axis)  output in form of input as well
*  as (x,y) for all (v1,v2,v3) is also supplied.
*
* Globals : n.a. 
*
* Returns : v3 updated.
* Date : Mon Apr  3, 1989; 01:24 pm
*/
static void vadd(v1,v2,v3)
VECTOR *v1,*v2,*v3;
{
  double x,y,axis;

  /* 
  * Make sure both are in either + cyl or - cyl but
  * don't allow one in + cyl and one in - cyl.
  */
  if ( sign(v1->cyl) != sign(v2->cyl) ) {
    v2->sph = v2->sph+v2->cyl;
    v2->cyl = -(v2->cyl);
    if ( v2->axis >= 90.0 )
      v2->axis = v2->axis - 90.0;
    else
      v2->axis = v2->axis + 90.0;
  }
  /* 
  * convert both vectors to x,y coordinate system 
  */
  v1->x = v1->cyl*cos(deg_to_rad(v1->axis*2.0));
  v1->y = v1->cyl*sin(deg_to_rad(v1->axis*2.0));
  v2->x = v2->cyl*cos(deg_to_rad(v2->axis*2.0));
  v2->y = v2->cyl*sin(deg_to_rad(v2->axis*2.0));

  x     = 
  v3->x = v1->x + v2->x;
  y     = 
  v3->y = v1->y + v2->y;

  if ( x == 0.0 && y == 0.0 ) {
    v3->sph =
    v3->cyl = 
    v3->axis= 0.0;
    return;
  }

  v3->cyl = sqrt(pow(x,2.0)+pow(y,2.0));

  if ( x>0.0 && y>= 0.0 )
    axis = rad_to_deg(atan(y/x));
  else if ( x<0.0 )
    axis = rad_to_deg(atan(y/x))+180.0;
  else if ( x>0.0 && y<0.0 )
    axis = rad_to_deg(atan(y/x))+360.0;
  else if ( y>0.0 && x==0.0 )
    axis = 90.0;
  else if ( y<0.0 && x==0.0 )
    axis = 270.0;
  else
    axis = 0.0;

  v3->sph = (((v1->cyl)+(v2->cyl)-(v3->cyl))/2.0) 
    + v1->sph + v2->sph;
  v3->axis = axis;

  /* 
  * Make sure v3 is in - cyl 
  */
  if ( sign(v3->cyl) > 0 ) {
    v3->sph = v3->sph+v3->cyl;
    v3->cyl = -(v3->cyl);
    if ( v3->axis >= 180.0 )
      v3->axis = v3->axis - 180.0;
    else
      v3->axis = v3->axis + 180.0;
  }
  v3->axis = v3->axis / 2.0;
}

/*
* g e t _ s t p _ f l t  ( l ) [static void]
*
* Parameters :
*   l : LENS *
*
* Purpose : prompt user to input flat/steep radius in a non
*           ambiguous manner.
*
* Globals : 
*  hlpfmt : static sprintf format string.
*
* Returns : n.a. 
* Date : Tue Apr 11, 1989; 11:02 am
*/
static char *hlpfmt=
"Diopters or mm. Current Base = (%7.3lf/%7.3lf)mm (%6.2lf/%6.2lf)d";

static void get_stp_flt(l)
LENS *l;
{
#ifdef dos
  static char buf[80];
  static double stp,flt,tmp;
#else
  char buf[80];
  double stp,flt,tmp;
#endif
        extern int cur_row;
  extern double round(double);

  stp = l->t.steep_radius;
  flt = l->t.flat_radius;

  if ( stp == 0.0 ) {   /* never done before */
    tmp = l->radius[l->rings-1];

    sprintf(buf,"Current Base Curve is %7.3lfmm(%6.2lf)d",tmp
      ,mm_to_diopt(TEAR_INDEX,tmp));
    next(buf);

    help_msg("Diopters or MM");
    do {
      stp=get_dbl("Other Base Curve",&stp);
    } while (stp == 0.0);
    if ( stp > MAX_MM ) 
      stp = diopt_to_mm(TEAR_INDEX,stp);
    stp = round(stp);
    if ( stp > tmp ) {
      flt = stp;
      stp = tmp;
    } else
      flt = tmp;
  } else {
    if ( stp < 0.0 ) {   /* wiped lens (8) pressed */
      stp = - stp;
      flt = l->radius[l->rings-1];
    }
    sprintf(buf,hlpfmt,stp,flt,
      mm_to_diopt(TEAR_INDEX,stp),mm_to_diopt(TEAR_INDEX,flt));
    help_msg(buf);
    flt = get_dbl("Flat Base",&flt);
    if ( flt > MAX_MM ) flt = diopt_to_mm(TEAR_INDEX,flt);  
    flt = round(flt);

    sprintf(buf,hlpfmt,stp,flt,
      mm_to_diopt(TEAR_INDEX,stp),mm_to_diopt(TEAR_INDEX,flt));
    help_msg(buf);
    stp = get_dbl("Steep Base",&stp);
  
    if ( stp > MAX_MM ) stp = diopt_to_mm(TEAR_INDEX,stp);  
    stp = round(stp);

    if ( stp > flt )
      {tmp=stp;stp=flt;flt=tmp;}
  }
  help_msg("");
  l->t.steep_radius = stp;
  l->t.flat_radius = flt;
}

/*
* s w a p d  ( a, b ) [static void]
*
* Parameters :
*   a : double *
*   b : double *
*
* Purpose : swap a & b.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Tue Apr 11, 1989; 11:15 am
*/
static void swapd(a,b)
double *a,*b;
{
  double swap_tmp;

  swap_tmp = *a;
  *a = *b;
  *b = swap_tmp;
}
