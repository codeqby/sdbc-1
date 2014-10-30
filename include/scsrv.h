/*******************************************************
 * Secure Database Connect
 * SDBC 6.0 for ORACLE 
 * 2012.9.19 by ylh
 *******************************************************/

#ifndef SCSRVDEF
#define SCSRVDEF
#include <sc.h>

typedef struct {
   sdbcfunc funcaddr;
   const char *srvname;
} srvfunc;

typedef struct {
	struct S_SQL_Connect *SQL_Connect;
	pthread_t tid;
	void *var;
	int poolno; //ʹ�õ����ݿ����ӳغ�  
	int TCB_no; //�̳߳ط������еĻỰ��
	int o_timeout;//��״̬�����б���ԭ����TIMEOUTֵ
} T_SRV_Var;

#ifdef __cplusplus
extern "C" {
#endif
/* �����������¼�Ҫ֪ͨ�ͻ��ˣ�SendPackǰ,PROTO_NUM=PutEvent(conn,PROTO_NUM);
   ��ǰ��conn->Event_procҪ�����¼�������򣬷���ֵΪ�¼���,1-65535��
   ע�⣺�¼�������򲻿��Զ���������Դ�������Զ������ݿ���Դ */
int PutEvent(T_Connect *conn,int Evtno);

/* Server system interface functions */
extern int Rexec(T_Connect *connect,T_NetHead *NetHead);
extern int GetFile(T_Connect *connect,T_NetHead *NetHead);
extern int GetFile1(T_Connect *connect,T_NetHead *NetHead);
extern int PutFile(T_Connect *connect,T_NetHead *NetHead);
extern int Pwd(T_Connect *connect,T_NetHead *NetHead);
extern int ChDir(T_Connect *connect,T_NetHead *NetHead);
extern int filels(T_Connect *connect,T_NetHead *NetHead);
extern int PutEnv(T_Connect *connect,T_NetHead *NetHead);
extern int Echo(T_Connect *connect,T_NetHead *NetHead);
/*************************************************************
 * Process Per Connection  Server 
 * PPC_srv(): server entry poit 
 * cryptflg=0:not crypt,can be DO_CTYPT,or DO_CRYPT+CHECK_CRC 
 *************************************************************/
int PPC_srv( void (*NetInit)(T_Connect *,T_NetHead *),
	 srvfunc *func, void *userdata);
extern void setquit (void (*Quit)());
/***********************************************************
 * TPC_srv:Thread Per Connection  Server
 * һ�����ڶ��̵߳����ӳط�������� NetMain_r �������ڣ�
 * ../utility/thread/
 * TPOOL_srv:Thread Pool Server
 * һ������epoll���̳߳ط�������� TPOOL_srv�����ڸ���������OLTP���� �������ڣ�
 * ../utility/tpool/
 * conn_init:���߳̿�ʼ������Ԥ���� 
 * quit:�������˳�����
 * poolchk:���ڼ��ؽ����ĺ��� 
 * sizeof_gda:�û����������ݵĳ���,�����Ŀռ����߳��з���  
 * �����ṩȫ�ַ�������:
 * svcfunc Function[];
 ***********************************************************/

void TPC_srv(void (*conn_init)(T_Connect *,T_NetHead *),void (*quit)(int),void (*poolchk)(void),int sizeof_gda);
//tpool.c
int TPOOL_srv(void (*conn_init)(T_Connect *,T_NetHead *),void (*quit)(int),void (*poolchk)(void),int sizeof_gda);
void tpool_free();

struct event_node;
typedef struct event_node *pTCB;

void TCB_add(pTCB *queue,int TCBno);
int TCB_get(pTCB *queue);
sdbcfunc set_callback(int TCBno,sdbcfunc callback,int timeout);

T_Connect *get_TCB_connect(int TCBno);
void *get_TCB_ctx(int TCBno);
int get_TCB_status(int TCB_no);
/**
 * unset_callback
 * ����û��Զ���ص�����
 * @param TCB_no �ͻ������
 * @return ԭ�ص�����
 */
sdbcfunc  unset_callback(int TCB_no);

/**
 * get_callback
 * ȡ�û��Զ���ص������
 * @param TCB_no �ͻ������
 * @return �ص�����
 */

sdbcfunc get_callback(int TCB_no);
/**
 * get_event_status
 * ȡTCB״̬
 * @param TCB_no �ͻ������
 * @return TCB״̬
 */

/**
 * set_event
 * �û��Զ����¼�
 * @param TCB_no �ͻ������
 * @param fd �¼�fd ֻ֧�ֶ��¼� 
 * @param call_back �����¼��Ļص�����
 * @param timeout fd�ĳ�ʱ��,ֻ��������socket fd 
 * @return �ɹ� 0
 */
int set_event(int TCB_no,int fd,sdbcfunc call_back,int timeout);
/**
 * clr_event
 * ����û��Զ����¼� �ص�������������Ӧ����¼���
 * @param TCB_no �ͻ������
 * @return �ɹ� 0
 */
int clr_event(int TCB_no);
/**
 * get_event_fd
 * ȡ�¼�fd,�����ڻص�������ȡ���¼�fd 
 * @param TCB_no �ͻ������
 * @return �¼�fd
 */
int get_event_fd(int TCB_no);
/**
 * get_event_status
 * ȡ�¼�״̬
 * @param TCB_no �ͻ������
 * @return �¼�״̬
 */
int get_event_status(int TCB_no);

//clikey.c
/* Э����Կ */
int mk_clikey(int socket,ENIGMA2 *tc,u_int *family);

/* ȡ�������0��������ע��ɹ����滻0��Э�� * */
int get_srvname(T_Connect *conn,T_NetHead *NetHead);

#ifdef __cplusplus
}
#endif

#endif
