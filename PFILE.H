#ifdef __TURBOC__
# include "loc_pr.h"
#else
# include "/usr/lib/lens/loc_pr.h"
#endif
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/

typedef enum {to_file,to_paper} P_DEST;
extern P_DEST print_dest;

extern FILE *open_pfile();
extern void close_pfile();

