/*
 * LB Dispatch - G5
 * Author  : calvin
 * Email   : calvinwillliams.c@gmail.com
 * History : 2014-03-29 v1.0.0   create
 *
 * Licensed under the LGPL v2.1, see the file LICENSE in base directory.
 */

#ifndef _H_G5_
#define _H_G5_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>

#define VERSION		"1.0.0"

/*
config file format :
rule_id	mode	client_addr -> forward_addr -> server_addr ;
	mode		G  : manage port
			MS : master/slave mode
			RR : round & robin mode
			LC : least connection mode
			RT : response Time mode
			RD : random mode
			HS : hash mode
		client_addr	format : ip1.ip2.ip3.ip4:port
				ip1~4,port allow use '*' or '?' for match
				one or more address seprating by blank
		forward_addr	format : ip1.ip2.ip3.ip4:port
				one or more address seprating by blank
		server_addr	format : ip1.ip2.ip3.ip4:port
				one or more address seprating by blank
demo :
admin	G	192.168.1.79:* - 192.168.1.54:8060 ;
webdog	MS	192.168.1.54:* 192.168.1.79:* - 192.168.1.54:8079 > 192.168.1.79:8089 192.168.1.79:8090 ;
hsbl	LB	192.168.1.*:* - 192.168.1.54:8080 > 192.168.1.79:8089 192.168.1.79:8090 192.168.1.79:8091 ;

manage port command :
	ver
	list rules
	add rule ...
	modify rule ...
	remove rule ...
	dump rule
	list forwards
	quit
demo :
	add rule webdog2 MS 1.2.3.4:1234 - 192.168.1.54:1234 > 4.3.2.1:4321 ;
	modify rule webdog2 MS 4.3.2.1:4321 - 192.168.1.54:1234 > 1.2.3.4:1234 ;
	remove rule webdog2 ;
*/

#define FOUND				9	/* �ҵ� */
#define NOT_FOUND			4	/* �Ҳ��� */

#define MATCH				1	/* ƥ�� */
#define NOT_MATCH			-1	/* ��ƥ�� */

#define RULE_ID_MAXLEN			64	/* �ת������������ */
#define RULE_MODE_MAXLEN		2	/* �ת������ģʽ���� */

#define FORWARD_RULE_MODE_G		"G"	/* ����˿� */
#define FORWARD_RULE_MODE_MS		"MS"	/* ����ģʽ */
#define FORWARD_RULE_MODE_RR		"RR"	/* ��ѯģʽ */
#define FORWARD_RULE_MODE_LC		"LC"	/* ��������ģʽ */
#define FORWARD_RULE_MODE_RT		"RT"	/* ��С��Ӧʱ��ģʽ */
#define FORWARD_RULE_MODE_RD		"RD"	/* ���ģʽ */
#define FORWARD_RULE_MODE_HS		"HS"	/* HASHģʽ */

#define RULE_CLIENT_MAXCOUNT		10	/* �������������ͻ����������� */
#define RULE_FORWARD_MAXCOUNT		3	/* �������������ת������������ */
#define RULE_SERVER_MAXCOUNT		100	/* ������������������������� */

#define DEFAULT_FORWARD_RULE_MAXCOUNT	100	/* ȱʡ���ת���������� */
#define DEFAULT_CONNECTION_MAXCOUNT	1024	/* ȱʡ����������� */ /* ���ת���Ự���� = ����������� * 3 */
#define DEFAULT_TRANSFER_BUFSIZE	4096	/* ȱʡͨѶת����������С */

/* �����ַ��Ϣ�ṹ */
struct NetAddress
{
	char			ip[ 64 + 1 ] ;
	char			port[ 10 + 1 ] ;
	struct sockaddr_in	sockaddr ;
} ;

/* �ͻ�����Ϣ�ṹ */
struct ClientNetAddress
{
	struct NetAddress	netaddr ;
	int			sock ;
} ;

/* ת������Ϣ�ṹ */
struct ForwardNetAddress
{
	struct NetAddress	netaddr ;
	int			sock ;
} ;

/* �������Ϣ�ṹ */
struct ServerNetAddress
{
	struct NetAddress	netaddr ;
	int			sock ;
} ;

#define SERVER_UNABLE_IGNORE_COUNT	100

/* ת������ṹ */
struct ForwardRule
{
	char				rule_id[ RULE_ID_MAXLEN + 1 ] ;
	char				rule_mode[ RULE_MODE_MAXLEN + 1 ] ;
	
	struct ClientNetAddress		client_addr[ RULE_CLIENT_MAXCOUNT ] ;
	unsigned long			client_count ;
	struct ForwardNetAddress	forward_addr[ RULE_FORWARD_MAXCOUNT ] ;
	unsigned long			forward_count ;
	struct ServerNetAddress		server_addr[ RULE_SERVER_MAXCOUNT ] ;
	unsigned long			server_count ;
	
	unsigned long			select_index ;
	unsigned long			connection_count[ RULE_SERVER_MAXCOUNT ] ;
	
	union
	{
		struct
		{
			unsigned long	server_unable ;
		} RR[ RULE_SERVER_MAXCOUNT ] ;
		struct
		{
			unsigned long	server_unable ;
		} LC[ RULE_SERVER_MAXCOUNT ] ;
		struct
		{
			unsigned long	server_unable ;
			struct timeval	tv1 ;
			struct timeval	tv2 ;
			struct timeval	dtv ;
		} RT[ RULE_SERVER_MAXCOUNT ] ;
	} status ;
} ;

#define FORWARD_SESSION_TYPE_UNUSED	0	/* ת���Ựδ�õ�Ԫ */
#define FORWARD_SESSION_TYPE_MANAGE	1	/* �������ӻỰ */
#define FORWARD_SESSION_TYPE_LISTEN	2	/* ��������Ự */
#define FORWARD_SESSION_TYPE_CLIENT	3	/* �ͻ������ӻỰ */
#define FORWARD_SESSION_TYPE_SERVER	4	/* ��������� */

#define CONNECT_STATUS_CONNECTING	0	/* �첽���ӷ������ */
#define CONNECT_STATUS_CONNECTED	1	/* �������ѽ������� */

#define MANAGE_INPUT_BUFSIZE		1024	/* �����������뻺���� */
#define MANAGE_OUTPUT_BUFSIZE		1024	/* ����������������� */

#define TRY_CONNECT_MAXCOUNT		5	/* �첽�������ӷ���������� */

/* �����Ự�ṹ */
struct ListenNetAddress
{
	struct NetAddress	netaddr ;
	int			sock ;
	
	char			rule_mode[ 2 + 1 ] ;
} ;

/* ת���Ự�ṹ */
struct ForwardSession
{
	char				forward_session_type ;
	
	struct ListenNetAddress		listen_addr ;
	char				manage_input_buffer[ MANAGE_OUTPUT_BUFSIZE + 1 ] ;
	unsigned long			manage_input_buflen ;
	
	struct ClientNetAddress		client_addr ;
	unsigned long			client_index ;
	struct ServerNetAddress		server_addr ;
	unsigned long			server_index ;
	struct ForwardRule		*p_forward_rule ;
	char				connect_status ;
	unsigned long			try_connect_count ;
} ;

/* �����в����ṹ */
struct CommandParam
{
	char				*config_pathfilename ; /* -f */
	
	unsigned long			forward_rule_maxcount ; /* -r */
	unsigned long			forward_session_maxcount ; /* -c */
	unsigned long			transfer_bufsize ; /* -b */
	
	char				debug_flag ; /* -d */
} ;

/* �ڲ�����ṹ */
struct ServerCache
{
	struct timeval			tv ;
} ;

/* ���������� */
struct ServerEnv
{
	struct CommandParam		cmd_para ;
	
	struct ForwardRule		*forward_rule ;
	unsigned long			forward_rule_count ;
	
	int				event_env ;
	struct ForwardSession		*forward_session ;
	unsigned long			forward_session_maxcount ;
	unsigned long			forward_session_count ;
	unsigned long			forward_session_use_offsetpos ;
	
	struct ServerCache		server_cache ;
} ;

#define WAIT_EVENTS_COUNT		1024	/* �ȴ��¼��������� */

int G5( struct ServerEnv *pse );

#endif
