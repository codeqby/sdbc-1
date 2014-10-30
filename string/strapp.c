#include <strproc.h>

/************************************************************************/
/*function:trim ȥ���ַ���ǰ��ո�	                                    */
/************************************************************************/
/*char *trim(char *src)
{
	int i = 0;
	char *ptr = src;

	while(src[i] != '\0')
	{
		if(src[i] != ' ')
			break;
		else
			ptr++;
		i++;
	}
	for(i = strlen(src)-1; i >= 0; i--)
	{
		if(src[i] != ' ')
			break;
		else
			src[i] = '\0';
	}

	return ptr;
}
*/
#define ISSPACE(ch) ((ch)==' '||(ch)=='\r'||(ch)=='\n'||(ch)=='\f'||(ch)=='\b'||(ch)=='\t')
char *trim(char *src)
{
	char *p = src;
	char *ptr;
	if(p)
	{
		ptr = p + strlen(src) - 1;
		while(*p && ISSPACE(*p)) 
			p++;
		while(ptr > p && ISSPACE(*ptr)) 
			*ptr-- = '\0';
	}
	return p;
}

/************************************************************************/
/*function:rtrim ȥ���ַ�����Ŀո�		                                */
/************************************************************************/
char *rtrim(char *str)
{
	unsigned char *cp;

	cp = (unsigned char *)(str + strlen(str));
	while((*cp <=(unsigned char)' ') && (cp >= (unsigned char *)str))
	{
		*cp-- = 0;
	}

	return str;
}

/************************************************************************/
/*function:ltrim ȥ���ַ���ǰ�Ŀո�		                                */
/************************************************************************/
char *ltrim(char* str)
{
	strrevers(str);
	rtrim(str);		//���������rtrim()����
	strrevers(str);
	return str;
}

