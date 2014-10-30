
#ifndef SRMDEF

#define SRMDEF

#include <pack.h>

typedef struct {
	T_PkgType	*tp;			//ģ�� 
	char 		*result;		//�����
	char		*rp;			//���������ָ�� 
	void		*rec;			//���ݼ�¼
	const char	*hint;			//select֮�����ʾ���磬distinct,/*+rule*/
	int		Aflg;			//������+tp��recΪ�洢����
	const char	*pks;			//����
	const char	*tabname;
	char		*colidx;
	const char	*befor;			//�����ǰ�����ݣ��� with...as...select...
} SRM;

/* ��SRMģ�尴��ѡ���������ģ�� */
#define SRM_patt_copy(srm,tp,choose) patt_copy_col((tp),(srm).tp,(choose),(srm).colidx)

/* SRM_setBind:����ģ���е�bindtype,choose�������б�,NULLȫ���С�
   bindtype������0,NOSELECT,NOINSERT,NOSELECT|NOINSERT....
*/
#define SRM_setBind(srm,bindtype,choose) set_bindtype_idx((srm)->tp,(bindtype),(choose),abs((srm)->Aflg),(srm)->colidx)

#ifdef __cplusplus
extern "C" {
#endif

void SRM_init(SRM *srmp,void *record,T_PkgType *tp);
void SRM_free(SRM *srmp);
void PartternFree(SRM *srmp);
int mk_sdbc_type(char *type_name);

/* ����keyȡģ�� */
T_PkgType *SRM_getType(SRM *srmp,const char *key);

int SRM_pkg_pack(SRM *srmp,char *buf,char dlimit);
int SRM_pkg_dispack(SRM *srmp,char *buf,char dlimit);
#define SRM_pack(srmp,buf) SRM_pkg_pack((srmp),(buf),'|')
#define SRM_dispack(srmp,buf) SRM_pkg_dispack((srmp),(buf),'|')
char * SRM_getString(SRM *srmp,char *buf,char *key);
int SRM_setString(SRM *srmp,char *buf,char *key);
int SRM_getOne(SRM *srmp,char *buf,int idx);
int SRM_putOne(SRM *srmp,char *buf,int idx);
/**
 *  ���º���ȡ�������ָ�룬�������͡�������Ӧ���������
 */
// ������ȡָ�룬�����ʵ��������ؿ�ָ��
void *SRM_getP_by_key(SRM *srmp,char *key);
//���к�ȡָ�룬�����ʵ��кŷ��ؿ�ָ��
void *SRM_getP_by_index(SRM *srmp,int idx);

/* ����bind WHERE �Ӿ� */
char * mk_where(const char *keys,char *stmt);
char * SRM_mk_returning(SRM *srmp,char *keys,char *stmt);
int SRM_mk_select(SRM *srmp,char *DBOWN,char *where);
int SRM_mk_delete(SRM *srmp,char *DBOWN,char *where);
//���ɰ��UPDATE���:"UPDATE DBOWN.TABNAME "
char * SRM_mk_update(SRM *srmp,char *DBOWN,char *where);
/* ��ѡ����й���update��䣬���chooseΪ�գ�ȫ���� ,����β�� */
char * SRM_mk_upd_col(SRM *srmp,char *DBOWN,const char *choose,char *stmt);

#ifdef __cplusplus
}
#endif

#define SRM_RecSize(srmp) ((srmp)->tp[abs((srmp)->Aflg)].offset)
#define SRM_except_col(srmp,buf,except) except_col((buf),((srmp)->tp),(except))

#endif

