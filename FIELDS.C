#include "lens.h"

/*
* d u m p _ f i e l d s  ( filename, l ) 
*
* Parameters :
*   filename : string specifying output file name for ascii delimitted file
*   l : lens pointer to structure.
*
* Purpose : dump fields to filename in ascii delimitted format
*           basically for use by dbase/... format.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Modified: 09:14:00 10/19/90
* 1.4
*/
dump_fields(filename,l)
char *filename;
LENS *l;
{
  FILE *fp;

  if ( (fp = fopen(filename,"w")) == NULL )
    return;

  if ( l->lens_type & LT_LENTIC ) {
    fprintf(fp,"Replace LENT_OZ with \"%5.2f\"\n",l->lt_oz);
    fprintf(fp,"Replace LENT_JT with \"%5.2f\"\n",l->lt_jt);
    fprintf(fp,"Replace THICK with \"%4.2f/%4.2f\"\n",
      round(l->center_thick),round(l->lt_et) );
    goto dump_end;
  }
  if ( l->lens_type & (LT_DECARLE|LT_DDECARLE|LT_TRI_DD) ) {
    if ( l->lens_type & LT_TRI_DD ) {
      fprintf(fp,"Replace BC with \"%4.2f\"\n",
        round(l->itrm.trial_rad) );
    } else {
      fprintf(fp,"Replace BC with \"%4.2f/%4.2f\"\n",
        round(l->near_rad),round(l->dist_rad));
    }

    if ( l->lens_type & LT_TRI_DD ) {
			if ( l->lens_type & LT_TORIC_FRONT ) {
    		fprintf(fp,"Replace POWER with \"%5.2f%5.2fx%g\"\n", 
      		l->itrm.trial_pow,l->t.cyl,l->t.axis);
      } else {
				fprintf(fp,"Replace POWER with \"%+4.2f;%+3.1f,%+3.1f\"\n",
        	round(l->itrm.trial_pow),round(l->itrm.add_pow),
        	round(l->dist.add_pow) );
			}
      fprintf(fp,"Replace DIAM with \"%5.2f/%4.2f\"\n",
        l->diameter[0],(l->diameter[0] - l->itrm.truncate ));

    } else {
      fprintf(fp,"Replace POWER with \"%+4.2f=%+3.1f/%+3.1f\"\n",
        round(l->trial_pow),round(l->near_pow),round(l->dist_pow) );
    }


    if ( l->lens_type & LT_TRI_DD ) {
      fprintf(fp,"Replace OZ with \"%3.1f/%3.1f,%3.1f\"\n",
        l->diameter[l->rings-1],l->itrm.dist_oz,l->dist.dist_oz);
      fprintf(fp,"Replace BIF_SHT with \"%3.1f/%3.1f\"\n",
                  l->itrm.seght,l->dist.seght);
    } else {
      fprintf(fp,"Replace OZ with \"%5.2f/%4.2f\"\n",
        l->diameter[l->rings-1],l->oz);
      fprintf(fp,"Replace BIF_SHT with \"%5.2f\"\n",l->seght);
    }

    if ( l->trunc > 0 ) {
      fprintf(fp,"Replace DIAM with \"%5.2f/%4.2f\"\n",
        l->diameter[0],(l->diameter[0] - l->trunc));
    }

    /* thickness */
    fprintf(fp,"Replace THICK with \"%4.2f/",round(l->center_thick) );
    if ((l->extra_flags & EXRA_PRISM) || 
        (l->lens_type & (LT_TRI_DD | LT_DDECARLE) )) {
      fprintf(fp,"%4.2f\"\n",round(l->min_et)); 
      if ( l->lens_type & LT_TRI_DD ) {
        fprintf(fp,"Replace BAL_PRISM with \"%5.2f\"\n",
          round(l->itrm.ballast/0.16) );
      } else {
        fprintf(fp,"Replace BAL_PRISM with \"%5.2f\"\n",
          round(l->bal_prism/0.16) );
      }
    } else {
      fprintf(fp,"%4.2f\"\n",round(l->edge_thick)); 
    }
    /* end thickness */

    goto dump_end;
  }

  if ( l->lens_type & LT_TORIC_FRONT ) {
    fprintf(fp,"Replace POWER with \"%5.2f%5.2fx%g\"\n", 
      l->power,l->t.cyl,l->t.axis);
    if ( l->trunc > 0 ) {
      fprintf(fp,"Replace DIAM with \"%5.2f/%4.2f\"\n",
        l->diameter[0],(l->diameter[0] - l->trunc));
    }
    fprintf(fp,"Replace THICK with \"%4.2f/%4.2f\"\n",
      round(l->center_thick),round(l->min_et) );
    fprintf(fp,"Replace BAL_PRISM with \"%5.2f\"\n",
        round(l->bal_prism/0.16) );
    goto dump_end;
  }

  if ( l->extra_flags & EXRA_PRISM ) {
    fprintf(fp,"Replace THICK with \"%4.2f/%4.2f\"\n",
      round(l->center_thick),round(l->min_et) );
    fprintf(fp,"Replace BAL_PRISM with \"%5.2f\"\n",
        round(l->bal_prism/0.16) );
    goto dump_end;
  } 

  fprintf(fp,"Replace THICK with \"%4.2f/%4.2f\"\n",
    round(l->center_thick),round(l->edge_thick) );

dump_end:
  fclose(fp);
  return;
}
