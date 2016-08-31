#ifdef dos

#define Epson 1
#define Dmp132 2
#define Dmp132ibm 3

extern int _prtype;

#define UNDERLINEON(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033-1");\
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\017");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033-1");\
}
#define UNDERLINEOFF(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033-0");\
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\016");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033-0");\
}
#define BOLDON(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033G");\
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\037");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033E");\
}
#define BOLDOFF(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033H");\
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\040");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033F");\
}
#define PICA(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033\062\033P\033\022\033\065"); \
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\023");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033I1");\
}
#define ELITE(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033\062\033M"); \
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\027");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033:");\
}
#define COMPRESSED(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033\062\033P\033\017"); \
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\024");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033T\033I1\033\017");\
}
#define MICROFONT(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033\017\033S0\033A\006"); \
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033M");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033T\033I1\033\022\033\017\033S0");\
}
#define LPI6(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033\062"); \
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\066");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033\062");\
}
#define LPI8(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033\060"); \
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\070");\
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033\060");\
}
#define RESET(fp)	{\
	if ( _prtype == Epson )\
		fprintf(fp,"\033\@\033\062\033\065\033P"); \
	else if ( _prtype == Dmp132 )\
		fprintf(fp,"\033\066\033\040\033\023"); \
	else if ( _prtype == Dmp132ibm )\
		fprintf(fp,"\033\062\033I1\033T\033H");\
}

#else

#define BOLDON(fp) {fprintf(fp,"BOLD ON"); }
#define BOLDOFF(fp) {fprintf(fp,"BOLD OFF"); }
#define PICA(fp) {fprintf(fp,"PICA"); }
#define ELITE(fp) {fprintf(fp,"ELITE"); }
#define COMPRESSED(fp) {fprintf(fp,"COMPRESSED"); }
#define MICROFONT(fp) {fprintf(fp,"MICROFONT"); }
#define LPI6(fp) {fprintf(fp,"1/6"); }
#define LPI8(fp) {fprintf(fp,"1/8"); }
#define UNDERLINEON(fp) {fprintf(fp,"UL ON"); }
#define UNDERLINEOFF(fp) {fprintf(fp,"UL OFF"); }
#define RESET(fp) {fprintf(fp,"RESET"); }
#endif
