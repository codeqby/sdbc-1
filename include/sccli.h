/*******************************************************
 * Secure Database Connect
 * SDBC 6.0 for ORACLE 
 * 2012.09.15 by ylh
 *******************************************************/

#ifndef SCLIDEF
#define SCLIDEF
#include <pack.h>
#include <sc.h>

#ifndef TRAINBEGIN

#define TRANBEGIN 0
#define TRANCOMMIT 1
#define TRANROLLBACK 2

/* For ORACLE */
#define SQLNOTFOUND 1403
#define FETCHEND    100
#define DUPKEY   1
#define LOCKED 		 54

#endif
typedef struct {
	int usage;
	int srvn;		/* ������� */
	char *srvlist;          /* �������б� */
	char *srv_hash;		/* ���������� */
} svc_table;

typedef struct {
	char DSN[81];			/*database server name,no used*/
	char UID[81];			/*database user ID*/
	char PWD[81];			/*database user password, no used*/
	char DBOWN[81];			/* database owner */
	int Errno;
	int NativeError;
	char ErrMsg[2048];
	char SqlState[128];
	svc_table *svc_tbl;
	unsigned int ctx_id;            //32bit context id,copy to NetNead->O_NODE
	void *var;
} T_CLI_Var;

/* Client system functions */
#ifdef __cplusplus
extern "C" {
#endif

 /* used by client */
extern void Init_CLI_Var(T_CLI_Var *CLI_Var);
int Net_Connect( T_Connect *conn,void *userdata,u_int *new_family);

/* ����ͻ���Ҫ�����¼�,�յ�������Ӧ��󣬷��������¼��ŷ��� PROTO_NUM��,1-65535
 * ,Ӧ��¼PROTO_NUM,�ڱ��λỰȫ����ɺ� ����EventCatch(conn,PROTO_NUM),
 * ��ǰ��conn->Event_procҪ�����¼���������¼����������Զ���������Դ��
*/
int EventCatch(T_Connect *conn,int Evtno);
extern int N_Rexec(T_Connect *connect, char *cmd, int (*input)(char *), void (*output)(char *));
extern int N_Put_File(T_Connect *connect,char *local_file,char *remote_file);
extern int N_Put_File_Msg(T_Connect *connect,char *local_file,char *remote_file, void (*Msg)(int,int));
extern int N_Get_File(T_Connect *connect,char *local_file,char *remote_file);
extern int N_Get_File_Msg(T_Connect *connect, char *local_file, char *remote_file, void (*Msg)(int,int));
extern int N_PWD(T_Connect *connect,char *pwd_buf);
extern int N_ChDir(T_Connect *connect,char *dir);
extern int N_PutEnv(T_Connect *connect,char *env);

/* Client database functions */
extern int N_SQL_Prepare(T_Connect *,char *,T_SqlDa *);
/* Fetch() recnum:����ʱÿ��Fetch�ļ�¼��,0=ȫ����¼,����ʵ�ʵõ��ļ�¼��,NativeError:���� */
extern int N_SQL_Fetch(T_Connect *,int curno,char **result,int recnum);
/* Select() recnum:����ʱϣ��ȡ�õļ�¼��,0=ȫ����¼,����ʵ�ʵõ��ļ�¼��,NativeError:���� */
extern int N_SQL_Select(T_Connect *,char *cmd,char **data,int recnum);
extern int N_SQL_Close_RefCursor(T_Connect *connect,int ref_cursor);
extern int N_SQL_Close(T_Connect *,T_SqlDa *sqlda);
extern int N_SQL_Exec(T_Connect *,char * cmd);
/* st_lvs:״̬�����м����α����;�д�� */
extern int N_SQL_RPC(T_Connect *connect,char *stmt,char **data,int *ncols,int st_lvs);
extern int N_SQL_EndTran(T_Connect *connect,int TranFlag);
extern int N_insert_db(T_Connect *conn,char *tabname,void *data,T_PkgType *tp);
extern int getls(T_Connect *conn,T_NetHead *NetHead,char *path,FILE *outfile);

//���ҷ���ţ��޴˷��񷵻�1,=echo() 
// key=�����������ط���� 
int get_srv_no(T_CLI_Var *clip,char *key);
//�ͷŷ������������ 
void free_srv_list(T_CLI_Var *clip);
/*******************************************************
 * ȡ�÷������˷������б�
 * �������˵�0�ź��������ǵ�¼��֤��������¼��֤��ɺ�
 * �����0�ź���ֵ����  
 *      get_srvname();
 * 1�ź��������� 
 *   echo(); 
 * conn->freevarָ��free_srv_list,�����Ҫ�����ƺ����
 * Ӧ���ã������Ӧ����   
 * free_srv_list(T_CLI_Var *);
 * �ɹ�����0��ʧ��-1��
 *******************************************************/
int init_svc_no(T_Connect *conn);

#ifdef __cplusplus
}
#endif

#endif

/********************************************************************
 *int (* Function[])(T_Connect *conn,T_NetHead *NetHead) -- Used by server 
 * CLIENT or server.c befor include net.h,define NO_EXTERN_FUNCTION *
 ********************************************************************/

