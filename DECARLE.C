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

/*
* d e c a r _ p o w  ( mat_index, dc ) 
*
* Parameters :
*   mat_index : material index (example 1.505)
*   dc : pointer to DECARLE_DATA record.
*
* Purpose : calculate near/distance powers and radius.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Tue Aug 21, 1990; 08:35 am
*/
void decar_pow(mat_index,dc)
double mat_index;
DECARLE_DATA *dc;
{
  double tmpi;
  double m_diopt;
  double e_diopt;
  double induced_pow;

  /*
  * Calculate induced power caused by the difference in indexes 
  * (index of material and index of tear) and because of the 
  * required add power.
  */
  tmpi        = (mat_index - (double)1.0)*(double)1000.0;  
  m_diopt     = mm_to_diopt(tmpi,(double)8.0) - 
                mm_to_diopt(tmpi,(double)10.0);
  e_diopt     = mm_to_diopt(TEAR_INDEX,(double)8.0) - 
                mm_to_diopt(TEAR_INDEX,(double)10.0);
  induced_pow = (dc->add_pow) / ((m_diopt - e_diopt)/e_diopt);

  /*
  * Now calculate distance/near powers and radius using this induced_pow
  */
  if ( dc->trial_rad < MAX_MM )
    dc->trial_rad = mm_to_diopt(TEAR_INDEX,dc->trial_rad);

  dc->dist_pow = dc->trial_pow - induced_pow;

  dc->dist_rad = dc->trial_rad+induced_pow;
  dc->dist_rad = diopt_to_mm(TEAR_INDEX,dc->dist_rad);

  dc->near_pow = dc->trial_pow + dc->add_pow;
  dc->near_rad = diopt_to_mm(TEAR_INDEX,dc->trial_rad);
}

/*
* d e c a r _ d e c  ( dc ) 
*
* Parameters :
*   dc : pointer to DECARLE_DATA structure.
*
* Purpose : calculate decenter amount,prism,.... values required
* to manufacture decentered decarle lens specified by 'dc'.
*
* Globals : n.a. 
*
* Returns : updated DECARLE_DATA.
* Date : Wed Aug 22, 1990; 08:08 am
*/
void decar_dec(dc)
DECARLE_DATA *dc;
{
  double fr,fx,fy, sr,sx,sy;
  double zero_x,zero_y, rot_radius, theta;
  double cx,cy;
  double tmp;

  dc->seght = dc->i_seght;
  dc->decenter = (dc->dist_oz/2.0) - 
  ( (dc->near_oz/2.0) - dc->seght - dc->truncate 
    + dc->ballast + dc->blocking );

  if ( fabs(dc->decenter) > 0.01 ) {
    /*
    * Initialize center and radius of flat/steep radius.
    * Start values would give zero decenter value.
    *
    * fr = radius value, fx,fy = center of circle
    * sr = radius value, sx,sy = center of circle
    */
    fr = dc->near_rad;
    fx = 0.0;
    fy = 0.0;
  
    sr = dc->dist_rad;


    sx = fr - sr + (sag(sr,dc->dist_oz) - sag(fr,dc->dist_oz));
    sy = 0.0;
  
    /*
    * Store zero position which is where steep radius is positioned 
    * before decentration occures.  Decentration occures by re-position
    * of steep radius (center) on rot_radius circle.
    */
    zero_x      = sx;
    zero_y      = sy;
    rot_radius  = sx;
  
    theta       = asin(dc->decenter/fr);
    sx          = rot_radius * cos(theta);
    sy          = rot_radius * sin(theta);
  
    dc->prism   = sqrt(pow(zero_x - sx,2.0)+pow(zero_y - sy,2.0));
    dc->touch   = (fr * cos(theta)) - (sr * cos(theta));
    dc->tch_add = (sr * cos(theta) + sx) - (fr * cos(theta));
    dc->tch_ang = sy/sx;
  } else { 
    /* 
    * decenter amount, very small.  This special case is where
    * the decenter amount is basically 0.  
    */

    fr = dc->near_rad;
    fx = 0.0;
    fy = 0.0;

    sr = dc->dist_rad;
    sx = fr - sr + (sag(sr,dc->dist_oz) - sag(fr,dc->dist_oz));
    sy = 0.0;

    theta       = 0.0;

    dc->prism   = 0.0;
    dc->touch   = (fr * cos(theta)) - (sr * cos(theta));
    dc->tch_add = (sr * cos(theta) + sx) - (fr * cos(theta));
    dc->tch_ang = 0.0;
  }
}

#ifndef STAND
/*
* d e c a r _ p _ t r i  ( l ) 
*
* Parameters :
*   l : pointer to lens.
*
* Purpose : print decentered decarle TRI-focal manufacturing information.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Aug 23, 1990; 07:38 am
*/
void decar_p_tri(l,fp)
LENS *l;
FILE *fp;
{
  extern void outline(char *,FILE *);
  extern void nextline(FILE *);
  extern char  *pwr_fmt(double);

  nextline(fp); /* separation line */

  if ( l->itrm.truncate > 0.0 )
    print_trunc(l,l->itrm.truncate,fp);

  outline("Decentered Decarle Trifocal: ", fp); 
  nextline(fp);

  outline("Lensometer :",fp);
  outline(pwr_fmt(l->dist.dist_pow),fp);
  outline(",",fp);
  outline(pwr_fmt(l->itrm.dist_pow+l->dist.add_pow),fp);
  outline(",",fp);
  outline(pwr_fmt(l->itrm.near_pow+l->dist.add_pow),fp);
  outline("(Trial ",fp);
  outline(pwr_fmt(l->itrm.trial_pow),fp);
  outline(")",fp);
  nextline(fp);

  sprintf(buffer,"Seg Height :%7.2lf,%7.2lf ",
    l->dist.seght,l->itrm.seght);
  outline(buffer,fp);
  nextline(fp);

  outline("On Eye     :",fp);
  outline(pwr_fmt(l->itrm.trial_pow),fp);
  outline(",",fp);
  outline(pwr_fmt(l->itrm.trial_pow + l->itrm.add_pow),fp);
  outline(",",fp);
  outline(pwr_fmt(l->itrm.trial_pow + l->itrm.add_pow + l->dist.add_pow),fp);
  nextline(fp);
}
#endif


#ifdef STAND
#include <math.h>

DECARLE_DATA *ptr;
void decar_pow(double,DECARLE_DATA *);
void decar_dec(DECARLE_DATA *);

main()
{
  char buf[100];
  double ary[10];
  double index;
  int i,j;

  if ( (ptr = (DECARLE_DATA *)malloc(sizeof(DECARLE_DATA))) == NULL ) {
    exit();
  }

  i = 0;
  while ( !feof(stdin) ) {
    if ( gets(buf) != NULL ) {
      ary[i] = atof(buf);    
      printf("Input String = (%s)\n",buf);
      printf("value = %g\n",ary[i]);
      i = i + 1;
    }
  }
  if ( i > 8 ) {
    j = 0;
    ptr->trial_pow = ary[j++];
    ptr->trial_rad = ary[j++];
    ptr->add_pow   = ary[j++];
    ptr->dist_oz   = ary[j++];
    ptr->near_oz   = ary[j++];
    ptr->i_seght   = ary[j++];
    ptr->truncate  = ary[j++];
    ptr->ballast   = ary[j++];
    ptr->blocking  = ary[j++];
    index         = ary[j++];

    decar_pow(index,ptr);
    decar_dec(ptr);

    printf("Input : \n\n");fflush(stdout);
    printf("Trial Radius  \t%g\n",ptr->trial_rad);
    printf("Add Power     \t%g\n",ptr->add_pow);
    printf("Dist O.Z.     \t%g\n",ptr->dist_oz);
    printf("Near O.Z.     \t%g\n",ptr->near_oz);
    printf("Seg Height    \t%g\n",ptr->i_seght);
    printf("Truncation    \t%g\n",ptr->truncate);
    printf("Ballast prism \t%g\n",ptr->ballast);
    printf("Blocking Prism\t%g\n",ptr->blocking);
    printf("Index         \t%g\n",index);
    printf("\n-----------------------------------------\n\n");fflush(stdout);
    printf("prism         \t%g\n",ptr->prism);
    printf("Touch         \t%g\n",ptr->touch);
    printf("Touch Add     \t%g\n",ptr->tch_add);
    printf("Touch Angle   \t%g\n",ptr->tch_ang);
    printf("Decenter      \t%g\n",ptr->decenter);
    printf("Dist Rad      \t%g\n",diopt_to_mm(TEAR_INDEX,ptr->dist_rad));
    printf("Near Rad      \t%g\n",diopt_to_mm(TEAR_INDEX,ptr->near_rad));
    printf("\n-----------------------------------------\n\n");fflush(stdout);

  }
  else
    printf("\nInvalid Input\n");fflush(stdout);
}

/* ------- sag.o */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/

/* sag.c 5.4 6/16/89 */

extern double sqrt(double);
extern double pow(double,double);

/* s a g ( radius, diameter )
**
** return sag value of radius at diameter.
** method :
*/
double
sag(r,d)
double r;        /* radius of curve */
double d;        /* diameter at sag-value */
{
        return (double)(r - sqrt(r*r - pow((d/2.0),2.0)));
}

/* a s a g _ d ( radius, sagvalue )
**
** return diameter value of radius at sagvalue given.
*/
double
asag_d(r,sag)
double r,sag;
{
  return sqrt(4.0*r*r - 4.0*pow(r-sag,2.0));
}

/* a n t i _ s a g ( sagvalue, diameter)
**
** return radius value of diameter at sagvalue given.
*/
double
anti_sag(sagvalue, diameter)
double sagvalue;
double diameter;
{
        return ( (pow(diameter,2.0)/(4.0 * sagvalue) + sagvalue) / 2.0 );
}

/* a s a g _ r ( diameter, sagvalue )
**
** return radius value of diameter at sagvalue given.
*/
double
asag_r(d,sag)
double d,sag;
{
  return anti_sag(sag,d);
}

/* ------- math01.o */
/* @(#) math01.c 1.4@(#) 6/16/89 */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
#ifndef M_PI
# define M_PI  3.14159265358979323846
#endif

/* List of procedures/functions math01.c
* square( a ) 
* round( x ) 
* round_001( x ) 
* mm_to_diopt( index, mm ) 
* diopt_to_mm( index, diopters ) 
* rad_to_deg( x ) 
* deg_to_rad( x ) 
*/

/*
* s q u a r e  ( a ) 
*
* Parameters :
*   a : simple double value.
*
* Purpose : return a^2, a*a. 
*
* Globals : n.a. 
*
* Returns : a squared.
* Date : Fri Mar 31, 1989; 09:54 am
*/
double square(a)
double a;
{
  return a*a;
}

/*
* r o u n d  (195 lines
 x ) 
*
* Parameters :
*   x : double, value to round.
*
* Purpose : round x, to 2 places after decimal. 
*           I.E. input value = 2.23456 -> output = 2.23000,
*           value is properly rounded up at '5' and truncated at
*           values < '5' (49999999).
*
* Globals : n.a. 
*
* Returns : rounded x value
* Date : Fri Mar 31, 1989; 09:57 am
*/
double round(x)
double x;
{
  double tmp;

  x = x * 100.0;
  tmp = floor(x);
  if ( x-tmp >= .5)
    x = tmp + 1.0;
  else
    x = tmp;
  return x/100.0;
}

/*
* r o u n d _ 0 0 1 ( x ) 
*
* Parameters :
*   x : double, value to round.
*
* Purpose : round x, to 3 places after decimal. 
*           I.E. input value = 2.23456 -> output = 2.23500,
*           value is properly rounded up at '5' and truncated at
*           values < '5' (49999999).
*
* Globals : n.a. 
*
* Returns : rounded x value
* Date : Wed Apr 12, 1989; 11:49 am
*/
double round_001(x)
double x;
{
  double tmp;

  x = x * 1000.0;
  tmp = floor(x);
  if ( x-tmp >= .5)
    x = tmp + 1.0;
  else
    x = tmp;
  return x/1000.0;
}

/*
* m m _ t o _ d i o p t  ( index, mm ) 
*
* Parameters :
*   index : refractive index ( 1.0 - ref.index * 1000.0 )
*   mm : radius of curvature in mm.
*
* Purpose : Convert mm radius to diopters relative to given index.
*
* Globals : n.a. 
*
* Returns : Converted value.
* Date : Fri Mar 31, 1989; 10:02 am
*/
double mm_to_diopt( index, mm )
double index;    /* in the form 505 not 1.505 */
double mm;       /* must be changed to meters */
{
        double diopters;         /* return value */
        double m;                /* equiv to mm but in meters */

        index = index / 1000.0;
        m     = mm / 1000.0;

        diopters = index / m ;
        return diopters;
}

/*
* d i o p t _ t o _ m m  ( index, diopters ) 
*
* Parameters :
*   index : refractive index ( 1.0 - ref.index * 1000.0 )
*   diopters : radius of curvature in diopters.
*
* Purpose : convert diopter radius to mm radius relative to given index.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Fri Mar 31, 1989; 10:05 am
*/
double diopt_to_mm( index, diopters )
double index;    /* in the form 505 not 1.505 */
double diopters;
{
        double m,mm;

        index = index / 1000.0;
        m = index / diopters;
        mm = m * 1000;
        return mm;
}

/*
* r a d _ t o _ d e g  ( x ) 
*
* Parameters :
*   x : radian angle
*
* Purpose : convert radian angle to degree angle.
*
* Globals : n.a. 
*
* Returns : degrees of given radian value.
* Date : Fri Mar 31, 1989; 10:08 am
*/
double rad_to_deg(x)
double x;
{
        return ( x * (180.0/M_PI) );
}

/*
* d e g _ t o _ r a d  ( x ) 
*
* Parameters :
*   x : degree angle
*
* Purpose : convert degree angle to radian angle.
*
* Globals : n.a. 
*
* Returns : radians of given radian value.
* Date : Fri Mar 31, 1989; 10:16 am
*/
double deg_to_rad(x)
double x;
{
        return ( x * (M_PI/180.0) );
}
#endif
