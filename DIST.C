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
#include "point.h"

/* dist.c 5.3 6/16/89 */

double
dist(p1,p2)
POINT *p1,*p2;
{
        double result;
        double x_delta, y_delta;

        x_delta = p1->x - p2->x;
        y_delta = p1->y - p2->y;
        result = sqrt( pow(x_delta,2.0) + pow(y_delta,2.0) );

        return result;
}
