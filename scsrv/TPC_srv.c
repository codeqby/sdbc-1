/***********************************************************
 * SDBC5.X,一个基于多线程的连接池服务器框架  
 * TPC:Thread Per Connection
 ***********************************************************/
#include <signal.h>
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sdbc.h>
#include <bignum.h>

extern srvfunc Function[]; //user program.

static void * thread_work(void *param)
{
T_Connect Conn=*(T_Connect *)param;
T_NetHead Head;
int ret,logined=0;
T_SRV_Var ctx;
srvfunc *fp;
int svcnum=0;
int (*init)(T_Connect *conn,T_NetHead *head);
char addr[16];

char gda[Conn.SendLen+1];//本线程的全局数据区必须在此分配。 
	
	((T_Connect *)param)->Socket=-1;//通知主线程 
	if(Conn.SendLen>0) ctx.var=gda;
	else ctx.var=0;
	Conn.SendLen=0;
	ctx.tid=pthread_self();//标志多线程服务  
	ctx.poolno=0;
	ctx.SQL_Connect=NULL;
	ctx.TCB_no=-1;	//标志不是线程池
	Conn.Var=&ctx;
	init=Conn.only_do;
	Conn.only_do=0;
//借用only_do存放函数地址 conn_init  
	if(!Conn.only_do) for(fp=Function;fp->funcaddr!=0;fp++) svcnum++;

//	ShowLog(2,"%s:tid=%lX,sock=%d",__FUNCTION__,ctx.tid,Conn.Socket);
// 协商密钥 
	Conn.CryptFlg=mk_clikey(Conn.Socket,&Conn.t,Conn.family);
	if(Conn.CryptFlg<0) { //协商密钥失败
		LocalAddr(Conn.Socket,addr);
		ShowLog(1,"%s:tid=%lX addr=%s,协商密钥失败!",__FUNCTION__,
			ctx.tid,addr);
		freeconnect(&Conn);
		return NULL;
	}
	if(init) init(&Conn,&Head);

	while(1) {
		ret=RecvPack(&Conn,&Head);
		if(ret<0) {
			ShowLog(1,"%s:tid=%lX,接收结束,sock=%d,status=%d,%s",
				__FUNCTION__,ctx.tid,Conn.Socket,errno,strerror(errno));
			break;
		}
		ShowLog(4,"%s: tid=%lX,PROTO_NUM:%d PKG_LEN=%d,T_LEN=%d",__FUNCTION__,ctx.tid,
                        Head.PROTO_NUM,Head.PKG_LEN,Head.T_LEN);
		if(Head.PROTO_NUM==1){
                	Echo(&Conn,&Head);
               		continue;
        	}
		if(Head.PROTO_NUM==0xFFFF){
			ShowLog(0,"%s:Disconnect by client,tid=%lX",__FUNCTION__,ctx.tid);
			break;
		}
		if(!Head.PROTO_NUM) {
			if(!logined) {
				logined=Function[0].funcaddr(&Conn,&Head);
                		if(logined==-1) break;
			} else {
				get_srvname(&Conn,&Head);
			}
		} else if(Conn.only_do) {
			ret=Conn.only_do(&Conn,&Head);
			continue;
		} else {
			if(!logined) {//未登录
				ShowLog(1,"%s:未登录,tid=%lX",__FUNCTION__,ctx.tid);
				break;
			}
			if(Head.PROTO_NUM>svcnum) {
				ShowLog(1,"%s:没有这个服务号 %s",__FUNCTION__,Head.PROTO_NUM);
				break;
			}
			ret=Function[Head.PROTO_NUM].funcaddr(&Conn,&Head);
                	if(ret==-1) {
                        	ShowLog(0,"%s:Disconnect by server PROTO_NUM=%d,ret=%d",
                                	__FUNCTION__,Head.PROTO_NUM,ret);
				break;
			}
		}
	}
	freeconnect(&Conn);
	mthr_showid_del(ctx.tid);
	return NULL;
}

extern srvfunc *SRVFUNC;//used by get_srvname();
extern u_int family[];

void TPC_srv(void (*conn_init)(T_Connect *,T_NetHead *),void (*quit)(int),void (*poolchk)(void),int sizeof_gda)
{
int ret;
struct sockaddr_in sin,cin;
struct servent *sp;
char *p;
int s;
pthread_t pthread_id;
pthread_attr_t attr;
struct timeval tm;
fd_set efds;
socklen_t leng=1;
int sock=-1;
T_Connect Conn;
struct linger so_linger;

	tzset();

	ret= pthread_attr_init(&attr);
	if(ret) {
		ShowLog(1,"can not init pthread attr %s",strerror(ret));
		return ;
	}
//设置分离线程  
	ret=pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	if(ret) {
		ShowLog(1,"can't set pthread attr:%s",strerror(ret));
		return ;
	}
//设置线程堆栈保护区 256K  
	ret=pthread_attr_setguardsize(&attr,(size_t)(1024 * 256));

	SRVFUNC=Function;
	
	signal(SIGPIPE,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT ,SIG_IGN);
	signal(SIGPWR ,quit);
	signal(SIGTERM,quit);

	initconnect(&Conn);
	Conn.family=family;
	Conn.Var=0;

	p=getenv("SERVICE");
	if(!p || !*p) {
		ShowLog(1,"缺少环境变量 SERVICE ,不知守候哪个端口！");
		quit(3);
	}
//测试端口是否被占用 
	sock=tcpopen("localhost",p);
	if(sock>-1) {
		ShowLog(1,"端口 %s 已经被占用",p);
		close(sock);
		sock=-1;
		quit(255);
	}

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
//避免 TIME_WAIT
	so_linger.l_onoff=1;
	so_linger.l_linger=0;
	ret=setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);
	if(ret) ShowLog(1,"set SO_LINGER err=%d,%s",errno,strerror(errno));


	leng=sizeof(cin);
	int repeat=0;
		ShowLog(0,"work start!main sock=%d",sock);

	p=getenv("TIMEOUT");
        if(p && isdigit(*p)) {
                Conn.timeout=60*atoi(p);
        } else Conn.timeout=0;

	p=getenv("SENDSIZE");
        if(p && isdigit(*p)) {
                Conn.MTU=atoi(p);
        } else Conn.MTU=0;
//避免 TIME_WAIT
          so_linger.l_onoff=1;
          so_linger.l_linger=0;
          ret=setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);
          if(ret) ShowLog(1,"set SO_LINGER err=%d,%s",errno,strerror(errno));


	listen(sock,1000);// 以后用配置 
	while(1) {
		do {
			FD_ZERO(&efds);
			FD_SET(sock, &efds);
//健康检查周期5分钟 
			tm.tv_sec=300;
			tm.tv_usec=0;
			ret=select(sock+1,&efds,NULL,&efds,&tm);
//ShowLog(4,"%s:aft select ret=%d,sock=%d",__FUNCTION__,ret,sock);
			if(ret==-1) {
				ShowLog(1,"select error %s",strerror(errno));
				close(sock);
				quit(3);
			}
			if(ret==0 && poolchk) poolchk();
		} while(ret<=0);
		s=accept(sock,(struct sockaddr *)&cin,&leng);
		if(s<0) {
			ShowLog(1,"%s:accept err=%d,%s",__FUNCTION__,errno,strerror(errno));
			switch(errno) {
			case EMFILE:	//fd用完了,其他线程还要继续工作，主线程休息一下。  
			case ENFILE:
				sleep(30);
				continue;
			default:break;
			}
			sleep(3);
			if(++repeat < 20) continue;
			ShowLog(1,"%s:network fail! err=%s",__FUNCTION__,strerror(errno));
			close(sock);
			quit(5);
		}
		Conn.Socket=s;
		Conn.only_do=(int (*)())conn_init; //借用一下 
		Conn.SendLen=sizeof_gda;
		ret=pthread_create(&pthread_id,&attr,thread_work,&Conn);
		if(ret) {
			ShowLog(1,"%s:pthread_create:%s",__FUNCTION__,strerror(ret));
			close(s);
			if(ret==EAGAIN||ret==ENOMEM) {	//线程数用完了，休息一会，等一些线程退出 
				sleep(30);
			}
			continue;
		}
		while(Conn.Socket != -1) usleep(1000);
	}
	
	ret=pthread_attr_destroy(&attr);
	close(sock);
	quit(0);
}

