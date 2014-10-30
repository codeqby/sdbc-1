#include <ctype.h>
char * skipblk(char *);
char * skipblk(str)
char * str;
{
char *p;
	p=str;
	while(!(*p&0x80)&&isspace(*p)) p++;
	return(p);
}
