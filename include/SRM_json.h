#include <SRM.h>
#include <json_pack.h>

#ifndef SRM_JSON
#define SRM_JSON

#ifdef __cplusplus
extern "C" {
#endif

JSON_OBJECT SRM_toJSON(SRM *srmp,JSON_OBJECT json,const char *choose);
int SRM_fromJSON(SRM *srmp,JSON_OBJECT json);
/* ��src�е�ͬ����Ա������desc,��ʽ�Զ�ת�� */
int SRM_copy(SRM *desc,SRM *src,const char *choose);

#ifdef __cplusplus
}
#endif

#endif

