
/*[
 *      Name:                   adm_memory.c
 *
 *      Project:                ADM
]*/

/*! \internal
 * \file
 *          Provides a reference implementation of a memory handling API
 *
 * \brief   Memory handling APIs
 */

#include <string.h>
#include <stdlib.h>
#include <adm_typedefs.h>
#include <adm_memory.h>

#ifdef ADB_DBG_MEM

#define ADM_DBG_MEM_MAXNUM 1024
#define ADM_DBG_ADDR_MASK (0xEEEEEEEE)
#define ADM_DBG_MAX_LIMITED_SIZE ( 16*1024*1024)
#define ADM_DBG_MEM_FILENAME_LEN 32

typedef struct
{
   DSIZE addr;
   D32U len;
   DCHAR *funcname;
   D32U lineno;
   DSIZE totalSize;
   D32U totalCount;
} ADM_MEMDBG_RECORD;

typedef struct
{
   DSIZE totalSize;
   D32U totalCount;
} ADM_MEMDBG_REAL_TOTAL;

typedef struct
{
   DSIZE peakSingleSize;
   D32U peakCount;
   DSIZE peakTotalSize;
} ADM_MEMDBG_PEAK_TOTAL;

static ADM_MEMDBG_RECORD adm_memdbgList[ADM_DBG_MEM_MAXNUM]={0};
static ADM_MEMDBG_LOG_CB adm_memdbgLogFuncCb=NULL;
static ADM_MEMDBG_REAL_TOTAL adm_memdbgRealTotal={0};
static ADM_MEMDBG_PEAK_TOTAL adm_memdbgPeak={0};

#endif

void *adm_memzero(void *buf, DSIZE size)
{
    if (!buf)
        return NULL;

    return memset(buf, 0x00, size);    
}

void *adm_memset(void *dest, D32S val, DSIZE count, DSIZE dstLen)
{
    if (!dest)
        return NULL;

    if (dstLen < count)
        return NULL;
	
    return memset(dest, val, count);
}

void *adm_memcpy(void *dest, void *src, DSIZE srcLen, DSIZE dstLen)
{
    if (!dest || !src)
        return NULL;

    if (dstLen < srcLen)
        return NULL;


    return memcpy(dest, src, srcLen);
}

D32S adm_memcmp(const void *inBuf1, const void *inBuf2, D32U count)
{
    if (!inBuf1)
        return -1;
	
    if (!inBuf2)
        return 1;
	
    return memcmp(inBuf1, inBuf2, count);
}

void *adm_memmove(void *dest, const void *src, DSIZE srcLen, DSIZE dstLen)
{
    if (!dest || !src || (srcLen==0) || (dstLen==0))
        return NULL;
	
    if (dstLen < srcLen)
        return NULL;
	
    return memmove(dest, src, srcLen);
}

/*void *adm_malloc(D32U size)
{
    return (malloc(size));
}*/

void *__adm_malloc(D32U size, const DCHAR *funcname, D32U line)
{
	void *buf = malloc(size);

   if (buf == NULL)
   {
#if 0//defined(ADM_ENABLE_PAL_TRACE)
      DSIZE left=0;

      left = 0;
      trace("malloc fail!!, len: %d, left: %d", size, left);
#endif
   }

#ifdef ADM_DBG_MEM
   else
   {
   	  D32U i=0;
      if (size > adm_memdbgPeak.peakSingleSize)
      {
         adm_memdbgPeak.peakSingleSize = size;
      }
	  
      //adm_dbg_mem_index &= 1023;
      for(i=0;i<ADM_DBG_MEM_MAXNUM;i++)
      {
  		  if(adm_memdbgList[i].addr==0)
		  {
			  adm_memdbgList[i].len = (D32U)size;
			  adm_memdbgList[i].addr = (DSIZE)buf;
			  adm_memdbgList[i].funcname = (DCHAR *)funcname;
			  adm_memdbgList[i].lineno = (D32U)line;

			  adm_memdbgRealTotal.totalSize += (D32U)size;
			  adm_memdbgRealTotal.totalCount++;

			  if (adm_memdbgRealTotal.totalCount > adm_memdbgPeak.peakCount)
			  {
				 adm_memdbgPeak.peakCount = adm_memdbgRealTotal.totalCount;
			  }
			  if (adm_memdbgRealTotal.totalSize > adm_memdbgPeak.peakTotalSize)
			  {
				 adm_memdbgPeak.peakTotalSize = adm_memdbgRealTotal.totalSize;
			  }
			  
#if 0

			  if(adm_memdbgLogFuncCb)
				  adm_memdbgLogFuncCb("malloc[%d]: addr=0x%x, size=%d, pos:%d, func:%s, lineno:%d, totalsize:%ld, totalcount:%d", \
					  i, adm_memdbgList[i].addr, adm_memdbgList[i].len, i,\
					  adm_memdbgList[i].funcname, adm_memdbgList[i].len, \
					  adm_memdbgRealTotal.totalSize, adm_memdbgRealTotal.totalCount);
#endif
			  break;
		  }
      }
  
	  if (i == ADM_DBG_MEM_MAXNUM)
	  {
		//ASSERT(0);
	  }

      if (adm_memdbgPeak.peakSingleSize > ADM_DBG_MAX_LIMITED_SIZE)
      {
         //ASSERT(0);
		  if(adm_memdbgLogFuncCb)
			  adm_memdbgLogFuncCb("malloc size[%ld] overflow max single liminted size[%ld] !!!", \
			  adm_memdbgPeak.peakSingleSize, ADM_DBG_MAX_LIMITED_SIZE);
      }
	  
      if (adm_memdbgRealTotal.totalCount > ADM_DBG_MEM_MAXNUM)
      {
		  //ASSERT(0);
		   if(adm_memdbgLogFuncCb)
			   adm_memdbgLogFuncCb("malloc count[%d] overflow max liminted count[%d] !!!", \
			   adm_memdbgPeak.peakSingleSize, ADM_DBG_MEM_MAXNUM);
      }
	  
   }
#endif
	if (NULL != buf) {
		adm_memset(buf, 0x00, size, size);
	}

	return buf;
}


void __adm_free(void *pp, const DCHAR *funcname, D32U line)
{
    if((pp==NULL))
    {
        return;
    }

#ifdef ADM_DBG_MEM
	{
		D32U i = 0;

		for (i = 0; i < ADM_DBG_MEM_MAXNUM; i++)
		{
		  if (adm_memdbgList[i].addr == (DSIZE)pp)
		  {			  
			  adm_memdbgRealTotal.totalSize -= adm_memdbgList[i].len;
			  adm_memdbgRealTotal.totalCount--;
			  
			  if(adm_memdbgRealTotal.totalSize <0)
			  	adm_memdbgRealTotal.totalSize = 0;
			  if(adm_memdbgRealTotal.totalCount < 0)
			  	adm_memdbgRealTotal.totalCount=0;
#if 0
			  if(adm_memdbgLogFuncCb)
				  adm_memdbgLogFuncCb("free[%d]: addr=0x%x, size=%d, pos:%d, func:%s, lineno:%d, totalsize:%ld, totalcount:%d", \
					  i, adm_memdbgList[i].addr, adm_memdbgList[i].len, i,\
					  adm_memdbgList[i].funcname, adm_memdbgList[i].len, \
					  adm_memdbgRealTotal.totalSize, adm_memdbgRealTotal.totalCount);
#endif			  
			 adm_memdbgList[i].addr = 0;
			 break;
		  }
		}
		if (i == ADM_DBG_MEM_MAXNUM)
		{
		  //ASSERT(0);
		}
	}
#endif
	
    free(pp);
}

void adm_memlog_set(ADM_MEMDBG_LOG_CB func)
{
#ifdef ADM_DBG_MEM
	adm_memdbgLogFuncCb = func;
#endif
}

DBOOL adm_memdbg_view(void)
{
	D32U i=0, count=0;
#ifdef ADM_DBG_MEM

	if(adm_memdbgLogFuncCb)
	{
		adm_memdbgLogFuncCb("--------------------memview total infomation begin--------------------");
		adm_memdbgLogFuncCb("Allocated peakTotalSize:%d, peakCount:%d, peakSingleSize:%ld.", \
			adm_memdbgPeak.peakTotalSize, adm_memdbgPeak.peakCount, adm_memdbgPeak.peakSingleSize);		
		adm_memdbgLogFuncCb("Not Free totalSize:%ld, totalCount:%d. As list:", \
			adm_memdbgRealTotal.totalSize, adm_memdbgRealTotal.totalCount);		
		adm_memdbgLogFuncCb("--------------------memview total infomation end----------------------");
	}

	for(i=0;i<ADM_DBG_MEM_MAXNUM;i++)
	{
		if(adm_memdbgList[i].addr != 0)
		{
			count++;
			if(adm_memdbgLogFuncCb)
				adm_memdbgLogFuncCb("memview[%d]: addr=0x%x, size=%d, pos:%d, func:%s, lineno:%d", \
					count, adm_memdbgList[i].addr, adm_memdbgList[i].len, i,\
					adm_memdbgList[i].funcname, adm_memdbgList[i].len);
		}
	}
#endif
	if(count>0)
		return TRUE;
	else
		return FALSE;
}

void *adm_memdup(void *buf, D32U len)
{
    void *new=NULL;

    if (!buf || len<=0)
    {
        return NULL;
    }

    new = (void *)adm_malloc(len);
    if (new == NULL)
        return NULL;

    adm_memcpy(new, buf, len, len);

    return new;
}

void *adm_memchr(const void *buf, int ch, DSIZE count)
{
	if(!buf || count==0) return NULL;

	return memchr(buf, ch, count);
}

