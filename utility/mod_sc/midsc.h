#include <sdbc.h>
#include <sccli.h>
#include <json_pack.h>
#include "scpool.h"

#include "logs.stu"

typedef struct {
	char devid[17];
	char operid[17];
	char DBLABEL[256];
	T_Connect *server;
//	int status; //标志有状态服务
	char ShowID[100];
} GDA;

int Transfer(T_Connect *conn, T_NetHead *NetHead);
int login(T_Connect *conn,T_NetHead *NetHead);

