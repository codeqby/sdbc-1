/***********************************************
 * �̳߳ط����� 
 ***********************************************/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/resource.h>

#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/unistd.h>
#include <ctype.h>
#include <sdbc.h>

#ifdef __cplusplus
extern "C"
#endif
extern void set_showid(void *ctx);

extern srvfunc Function[];// appl function list
extern u_int family[];
static void *thread_work(void *param);

static int g_epoll_fd=-1;

// SDBC task control block for epoll event 
typedef struct event_node {
	struct event_node *next;
	int events;
	int fd;
	T_Connect conn;
	T_NetHead head;
	T_SRV_Var sv;
	char	   *ctx;
	sdbcfunc call_back;
	INT64 timestamp;
	int timeout;
	volatile int status; //-1 δ���ӣ�0:δ��¼��1���ѵ�¼
} TCB;

static int do_epoll(TCB *task,int op);

typedef  struct {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	TCB *queue;
	int svc_num;
	char flg;	//���ػ��߳�0����1
} Qpool;

//��������
static Qpool rpool={PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,NULL,-1,0};

//�̳߳ؽڵ�
typedef struct {
	pthread_t tid;
	int status;
	INT64 timestamp;
} resource;
//�̳߳�
static struct {
	pthread_mutex_t mut;
	int num;
	int rdy_num;
	resource *pool;
	pthread_attr_t attr;
} tpool={PTHREAD_MUTEX_INITIALIZER,0,1,NULL};
//�����
static  struct {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	int max_client;
	TCB *pool;
	TCB *free_q;
} client_q={PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,NULL,NULL};

T_SRV_Var * get_SRV_Var(int TCBno)
{
	if(TCBno<0 || TCBno>=client_q.max_client) return NULL;
	return &client_q.pool[TCBno].sv;
}

void TCB_add(TCB **rp,int TCBno)
{
TCB *en;
	if(TCBno<0 || TCBno>=client_q.max_client) return;
	en=&client_q.pool[TCBno];
//	en->timestamp=now_usec();
	if(en->next) {
		ShowLog(1,"%s:TCB:%d �Ѿ��ڶ�����",__FUNCTION__,TCBno);
		return;//����������������
	}
	if(!rp) {
		pthread_mutex_lock(&rpool.mut);
		rp=&rpool.queue;
	}
	if(!*rp) {
		*rp=en;
		en->next=en;
	} else {
		en->next=(*rp)->next;//���Ӷ�ͷ
		(*rp)->next=en;//�ҵ���β
		*rp=en;//ָ����β
	}
	if(*rp==rpool.queue) {
		pthread_mutex_unlock(&rpool.mut);
//ShowLog(1,"%s:TCB:%d,tid=%lX",__FUNCTION__,TCBno,pthread_self());
		pthread_cond_signal(&rpool.cond); //���ѹ����߳�	
	}
}

int TCB_get(TCB **rp)
{
TCB *enp;
	if(!rp || !*rp) return -1;
	enp=(*rp)->next;//�ҵ���ͷ
	if(!enp->next) {
		ShowLog(1,"%s:TCB=%d,��δ�ڶ����У�",__FUNCTION__,enp->sv.TCB_no);
		return -2;
	}
	if(enp->next == enp) *rp=NULL;//���һ����
	else (*rp)->next=enp->next;//�µĶ�ͷ
	enp->next=NULL;
	return enp->sv.TCB_no;
}

sdbcfunc  set_callback(int TCBno,sdbcfunc callback,int timeout)
{
sdbcfunc old;
TCB *task;

	if(TCBno<0 ||TCBno>client_q.max_client) return (sdbcfunc)-1;
	task=&client_q.pool[TCBno];
	task->timeout=timeout;
	old=task->call_back;
	task->call_back=callback;
	return old;
}
/**
 * unset_callback
 * ����û��Զ���ص������
 * @param TCB_no �ͻ������
 * @return ԭ�ص�����
 */
sdbcfunc unset_callback(int TCB_no)
{
sdbcfunc old;
TCB *task;
        if(TCB_no<0 || client_q.max_client <= TCB_no) return NULL;
	task=&client_q.pool[TCB_no];
	old=task->call_back;
        task->call_back=NULL;
        task->timeout=task->conn.timeout;
	
        return old;
}

/**
 * get_callback
 * ȡ�û��Զ���ص������
 * @param TCB_no �ͻ������
 * @return �ص�����
 */
sdbcfunc get_callback(int TCB_no)
{
        if(TCB_no<0 || client_q.max_client <= TCB_no) return NULL;
	return client_q.pool[TCB_no].call_back;
}

T_Connect *get_TCB_connect(int TCBno)
{
	if(TCBno<0 ||TCBno>client_q.max_client) return NULL;
	return &client_q.pool[TCBno].conn;
}

void *get_TCB_ctx(int TCBno)
{
	if(TCBno<0 ||TCBno>client_q.max_client) return NULL;
	return client_q.pool[TCBno].ctx;
}
void tpool_free()
{
int i;
	if(client_q.pool) {
		if(client_q.pool[0].ctx) 
			free(client_q.pool[0].ctx);
		for(i=0;i<client_q.max_client;i++) {
			client_q.pool[i].conn.Var=NULL;
			freeconnect(&client_q.pool[i].conn);
		}
		free(client_q.pool);
		client_q.pool=NULL;
		client_q.free_q=NULL;
	}
	client_q.max_client=0;

	rpool.svc_num=-1;
	rpool.flg=0;

	pthread_mutex_destroy(&tpool.mut);
	if(tpool.pool) {
		free(tpool.pool);
		tpool.pool=NULL;
	}
	tpool.num=0;
	pthread_attr_destroy(&tpool.attr);
	if(g_epoll_fd > -1) close(g_epoll_fd);
	g_epoll_fd=-1;
	return ;
}

//�������߳�
static int new_wt(int n)
{
pthread_t tid;
int ret;

	if(n<0) return n;
	tpool.pool[n].status=1;
	tpool.pool[n].timestamp=now_usec();
	tpool.pool[n].tid=-1;
	ret=pthread_create(&tid,&tpool.attr,thread_work,&tpool.pool[n]);
        if(ret) {
		tpool.pool[n].tid=0;
                ShowLog(1,"%s:pthread_create:%s",__FUNCTION__,strerror(ret));
		return ret;
        }
	return 0;
}
extern srvfunc *SRVFUNC;
static int tpool_init(int size_ctx)
{
char *p;
int ret,i,limit;
int mtu,timeout;
struct rlimit sLimit;
TCB *task;

	p=getenv("TIMEOUT");
        if(p && isdigit(*p)) {
                timeout=60*atoi(p);
        } else timeout=0;
	p=getenv("SENDSIZE");
        if(p && isdigit(*p)) {
                mtu=atoi(p);
        } else mtu=0;

	rpool.svc_num=-1;
	SRVFUNC=Function;//used by get_srvname();

	limit=getrlimit(RLIMIT_NOFILE,&sLimit);
	if(limit==0) {
		limit=sLimit.rlim_cur;
	}

	p=getenv("MAXCLT");
	if(!p || !isdigit(*p)) {
		ShowLog(4,"%s:ȱ�ٻ�������MAXCLT,����Ϊ2",__FUNCTION__);
		client_q.max_client=2;
	} else {
		client_q.max_client=atoi(p);
		if(limit>0) {
			i=(limit<<3)/10;
			if(client_q.max_client > i) client_q.max_client=i;
		}
	}
	ShowLog(0,"%s:MAXCLIENT=%d",__FUNCTION__,client_q.max_client);
	if(NULL==(client_q.pool=(TCB *)malloc((client_q.max_client+1) * sizeof(TCB)))) return -4;
	if(size_ctx>0)
		if(NULL==(client_q.pool[0].ctx=malloc((client_q.max_client+1) * size_ctx))) {
			free(client_q.pool);
			client_q.pool=NULL;
			return -2;
		} else ;
	else client_q.pool[0].ctx=NULL;
	client_q.free_q=NULL;
	
	task=client_q.pool;
	for(i=0;i<=client_q.max_client;i++,task++) {
		initconnect(&task->conn);
		task->next=NULL;
		task->call_back=NULL;
		task->sv.TCB_no=i;
		task->timeout=0;
		task->conn.timeout=timeout;
		task->conn.MTU=mtu;
		task->conn.family=family;
		task->events=0;
		task->sv.poolno=-1;
		task->sv.SQL_Connect=NULL;
		task->status=-1;
		if(!client_q.pool[0].ctx) task->ctx=NULL;
		else if(i>0) task->ctx=client_q.pool[0].ctx+i*size_ctx;
		task->sv.var=task->ctx;
		TCB_add(&client_q.free_q,i);
	}

	p=getenv("RDY_NUM");
	if(p && isdigit(*p)) tpool.rdy_num=atoi(p);
	else tpool.rdy_num=1;
	p=getenv("MAXTHREAD");
	if(!p || !isdigit(*p)) {
		tpool.num=tpool.rdy_num+1;
		ShowLog(4,"%s:ȱ�ٻ�������MAXTHREAD,����Ϊ%d",__FUNCTION__,tpool.num);
	} else tpool.num=atoi(p);
	if(NULL==(tpool.pool=(resource *)malloc(tpool.num * sizeof(resource)))) {
		if(client_q.pool) {
			free(client_q.pool);
			client_q.pool=NULL;
		}
		return -3;
	}
	ret= pthread_attr_init(&tpool.attr);
        if(ret) {
                ShowLog(1,"%s:can not init pthread attr %s",__FUNCTION__,strerror(ret));
        } else {
//���÷����߳�
        	ret=pthread_attr_setdetachstate(&tpool.attr,PTHREAD_CREATE_DETACHED);
        	if(ret) {
               	 ShowLog(1,"%s:can't set pthread attr PTHREAD_CREATE_DETACHED:%s",
		 	__FUNCTION__,strerror(ret));
       		}
//�����̶߳�ջ������ 256K
		pthread_attr_setguardsize(&tpool.attr,(size_t)(1024 * 256));
	}
	for(i=0;i<tpool.num;i++) {
		tpool.pool[i].tid=0;
		tpool.pool[i].status=0;
		tpool.pool[i].timestamp=0;
	}	
	rpool.queue=NULL;
//ShowLog(5,"%s:maxfd=%d,maxclt=%d",__FUNCTION__,limit,client_q.max_client);
	if( 0 <= (g_epoll_fd=epoll_create(limit>0?limit<(client_q.max_client<<1)?limit:client_q.max_client<<1:client_q.max_client))) {
		for(i=0;i<tpool.num;i++) new_wt(i);
		return 0;
	}
	ShowLog(1,"%s:epoll_create err=%d,%s",
		__FUNCTION__,errno,strerror(errno));
	tpool_free();
	return SYSERR;
}
/*
static void rdy_add(TCB *en)
{
	if(en->next) { //TCB �Ѿ��ڶ�����
		ShowLog(1,"%s:TCB:%d �Ѿ��ڶ�����",__FUNCTION__,en->sv.TCB_no);
		return;
	}
	if(!rpool.queue) {
		rpool.queue=en;
		en->next=en;
	} else {
		en->next=rpool.queue->next;
		rpool.queue->next=en;
		rpool.queue=en;
	}
}
*/
static TCB *rdy_get()
{
TCB *enp;
	if(!rpool.queue) return NULL;
	enp=rpool.queue->next;
	if(enp==NULL) {
		ShowLog(1,"%s:bad ready queue TCB:%d!",__FUNCTION__,rpool.queue->sv.TCB_no);
		enp=rpool.queue;
		rpool.queue=NULL;
		return enp;
	}
	if(enp->next == enp) rpool.queue=NULL;
	else rpool.queue->next=enp->next;
	enp->next=NULL;
	return enp;
}
/**
 * set_event
 * �û��Զ����¼�
 * @param TCB_no �ͻ������
 * @param fd �¼�fd ֻ֧�ֶ��¼� 
 * @param call_back �����¼��Ļص�����
 * @param timeout fd�ĳ�ʱ��,ֻ��������socket fd
 * @return �ɹ� 0
 */
int set_event(int TCB_no,int fd,sdbcfunc call_back,int timeout)
{
TCB *task;

	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	task=&client_q.pool[TCB_no];
	task->fd=fd;
	task->call_back=call_back;
	task->timestamp=now_usec();
	task->timeout=timeout;
	return do_epoll(task,(task->fd==task->conn.Socket)?EPOLL_CTL_MOD:EPOLL_CTL_ADD);
}

/**
 * clr_event
 * ����û��Զ����¼�
 * @param TCB_no �ͻ������
 * @return �ɹ� 0
 */
int clr_event(int TCB_no)
{
TCB *task;
int ret;
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	task=&client_q.pool[TCB_no];
	if(task->fd == task->conn.Socket) {
		task->call_back=NULL;
		task->status=1;
		return -2;
	}
	task->call_back=NULL;
	ret=do_epoll(task,EPOLL_CTL_DEL);
	if(ret) ShowLog(1,"%s:tid=%lX,TCB:%d,do_epoll ret=%d",
		__FUNCTION__,pthread_self(),TCB_no,ret);
	task->fd=task->conn.Socket;
	task->timeout=task->conn.timeout;
	return 0;
}

/**
 * get_event_fd
 * ȡ�¼�fd
 * @param TCB_no �ͻ������
 * @return �¼�fd
 */
int get_event_fd(int TCB_no)
{
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	return client_q.pool[TCB_no].fd;
}
/**
 * get_event_status
 * ȡ�¼�״̬
 * @param TCB_no �ͻ������
 * @return �¼�״̬
 */
int get_event_status(int TCB_no)
{
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	return client_q.pool[TCB_no].events;
}

/**
 * get_event_status
 * ȡTCB״̬
 * @param TCB_no �ͻ������
 * @return TCB״̬
 */
int get_TCB_status(int TCB_no)
{
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	return client_q.pool[TCB_no].status;
}

static void client_del(TCB *task)
{
int status=task->status;
struct linger so_linger;

	task->fd=-1;
	so_linger.l_onoff=1;
        so_linger.l_linger=0;
        setsockopt(task->conn.Socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);

	pthread_mutex_lock(&tpool.mut);
	freeconnect(&task->conn);
	task->events=0;
	task->status=-1;
	task->timeout=0;
	pthread_mutex_unlock(&tpool.mut);
	pthread_mutex_lock(&client_q.mut);
	TCB_add(&client_q.free_q,task->sv.TCB_no);
	pthread_mutex_unlock(&client_q.mut);
	pthread_cond_signal(&client_q.cond); //�������߳�	
	if(status>-1) ShowLog(3,"%s:tid=%lX,TCB:%d deleted!",__FUNCTION__,pthread_self(),task->sv.TCB_no);
}

//����������
static int do_epoll(TCB *task,int op)
{
struct epoll_event epv = {0, {0}};
int  status,ret;
	if(task->fd<0) return FORMATERR;
	status=task->status;
	epv.events =  EPOLLIN|EPOLLONESHOT;
	epv.data.ptr = task;
	if(task->next) {
		ShowLog(1,"%s:tid=%lX,TCB:%d �Ѿ��ڶ�����,fd=%d,Sock=%d",__FUNCTION__,
			pthread_self(),task->sv.TCB_no,task->fd,task->conn.Socket);
		return -1;
	}
	status=task->status;
	epv.events =  EPOLLIN|EPOLLONESHOT;
	epv.data.ptr = task;
	task->events=0;
	ret=epoll_ctl(g_epoll_fd,op,task->fd,&epv);
	if(ret<0 || op==EPOLL_CTL_DEL) {
		if(ret<0) {
			if( errno != EEXIST) ShowLog(1,"%s:tid=%lX,epoll_ctl fd[%d]=%d,op=%d,ret=%d,err=%d,%s",__FUNCTION__,
				pthread_self(),task->sv.TCB_no, task->fd,op,ret,errno,strerror(errno));
		} else {
			task->fd=-1;
			if(task->status>-1) 
			   ShowLog(3,"%s:tid=%lX epoll_ctl fd[%d]=%d,deleted,op=%d",__FUNCTION__,
				pthread_self(),task->sv.TCB_no, task->fd,op);
		}
	}
	return ret;
}
//�����߳�
static int do_work(TCB *task)
{
int ret;
T_Connect *conn;
void (*init)(T_Connect *,T_NetHead *);
T_SRV_Var *ctx=&task->sv;

	ctx->tid=pthread_self();//��־���̷߳���
	conn=&task->conn;
	conn->Var=ctx;
	ret=0;
	if(!task->call_back) { //SDBC��׼�¼� 
//Э����Կ
		if(task->status==-1) {
			init=(void (*)())conn->only_do;
			conn->only_do=0;
			ret=mk_clikey(conn->Socket,&conn->t,conn->family);
			if(ret<0) {
				if(ret!=SYSERR) 
					ShowLog(1,"%s:tid=%lX,TCB:%d,Э����Կʧ��!,ret=%d",__FUNCTION__,
							ctx->tid,task->sv.TCB_no,ret);
	//�ͷ�����
				return -1;
			} 
			conn->CryptFlg=ret;
			task->status=0;
			if(init) init(conn,&task->head);
			task->timeout=conn->timeout;
			return 0;
		}
	
		if(task->status>0) set_showid(task->ctx);
		ret=RecvPack(conn,&task->head);
		task->timestamp=now_usec();
		if(ret) {
			ShowLog(1,"%s:TCB:%d,���մ���,tid=%lX,err=%d,%s,event=%08X",__FUNCTION__,
				task->sv.TCB_no,ctx->tid,errno,strerror(errno),task->events);
			return -1;
		}
		ShowLog(4,"%s: tid=%lX,TCB:%d,PROTO_NUM=%d pkg_len=%d,t_len=%d,USEC=%llu",
			__FUNCTION__,ctx->tid,task->sv.TCB_no,
	                task->head.PROTO_NUM,task->head.PKG_LEN,task->head.T_LEN,
			task->timestamp);
	
		if(task->head.PROTO_NUM==65535) {
			ShowLog(3,"%s: disconnect by client",__FUNCTION__);
			return -1;
		} else if(task->head.PROTO_NUM==1){
	                ret=Echo(conn,&task->head);
	        } else if(task->status==0) {
			if(!task->head.PROTO_NUM) {
				ret=Function[0].funcaddr(conn,&task->head);
				if(ret>=0) {
					task->status=ret;
				}
	                } else {
	                        ShowLog(1,"%s:TCB:%d,δ��¼",__FUNCTION__,task->sv.TCB_no);
	                        return -1;
	                }
		} else if (conn->only_do) {
			ret=conn->only_do(conn,&task->head);
		} else {
			if(task->head.PROTO_NUM==0) {
				ret=get_srvname(conn,&task->head);
			} else if(task->head.PROTO_NUM>rpool.svc_num) {
	                         ShowLog(1,"%s:û���������� %d",__FUNCTION__,task->head.PROTO_NUM);
	                         ret=-1;
	                } else {
	                        ret=Function[task->head.PROTO_NUM].funcaddr(conn,&task->head);
	                        if(ret==-1)
					ShowLog(1,"%s:TCB:%d,disconnect by server",__FUNCTION__,
						task->sv.TCB_no);
			}
		}
	} else { //�û��Զ����¼�
		task->timestamp=now_usec();
		set_showid(task->ctx);//Showid Ӧ���ڻỰ�����Ľṹ�� 
		ShowLog(5,"%s:call_back tid=%lX,USEC=%llu",__FUNCTION__,ctx->tid,task->timestamp);
		ret=task->call_back(conn,&task->head);
		if(task->status==0)  //finish_login,return 0 or 1
			task->status=ret;
	        if(ret==-1)
			ShowLog(1,"%s:TCB:%d,disconnect by server",__FUNCTION__,
				task->sv.TCB_no);
	}
	task->timeout=conn->timeout;
	return ret;
}

static void *thread_work(void *param)
{
resource *rs=(resource *)param;
int ret,fds;
TCB *task;
struct epoll_event event;

	rs->tid=pthread_self();
	while(1) {
	int timeout=0;
//�Ӿ�������ȡһ������
		pthread_mutex_lock(&rpool.mut);
		while(!(task=rdy_get())) {
			if(rpool.flg >= tpool.rdy_num) break;
			rpool.flg++;
			ret=pthread_cond_wait(&rpool.cond,&rpool.mut); //û�����񣬵ȴ�
 			rpool.flg--;
		}
		pthread_mutex_unlock(&rpool.mut);
		if(!task) {
			fds = epoll_wait(g_epoll_fd, &event, 1 , -1);
			if(fds < 0){
       	 			ShowLog(1,"%s:epoll_wait err=%d,%s",__FUNCTION__,errno,strerror(errno));
				sleep(30);
				continue;
       	 		}
		 	task = (TCB *)event.data.ptr;
			timeout=task->timeout;
			task->timeout=0;
			if(task->events == 0X10||task->fd==-1) {
				ShowLog(1,"%s:task already timeout",__FUNCTION__);
				continue;//�Ѿ����볬ʱ״̬
			}
			task->events=event.events;
		}
		rs->timestamp=now_usec();
		ret=do_work(task); //������ķ���
		task->timestamp=now_usec();//task��rs�Ĳ����Ӧ������ִ��ʱ��
		switch(ret) {
		case -1:
			do_epoll(task,EPOLL_CTL_DEL);
			client_del(task);
		case -5:
			break;
		default:
			if(!task->timeout) task->timeout=timeout; //timeout û�б�Ӧ�����ù�
			if(do_epoll(task,EPOLL_CTL_MOD) && errno != EEXIST) {
				ShowLog(1,"%s:cancel by server",__FUNCTION__);
				client_del(task);
			}
			break;
		}
		mthr_showid_del(rs->tid);
	}
	ShowLog(1,"%s:tid=%lX canceled",__FUNCTION__,pthread_self());
	mthr_showid_del(rs->tid);
	rs->timestamp=now_usec();
	rs->status=0;
	rs->tid=0;
	return NULL;
}

//��鳬ʱ������
int check_TCB_timeout()
{
int i,cltnum=client_q.max_client;
TCB * tp=client_q.pool;
INT64 now=now_usec();
int num=0,t;

        for(i=0;i<cltnum;i++,tp++) {
		if(tp->timeout<=0 || tp->fd<0) continue;
               	t=(int)((now-tp->timestamp)/1000000);
		if(t<tp->timeout) continue;
		tp->events=0X10;
                if(tp->call_back) TCB_add(NULL,tp->sv.TCB_no);
                else {
			do_epoll(tp,EPOLL_CTL_DEL);
			client_del(tp);
			if(tp->status<1) ShowLog(1,"%s:TCB:%d to cancel,t=%d",__FUNCTION__,i,t);
			else ShowLog(1,"%s:TCB:%d to delete,t=%d",__FUNCTION__,i,t);
			num++;
		}
        }
        return num;
}

static int get_task_no()
{
int i,ret;
struct timespec abstime;
	abstime.tv_sec=0;
	pthread_mutex_lock(&client_q.mut);
	while(0>(i=TCB_get(&client_q.free_q))) {
		if(abstime.tv_sec==0) ShowLog(1,"%s:���������������",__FUNCTION__);
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec+=5;
		ret=pthread_cond_timedwait(&client_q.cond,&client_q.mut,&abstime);
		if(ret==ETIMEDOUT) {
			pthread_mutex_unlock(&client_q.mut);
			check_TCB_timeout();
			pthread_mutex_lock(&client_q.mut);
		}
		continue;
	}
	pthread_mutex_unlock(&client_q.mut);
	return i;
}

/***************************************************************
 * �̳߳�ģ�͵���ں��������߳���Ҫ������Դ����
 *Ӧ�ò���ķ�������
 * extern srvfunc Function[];
 */
int TPOOL_srv(void (*conn_init)(T_Connect *,T_NetHead *),void (*quit)(int),void (*poolchk)(void),int sizeof_gda)
{
int ret,i;
int s;
struct sockaddr_in sin,cin;
struct servent *sp;
char *p;
struct timeval tm;
fd_set efds;
socklen_t leng=1;
int sock=-1;
srvfunc *fp;
TCB *task;
struct linger so_linger;

	tzset();

	signal(SIGPIPE,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT ,SIG_IGN);
	signal(SIGPWR ,quit);
	signal(SIGTERM,quit);

	p=getenv("SERVICE");
	if(!p || !*p) {
		ShowLog(1,"ȱ�ٻ������� SERVICE ,��֪�غ��ĸ��˿ڣ�");
		quit(3);
	}
//���Զ˿��Ƿ�ռ�� 
	sock=tcpopen("localhost",p);
	if(sock>-1) {
		ShowLog(1,"�˿� %s �Ѿ���ռ��",p);
		close(sock);
		sock=-1;
		quit(255);
	}

	ret=tpool_init(sizeof_gda);
	if(ret) return(ret);

	for(fp=Function;fp->funcaddr!=0;fp++) rpool.svc_num++;
	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if(isdigit(*p)){
		sin.sin_port=htons((u_short)atoi(p));
	} else {
		if((sp=getservbyname(p,"tcp"))==NULL){
        		ShowLog(1,"getsrvbyname %s error",p);
        		quit(3);
		}
		sin.sin_port=(u_short)sp->s_port;
	}

	sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0) {
		ShowLog(1,"open socket error=%d,%s",errno,
			strerror(errno));
		quit(3);
	}

	bind(sock,(struct sockaddr *)&sin,sizeof(sin));
	leng=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&leng,sizeof(leng));
//���� TIME_WAIT
        so_linger.l_onoff=1;
        so_linger.l_linger=0;
        ret=setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);
        if(ret) ShowLog(1,"set SO_LINGER err=%d,%s",errno,strerror(errno));

	listen(sock,client_q.max_client);

	ShowLog(0,"main start tid=%lX sock=%d",pthread_self(),sock);
	
	int repeat=0;
	leng=sizeof(cin);

	while(1) {
		do {
			FD_ZERO(&efds);
			FD_SET(sock, &efds);
//�����������
			tm.tv_sec=15;
			tm.tv_usec=0;
			ret=select(sock+1,&efds,NULL,&efds,&tm);
			if(ret==-1) {
				ShowLog(1,"select error %s",strerror(errno));
				close(sock);
				quit(3);
			}
			if(ret==0) {
				check_TCB_timeout();
				if(poolchk) poolchk();
			}
		} while(ret<=0);
		i=get_task_no();
		task=&client_q.pool[i];
		s=accept(sock,(struct sockaddr *)&cin,&leng);
		if(s<0) {
			ShowLog(1,"%s:accept err=%d,%s",__FUNCTION__,errno,strerror(errno));
                       	client_del(task);
			switch(errno) {
			case EMFILE:	//fd������,�����̻߳�Ҫ�������������߳���Ϣһ�¡�  
			case ENFILE:
				sleep(30);
				continue;
			default:break;
			}
			sleep(15);
			if(++repeat < 20) continue;
			ShowLog(1,"%s:network fail! err=%s",__FUNCTION__,strerror(errno));
			close(sock);
			quit(5);
		}
		repeat=0;
		task->fd=task->conn.Socket=s;
		task->timestamp=now_usec();
		task->timeout=60;
		task->status=-1;
		task->conn.only_do=(int (*)())conn_init;
		ret=do_epoll(task,EPOLL_CTL_ADD);//���񽻸������߳���
	}
	
	close(sock);
	tpool_free();
	return (0);
}
