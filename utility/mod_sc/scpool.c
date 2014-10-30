/**************************************************
 * SDBC���ӳع���
 **************************************************/
#include <ctype.h>
#include <sys/utsname.h>
#include <bignum.h>
#include <pack.h>
#include <json_pack.h>
#include <scsrv.h>
#include <sccli.h>
#include "scpool.h"
#include <scry.h>

#include "logs.tpl"
#include "logs.stu"
static int log_level=0;

T_PkgType SCPOOL_tpl[]={
        {CH_INT,sizeof(int),"d_node",0,-1},
        {CH_CHAR,17,"DEVID"},
        {CH_CHAR,256,"LABEL"},
        {CH_CHAR,17,"UID"},
        {CH_CHAR,14,"PWD"},
        {CH_INT,sizeof(int),"NUM"},
        {CH_INT,sizeof(int),"NEXT_d_node"},
        {CH_CHAR,81,"HOST"},
        {CH_CHAR,21,"PORT"},
        {CH_INT,sizeof(int),"MTU"},
        {CH_CHAR,172,"family"},
        {-1,0,0,0}
};

extern T_PkgType SCPOOL_tpl[];
typedef struct {
        int d_node;
        char DEVID[17];
        char LABEL[256];
        char UID[17];
        char PWD[14];
        int NUM;
        int NEXT_d_node;
        char HOST[81];
        char PORT[21];
        int MTU;
        char family[172];
} SCPOOL_stu;

typedef struct {
	int next;
	int TCBno;
	T_Connect Conn;
	T_CLI_Var cli;
	INT64 timestamp;
} resource;

typedef struct {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	int d_node;
	int resource_num;
	SCPOOL_stu log;
	u_int family[32];
	svc_table svc_tbl;
	resource *lnk;
	int free_q;
}pool;

static int SCPOOLNUM=0;
static pool *scpool=NULL;
//�ͷ����ӳ�  
void scpool_free()
{
int i,n;

	if(!scpool) return;
	for(n=0;n<SCPOOLNUM;n++) {
		pthread_cond_destroy(&scpool[n].cond);
		pthread_mutex_destroy(&scpool[n].mut);
		if(scpool[n].lnk) {
			for(i=0;i<scpool[n].resource_num;i++) {
			    if(scpool[n].lnk[i].Conn.Socket > -1) {
				disconnect(&scpool[n].lnk[i].Conn);
			    }
			}
			free(scpool[n].lnk);
		}
	}
	free(scpool);
	scpool=NULL;
}

//��ʼ�����ӳ�  
int scpool_init()
{
int n,i,ret;
char *p,buf[512];
INT64 now;
FILE *fd;
JSON_OBJECT cfg,json;
SCPOOL_stu node;

	if(scpool) return 0;
	p=getenv("SCPOOLCFG");
	if(!p||!*p) {
		ShowLog(1,"%s:ȱ�ٻ�������SCPOOLCFG!",__FUNCTION__);
		return -1;
	}
	fd=fopen((const char *)p,"r");
	if(!fd) {
		ShowLog(1,"%s:CFGFILE %s open err=%d,%s",__FUNCTION__,
			p,errno,strerror(errno));
		return -2;
	}
	cfg=json_object_new_array();
	while(!ferror(fd)) {
		fgets(buf,sizeof(buf),fd);
		if(feof(fd)) break;
		TRIM(buf);
		if(!*buf || *buf=='#') continue;
		ret=net_dispack(&node,buf,SCPOOL_tpl);
		if(ret<=0) continue;
		json=json_object_new_object();
		struct_to_json(json,&node,SCPOOL_tpl,0);
		json_object_array_add(cfg,json);
	}
	fclose(fd);
	SCPOOLNUM=json_object_array_length(cfg);
	if(SCPOOLNUM <=0 ) {
		json_object_put(cfg);
		ShowLog(1,"%s:empty SCPOOL",__FUNCTION__);
		return -3;
	}
	scpool=(pool *)malloc(SCPOOLNUM * sizeof(pool));
	if(!scpool) {
		json_object_put(cfg);
		SCPOOLNUM=0;
		return MEMERR;
	}

	p=getenv("SCPOOL_LOGLEVEL");
	if(p && isdigit(*p)) log_level=atoi(p);

	now=now_usec();
    for(n=0;n<SCPOOLNUM;n++) {

	if(0!=(i=pthread_mutex_init(&scpool[n].mut,NULL))) {
		ShowLog(1,"%s:mutex_init err %s",__FUNCTION__,
			strerror(i));
		json_object_put(cfg);
		return -12;
	}
	
	if(0!=(i=pthread_cond_init(&scpool[n].cond,NULL))) {
		ShowLog(1,"%s:cond init  err %s",__FUNCTION__,
			strerror(i));
		json_object_put(cfg);
		return -13;
	}
	json=json_object_array_get_idx(cfg,n);
	json_to_struct(&scpool[n].log,json,SCPOOL_tpl);
	scpool[n].d_node=scpool[n].log.d_node;
	scpool[n].svc_tbl.srvn=-1;
	scpool[n].resource_num=scpool[n].log.NUM>0?scpool[n].log.NUM:1;
	scpool[n].lnk=(resource *)malloc(scpool[n].resource_num * sizeof(resource));
	if(!scpool[n].lnk) {
		ShowLog(1,"%s:malloc lnk error!",__FUNCTION__);
		scpool[n].resource_num=0;
		continue;
	}
	scpool[n].free_q=scpool[n].resource_num-1;
	for(i=0;i<scpool[n].resource_num;i++) {
		scpool[n].lnk[i].TCBno=-1;
		Init_CLI_Var(&scpool[n].lnk[i].cli);
		scpool[n].lnk[i].cli.Errno=-1;
		initconnect(&scpool[n].lnk[i].Conn);
		scpool[n].lnk[i].Conn.pos=i;
		strcpy(scpool[n].lnk[i].Conn.Host,scpool[n].log.HOST);
		strcpy(scpool[n].lnk[i].Conn.Service,scpool[n].log.PORT);
		if(*scpool[n].log.family)
			str_a64n(32,scpool[n].log.family,scpool[n].family);
		if(i<scpool[n].resource_num-1) scpool[n].lnk[i].next=i+1;
                else scpool[n].lnk[i].next=0;
		scpool[n].lnk[i].timestamp=now;
	}
	ShowLog(2,"scpool[%d],link num=%d",n,scpool[n].resource_num);
    }
	json_object_put(cfg);
	return SCPOOLNUM;
}
static int lnk_no(pool *pl,T_Connect *conn)
{
int i,e;
resource *rs=pl->lnk;

      	if(conn->pos<pl->resource_num ) {
		if(conn != &pl->lnk[conn->pos].Conn) {
       	 		ShowLog(1,"%s:conn not equal pos=%d",__FUNCTION__, conn->pos);
		} else return conn->pos;
      	} 
	e=pl->resource_num-1;
	for(i=0;i<=e;i++,rs++) {
		if(conn == &rs->Conn) {
			conn->pos=i;
			return i;
		}
	}
	return -1;

}
static int get_lnk_no(pool *pl)
{
int i,*ip,*np;
resource *rs;

        if(pl->free_q<0) return -1;
	ip=&pl->free_q;
	rs=&pl->lnk[*ip];
        i=rs->next;
	np=&pl->lnk[i].next;
        if(i==*ip) *ip=-1;
        else rs->next=*np;
        *np=-1;
        return i;
}

static void add_lnk(pool *pl,int i)
{
int *np,*ip=&pl->lnk[i].next;
        if(*ip>=0) return;
	np=&pl->free_q;
        if(*np < 0) {
                *np=i;
                *ip=i;
	} else { //�����ͷ  
	resource *rs=&pl->lnk[*np];
                *ip=rs->next;
                rs->next=i;
		if(pl->lnk[i].Conn.Socket<0) *np=i;//�������Ŷ�β 
        }
}
static int log_ret(T_Connect *conn,T_NetHead *head)
{
int ret,event;
T_SRV_Var *srvp=(T_SRV_Var *)conn->Var;
resource *rs;
pool *pl;
T_Connect *Conn;
T_NetHead Head;
sdbcfunc f;
log_stu logs;

	event=get_event_status(srvp->TCB_no);
	clr_event(srvp->TCB_no);
//ShowLog(5,"%s:tid=%lu,poolno=%d,entry TCB:%d",__FUNCTION__,pthread_self(),srvp->poolno,srvp->TCB_no);
	if(srvp->poolno < 0) {
		ShowLog(1,"%s:tid=%lu,poolno=%d,entry TCB:%d",__FUNCTION__,
			pthread_self(),srvp->poolno,srvp->TCB_no);
		unbind_sc(srvp->TCB_no);
		return -1;
	}
	pl=&scpool[srvp->poolno];
	rs=pl->lnk;
	for(ret=0;ret<pl->resource_num;ret++,rs++) {
		if(rs->TCBno == srvp->TCB_no) break;
	}
	if(ret == pl->resource_num) {
		unbind_sc(srvp->TCB_no);
		return -1;
	}
	Conn=&rs->Conn;
	f=Conn->only_do;
	set_callback(srvp->TCB_no,Conn->only_do,Conn->timeout);
	Conn->only_do=NULL;
        if(event!=1) { //EPOLLIN
                ShowLog(1,"%s:���з��������س�ʱ,%08X",__FUNCTION__,ret);
		unbind_sc(srvp->TCB_no);
		rs->cli.Errno=-1;
                release_SC_connect(&Conn,srvp->TCB_no,srvp->poolno);
		TCB_add(NULL,srvp->TCB_no); //���뵽���������
		return -5;
        }
ShowLog(5,"%s:TCB=%d,poolno=%d.%d",__FUNCTION__,srvp->TCB_no,srvp->poolno,ret);

	ret=RecvPack(Conn,&Head);
	if(ret) { //����ʧ��
		rs->cli.Errno=errno;
		stptok(strerror(errno),rs->cli.ErrMsg,sizeof(rs->cli.ErrMsg),0);
		unbind_sc(srvp->TCB_no);
		ShowLog(1,"%s:network error %d,%s",__FUNCTION__,rs->cli.Errno,rs->cli.ErrMsg);
		rs->cli.Errno=-1;
		release_SC_connect(&Conn,srvp->TCB_no,srvp->poolno);
		TCB_add(NULL,srvp->TCB_no); //���뵽���������
		return -5;
	}
	if(Head.ERRNO1 || Head.ERRNO2) {  //loginʧ��
		ShowLog(1,"%s:HOST=%s,login error ERRNO1=%d,ERRNO2=%d,%s",__FUNCTION__,
			Conn->Host,Head.ERRNO1,Head.ERRNO2,Head.data);
errret:
		unbind_sc(srvp->TCB_no);
		stptok(Head.data,rs->cli.ErrMsg,sizeof(rs->cli.ErrMsg),0);
		rs->cli.Errno=-1;
		release_SC_connect(&Conn,srvp->TCB_no,srvp->poolno);
		TCB_add(NULL,srvp->TCB_no); //���뵽���������
		return -5;
	}
	net_dispack(&logs,Head.data,log_tpl);
	strcpy(rs->cli.DBOWN,logs.DBOWN);
	strcpy(rs->cli.UID,logs.DBUSER);
//ȡ������
	rs->cli.svc_tbl=&pl->svc_tbl;
	pthread_mutex_lock(&pl->mut);
	if(pl->svc_tbl.srvn <= 0) {
	    ret=init_svc_no(&rs->Conn);
	    if(ret) { //ȡ������ʧ��
		pthread_mutex_unlock(&pl->mut);
		ShowLog(1,"%s:HOST=%s,init_svc_no error ERRNO1=%d,ERRNO2=%d,%s",__FUNCTION__,
			Conn->Host,Head.ERRNO1,Head.ERRNO2,Head.data);
		goto errret;
	    }
	} else {
		pl->svc_tbl.usage++;
                Conn->freevar=(void (*)(void *)) free_srv_list;
        }

	pthread_mutex_unlock(&pl->mut);
	rs->cli.Errno=ret;
	*rs->cli.ErrMsg=0;
	return f(conn,head);
}

static int to_log_ret(T_Connect *conn,T_NetHead *head)
{
int ret;
T_SRV_Var *srvp=(T_SRV_Var *)conn->Var;
resource *rs;
pool *pl;

	unset_callback(srvp->TCB_no);
	if(srvp->poolno<0) {
		ShowLog(1,"%s:tid=%lu,poolno=%d,entry TCB:%d",__FUNCTION__,
			pthread_self(),srvp->poolno,srvp->TCB_no);
		unbind_sc(srvp->TCB_no);
		return -1;
	}
	pl=&scpool[srvp->poolno];
	rs=pl->lnk;
	for(ret=0;ret<pl->resource_num;ret++,rs++) {
		if(rs->TCBno == srvp->TCB_no) break;
	}
	if(ret == pl->resource_num) {
		unbind_sc(srvp->TCB_no);
		return -1;
	}
//���񽫻ص�epoll���� 
	set_event(srvp->TCB_no,rs->Conn.Socket,log_ret,rs->Conn.timeout);
	return -5;
}

//����
static int sc_connect(pool *p1,resource *rs)
{
int ret=-1;
T_NetHead Head;
struct utsname ubuf;
char buf[270],finger[256],*p,addr[20];

	ret=Net_Connect(&rs->Conn,&rs->cli,*p1->log.family?p1->family:NULL);
	if(ret) {
		rs->cli.Errno=errno;
		stptok(strerror(errno),rs->cli.ErrMsg,sizeof(rs->cli.ErrMsg),0);
		return -1;
	}

	p=getenv("TIMEOUT");
	if(p && isdigit(*p)) {
		Head.ERRNO2=60*atoi(p);
	} else Head.ERRNO2=0;
	p=getenv("TCPTIMEOUT");
	if(p && isdigit(*p)) {
		rs->Conn.timeout=atoi(p);
	} else rs->Conn.timeout=0;

//login
	uname(&ubuf);
	p=buf;
	Head.O_NODE=LocalAddr(rs->Conn.Socket,addr);
	fingerprint(finger);
	p+=sprintf(p,"%s|%s|%s,%s|||",p1->log.DEVID,p1->log.LABEL,
		ubuf.nodename,finger);
	rs->Conn.MTU=p1->log.MTU;
	Head.PROTO_NUM=0;
	Head.D_NODE=p1->log.NEXT_d_node;
	Head.ERRNO1=rs->Conn.MTU;
	Head.PKG_REC_NUM=0;
	Head.data=buf;
	Head.PKG_LEN=strlen(Head.data);
	ret=SendPack(&rs->Conn,&Head);
//���񽫻ص�rdy���� 
	rs->Conn.only_do=set_callback(rs->TCBno,to_log_ret,rs->Conn.timeout);
	if(!rs->Conn.only_do) {
		rs->Conn.only_do=(sdbcfunc)1;
	}
	return 0;
}

//ȡ����  
T_Connect * get_SC_connect(int TCBno,int n,int flg)
{
int i,ret;
pool *pl;
resource *rs;
pthread_t tid=pthread_self();

	if(!scpool || n<0 || n>=SCPOOLNUM) return NULL;
	pl=&scpool[n];
	if(!pl->lnk) {
		ShowLog(1,"%s:��Ч�����ӳ�[%d]",__FUNCTION__,n);
		return NULL;
	}
	if(0!=pthread_mutex_lock(&pl->mut))  return (T_Connect *)-1;
	while(0>(i=get_lnk_no(pl))) {
	    if(flg) {	//flg !=0,don't wait
		pthread_mutex_unlock(&pl->mut);
		return NULL;
	    }
//	    if(log_level) ShowLog(log_level,"%s tid=%lu pool[%d] suspend",
//		__FUNCTION__,pthread_self(),n);
	    pthread_cond_wait(&pl->cond,&pl->mut); //û����Դ���ȴ� 
//	    if(log_level) ShowLog(log_level,"%s tid=%lu pool[%d] weakup",
//		__FUNCTION__,pthread_self(),n);
	}
	pthread_mutex_unlock(&pl->mut);
	rs=&pl->lnk[i];
	rs->Conn.pos=i;
	rs->TCBno=TCBno;
	rs->timestamp=now_usec();
	if(rs->Conn.Socket<0 || rs->cli.Errno<0) {
		ret=sc_connect(pl,rs);
		if(ret) {
			ShowLog(1,"%s:scpool[%d].%d ����%s/%s��:err=%d,%s",
				__FUNCTION__,n,i,pl->log.HOST,pl->log.PORT,
				rs->cli.Errno, rs->cli.ErrMsg);
			rs->TCBno=-1;
			rs->cli.Errno=-1;
			pthread_mutex_lock(&pl->mut);
			add_lnk(pl,i);
			pthread_mutex_unlock(&pl->mut);
			return (T_Connect *)-1;
		} 
	} 
	if(log_level) ShowLog(log_level,"%s tid=%lu,TCB:%d,pool[%d].%d,USEC=%llu",__FUNCTION__,
			tid,TCBno,n,i,rs->timestamp);
	rs->cli.Errno=0;
	*rs->cli.ErrMsg=0;
	return &rs->Conn;
}
//�黹����  
void release_SC_connect(T_Connect **Connect,int TCBno,int n)
{
int i;
pthread_t tid=pthread_self();
pool *pl;
resource *rs;
T_CLI_Var *clip;
//ShowLog(5,"%s:tid=%lu,TCB:%d,poolno=%d",__FUNCTION__,pthread_self(),TCBno,n);
	if(!Connect || !scpool || n<0 || n>=SCPOOLNUM) {
		ShowLog(1,"%s:TCB:%d,poolno=%d,����Ĳ���",__FUNCTION__,TCBno,n);
		return;
	}
	if(!*Connect) {
		ShowLog(1,"%s:TCB:%d,Conn is Empty!",__FUNCTION__,TCBno);
		return;
	}
	(*Connect)->CryptFlg &= ~UNDO_ZIP;
	clip=(T_CLI_Var *)((*Connect)->Var);
	pl=&scpool[n];
	i=lnk_no(pl,*Connect);
	if(i>=0) {
		rs=&pl->lnk[i];
if(TCBno!=rs->TCBno) {
	ShowLog(1,"%s:TCB:%d,TCB not equal!",__FUNCTION__,TCBno);
}
if(clip != &rs->cli) {
	ShowLog(1,"%s:TCB:%d,clip not equal!",__FUNCTION__,TCBno);
}
		if(rs->cli.Errno==-1) {  //����ʧЧ
			ShowLog(1,"%s:scpool[%d].%d to fail!",__FUNCTION__,n,i);
			disconnect(&rs->Conn);
		}
		rs->TCBno=-1;
		clip->Errno=0;
		*clip->ErrMsg=0;
		
		pthread_mutex_lock(&pl->mut);
		add_lnk(pl,i);
		pthread_mutex_unlock(&pl->mut);
		pthread_cond_signal(&pl->cond); //����еȴ����ӵ��߳̾ͻ����� 
  		rs->timestamp=now_usec();
		*Connect=NULL;
		if(log_level) ShowLog(log_level,"%s tid=%lu,TCB:%d,pool[%d].%d,USEC=%llu",
					__FUNCTION__,tid,TCBno,n,i,rs->timestamp);
		return;
	}
	ShowLog(1,"%s:��pool[%d]��δ���ָ�TCBno:%d",__FUNCTION__,n,TCBno);
	clip->Errno=0;
	*clip->ErrMsg=0;
	*Connect=NULL;
}
//���ӳؼ�� 
void scpool_check()
{
int n,i,num;
pool *pl;
resource *rs;
INT64 now;
char buf[40];
T_Connect *conn=NULL;

        if(!scpool) return;
        now=now_usec();
        pl=scpool;

        for(n=0;n<SCPOOLNUM;n++,pl++) {
                if(!pl->lnk) continue;
                rs=pl->lnk;
                num=pl->resource_num;
//              if(log_level) ShowLog(log_level,"%s:scpool[%d],num=%d",__FUNCTION__,n,num);
                pthread_mutex_lock(&pl->mut);
                for(i=0;i<num;i++,rs++) if(rs->TCBno<0) {
                        if(rs->Conn.Socket>-1 && (now-rs->timestamp)>299000000) {
//����ʱ��̫����     
ShowLog(1,"%s:scpool[%d].%d,Socket=%d,to free",__FUNCTION__,n,i,rs->Conn.Socket);
//                              rs->Conn.Var=NULL; //�����ڴ�й©��
                                disconnect(&rs->Conn);
                                rs->cli.Errno=-1;
                                if(log_level)
                                        ShowLog(log_level,"%s:Close SCpool[%d].%d,since %s",__FUNCTION__,
                                        n,i,rusecstrfmt(buf,rs->timestamp,YEAR_TO_USEC));
                        }
                } else {
                        if(rs->Conn.Socket>-1 && (now-rs->timestamp)>299000000) {
//ռ��ʱ��̫����     
                              if(-1==get_TCB_status(rs->TCBno)) {
                                //TCB�Ѿ��������ͷ�֮
                                        unbind_sc(rs->TCBno);
                                        ShowLog(1,"%s:scpool[%d].%d TCB:%d to release",
                                                __FUNCTION__,n,i,rs->TCBno);
                                        rs->cli.Errno=-1;
                                        conn=&rs->Conn;
                			pthread_mutex_unlock(&pl->mut);
                                        release_SC_connect(&conn,rs->TCBno,n);
                			pthread_mutex_lock(&pl->mut);
                                } else {
                                    if(log_level) ShowLog(log_level,"%s:scpool[%d].lnk[%d] used by TCB:%d,since %s",
                                        __FUNCTION__,n,i,rs->TCBno,
                                        rusecstrfmt(buf,rs->timestamp,YEAR_TO_USEC));
                                }
                        }
                }
                pthread_mutex_unlock(&pl->mut);
        }
}

/**
 * ����d_nodeȡ���ӳغ�  
 * ʧ�ܷ���-1
 */
int get_scpool_no(int d_node)
{
int n;
	if(!scpool) return -1;
	for(n=0;n<SCPOOLNUM;n++) {
		if(scpool[n].d_node==d_node) return n;
	}
	return -1;
}

int get_scpoolnum()
{
	return SCPOOLNUM;
}

char *get_LABEL(int poolno)
{
	if(poolno<0 || poolno>=SCPOOLNUM) return NULL;
	return scpool[poolno].log.LABEL;
}

