/**************************************************
 * Ϊÿ���̷߳�����Showid����tpool.c���� 
 * Showid��λ����Ӧ�ó������գ����ﲢ��֪��
 * ��������պ�����Ӧ����Ӧ��ϵͳ���� 
#include "../thread/thrsrv.h"
 **************************************************/

#include <sys/types.h>

#ifdef __cplusplus
extern "C" 
#endif

void set_showid(void *ctx)
{
//pthread_t tid=pthread_self();
//GDA *gp=(GDA *)ctx;
	if(!ctx) return;
//	mthr_showid_add(tid,gp->ShowID);
}
