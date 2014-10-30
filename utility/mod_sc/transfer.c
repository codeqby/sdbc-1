/****************************************
 * һ������ģ�� 
 ***************************************/
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "midsc.h"

extern void quit(int n);
static int from_server(T_Connect *client,T_NetHead *NetHead);

int from_client(T_Connect *client,T_NetHead *NetHead)
{
T_SRV_Var *srvp=(T_SRV_Var *)client->Var;
GDA *gp=(GDA *)srvp->var;
int i,ret;
T_Connect *server; 
char tmp[256];
int proto_num;
unsigned int t_cont;

	unset_callback(srvp->TCB_no);
	server=gp->server;
	i=get_event_status(srvp->TCB_no);
	if(i!=1) { //EPOLLIN
		sprintf(tmp,"�ͻ���״̬��ʱ,event=0X%08X", i);
clierr:
		((T_CLI_Var *)gp->server->Var)->Errno=-1;
		client->status=0;
		ShowLog(1,"%s:tid=%lu,TCB:%d at clierr,%s",__FUNCTION__,
			srvp->tid,srvp->TCB_no,tmp);
		((T_CLI_Var *)gp->server->Var)->Errno=-1;//״̬ز���ˣ�����ر�����
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		return -1;
	}
	i=RecvPack(client,NetHead);
	if(i<0){
		sprintf(tmp,"Recv client fail ret=%d,errno=%d,%s",
			i,errno,strerror(errno));
		goto clierr;
	} 
	t_cont=NetHead->ERRNO2;
	proto_num=NetHead->PROTO_NUM;
	i=SendPack(server,NetHead);
	ShowLog(3,"%s:TCB:%d,tid=%lu,Send to serv proto_num=0X%04X,t_cont=%d,SendPack ret=%d,USEC=%lld",
		__FUNCTION__,srvp->TCB_no,srvp->tid,proto_num,t_cont,i,now_usec());
	if(i){
		NetHead->ERRNO1=errno;
		NetHead->ERRNO2=PACK_NOANSER;
		sprintf(tmp,"Send to %s fail ret=%d,errno=%d,%s",
			server->Host,i,errno,strerror(errno));
		ret=0;
		client->status=0;
		((T_CLI_Var *)gp->server->Var)->Errno=-1;//״̬ز���ˣ�����ر�����
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		if(t_cont!=PACK_NOANSER) {
			NetHead->O_NODE=LocalAddr(client->Socket,NULL);
			NetHead->ERRNO2=PACK_NOANSER;
			NetHead->data=tmp;
			NetHead->PKG_LEN=strlen(NetHead->data);
			i=SendPack(client,NetHead);
		}
		ShowLog(1,"%s:tid=%lu,TCB:%d at errret,%s",__FUNCTION__,
			srvp->tid,srvp->TCB_no,tmp);
		return ret;
	} 
	if(t_cont==PACK_NOANSER) {
		client->status=0;
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		ShowLog(2,"%s:tid=%lu,TCB:%d,PACK_NOANSER",__FUNCTION__,srvp->tid,srvp->TCB_no);
		return 0;
	}  
	if(t_cont==PACK_CONTINUE) {
		set_callback(srvp->TCB_no,from_client,60);
		return 0;
	}
	if(t_cont != PACK_STATUS) client->status=0;
//��һ��������:from_server,��һ�������ǣ����������
	i=set_event(srvp->TCB_no,server->Socket,from_server,60);//��״ֻ̬����60��
	if(i) {
		ShowLog(1,"%s:TCB:%d set_event error %d",__FUNCTION__,
			srvp->TCB_no,i);
		return -1;
	}
	return -5;//�ͷű��߳�
}

//���������ӣ��ӷ������հ������ͻ���
static int from_server(T_Connect *client,T_NetHead *NetHead)
{
int i,mflg=0;
unsigned t_cont;
char tmp[256];
T_SRV_Var *srvp=(T_SRV_Var *)client->Var;
int efd,ret=0;
int proto_num=NetHead->PROTO_NUM&0xFFFF;
	if(!srvp) {
		NetHead->ERRNO1=-1;
		NetHead->ERRNO2=-1;
		sprintf(tmp,"srvp is null");
		ret=-1;
err2:
		ShowLog(1,"%s:tid=%lu,%s",__FUNCTION__,pthread_self(),tmp);
		NetHead->data=tmp;
		NetHead->PKG_LEN=strlen(NetHead->data);
		NetHead->PKG_REC_NUM=0;
		NetHead->O_NODE=LocalAddr(client->Socket,0);
		client->CryptFlg &= ~UNDO_ZIP;
		i=SendPack(client,NetHead);
		return ret;
	}
GDA *gp=(GDA *)srvp->var;

	if(!gp) {
		sprintf(tmp,"ctx is null");
err1:
		NetHead->ERRNO2=-1;
		if(-2==clr_event(srvp->TCB_no)) {
			ShowLog(1,"%s:TCB:%d gp empty ����¼���!",__FUNCTION__,srvp->TCB_no);
		}
		NetHead->ERRNO1=errno;
 	   	ShowLog(1,"%s:tid=%lu,TCB:%d %s",__FUNCTION__,srvp->tid,srvp->TCB_no,tmp);
		ret=-1;
		goto err2;
	}
T_Connect *server=gp->server;
	if(!server) {
		sprintf(tmp,"event fd=%d, server is null!", get_event_fd(srvp->TCB_no));
		ret=-1;
		goto err1;
	}
	i=get_event_status(srvp->TCB_no);
	if(i!=1) { //EPOLLIN
		efd=get_event_fd(srvp->TCB_no);
		sprintf(tmp,"���з�����%s/%s���س�ʱ,event=0X%08X,fd=%d",
			gp->server->Host,gp->server->Service,i,efd);
		NetHead->ERRNO1=PACK_NOANSER;
		NetHead->ERRNO2=-1;
		((T_CLI_Var *)server->Var)->Errno=-1;
err:
		if(-2==clr_event(srvp->TCB_no)) {
			ShowLog(1,"%s:TCB:%d err ����¼���!",__FUNCTION__,srvp->TCB_no);
		}
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		ret=0;
		goto err2;
	}
	i=RecvPack(server,NetHead);
	if(i<0){
		sprintf(tmp,"recv from server %s fail i=%d,errno=%d,%s",
			server->Host,i,errno,strerror(errno));
		NetHead->ERRNO1=PACK_NOANSER;
		NetHead->ERRNO2=errno;
		((T_CLI_Var *)gp->server->Var)->Errno=-1;
		goto err;
	} 
	t_cont=NetHead->ERRNO2;
	if(t_cont==PACK_STATUS) client->status=1;//�ɷ����������״̬�������ɿͻ���ά�ֺͳ���.
	if(t_cont!=PACK_CONTINUE || client->status) {
//��һ��������:SDBC��׼����,��һ�������ǣ����������
		i=clr_event(srvp->TCB_no);
		if(-2==i) {
			ShowLog(1,"%s:TCB:%d,����¼���!",__FUNCTION__,srvp->TCB_no);
		}
		else if(client->status) { //��״̬�����ͷ���Դ������client 
//��һ��������:from_client,��һ�������ǣ����������
			ShowLog(3,"%s:TCB:%d,tid=%lu,Status Recv from server %s PROTO_NUM=0X%04X,"
		  		"ERRNO1=%d,ERRNO2=%d,PKG_LEN=%d,T_LEN=%d,t_cont=%08X,USEC=%lld",
					__FUNCTION__,srvp->TCB_no,srvp->tid,
					server->Host,proto_num, NetHead->ERRNO1, NetHead->ERRNO2,
					NetHead->PKG_LEN,NetHead->T_LEN,t_cont, now_usec());
			if(0!=(i=SendPack(client,NetHead)) ||
			   0!=(i=set_event(srvp->TCB_no,client->Socket,from_client,60))) {//��״ֻ̬����60��
				ShowLog(1,"%s:TCB:%d set_event error %d",__FUNCTION__,
					srvp->TCB_no,i);
				return -1;
			}
			return -5;//�ͷű��߳�
		}
//��������������Ѿ����,�ͷŹ�����Դ��
		if(NetHead->PKG_LEN) {
			server->RecvLen=0;
			server->RecvBuffer=NULL;
			mflg=1;
		}
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
	}
	i=SendPack(client,NetHead);
	ShowLog(3,"%s:TCB:%d,tid=%lu,Recv from server %s PROTO_NUM=0X%04X,"
		  "ERRNO1=%d,ERRNO2=%d,PKG_LEN=%d,T_LEN=%d,t_cont=%08X,USEC=%lld",
			__FUNCTION__,srvp->TCB_no,srvp->tid,
			server->Host,proto_num, NetHead->ERRNO1, NetHead->ERRNO2,
			NetHead->PKG_LEN,NetHead->T_LEN,t_cont, now_usec());

	if(mflg) {
		free(NetHead->data);
		NetHead->data=NULL;
		NetHead->PKG_LEN=0;
	}
	return 0;
}
//�ڶ������ӣ��ӿͻ����հ�������������
int do_Transfer(T_Connect *client,T_NetHead *NetHead)
{
T_SRV_Var *srvp=(T_SRV_Var *)client->Var;
GDA *gp=(GDA *)srvp->var;
int i;
T_Connect *server; 
char tmp[256];
unsigned int t_cont;

	unset_callback(srvp->TCB_no);
	server=gp->server;
	if(!server) {
		sprintf(tmp,"Server connect fault tid=%lu!TCB:%d",
			pthread_self(),srvp->TCB_no);
		NetHead->ERRNO1=PACK_NOANSER;
		NetHead->ERRNO2=-1;
		NetHead->O_NODE=LocalAddr(client->Socket,NULL);
		NetHead->data=tmp;
		NetHead->PKG_LEN=strlen(NetHead->data);
		i=SendPack(client,NetHead);
		ShowLog(1,"%s:%s",__FUNCTION__,tmp);
		return 0;
	}
	if(client->CryptFlg & UNDO_ZIP) server->CryptFlg |= UNDO_ZIP;
	t_cont=NetHead->ERRNO2;
	i=SendPack(server,NetHead);
	ShowLog(3,"%s:TCB:%d,tid=%lu,Send to serv proto_num=0X%04X,t_cont=%d,SendPack ret=%d,USEC=%lld",
		__FUNCTION__,srvp->TCB_no,srvp->tid,NetHead->PROTO_NUM&0xFFFF,t_cont,i,now_usec());
	if(i){
		NetHead->ERRNO1=errno;
		NetHead->ERRNO2=i;
		sprintf(tmp,"Send to %s fail ret=%d,errno=%d,%s",
			server->Host,i,errno,strerror(errno));
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		if(t_cont!=PACK_NOANSER) {
			NetHead->O_NODE=LocalAddr(client->Socket,NULL);
			NetHead->ERRNO2=PACK_NOANSER;
			NetHead->data=tmp;
			NetHead->PKG_LEN=strlen(NetHead->data);
			i=SendPack(client,NetHead);
		}
		ShowLog(1,"%s:tid=%lu,TCB:%d at errret,%s",__FUNCTION__,
			srvp->tid,srvp->TCB_no,tmp);
		return 0;
	} 
	if(t_cont==PACK_NOANSER) {
		release_SC_connect(&gp->server,srvp->TCB_no,srvp->poolno);
		ShowLog(2,"%s:tid=%lu,TCB:%d,PACK_NOANSER",__FUNCTION__,srvp->tid,srvp->TCB_no);
		return 0;
	}  
	if(t_cont==PACK_CONTINUE) {
		set_callback(srvp->TCB_no,from_client,60);
		return 0;
	}
//��һ��������:from_server,��һ�������ǣ����������
	i=set_event(srvp->TCB_no,server->Socket,from_server,server->timeout);
	if(i) {
		ShowLog(1,"%s:TCB:%d set_event error %d",__FUNCTION__,
			srvp->TCB_no,i);
		return -1;
	}
	return -5;//�ͷű��߳�
}
//��һ�����ӣ�����SDBC�����ճ���ȡ���ӳ�
int Transfer(T_Connect *client,T_NetHead *NetHead)
{
T_SRV_Var *srvp=(T_SRV_Var *)client->Var;
GDA *gp;
int ret,TCBno;
char tmp[256];
	
	if(!srvp) {
		sprintf(tmp,"srvp is null");
		NetHead->ERRNO1=-1;
	} else {
		gp=(GDA *)srvp->var;
		if(NetHead->ERRNO2==PACK_STATUS)
			client->status=1;
		TCBno=srvp->TCB_no;
//��һ��������:do_Transfer,��һ�������ǣ����ӳصȴ�����
		ret=get_s_connect(TCBno,srvp->poolno,&gp->server,do_Transfer);
		if(0==ret) return do_Transfer(client,NetHead);//���Ѿ����ˣ�ֱ�ӵ���
		if(ret==1) return -5; //�ͷű��߳�
		sprintf(tmp,"get connect pool fail!");
		NetHead->ERRNO1=PACK_NOANSER;
	}

	NetHead->ERRNO1=-1;
	NetHead->ERRNO2=-1;
   	ShowLog(1,"%s:%s",__FUNCTION__,tmp);
	NetHead->O_NODE=LocalAddr(client->Socket,NULL);
	NetHead->data=tmp;
	NetHead->PKG_LEN=strlen(NetHead->data);
	client->CryptFlg &= ~UNDO_ZIP;
	ret=SendPack(client,NetHead);
	return 0;
}

