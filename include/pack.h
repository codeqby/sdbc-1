/*******************************************************
 * Secure Database Connect
 * SDBC 6.0 for ORACLE 
 * 2012.9.19 by ylh
 *******************************************************/

#ifndef PACKDEF
#define PACKDEF

#include <sys/types.h>
#include <datejul.h>

#define CH_BYTE 0
#define CH_CHAR 1
#define CH_TINY 2
#define CH_SHORT 3
#define CH_INT 4
#define CH_LONG 5
#define CH_INT64 6
#define	CH_FLOAT 7
#define CH_DOUBLE 8
#define CH_LDOUBLE 9
#define CH_STRUCT 125
#define CH_CLOB	126

extern const char NAN_NULL[sizeof(double)];
#define DOUBLENULL (*(double *)NAN_NULL)
#define FLOATNULL (*(float *)NAN_NULL)
#define SHORTNULL (short)(1<<(8*sizeof(short)-1)) 
#define INTNULL   (int)(1<<(8*sizeof(int)-1)) 
#define LONGNULL  (1L<<(8*sizeof(long)-1))
#define INT64NULL  ((INT64)1<<(8*sizeof(INT64)-1))
#define TINYNULL  0X80


/*****************************************************
 * define extend data type for net_pack()
 *****************************************************/
#define CH_DATE		(CH_CHAR|0x80) 
#define CH_CNUM		(CH_CHAR|0x100) 
#define CH_JUL  	(CH_INT4|0X80)
#define CH_MINUTS	(CH_INT4|0x100)
#define CH_TIME		(CH_INT64|0x80)
#define CH_USEC		(CH_INT64|0x200)
/* used by varchar2 as date(year to day),default format is "YYYYMMDD" */
#define CH_CJUL		(CH_INT4|0x200)
#define CH_CMINUTS	(CH_INT4|0x400)
/* used by varchar2 as date(year to sec),default format is "YYYYMMDDHH24MISS" */
#define CH_CTIME	(CH_INT64|0x100)

#define YEAR_TO_DAY_LEN 11
#define YEAR_TO_MIN_LEN 17
#define YEAR_TO_SEC_LEN 20
#define YEAR_TO_USEC_LEN 27

extern const char YEAR_TO_DAY[];
extern const char YEAR_TO_MIN[];
extern const char YEAR_TO_SEC[];
extern const char YEAR_TO_USEC[];
//���������ν�ġ�ģ�塱�ˣ���ʹ�����ܹ�"��"��δ֪�ṹ�����ݡ�
typedef struct {
	    INT4 type;
	    INT4 len; // in byte
	    const char *name;
	    const char *format;
	    INT4 offset;
	    int bindtype; //default=0
} T_PkgType;
// for bindtype
#define RETURNING 1     //this column only for returnning
#define NOINS	  2	//this column don't insert or update
#define NOSELECT  4     //this column don't select only used by bind

#ifdef NETVAR
#define VAR
#else
#ifdef __cplusplus
#define VAR extern "C" 
#else
#define VAR extern
#endif
#endif


VAR T_PkgType ByteType[];
VAR T_PkgType CharType[];
VAR T_PkgType TinyType[];
VAR T_PkgType ShortType[];
VAR T_PkgType IntType[];
VAR T_PkgType LongType[];
VAR T_PkgType DoubleType[];
VAR T_PkgType FloatType[];
#define MINUTSNULL INTNULL

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************
 * netvar.c
 *********************************************/
int set_offset(T_PkgType *pkg_type);
int isnull(void *vp,int type);

/***************************************************************
 * char *str="aaa|bbb||123||";
 * make values to char *values
 * values:  'aaa','bbb','',123,null
 **************************************************************/
extern char *mkvalues(char *values,char *str, T_PkgType *tp);
/* ֱ�Ӵӽṹ������Values
*/

extern char *mk_values(char *values,void  *data, T_PkgType *tp);
/* ����һ��vALUES�ע�⣬typ��ģ���е�һ������ ����value��β�� */
extern char *mkvalue(char *value,char *data,T_PkgType *typ);

/*******************************************************
 * ģ�濽����choose��ѡ��������Ϊ�գ�ȫ����
 * choose:"0-3,6-11",���к�ѡ��"devid,flag"������ѡ��
 * dest��������㹻�Ŀռ�
 *******************************************************/
int patt_copy_col(T_PkgType * dest,T_PkgType * src,const char *choose,char *colidx);
#define patt_copy(dest,src,choose) patt_copy_col((dest),(src),(choose),0)


/*************************************************************
 * create for UPDATE SET SUBstmt or INSERT
 * returned str="...,...,.."
 ************************************************************/
extern char *mkset(char *str, T_PkgType *tp);

/*************************************************************
 * create for SELECT
 * if tabname is null returned field="...,...,.."
 * else  returned field="tabname.field1,tabname.field2,..."
 ************************************************************/
extern char *mkfield(char *field, T_PkgType *tp,char *tabname);

/*************************************************************
 * create UPDATE SUBstmt for ORACLE
 * data="...|...|...|"
 * returned str="SET(..,..,..)=(SELECT ..,..,.. FROM DUAL)"
 *************************************************************/
extern char *mkupdate(char *str, char *data,T_PkgType *tp);
/* ֱ�Ӵӽṹ������ Update
*/
extern char *mk_update(char *str,void  *data,T_PkgType *tp);


/*************************************************************
 * pkg_pack 2009.5.13 by ylh
 *************************************************************/
extern int pkg_dispack(void *net_struct,char *buf,T_PkgType *pkg_type,char delimit);
#define net_dispack(stu,buf,tpl) pkg_dispack((stu),(buf),(tpl),'|')
extern int pkg_pack(char *buf,void *net_struct,T_PkgType *pkg_type,char delimit);
#define net_pack(buf,stu,tpl) pkg_pack((buf),(stu),(tpl),'|')
extern int get_one_str(char *str,void *data,T_PkgType *pkg_type,char dlmt);
#define get_one(str,data,pkg_type,i,dlmt) get_one_str((str),(data),&(pkg_type)[(i)],(dlmt))
extern int put_str_one(void *data,char *str,T_PkgType *pkg_type,char dlmt);
#define put_one(data,str,pkg_type,i,dlmt) put_str_one((data),(str),&(pkg_type)[(i)],(dlmt))
/*******************************************
 * �ұ���
 *******************************************/
extern const char *plain_name(const char *name);
extern int pkg_getnum(char *key,T_PkgType *tpl);
extern T_PkgType * pkg_getType(char *key,T_PkgType *tpl);
/* getitem: if key exist return content(in buf),else return NULL */
extern char *getitem_idx(char *buf,void *stu,T_PkgType *tpl,char *key,char *colidx,int colnum);
#define getitem(buf,stu,tpl,key) getitem_idx((buf),(stu),(tpl),(key),0,0)
/* putitem: if key exist return fieldnumber(>=0),else return -1 */
extern int putitem_idx(void *stu,char *buf,T_PkgType *tpl,char *key,char *colidx,int colnum);
#define putitem(stu,buf,tpl,key) getitem_idx((stu),(buf),(tpl),(key),0,0)
extern int data_init(void *data,T_PkgType *type);
extern char *mk_col_idx(T_PkgType *tpl);
/* ˫дsrc�еĵ����� ����β�� */
char * ext_copy(char *dest,const char *src);
/* �жϸ�ʽ����Ƿ�timestamp ���� */
char * is_timestamp(char *format);
extern int index_col(const char *idx,int colnum,char *key,T_PkgType *tp);
/* ��ģ����ѡȡ��ȥexcept֮�������->buf,�������� */
int except_col(char *buf,T_PkgType *tp,const char *except);
/* ���ģ���еİ󶨱�־��flg=NOINS or RETURNING or NOSELECT */
#define ALL_BINDTYPE -1
void clean_bindtype(T_PkgType *tp,int flg);
int set_bindtype_idx(T_PkgType *tp,int bindtype,const char *choose,int cols,char *idx);

#ifdef __cplusplus
}
#endif
/* set_bindtype():����ģ���е�bindtype,choose�������б�,NULLȫ���С�
   bindtype������0,NOSELECT,NOINSERT,NOSELECT|NOINSERT....
*/
#define set_bindtype(tp,bindtype,choose) set_bindtype_idx((tp),(bindtype),(choose),0,NULL)

#endif
