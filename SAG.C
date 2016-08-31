#include <math.h>
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

