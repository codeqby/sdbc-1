#include <unistd.h>
#include "midsc.h"
#include <dw.h>
#include <scry.h>
#include <enigma.h>
#include <crc.h>
#include <crc32.h>

//#include <string.h>

extern srvfunc Function[];

struct login_s {
	char devid[17];
	char label[256];
	char CA[100];
	char uid[17];
	char pwd[14];
};
T_PkgType login_type[]={
	{CH_CHAR,17,"devid",0,-1},
	{CH_CHAR,256,"label"},
	{CH_CHAR,100,"CA"},
	{CH_CHAR,17,"uid"},
	{CH_CHAR,14,"pwd"},
	{-1,0}
};	

static int login_finish(T_Connect *conn,T_NetHead *NetHead);

int login(T_Connect *conn,T_NetHead *NetHead)
{
int ret,crc;
char tmp[200];
char *cp,*key;
char tmp1[1024],cliaddr[20];
DWS dw;
struct login_s logrec;
ENIGMA2 egm;
FILE *fd;
T_SRV_Var *up;
GDA *gp;
//u_int e[RSALEN],m[RSALEN];

	up=(T_SRV_Var *)conn->Var;
	gp=(GDA *)up->var;
	StrAddr(NetHead->O_NODE,cliaddr);
ShowLog(5,"%s:TCB:%d Client IP Addr=%s,Net_login %s",__FUNCTION__,up->TCB_no,cliaddr,NetHead->data);
	net_dispack(&logrec,NetHead->data,login_type);
	strcpy(gp->devid,logrec.devid);
	sprintf(gp->ShowID,"%s:%s:%d",logrec.devid,cliaddr,up->TCB_no);
	mthr_showid_add(up->tid,gp->ShowID);

	if((int)NetHead->ERRNO1>0) conn->MTU=NetHead->ERRNO1;
	if((int)NetHead->ERRNO2>0) conn->timeout=NetHead->ERRNO2;

	cp=getenv("KEYFILE");
	if(!cp||!*cp) {
		strcpy(tmp1,"缺少环境变量 KEYFILE");
		NetHead->ERRNO1=-1;
errret:
		ShowLog(1,"%s:Error %s",__FUNCTION__,tmp1);
		NetHead->ERRNO2=-1;
		NetHead->PKG_REC_NUM=0;
		NetHead->O_NODE=LocalAddr(conn->Socket,NULL);
		NetHead->data=tmp1;
		NetHead->PKG_LEN=strlen(NetHead->data);
    		SendPack(conn,NetHead);
		return 0; // fail
	}
/* read key */
	crc=0;
reopen:
	ret=initdw(cp,&dw);
	if(ret) {
		if((errno==24)&& (++crc<5)) {
			sleep(15);
			goto reopen;
		}
		sprintf(tmp1,"Init dw %s error %d",cp,ret);
		NetHead->ERRNO1=-1;
		goto errret;
	}
	crc=ssh_crc32((unsigned char *)logrec.devid,strlen(logrec.devid));
	key=getdw(crc,&dw);
	if(!key) {
		freedw(&dw);
                sprintf(tmp1,"无效的 DEVID");
		NetHead->ERRNO1=-1;
                goto errret;
        }

//ShowLog(5,"getdw key=%s",key);
	enigma2_init(&egm,key,0);
//check LABEL
	ret=a64_byte(gp->DBLABEL,logrec.label);
	enigma2_decrypt(&egm,gp->DBLABEL,ret);
	gp->DBLABEL[ret]=0;

/* check CA */
	memset(gp->operid,0,sizeof(gp->operid));
	cp=getenv("CADIR");
	if(!cp||!*cp) cp=(char *)".";
    if(strcmp(gp->devid,"REGISTER")) {
	strncpy(gp->operid,logrec.uid,sizeof(gp->operid)-1);
	sprintf(tmp,"%s/%s.CA",cp,logrec.devid);
//ShowLog(5,"CAfile=%s,key=%s",tmp,key);
	fd=fopen(tmp,"r");
	if(!fd) {
		if(errno==2) {
		    crc=strlen(logrec.CA);
		    enigma2_encrypt(&egm,logrec.CA,crc);
		    byte_a64(tmp1,logrec.CA,crc);
//ShowLog(5,"CA=%s",tmp1);
		    fd=fopen(tmp,"w");
		    if(!fd) {
			sprintf(tmp1,"write %s err=%d",tmp,errno);
			NetHead->ERRNO1=-1;
err1:
			freedw(&dw);
			goto errret;
		    }
		    fprintf(fd,"%s\n",tmp1);
		    fclose(fd);
		} else {
			sprintf(tmp1,"open CAfile %s err=%d",tmp,errno);
			NetHead->ERRNO1=-1;
			goto err1;
		}
	} else {
		fgets(tmp1,sizeof(logrec.CA),fd);
		fclose(fd);
		TRIM(tmp1);
		ret=a64_byte(tmp,tmp1);
		enigma2_decrypt(&egm,tmp,ret);
		tmp[ret]=0;
		if(strcmp(tmp,logrec.CA)) {
			sprintf(tmp1,"CA 错误");
ShowLog(1,"%s:%s CA=%s log=%s len=%d",__FUNCTION__,tmp1,tmp,logrec.CA,ret);
			NetHead->ERRNO1=-1;
			goto err1;
		}
	}
    } else {   //未注册客户端注册
	char *p;
	char *keyD;
/* REGISTER label|CA|devfile|CHK_Code| */

ShowLog(2,"REGISTER %s",logrec.uid);
	if(!*logrec.uid) {
		sprintf(tmp1,"REGSTER is empty!");
		NetHead->ERRNO1=-1;
		goto err1;
	}
//uid=devfile
	crc=0xFFFF&gencrc((unsigned char *)logrec.uid,strlen(logrec.uid));
//pwd=CHK_Code
	sscanf(logrec.pwd,"%04X",&ret);
	ret &= 0xFFFF;
	if(ret != crc) {
		sprintf(tmp1,"REGISTER:devfile CHK Code error! ");//, crc,ret);
		NetHead->ERRNO1=-1;
		goto err1;
	}
	p=stptok(logrec.uid,logrec.devid,sizeof(logrec.devid),".");//logrec.devid=准备注册的DEVID
	crc=ssh_crc32((unsigned char *)logrec.devid,strlen(logrec.devid));
	keyD=getdw(crc,&dw);
	if(!keyD) {
		sprintf(tmp1,"注册失败,%s:没有这个设备！",
				logrec.devid);
		NetHead->ERRNO1=-1;
		goto err1;
	}
	enigma2_init(&egm,keyD,2);

	sprintf(tmp,"%s/%s.CA",cp,logrec.devid);
ShowLog(5,"REGISTER:%s",tmp);
	if(0!=(fd=fopen(tmp,"r"))) {
		fgets(tmp1,81,fd);
		fclose(fd);
		TRIM(tmp1);
		ret=a64_byte(tmp,tmp1);
		enigma2_decrypt(&egm,tmp,ret);
		tmp[ret]=0;
		if(strcmp(tmp,logrec.CA)) {
			sprintf(tmp1,"注册失败,%s 已被注册,使用中。",
					logrec.devid);
			NetHead->ERRNO1=-1;
			goto err1;
		}
	} else if(errno != 2) {
		sprintf(tmp1,"CA 错误");
		NetHead->ERRNO1=-1;
		goto err1;
	}
/*把设备特征码写入文件*/
	fd=fopen(tmp,"w");
	if(fd) {
	int len=strlen(logrec.CA);
		enigma2_encrypt(&egm,logrec.CA,len);
		byte_a64(tmp1,logrec.CA,len);
		fprintf(fd,"%s\n",tmp1);
		fclose(fd);
	}
	else ShowLog(1,"net_login:REGISTER open %s for write,err=%d,%s",
		tmp,errno,strerror(errno));

	freedw(&dw);
	sprintf(tmp,"%s/%s",cp,logrec.uid);
	fd=fopen(tmp,"r");
	if(!fd) {
		sprintf(tmp1,"REGISTER 打不开文件 %s err=%d,%s",
					logrec.CA,errno,strerror(errno));
		goto errret;
	}
	fgets(logrec.uid,sizeof(logrec.uid),fd);
	TRIM(logrec.uid);
	ShowLog(2,"REGISTER open %s",tmp);
	fclose(fd);
	cp=tmp1;
	cp+=sprintf(cp,"%s|%s|", logrec.devid,logrec.uid);
	cp+=sprintf(cp,"%s|",rsecstrfmt(tmp,now_sec(),YEAR_TO_SEC));
	NetHead->data=tmp1;
	NetHead->PKG_LEN=strlen(NetHead->data);
	NetHead->ERRNO1=0;
	NetHead->ERRNO2=0;
	NetHead->O_NODE=LocalAddr(conn->Socket,NULL);
	NetHead->PKG_REC_NUM=0;
    	SendPack(conn,NetHead);
	return -1;
    } //未注册客户端注册完成

	freedw(&dw);
	up->poolno=get_scpool_no(NetHead->D_NODE);
	if(up->poolno<0) {
		sprintf(tmp1,"非法的D_NODE %d",NetHead->D_NODE);
		NetHead->ERRNO1=-1;
		goto errret;
	}
	ret=get_s_connect(up->TCB_no,up->poolno,&gp->server,login_finish);
	if(ret==0) return login_finish(conn,NetHead);
	else if(ret==1) return -5;
	sprintf(tmp1,"连接服务器失败!");
	NetHead->ERRNO1=PACK_NOANSER;
	goto errret;
}

static int login_finish(T_Connect *conn,T_NetHead *NetHead)
{
char *cp;
T_SRV_Var *up=(T_SRV_Var *)conn->Var;
GDA *gp=(GDA *)up->var;
T_CLI_Var *clip;
char tmp[30],tmp1[256];
int uz;

	unset_callback(up->TCB_no);
//	up->poolno=get_scpool_no(NetHead->D_NODE);
/* 怎么认证还得想办法,这是按scpool的用户登录的
	cp=get_LABEL(up->poolno);
	if(cp && strcmp(cp,gp->DBLABEL)) {
		sprintf(tmp1,"错误的cp=%s,DBLABEL %s",cp,gp->DBLABEL);
		goto errret;
	}
*/
	if(!gp->server) {
		sprintf(tmp1,"connect to server fault,TCB:%d", up->TCB_no);
errret:
		ShowLog(1,"%s:Error:%s",__FUNCTION__,tmp1);
                NetHead->ERRNO2=-1;
		NetHead->ERRNO1=PACK_NOANSER;
		NetHead->O_NODE=LocalAddr(conn->Socket,NULL);
                NetHead->PKG_REC_NUM=0;
                NetHead->data=tmp1;
                NetHead->PKG_LEN=strlen(NetHead->data);
                SendPack(conn,NetHead);
                return 0; // fail
	}
	uz = (conn->CryptFlg & gp->server->CryptFlg) & DO_ZIP;
	clip=(T_CLI_Var *)gp->server->Var;
	release_SC_connect(&gp->server,up->TCB_no,up->poolno);
	cp=tmp1;
	if(clip) {
		cp+=sprintf(cp,"%s|%s|%s|%s|",
			gp->devid,gp->operid,
			clip->UID,clip->DBOWN);
	} else cp+=sprintf(cp,"%s|%s|||",gp->devid,gp->operid);


	cp+=sprintf(cp,"%s|%d|",rsecstrfmt(tmp,now_sec(),YEAR_TO_SEC),up->TCB_no);
	ShowLog(2,"%s:%s Login success",__FUNCTION__,tmp1);

	NetHead->data=tmp1;
	NetHead->PKG_LEN=strlen(NetHead->data);
	NetHead->ERRNO1=0;
	NetHead->ERRNO2=0;
	NetHead->O_NODE=LocalAddr(conn->Socket,NULL);
	NetHead->PKG_REC_NUM=0;
    	SendPack(conn,NetHead);
	if(uz) conn->CryptFlg |= UNDO_ZIP;
	return 1;
}

