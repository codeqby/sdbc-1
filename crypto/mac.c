#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
//#include <sys/sockio.h>

#define MAXINTERFACES 8

/*
��linux�£��ж�����״̬
ioctl(sockfd, SIOCGIFFLAGS, &ifr);
return ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING));
���Կ������Ŀ¼����֪���ж�������
/sys/class/net/

*/
int get_mac(char* out)
{
char *mac;
register int fd,intrface;
struct ifreq buf[MAXINTERFACES];
struct ifconf ifc;

	*out=0;
	mac=out;
	if((fd=socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		return -1;
	}
	ifc.ifc_len =  sizeof buf;
	ifc.ifc_buf = (caddr_t)buf;
	if(ioctl(fd,SIOCGIFCONF,(char *)&ifc))
	{
		close(fd);
		return -2;
	}
	// ��ȡ�˿���Ϣ
	intrface = ifc.ifc_len/sizeof(struct ifreq);
	// ���ݶ˿���Ϣ��ȡ�豸IP��MAC��ַ
	while(intrface-- > 0 )
	{
 		if(!strcmp(buf[intrface].ifr_name,"lo")) {
			continue;
		}
/*
		// ��ȡ�豸����
		if(!(ioctl(fd,SIOCGIFFLAGS,(char *)&buf[intrface])))
		{
			if(buf[intrface].ifr_flags & IFF_PROMISC)
			{
				retn++;
			}
		} else {
			ShowLog(1,"%s: ioctl device %s",__FUNCTION__,buf[intrface].ifr_name);
			continue;
		}
*/
		// ��ȡMAC��ַ
		if(ioctl(fd,SIOCGIFHWADDR,(char *)&buf[intrface]))
		{
			continue;
		}
		mac+=sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X;",
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[0],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[1],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[2],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[3],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[4],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]);
	}
	close(fd);
	return 0;
}
