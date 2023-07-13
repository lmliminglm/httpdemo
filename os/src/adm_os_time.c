#define LOG_TAG "ADM_OS"


#include <time.h>
#include <sys/time.h>
#ifndef ADM_OS_QNX
#include <sys/sysinfo.h>
#endif

#include "adm_typedefs.h"
#include "adm_memory.h"
#include "adm_string.h"
#include "adm_time.h"
// #include "adm_debug.h"


void adm_time_get_monotonic(struct timeval *tv)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
}

void adm_time_get_realtime(struct timeval *tv)
{
	struct timespec ts;

	clock_gettime(CLOCK_REALTIME, &ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
}

D32S adm_time_get(ADM_TIME_T *time)
{
	struct timeval tv;
	struct tm nowtime;

	if(!time)
		return -1;
	
	adm_time_get_realtime(&tv);
	localtime_r(&tv.tv_sec, &nowtime);

	time->year = nowtime.tm_year+1900;
	time->month = nowtime.tm_mon+1;
	time->day = nowtime.tm_mday;
	time->hour = nowtime.tm_hour;
	time->min = nowtime.tm_min;
	time->sec = nowtime.tm_sec;
	time->msec = tv.tv_usec/1000;

	return 0;
}


DCHAR *adm_time_get_time_string(void)
{
    ADM_TIME_T time;
	static DCHAR time_str[64]={0};

	adm_time_get(&time);
	
    adm_memset(time_str, 0, 64, 64);
	adm_snprintf(time_str, 64, "[%04d-%02d-%02d %02d:%02d:%02d.%03d]", \
	time.year, time.month, time.day, time.hour, time.min, time.sec,time.msec);

    return time_str;
}

D64U adups_time_get_timestamp(void)
{
    D64U ts=0;
	struct timeval tm;
	
	gettimeofday(&tm,NULL);
	ts = tm.tv_sec;
    
    return ts;
}

#define SECOND_OF_DAY   86400
D8U DayOfMon[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

void adm_time_from_timestamp(unsigned long sec, ADM_TIME_T *tTime)
{
  D16U i,j,day;
  D64U day_tmp;
  
  day_tmp = sec / SECOND_OF_DAY;
  sec = sec % SECOND_OF_DAY;
  
  i = 1970;
  while(day_tmp > 365)
  {
   if(((i%4==0)&&(i%100!=0)) || (i%400==0)) 
    day_tmp -= 366;
   else
    day_tmp -= 365;
   i++;
  }
  if((day_tmp == 365) && !(((i%4==0)&&(i%100!=0)) || (i%400==0)))
  {
   day_tmp -= 365;
   i++;
  }
  tTime->year = i;
  for(j=0;j<12;j++)
  {
   if((j==1) && (((i%4==0)&&(i%100!=0)) || (i%400==0)))
    day = 29;
   else
    day = DayOfMon[j];
   if(day_tmp >= day) day_tmp -= day;
   else break;
  }
  tTime->month = j+1;
  tTime->day = day_tmp+1;
  tTime->hour = ((sec / 3600)+8)%24;
  tTime->min = (sec % 3600) / 60;
  tTime->sec = (sec % 3600) % 60;
}


void adm_time_test_entry(void)
{
	// DCHAR *timeStr = adm_time_get_time_string();
	// D64U ts = adups_time_get_timestamp();
}

