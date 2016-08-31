/* #include "lens.h" */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
#ifdef __ECO
#  define dos
#endif
#include <stdio.h>
#include <curses.h>
#include "pfile.h"
#include <ctype.h>

/* pfile.c 5.17 8/23/89 */

static int pnum = 0;
static char *term;	/* terminal type string */
static char *fn;	/* tmp file name */
static FILE *fileptr = NULL; /* file pointer passed back from open_pfile */
P_DEST print_dest = to_paper;
static char print_file[80];
extern int scripti;	/* script output file flag */

/*
* o p e n _ p f i l e  (  ) 
*
* NO-Parameters :
* Purpose : Open print file or signal pc-vt emulator to start printing.
*
* Globals : n.a. 
*
* Returns : File pointer to output file.
* Date : Thu Mar 30, 1989; 02:18 pm
*/
FILE *
open_pfile()
{
#ifdef dos
	if ( scripti ) 
		fileptr = fopen("\\dev\\nul","a");
	else
		fileptr = fopen("\\dev\\prn","a");
	return fileptr;
#else
	if ( scripti )
		strcpy(print_file,"/dev/null");
	else
		sprintf(print_file,"/usr/tmp/lens.%d.%d",getpid(),pnum++);

	if ( print_dest == to_file ) {
		if ( (fileptr = fopen(print_file,"a+")) == NULL ) {
			printf("Error opening print_file %s\n",print_file);
			resetty();
			exit(0);
		} else
		return fileptr;
	} else {					
		fn = print_file;
        	if ( ( fileptr = fopen(fn,"w") ) == NULL ) { 
			printf("Error opening print file %s\n",fn);
			resetty();
                	exit(0);
        	}
		return fileptr;
	}
#endif
}

/*
* c l o s e _ p f i l e  (  ) 
*
* NO-Parameters :
* Purpose : close print file and cause printout to occure.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Thu Dec 22, 1988; 01:35 pm
*/
void
close_pfile()
{	
	fclose(fileptr);
#ifndef dos
	if ( scripti ) return;
	if ( print_dest == to_paper ) 
		dump_to_paper(fn);
#endif
}

#ifndef dos

static
dump_to_paper(fn)
char *fn;
{
	void print_spool();
	void print_pc_vt();
	extern char *getenv();

	term = getenv("TERM");
	if ( strncmp(term,"pc-vt",5) == 0 ) {
		print_pc_vt(fn);
		unlink(fn);
	} else
		print_spool(fn);
}

static void 
print_spool(fn)
char *fn;
{
	extern char *getenv();
	int c;
	char path[40];
	char buf[100];

	term = getenv("LOC_PR");
	strcpy(path,"/usr/lib/lens/");

	if ( strlen(term) == 0 ) 
		strcat(path,"dumb");
	else
		strcat(path,term);

	sprintf(buf,"%s < %s | lpr -s",path,fn);
	system(buf);
	unlink(fn);

	move(0,0);
	printw("Output Sent to system printer (%s)\n",fn);
	printw("Press Return to continue\n");
	refresh();
	do { c = getch(); } while ( c!='\n' && c!='\r' );
}

static void 
print_pc_vt(fn)
char *fn;
{
	extern char *getenv();
	int ret_count;
	int c;
	char path[80];
	char buf[100];

	if ( status_pc_vt() == -1 ) return;

	term = getenv("LOC_PR");
	strcpy(path,"/usr/lib/lens/");

	if ( strlen(term) == 0 ) 
		strcat(path,"dumb");
	else
		strcat(path,term);

	/* pc-vt 8.4 code that turns printer on */
	printf("\033[5i");
	fflush(stdout);

	sprintf(buf,"%s < %s",path,fn);		
	system(buf);

	/* pc-vt 8.4 code that turns printer off */
	printf("\033[4i");
	fflush(stdout);
}

static
int status_pc_vt()
{
	register int c;
	int retval = 0;
	int time_count = 0;

	noecho();
	do {
	printf("\033%c%c15n",'[','\?');	/* ask for status */
	fflush(stdout);
	
	c = getch(); if ( c != 27 ) return -1;
	c = getch(); if ( c != '[' ) return -1;
	c = getch(); if ( c != '\?') return -1;
	c = getch(); if ( c != '1' ) return -1;
	c = getch(); 
		if ( c == '0' ) retval = 1;	/* O.K. */
		else if ( c == '1' ) retval = 0;/* not ready */
		else if ( c == '3' ) retval = -1;/* Error */
	c = getch(); if ( c != 'n' ) return -1;

	if ( retval == 1 ) return 1;	/* o.k. */
	if ( retval == -1 ) return -1;	/* Error don't print */
	
	move (0,0);
	printw("Printer not ready, please check if printer is on-line...");
	/* printw("\nPress Return when Ready.\n"); */
	refresh();

	if ( time_count == 0 ) {
		putchar(0x07);
		fflush(stdout);
	}
	time_count++;
	if (time_count >= 5) 
		time_count = 0;

	if ( retval == 0 )
		sleep(1);
	/* do { c = getch(); } while ( c!='\n' && c!='\r' ); */
	} while ( retval == 0 );
	echo();
}
#endif
