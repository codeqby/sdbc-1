#include <Binary_search.h>

int Binary_EQUAL(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n))
{
int l,m,ret;
	if(!key||!data) return -1;
	l=0;
	data_count--;
	while(data_count>=l) {
		m=l+((data_count-l)>>1);
		if(!(ret=compare(key,data,m))) return m;
		if(ret>0)  data_count=m-1;
		else l=m+1;
	}
	return -1;
}

int Binary_GT(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n))
{
int l,m,ret;
int result=-1;

	if(!key||!data) return -1;
	l=0;
	data_count--;
	while(data_count>=l) {
		m=l+((data_count-l)>>1);
		ret=compare(key,data,m);
		if(ret>0)  {
			result=m;
			data_count=m-1;
		} else l=m+1;
	}
	return result;
}

int Binary_GTEQ(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n))
{
int l,m,ret;
int result=-1;

	if(!key||!data) return -1;
	l=0;
	data_count--;
	while(data_count>=l) {
		m=l+((data_count-l)>>1);
		ret=compare(key,data,m);
		if(!ret) {
			return m;
		} else if(ret>0)  {
			result=m;
			data_count=m-1;
		} else l=m+1;
	}
	return result;
}

int Binary_LT(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n))
{
int l,m,ret;
int result=-1;

	if(!key||!data) return -1;
	l=0;
	data_count--;
	while(data_count>=l) {
		m=l+((data_count-l)>>1);
		ret=compare(key,data,m);
		if(ret>=0) data_count=m-1;
		else {
			result=m;
			l=m+1;
		}
	}
	return result;
}

int Binary_LTEQ(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n))
{
int l,m,ret;
int result=-1;

	if(!key||!data) return -1;
	l=0;
	data_count--;
	while(data_count>=l) {
		m=l+((data_count-l)>>1);
		ret=compare(key,data,m);
		if(ret==0) return m;
		if(ret>=0) data_count=m-1;
		else {
			result=m;
			l=m+1;
		}
	}
	return result;
}

