#include <SRM_json.h>

#ifndef DAU_JSON
#define DAU_JSON

#define DAU_toJSON(DP,json,choose) SRM_toJSON(&(DP)->srm,(json),(choose))
#define DAU_fromJSON(DP,json) SRM_fromJSON(&(DP)->srm,(json))
/* ��SRMģ�尴��ѡ���������ģ�� */
#define DAU_patt_copy(DP,tp,choose) SRM_patt_copy(&(DP)->srm,(tp),(choose))

#endif

