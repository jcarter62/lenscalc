/* @(#) lens.h 5.29@(#) 8/21/90 07:52:49 */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
/* 
* define 'dos' if running eco-c88 compiler 
*/
#ifndef dos
# ifdef __ECO|MSDOS
#  define dos
   extern double atof(char *);
   extern int atoi(char *);
# endif
#endif

/* include the other system include files */
/* ------------ */
#ifndef stdin
# include <stdio.h>
#endif
/* ------------ */
#ifndef bool
# include <curses.h>
#endif
/* ------------ */
#ifndef HUGE_VAL
# include <math.h>
#endif

/* s a g . c */
extern double sag(double , double );
extern double anti_sag(double ,double );
extern double anti_sag_iteration(double ,double );

/* m a t h 0 1 . c */
extern double square(double);
extern double round(double);
extern double mm_to_diopt(double , double );
extern double diopt_to_mm(double , double );
extern double rad_to_deg(double);
extern double deg_to_rad(double);

/* i w . c */
extern int get_int(char *,int );
extern double get_float(char *,double *);
extern double get_dbl(char *,double *);

#define TEAR_INDEX      337.5
#define MAX_MM          30.0
#define MAX_RINGS       6


typedef struct { /* (sph cyl axis) data */
	double sph,cyl,axis;
	double x,y;
} VECTOR;

typedef struct { /* toric lens data */

	VECTOR	v1,v2,v3;	/* vectors used in +cyl with axis */

	double  pow_flat,	/* toric power in flat merid. */
		pow_steep;	/* toric power in steep merid. */
	/* 
	* toric secondary information needed to determine
	* dimensions and radius at axis != (0 or 90)
	*/
	double	ioz_axis,	/* inner oz at a given axis */
		ooz_axis,	/* outer oz at a given axis */
		rad_axis,	/* radius at given axis */
		et_axis,	/* edge thickness at given axis */
		jt_axis;	/* junction thickness at axis */

	double  cyl,res_cyl,	/* toric front data */
		axis_fm,axis;	/* axis of base/flat meridian */
				/* toric secondary data */
	double	steep_radius,	/* this will be base + 1.0 mm (rounded) */
		flat_radius,	/* input */
		oz_flat,	/* input */
		oz_steep,
		ooz_flat,	/* toric secondary flat outer oz */
		ooz_steep,	/* toric secondary steep outer oz */
		jt_flat,	/* used if lenticular only */
		jt_steep,
		et_flat,	/* both et should be */
		et_steep;	/* at least a .1 edge */
		/* 
		* The following data is a copy of the front surface
		* data for the flat meridian used to cut the front
		* surface.
		*/
	double	lt_ca,		/* cap angle */
		lt_ld,		/* lower dial */
		lt_et,		/* lenticular edge */
		lt_oz,		/* oz */
		lt_rad,		/* lenticular radius */
		lt_set,		/* set value */
		lt_tkoff,	/* take off */
		lt_jt;		/* junction thickness */

	double	save_et,	/* saved value of desired et. */
		save_jt,	/* same of junction thickness */
		save_lt_et;	/* Saved value of lenticular edge thick */

	int     tor_type;	/* TOR_SPH/TOR_CYL defines bellow */

} TORIC_DATA;

typedef struct { /* toric lens data for round o.z. */

	double	diam[MAX_RINGS],
		rad[MAX_RINGS];
	int rings;
} TORIC_ROUND;

typedef struct { /* decentered decarle data */
	double trial_pow;	/* trial lens power (Sphere power) */
	double trial_rad;	/* trial lens radius in mm */
	double add_pow;		/* add power required on top of trial lens */

	double dist_oz;		/* size of oz at distance */
	double dist_pow;	/* calculated distance power */
	double dist_rad;  /* calculated distance radius in mm */

	double near_oz;		/* size of oz at near */
	double near_pow;	/* calculated near power */
	double near_rad;  /* calculated near radius in mm */

	double truncate;	/* truncation off bottom of lens */
	double seght;			/* seg height after truncation */
	double i_seght;		/* seg height after truncation */
	double ballast;		/* ballast prism in mm */
	double blocking;	/* blocking prism in mm */

	/* calculated values used for manufacturing */
	double decenter;	/* offset distance in mm required */
	double prism;			/* offset in mm required to produce seght */
	double touch;			/* distance required to touch cutting tool */
										/* to button after near curve */
	double tch_add;		/* amount (distance) to remove after touch */
	double tch_ang;		/* angle at which touch will occure */
	double bellow;		/* seg height relative to center of lens */

	/* rotation information */
	int rotation;			/* see ROT_* defines */
	double r_seght;		/* seg height after rotation applied. */
	double r_dprism;	/* down prism */
	double r_sprism;	/* side prism */

} DECARLE_DATA;
/*
* defines required for above 'int rotation' */
#define ROT_NONE	0
#define ROT_RIGHT	1
#define ROT_LEFT	2
#define ROT_OZ		4
#define ROT_PRISM	8
#define ROT_IN		16
#define ROT_OUT		32


typedef struct {
  int      rings;                  /* input : # of curves */
  double   radius[MAX_RINGS];      /* radius of each curve */
  double   diameter[MAX_RINGS];    /* diam (oz) of each curve */
  double   rad_dif[MAX_RINGS];     /* radius[i] - radius[i-1] */
  double   sag_dif[MAX_RINGS];
  double   sag_plus_rad[MAX_RINGS];
  double   sag_sum[MAX_RINGS];
  double   sag_front;              /* sag @ front_radius at lens size */
  double   front_radius;
  double   center_thick;
  double   edge_thick;
	double   min_et;	/* used with EXRA_PRISM */
  double   power;
  double   index;
	char     mat_type[20];

	DECARLE_DATA   itrm,dist;
  /* decarle data */
  double   trial_pow,trial_rad;
  double   add_pow;
  /* decarle results */
	double   near_rad,near_pow;
	double   dist_rad,dist_pow;
  /* decenter decarle data */
  double   oz, seght, lens_size, trunc, bal_prism, blk_prism;
  /* results */
  double   decenter, prism, touch, tch_add, tch_angle, bel_val;
	/* if decentered decarle, then also if rotated in/out */
	double   rotang;	/* angle of rotation */
	char     rot_rl;	/* right/left lens */
	char     rot_io;	/* in/out rotation */
	double   rot_sht;	/* seg hight after rotation */
	double   rot_sidep;	/* side prism */
	double   rot_downp;	/* down prism */

  /* lenticular data */
  double  cap_ang;        /* cap angle used on chase */
  double  lower_dial;     /* radius difference used on chase */
  double  lt_et;          /* lenticular edge thickness */
  double  lt_oz;          /* lenticular o.z. */
  double  lt_rad;         /* lenticular radius (flange rad) */
  double  lt_set;         /* set value */
  double  lt_tkoff;       /* take off value */
  double  lt_jt;

  /* plateau data */
  double  cap_ang_p;      /* angle used on chase */
  double  lower_dial_p;   /* radius difference used on chase */
  /* extra1l = amout to touch after setting lower dial and upper */
  /* extra2l = additional amout to cut after extra1l */

	TORIC_DATA t;		/* see above */
	TORIC_ROUND round;	/* see above */

	/* Aspheric lens information */
	double  edge_lift;	
	double  a_angle,a_offset,a_rad;
	double  ld_dif,a_el_tan,a_outer_oz;

  double  extra1l,extra2l;
  double  extra3l,extra4l;

	/* thickness for minimum button thickness */
	double  min_bt;

  int mod_flag;
  int lens_type;
	int	extra_flags;
  int pref;
} LENS;


/* preferences ct/et */
#define CT_PREF         1
#define ET_PREF         2
#define JT_PREF		3

/* extra flags definitions */
#define EXRA_NONE	0
#define EXRA_PRISM	1

/* lens types */
#define LT_REG          0
#define LT_LENTIC       1
#define LT_DECARLE      2
#define LT_TORIC_FRONT	4
#define LT_PLATEAU      8
#define LT_OP_CRES	16
#define LT_DDECARLE	32
#define LT_TOR_SEC	64
#define LT_TOR_OOZ	128
#define LT_TOR_ROZ	256
#define LT_DECENTER	512
#define LT_ASPHERIC	1024
#define LT_TRI_DD		2048

/* Modify flags */
#define MF_ALL          0
#define MF_RINGS        1
#define MF_CT           2
#define MF_ET           3
#define MF_POW          4
#define MF_INDEX        5

/* bitoric types. */
#define TOR_SPH_F	0
#define TOR_CYL_F1	1
#define TOR_CYL_F2	2
#define TOR_CYL_F3	3
#define TOR_CYL_F4	4

extern int DEBUG;
extern FILE *dbg;


/* to take care of test input etc. */
/* routines should be found in get.c */
#ifndef local_getch

# ifdef getch
#  undef getch
# endif
# define getch local_getch

# ifdef getstr
#  undef getstr
# endif
# define getstr local_getstr

extern int local_getch();
extern char *local_getstr(char *);

#endif /* local_getch */

extern char help_buf[];	/* used to store help message */

extern char buffer[100];
extern char buffer1[100];
extern double dbl_tmp,dbl_tmp1,dbl_tmp2;

extern char *WORKING,*NOT_WORKING;

#ifndef __TURBOC__
	extern double atof(char *);
	extern char *strdup(char *);
#endif
