/* @(#) main.c 5.50@(#) 8/29/90 09:21:58 */
/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
#include "lens.h"
#include "table.h"
#include <signal.h>
#include "pfile.h"
#include "version.h"

#ifdef dos
int	_prtype = 0;
#endif

extern int	sig_exit();
static LENS l;

/* Global variables used throughout lens program */

/* Current row position 0 to 24 */
int	cur_row;

/* Blink status flag. if != 0, then blink is o.k. */
int	BLINK_ON = 1;

/* Script-file input/output flags & FILE *pointers */
int	scripti;		/* input script file input (true/false)(1/0) */
FILE *fp_iscript;	/* input script file pointer */
char	*fn_iscript, *fn_oscript;
int	scripto;		/* output script file input (true/false)(1/0) */
FILE *fp_oscript;	/* output script file pointer */

/* Just declared here to simplify things.  Buffers & temporaries */
/* Under "cc -dos -K", sending address of stack variable is a problem */
/* So when doing sprintf(buf,"format",variable,...), just use the global */
/* buffers bellow. */
char	buffer[100];
char	buffer1[100];
double	dbl_tmp, dbl_tmp1, dbl_tmp2;

/* Debugging flags & file pointer */
int	CLOCK, DEBUG;	/* externals set as program options */
FILE *dbg;		/* used for debug output */

/* 
* If user wishes to direct printout of lens to a file
* user can specify output by using the program argument
* "lens pout filename".
* file name is saved in print_out_fn[].
*/
char	*print_out_fn = NULL;

/*
Fri Dec 22 08:01:22 PST 1989
* noround, flag= true, used to specify that user doesn't wish to be
* asked if rounding of radius values input is ok.
*/
int NOROUND;

/*
Fri Dec 22 08:01:29 PST 1989
* noautomaat, flag=true, used to specify that material databases is
* not automatically read into program. (occurs in pr_lens.c)
*/
int NOAUTOMAT;

/*
Fri Dec 29 07:56:51 PST 1989
* fields_out, file name which is used to put fields in ascii delimitted
* format at exit of program. I.E. "ct/et","Power",... in numerical format
*/
char *fields_out = NULL;

/*
Wed Jan 10 08:44:38 PST 1990
* notes_line, line of text printed on paper/file at top of output.
*/
char *notes_line = NULL;


/*
* m a i n  ( argc, argv ) 
*
* Parameters :
*   argc : argument count.
*   argv : argument pointers.
*
* Purpose : main entry point of lens program.
*
* Globals : above globals, initialize and start ball rolling.
*
* Returns : nothing.
* Date : Fri Dec 22, 1989; 08:01 am
*/
main(argc, argv)
int	argc;
char	**argv;
{
	int	i;
	double	tmp;
	int	val;
	int	modified;
	extern double	round(double);
	char	*prname;
	extern char	*getenv();

	/* 
	* make all signals allowed by xenix o.s. to go to non
	* default function.  There we can gracefully exit.
	*/
#ifndef dos
	(void)signal(SIGHUP, sig_exit);
	(void)signal(SIGINT, sig_exit);
	(void)signal(SIGQUIT, sig_exit);
	(void)signal(SIGILL, sig_exit);
	(void)signal(SIGTRAP, sig_exit);
	(void)signal(SIGIOT, sig_exit);
	(void)signal(SIGEMT, sig_exit);
	(void)signal(SIGFPE, sig_exit);
	(void)signal(SIGBUS, sig_exit);
	(void)signal(SIGSEGV, sig_exit);
	(void)signal(SIGSYS, sig_exit);
	(void)signal(SIGPIPE, sig_exit);
	(void)signal(SIGALRM, sig_exit);
	(void)signal(SIGTERM, sig_exit);
#endif

#ifdef dos
	prname = getenv("LOC_PR");
	if ( strcmp(prname, "EPSON") == 0 )
		_prtype = Epson;
	else if ( strcmp(prname, "DMP132") == 0 )
		_prtype = Dmp132;
	else if ( strcmp(prname, "DMP132IBM") == 0 )
		_prtype = Dmp132ibm;
	else if ( strcmp(prname, "IBM") == 0 )
		_prtype = Dmp132ibm;
#endif
	/*
	* parse arguments and put program in modes specified
	* by arguments.
	*/
	arguments(argv, argc);
	/* 
	* Initialize curses screen, debug file output & script files 
	*/
	init();
	copyright();

	/* If we are doing script input/output don't read saved
	*  lens data 
	*/
	if ( scripti || scripto ) {
		init_lens(&l);
		input_lens(&l);
	} else {
		if ( restore_data(&l) == TRUE )
			display_lens(&l);
		else {
			init_lens(&l);
			input_lens(&l);
		}
	}

	while (1) {               /* forever loop */
		BLINK_ON = 1;
		val = dis_menu();
		BLINK_ON = 0;
		display_lens(&l);
		BLINK_ON = 1;
		modified = FALSE;
		switch (val) {
		case REDRAW :	/* ^L */
			clear();
			refresh();
			break;
		case MOD_CT :
			l.center_thick = get_float("New Center thickness ",
			     				&l.center_thick);
			/* clear prism bit/flag if we have prism */
			if ( l.extra_flags & EXRA_PRISM )
				l.extra_flags &= ~EXRA_PRISM;
			l.pref = CT_PREF;
			modified = TRUE;
			break;
		case MOD_ET :
			if ((l.lens_type & LT_LENTIC) &&  (l.lens_type &
			    LT_TOR_SEC) ) {
				l.t.save_lt_et = get_float( "Toric Sec. Lenitc ET ",
				     &(l.t.save_lt_et));
			} else if (( l.lens_type & LT_LENTIC ) &&  (l.lens_type &
			    LT_TOR_ROZ) ) {
				l.t.save_lt_et =  get_float("Lenticular Edge Thickness",
				     				    &l.t.save_lt_et);
			} else if ( l.lens_type & LT_LENTIC ) {
				l.lt_et = get_float("Lenticular Edge Thickness",
				     					&l.lt_et);
			} else if ( l.lens_type & LT_TOR_SEC ) {
				l.t.save_et = get_float("Toric Sec. ET ",
				     					&(l.t.save_et));
				l.pref = ET_PREF;
			} else {
				l.edge_thick = get_float("New Edge Thickness ",
				     					&l.edge_thick);
				l.pref = ET_PREF;
			}
			/* clear prism bit/flag if we have prism */
			if ( l.extra_flags & EXRA_PRISM )
				l.extra_flags &= ~EXRA_PRISM;
			modified = TRUE;
			break;
		case MOD_POWER :
			if ( l.lens_type & LT_TRI_DD) {
				l.itrm.trial_pow = get_float("New Trial Power",
				     					&l.itrm.trial_pow);
			} else if ( l.lens_type & (LT_DDECARLE | LT_DECARLE)) {
				l.trial_pow = get_float("New Trial Power",
				     					&l.trial_pow);
			} else if ( l.lens_type & (LT_TOR_OOZ | LT_TOR_ROZ)) {
				l.t.pow_flat = get_float("Flat Meridian Power",
				     					&l.t.pow_flat);
			} else {
				l.power = get_float("New Power ", &l.power);
			}
			modified = TRUE;
			break;
		case MOD_MATERIAL :  
			 {
				extern char	*mat_name, *mat_bfvp;
				extern double	mat_index;

				mod_mat();
				if ( strlen(mat_name) ) {
					l.index = mat_index;
					strcpy(l.mat_type, mat_name);
					strcat(l.mat_type, mat_bfvp);
					modified = TRUE;
				}
				break;
			}
		case MOD_INDEX :
			cur_row = 0;
			move(cur_row++, 0);
			l.index = get_float("New Index ", &l.index);

			strcpy(buffer, l.mat_type);
			move(cur_row++, 0);
			printw("Material Name (%s) ", buffer);
			getstr(buffer);
			if ( strlen(buffer) )
				strcpy(l.mat_type, buffer);

			modified = TRUE;
			break;
		case MOD_BASE :
			if ( l.lens_type & (LT_TOR_OOZ | LT_TOR_ROZ) ) {
				l.t.steep_radius = get_float("Steep Base",
				     					&l.t.steep_radius);
				l.t.flat_radius = get_float("Flat Base",
				    &l.t.flat_radius);
				if ( l.lens_type & LT_TOR_OOZ ) {
					l.radius[l.rings-1] = l.t.steep_radius;
				} else {
					l.round.rad[l.round.rings-1] =  l.t.flat_radius;
					l.radius[l.rings-1] = l.t.steep_radius;
				}
			} else {
				l.radius[l.rings-1] = get_float( "New Base Curve ",
				     &l.radius[l.rings-1]);
				if ( l.radius[l.rings-1] >= MAX_MM ) {
					l.radius[l.rings-1] =  (double)diopt_to_mm(TEAR_INDEX,
					     							l.radius[l.rings-1]);
					l.radius[l.rings-1] =  round(l.radius[l.rings-1]);
				}
			}
			modified = TRUE;
			break;
		case MAKE_LENTIC :
			i = 0;
			while ( i == 0 ) {
				l.lt_oz = get_float("Lenticular O.Z. ", &l.lt_oz);
				if ( l.lt_oz > l.diameter[0] ) {
					dis_er_msg("Lenticular o.z. must <= lens diameter");
					i = 0;
				} else {
					ers_er_msg();
					i = 1;
				}
			}
			if ( l.lens_type & (LT_TOR_SEC | LT_TOR_ROZ))
				l.lt_et = l.t.save_lt_et;

			l.lt_et = get_float("Lenticular Edge Thickness ",
			     					&l.lt_et);

			if ( l.lens_type & (LT_TOR_SEC | LT_TOR_ROZ) )
				l.t.save_lt_et = l.lt_et;

			l.lens_type |= LT_LENTIC;
			modified = TRUE;
			break;
		case MAKE_PRINT :
			/*
			* if user has specified where output
			* will go, print lens to output file.
			*/
			/* Removed temoraraly
			if ( print_out_fn != NULL ) {
				FILE * fout;

				print_dest = to_file;
				fout = fopen(print_out_fn, "a");
				if ( fout == NULL ) {
					clear();
					move(0, 0);
					printw("Error opening %s", print_out_fn);
					refresh();
					nodelay(stdscr, FALSE);
					endwin();
					if (DEBUG) 
						fclose(dbg);
					exit();
				}
				print_lens(&l, fout);
				fclose(fout);
			} else */
			{
				/* otherwise, just print to paper */
				print_dest = to_paper;
				print_lens(&l, NULL);
			}
			break;

		case PRISM_OPT :
			/* convert to prism, not mm offset */
			l.bal_prism = l.bal_prism / 0.16;
			help_msg("A value <= .64 = MM offset, > .64 = Prism amount");
			l.bal_prism = get_float("Ballast Prism", &l.bal_prism);
			if ( l.bal_prism > .64 )	/* 4 prism */
				l.bal_prism = l.bal_prism * 0.16;
			if ( l.min_et < .05 || l.min_et > 1.0 )
				l.min_et = l.edge_thick;
			dbl_tmp = l.min_et;
			help_msg( "This value is used to calculate the required thickness based on above prism");
			dbl_tmp = get_float("Minimum Edge Thickness", &dbl_tmp);
			l.min_et = dbl_tmp;
			dbl_tmp += (l.diameter[0] * (l.bal_prism / 0.16)
			    / (2.0 * 49.0));
			sprintf(buffer, "This is the e.t. required to produce a finished e.t. of %4.2lf with %g prism",
			     l.min_et, (l.bal_prism / 0.16));
			help_msg(buffer);
			dbl_tmp = get_float("Edge Thickness + Ballast prism edge",
			     				&dbl_tmp);
			if ( dbl_tmp > l.edge_thick )
				l.edge_thick = dbl_tmp;
			l.pref = ET_PREF;

			/* 
			* recalculate minimum et in case bal+min et is not
			* used for the reason above where the new bal+min
			* edge thickness is less than the current lens et.
			*/
			l.min_et = l.edge_thick -  ( l.diameter[0] * (l.bal_prism
			    / 0.16) / (2.0 * 49));

			/* set prism bit/flag */
			l.extra_flags |= EXRA_PRISM;

			if ( l.lens_type & LT_TOR_SEC ) {
				do {
					help_msg("Enter 0 for flat,90 for steep meridian");
					l.t.axis =  get_float("Toric-Sec prism Axis, 0=flat/90=steep",
					     			              &l.t.axis);
				} while ( l.t.axis != 0.0 && l.t.axis !=
				    90.0 );
				l.t.save_et = l.edge_thick;
				l.pref = ET_PREF;
			}
			help_msg("");
			modified = TRUE;
			break;
		case PRINT_TO_FILE :
			print_dest = to_file;
			print_lens(&l, NULL);
			break;
		case NEW_LENS :
			wipe_lens(&l);
			l.lens_type = LT_REG;
			l.extra_flags = EXRA_NONE;
			display_lens(&l);
			input_lens(&l);
			break;
		case TORIC_MENU :
			toric_menu(&l);
			modified = TRUE;
			break;
		case BIF_MENU :
			bifocal_menu(&l);
			modified = TRUE;
			break;
		case SPECIAL_MENU :
			special_menu(&l);
			modified = TRUE;
			break;
		case MOD_JUNCT :
			if ( l.lens_type & LT_LENTIC ) {
				l.pref = CT_PREF;
				if ( l.lens_type & (LT_TOR_SEC | LT_TOR_ROZ)) {
					dbl_tmp = l.t.jt_flat;
					dbl_tmp = get_float( "Toric Junction thickness ",
					     				  	&dbl_tmp);
				} else {
					dbl_tmp = l.lt_jt;
					dbl_tmp = get_float("Junction thickness",
					     				  		&dbl_tmp);
				}

				if ( l.lens_type & (LT_TOR_ROZ | LT_TOR_SEC) ) {
					l.t.jt_flat = dbl_tmp;
					l.t.save_jt = dbl_tmp;
					l.pref = JT_PREF;
				} else
					junct_thick(&l, dbl_tmp);
				modified = TRUE;
			}
			break;
		case SAVE_DATA :
			save_data(&l);
			break;
		case PRINT_VER :
			clear();
			move(0, 0);
			printw("program : %s", argv[0]);
			move(1, 0);
			printw("%s", version);
			move(2, 0);
			printw("Press <Enter> to continue.");
			refresh();
			getstr(buffer);
			break;
#ifndef dos
		case GRIPE :
			 {
				char	buf[100];
				char	mfile[20];
				FILE * mf;

				clear();
				refresh();
				/* resetty(); */

				sprintf(mfile, "/tmp/lm.%d", getpid());
				sprintf(buf, "date > %s", mfile);
				system(buf);
				if ( (mf = fopen(mfile, "a")) == NULL )
					break;
				printf("Enter Your Gripe, end with blank line\n");
				fprintf(mf, "\nprogram : %s\n%s\n\n", argv[0],
				     version);
				do {
					echo();
					fprintf(mf, "%s\n", gets(buf));
				} while ( strlen(buf) > 0 );
				printf("Appending current lens to your comments.\n");
				fprintf(mf, "----------------\n\n");
				print_lens(&l, mf);
				fclose(mf);
				sprintf(buf, "/usr/lib/lens/dumb < %s | mail -s lens_program_gripe jim ",
				     mfile);
				system(buf);
				unlink(mfile);
				printf("Thank you, \n");
				printf("Your gripe has been mailed to jim\n");
				sleep(2);

				init();
				display_lens(&l);
				break;
			}
#endif
		case SHELL :
			clear();
			refresh();
			resetty();
			endwin();
			printf("Return to lens program by typing 'exit'\n\n");
#ifdef dos
			system("command.com");
#else
			system("/bin/csh");
#endif
			init();
			display_lens(&l);
			break;
		case EXIT_SAVE :
			save_data(&l);
		case EXIT_PROG :
			if ( print_out_fn != NULL ) {
				FILE * fout;

				print_dest = to_file;
				fout = fopen(print_out_fn, "w");
				if ( fout == NULL ) {
					clear();
					move(0, 0);
					printw("Error opening %s", print_out_fn);
					refresh();
					nodelay(stdscr, FALSE);
					endwin();
					if (DEBUG) 
						fclose(dbg);
					exit();
				}
				print_lens(&l, fout);
				fclose(fout);
			} 
			clear();
			refresh();
			resetty();
#ifndef dos
			nodelay(stdscr, FALSE);
#endif
			endwin();
			if ( DEBUG ) 
				fclose(dbg);
			if ( fields_out != NULL )
				dump_fields(fields_out,&l);
			exit();
		default :
			break;
		}
		if ( modified )
			lenscalc(&l);
		display_lens(&l);
	}
}



/*
* a r g u m e n t s  ( argv, argc ) 
*
* Parameters :
*   argv : see main()
*   argc : same.
*
* Purpose : Parse arguments & set global flags/variables.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Fri Dec 22, 1989; 08:02 am
*/
static arguments(argv, argc)
char	**argv;
int	argc;
{
	extern int	CLOCK;
	extern int	DEBUG;
	extern int	scripti;
	extern int	NOROUND,NOAUTOMAT;
	register int	i;
	register char	*p;

#ifndef dos
	CLOCK = 1;
#else
	CLOCK = 0;
#endif
	DEBUG = 0;
	i = 1;
	scripti = 0;
	scripto = 0;
	NOROUND = FALSE;
	NOAUTOMAT = FALSE;

	while ( i < argc ) {
    p = argv[i];
    to_lower_str(p);
    if ( *p == '-' ) 
      p++;
    if ( strcmp(p, "noround") == 0 )
    {
      NOROUND = TRUE;
      i++;
    } else 
    if ( strcmp(p,"nomat") == 0 )
    {
      NOAUTOMAT = TRUE;
      i++;
    } else
    if ( strcmp(p, "out") == 0 ) {
      if ( (i + 1) < argc ) {
        fields_out = strdup(argv[i+1]);
        i += 2;
      } else
        i++;
    } else
    if ( strcmp(p, "note") == 0 ) {
      if ( (i + 1) < argc ) {
        notes_line = strdup(argv[i+1]);
        i += 2;
      } else
        i++;
    } else
    if ( strcmp(p, "pout") == 0 ) {
      if ( (i + 1) < argc ) {
        print_out_fn = strdup(argv[i+1]);
        i += 2;
      } else
        i++;
    } else if /* clock if */
    ( ( strcmp(p, "clock") == 0 ) ||  ( strcmp(p, "clk") == 0 ) ||
        ( strcmp(p, "time") == 0 ) ) {
      if ( (i + 1) < argc )  {
        switch (on_off(argv[i+1])) {
				case 1 : 
					CLOCK = 1; 
					i += 2; 
					break;
				case 0 : 
					CLOCK = 0; 
					i += 2; 
					break;
				default:
					printf("Error: %s %s\n", argv[i],
					     argv[i+1]);
					printf("Program switch used with incorret verb.\n");
					printf("Legal Verbs are ON|OFF|YES|NO|TRUE|FALSE\n");
					exit(0);
				} /* switch */
			} /* if (i+1) < argc */ else {
				i++;
				CLOCK = 0;	/* turn off */
			}
		} else if /* clock if */
		( ( strcmp(p, "d") == 0 ) ||  ( strcmp(p, "db") == 0 ) ||
		    ( strcmp(p, "debug") == 0 ) ) {
			if ( (i + 1) < argc ) {
				switch (on_off(argv[i+1])) {
				case 1 : 
					DEBUG = 1; 
					i += 2; 
					break;
				case 0 : 
					DEBUG = 0; 
					i += 2; 
					break;
				default:
					printf("Error: %s %s\n", argv[i],
					     argv[i+1]);
					printf("Program switch used with incorret verb.\n");
					printf("Legal Verbs are ON|OFF|YES|NO|TRUE|FALSE\n");
					exit(0);
				} /* switch */
			} /* if (i+1) < argc */ else {
				i++;
				DEBUG = 1;	/* turn on */
			}
		} else if ( ( strcmp(p, "scripti") == 0 ) ||  ( strcmp(p,
		     "sci") == 0 ) ||  ( strcmp(p, "si") == 0 ) ) {
			if ( (i + 1) < argc )  {
				if ( strcmp(argv[i+1], "-") == 0 ) {
					fp_iscript = stdin;
					scripti = 1;  /* true */
				} else {
					/* check for read access */
					if ( access(argv[i+1], 04) == -1 ) {
						printf("Read access denied for %s\n",
						     argv[i+1]);
					} else {
						fn_iscript = argv[i+1];
						fp_iscript = fopen(argv[i+1],
						     "r");
						if ( fp_iscript == NULL )
							printf("Can't open %s\n",
							     argv[i+1]);
						else
							scripti = 1;
					}
				}
				i += 2;	/* successfull */
			} else {
				i++;		/* only one argument given */
				printf("Error: %s \n", argv[i]);
				printf("Scripti option requires file name or,\n");
				printf("\"-\" for standard input (stdin)\n");
				exit(0);
			}
		} else if ( ( strcmp(p, "scripto") == 0 ) ||  ( strcmp(p,
		     "sco") == 0 ) ||  ( strcmp(p, "so") == 0 ) ) {
			if ( (i + 1) < argc )  {
				if ( strcmp(argv[i+1], "-") == 0 ) {
					fp_oscript = stdout;
					scripto = 1;  /* true */
				} else {
					static char	script_name[100];

					to_lower_str(argv[i+1]);
					if ( strcmp(argv[i+1], "lib") ==
					    0 ) {
						new_scriptfile(script_name);
						fp_oscript = fopen(script_name,
						     "w");
						fn_oscript = script_name;
					} else {
						fp_oscript = fopen(argv[i+1],
						     "w");
						fn_oscript = argv[i+1];
					}

					if ( fp_oscript == NULL )
						printf("Can't open %s\n",
						     argv[i+1]);
					else
						scripto = 1;
				}
				i += 2;	/* successfull */
			} else {
				i++;		/* only one argument given */
				printf("Error: %s \n", argv[i]);
				printf("Scripto option requires file name or,\n");
				printf("\"-\" for standard output (stdout)\n");
				exit(0);
			}
		} else {
			printf("Error: %s\n", argv[i]);
			printf("Unknown Program Switch.\n");
			printf("Legal switches are :\n");
			printf("\tDescription  - Valid switches,    Options\n");
			printf("\t-------------  ---------------- -----------------\n");
			printf("\tClock        - clock,clk,time   [on|off|yes|no]\n");
			printf("\tDebugging    - debug,db,d       [on|off|yes|no]\n");
			printf("\tScript input - scripti,sci,si   (-|file)\n");
			printf("\tScript output- scripto,sco,so   (-|file|lib)\n");
			printf("\tRadius Round - noround          none\n");
			printf("\tMaterial*    - nomat            none\n");
			printf("\tNotes Line   - note             (file)\n");
			printf("\tPrint file   - out              (file)\n");
			printf("\t-------------  ---------------- -----------------\n");
			printf("\t [options] -- Optional\n");
			printf("\t (options) -- Required\n");
			printf("\t * Material database is normaly automatically loaded.\n");
			printf("\t Using the \"nomat\" switch, suppresses auto-load,\n");
			printf("\t until the user explicitly enters the material menu.\n");
			exit(0);
		}
	}

	if ( scripti || scripto )
		CLOCK = 0;

	if ( DEBUG ) {
		printf("Clock = %s\n", (CLOCK ? "ON" : "OFF"));
		printf("Debug = %s\n", (DEBUG ? "ON" : "OFF"));
		printf("Press Return\n");
		gets("buffer ");
	}
}



/*
* o n _ o f f  ( s ) 
*
* Parameters :
*   s : input string.
*
* Purpose : determine if (s) == ON,OFF,TRUE,FALSE,YES,NO and 
*           return (1) for ON, (0) for OFF, (-1) for other.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Tue Apr 18, 1989; 09:47 am
*/
static on_off(s)
char	*s;
{
	to_lower_str(s);
	if ( (strcmp(s, "yes") == 0) ||  (strcmp(s, "true") == 0) ||  (strcmp(s,
	     "on") == 0) ||  (strcmp(s, "1") == 0) )
		return 1;	/* true */
	if ( (strcmp(s, "no") == 0) ||  (strcmp(s, "false") == 0) ||  (strcmp(s,
	     "off") == 0) ||  (strcmp(s, "0") == 0) )
		return 0;
	return - 1;	/* error, not in lists above */
}


#include <ctype.h>

/*
* t o _ l o w e r _ s t r  ( s ) 
*
* Parameters :
*   s : input string.
*
* Purpose : change input string to lower case string.
*
* Globals : n.a. 
*
* Returns : modified string (s).
* Date : Tue Apr 18, 1989; 09:50 am
*/
static to_lower_str(s)
char	*s;
{
	while (*s) {
		if ( isupper(*s) ) 
			*s = tolower(*s);
		s++;
	}
}


/*
* n e w _ s c r i p t f i l e  ( s ) 
*
* Parameters :
*   s : input/output file name.
*
* Purpose : return file name of script file in form file.# where
*           # is a base 10 number.  The number is incremented until
*           file.# dosn't exist.  
*
* Globals : n.a. 
*
* Returns : modified (s).
* Date : Tue Apr 18, 1989; 09:51 am
*/
static new_scriptfile(s)
char	*s;
{
	int	i = 0;
	do {
#ifdef dos
		sprintf(s, "\\script.%d", i++);
#else		
		sprintf(s, "/u/jim/lens/script/%s.%d", getlogin(), i++);
#endif
	} while ( access(s, 00) == 0 );
}


/*
* c o p y r i g h t  (  ) 
*
* NO-Parameters :
* Purpose : display copyright message on screen.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Fri Jun 16, 1989; 03:01 pm
*/
copyright()
{
	clear();
	boxdraw(5, 10, 15, 67);
	center_line(7, "lenscalc");
	center_line(8, version);
	center_line(10, "(c)opyright 1988 James J. Carter");
	center_line(11, "All Rights Reserved.");
	center_line(13, "(press any key)");
	refresh();
	getch();
	clear();
	refresh();
}


/*
* c e n t e r _ l i n e  ( row, str ) 
*
* Parameters :
*   row : row to display information on.
*   str : string to display centered on row (row).
*
* Purpose : display string (str) on row (row) centered.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Fri Jun 16, 1989; 03:02 pm
*/
center_line(row, str)
int	row;
char	*str;
{
	int	col;
	col = 37 - strlen(str) / 2;
	move(row, col);
	printw(str);
}


/*
* b o x d r a w  ( r1, c1, r2, c2 ) 
*
* Parameters :
*   r1 : coordinates for box edge.
*   c1 : 
*   r2 : 
*   c2 : 
*
* Purpose : draw box on screen that includes r1,c1 r2,c2
*  On msdos, use double box graphic characters.
*
* Globals : n.a. 
*
* Returns : n.a. 
* Date : Fri Jun 16, 1989; 02:59 pm
*/
boxdraw(r1, c1, r2, c2)
int	r1, c1, r2, c2;
{
	int	i;

	for (i = r1 + 1; i < r2; i++) {
		move(i, c1);
#ifdef MSDOS
		addch(186);
#else
		addch('|');
#endif
		move(i, c2);
#ifdef MSDOS
		addch(186);
#else
		addch('|');
#endif
	}

	for (i = c1 + 1; i < c2; i++) {
		move(r1, i);
#ifdef MSDOS
		addch(205);
#else
		addch('-');
#endif
		move(r2, i);
#ifdef MSDOS
		addch(205);
#else
		addch('-');
#endif
	}

#ifdef MSDOS
	mvaddch(r1, c1, 201);
	mvaddch(r2, c1, 200);
	mvaddch(r1, c2, 187);
	mvaddch(r2, c2, 188);
#else
	mvaddch(r1, c1, '/');
	mvaddch(r2, c1, '\\');
	mvaddch(r1, c2, '\\');
	mvaddch(r2, c2, '/');
#endif
}
