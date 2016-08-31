#include "lens.h"
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/

/* mod_et.c 5.3 6/16/89 */

#define SMALL .001

mod_et(l)
register LENS *l;
{
        extern double fabs(double );
        float tmp_val;
        float diff;

        l->edge_thick = get_float("New Edge Thickness ",&(l->edge_thick));
        l->pref = ET_PREF;
#ifdef XXXX
        do {
                diff = fabs(tmp_val - (l->edge_thick)) /* /2.0 */ ;
                if ( diff <= SMALL ) diff = SMALL;

                if ( tmp_val > (l->edge_thick) ) 
                        l->center_thick += diff;
                else
                        l->center_thick -= diff;
                lenscalc(l);
#ifdef DEBUG
                display_lens(l);
#endif
        } while ( fabs(tmp_val - (l->edge_thick)) > .0005 ) ;
#endif
}
