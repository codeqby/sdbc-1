/* ��������18dʱ���õ�У����� check.c  */
#include <stdio.h>
#include <math.h>
#include "calc.h"
#include "srw.h"

extern int errno;
extern struct srw_s Table;
struct srw_s *srwp=&Table;
double chkdata(int,int,char *[CALCID]);

char *stptok();
int row;          /* ��ǰ�� */

/*    У����   1:��ȷ   0:����    */

/* ����У������������ check.c  */
check(jfd)    
FILE *jfd;
{
int n,cc,err=0;
	/*wclear(wp);   ��У���� */
	err=dispchk(jfd,srwp);
	return(err);
}

/*  ���ֵ�,��Ļ��ʾ?   */
dispchk(jfd)
FILE *jfd;
{
char strj[256];
int cc;
int err;
double x,y;
	err=1;
	while(fgets(strj,sizeof(strj),jfd)) {
		if(feof(jfd)) break;
		TRIM(strj);
		if(*strj=='#' || *strj==';' || !*strj) continue;
		if(!chkstr(strj,&x,&y,chkdata)) {
			err=0;
/* ��У�鲻�ɹ�����(err=0)��У���ֵ�ĸ��м�����������ֵ��У������ʾ */
			printf("%lg(%lg).........%s\n",x,y,strj); 
		}
	}
	return(err);
}

double chkdata(n,argc,argv)
int n,argc;
char *argv[CALCID];
{
	if(argc==1) {
		s(3,argv[1]);
	}
	else if(argc==2) {
		s(1,argv[1]);
		s(3,argv[2]);
	}
	return(readitem(srwp,n));
}
