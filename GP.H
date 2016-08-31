/* 
 +--------------------------------------------------------------+
 |                                                              |
 |           LENSCALC (Contact Lens Design Program)             |
 |              (c)opyright 1988 James J. Carter                |
 |                   All Rights Reserved.                       |
 |                                                              |
 +--------------------------------------------------------------+
*/
/* List of procedures/functions dis_cg.c
* SetWindowVP(  ) [static]
* SetWindow( x1, y1, x2, y2 ) 
* SetViewport( x1, y1, x2, y2 ) 
* World2Screen( x, y, screen_x, screen_y ) [static]
* MoveTo( x, y ) 
* LineTo( x, y ) 
* DrawChar( c ) 
* DrawText( s ) 
* Transmit_Coords( x, y ) [static]
* ShowLine( x1, y1, x2, y2 ) 
* ShowChar( c, x, y ) 
*/
SetWindow(double,double,double,double);
SetViewport(double,double,double,double);
MoveTo(double,double);
LineTo(double,double);
DrawText(char *);
DrawChar(int);
circle(double,double,double);
arc(double,double,double,double,double);

extern double Minx,Maxx,Miny,Maxy;

#define tek4010 1
#define cgi 2

