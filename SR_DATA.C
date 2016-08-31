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
#include <fcntl.h>
#ifdef MSDOS
# include <io.h>
#endif


/* sr_data.c 5.9 6/16/89 */

#define BLOCK_SIZE      (sizeof(LENS))
#define FILE_NAME       "lens.dat"

extern char * home_file(char *);

save_data(data)
char *data;
{
        int wrt,fd,bytes;

        bytes = BLOCK_SIZE;
	
        if ( (fd = open(home_file(FILE_NAME),O_WRONLY|O_CREAT,0666)) != -1 ) {
#ifdef MSDOS
		setmode(fd,O_BINARY);
#endif
                wrt = write(fd,(char *)data,bytes);
                if ( wrt != bytes ) {
                        error("save_data: write");
                }
                close(fd);
        }
}

int
restore_data(data)
char *data;
{
        int bytes,fd;

        if ( (fd=open(home_file(FILE_NAME),O_RDONLY,0666)) != -1 ) {
#ifdef MSDOS
		setmode(fd,O_BINARY);
#endif
                if ( (bytes = read(fd,data,bytes)) != BLOCK_SIZE ) {
			sprintf(buffer,"Restore_data:%d bytes",bytes);
                        error(buffer);
                        close(fd);
                        return 0;
                }
                close(fd);
                return 1;
        } else
                return 0;
}

error(s)
char *s;
{
                clear();
                move(0,0);
                printw(s);
                refresh();
                getch();
}

static char *
home_file(file)
char *file;	/* file name to add on to home path */
#ifdef dos
{
	static char buffer[100];

	strcpy(buffer,"\\");
	strcat(buffer,file);

	return buffer;
}
#else
{
	static char buffer[100];
	char *home_path;

	home_path = (char *)getenv("HOME");
	if ( home_path == NULL ) { /* no home path put in /tmp */
		strcpy(buffer,"/tmp/");
		strcat(buffer,file);
	} else {
		strcpy(buffer,home_path);
		strcat(buffer,"/");
		strcat(buffer,file);
	}
	return buffer;
}	
#endif
