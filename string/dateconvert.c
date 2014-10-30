#include <datejul.h>

/************************************************************************/
/*function:DateTimeConvert ����ʱ����ֵ���ַ���ָ����ʽת��				*/
/*description:fmt(YYYYMMDDHHmmSS��ʽ)									*/
/*���ָ���ַ���'date'ʱ�����ַ���ת���ɾ���(1970.01.01 00:00:00 GMT	*/
/*����ɱ���ʱ��)��ʱ������������ָ��ʱ�������,��date����Ϊ��,������	*/
/*��(1970.01.01 00:00:00 GMT����ɱ���ʱ��)��ʱ�������ת�����ַ���		*/
/*��Ч��ʱ�䷶Χ��1970.1.1 00:00:00 -- 2038.1.19(GREEMWITHʱ��)			*/
/************************************************************************/
void DateTimeConvert(char *date,time_t *secs,char *fmt)
{
	int i;
	int len;
	time_t tt;
	struct tm tim;
	char tmp[5];

	tzset();
	memset(&tim, 0, sizeof(tim));	
	if ('\0' == date[0]) 
	{
		/* ��ֵת�� */
		tt = (0 == *secs) ? time(NULL) : *secs;
		tim = *localtime(&tt);
		len = strlen(fmt);
		sprintf(tmp, "%04d", tim.tm_year + 1900);
		for (i = 0; i < len; i++) 
		{
			if (!strncmp("YYYY", fmt + i, 4)) 
			{
				strncpy(date + i, tmp, 4);
				i = i + 3;
			} 
			else if (!strncmp(fmt + i, "YY", 2)) 
			{
				strncpy(date + i, tmp + 2, 2);
				i++;
			} 
			else if (!strncmp(fmt + i, "MM", 2)) 
			{
				sprintf(date + i, "%02d", tim.tm_mon + 1 );
				i++;
			} 
			else if (!strncmp(fmt + i, "DD", 2)) 
			{
				sprintf(date + i, "%02d", tim.tm_mday);
				i++;
			} 
			else if (!strncmp(fmt + i, "HH", 2)) 
			{
				sprintf(date + i, "%02d", tim.tm_hour);
				i++;
			} 
			else if (!strncmp(fmt + i, "mm", 2)) 
			{
				sprintf(date + i, "%02d", tim.tm_min);
				i++;
			} 
			else if (!strncmp(fmt + i, "SS", 2)) 
			{
				sprintf(date + i, "%02d", tim.tm_sec);
				i++;
			} 
			else 
			{
				date[i] = fmt[i];
			}
		}			/* end of for */
		date = '\0';
		if (secs != NULL) 
		{
			*secs = tt;
		}
	} 
	else 
	{
		/*��ת��ֵ*/
		len = strlen(fmt);
		for (i = 0; i < len; i++) 
		{
			memset(tmp, 0x00, sizeof(tmp));
			strncpy(tmp, date + i, 2);
			if (!strncmp("YYYY", fmt + i, 4)) 
			{
				strncpy(tmp, date + i, 4);
				tim.tm_year = atoi(tmp) - 1900;
				i = i + 3;
			} 
			else if (!strncmp(fmt + i, "YY", 2)) 
			{
				tim.tm_year = atoi(tmp);
				i++;
			} 
			else if (!strncmp(fmt + i, "MM", 2)) 
			{
				tim.tm_mon = atoi(tmp) - 1;
				i++;
			} 
			else if (!strncmp(fmt + i, "DD", 2)) 
			{
				tim.tm_mday = (0 == tim.tm_mday) ? 1 : atoi(tmp);
				tim.tm_mday = atoi(tmp);
				i++;
			} 
			else if (!strncmp(fmt + i, "HH", 2)) 
			{
				tim.tm_hour = atoi(tmp);
				i++;
			} 
			else if (!strncmp(fmt + i, "mm", 2)) 
			{
				tim.tm_min = atoi(tmp);
				i++;
			} 
			else if (!strncmp(fmt + i, "SS", 2)) 
			{
				tim.tm_sec = atoi(tmp);
				i++;
			} 
		}		/* end of for */		
		tt = mktime(&tim);
		*secs = tt;
	}
	return; 
}

/************************************************************************/
/*function:DateFormatCovert ����ʱ���ʽת��							*/
/*description:fmt(YYYYMMDDHHmmSS��YYYY-MM-DD HH:mm:SS�ȸ�ʽ)			*/
/*��'src'ת����fmt��ʽ��ŵ�'dest'��									*/
/************************************************************************/
int DateFormatCovert(char *dest,char *src,char *fmt)
{
	char year[5] = "",month[3] = "",day[3] = "";
	char hour[3] = "",minute[3] = "",second[3] = "";

	if ((strlen(src) < 8))
	{
		printf("the source string error!\n");
		return -1;
	}
	if ((strcmp(fmt,"YYYY-MM-DD")) == 0)				/* ��YYYYMMDDת����YYYY-MM-DD */
	{
		sscanf(src,"%4s%2s%2s",year,month,day);
		sprintf(dest,"%4s-%2s-%2s",year,month,day);
	}
	else if ((strcmp(fmt,"YYYYMMDD")) == 0)				/* ��YYYY-MM-DDת����YYYYMMDD */
	{
		sscanf(src,"%4s-%2s-%2s",year,month,day);
		sprintf(dest,"%4s%2s%2s",year,month,day);
	}
	else if ((strcmp(fmt,"YYYY-MM-DD HH:mm")) == 0)		/* ��YYYYMMDDHHmmת����YYYY-MM-DD HH:mm */
	{
		sscanf(src,"%4s%2s%2s%2s%2s",year,month,day,hour,minute);
		sprintf(dest,"%4s-%2s-%2s %2s:%2s",year,month,day,hour,minute);
	}
	if ((strcmp(fmt,"YYYYMMDDHHmm")) == 0)				/* ��YYYY-MM-DD HH:mmת����YYYYMMDDHHmm */
	{
		sscanf(src,"%4s-%2s-%2s %2s:%2s",year,month,day,hour,minute);
		sprintf(dest,"%2s%2s%2s%2s",month,day,hour,minute);
	} 
	else if ((strcmp(fmt,"YYYY-MM-DD HH:mm:SS")) == 0)	/* ��YYYYMMDDHHmmSSת����YYYY-MM-DD HH:mm:SS */
	{
		sscanf(src,"%4s%2s%2s%2s%2s%2s",year,month,day,hour,minute,second);
		sprintf(dest,"%4s-%2s-%2s %2s:%2s:%2s",year,month,day,hour,minute,second);
	}
	else if ((strcmp(fmt,"YYYYMMDDHHmmSS")) == 0)		/* ��YYYY-MM-DD HH:mm:SSת����YYYYMMDDHHmmSS */
	{
		sscanf(src,"%4s-%2s-%2s %2s:%2s:%2s",year,month,day,hour,minute,second);
		sprintf(dest,"%4s%2s%2s%2s%2s%2s",year,month,day,hour,minute,second);
	}
	else
	{
		printf("DateFormatCovert don't support format!\n");
		return -1;
	}
	return 0;
}
