/****************************** description *****************************/
/*Copyright (C), 2006-2009, EASYWAY. Co., Ltd.							*/
/*function:�ַ���������												*/
/*author:yulihua��huangpeng												*/
/*modify date:2009-4-21													*/
/************************************************************************/
#include <ctype.h>
#include <strproc.h>

#ifndef MIN
#define MIN(a,b) ((a>b)?b:a)
#endif

/*function:iscc �ж��Ƿ��Ǻ���*/
int iscc(unsigned char ch)
{
	return (ch >= 0x81 && ch < 0xff);
}
/*function:cc1 ȡ���ֵ�һ���ֽ�*/
static int cc1(unsigned char *bp,unsigned char *bufp)
{
	register unsigned char *p;
	register int i = 0;
	for(p = bufp; iscc(*p); p--)
	{
		i++;
		if(p <= bp) 
			break;
	}
	return (i & 1);
}
/*function:firstcc ���ֵĵ�һ���ֽ�*/
int firstcc(unsigned char *bp,unsigned char *bufp)
{
	if(!bufp || !(*bufp) || (bufp < bp) || !iscc(*bufp)) 
		return 0;
	return (cc1(bp, bufp));
}
/*function:secondcc ���ֵĵڶ����ֽ�*/
int secondcc(unsigned char *bp,unsigned char *bufp)
{
	if(!firstcc(bp, bufp-1)) 
		return 0;
	if(*bufp == 0x7f) 
		return 0;
	if((*bufp >= 0x80) && (*bufp <= 0xfe)) 
		return 1; 
	if(GBK_flag && *bufp>=0x40 && *bufp<0X7f) return 1;
	return 0;
}

/************************************************************************/
/*function:stptok �ֽ��ַ���Ϊһ���Ǵ�	                        */
/*description:���ݷָ���'tok'����'src'�ֽ⵽'trg'�У����'tok'�����ã���*/
/*����'src'��len����Сֵ����'src'������'trg'�С����Խ�������г��ֵ���  */
/*'tok'��ͬ�ķָ������ֽ⡣						*/
/************************************************************************/
char *stptok(const char *src,char *trg,int len,const char *tok)
{
	register unsigned char *p;
	if(trg) *trg = 0;
	p = (unsigned char *)src;
	if(!p || !(*p)) return (char *)src ;
	if(tok && *tok) {
		while(*p) {
			if(strchr(tok, *p)) {
				if((p > (unsigned char *)src) && GBK_flag && firstcc((unsigned char *)src, p-1)) {
					p++;
					continue;
				}
				break;
			}
			p++;
		}
	} else {
		size_t l=strlen(src);
		p=(unsigned char *)src+MIN(l,(size_t)len);
	}
	if(len==0) return (char *)p;
	if(!trg) return ((size_t)((const char *)p-src))<len?(char *)p:(char *)(src+len);

	while(*src && --len) {
		if((unsigned char *)src == p) 
			break;
		*trg++ = *src++;
	}
	*trg = 0;

	return (char *)src;
}

/************************************************************************
 *function:strsubst �滻�ַ�������	        
 *description:��'str'�滻'from'��ǰcnt���ַ�	
 * ����ָ���滻��ߵ��ֽڡ� 
 ************************************************************************/
char *strsubst(char *from,int cnt,char *str)
{
	int i;
	register char *cp, *cp1, *cp2;

 	if(!from) 
		return 0;
	i = strlen(from);
	if(cnt < 0) 
		cnt = 0;
	else if(cnt > i) 
		cnt = i;
	else ;
	i = str ? strlen(str) : 0;
	if(i < cnt)				/* delete some char*/
	{  
		cp1 = from + i;
		cp = from + cnt;
		while(*cp) 
			*cp1++ = *cp++;
		*cp1 = 0;
	}
	else if (i > cnt)			/* extend some*/
	{ 
		cp2 = from + cnt;
		cp = from + strlen(from);
		cp1 = cp + i - cnt;
		while(cp >= cp2) 
			*cp1-- = *cp--;
	}
	else ;
	if(str) 
		strncpy(from, str, i);

	return (from + i);
}
/************************************************************************
 *function:strins �����ַ�����                                          
 *description:��'str'ǰ�����ַ�'ch'	
 * ����ָ���滻��ߵ��ֽڡ�
 ************************************************************************/
char *strins(char *str,char ch)
{
	char p[2];

	p[0] = ch;
	p[1] = 0;

	return strsubst(str, 0, p);
}


/************************************************************************/
/*function:strupper ���ַ����е�Сд��ĸת���ɴ�д                      */
/************************************************************************/
char *strupper(char *str)
{
register	char *p;

	if(!str) 
		return str;
	for(p = str; *p; p++) {
		if((*p & 128) && GBK_flag) {
			p++;
			continue;
		}
		*p=toupper(*p);
//		if(islower(*p)) *p -= 'a'-'A';
	}

	return str;
}

/************************************************************************/
/*function:strlower ���ַ����еĴ�д��ĸת����Сд                      */
/************************************************************************/
char *strlower(char *str)
{
	char *p;

	if(!str) 
		return str;
	for(p = str; *p; p++) {
		if((*p & 128) && GBK_flag) {
			p++;
			continue;
		}
		*p=tolower(*p);
//		if(isupper(*p)) *p += 'a'-'A';
	}

	return str;
}
