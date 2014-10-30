/*******************************************************
 * Secure Database Connect
 * sqli.h -- SDBC 6.0 for ORACLE 
 * 2012.9.13 by ylh
 *******************************************************/

#ifndef SQLIDEF
#define SQLIDEF

#define MAXCONNECT 8
#define MAXCURSOR MAXCONNECT*16 
#define TRANBEGIN 0
#define TRANCOMMIT 1
#define TRANROLLBACK 2

/* For ORACLE */
#define SQLNOTFOUND 1403
#define FETCHEND    100
#define DUPKEY   1
#define LOCKED 		 54
#define DBFAULT	3114

#include <pack.h>
#include <sqlora.h>

typedef struct S_SQL_Connect {
	char DSN[81];			/*database server name*/
	char UID[81];			/*database user ID*/
	char PWD[81];			/*database user password*/
	char DBOWN[81];
  	sqlo_db_handle_t dbh;		/*oracle oci ���ݿ���*/
	char ErrMsg[2048];
	char SqlState[128];
	int NativeError;
	int Errno;
	unsigned int pos;		/* �����ӳ��е�λ�� */
} T_SQL_Connect;

#define SQL_Rpc ORA_Rpc
#ifdef __cplusplus
extern "C" {
#endif

/* Kernel database functions */
extern int ___SQL_GetError(T_SQL_Connect *SQL_Connect);
int ___SQL_init_sqlora(int maxconnect,int maxcurs);
extern int ___SQL_OpenDatabase__( T_SQL_Connect *sql_var);
extern int ___SQL_Prepare(T_SQL_Connect *SQL_Connect,char *stmt,int bind_num,char *bind_v[]);
#define ___SQL_Prepare__(SQL_Connect,stmt)  ___SQL_Prepare(SQL_Connect,stmt,0,0)
extern char *___SQL_Fetch(T_SQL_Connect *SQL_Connect,int curno,int *recnum);
extern int ___SQL_Close__(T_SQL_Connect *SQL_Connect,int CursorNo);
extern int	___SQL_CloseDatabase__( T_SQL_Connect *SQL_Connect );
extern int ___SQL_Init_SQL_Connect(T_SQL_Connect *SQL_Connect);
extern int ___SQL_Exec(T_SQL_Connect *SQL_Connect ,char *stmt);
extern int ___SQL_Free_SQL_Connect(int pth_id,int Flag);
extern int FreeFullConnectPool();
extern int SQL_Check_Stmt(char *cmd);
extern int ___SQL_Transaction__(T_SQL_Connect *SQL_Connect,int flag);
extern int ___SQL_Select__(T_SQL_Connect *SQL_Connect,char *stmt,
						char **rec,int recnum);
int db_open1(T_SQL_Connect *SQL_Connect,char *dblabel);
char *decodeprepare(char *dblabel);
int ORA_Rpc(T_SQL_Connect *SQL_Connect,char *cmd,char **result);
//��֧�ְ󶨱��������Ƽ�ʹ�ã��� DAU_insert ����֮   
int insert_db(T_SQL_Connect *SQL_Connect,char *tabname,void *data,T_PkgType *type,char *msg);

/********************************************************************
 * �ṩ���̻߳��������ӳز���  (pool_new.c)
 * ��������  
 * DBPOOLNUM �������ݿ����ӳ���,ÿ���ؿ������Ӳ�ͬ�����ݿ���û���ȱʡ1��  
 * DBLABELn �������ݿ����ӳص����ݿ��ǩ��n=0~DBPOOLNUM-1��  
 * DBPOOLn_NUM ����ÿ�����ݿ����ӳص������� �� ȱʡ1�� n=0~DBPOOLNUM-1��
 ********************************************************************/

//�ͷ����ݿ����ӳ�  
void free_DB_pool(void);
//��ʼ�����ݿ� ���ӳ�  
int init_DB_pool(void);
//�������ݿ�  һ�㲻ʹ�ã������ӳ��ڲ����  
int connect_db(T_SQL_Connect *SQL_Connect,char *DBLABEL);
//ȡ���ݿ�����  
int _get_DB_connect(T_SQL_Connect **SQL_Connect,int poolno,int flg);
//�黹���ݿ�����  
void release_DB_connect(T_SQL_Connect **SQL_Connect,int poolno);
//���ݿ�ؼ�� 
void dbpool_check(void);
/**
 * ����DBLABELȡ���ݿ�غ�  
 * ʧ�ܷ���-1
 */
int get_dbpool_no(char *DBLABEL);
int get_DBpoolnum();
int get_rs_num(int poolno);
char * get_DBLABEL(int poolno);

#ifdef __cplusplus
}
#endif

#define get_DB_connect(SQL_Connect,poolno) _get_DB_connect(SQL_Connect,poolno,0)
#define db_open(SQL_Connect) db_open1((SQL_Connect),NULL)

#endif
