/* @(#) mod_mat.c 5.18@(#) 8/29/90 09:21:17 */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
#ifdef __TURBOC__

/* globals */
char *mat_name;		/* material name */
char *mat_bfvp;		/* bvp or fvp power indicator. */
double mat_index;	/* material index. */
double mat_cost;	/* Base price of material per lens. */

int mod_mat() { return 0; }
double minimum_ct(mat_name,pow) char *mat_name; double pow; { return 0.15; }
double minimum_et(mat_name,pow) char *mat_name; double pow; { return 0.10; }
double minimum_jt(mat_name,pow) char *mat_name; double pow; { return 0.15; }

#else

/* #include <fcntl.h> */
#include "lens.h"
#include <ctype.h>
#include <memory.h>
#include <malloc.h> 
#include "pfile.h"
#include <string.h>

/* List of procedures/functions mod_mat.c
* mod_mat(  )  [global] 
* read_mat_file( m ) [static, int]
* write_mat_file( m, n ) [static void]
* display( m, n ) [static int]
* pick_cv_cc( i ) [static ]
* add_mat( n ) [static ]
* change_mat( n ) [static ]
* mat_pick( k ) [static ]
* stp_info( k ) [static ]
* delete_mat( n ) [static ]
* tolower_s( s ) [static ]
* sort( ary, n ) [static ]
* matcomp( a, b ) [static ]
* in( c, s ) [static int]
* lookup_material( name, power ) [static struct stp *]
* minimum_ct( mat_name, pow ) [global double]
* minimum_et( mat_name, pow ) [global double]
* minimum_jt( mat_name, pow ) [global double]
*/

#ifdef dos
#define MATFILE "MAT.DAT"

/* save_page used for quick page updates */
static char *save_page[5];
extern char *save_screen(char *);
extern void restore_screen(char *);

#else
#define MATFILE "/usr/lib/lens/mats.dat"
#endif
#define next() {move(cur_row++,0);clrtoeol();}

struct stp {
	double min_pow;
	double max_pow;
	double min_et;
	double min_ct;
	double min_jt;
	double steepen;
	struct stp *ptr;
};

struct mat {
	char *name;
	double cvup;
	double ccup;
	double cost;		/* base price for material tri-curve sph */
	struct stp *ptr;
};

static struct mat *m = (struct mat *)NULL;
static struct stp *list[20];
extern int cur_row;
static page = 0;	/* 0,1,2,... 20 materials per page */
static int n;		/* number of materials */
static modified = 0;	/* False, not modified */

/* globals */
char *mat_name;		/* material name */
char *mat_bfvp;		/* bvp or fvp power indicator. */
double mat_index;	/* material index. */
double mat_cost;	/* Base price of material per lens. */

#define MAX_MATERIALS 100


/*
* m o d _ m a t  (  )  [global] 
*
* NO-Parameters :
* Purpose : modify/pick/... materials from material data base.
*
* Globals : 
*       n : number of entries in material data base.
*       m : matierial data base entries.
*
* Returns : Ok/Error (1/0)
* Date : Mon Mar 27, 1989; 02:39 pm
*/
int
mod_mat()
{
	FILE *fp;
	int i;

	/* 
	* If this is the first time, m == NULL.
	* We need to get enough memory for MAX_MATERIALS without
	* (struct stp) stuff.
	*/
	if ( m == (struct mat *)NULL ) 
		read_mat_file();

	n = display(n);

	if ( n < 0 ) return 0;	/* error */
	if ( modified ) {
		clear();
		move(LINES/2,COLS/2-12);
		attron(A_BLINK);
		printw("--Writing Material File--");
		attroff(A_BLINK);
		refresh();
		write_mat_file(n);
		clear();
	}
	return 1;		/* o.k. */
}


/*
* r e a d _ m a t _ f i l e  ( m ) [static, int]
*
* Parameters :
*   m : material data array.
*
* Purpose : Read materials data base into memory, and return 
*           the number of entries.
*
* Globals : n.a.
*
* Returns : number of entries in data base. Or -1 if error.
* Date : Mon Mar 27, 1989; 02:39 pm
*/
static int
read_mat_file()
{
	extern struct mat *m;
	FILE *fp;
	struct stp *p,*endp;
	register int i;

	clear();
	move(LINES/2,COLS/2-12);
	attron(A_BLINK);
	printw("--Reading Material File--");
	attroff(A_BLINK);
	refresh();
	m = (struct mat *)malloc(sizeof(struct mat)*MAX_MATERIALS);
	if ( m == NULL ) return 0;	/* error */
	for (i=0;i<MAX_MATERIALS;i++) {
		m[i].name = (char *)NULL;
		m[i].ptr = (struct stp *)NULL;
	}
#ifdef dos
	/* start saved pages to not saved. */
	for (i=0;i<5;i++)
		save_page[i] = NULL;
#endif
	if ( ( fp = fopen(MATFILE,"r") ) == (FILE *)NULL ) {
		perror("Read material file");
		resetty();
		exit();
	}
	i = 0;
	while ( fgets(buffer,100,fp) != (char *)NULL ) {	/* read mat name */
		buffer[strlen(buffer)-1] = '\0';	/* delete new line */
		m[i].name = (char *)malloc((unsigned)strlen(buffer)+1);
		strcpy(m[i].name,buffer);
		m[i].ptr = (struct stp *)NULL;
#ifdef dos
		fscanf(fp,"%lf : %lf : %lf\n",
#else
		fscanf(fp,"%lg : %lg : %lg\n",
#endif
			&(m[i].cvup),&(m[i].ccup),&(m[i].cost));
		do {
			fgets(buffer,100,fp);
			buffer[strlen(buffer)-1] = '\0';	/* delete new line */
			if ( strncmp(buffer,":::::",4) != 0 ) {
				p = (struct stp*)malloc(sizeof(struct stp));
				if ( p == NULL ) return -1;
				p->ptr = (struct stp *)NULL;
				if ( m[i].ptr == (struct stp *)NULL ) {
					m[i].ptr = p;
					endp = p;
				} else {
					endp->ptr = p;
					endp = p;
				}
#ifdef dos
			sscanf(buffer,"%lf : %lf : %lf : %lf : %lf : %lf :",
#else
			sscanf(buffer,"%lg : %lg : %lg : %lg : %lg : $lg :",
#endif
				&(p->min_pow),
				&(p->max_pow),
				&(p->min_et),
				&(p->min_ct),
				&(p->min_jt),
				&(p->steepen));
			}
		} while ( strncmp(buffer,":::::",4) );
		i++;
	}
	fclose(fp);

	clear();
	refresh();

	n = i;		/* the number of materials input from file */
	/* return i; */
}

/*
* w r i t e _ m a t _ f i l e  ( m, n ) [static void]
*
* Parameters :
*   m : material entries, data base to write to file.
*   n : number of entries in data base.
*
* Purpose : Write data base out to file, and make changes permanent.
*
* Globals : modified : flag = false after write to file.
*
* Returns : n.a. 
* Date : Mon Mar 27, 1989; 02:39 pm
*/
static write_mat_file(n)
register int n;
{
	extern struct mat *m;
	FILE *fp;
	struct stp *p;
	register int i;

	if ( (fp = fopen(MATFILE,"w")) == NULL ) {
		resetty();
		perror("Write material file");
		exit();
	}
#ifdef dos
	for (i=0;i<5;i++) {
		if ( save_page[i] != NULL ) {
			free(save_page[i]);
		}
		save_page[i] = NULL ;
	}
#endif

	for (i=0;i<n;i++) {
		fprintf(fp,"%s\n",m[i].name);
		fprintf(fp,"%lg : %lg : %lg\n",m[i].cvup,m[i].ccup,m[i].cost);
		p = (struct stp *)m[i].ptr;
		while (p != (struct stp *)NULL ) {
			fprintf(fp,"%lg : %lg : %lg : %lg : %lg : %lg :\n",
				p->min_pow,p->max_pow,p->min_et,p->min_ct,
				p->min_jt,p->steepen );
			p = p->ptr;
		}
		fprintf(fp,":::::\n");
	}
	fclose(fp);
	modified = FALSE;
}


/*
* d i s p l a y  ( m, n ) [static int]
*
* Parameters :
*   n : number of entries in material array.
*
* Purpose : display information on user's screen and allow user to
*           pick/change/add/delete.
*
* Globals : n.a. 
*
* Returns : Number of entries in array after user operates.
* Date : Mon Mar 27, 1989; 02:39 pm
*/
static int
display(n)
register int n;
{
	extern struct mat *m;
	int choice,c;
	int cur_row;
	int i;

	mat_name = NULL;
	mat_bfvp = NULL;
	mat_index = 1.000;

	while (1) {
#ifdef dos
	if ( save_page[page] != NULL ) {
	  restore_screen(save_page[page]);
	  move(LINES-2,0);
	  refresh();
	} else {
#endif
	clear();
	cur_row = 0;
	next();printw("Pick material type by entering # <%d - %d>",1,n);
	next();printw("or <a> - add, <c> - change, <d> - delete, <q> - quit");
	next();printw("<PRINT> or <Control-P> - Make Material List on Printer");
	if ( n > 20 )
		{ next(); printw(" or <space> for next page"); }
	next();
	next();printw("    Material Name           BVP      FVP  Base Cost");
	i=page*15;
	while ( i<n && i < (page+1)*15 ) {
		next();
		printw("%2d  %-20s %8.4f %8.4f %6.2f",
			i+1,m[i].name,m[i].cvup,m[i].ccup,m[i].cost);
		i++;
	}
	next();
	refresh();
#ifdef dos
	save_page[page] = save_screen((char *)NULL);
	}
#endif

	/* get user response */
	noecho();
	do c = getch(); while ( ! in(c,"acdq 0123456789\") );
	echo();

	if ( c == 'a' ) {add_mat(n);n++;modified++;}
	else if ( c == 'c' ) {change_mat(n);modified++;}
	else if ( c == 'd' ) {delete_mat(n);n--;modified++;}
	else if ( c == 'q' ) {clear();refresh();return(n);}
	else if ( c == ' ' ) {page++;if ( page*15 > n ) page = 0;}
	else if ( isdigit(c) ) {
		printw("%c",c);
		refresh();
		buffer[0] = c;
		echo();	
        	getstr(&(buffer[1]));
		noecho();
		choice = atoi(buffer);
		if ( choice > 0 && choice <= n ) { /* picked choice material */
			pick_cv_cc(choice-1);
			clear();refresh();
			return(n);
		}
	} else if ( c == '\' ) {
		FILE *pf;

		pf = open_pfile();

fprintf(pf,"    Material Name           BVP      FVP  Base Cost\n\n");
		i=0;
		while ( i<n ) {
			fprintf(pf,"%2d  %-20s %8.4f %8.4f %6.2f\n",
				i+1,m[i].name,m[i].cvup,m[i].ccup,m[i].cost);
			i++;
		}
		close_pfile();
	} 

	}	
}


/*
* p i c k _ c v _ c c  ( i ) [static ]
*
* Parameters :
*   i : material array index m[i]....
*
* Purpose : prompt user to pick BVP/FVP for given material m[i].name.
*
* Globals : 
*  mat_name : name of material
*  mat_bfvp : either "FVP" or "BVP"
*  mat_index: double, index of refraction for given material/[fb]vp.
*  mat_cost : base cost of this material / lens.
*
* Returns : n.a. 
* Date : Tue Jan  3, 1989; 09:40 am
*/
static pick_cv_cc(i)
int i;
{
	int c;

	mat_name = m[i].name;
	mat_cost = m[i].cost;
	clear();
	cur_row=0;
	next();
	next();printw("You have picked ");
	standout();
	printw("%s",m[i].name);
	standend();
	printw(" as the material.");
	next();
	next();printw("Please pick the index type,");
	next();
	next();printw("1 - BVP/CVUP  <Back Vertex> ");
	next();printw("2 - FVP/CCUP  <Front Vertex>");
	next();printw(" 1 or 2 or <return picks 1>");
	refresh();
	noecho();
	do {
		c = getch();
		if ( c == '1' || c == '\n' || c == '\r' ) {
			mat_index = m[i].cvup;
			mat_bfvp = " BVP";
		} else if ( c == '2' ) {
			mat_index = m[i].ccup;
			mat_bfvp = " FVP";
		}
	} while ( c!='1' && c!='\n' && c!='\r' && c!='2' );
}	


/*
* a d d _ m a t  ( n ) [static ]
*
* Parameters :
*   n : number of materials in list.
*
* Purpose : Add one material to current list.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:19 am
*/
static add_mat(n)
int n;
{
	int fd,i;

	echo();
	clear();
	cur_row=0;

	next();
	printw("Add Material to database");
	next();
	next();
	printw("Material Name :");refresh();
	getstr(buffer);
	bs_check(buffer);
	m[n].name = (char *)malloc(strlen(buffer)+2);
	strcpy(m[n].name,buffer);
	next();
	printw("Index BVP  : ");refresh();
	getstr(buffer);
	bs_check(buffer);
	m[n].cvup = atof(buffer);
	next();
	printw("Index FVP  : ");refresh();
	getstr(buffer);
	bs_check(buffer);
	m[n].ccup = atof(buffer);
	next();
	printw("Cost       : ");refresh();
	getstr(buffer);bs_check(buffer);
	m[n].cost = atof(buffer);

	if ( (m[n].ptr = (struct stp *)malloc(sizeof(struct stp))) == NULL ) {
		next();printw("Error in add_mat.");refresh();
		resetty();exit();
	}
	m[n].ptr->min_pow = -100.0;
	m[n].ptr->max_pow = 100.0;
	m[n].ptr->min_et  = 0.10;
	m[n].ptr->min_ct  = 0.15;
	m[n].ptr->min_jt  = 0.15;
	m[n].ptr->steepen = 0.0;

	mat_pick(n);

	next();
	printw("updating database...");
	refresh();
	/* write data to MATFILE */
	sort(m,n+1);
}


/*
* c h a n g e _ m a t  ( n ) [static ]
*
* Parameters :
*   n : number of items in list.
*
* Purpose : allow user to change an item in the list of materials.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:20 am
*/
static change_mat(n)
int n;
{
	int fd,i,sl,matches,last_mch;
	int c;

	echo();
top:
#ifndef dos
	cur_row=0;
	for (i=0;i<4;i++) next(); 
	cur_row=0;
#else
	cur_row=22;
#endif
	next();
	printw("Change Material in database");
	next();
	printw("Material Name or Number :");refresh();
	getstr(buffer);
	bs_check(buffer);

	/* if input number then extract string and put in buf */
	if ( isdigit(buffer[0]) ) {
		i = atoi(buffer);
		strcpy(buffer,m[i-1].name);
	}

	/* lookup name in current database */
	tolower_s(buffer);
	sl = strlen(buffer);
	matches = 0;
	last_mch = -1;
	for (i=0;i<n;i++) {
		if ( strncmp(buffer,m[i].name,sl) == 0 ) {
			if ( strlen(m[i].name) == sl ) {
				matches = 1;
				last_mch = i;
				break;
			}
			matches++;
			last_mch=i;
		}
	}
	if ( matches == 0 ) {
		next();
		printw("No match found for %s",buffer);
		refresh();
		goto top;
	} else
	if ( matches == 1 ) {
		mat_pick(last_mch);
		modified = TRUE;
	} else {
		next();
		printw("More than one match found for %s",buffer);
		next();
		printw("Could you be more specific");
		refresh();
		goto top;
	}

	printw("\n\nupdating database...");
	refresh();

	for (i=0;i<n;i++) /* convert to lower case */
		tolower_s(m[i].name);
	sort(m,n);
}


/*
* m a t _ p i c k  ( k ) [static ]
*
* Parameters :
*   k : material list item number.
*
* Purpose : allow user to change the material information.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:21 am
*/
static mat_pick(k)
int k;	/* material picked, allow user to change m[k] */
{
	int c,cur_row;

	cur_row = 0;
	clear();
	next(); 
	next(); printw("Material Name : %s",m[k].name);
	next(); printw("Index BVP    : %8.4f",m[k].cvup);
	next(); printw("Index FVP    : %8.4f",m[k].ccup);
	next(); printw("Base Cost    : %6.2f",m[k].cost);
	next(); printw("Steepen Information");
	next();
	next(); 
	printw("1-name, 2-BVP, 3-FVP, 4-Cost, 5-Steepen Info, 6-quit ");
	refresh();
	noecho();
	do { c = getch(); } while ( c < '1' || c > '6' );
	echo();
	switch (c) {
	case '1' : /* Material Name */
		next();
		printw("New Material Name : "); refresh();
		getstr(buffer);
		bs_check(buffer);
		if ( strlen(buffer) > strlen(m[k].name) ) {
			char *p;

			p = (char *)realloc(m[k].name,strlen(buffer)+1);
			if ( p == NULL ) return;
			m[k].name = p;
			strcpy(p,buffer);
		} else
			strcpy(m[k].name,buffer);
		break;
	case '2' : /* bvp index */
		next();
		printw("Index BVP  : "); refresh();
		getstr(buffer);
		bs_check(buffer);
		m[k].cvup = atof(buffer);
		break;
	case '3' : /* fvp index */
		next();
		printw("Index FVP  : "); refresh();
		getstr(buffer);
		bs_check(buffer);
		m[k].ccup = atof(buffer);
		break;
	case '4' : /* cost */
		next();
		printw("Base Cost : "); refresh();
		getstr(buffer);
		bs_check(buffer);
		m[k].cost = atof(buffer);
		break;
	case '5' : /* Steepen Information */
		stp_info(k);
		return;
	case '6' : /* quit */
		break;
	} /* switch */	
}


/*
* s t p _ i n f o  ( k ) [static ]
*
* Parameters :
*   k : material list item number to change
*
* Purpose : Allow user to change the steepening information for this mat.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:22 am
*/
static stp_info(k)
int k;
{
	int c,swap;
	int i,j,n;
	extern struct stp *list[20];
	struct stp *p;

	/* construct list of pointers in list[] */
	i = 0;
	p = m[k].ptr;	
	while (p) {
		list[i] = p;
		p = p->ptr;
		i++;
	}
	n = i;

	/* loop, doing (display,choice,modify) */
	do {
		cur_row = 0;
		clear();
		next();printw("Steepen Information for %s",m[k].name);
		next();
		next();
		printw(" #  Min Pow, Max Pow    ct     et    jt   Steepen");
		for (i=0;i<n;i++) {
			next();
			p = list[i];
		        printw("%2d %+8.2lf %+8.2lf ",
				i+1, p->min_pow,p->max_pow);
		        printw("%6.3lf %6.3lf %6.3lf %6.3lf",
				p->min_ct,p->min_et,p->min_jt,p->steepen);
		}
		next();
		next();
		printw("c-change,a-add,d-delete,q-quit");
		refresh();

		noecho();
		do { c = getch(); } while ( !in(c,"cadq") );
		echo();

		switch(c) {
		case 'a' : /* add item of steepening data */
			p = (struct stp *)malloc(sizeof(struct stp));
			memcpy(p,list[n-1],sizeof(struct stp));
			list[n] = p;
			p->ptr = NULL;
			list[n-1]->ptr = p;
			p->min_pow = get_dbl("Minimum Power ",&(p->min_pow));
			p->max_pow = get_dbl("Maximum Power ",&(p->max_pow));
			p->min_et  = get_dbl("Minimum E.T.  ",&(p->min_et));
			p->min_ct  = get_dbl("Minimum C.T.  ",&(p->min_ct));
			p->min_jt  = get_dbl("Minimum J.T.  ",&(p->min_jt));
			p->steepen = get_dbl("Steepen Amount",&(p->steepen));
			n++;
			break;
		case 'c' : /* change item */
			next();printw("Item to change ?");refresh();
			scanw("%d",&i);
			if ( i <= 0 || i > n ) break;
			p = list[i-1];
			p->min_pow = get_dbl("Minimum Power ",&(p->min_pow));
			p->max_pow = get_dbl("Maximum Power ",&(p->max_pow));
			p->min_et  = get_dbl("Minimum E.T.  ",&(p->min_et));
			p->min_ct  = get_dbl("Minimum C.T.  ",&(p->min_ct));
			p->min_jt  = get_dbl("Minimum J.T.  ",&(p->min_jt));
			p->steepen = get_dbl("Steepen Amount",&(p->steepen));
			break;
		case 'd' : /* delete item */
			next();printw("Item to change ?");refresh();
			scanw("%d",&i);
			i = i-1;
			if ( i < 0 || i > n ) break;
			free(list[i]);
			for (j=i;j<n-1;j++)
				list[j] = list[j+1];
			n--;	
			break;
		default :
			break;
		}
		/* re link list of items */
		for (i=0;i<n-1;i++) 
			list[i]->ptr = list[i+1];
		list[n] = NULL;
		m[k].ptr = list[0];
	} while ( c != 'q' );

	/* Begin Sort, simple buble sort */
	do {
		swap = 0;
		for(i=0;i<n-1;i++) 
			if ( list[i]->min_pow > list[i+1]->min_pow ) {
				p = list[i];
				list[i] = list[i+1];
				list[i+1] = p;
				swap++;
			}
	} while ( swap );
	/* re link list of items */
	list[n] = NULL;
	m[k].ptr = list[0];
	for (i=0;i<n;i++) 
		list[i]->ptr = list[i+1];
	/* End Sort */	
}


/*
* d e l e t e _ m a t  ( n ) [static ]
*
* Parameters :
*   n : number of items in material list.
*
* Purpose : delete one material from list.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:23 am
*/
static delete_mat(n)
int n;
{
	int j,i;
	int choice;

loop:
	next();printw("Material entry to delete "); refresh();
	echo();	
        getstr(buffer);
	if ( strlen(buffer) <= 0 ) return;
	bs_check(buffer);
	noecho();
	choice = atoi(buffer)-1;
	if ( choice >= n || choice < 0 ) {
		next();
		printw("Not found");
		goto loop;
	}

	/* crunch list and delete entry 'choice' */
	for (j=choice;j<n-1;j++)
		m[j] = m[j+1];
	return;
}


/* utility functions used in this module only */

/*
* t o l o w e r _ s  ( s ) [static ]
*
* Parameters :
*   s : input string.
*
* Purpose : convert first word in string to lower case.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:24 am
*/
static tolower_s(s)
char *s;
{
	while (*s) {
		if ( *s == ' ' ) break;
		if ( isupper(*s) ) 
			*s = tolower(*s);
		s++;
	}
}

/*
* s o r t  ( ary, n ) [static ]
*
* Parameters :
*   ary : input array.
*   n : number of elements.
*
* Purpose : Sort input array, uses library qsort.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:25 am
*/
static sort(ary,n)
struct mat *ary;
int n;
{
	extern int matcomp();

	qsort((char *)ary,(unsigned)n,(unsigned)sizeof(struct mat),matcomp);
}

/*
* m a t c o m p  ( a, b ) [static ]
*
* Parameters :
*   a : material structure.
*   b : material structure.
*
* Purpose : compare a and b, and return value to qsort.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:26 am
*/
static int matcomp(a,b)
struct mat *a,*b;
{
	return strcmp(a->name,b->name);
}

/*
* i n  ( c, s ) [static int]
*
* Parameters :
*   c : one character
*   s : string of characters
*
* Purpose : determine if c is in s.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Jan  5, 1989; 09:27 am
*/
static int 
in(c,s)
char c;		/* check to see if c is in */
char *s;	/* string of characters */
{
	while (*s) 
		if ( c == *s++ ) return 1;
	return 0;
}


/*
* l o o k u p _ m a t e r i a l  ( name, power ) [static struct stp *]
*
* Parameters :
*   name : material name
*   power : curent power of lens in question.
*
* Purpose : lookup steepening information for given material
*           with given power.
*
* Globals : m[] & n.
*
* Returns : struct stp *, or NULL if not found.
* Date : Fri Dec 22, 1989; 08:09 am
*/
static struct stp * lookup_material(name,power)
char *name;
double power;
{
  extern int NOAUTOMAT;
  static char save_name[80];
  static double save_power;
  static struct stp *save_ptr=(struct stp *)NULL;
  int i;
  struct stp *ptr;

  if ( m == NULL ) /* have not read materials file */
  {
    if ( NOAUTOMAT )
      return (struct stp *)NULL;
    else
      read_mat_file();
  }
  for ( i=0;i<n;i++ ) {
    if ( strcmp(m[i].name,name) == 0 ) 
      goto found;
  }
  return (struct stp *)NULL;

found:
  /*
  * now lookup steepen information, based on power.
  */
  ptr = m[i].ptr;
  while ( ptr != (struct stp *)NULL ) {
    if ( ptr->min_pow <= power && ptr->max_pow >= power ) 
      return ptr;
    ptr = ptr -> ptr;
  }
  return (struct stp *)NULL;
}

/*
* m i n i m u m _ c t  ( mat_name, pow ) [global double]
*
* Parameters :
*   mat_name : material name + [BF]VP indicator.
*   pow : power of lens we want a minimum ct for.
*
* Purpose : lookup minimum center thickness for given
*           lens and return value.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 17, 1989; 10:53 am
*/
double minimum_ct(mat_name,pow)
char *mat_name;
double pow;
{
	char *s;
	struct stp *p;
	double val;

	if ( strlen(mat_name) < 4 ) 
		return 0.15;
				/* strdup() uses malloc */
	s = strdup(mat_name);	/* make a copy of name */
	s[strlen(s)-4] = '\0';	/* get rid of FVP/BVP */

	if ( (p = lookup_material(s,pow) ) == NULL ) {
		val = 0.15;
	} else {
		val = p -> min_ct;
	}

	free(s);		/* get rid of copy of name */
	return val;
}

/*
* m i n i m u m _ e t  ( mat_name, pow ) [global double]
*
* Parameters :
*   mat_name : material name + [BF]VP indicator.
*   pow : power of lens we want a minimum ct for.
*
* Purpose : lookup minimum edge thickness for given
*           lens and return value.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 17, 1989; 10:53 am
*/
double minimum_et(mat_name,pow)
char *mat_name;
double pow;
{
	char *s;
	struct stp *p;
	double val;

	if ( strlen(mat_name) < 4 ) 
		return 0.10;
				/* strdup() uses malloc */
	s = strdup(mat_name);	/* make a copy of name */
	s[strlen(s)-4] = '\0';	/* get rid of FVP/BVP */

	if ( (p = lookup_material(s,pow) ) == NULL ) {
		val = 0.10;
	} else {
		val = p -> min_et;
	}

	free(s);		/* get rid of copy of name */
	return val;
}

/*
* m i n i m u m _ j t  ( mat_name, pow ) [global double]
*
* Parameters :
*   mat_name : material name + [BF]VP indicator.
*   pow : power of lens we want a minimum ct for.
*
* Purpose : lookup minimum edge thickness for given
*           lens and return value.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Mon Apr 17, 1989; 10:53 am
*/
double minimum_jt(mat_name,pow)
char *mat_name;
double pow;
{
	char *s;
	struct stp *p;
	double val;

	if ( strlen(mat_name) < 4 ) 
		return 0.15;
				/* strdup() uses malloc */
	s = strdup(mat_name);	/* make a copy of name */
	s[strlen(s)-4] = '\0';	/* get rid of FVP/BVP */

	if ( (p = lookup_material(s,pow) ) == NULL ) {
		val = 0.15;
	} else {
		val = p -> min_jt;
	}

	free(s);		/* get rid of copy of name */
	return val;
}
#endif /* __turboc__ */
