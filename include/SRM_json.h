#include <SRM.h>
#include <json_pack.h>

#ifndef SRM_JSON
#define SRM_JSON

#ifdef __cplusplus
extern "C" {
#endif

JSON_OBJECT SRM_toJSON(SRM *srmp,JSON_OBJECT json,const char *choose);
int SRM_fromJSON(SRM *srmp,JSON_OBJECT json);
/* 将src中的同名成员拷贝到desc,格式自动转换 */
int SRM_copy(SRM *desc,SRM *src,const char *choose);

#ifdef __cplusplus
}
#endif

#endif

