#include "midsc.h"


#define M_ST_OFF(struct_type, member)    \
        (int)((void *) &(((struct_type *) 0)->member))

T_PkgType SCPOOL_tpl[]={
        {CH_INT,sizeof(int),"D_NODE",0,-1},
        {CH_CHAR,17,"DEVID"},
        {CH_CHAR,256,"LABEL"},
        {CH_CHAR,17,"UID"},
        {CH_CHAR,14,"PWD"},
        {CH_INT,sizeof(int),"NUM"},
        {CH_INT,sizeof(int),"NEXT_D_NODE"},
        {CH_CHAR,81,"HOST"},
        {CH_CHAR,21,"PORT"},
        {CH_INT,sizeof(int),"MTU"},
        {CH_CHAR,172,"family"},
        {-1,0,0,0}
};

typedef struct {
        int d_node;
        char DEVID[17];
        char LABEL[256];
        char UID[17];
        char PWD[14];
        int NUM;
        int NEXT_D_NODE;
        char HOST[81];
        char PORT[21];
        int MTU;
        char family[172];
} SCPOOL_stu;


typedef struct  {
	int a1;
	short a2;
	char a3[9];
	int a4;
}a;

int main()
{
//SCPOOL_stu node;
	printf("D_NODE offset=%d\n",M_ST_OFF(SCPOOL_stu,d_node));
	printf("DEVID offset=%d\n",M_ST_OFF(SCPOOL_stu,DEVID));
	printf("LABEL offset=%d\n",M_ST_OFF(SCPOOL_stu,LABEL));
	printf("UID offset=%d\n",M_ST_OFF(SCPOOL_stu,UID));
	printf("PWD offset=%d\n",M_ST_OFF(SCPOOL_stu,PWD));
	printf("NUM offset=%d\n",M_ST_OFF(SCPOOL_stu,NUM));
	printf("NEXT_D_NODE offset=%d\n",M_ST_OFF(SCPOOL_stu,NEXT_D_NODE));
	printf("HOST offset=%d\n",M_ST_OFF(SCPOOL_stu,HOST));
	printf("PORT offset=%d\n",M_ST_OFF(SCPOOL_stu,PORT));
	printf("MTU offset=%d\n",M_ST_OFF(SCPOOL_stu,MTU));
	printf("family offset=%d\n",M_ST_OFF(SCPOOL_stu,family));
	printf("\n");
	printf("a1 offset=%d\n",M_ST_OFF(a,a1));
	printf("a2 offset=%d\n",M_ST_OFF(a,a2));
	printf("a3 offset=%d\n",M_ST_OFF(a,a3));
	printf("a4 offset=%d\n",M_ST_OFF(a,a4));
	return 0;
}
