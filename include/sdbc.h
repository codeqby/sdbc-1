/*******************************************************
 * Secure Database Connect
 * SDBC 6.0 for ORACLE 
 * 2012.09.15 by ylh
 *******************************************************/

#ifndef SDBCDEF
#define SDBCDEF

#include <sqli.h>
#include <scsrv.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Server database interface functions */
extern int SQL_OpenDatabase(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Prepare(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Select(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Exec(T_Connect *connect ,T_NetHead *NetHead);
extern int SQL_Fetch(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Close(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_RPC(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_CloseDatabase(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_EndTran(T_Connect *connect,T_NetHead *NetHead);
extern int locktab(T_Connect *connect,T_NetHead *NetHead);
//extern int SQL_Quit(T_SRV_Var *);
extern const char * _get_data_type_str(int dtype);
/************************************************
 * mod_DB.c:���ݿ����ӳظ߼�������
 ************************************************/ 
/**
 * DB_connect_MGR:�����ݿ����ӳظ߼��������������ݿ�����(mod_DB.c)
 * ����ֵ:0:�ɹ�ȡ�����ӡ�1:����æ���������������У�-1:����
 */
int DB_connect_MGR(int TCBno,int poolno,T_SQL_Connect **sqlp,sdbcfunc call_back);
/* �ͷ�mod_DB�Ķ��нṹ   */
void mod_DB_free(void);
/* ȡ�������ṹ */
T_SRV_Var * get_SRV_Var(int TCBno);
/******************************************************
 * bind_DB.c
 * ���mod_DB,����ȡ�����ݿ����Ӻ󣬴��ݸ���Ӧ��TCB,
 * �����Ӧ��TCB�г�ȥ֮�� ���Ա�����Ӧ����������
 *****************************************************/
int bind_DB(int TCBno,T_SQL_Connect *sql);
int unbind_DB(int TCBno);



#ifdef __cplusplus
}
#endif

#endif
