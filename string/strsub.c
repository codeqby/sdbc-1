#include <stdlib.h>

/************************************************************************/
/*function:strsub ��ȡ�ַ�������                                        */
/*description:'src'�н�ȡ��start��ʼ��cnt���ַ���'dest'��               */
/************************************************************************/
void strsub(char *dest, const char *src,int start,int cnt)
{
    strncpy(dest, src + start, cnt);
    dest[cnt] = '\0';
}

