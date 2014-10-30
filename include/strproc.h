/*****************************************************************************
          DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                   Version 6, January 2014

Copyright (C) 2014 Lihua-Yu <ylh2@sina.com>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

 0. You just DO WHAT THE FUCK YOU WANT TO.
********************************************************************************/
// For SDBC 6.0
#ifndef STRPROC_H
#define STRPROC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <inttypes.h>  //C99

#if defined _LP64 || defined __LP64__ || defined __64BIT__ || _ADDR64 || defined _WIN64 || defined __arch64__ || __WORDSIZE == 64 || (defined __sparc && defined __sparcv9) || defined __x86_64 || defined __amd64 || defined __x86_64__ || defined _M_X64 || defined _M_IA64 || defined __ia64 || defined __IA64__
      #define SDBC_PTR_64
#endif 
#define SYSERR -1
#define LENGERR -2
#define POINTERR -3
#define MEMERR -5
#define NOTLOGIN -6
#define FORMATERR -7
#define CRCERR -8
#define TIMEOUTERR -11

#if defined WIN32 || defined _WIN64
#include <winsock2.h>
typedef __int64 INT64;
typedef int pthread_t;
typedef int pthread_mutex_t;
#define timezone _timezone
typedef unsigned int u_int;
#else
#       include <pthread.h>
        typedef int64_t INT64;
#endif

#define CH_INT4 CH_INT
typedef int32_t INT4;

extern char *(*encryptproc)(char *mstr);
/***************************************************************************
 * һЩ���������GBK�ַ�����ߵ�Ӣ����ĸ�����У�ʹ��GBK�ַ���ʱ����ñ�־Ϊ1
 * ��ȱʡֵΪ0
 ***************************************************************************/
extern char GBK_flag;

#ifdef __cplusplus
extern "C" {
#endif 

/************************************************************************/
/*function:stptok �ֽ��ַ���Ϊһ���Ǵ�		                        */
/*description:���ݷָ���'tok'����'src'�ֽ⵽'trg'�У����'tok'�����ã���*/
/*����'src'��len����Сֵ����'src'������'trg'�С����Խ�������г��ֵ���  */
/*'tok'��ͬ�ķָ������ֽ⡣����ָ�����'tok'�ַ���ָ��					*/
/*���⣺																*/
/*	char *p=stptok(from,0,0,char *tok); ���������Ŀ�괮�ͳ���Ϊ0ʱ,	*/
/*	��ֻ���س���'tok'�ַ���ָ��											*/
/************************************************************************/
/*function:iscc �ж��Ƿ��Ǻ���*/
int iscc(unsigned char ch);
/*function:firstcc ���ֵĵ�һ���ֽ�*/
int firstcc(unsigned char *bp,unsigned char *bufp);
/*function:secondcc ���ֵĵڶ����ֽ�*/
int secondcc(unsigned char *bp,unsigned char *bufp);
char *stptok(const char *src,char *trg,int len,const char *tok);
/**********************************************************************
 * stctok:����strtok,�ṩ�̰߳�ȫ�ģ�֧�ֺ���token���ַ����ֽ⺯����
 * ǰ����������strtok_r��ͬ,int *by:ָʾ������ʾ�ַ�������tok���Ǹ��ַ��ضϵ�
 * -1��ʾ���Ǳ�tok�ضϵġ�
 *********************************************************************/
extern char *stctok(const char *str,char *ctok,char **savep,int *by);
extern char *cstrchr(const char *from,char *tok);

/************************************************************************/
/*function:strsubst �滻�ַ�������				                        */
/*description:��'str'�滻'from'��ǰcnt���ַ�							*/
/*����ָ���滻�����һ���ַ���ָ��										*/
/************************************************************************/
char *strsubst(char *from,int cnt,char *str);

/************************************************************************/
/*function:strsub ��ȡ�ַ�������                                        */
/*description:'src'�н�ȡ��start��ʼ��cnt���ַ���'dest'��(start���Ϊ0)	*/
/************************************************************************/
void strsub(char *dest, const char *src,int start,int cnt);

/************************************************************************/
/*function:strins �����ַ�����                                          */
/*description:��'str'ǰ�����ַ�'ch',������һ���ַ���ָ��				*/
/************************************************************************/
char *strins(char *str,char ch);

/************************************************************************/
/*function:strdel ɾ���ַ�����		                                    */
/*description:ɾ��'str'ͷһ���ַ�										*/
/************************************************************************/
#define strdel(str) strsubst((str),1,(char *)0)

/************************************************************************/
/*function:trim ȥ���ַ���ǰ��ո�	                                    */
/************************************************************************/
char *trim(char *src);

/************************************************************************/
/*function:rtrim ȥ���ַ�����Ŀո�	 	                                */
/************************************************************************/
char *rtrim(char *str);

/************************************************************************/
/*function:ltrim ȥ���ַ���ǰ�Ŀո�		                                */
/************************************************************************/
char *ltrim(char* str);

/************************************************************************/
/*function:strupper ���ַ����е�Сд��ĸת���ɴ�д                      */
/************************************************************************/
char *strupper(char *str);

/************************************************************************/
/*function:strlower ���ַ����еĴ�д��ĸת����Сд                      */
/************************************************************************/
char *strlower(char *str);
/**************************************************
 * TRIM: delete suffix space of str
 *****************************************************/
char * TRIM(char *str);
char *trim_all_space(char *str);
char *sc_basename(char *path);
/******************************************************
 * skipblk: skip prefix space of str
 *****************************************/
char *skipblk(char *str);
/* copy *src to dst break by c return tail of dst,*src to tail  */
char *strtcpy(char *dst,char **src,char c);
#ifndef  __USE_XOPEN2K8
char *stpcpy(char *dest,const char *src);
#endif

char *strrevers(char *str);


/***********************************************
 * ShowLog: write debug or log message to stderr
 * logfile name  defined by enveroment varibale LOGFILE
 * LOGLEVEL defined by enveroment varibale LOGLEVEL
 * if DEBUG_level <= LOGLEVEL then write message to stderr
 * example:
 * LOGLEVEL=3
 * LOGFILE=/tmp/rec
 * today is 2001-02-08 14:29'03
 * then stderr=/tmp/rec08.log
 * if DEBUG_level <=3 ,then write the message to /tmp/rec08.log as:
 * DEBUG_level Showid yyyy/mm/dd 14:29'03 message
 *
 ***********************************************/
extern char *Showid;
extern char LOGFILE[];
int ShowLog(int DEBUG_level,const char *fmt,...);
/*************************************************************
 *  ���߳���־,���ÿ���̴߳���һ���ͻ���� 
 * ����֤�ͻ������ɺ���� mthr_showid_add,��Showid����ϵͳ 
 * ���߳�����ǰ������ mthr_showid_del�����Showid 
 * mthr_showid_del() ����ɾ���Ĳ�ţ�0=ûɾ
 ************************************************************/
void mthr_showid_add(pthread_t tid,char *showid);
int mthr_showid_del(pthread_t tid);

//�������ļ������������� 
int envcfg(char *configfile);
int strcfg(char *str);

int isrfile(char *path);
int isdir(char *path);
/* ���ͱ��ַ���������β����*/
char * itoStr(register int n, char *s);
char * lltoStr(register INT64 n, char *s);
/********************************************
 *  fround.c 
 * flg:0-��ѧ����,1-��,2-����,3-ֻ��,4-ֻ��,5-��������,
 ********************************************/
double f_round(double x,int flg,int dig_num);
#define sc_round(x) f_round((x),5,0)
// used by open_auth() and ldap_auth()
int open_auth(char * authfile,char *name, char *DNS,char *UID,char *Pwd,char *DBOWN);
extern char *decodeprepare(char *dblabel);

#ifdef __cplusplus
}
#endif 
#define SQL_AUTH open_auth

#endif
