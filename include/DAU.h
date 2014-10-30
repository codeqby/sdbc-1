/* DAU=Data Access Utility 
	2008-11-18 by ylh */


#ifndef DAUDEF

#define DAUDEF
#include <SRM.h>
#include <sqli.h>
#include <tree.h>

typedef struct {
	SRM			srm;		//SRM=Struct Relational Map
	T_SQL_Connect		*SQL_Connect;
	char 			*tail;		//bind����ʱָ��
	sqlo_stmt_handle_t	cursor;		//prepare cursor
	T_Tree			*bt_pre;	//prepare bind tree
	sqlo_stmt_handle_t	ins_sth;	//insert cursor
	T_Tree			*bt_ins;	//insert bind tree
	sqlo_stmt_handle_t	upd_sth;	//update cursor
	T_Tree			*bt_upd;	//update bind tree
	sqlo_stmt_handle_t	del_sth;	//delete cursor
	T_Tree			*bt_del;	//delete bind tree
	unsigned int		pos;            //DAU��ʹ��
} DAU;

// set col_to_lower = 1, ����ģ��ʱ������Сд��
extern char col_to_lower;

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
 * ��ʼ��DAU���������ݿ��record
 ********************************************/
int DAU_init(DAU *DP,T_SQL_Connect *SQL_Connect,char *tabname,void *record,T_PkgType *tp);

//msg ��������SQL���ͷ��ش�����Ϣ
int DAU_insert(DAU *DP,char *msg);

//where��where�Ӿ䣬ͬʱ��������ռ�����������SQL���
//����Ѿ���update��ͷ�ģ���ֱ�����ã���������
int DAU_update(DAU *DP,char *where);

//where��where�Ӿ䣬ͬʱ��������ռ�����������SQL���
//����Ѿ���delete��ͷ�ģ���ֱ�����ã���������
int DAU_delete(DAU *DP,char *where);
/****************************************************
 * ����select��䣬
 * where��where�Ӿ䣬ͬʱ��������ռ�����������SQL���
 * ����Ѿ���select��ͷ�ģ���ֱ�����ã���������
 ****************************************************/
int DAU_select(DAU *DP,char *where,int rownum);

/*****************************************************
 * ���α귽ʽ�򿪽����
 * where ����������䣬 ��Ҫ��������ռ�����������SQL���
 * ͬʱҲ�Ǵ��Fetch�������buffer
 * ����Ѿ���select��ͷ�ģ���ֱ�����ã���������
 ****************************************************/
int DAU_prepare(DAU *DP,char *where);

//ȡ����������һ����¼���ɹ�����0
//�ܹ��Զ�ʶ��DAU��select�Ļ���prepare��
int DAU_next(DAU *DP);

/*****************************************************
 * ִ���޷��ؽ������SQL��䣬ʹ��DAU_delete���α꣬
 * ��Ҫ��DAU_deleteͬʱʹ��
 * DAU�еĽṹ��ģ��������bind����Ҫ��
 * �漰������$DB.���Ž���DBOWNȡ����
 *****************************************************/
int DAU_exec(DAU *DP,char *stmt);
int bind_exec(DAU *DP,char *stmt);

/*********************************************
 * ����select
 * where��where�Ӿ䣬ͬʱ��������ռ�����������SQL���
 * ����Ѿ���select��ͷ�ģ���ֱ�����ã���������
 * ��֧��bind 
 *********************************************/
int DAU_getm(int n,DAU DP[],char *where,int rownum);

//����select, ȡ����������һ����¼���ɹ�����0
int DAU_nextm(int n,DAU *DP);

//�ͷ�DAU�����α�״̬�¹ر��α�
void DAU_free(DAU DP[]);
extern void  DAU_freem(int n,DAU *DP);

/* ���������ĺ���������������⻹����������������stmt������*stmt=0 */
extern int select_by_PK(DAU *DP,char *stmt);
extern int prepare_by_PK(DAU *DP,char *stmt);
extern int update_by_PK(DAU *DP,char *stmt);
extern int delete_by_PK(DAU *DP,char *stmt);
/* ���޸ģ�Ϊ�������ʱ�����ٶȺ��������⣬ע���������޸������ĳ�ͻ */
extern int dummy_update(DAU *DP,char *stmt);
// at SRM_to_DAU.c
extern int SRM_to_DAU(DAU *DP,T_SQL_Connect *SQL_Connect,SRM *srmp);
extern int DAU_to_SRM(DAU *DP,SRM *srmp);
extern int SRM_next(SRM *srmp);
// at bind.c
extern int bind_ins(DAU *DP,char *buf);
extern int bind_select(DAU *DP,char *stmt,int recnum);
extern int print_bind(void *content);
/**
 * ��ӡ bind ֵ����stmt�� 
 */
extern int DAU_print_bind(DAU *DP,char *stmt);

int get_upd_returning(DAU *DP);

#ifdef __cplusplus
}
#endif

//���ɰ��UPDATE���:"UPDATE DBOWN.TABNAME "
#define DAU_mk_update(DP,buf) SRM_mk_update(&(DP)->srm,(DP)->SQL_Connect->DBOWN,(buf))
/* ��ѡ����й���update��䣬���chooseΪ�գ�ȫ���� ,����β�� */
#define DAU_mk_upd_col(DP,choose,stmt) SRM_mk_upd_col(&(DP)->srm,(DP)->SQL_Connect->DBOWN,(choose),(stmt));


#define DAU_pkg_pack(DP,buf,dlimit) SRM_pkg_pack(&(DP)->srm,(buf),(dlimit))
#define DAU_pkg_dispack(DP,buf,dlimit) SRM_pkg_dispack(&(DP)->srm,(buf),(dlimit))
#define DAU_pack(DP,buf) SRM_pack(&(DP)->srm,(buf))
#define DAU_dispack(DP,buf) SRM_dispack(&(DP)->srm,(buf))
#define DAU_getString(DP,buf,key) SRM_getString(&(DP)->srm,(buf),(key))
#define DAU_setString(DP,buf,key) SRM_setString(&(DP)->srm,(buf),(key))
#define DAU_getOne(DP,buf,key) SRM_getOne(&(DP)->srm,(buf),(key))
#define DAU_putOne(DP,buf,key) SRM_putOne(&(DP)->srm,(buf),(key))

//#define DAU_patt_copy(DP,tpl,choose) SRM_patt_copy(&(DP)->srm,(tpl),(choose))

/**
 *  ���º���ȡ�������ָ�룬�������͡�������Ӧ���������
 */
// ������ȡָ�룬�����ʵ��������ؿ�ָ��
#define DAU_getP_by_key(DP,key) SRM_getP_by_key(&(DP)->srm,(key))
//���к�ȡָ�룬�����ʵ��кŷ��ؿ�ָ��
#define DAU_getP_by_index(DP,idx) SRM_getP_by_index(&(DP)->srm,(idx))

/* ��src�е�ͬ����Ա������desc,��ʽ�Զ�ת�� */
#define DAU_copy(desc,src,choose) SRM_copy(&(desc)->srm,&(src)->srm,(choose))

/* ��RETURNING�Ӿ�Ĳ������ */
#define DAU_ins_returning(DP,stmt) bind_ins((DP),(stmt))
/* ����RETURNING�Ӿ䣬����β�� */
#define DAU_mk_returning(DP,choose,stmt) SRM_mk_returning(&(DP)->srm,(choose),(stmt))
/* DAU_setBind():����ģ���е�bindtype,choose�������б�,NULLȫ���С�
   bindtype������0,NOSELECT,NOINSERT,NOSELECT|NOINSERT....
*/
#define DAU_setBind(DP,bindtype,choose) SRM_setBind(&(DP)->srm,(bindtype),(choose))

#define DAU_RecSize(DP) SRM_RecSize(&(DP)->srm)
/* ����keyȡģ�� */
#define DAU_getType(DP,key) SRM_getType(&(DP)->srm,(key))
#define DAU_getRec(DP) ((DP)->srm.rec)
#define DAU_except_col(DP,buf,except) SRM_except_col(&(DP)->srm,(buf),(except))

#endif
