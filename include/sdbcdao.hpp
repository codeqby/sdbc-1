/*********************************************************
 * DAU��C++�ӿ�
 *********************************************************/

#ifndef DAUDEF

#include <DAU.h>
#include <DAU_json.h>

class SdbcDAO {
	protected:
		DAU dau;
	public:
//���캯������������
//��ģ��
	SdbcDAO(void)
        {
            DAU_init(&dau,NULL,NULL,NULL,NULL);
        }

//�����ݿ��ȡģ��
	SdbcDAO(T_SQL_Connect *SQL_Connect,char *tabname)
        {
            int ret;

            ret=init(SQL_Connect,tabname,NULL,NULL);
            if(ret<0) {
                throw Showid;
            }
        }
//ʹ�þ�̬ģ�壬����ģ���ȡ����
	SdbcDAO(T_SQL_Connect *SQL_Connect,void *rec,T_PkgType *tpl)
        {
            init(SQL_Connect,NULL,rec,tpl);
        }

//�ñ���ȡ��ģ����ı���
	SdbcDAO(T_SQL_Connect *SQL_Connect,char *tabname,void *rec,T_PkgType *tpl)
	{
            int ret;
            ret=init(SQL_Connect,tabname,rec,tpl);
            if(ret<0) {
                throw Showid;
	    }
	}
//���Ѵ��ڵĶ������³�ʼ��
	int  init(T_SQL_Connect *SQL_Connect,char *tabname,void *rec,T_PkgType *tpl)
	{
	 	DAU_init(&dau,SQL_Connect,tabname,rec,tpl);
	}

	void Data_init(void)
	{
            if(dau.srm.tp) data_init(dau.srm.rec,dau.srm.tp);
	}

//��������
	~SdbcDAO(void)
        {
            DAU_free(&dau);
        }

//bean����
	DAU *getDAU()
	{
	 return &dau;
	}

	T_SQL_Connect *getSqlConnect(void)
	{
	 return dau.SQL_Connect;
	}

	T_PkgType *getTemplate()
	{
	 return dau.srm.tp;
	}

	T_PkgType *getTemplate(const char *key)
	{
	int n;
           n=index_col(dau.srm.colidx,abs(dau.srm.Aflg),key,dau.srm.tp);
           if(n<0) return NULL;
           return &dau.srm.tp[n];
	}

	int getTplNo(const char *colName)
	{
	 return index_col(dau.srm.colidx,abs(dau.srm.Aflg),colName,dau.srm.tp);
	}

	int getColNum(void)
	{
	 return abs(dau.srm.Aflg);
	}

	int getRecSize(void)
	{
	 if(!dau.srm.tp) return -1; 
	 return dau.srm.tp[abs(dau.srm.Aflg)].offset;
	}

	void *getRec(void)
	{
	 return dau.srm.rec;
	}

	void *getRec(char *colname)
	{
	 return DAU_getP_by_key(&dau,colname);
	}

	void *getRec(int col_no)
	{
	 return DAU_getP_by_index(&dau,col_no);
	}

	void setHint(const char *hint)
	{
	 dau.srm.hint=hint;
	}

	void setBefor(const char *befor)
	{
	 dau.srm.befor=befor;
	}

//��������
	int select(char *stmt)
	{
	  return DAU_select(&dau,stmt,0);
	}

	int select(char *stmt,int recnum)
	{
	  return DAU_select(&dau,stmt,recnum);
	}

	int nextRow(void)
	{
	  return DAU_next(&dau);
	}

	int prepare(char *stmt)
	{
	  return DAU_prepare(&dau,stmt);
	}

	int insert(char *stmt)
	{
	  return DAU_insert(&dau,stmt);
	}

	int update(char *stmt)
	{
	  return DAU_update(&dau,stmt);
	}

	int Delete(char *stmt)
	{
	  return DAU_delete(&dau,stmt);
	}

	int exec(char *stmt)
	{
	  return DAU_exec(&dau,stmt);
	}

//����ѡ��������
	int selectCol(const char *choose)
	{
	char stmt[10240];
	int ret=DAU_except_col(&dau,stmt,choose);
    		if(ret<=0) return ret;
		DAU_setBind(&dau,NOSELECT,stmt);
	}

	int distinctCol(const char *choose)
	{
	  dau.srm.hint="DISTINCT";
	  return selectCol(choose);
	}

//��ѡ����й���update��䣬���chooseΪ�գ�ȫ���� ,����β��
	char *mkUpdateByCol(const char *choose,char *stmt)
	{
	  return DAU_mk_upd_col(&dau,choose,stmt);
	}

	int setBind(int bindtype,const char *choose)
	{
	  return DAU_setBind(&dau,bindtype,choose);
	}

	int exceptCol(char *buf,char *choose)
	{
	  return DAU_except_col(&dau,buf,choose);
	}

//��ӡ���������İ󶨱���ֵ
	int printBind(char *buf)
	{
	  return DAU_print_bind(&dau,buf);
	}

//���л�/�����л�
	int pack(char *buf)
	{
	  return DAU_pack(&dau,buf);
	}

	int pack(char *buf,char delimit)
	{
	  return DAU_pkg_pack(&dau,buf,delimit);
	}

	int unpack(char *buf)
	{
	  return DAU_dispack(&dau,buf);
	}

	int unpack(char *buf,char delimit)
	{
	  return DAU_pkg_dispack(&dau,buf,delimit);
	}

	int getOne(char *buf,T_PkgType *tpl)
	{
	  return get_one_str(buf,dau.srm.rec,tpl,0);
	}

	int getOne(char *buf,const char *col_name)
	{
	  return getOne(buf,getTemplate(col_name));
	}

	int putOne(char *buf,T_PkgType *tpl)
	{
	  return put_str_one(dau.srm.rec,buf,tpl,0);
	}

	int putOne(char *buf,const char *col_name)
	{
	  return putOne(buf,getTemplate(col_name));
	}

#ifdef DAU_JSON
	JSON_OBJECT toJSON(JSON_OBJECT json,char *choose)
	{
	  return DAU_toJSON(&dau,json,choose);
	}

	int fromJSON(JSON_OBJECT json)
	{
	  return DAU_fromJSON(&dau,json);
	}
#endif
};

#endif
