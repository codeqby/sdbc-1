/*****************************************
 * OAD.h ORACLE Array Describe 
 * SDBC��ORACLE��������������ݽṹ    
 * bindʱ��Ҫ�ı�ʾ�����м�״̬���г���    
 * �Լ���SDBC�в�����ORACLEֱ�Ӷ�Ӧ������   
 * ��Ҫ������ַ�������  
 * ��.h��#include <DAU.h> ֮��ʹ��    
 *****************************************/

#ifndef A_COL_LEN

#define A_COL_LEN 32

typedef struct {	//����bind�ģ�ÿ��һ�� 
	char  *name;	//����  
	short *ind;		//��ʾ��, malloc(max_rows_of_batch * sizeof(short));
	short *r_code;	//�м�״̬��,  �ַ�����,      malloc(max_rows_of_batch * sizeof(short));
	short *r_len;	//���ص��г���,define_by_pos �ַ�������,   malloc(max_rows_of_batch * sizeof(short));
	char *a_col; 	//������ַ������� malloc(max_rows_of_batch * A_COL_LEN); 
} col_bag;			// �а�   

typedef struct ora_array_desc {
	SRM *srm;			//SRM
	T_SQL_Connect *SQL_Connect;	//���ݿ���
	sqlo_stmt_handle_t sth;		//�α�
	int max_rows_of_batch;		//ÿ���������
	int cols;			//����bind������
	col_bag *cb;			//malloc(cols * sizeof(col_bag));
	T_Tree *bind_tree;		//����
	void *recs;			//���ݼ�¼ 
	int begin;			//execʱ����ʼ�к� 
	int rows;			//ʵ�ʲ��������� 
	int reclen;			//��¼���� 
	int a_col_flg;			//�����в��� 
	unsigned int pos;		//OAD��ʹ��
} OAD;

#define OAD_get_DAU(oadp) (DAU *)((oadp)->srm)

#ifdef __cplusplus
extern "C" {
#endif

//OAD_init��DAU�����ͷ�,OAD_free���ͷ�DAU  
void OAD_init(OAD *oad,DAU *DP,void *recs,int max_rows_of_batch);
void OAD_free(OAD *oad);
int OAD_mk_ins(OAD *oad,char *stmt);
int OAD_mk_update(OAD *oadp,char *stmt);
int OAD_exec(OAD *oad,int begin,int rowno);
char * OAD_pkg_dispack(OAD *oad,int n,char *buf,char delimit);
char * OAD_pkg_pack(OAD *oad,int n,char *buf,char delimit);

#ifdef __cplusplus
}
#endif

#endif

