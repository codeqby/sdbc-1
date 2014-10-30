
#ifndef BB_TREE_H
#define BB_TREE_H

#include <pthread.h>

struct B_Tree {
	int Ldepth;
	int Rdepth;
	struct B_Tree *Left;
	struct B_Tree *Right;
//	void *Content;
	char Content[0];
};
typedef struct B_Tree T_Tree;

#ifdef __cplusplus
extern "C" {
#endif
/*********************************************************************
 * BB_Tree_Add:�����ݼӵ���
 * ������
 * sp:���ڵ㣬content:�ڵ����ݣ��������κ����ݽṹ
 * len:content�ĳ���,Cmp_rec:��¼�Ƚ�����sp_content>contentʱ����>0,
 * <ʱ����<0,=ʱ����0��user_add_tree����ڵ�ʱ�����Ĵ������a
 * �����µĸ��ڵ� 
 ************************************************************************/
T_Tree * BB_Tree_Add( T_Tree *sp,void *content,int len, 
		int (*Cmp_rec)(void *sp_content,void *content,int len),
		int (*user_add_tree)(T_Tree *sp,void *content,int len));
/* BB_Tree_Scan: proc(): if return 0  continue sacn ; else break scan  */
int BB_Tree_Scan(T_Tree *sp, int (*proc)(void *content));
void BB_Tree_Free(T_Tree **sp,void (*user_free)(void *val));
//����=key�Ľڵ�
T_Tree * BB_Tree_Find(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *sp_content,void *content_key,int len));
//����>key�Ľڵ�
T_Tree * BB_Tree_GT(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
//����>=key�Ľڵ�
T_Tree * BB_Tree_GTEQ(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
//����<key�Ľڵ�
T_Tree * BB_Tree_LT(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
//����<=key�Ľڵ�
T_Tree * BB_Tree_LTEQ(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
#ifndef MAX
#define MAX(a,b) (((a)>(b))?a:b)
#endif

/* ɾ��ָ���ڵ㣬�����µĸ��ڵ�,*flg��ֵ=0,����0δɾ���������ʾ�ڵڼ���ɾ�� ,
   tp���ڵ�,content_key���Ҽ�ֵ,size_key,content_key�ĳ���,Comp�Ƚ�����
   user_free�û��Ľڵ�����ɾ������,����0ɾ���ɹ�,���ط�0�ڵ㽫����ɾ��.
   ����ֵ:�µĸ��ڵ� */

T_Tree * BB_Tree_Del(T_Tree *tp,void *content_key,int size_key,
            int (*Comp)(void *node,void *content,int size_content),
            int (*user_free)(void *content),int *flg);
T_Tree * BB_Tree_MAX(T_Tree *sp);
T_Tree * BB_Tree_MIN(T_Tree *sp);
//btΪ������ָ�룬����ֵΪbt�Ľڵ���*/
// contextΪӦ���ṩ�����������ݣ���counterʹ��
//counter��Ӧ���ṩ���ж��Ƿ���ϼ��������������Ϸ���0.
int BB_Tree_Count(T_Tree *  bt,void *context,
	int (*counter)(T_Tree *bt,void *context));

#ifdef __cplusplus
}
#endif

#endif
