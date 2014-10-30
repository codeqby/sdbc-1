/*******************************************
 * sc.h: Secure Connect Package
 * 2014.09 by YuliHua for SDBC7.0
 *******************************************/
#ifndef HEADPACKLENGTH

#include <errno.h>
#include <sys/types.h>
#include <strproc.h>
#include <enigma.h>

#define HEADPACKLENGTH 36

#ifndef INTNULL
#define INTNULL 0X80000000
#endif

#define		PARANUM 9
#define 	PROTO_NUM   para[0]     	/*Э���:
					�ͻ����з�����ʱ�ǵ��úţ�
					����������ʱ���¼��ţ�1��65535 */
#define		ERRNO1	    para[1]     	/*��������	*/
#define		ERRNO2	    para[2]     	/*����������	*/
#define		PKG_REC_NUM para[3]      	/*���ݼ�¼��	*/
#define		PKG_LEN	    para[4]        	/*���ݰ�����	*/
#define		T_LEN	    para[5]   		/* ���䳤��  */
#define		O_NODE      para[6]      	/*ԭ����ַ*/
#define		D_NODE	    para[7]		/*Ŀ�Ľ������*/
#define		PKG_CRC	    para[8]		/*���ݰ�CRC */

typedef struct {             	/*Э��ͷ	*/
	int	para[PARANUM];
	char	*data;
} T_NetHead;

typedef struct S_Connect {
	int	Socket;			/*socket*/
	char	Host[81];		/*���������ַ*/
	char	Service[21];		/*�ſ�������*/
	u_int   *family;		/* ���ʶ����Կ */
	INT4	SendLen;
	char	*SendBuffer;		/*���ͻ�����*/
	INT4	RecvLen;
	char	*RecvBuffer;		/*���ܻ�����*/
	int	CryptFlg;		/* ���ܱ�־ */
	ENIGMA2 t; 			/* ���ܲ����� */
	void 	*Var;  			/* �û���������ָ�� Free By user,*/
	void	(*freevar)(void *); 	/* offer function FreeVar(void *p);*/
	int	(*only_do)(struct S_Connect *,T_NetHead *);
	unsigned int 	timeout;	/* for second */
	unsigned int	MTU;
	unsigned int 	status;		/* ����״̬,0:��״̬ */
/* �¼��������ĵ�ַ */
	int	(*Event_proc)(struct S_Connect *conn,int id);
	unsigned int pos; //���ӳ���
} T_Connect;

/* define for T_Connect.CryptFlg*/
#define DO_CRYPT 3
#define CHECK_CRC 4
#define DO_ZIP 8
#define UNDO_ZIP 0X80000000

#define SDBC_BLKSZ 65536
/* NetHead->ERRNO2,ת������־ */
#define PACK_CONTINUE 0X80000000

/* �ͻ���֪ͨת�������ð�����Ҫ�������ش� */
#define PACK_NOANSER ((PACK_CONTINUE)|1)

/* NetHead->ERRNO2,��״̬�����־ */
#define PACK_STATUS 0X80000002

typedef struct {
    char  sqlname[49];
    int sqltype;
    int sqllen;
    char  sqlformat[49];
} T_SqlVar;

typedef struct {
    int  cursor_no;
    int cols;
    T_SqlVar *sqlvar;
} T_SqlDa;

typedef int (*sdbcfunc)(T_Connect *conn,T_NetHead *head);

#ifdef __cplusplus
extern "C" {
#endif


int quick_send_pkg(T_Connect *connect,T_NetHead *nethead);
int  SendPack(T_Connect *connect,T_NetHead *nethead);
int  RecvPack(T_Connect *connect,T_NetHead *nethead);
int peeraddr(int socket,char in_addr[16]);
char *StrAddr(INT4 addr,char str[16]);
INT4 LongAddr(char *p);
INT4 LocalAddr(int Sock, char szAddr[16]);
int tcpopen(char *host,char *server);
int SendNet(int socket,char *buf,int len,int MTU);
//timout for second 
int RecvNet(int s,char *buf,int n,int timeout);
/*********************************************
 * init the T_Connect 
 *********************************************/
void initconnect(T_Connect *connect);

/***************************************
 * client end
 * disconnect, free T_Connect.
 ***************************************/
void disconnect(T_Connect *conn);
void freeconnect(T_Connect *conn);

#ifdef __cplusplus
}
#endif

#endif
