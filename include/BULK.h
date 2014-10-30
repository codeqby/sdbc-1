/*****************************************
 * BULK.h ORACLE BULK read 
 * SDBC��ORACLE��������������ݽṹ    
 * bindʱ��Ҫ�ı�ʾ�����м�״̬���г���    
 * defineʱ��Ҫ�ı�ʾ�����м�״̬���г���    
 * �Լ���SDBC�в�����ORACLEֱ�Ӷ�Ӧ������   
 * ��Ҫ������ַ�������  
 * ��.h��#include <DAU.h> ֮��ʹ��    
 *****************************************/

#ifndef BULK_A_COL_LEN

#define BULK_A_COL_LEN 32

typedef struct {	//����bind��define�ģ�ÿ��һ�� 
	char  *name;	//����  
	short *ind;	//��ʾ��, malloc(max_rows_of_batch * sizeof(short));
	short *r_code;	//�м�״̬��,  �ַ�����,      malloc(max_rows_of_batch * sizeof(short));
	short *r_len;	//���ص��г���,define_by_pos �ַ�������,   malloc(max_rows_of_fetch * sizeof(short));
	char *a_col; 	//������ַ������� malloc(max_rows_of_batch * BULK_A_COL_LEN); 
} col_bag;			// �а�   

typedef struct ora_bulk_desc {
	SRM *srm;			//SRM
	T_SQL_Connect *SQL_Connect;	//���ݿ���
	sqlo_stmt_handle_t sth;		//�α�
	int bind_rows;			//���������������
	int max_rows_of_fetch;		//ÿ����ȡ���������
	int cols;			//����bind������(��������)
	int dcols;			//����define������(������)
	col_bag *cb;			//malloc(cols * sizeof(col_bag));
	col_bag *dcb;			//malloc(dcols * sizeof(col_bag));
	T_Tree *bind_tree;		//����
	void *recs;			//���ݼ�¼ 
	int prows;			//�ۼ����� 
	int rows;			//ʵ�ʲ��������� 
	int reclen;			//��¼���� 
	int pos;			//BULK��ʹ��
} BULK;

#define BULK_get_DAU(bulkp) (DAU *)((bulkp)->srm)

#ifdef __cplusplus
extern "C" {
#endif

//BULK_init��DAU�����ͷ�,BULK_free���ͷ�DAU  
void BULK_init(BULK *bulk,DAU *DP,void *recs,int max_rows_of_fetch);
void BULK_free(BULK *bulk);
int BULK_prepare(BULK *bulkp,char *stmt,int bind_rows);
int BULK_fetch(BULK *bulk);
char * BULK_pkg_dispack(BULK *bulk,int n,char *buf,char delimit);
char * BULK_pkg_pack(BULK *bulk,int n,char *buf,char delimit);

#ifdef __cplusplus
}
#endif

#endif

