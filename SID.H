/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
version(s)
char *s;
{
	clear();
	move(2,10);
	printw("%s - Version - 5.2, Date - 6/16/89, Time - 15:29:18",s);
	move(3,10);
	printw("Press return to continue....");
	refresh();
}
	
