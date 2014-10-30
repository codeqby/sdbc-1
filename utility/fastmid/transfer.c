/****************************************
 * һ������ģ�� 
 ***************************************/
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "midsc.h"

#define MIN(a,b) ((a) < (b))?(a):(b)
extern void quit(int n);

#define longaddr(s) LongAddr(s)
//���������ӣ��ӷ������հ������ͻ���
static int from_server(T_Connect *client,T_NetHead *NetHead)
{
int ret=0,i,t_cont;
char tmp[256];
T_SRV_Var *srvp=(T_SRV_Var *)client->Var;
	if(!srvp) {
		NetHead->ERRNO1=-1;
		NetHead->ERRNO2=-1;
		sprintf(tmp,"srvp is null");
		ret=-1;
err2:
		NetHead->data=tmp;
		NetHead->PKG_LEN=strlen(NetHead->data);
		NetHead->PKG_REC_NUM=0;
		NetHead->O_NODE=LocalAddr(client->Socket,0);
		i=SendPack(client,NetHead);
		ShowLog(1,"%s:tid=%lu,%s",__FUNCTION__,pthread_self(),tmp);
		return ret;
	}
GDA *gp=(GDA *)srvp->var;

	if(!gp) {
		sprintf(tmp,"ctx is null");
		NetHead->ERRNO2=-1;
err1:
		if(-2==clr_event(srvp->TCB_no)) {
			ShowLog(1,"%s:TCB:%d err1 ����¼���!",__FUNCTION__,srvp->TCB_no);
		}
		NetHead->ERRNO1=errno;
 	   	ShowLog(1,"%s:tid=%lu,TCB:%d %s",__FUNCTION__,pthread_self(),srvp->TCB_no,tmp);
		ret=-1;
		goto err2;
	}
	if(!gp->server) {
		sprintf(tmp,"event fd=%d, server is null!", get_event_fd(srvp->TCB_no));
		ret=-1;
		goto err1;
	}
T_CLI_Var *clip=(T_CLI_Var *)gp->server->Var;
	if(!clip) {
		sprintf(tmp,"tid=%lu,TCB:%d,clip is empty!",pthread_self(),srvp->TCB_no);
		ret=-1;
		goto err;
	}
//T_CLI_Var *clip=(T_CLI_Var *)gp->server->Var;
	i=get_event_status(srvp->TCB_no);
	if(i!=1) { //EPOLLIN
		sprintf(tmp,"���з��������س�ʱ,event=0X%08X",i);
		NetHead->ERRNO2=-1;
err:
		if(-2==clr_event(srvp->TCB_no)) {
			ShowLog(1,"%s:TCB:%d err ����¼���!",__FUNCTION__,srvp->TCB_no);
		}
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
 	   	ShowLog(1,"%s:tid=%lu,TCB:%d %s",__FUNCTION__,pthread_self(),srvp->TCB_no,tmp);
		ret=0;
		goto err2;
	}
	i=RecvPack(gp->server,NetHead);
	if(i<0){
		sprintf(tmp,"recv from server %s fail i=%d,errno=%d,%s",
			gp->server->Host,i,errno,strerror(errno));
		NetHead->ERRNO2=i;
		if(clip) clip->Errno=-1;
		goto err;
	} 
	t_cont=NetHead->ERRNO2;
	ShowLog(3,"%s:TCB:%d,tid=%lu,Recv from server %s PROTO_NUM=0X%04X,ERRNO1=%d,ERRNO2=%d,PKG_LEN=%d,T_LEN=%d,t_cont=%08X",
		__FUNCTION__,srvp->TCB_no,pthread_self(),
		gp->server->Host,NetHead->PROTO_NUM, NetHead->ERRNO1,
		NetHead->ERRNO2, NetHead->PKG_LEN,NetHead->T_LEN,t_cont);

	i=SendPack(client,NetHead);
//ShowLog(5,"SendPack to client i=%d",i);

	if(t_cont==PACK_CONTINUE) {
//���������Ҫ���������ͣ��Ͳ�����¼������������������������		
//��һ��������Ȼ��:from_server,��һ�������ǣ����������
		return 0;
	}
//��һ��������:SDBC��׼����,��һ�������ǣ����������
	ret=conn_lock(srvp->poolno,clip->NativeError);
	i=clr_event(srvp->TCB_no);
	if(-2==i) {
		ShowLog(1,"%s:TCB:%d,����¼���!",__FUNCTION__,srvp->TCB_no);
	}
//��������������Ѿ����,�ͷŹ�����Դ��
	release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
	i=conn_unlock(srvp->poolno,clip->NativeError);
//ShowLog(5,"%s:lock=%d,unlock=%d",__FUNCTION__,ret,i);
	return 0;
}
//�ڶ������ӣ��ӿͻ����հ�������������
int do_Transfer(T_Connect *client,T_NetHead *NetHead)
{
T_SRV_Var *srvp=(T_SRV_Var *)client->Var;
GDA *gp=(GDA *)srvp->var;
int ret,i;
T_Connect *server; 
char tmp[256];
int proto_num=NetHead->PROTO_NUM;
int t_cont;

	unset_callback(srvp->TCB_no);
	server=gp->server;
	if(!server) {
		sprintf(tmp,"%s:Server connect fault tid=%lu!TCB:%d",__FUNCTION__,
			pthread_self(),srvp->TCB_no);
		NetHead->ERRNO1=NetHead->ERRNO2=-1;
		NetHead->O_NODE=LocalAddr(client->Socket,NULL);
		NetHead->data=tmp;
		NetHead->PKG_LEN=strlen(NetHead->data);
		i=SendPack(client,NetHead);
		ShowLog(1,"%s:%s",__FUNCTION__,tmp);
		return 0;
	}
/* if data is'nt ascii, bug
	ShowLog(5,"send to srv len:%d,SeneLen=%d,RecvLen=%d,data=%s",
		NetHead->PKG_LEN,client->SendLen,client->RecvLen,NetHead->data);
*/
	t_cont=NetHead->ERRNO2;
	i=SendPack(server,NetHead);
ShowLog(3,"%s:TCB:%d,tid=%lu,Send to serv proto_num=0X%04X,t_cont=%d,SendPack ret=%d",
	__FUNCTION__,srvp->TCB_no,pthread_self(),proto_num,t_cont,i);
	if(i){
		NetHead->ERRNO1=errno;
		NetHead->ERRNO2=i;
		sprintf(tmp,"Send to %s fail ret=%d,errno=%d,%s",
			server->Host,i,errno,strerror(errno));
errret:
		ShowLog(1,"%s:tid=%lu,TCB:%d at errret,%s",__FUNCTION__,pthread_self(),srvp->TCB_no,tmp);
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		NetHead->O_NODE=LocalAddr(client->Socket,NULL);
		NetHead->data=tmp;
		NetHead->PKG_LEN=strlen(NetHead->data);
		i=SendPack(client,NetHead);
		return -1;
	} 
	while(t_cont==PACK_CONTINUE) {
    		i=RecvPack(client,NetHead);
		if(i) {
			ShowLog(1,"%s:PACK_CONTINUE tid=%lu,TCB:%d,recv from client ret=%d,errno=%d,%s",__FUNCTION__,
				 pthread_self(),srvp->TCB_no,errno,strerror(errno));
			release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
			return -1;
		}
//ShowLog(5,"Transfer c->s CONTINUE PROTO_NUM=%04X",NetHead->PROTO_NUM);
		t_cont=NetHead->ERRNO2;
//ShowLog(5,"Transfer CONTINUE data=%s",NetHead->data);
    		i=SendPack(server,NetHead);
		if(i) {
			NetHead->ERRNO1=errno;
			NetHead->ERRNO2=i;
			sprintf(tmp,"Send to %s fail ret=%d,errno=%d,%s",
				server->Host,i,errno,strerror(errno));
			goto errret;
		}
	}
	if(t_cont==PACK_NOANSER) {
		ShowLog(2,"%s:tid=%lu,TCB:%d,PACK_NOANSER",__FUNCTION__,srvp->TCB_no);
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		return 0;
	}  
T_CLI_Var *clip=(T_CLI_Var *)server->Var;
//��һ��������:from_server,��һ�������ǣ����������
	ret=conn_lock(srvp->poolno,clip->NativeError);
	i=set_event(srvp->TCB_no,server->Socket,from_server,10);
/*
	if(i) {
		ShowLog(1,"%s:set_event ret=%d",__FUNCTION__,i);
	}
*/
	i=share_lnk(srvp->poolno,server);
	i=conn_unlock(srvp->poolno,clip->NativeError);
	if(i<0) {
		ShowLog(1,"%s:share_lnk ret=%d",__FUNCTION__,i);
	}
//ShowLog(5,"%s:lock=%d,unlock=%d",__FUNCTION__,ret,i);
	return -5;//�ͷű��߳�
}
//��һ�����ӣ�����SDBC�����ճ���ȡ���ӳ�
int Transfer(T_Connect *client,T_NetHead *NetHead)
{
T_SRV_Var *srvp=(T_SRV_Var *)client->Var;
GDA *gp=(GDA *)srvp->var;
int ret,TCBno;
char tmp[256];
	
	if(!srvp) {
		sprintf(tmp,"srvp is null");
	} else {
		TCBno=srvp->TCB_no;
//��һ��������:do_Transfer,��һ�������ǣ����ӳصȴ�����
		ret=get_s_connect(TCBno,srvp->poolno,&gp->server,do_Transfer);
		if(0==ret) return do_Transfer(client,NetHead);//���Ѿ����ˣ�ֱ�ӵ���
		if(ret==1) return -5; //�ͷű��߳�
		sprintf(tmp,"get connect pool fail!");
	}

	NetHead->ERRNO1=-1;
	NetHead->ERRNO2=-1;
   	ShowLog(1,"%s:%s",__FUNCTION__,tmp);
	NetHead->O_NODE=LocalAddr(client->Socket,NULL);
	NetHead->data=tmp;
	NetHead->PKG_LEN=strlen(NetHead->data);
	ret=SendPack(client,NetHead);
	return -1;
}

