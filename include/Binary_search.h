/********************************************************
 * Binary_search.h,ͨ�ö��ַ����ҳ���
 ********************************************************/
#ifndef BINARY_SEARCH
#define BINARY_SEARCH

#ifdef __cplusplus
extern "C" {
#endif
//����������±꣬-1û�ҵ�
int Binary_EQUAL(void *key,		//���Ҽ�ֵ��������data��ͬ
		  void *data,		//������������飬������Ӧ���Զ���
		  int data_count,	//����Ĵ�С
//�ȽϺ����������������������,����data[n]-*key�����򷵻�*key-data[n]��
		  int (*compare)(void *key,void *data,int n));
//GT,LT�ȴ�Сָ������˳��Ĵ�С��ע�ⷴ�������
int Binary_GT(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
int Binary_GTEQ(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
int Binary_LT(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
int Binary_LTEQ(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
//���������ڴ��ظ�ֵ����������
//=key�ĵ�һ��Ԫ�ص��±꣬-1=û�ҵ�,ע�⣺��STL��ͬ
int lowerBound(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
//>key�ĵ�һ��Ԫ�ص��±꣬-1=û�ҵ� ����������ϳ����ֲ�ѯ�����磺>,>=,<,<=.
int upperBound(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));

#ifdef __cplusplus
}
#endif

#endif
