#include <strproc.h>

char *strtcpy(char *dst,char **src,char c)
{
register char *s,*t;

    if(!dst || !src || !*src) return dst;
	s=*src,t=dst;
    while(*s && (*s != c)) *t++=*s++;
    *t=0;
	*src=s;
    return t;
}

/* ���ϵͳδ����stpcpy,�ڴ˶��� */

#ifndef  __USE_XOPEN2K8
char * stpcpy(char *t,const char *s)
{
	return strtcpy(t,&s,0);
}
#endif

