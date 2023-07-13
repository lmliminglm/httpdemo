/**
 * @file    abup_log_zlog.h
 * @author  liming@abup.com
 * @brief   log模块对内接口声明
 * @note    
 * @version 0.1: 文件创建 
 * @date    2022-8-27
 *  
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */

#ifndef __ABUP_LOG_ZLOG_H__
#define __ABUP_LOG_ZLOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include "libfile.h"
#include "abup_log_core.h"

#define DATA_SIZE   10  
#define DATA_SIZE2  20

#define DEFAULT_LOG_CONF_FILE           "abup_log.conf"      /**log日志配置文件名称*/

/**需要写入配置文件的字符串的拼接宏*/
#define LOG_LEVEL_TRUN(a)       (logLevel[a-1].levelString)
#define DEFAULT_CONF_DATA10                  "" \
"[global]\n" \
"\tstrict init = false\n" \
"[formats]\n" \
    "\tsimple = \"%c <\%d(\%m-\%d \%T).\%ms> \%V [\%f:\%L \%U]: \%m\%n\"\n" \
"[rules]\n" 

/**
 * @struct ABUP_LOG_LEVEL
 * @brief  日志等级 
 * @note   定义log日志等级的枚举。
 */
typedef struct LOG_LEVEL_INFO
{
    int levelInt;
    char levelString[DATA_SIZE];
} LOG_LEVEL_INFO_T;

/**
 * @brief        vdi回调接口
 * @note         该接口主要供vdi调用，收集vdi的日志
 * @param  [in]  DCHAR *otime  时间
 * @param  [in]  D32S iModID  
 * @param  [in]  const D32S inLogLevel  日志等级
 * @param  [in]  const DCHAR* inFuncName  函数名称
 * @param  [in]  D32S inLineNumber  行号
 * @param  [in]  const DCHAR *format  格式
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
extern void abup_log_zlogCallback(DCHAR *otime, 
                                    D32S iModID,    
                                    const D32S inLogLevel,
                                    const DCHAR* inFuncName, 
                                    D32S inLineNumber,
                                    const DCHAR *format,...);

/**
 * @brief        log打印接口
 * @note         调用该接口，即可将日志存储到日志文件
 * @param  [in]  DCHAR * target 日志类别
 * @param  [in]  const DCHAR* fileName 文件名称
 * @param  [in]  const D32S fileName_length 文件长度
 * @param  [in]  const DCHAR* funcName 函数名称
 * @param  [in]  const D32S funcName_length 函数名称长度
 * @param  [in]  const D32S inLineNumber 行号
 * @param  [in]  const D32S level 等级
 * @param  [in]  const DCHAR *format 格式
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
extern void abup_log_msgOutput(DCHAR * target, 
                                const DCHAR* fileName, 
                                const D32S fileName_length, 
                                const DCHAR* funcName, 
                                const D32S funcName_length, 
                                const D32S inLineNumber, 
                                const D32S level , 
                                const DCHAR *format,...);

/**
 * @brief        log打印接口
 * @note         调用该接口，即可将日志存储到日志文件
 * @param  [in]  DCHAR * target 日志类别
 * @param  [in]  const DCHAR* fileName 文件名称
 * @param  [in]  const D32S fileName_length 文件长度
 * @param  [in]  const DCHAR* funcName 函数名称
 * @param  [in]  const D32S funcName_length 函数名称长度
 * @param  [in]  const D32S inLineNumber 行号
 * @param  [in]  const D32S level 等级
 * @param  [in]  const DCHAR *format 格式
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
extern void abup_log_businessMsgOutput(DCHAR * target, 
                                const DCHAR* fileName, 
                                const D32S fileName_length, 
                                const DCHAR* funcName, 
                                const D32S funcName_length, 
                                const D32S inLineNumber, 
                                const D32S level , 
                                const DCHAR *format,...);

/**
 * @brief        log模块反初始化
 * @note         
 * @param  [in]  void
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-27
 * @author       liming
 */
extern void abup_log_zlogfini(void);

/**
 * @brief        zlog初始化
 * @note         调用该接口 创建zlog初始化需要的文件目录
 * @param  [in]  DCHAR *logDir 整个日志模块目录
 * @param  [in]  DCHAR *logsDir 存放具体日志的目录
 * @param  [in]  ABUP_LOG_LEVEL eLogLevel 日志级别
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-29
 * @author       liming
 */
extern D32S abup_zlog_init(DCHAR *logDir, DCHAR *logsDir, ABUP_LOG_LEVEL eLogLevel);

#ifdef __cplusplus
}
#endif

#endif // __ABUP_LOG_ZLOG_H__