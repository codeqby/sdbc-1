#ifndef MULTI_HASH
#define MULTI_HASH

/**
 * ���� �����ظ���ֵ�ľ�̬hash��
 * ԭ���ݱ��������飬�ظ���ֵ��������һ��
 * hashһ��������ԭ���ݲ�������ɾ���ģ������ؽ�hash��
 */
typedef struct {
	void *data;		//ԭ��������
	int  data_count;	//ԭ���ݵĳߴ�
	int  key_count;		//�����������û�����룬=data_count
	int (*do_hash)(void *key,int key_count);//hash����
	void *(*getdata)(void *data,int n);//ȡ���ݵĺ���������&data[n]
	int (*key_cmp)(void *data,void *key);//�Ƚ�&data[n]��key�ĺ���,��ȷ���0
	void *index;//��������multi_hash���ɣ��ñ����Լ��ͷ�
} hash_paramiter;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * ���� �����ظ���ֵ��hash��
 * ԭ���ݱ��������飬�ظ���ֵ��������һ��
 * para��hash������
 * mult_hash()���س�ͻ����һ����key_count��1/3����,��ѡ��do_hashʱ���ο���
 */
int multi_hash(hash_paramiter *param);

/**
 * ����mukti_hash
 * para��hash������,��multi_hash��ͬ
 * ����ֵ�������±ꡣ-1û�ҵ�
 * a_count���ظ�key�����ݵ�����
 */
int multi_hash_find(void *key,hash_paramiter *para,int *a_count);

#ifdef __cplusplus
}
#endif

#endif
