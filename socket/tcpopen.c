
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <ctype.h>
#include <netdb.h>
#include <sc.h>
int tcpopen(char *host,char *server)
{
int i,mflg=0,ret;
int  s;
struct sockaddr_in sa;
struct hostent *hp=NULL;
struct servent *sp;
char addr[41],*cp;
char *envp; 
struct timeval tmout;

	if((hp=gethostbyname(host))==NULL){
		if(isdigit(*host)){
		    cp=host;
		    i=0;
		    while(cp&&*cp&&i<4){
			addr[i]=strtol(cp,&cp,10);
			if(*cp=='.')cp++;
			i++;
		    }
		    if((hp=gethostbyaddr(addr,4,AF_INET))==NULL){
		        hp=(struct hostent *)malloc(sizeof(struct hostent));
			if(!hp) {
				return MEMERR;
			}
		        hp->h_name=strdup(host);
		        hp->h_aliases=0;
		        hp->h_addrtype=AF_INET;
		        hp->h_length=4;
		        hp->h_addr_list=(char **)malloc(sizeof(char *)*2);
		        hp->h_addr_list[0]=(char *)malloc(sizeof(char *)*8);
		        hp->h_addr_list[1]=0;
		        memcpy(hp->h_addr,addr,4);
			mflg=1;
		    }
		} else {
			ShowLog(1,"gethostbyname %s error",host);
			return(-2);
		}
	}
	if(!hp->h_addr) {
                if(mflg) {free(hp->h_name);
                	free(hp->h_addr_list[0]);
          		free(hp->h_addr_list);
			free(hp);
		}
		return FORMATERR;
	}
	bcopy((char *)hp->h_addr,(char *)&sa.sin_addr,hp->h_length);
	sa.sin_family=hp->h_addrtype;
    	if(isdigit(server[0])){
	    sa.sin_port=htons((u_short)atol(server));
    	} else {
	    if((sp=getservbyname(server,"tcp"))==NULL){
                if(mflg) {free(hp->h_name);
                	free(hp->h_addr_list[0]);
          		free(hp->h_addr_list);
			free(hp);
		}
		ShowLog(1,"getsrvbyname %s error",server);
		return(-3);
	    }
	    sa.sin_port=(u_short)sp->s_port;
	}
	if((s=socket(hp->h_addrtype,SOCK_STREAM,0))<=0){
		i=errno;
                if(mflg) {free(hp->h_name);
                	free(hp->h_addr_list[0]);
          		free(hp->h_addr_list);
			free(hp);
		}
		ShowLog(1,"tcpopen socket error %d,%s",errno,strerror(errno));
		return -4;
	}
        if(mflg) {free(hp->h_name);
              	free(hp->h_addr_list[0]);
        	free(hp->h_addr_list);
		free(hp);
	}
/* ���Ҫ�Ѿ���������״̬��soket�ڵ���closesocket��ǿ�ƹرգ�������TIME_WAIT�Ĺ��̣�
BOOL bDontLinger = FALSE;
setsockopt(s,SOL_SOCKET,SO_DONTLINGER,(const char*)&bDontLinger,sizeof(BOOL));
for WINDOWS */

#ifdef LINUX
	i=1;
	setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i));
#endif
    envp=getenv("TCPTIMEOUT");
    if(envp && isdigit(*envp)) {
           tmout.tv_sec=atoi(envp);
           tmout.tv_usec=0;
           ret=setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char *)&tmout,sizeof(tmout));
           if(ret) {
                   ShowLog(1,"%s:setsockopt RCVTIMEO %d error %s",__FUNCTION__,tmout.tv_sec,strerror(errno));
           }
           ret=setsockopt(s,SOL_SOCKET,SO_SNDTIMEO,(char *)&tmout,sizeof(tmout));
           if(ret) {
                   ShowLog(1,"%s:setsockopt SNDTIMEO %d error %s",__FUNCTION__,tmout.tv_sec,strerror(errno));
           }
    }
	if((int)connect(s,(struct sockaddr *)&sa,sizeof(sa))< 0){
		ShowLog(1,"connect %s/%s  error %d",host,server,errno);
		i=errno;
		close(s);
		s=-1;
		errno=i;
	}
	return(s);
}
