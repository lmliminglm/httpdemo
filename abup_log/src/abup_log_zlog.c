/**
 * @file    abup_log_zlog.c
 * @author  liming@abup.com
 * @brief   log模块接口定义
 * @note    主要依赖zlog进行接口封装，封装成适合我们的log模块
 * @version 0.1: 文件创建 
 * @date    2022-08-18
 *  
 * @copyright Copyright © 2018 All rights 上海艾拉比智能科技有限公司
 */

#include <stdarg.h>
#include "zlog.h"
#include "abup_log_zlog.h"
#include "abup_config.h"

zlog_category_t *dc = NULL;
zlog_category_t *vdi = NULL;
zlog_category_t *host = NULL;

static LOG_LEVEL_INFO_T  logLevel[] = {
                                        {1, ".DEBUG"},
                                        {2, ".INFO"},
                                        {3, ".NOTICE"},
                                        {4, ".WARN"},
                                        {5, ".ERROR"},
                                        {6, ".FATAL"}
                                                     };

/**
 * @brief        zlog配置文件初始化
 * @note         写入log的默认配置
 * @param  [in]  const DCHAR* logCfg 配置文件名称,
 * @param  [in]  const DCHAR *logsDir log具体日志目录
 * @param  [in]  ABUP_LOG_LEVEL elogLevel 日志级别
 * @param  [in]  D32S categoryCount 分类个数
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
static D32S zlog_conf_init(const DCHAR* logCfg, const DCHAR *logsDir, ABUP_LOG_LEVEL elogLevel, D32S categoryCount, ...)
{
    FILE* fp = NULL;
    D32S errCode = 0;
    DCHAR logConfData2[ABUP_DATA_SIZE] = {0};
    DCHAR str_temp[ABUP_DATA_SIZE] = {0};
    DCHAR str_temp1[ABUP_DATA_SIZE] = {0};
    va_list ap;
    D32S i = 0;
    DCHAR *pcCategory = NULL;

    do
    {
        fp = fopen(logCfg, "w+");
        if(NULL == fp)
        {
            fprintf(stderr, "zlog config file init failure\n");
            errCode = -1;
            break;
        }
        
        /**组装配置文件中输出到控制台所需要的字符串*/
        if(1 != fwrite(DEFAULT_CONF_DATA10, strlen(DEFAULT_CONF_DATA10), 1, fp))
        {
            fprintf(stderr, "abup log write conf failure\n");
            errCode = -1;
            break;
        }

        va_start(ap, categoryCount);

        for (i = 0; i < categoryCount; i++)
        {
            pcCategory = va_arg(ap, DCHAR *);
            if (!IS_SHOW_HOST_LOG && 0 == strcmp(pcCategory, DEFAULT_TARGET_HOST))
            {
                fprintf(stdout, "skip host config\n");
                continue;
            }
            
            adm_snprintf(str_temp, ABUP_DATA_SIZE, "\t%s%s >stdout; simple\n", pcCategory, LOG_LEVEL_TRUN(elogLevel));
            if (1 != fwrite(str_temp, strlen(str_temp), 1, fp))
            {
                fprintf(stderr, "abup log write stdout conf failure\n");
                errCode = -1;
                break;
            }

            adm_snprintf(str_temp, ABUP_DATA_SIZE, "\t%s%s \"%s/%s.log\", 5MB*1; simple\n", pcCategory, LOG_LEVEL_TRUN(elogLevel), logsDir, pcCategory);
            if (1 != fwrite(str_temp, strlen(str_temp), 1, fp))
            {
                fprintf(stderr, "abup log write file conf failure\n");
                errCode = -1;
                break;
            }
        }
        
        va_end(ap);

        fclose(fp);
        fp = NULL;

        if (0 == errCode)
        {
            printf("zlog_conf_init success\n");
        }
        
    } while (0);
    
    return errCode;
}

/**
 * @brief        vdi回调接口
 * @note         该接口主要供vdi调用，收集vdi的日志
 * @param  [in]  DCHAR *otime 时间
 * @param  [in]  D32S iModID
 * @param  [in]  const D32S inLogLevel 等级
 * @param  [in]  const DCHAR* inFuncName  函数名称
 * @param  [in]  D32S inLineNumber 行号
 * @param  [in]  const DCHAR *format 格式
 * @return       D32S 0:success Non-0:fail
 * @date         2022-08-18
 * @author       liming
 */
void abup_log_zlogCallback(DCHAR *otime, 
                            D32S iModID,
                            const D32S inLogLevel,
                            const DCHAR* inFuncName,
                            D32S inLineNumber,
                            const DCHAR *format,...)
{
	DCHAR szbuf[ABUP_DATA_SIZE];
	va_list pArgs;
    D32S iConvert_LogLevel = 0;

    /** 日志等级的转化 */
    switch (inLogLevel)
    {
    case 0:
        iConvert_LogLevel = LOG_LEVEL_FATAL*DATA_SIZE2;
        break;
    case 1:
        iConvert_LogLevel = LOG_LEVEL_ERROR*DATA_SIZE2;
        break;    
    case 2:
        iConvert_LogLevel = LOG_LEVEL_WARN*DATA_SIZE2;
        break;
    case 3:
        iConvert_LogLevel = LOG_LEVEL_INFO*DATA_SIZE2;
        break;  
    default:
        iConvert_LogLevel = LOG_LEVEL_DEBUG*DATA_SIZE2;
        break;
    }
	
    /**可变参数序列化*/
	va_start(pArgs, format);
	vsnprintf(szbuf, sizeof(szbuf), format, pArgs);
	va_end(pArgs); 
	zlog(vdi, DEFAULT_FILE_VDI, ABUP_FILE_SIZE, inFuncName, strlen(inFuncName), inLineNumber, iConvert_LogLevel, "%s", szbuf);
}

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
                        const DCHAR *format,...)
{
    DCHAR szbuf[ABUP_DATA_SIZE] = {0};
	va_list pArgs;
    D32S temp_level = 0;
	
    /**可变参数序列化*/
    temp_level = level*DATA_SIZE2;
	va_start(pArgs, format);
	vsnprintf(szbuf, sizeof(szbuf), format, pArgs);
	va_end(pArgs); 

    zlog(dc, fileName, fileName_length, funcName, funcName_length, inLineNumber, temp_level, "%s", szbuf);
}

/**
 * @brief        host 业务log打印接口
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
                        const DCHAR *format,...)
{
    DCHAR szbuf[ABUP_DATA_SIZE] = {0};
	va_list pArgs;
    D32S temp_level = 0;
	
    /**可变参数序列化*/
    temp_level = level*DATA_SIZE2;
	va_start(pArgs, format);
	vsnprintf(szbuf, sizeof(szbuf), format, pArgs);
	va_end(pArgs); 

    zlog(host, fileName, fileName_length, funcName, funcName_length, inLineNumber, temp_level, "%s", szbuf);
}

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
D32S abup_zlog_init(DCHAR *logDir, DCHAR *logsDir, ABUP_LOG_LEVEL eLogLevel)
{
    D32S errCode = -1;
    DCHAR logVDIFile[ABUP_FILEPATH_SIZE] = {0};
    DCHAR logDCFile[ABUP_FILEPATH_SIZE] = {0};
    DCHAR logHOSTFile[ABUP_FILEPATH_SIZE] = {0};
    DCHAR logConf[ABUP_FILEPATH_SIZE] = {0};

    do
    {
        if ((!logDir) || (!logsDir)) 
        {
            fprintf(stderr, "logDir or logsDir is NULL error\n");
            errCode = -1;
            break;
        }

        /**创建配置文件目录*/
        adm_snprintf(logConf, ABUP_FILEPATH_SIZE, "%s/%s", logDir, DEFAULT_LOG_CONF_FILE);
        
        if(1 != rvdc_file_exist(logConf))
        {
            /** 创建配置文件 */
            if(0 != rvdc_file_create(logConf))
            {
                fprintf(stderr, "logConf = %s no exits", logConf);
                errCode = -1;
                break;
            }
        }

        /** 向配置文件中写默认配置 */
        adm_snprintf(logVDIFile, ABUP_FILEPATH_SIZE, "%s/%s.log", logsDir, DEFAULT_TARGET_VDI);
        adm_snprintf(logDCFile, ABUP_FILEPATH_SIZE, "%s/%s.log", logsDir, DEFAULT_TARGET_DC);
        adm_snprintf(logHOSTFile, ABUP_FILEPATH_SIZE, "%s/%s.log", logsDir, DEFAULT_TARGET_HOST);
        fprintf(stderr, "vdilog file directory%s\n",logVDIFile);
        fprintf(stderr, "dclog file directory%s\n",logDCFile);
        fprintf(stderr, "hostlog file directory%s\n",logHOSTFile);

        /**zlog配置文件初始化*/
        if(0 != zlog_conf_init(logConf, logsDir, eLogLevel, 3, DEFAULT_TARGET_VDI, DEFAULT_TARGET_DC, DEFAULT_TARGET_HOST))
        {
            fprintf(stderr, "log write default conf error"); 
            errCode = -2;
            break;
        }

        fprintf(stderr, "zlog Configuration file initialization %s\n",logConf);
        if(0 != zlog_init(logConf))
        {
            fprintf(stderr, "defalut conf init failure, adm log init failure\n");
            errCode = -3;
            break;
        }

        /**设置target*/
        dc = zlog_get_category(DEFAULT_TARGET_DC);
        if (NULL == dc) 
        { 
            fprintf(stderr, "get cat(%s) fail\n", DEFAULT_TARGET_DC);
            zlog_fini();
            errCode = -4;
            break;
        }
        
        /**设置target*/
        vdi = zlog_get_category(DEFAULT_TARGET_VDI);
        if (NULL == vdi) 
        { 
            fprintf(stderr, "get cat(%s) fail\n", DEFAULT_TARGET_VDI);
            zlog_fini();
            errCode = -1;
            break;
        }

        host = zlog_get_category(DEFAULT_TARGET_HOST);
        if (NULL == host)
        {
            fprintf(stderr, "get cat(%s) fail\n", DEFAULT_TARGET_HOST);
            zlog_fini();
            errCode = -4;
            break;
        }
        errCode = 0; 
    } while (0);

    return errCode;
}

/**
 * @brief        zlog模块反初始化
 * @note         
 * @param  [in]  void
 * @return       void
 * @date         2022-08-27
 * @author       liming
 */
 void abup_log_zlogfini(void)
 {
    zlog_fini();
 }