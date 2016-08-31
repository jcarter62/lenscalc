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
#include <math.h>
#ifndef M_PI
# define M_PI	3.14159265358979323846
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
* r o u n d  ( x ) 
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
