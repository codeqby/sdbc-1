/*    chkstr.c   ��һ��У���ֵ���д���  */
#include <sdbc.h>
#include "calc.h"
double fabs(double x);

static int chkline();
/*   ���� ��1 ��    0 ��   */
chkstr(strj,px,py,chkdata)  /*  ��У���зֽ����ֵ����ֵ�Ҽ���  */
char *strj;
double *px,*py;
double (*chkdata)(int,int,char *[CALCID]);
{
register char *pnow;
char lft[512];
static char rel[]="&|";
int cc,c;
	*px=*py=0;
	if(!strj || !*strj) return(1);
	while(1) {
 /* ��У������'&|'�ض� */
		pnow=stptok(strj,lft,sizeof(lft),rel);
		cc=chkline(lft,px,py,chkdata);
		if(!*pnow) break;
		c=(*pnow=='&');
		*pnow=0;
		if(!(*(++pnow)) || (cc ^ c)) break;
		strcpy(strj,pnow);
	} 
	return cc;
}
static int chkline(strj,px,py,chkdata)
char *strj;
double *px,*py;
double (*chkdata)(int,int,char *[CALCID]);
{
register char *cp, *pnow;
char lft[512],rule[4];
static char rel[]="!=<>";
int cc;
	pnow=stptok(strj,lft,sizeof(lft),rel); /* ��У������'!=<>'�ض� */
	if(!*pnow) {              /*  У������'!=<>',������ֵ  */
		*px=calculator(strj,chkdata);
		*py=0.;

		return(*px != *py);
	}
	cp=rule;
	while(strchr(rel,*pnow) && cp-rule<sizeof(rule)-1) { /* ��У�����е�`!=<>`����rule�� */
		*cp++ = *pnow++;
	}
	*cp=0;
	*px=round(calculator(lft,chkdata));      /* ������ֵ */
	*py=round(calculator(pnow,chkdata));     /* ������ֵ */
/*
fprintf(stderr,"chkstr %f %s %f\n",*px,rule,*py);
*/

	cc=compare(*px,rule,*py);

	return(cc);
}

int compare(x,rel,y)  /*  �Ƚ� x y  */
double x,y;
register char *rel;
{
	if(isnull(&x,CH_DOUBLE) ||
		isnull(&y,CH_DOUBLE)) return 0;

	if (!rel)  return(x);
	else if (!strncmp(rel,"==",2)) return(x==y);
	else if (!strncmp(rel,">=",2)) return((x-y)>=0);
	else if (!strncmp(rel,"<=",2)) return((x-y)<=0);
	else if (!strncmp(rel,"!=",2)) return((x-y)!=0);
	else if (!strncmp(rel,"=",1))  return(x==y);
	else if (!strncmp(rel,">",1))  return(x>y);
	else if (!strncmp(rel,"<",1))  return(x<y);
	else {
		return(fabs(x-y) < 0.0001);  
	}
}

