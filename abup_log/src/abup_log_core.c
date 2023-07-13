/**
 * @file    abup_log_core.c
 * @author  liming@abup.com
 * @brief   log模块接口定义
 * @note    主要依赖zlog进行接口封装，封装成适合我们的log模块
 * @version 0.1: 文件创建 
 * @date    2022-08-18
 *  
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "libfile.h"
#include "zlog.h"
#include "abup_log_zlog.h"
#include "abup_config.h"

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
D32S abup_log_callback(DCHAR *otime,
                        D32S iModID,
                        const D32S inLogLevel,
                        const DCHAR* inFuncName,
                        D32S inLineNumber,
                        const DCHAR *format,...)
{
	DCHAR szbuf[ABUP_DATA_SIZE] = {0};
	va_list pArgs ;
	
    /**可变参数序列化*/
	va_start(pArgs, format);
	vsnprintf(szbuf, sizeof(szbuf), format, pArgs);
	va_end(pArgs); 
    abup_log_zlogCallback(otime, iModID, inLogLevel, inFuncName, inLineNumber,"%s", szbuf);
	return 0;
}

/**
 * @brief        log模块初始化
 * @note         初始化主要进行，log模块目录的创建，文件的创建，配置的初始化
 * @param  [in]  const char* workDir
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
D32S abup_log_Init(const DCHAR* workDir)
{
    D32S errCode = -1;
    DCHAR logDir[ABUP_FILEPATH_SIZE] = {0};
    DCHAR logsDir[ABUP_FILEPATH_SIZE] = {0};

    do
    {
        if (!workDir) 
        {
            fprintf(stderr, "workDir is NULL error\n");
            errCode = -1;
            break;
        }
        
        /** 如果工作目录不存在，那么创建工作目录 */
        if(0 != rvdc_dir_create(workDir))
        {
            fprintf(stderr, "rvdc_dir_create error\n");
            errCode = -1;
            break;
        }

        /** 创建日志目录 */
        adm_snprintf(logDir, ABUP_FILEPATH_SIZE, DEFAULT_LOG_DIR,  workDir);
        fprintf(stderr, "log file directory %s\n",logDir);
        if(0 != rvdc_dir_create(logDir))
        {
            fprintf(stderr, "rvdc_dir_create error\n");
            errCode = -1;
            break;
        }

        /** 创建具体日志目录 */
        adm_snprintf(logsDir, ABUP_FILEPATH_SIZE, DEFAULT_LOGS_DIR, logDir);
        fprintf(stderr, "Specific log file directory %s\n",logsDir);
        if(0 != rvdc_dir_create(logsDir))
        {
            fprintf(stderr, "rvdc_dir_create error\n");
            errCode = -1;
            break;
        }

        /**zlog初始化*/
        printf("log level: %d\n", LOG_LEVEL);
        errCode = abup_zlog_init(logDir,logsDir, LOG_LEVEL);   
    } while (0);
    
    return errCode;
}

/**
 * @brief        log模块反初始化
 * @note         
 * @param  [in]  void
 * @return       void
 * @date         2022-08-18
 * @author       liming
 */
void abup_log_fini(void)
{
    abup_log_zlogfini();
}
