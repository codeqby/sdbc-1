#include <strproc.h>
#include <bignum.h>
#include <dw.h>
#include <enigma.h>
#include <crc32.h>

extern char *(*encryptproc)(char *mstr);
// �̲߳���ȫ
static char keyid[20]="";
static char *encryptpass(char *mstr)
{
ENIGMA2 egm;
int ret;
char tmp[41];
        if(!*keyid) return mstr;
        ret=a64_byte(tmp,mstr);
	enigma2_init(&egm,keyid,0);
	enigma2_decrypt(&egm,tmp,ret);
        tmp[ret]=0;
        strcpy(mstr,tmp);
        return mstr;
}

char *decodeprepare(char *dblabel)
{
char *p;
DWS dw;
int ret;
/********************************************************************
 * �û��������׼��
 ********************************************************************/
	p=getenv("KEYFILE");
	if(!p||!*p) {
		ShowLog(1,"ȱ�ٻ������� KEYFILE");
		ret=-1;
	} else {
		ret=initdw(p,&dw);
		if(ret) {
			ShowLog(1,"Init dw %s error %d",p,ret);
		}
	}
	if(dblabel && *dblabel) p=dblabel;
	else {
		p=getenv("DBLABEL");
		if(!p||!*p) {
			p=NULL;
			ret=-1;
		}
	}
	if(!ret) {
unsigned crc;
char *cp;
		crc=ssh_crc32((const unsigned char *)p,strlen(p));
		cp=getdw(crc,&dw);
		if(!cp) {
			freedw(&dw);
			ShowLog(1,"%s:��Ч�� KEYID %s",__FUNCTION__,p);
		} else {
			strcpy(keyid,cp);
			encryptproc=encryptpass;
		}
		freedw(&dw);
	}
	return p;
}

