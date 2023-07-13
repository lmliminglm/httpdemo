/**
 * @file httpdemo.c
 * @author liming (liming@abupdate.com)
 * @brief 
 * @version 0.1
 * @date 2023-07-13
 * 
 * @copyright Copyright (c) 2023  上海艾拉比智能科技有限公司
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include "adm_string.h"
#include "cJSON.h"
#include "adm_typedefs.h"
#include "adm_memory.h"
#include "curl/curl.h"


#define ADM_ECODE_SUCCESSED                        0
#define ADM_ECODE_FAILED                           -1
#define ADM_ECODE_INVALID_PARAM                    -2
#define ADM_ECODE_APPLY_MEMORY_FAILED              -3
#define ADM_ECODE_CURL_INIT_FAILED                 -4
#define ADM_ECODE_CURL_SET_ERR                     -5
#define ADM_ECODE_CURL_PERFORM_ERR                 -6
#define ADM_ECODE_CURL_GETINFO_ERR                 -7
#define ADM_ECODE_CURL_EASY_CLEANUP_ERR            -8
#define ADM_ECODE_CURL_SLIST_FREE_ALL_ERR          -9
#define ADM_ECODE_CURL_EASY_SETOPT_ERR             -10
#define ADM_ECODE_CURL_EASY_SETOPT_URL_ERR         -11
#define ADM_ECODE_CURL_EASY_SETOPT_WRITEFUNCTION_ERR   -12


#define ABUP_GWHOST_REGISTER_COMMONDTYPE                "COMMON_REGIST"         /** 注册请求以及响应注册请求 */
#define ABUP_GWHOST_SIGN                                "NULL"                  /** 签名(默认值，合创暂时不使用) */
#define ABUP_RII_SIGN_LENTH                             64                      /** 签名最大长度 */
#define PARAM_PAYLOAD                                   "payload"               /** 负载 */
#define PARAM_VIN                           "vin"           /** 车辆唯一识别码（17位字符串） */
#define ABUP_RII_REQUEST_BODY_MAX_LENGTH                (10*1024)               /** 请求体最大长度 */
#define     ABUP_RII_REMOTE_BASE_URL    "https://mock.apifox.cn/m1/2974256-0-default"
#define     URL_JSON_REPORT                "/rds/api/idiagnosis/V30/handle"

#define ABUP_VRS_INFO_LENGTH            100     /** 车辆注册信息元素数组最大值 */
#define ABUP_VRS_VIN_LENGTH             20      /** 车辆识别码字符串最大字节数 */

#define ADM_IS_ERR(iRet)        (ADM_ECODE_SUCCESSED != (iRet))
#define ADM_IS_SUCCESS(iRet)    (ADM_ECODE_SUCCESSED == (iRet))
#define ADM_IS_NULL(pstCb)      (NULL == (pstCb))

#define PARAM_COMMAND_TYPE                  "commandType"   /** 诊断交互类型 */
#define PARAM_SIGN                          "sign"          /** 签名 */
#define PARAM_DEVICE_ID                     "deviceId"      /** 车辆身份ID */
#define PARAM_TIMESTAMP                     "timestamp"     /** UNIX 时间戳 */

#define ABUP_IF_REMOTE_INFO_LENGTH          100             /** 车辆注册信息元素数组最大值 */

#define RESPONSE_STATUS_SUCCESS (10000)          /** status 状态码：成功 */
#define PARAM_RESPONSE_MSG                  "msg"           /** 业务状态描述信息 */
#define PARAM_DATA                          "data"          /** RDS和RDM诊断业务交互具体参数 */
#define PARAM_SECRET                        "secret"        /** 车端消息签名和加密解密key */
#define PARAM_RESPONSE_STATUS               "status"        /** status状态码（取值范围:1000~9999） */ 


/**
 * @struct  MemoryStruct
 * @brief   管理动态分配的内存的结构体 
 */
typedef struct
{
    DCHAR *memory;  /** 管理动态分配的内存的指针变量 */
    size_t size;    /** 记录保存到 memory 指针所指的内存中的数据的长度 */
} MemoryStruct;

typedef struct ABUP_VRS_REGISTER_INFO_ 
{
    DCHAR vinCode[ABUP_VRS_VIN_LENGTH];               /** 车辆的vin码 */  
    DCHAR deviceId[ABUP_VRS_INFO_LENGTH];             /** 车辆身份ID（vin的hexMd5值）*/  
    DCHAR secret[ABUP_VRS_INFO_LENGTH];               /** 车端消息签名key */
} ABUP_VRS_REGISTER_INFO_T;

/**
 * @struct  ABUP_RII_GWHOST_COMMON_REQ_T
 * @brief   通用请求结构体
 */
typedef struct ABUP_RII_GWHOST_COMMON_REQ_
{
    D64U timestamp;                             /** UNIX 时间戳（精确到秒，十位数） */
    DCHAR deviceId[100];                        /** 设备 id，首次注册默认填充空字符串 */
    DCHAR commandType[64];                      /** 诊断交互类型 */
    DCHAR sign[64];                             /** 签名 */
} ABUP_RII_GWHOST_COMMON_REQ_T;

typedef struct ABUP_IF_REMOTE_REGISTER_INFO_
{
    DCHAR deviceId[ABUP_IF_REMOTE_INFO_LENGTH];             /** 车辆身份ID（vin的hexMd5值） */
    DCHAR secret[ABUP_IF_REMOTE_INFO_LENGTH];               /** 车端消息签名key   */
} ABUP_IF_REMOTE_REGISTER_INFO_T;

/**
 * @brief           解析注册响应信息
 * @param   [in]    DCHAR *pcRspData 注册响应信息
 * @param   [out]   ABUP_IF_REMOTE_REGISTER_INFO_T *pstRegInfo 注册信息结构体
 * @param   [out]   D32S *piRemoteStatus 业务状态码
 * @return          D32S 0: Success，Non-0：Fail   
 */
static D32S parseRegisterRsp(
    DCHAR *pcRspData,
    ABUP_IF_REMOTE_REGISTER_INFO_T *pstRegInfo,
    D32S *piRemoteStatus)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    cJSON *pstRoot = NULL;
    cJSON *pstCJSON = NULL;
    cJSON *pstPayloadJsonVal = NULL;
    cJSON *pstDataJsonVal = NULL;

    do
    {
        /* 参数判空 */
        if(ADM_IS_NULL(pcRspData) || ADM_IS_NULL(pstRegInfo) || ADM_IS_NULL(piRemoteStatus))
        {
            printf("Invalid parameters\n");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        printf("response json: %s\n", pcRspData);

        /* 解析注册响应信息 */
        pstRoot = cJSON_Parse(pcRspData);
        if(ADM_IS_NULL(pstRoot))
        {
            printf("Json object to json string failed\n");
            iRet = -1;
            break;
        }

        /* 解析payload */
        pstPayloadJsonVal = cJSON_GetObjectItem(pstRoot, PARAM_PAYLOAD);
        if(ADM_IS_NULL(pstPayloadJsonVal))
        {
            printf("Parse seqNo json object from json object failed\n");
            iRet = -1;
            break;
        }

        /* 解析 status */
        pstCJSON = cJSON_GetObjectItem(pstPayloadJsonVal, PARAM_RESPONSE_STATUS);
        if (!cJSON_IsNumber(pstCJSON))
        {
            printf("Parse status json object from json object failed\n");
            iRet = -1;
            break;
        }
        *piRemoteStatus = pstCJSON->valueint;
        

        /* 判断 status 是否成功 */
        if (10000 != *piRemoteStatus)
        {
            printf("status: %d, expected: %d\n", *piRemoteStatus, RESPONSE_STATUS_SUCCESS);
            break;
        }

        /* 解析msg */
        pstCJSON = cJSON_GetObjectItem(pstPayloadJsonVal, PARAM_RESPONSE_MSG);
        if (!cJSON_IsString(pstCJSON))
        {
            printf("Parse msg json object from json object failed\n");
            iRet = -1;
            break;
        }
        printf("msg: %s\n", pstCJSON->valuestring);

        /* 解析 data */
        pstDataJsonVal = cJSON_GetObjectItem(pstPayloadJsonVal, PARAM_DATA);
        if(ADM_IS_NULL(pstDataJsonVal))
        {
            printf("Parse data json object from json object failed\n");
            iRet = -1;
            break;
        }

        /* 解析 deviceId */
        pstCJSON = cJSON_GetObjectItem(pstDataJsonVal, PARAM_DEVICE_ID);
        if(!cJSON_IsString(pstCJSON))
        {
            printf("Parse deviceId json object from json object failed\n");
            iRet = -1;
            break;
        }
        memcpy(pstRegInfo->deviceId, pstCJSON->valuestring, 
            ABUP_IF_REMOTE_INFO_LENGTH);

        /* 解析 secret */
        pstCJSON = cJSON_GetObjectItem(pstDataJsonVal, PARAM_SECRET);
        if(!cJSON_IsString(pstCJSON))
        {
            printf("Parse secret json object from json object failed\n");
            iRet = -1;
            break;
        }
        memcpy(pstRegInfo->secret, pstCJSON->valuestring, 
            ABUP_IF_REMOTE_INFO_LENGTH);
    } while (0);
    
    /* 资源释放 */
    cJSON_Delete(pstRoot);

    return iRet;
}

/**
 * @brief           通用上报信息 cJSON 封装
 * @note            将通用上报信息封装在 cJSON 结构体中
 * @param   [in]    pstCommonRequest 通用请求信息结构体
 * @return          cJSON * cJSON 结构指针变量，
 *                  注意：如果返回值不为 NULL，需要调用方调用 cJSON_Delete() 函数释放。
 */
static cJSON *createCommonReqJsonVal(ABUP_RII_GWHOST_COMMON_REQ_T *pstCommonRequest)
{
    cJSON *pstResult = NULL;
    D32S iStatus = ADM_ECODE_SUCCESSED;   
    cJSON *pstCJSON = NULL; 

    do
    {
        /* 参数判空 */
        if(NULL == pstCommonRequest)
        {
            printf("param commonRequest is NULL\n");
            break;
        }
        
        /* 创建作为根的 cJSON */
        pstResult = cJSON_CreateObject();
        if(NULL == pstResult)
        {
            printf("Failure in cJSON_CreateObject()\n");
            break;
        }

        /* 判断 commandType 的有效性 */
        if('\0' == pstCommonRequest->commandType[0])
        {
            printf("invalid commandType\n");
            iStatus = ADM_ECODE_FAILED;
            break;
        }

        /* 添加 commandType 到 json 中 */
        if (ADM_IS_NULL(cJSON_AddStringToObject(pstResult, PARAM_COMMAND_TYPE, 
            pstCommonRequest->commandType)))
        {
            printf("add commandType to json failed\n");
            iStatus = ADM_ECODE_FAILED;
            break;
        }

        /* 判断 sign 的有效性 */
        if('\0' == pstCommonRequest->sign[0])
        {
            printf("invalid sign\n");
            iStatus = ADM_ECODE_FAILED;
            break;
        }
        
        /* 添加 sign 到 json 中 */
        if (ADM_IS_NULL(cJSON_AddStringToObject(pstResult, PARAM_SIGN, 
            pstCommonRequest->sign)))
        {
            printf("add sign to json failed\n");
            iStatus = ADM_ECODE_FAILED;
            break;
        }

        // /* 判断 deviceId 的有效性 */
        // if('\0' == pstCommonRequest->deviceId[0])
        // {
        //     printf("invalid deviceId\n");
        //     iStatus = ADM_ECODE_FAILED;
        //     break;
        // }

        /* 添加 deviceId 到 json 中 */
        if (ADM_IS_NULL(cJSON_AddStringToObject(pstResult, PARAM_DEVICE_ID, 
            pstCommonRequest->deviceId)))
        {
            printf("add deviceId to json failed\n");
            iStatus = ADM_ECODE_FAILED;
            break;
        }

        /* 添加 timestamp 到 json 中 */
        if (ADM_IS_NULL(cJSON_AddNumberToObject(pstResult, PARAM_TIMESTAMP,
            pstCommonRequest->timestamp)))
        {
            printf("add timestamp to json failed\n");
            iStatus = ADM_ECODE_FAILED;
            break;
        }
    } while (0);
    
    /* 组装 json 存在错误时，需要释放资源 */
    if (ADM_IS_ERR(iStatus))
    {
        cJSON_Delete(pstResult);
        pstResult = NULL;
    }

    return pstResult;
}

/**
 * @brief           响应回调
 * @note            以接收所访问网页的一个或多个数据块；一次访问的多个数据块会通过多次回调此函数获得，
 *                  并通过 realloc() 方式不断追加到 MemoryStruct 的 memory 指针所指内存中。
 * @param   [in]    void *pvContents 
 * @param   [in]    size_t size
 * @param   [in]    size_t nmember
 * @param   [in]    void *userp
 * @return          size_t
 */
static size_t responseCallback(void *pvContents, size_t size, size_t nmember, void *userp)
{
    size_t realsize = size * nmember;
    MemoryStruct *pstMemory = (MemoryStruct *)userp;
    DCHAR *pcMemory = realloc(pstMemory->memory, pstMemory->size + realsize + 1);

    if (ADM_IS_NULL(pcMemory))
    {
        printf("reallocing memory failed\n");
        realsize = 0;
    }
    else
    {
        pstMemory->memory = pcMemory;
        memcpy(&(pstMemory->memory[pstMemory->size]), pvContents, realsize);
        pstMemory->size += realsize;
        pstMemory->memory[pstMemory->size] = 0;
    }
    
    return realsize;
}

/**
 * @brief 填充通用请求结构体
 * @param [in] pstCommonReq 通用请求结构体
 * @param [in] pcCommandType 命令类型
 * @param [in] iCommandType 命令类型字节数
 * @return D32S 0: Success，Non-0：Fail
 */
static D32S fillCommonReq(
    ABUP_RII_GWHOST_COMMON_REQ_T *pstCommonReq, 
    const DCHAR *pcCommandType, 
    D32S iCommandType)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    ABUP_VRS_REGISTER_INFO_T stRegisterInfo = { 0 };

    do
    {
        if (ADM_IS_NULL(pstCommonReq) || ADM_IS_NULL(pcCommandType))
        {
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }
        
        // createSeqNum(pstCommonReq->seqNo);
        pstCommonReq->timestamp = time(NULL);
        memcpy(pstCommonReq->commandType, pcCommandType, iCommandType);
        memcpy(pstCommonReq->sign, ABUP_GWHOST_SIGN, ABUP_RII_SIGN_LENTH);
    } while (0);
    
    return iRet;
}

/**
 * @brief           注册请求信息 cJSON 封装
 * @param   [in]    pstCommonRequest 通用请求信息结构体
 * @param   [in]    pcVin 车辆唯一识别码
 * @return          cJSON * cJSON 结构指针变量，
 *                  注意：如果返回值不为 NULL，需要调用方用 cJSON_Delete() 函数释放。
 */
static cJSON *createRegisterReqJsonVal(
    ABUP_RII_GWHOST_COMMON_REQ_T *pstCommonRequest, 
    const DCHAR *pcVin)
{
    cJSON *pstRet = NULL;
    cJSON *pstRoot = NULL;
    cJSON *pstPayload = NULL;

    do
    {
        /* 参数判空 */
        if (NULL == pstCommonRequest || NULL == pcVin )
        {
            printf("invlid params\n");
            break;
        }

        pstRoot = createCommonReqJsonVal(pstCommonRequest);
        if (NULL == pstRoot)
        {
            printf("Failure in createCommonReqJsonVal(commonRequest)\n");
            break;
        }
        
        /* 添加 payload 到 json 中 */
        pstPayload = cJSON_AddObjectToObject(pstRoot, PARAM_PAYLOAD);
        if (NULL == pstPayload)
        {
            printf("add payload to json failed\n");
            break;
        }

        /* 添加 vin 到 payload 中 */
        if (NULL == cJSON_AddStringToObject(pstPayload, PARAM_VIN, pcVin))
        {
            printf("add vin to json failed\n");
            break;
        }
        
        /* 到这里则说明构建 json 成功，赋值给返回值 */
        pstRet = pstRoot;
    } while (0);

    if (NULL == pstRet)
    {
        cJSON_Delete(pstRoot);
    }
    
    return pstRet;
}

/**
 * @brief           创建注册请求信息 json
 * @param   [in]    const DCHAR *pcVin 车辆唯一识别码
 * @param   [out]   DCHAR *pcReqStr 注册请求信息 json 字符串
 * @return          DCHAR * 字符串。注意：如果不为 NULL，需要调用方负责 free。
 */
static D32S createRegisterReqStr(const DCHAR* pcVin, DCHAR *pcRequestBody)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    DCHAR *pcPrintJson = NULL;
    cJSON *pstRoot = NULL;
    ABUP_RII_GWHOST_COMMON_REQ_T stGwHostCommonReq = { 0 };

    do
    {
        /* 参数判空 */
        if (ADM_IS_NULL(pcVin) || ADM_IS_NULL(pcRequestBody))
        {
            printf("invalid param\n");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }
        
        /* 填充通用请求结构 */
        if (ADM_IS_ERR(fillCommonReq(&stGwHostCommonReq, ABUP_GWHOST_REGISTER_COMMONDTYPE, 
            sizeof(ABUP_GWHOST_REGISTER_COMMONDTYPE))))
        {
            printf("fill common req failed\n");
            iRet = ADM_ECODE_FAILED;
            break;
        }

        /* 创建注册请求 json */
        pstRoot = createRegisterReqJsonVal(&stGwHostCommonReq, pcVin);
        if(ADM_IS_NULL(pstRoot))
        {
            printf("Create register request json value failed\n");
            iRet = ADM_ECODE_FAILED;
            break;
        }
        
        /* json 对象转为 json 字符串 */
        pcPrintJson = cJSON_Print(pstRoot);
        if (ADM_IS_NULL(pcPrintJson))
        {
            printf("json to string failed\n");
            iRet = ADM_ECODE_FAILED;
            break;
        }
        printf("request json: %s\n", pcPrintJson);

        /* 将 json 字符串拷贝到 pcReqStr 中 */
        adm_snprintf(pcRequestBody, ABUP_RII_REQUEST_BODY_MAX_LENGTH, "%s", pcPrintJson);
    } while (0);

    /* 资源释放 */
    cJSON_Delete(pstRoot);
    adm_free(pcPrintJson);

    return iRet;
}

/**
 * @brief           json 上报 post 请求
 * @note            
 * @param   [in]    DCHAR *pcUrl 接口地址
 * @param   [in]    DCHAR *pcRequestBody 请求体字符串
 * @param   [out]   DCHAR *pcResponseBody 响应体字符串
 * @return          D32S 0:success, Non-0:fail
 */
static D32S postJsonReport(DCHAR *pcUrl, DCHAR *pcRequestBody, DCHAR *pcResponseBody)
{
    D32S iRet = ADM_ECODE_SUCCESSED;
    CURL *pstCurl = NULL;
    struct curl_slist *pstHeaders = NULL;
    CURLcode eCurlCode = CURLE_OK;
    MemoryStruct stChunk = { 0 };   /** 用于在内存中保存接收到的数据 */

    do
    {
        /** 参数判空 */
        if (ADM_IS_NULL(pcRequestBody) || ADM_IS_NULL(pcResponseBody))
        {
            printf("Invaild paramters\n");
            iRet = ADM_ECODE_INVALID_PARAM;
            break;
        }

        stChunk.memory = adm_malloc(1);
        if (ADM_IS_NULL(stChunk.memory))
        {
            printf("allocating memory failed\n");
            iRet = ADM_ECODE_APPLY_MEMORY_FAILED;
            break;
        }
        stChunk.size = 0;

        /** 初始化 curl */
        pstCurl = curl_easy_init();
        if (ADM_IS_NULL(pstCurl))
        {
            printf("curl_easy_init() failed\n");
            iRet = ADM_ECODE_CURL_INIT_FAILED;
            break;
        }
        
        /** 给 curl 配置请求数据 */
        pstHeaders = curl_slist_append(pstHeaders, "Content-Type: application/json");
        if (ADM_IS_NULL(pstHeaders)) 
        {
            printf("pstHeaders is NULL\n");
            iRet = ADM_ECODE_CURL_SET_ERR;
            break;
        }

        curl_easy_setopt(pstCurl, CURLOPT_HTTPHEADER, pstHeaders);
        curl_easy_setopt(pstCurl, CURLOPT_MAXREDIRS, 3L);                               /** 请求重试次数为 3 次 */
        curl_easy_setopt(pstCurl, CURLOPT_VERBOSE, /*1L*/0L);
        curl_easy_setopt(pstCurl, CURLOPT_URL, pcUrl);
        curl_easy_setopt(pstCurl, CURLOPT_WRITEFUNCTION, responseCallback);             /** 响应回调 */
        curl_easy_setopt(pstCurl, CURLOPT_WRITEDATA, &stChunk);                         /** 响应回调传参 */
        curl_easy_setopt(pstCurl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(pstCurl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(pstCurl, CURLOPT_POSTFIELDS, pcRequestBody);                   /** 请求体 */
        curl_easy_setopt(pstCurl, CURLOPT_POSTFIELDSIZE, adm_strlen(pcRequestBody));    /** 请求体长度 */
        curl_easy_setopt(pstCurl, CURLOPT_FAILONERROR, 1L);                             /** 设置改参数后服务器httpCode>400, 则curlRet返回错误码22 */
        curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(pstCurl, CURLOPT_SSL_VERIFYHOST, 0L);

        /** 执行 curl */
        eCurlCode = curl_easy_perform(pstCurl);
        if (CURLE_OK != eCurlCode)
        {
            printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(eCurlCode));
            iRet = 100;
            break;
        }

        memcpy(pcResponseBody, stChunk.memory, stChunk.size);
        pcRequestBody[stChunk.size] = '\0';
    } while (0);
    
    /** 资源释放 */
    curl_easy_cleanup(pstCurl);
    curl_slist_free_all(pstHeaders);
    adm_free(stChunk.memory);

    return iRet;
}

/**
 * @brief 测试http请求
 * 
 */
static D32S test01()
{
    D32S iRet = 0;
    DCHAR requestBody[1024] = {0};
    DCHAR responseBody[1024] = { 0 };
    DCHAR Vin[20] = "12345678901234567";
    DCHAR szRegisterReqUrl[1024] = { 0 };
    ABUP_IF_REMOTE_REGISTER_INFO_T stRegInfo = { 0 };
    D32S iRemoteStatus = 0;

    /* 创建注册的请求体 */
    iRet = createRegisterReqStr(Vin, requestBody);
    if(ADM_IS_ERR(iRet))
    {
        printf("Create register request string failed\n");
        return -1;
    }

    /* 组装请求的 url */
    adm_snprintf(szRegisterReqUrl, sizeof(szRegisterReqUrl), "%s%s", ABUP_RII_REMOTE_BASE_URL, URL_JSON_REPORT);

    /* 发起请求 */
    iRet = postJsonReport(szRegisterReqUrl, requestBody, responseBody);
    if (ADM_IS_ERR(iRet))
    {
        printf("http post failed: %d\n", iRet);
        return -1;
    }

    /* 解析响应的数据 */
    iRet = parseRegisterRsp(responseBody, &stRegInfo, &iRemoteStatus);
    if(ADM_IS_ERR(iRet))
    {
        printf("Parse register response failed\n");
        return -1;
    }
    printf("____________________________成功——————————————————————————————————————————\n");
}

/**
 * @brief 测试httpdemo
 * 
 * @return int 
 */
int main()
{
    // 测试http请求
    test01();
    return 0;
}

