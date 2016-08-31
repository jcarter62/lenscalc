/* math.c 1.3 6/16/89 */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
#include <stdio.h>
#include <math.h>

/*
* m a t h e r r  ( x ) 
*
* Parameters :
*   x : exception sent from math routine cos(),sin(),...
*
* Purpose : handle math error.  In this case below, pow(-x,y) generates
*           a DOMAIN error on the "cc -O -Mm0d -CSON -K -dos -Ddos".  
*           --- This is used only in the dos version ---
*
* Globals : n.a. 
*
* Returns : The correct value for pow(). given the correct error type 
*           see below.  All other error types return 0, and proceed as if
*           this math error routine didn't exist.
* Date : Fri Mar 24, 1989; 08:03 am
*/
int matherr(x)
register struct exception *x;
{
	if ( x->type == DOMAIN ) {
		if ( strcmp(x->name,"pow") == 0 ) {
			if ( x->arg1 < 0.0 ) {
				x->retval = pow(-(x->arg1),x->arg2);
				return (1);/* OK, continue with no error */
			}
		}
	}
	return (0); /* all other errors proceed normally */
}
