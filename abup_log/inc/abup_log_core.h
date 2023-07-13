/**
 * @file    abup_log_core.h
 * @author  liming@abup.com
 * @brief   log模块对外接口声明
 * @note    
 * @version 0.1: 文件创建 
 * @date    2022-8-18
 *  
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */

#ifndef __ABUP_LOG_CORE_H__
#define __ABUP_LOG_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif	

#include "adm_typedefs.h"

#define DEFAULT_TARGET_DC               "dc"  
#define DEFAULT_TARGET_VDI              "vdi"
#define DEFAULT_TARGET_HOST      		"host" 
#define DEFAULT_FILE_VDI                "vdi.c"              /**vdi调用日志打印的文件名*/
#define DEFAULT_LOG_DIR                 "%s/log"             /**log日志整个模块目录*/
#define DEFAULT_LOGS_DIR                "%s/logs"            /**log具体日志目录*/

#define RDM_VERBOSE()                                               \
    do {                                                            \
        log_d("%s:%s:%d xxxxxx", __FILE__, __func__, __LINE__);   \
    } while (0)

/**
 * @enum ABUP_LOG_LEVEL
 * @brief  日志等级 
 * @note   定义log日志等级的枚举。
 */
typedef enum 
{
	LOG_LEVEL_DEBUG = 1,
	LOG_LEVEL_INFO = 2,
	LOG_LEVEL_NOTICE = 3,
	LOG_LEVEL_WARN = 4,
	LOG_LEVEL_ERROR = 5,
	LOG_LEVEL_FATAL = 6
} ABUP_LOG_LEVEL;


#define ABUP_FILEPATH_SIZE  256
#define ABUP_DATA_SIZE  10*1024
#define ABUP_FILE_SIZE  5

/** log各个等级打印所对印的接口及传参  */
#define log_f(...)  abup_log_msgOutput(DEFAULT_TARGET_DC, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_FATAL, __VA_ARGS__)
#define log_e(...)  abup_log_msgOutput(DEFAULT_TARGET_DC, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)
#define log_w(...)  abup_log_msgOutput(DEFAULT_TARGET_DC, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_WARN, __VA_ARGS__)
#define log_n(...)  abup_log_msgOutput(DEFAULT_TARGET_DC, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_NOTICE, __VA_ARGS__)
#define log_i(...)  abup_log_msgOutput(DEFAULT_TARGET_DC, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_INFO, __VA_ARGS__)
#define log_d(...)  abup_log_msgOutput(DEFAULT_TARGET_DC, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)

/** host log各个等级打印所对印的接口及传参  */
#define hlog_f(...)  abup_log_hostMsgOutput(DEFAULT_TARGET_HOST, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_FATAL, __VA_ARGS__)
#define hlog_e(...)  abup_log_hostMsgOutput(DEFAULT_TARGET_HOST, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_ERROR, __VA_ARGS__)
#define hlog_w(...)  abup_log_hostMsgOutput(DEFAULT_TARGET_HOST, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_WARN, __VA_ARGS__)
#define hlog_n(...)  abup_log_hostMsgOutput(DEFAULT_TARGET_HOST, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_NOTICE, __VA_ARGS__)
#define hlog_i(...)  abup_log_hostMsgOutput(DEFAULT_TARGET_HOST, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_INFO, __VA_ARGS__)
#define hlog_d(...)  abup_log_hostMsgOutput(DEFAULT_TARGET_HOST, __FILE__, sizeof(__FILE__)-1, __func__, sizeof(__func__)-1, __LINE__, LOG_LEVEL_DEBUG, __VA_ARGS__)


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
 * @return       void
 * @date         2022-08-18
 * @author       liming
 */
void abup_log_msgOutput(DCHAR * target, 
						const DCHAR* fileName, 
						const D32S fileName_length, 
						const DCHAR* funcName, 
						const D32S funcName_length, 
						const D32S inLineNumber, 
						const D32S level , 
						const DCHAR *format,...);

/**
 * @brief        host log打印接口
 * @note         调用该接口，即可将日志存储到日志文件
 * @param  [in]  DCHAR * target 日志类别
 * @param  [in]  const DCHAR* fileName 文件名称
 * @param  [in]  const D32S fileName_length 文件长度
 * @param  [in]  const DCHAR* funcName 函数名称
 * @param  [in]  const D32S funcName_length 函数名称长度
 * @param  [in]  const D32S inLineNumber 行号
 * @param  [in]  const D32S level 等级
 * @param  [in]  const DCHAR *format 格式
 * @return       void
 * @date         2022-08-18
 * @author       liming
 */
void abup_log_hostMsgOutput(DCHAR * target, 
						const DCHAR* fileName, 
						const D32S fileName_length, 
						const DCHAR* funcName, 
						const D32S funcName_length, 
						const D32S inLineNumber, 
						const D32S level , 
						const DCHAR *format,...);


/**
 * @brief        log模块初始化
 * @note         初始化主要进行，log模块目录的创建，文件的创建，配置的初始化
 * @param  [in]  const char* workDir
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
extern D32S abup_log_Init(const DCHAR* workDir);

/**
 * @brief        log模块反初始化
 * @note         
 * @param  [in]  void
 * @return       void
 * @date         2022-08-18
 * @author       liming
 */
extern void abup_log_fini(void);

/**
 * @brief        vdi回调接口
 * @note         该接口主要供vdi调用，收集vdi的日志
 * @param  [in]  const char* workDir
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
extern D32S abup_log_callback(DCHAR *otime,
								D32S iModID,
								D32S inLogLevel,
								const DCHAR* inFuncName, 
								D32S inLineNumber,
								const DCHAR *format,...);

#ifdef __cplusplus
}
#endif

#endif // __ABUP_LOG_CORE_H__
