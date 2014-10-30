/* ttree.c ����ƽ������� */
#include <stdio.h>
#include <tree.h>

extern T_Tree * BB_Tree_Delete(T_Tree *tp,void *content_key,int size_key,
            int (*Comp)(void *node,void *content,int size_content),
			            void (user_free)(void *content),int *flg);

static int Tree_Cmp(rec1,rec2,len)
void *rec1;
void *rec2;
int len;
{
int *ip1,*ip2;
	ip1=(int *)rec1;
	ip2=(int *)rec2;
	if(*ip1 > *ip2) return 2;
	else if(*ip1 < *ip2) return -2;
    return 0;
}

void BB_Tree_print(T_Tree *sp)
{
int *ip;

    if(!sp)return ;
    if(sp->Left){
        BB_Tree_print(sp->Left);
    }
	ip=(int *)sp->Content;
	printf("content=%d,\t\tL=%d\tR=%d\n",*ip,sp->Ldepth,sp->Rdepth);
    if(sp->Right)
      BB_Tree_print(sp->Right);
    return ;
}

int main()
{
int num,ret;
T_Tree *root=0;
char buf[10];

	printf("����num��q����:\n");
	while((ret=scanf("%d",&num))==1) {
		root=BB_Tree_Add(root,&num,sizeof(num),Tree_Cmp,0);
	}
	BB_Tree_print(root);
	fgets(buf,sizeof(buf),stdin);
	while(!ferror(stdin) && root) {
		printf("����ɾ���Ľڵ�ţ�\n");
		ret=scanf("%d",&num);
		if(ret != 1) break;
		root=BB_Tree_Del(root,&num,sizeof(num),Tree_Cmp,0,&ret);
		printf("ret=%d,\t\tcontent\tL\tR\n",ret);
		BB_Tree_print(root);
	}
	BB_Tree_Free(&root,0);
	return 0;
}
