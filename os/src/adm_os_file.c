/*
* Description: file system 
* Author by: 
* Date: 2017-03-02
* 
*/

#define LOG_TAG   "ADM_FS"

#define _FILE_OFFSET_BITS 64

#include <stdio.h>
#include <sys/stat.h>
//#include <sys/statfs.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#include <stdarg.h>


#include "adm_typedefs.h"
#include "adm_error.h"
#include "adm_file.h"
#include "adm_string.h"
#include "adm_memory.h"

// #include "adm_debug.h"

// #define ADM_WORKING_DIR          "/opt/adm/"

ADM_ERROR adm_fopen(const DCHAR *inPath, const DCHAR *inMode,
                          void **outFile)
{
    struct stat sb1, sb2;
    ADM_ERROR result = ADM_ERR_OK;
    int lstatResult, lstatErrno;
    FILE *fp = NULL;

    *outFile = NULL;
    lstatResult = lstat(inPath, &sb1);
    lstatErrno = errno;

    fp = fopen(inPath, inMode);
    if (!fp) {
        //Could not open file
        result = ADM_ERR_STORAGE_OPEN;
        goto exit;
    }

    //Check for errors
    if (lstatResult == -1) {
        if (lstatErrno != ENOENT) {
            //Could not get file stat
            result = ADM_ERR_UNSPECIFIC;
            adm_fclose((void **)&fp);
        }
    } else if ((sb1.st_mode & S_IFMT) == S_IFLNK) {
        //File is a symlink (not supported)
        result = ADM_ERR_BAD_INPUT;
        adm_fclose((void **)&fp);
    } else if (fstat(fileno(fp), &sb2) == -1) {
        //Could not get file stat
        result = ADM_ERR_UNSPECIFIC;
        adm_fclose((void **)&fp);
    } else if (sb1.st_dev != sb2.st_dev || sb1.st_ino != sb2.st_ino) {
        //file switched between the fopen call and the first lstat call
        result = ADM_ERR_BAD_INPUT;
        adm_fclose((void **)&fp);
    }

exit:
    *outFile = (void *)fp;
    return result;
}

ADM_ERROR adm_fclose(void **outFile)
{
    ADM_ERROR result = ADM_ERR_OK;
    FILE *fp = (FILE *)*outFile;

    if (fp && fclose(fp))
        result = ADM_ERR_UNSPECIFIC;

    *outFile = NULL;

    return result;
}

ADM_ERROR adm_fsync(void *inFile)
{
    ADM_ERROR result = ADM_ERR_OK;
    FILE *fp = (FILE *)inFile;
    int fd;
    if (fflush(fp)) {
        if (errno != EBADF) { // EBADF Stream is not an open stream, or is not open for writing.
            result = ADM_ERR_UNSPECIFIC;
            goto end;
        }
    }

    fd = fileno(fp);
    if (fd == -1) {
        result = ADM_ERR_UNSPECIFIC;
        goto end;
    }

    if (fsync(fd))
        result = ADM_ERR_UNSPECIFIC;
end:
    return  result;
}

ADM_ERROR adm_frename(const DCHAR *inOldpath, const DCHAR *inNewpath)
{
    ADM_ERROR result = ADM_ERR_OK;
    int ret = rename(inOldpath, inNewpath);

    if (ret == -1)
        result = ADM_ERR_UNSPECIFIC;

    return result;
}

ADM_ERROR adm_fseek(void *inFile, D32U inOffset, E_ADM_FILE_SEEK_T inFromWhere)
{
    ADM_ERROR result = ADM_ERR_OK;
    FILE *fp = (FILE *)inFile;
    if (fp) {
        int position = SEEK_SET;
        switch (inFromWhere) {
        case E_ADM_FILE_SEEK_START:
            position = SEEK_SET;
            break;
        case E_ADM_FILE_SEEK_CURRENT:
            position = SEEK_CUR;
            break;
        case E_ADM_FILE_SEEK_END:
            position = SEEK_END;
            break;
        default:
            break;
        }

        if (lseek(fileno(fp), (off_t)inOffset, position) == (off_t) - 1) {
            result = ADM_ERR_STORAGE_SEEK;
        }

    }

    return result;
}

#define MAX_WRITE_RETRY 100
ADM_ERROR adm_fwrite(const void *inDataBuffer, D32U inDataLength, D32U *outTotalWrittenSize, void *inFile)
{
   // D32U writtedSize = 0;
    ADM_ERROR result = ADM_ERR_OK;
    D32U retryCounter = 0;
    D32U offset, written, totalWritten = 0;
    const DCHAR *data = (const DCHAR *)inDataBuffer;
    FILE *fp = (FILE *)inFile;

    if (!inDataBuffer || !inFile) {
        result = ADM_ERR_STORAGE_WRITE;
        goto end;
    }

    for (offset = 0; offset < inDataLength; offset += (D32U) written) {
        written = (D32U)fwrite(&data[offset], 1, (size_t)(inDataLength - offset), fp);

        if (ferror(fp)) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(10000);
                if (retryCounter++ > MAX_WRITE_RETRY) {
                    result = ADM_ERR_STORAGE_WRITE;
                    goto end;
                }
                continue;
            }
            result = ADM_ERR_STORAGE_WRITE;
            goto end;
        } else {
            totalWritten += written;
        }
    }


end:
    if (outTotalWrittenSize)
        *outTotalWrittenSize = totalWritten;

    return result;
}

ADM_ERROR adm_fread(void *outBuffer, D32U inBufferSize, D32U *outTotalReadSize, void *inFile)
{
    D32U readSize = 0;
    FILE *fp = (FILE *)inFile;
    ADM_ERROR result = ADM_ERR_OK;

    if (!outBuffer || !inFile)
        goto end;

    readSize = (D32U)fread(outBuffer, 1, inBufferSize, fp);

    if (ferror(fp))
        result = ADM_ERR_STORAGE_READ;

end:

    if (outTotalReadSize) {
        *outTotalReadSize = readSize;
    }
    return result;
}

ADM_ERROR adm_fremove(const DCHAR *inFileFullPathName)
{
    ADM_ERROR result = ADM_ERR_OK;

    if (!inFileFullPathName)
        goto end;

    result = !remove(inFileFullPathName) ? ADM_ERR_OK : ADM_ERR_STORAGE_REMOVE;

end:
    return result;
}

ADM_ERROR adm_fgetfilesize(const DCHAR *inFileFullPathName,  D32U *outSize)
{
    ADM_ERROR result = ADM_ERR_OK;
    struct stat statBuffer;

    if (!inFileFullPathName) {
        result =  ADM_ERR_BAD_INPUT;
        goto end;
    }

    if (stat(inFileFullPathName, &statBuffer) == 0) {
        if (statBuffer.st_size < 0) {
            result = ADM_ERR_STORAGE_READ;
        } else {
            *outSize = (D32U)statBuffer.st_size;
        }
    } else if (errno != ENOENT) {
        result = ADM_ERR_STORAGE_READ;
    }


end:
    return result;
}

ADM_ERROR adm_fgetpkgmaxsize(D32U *outMaxSize)
{
    ADM_ERROR ret = ADM_ERR_OK;
    if (outMaxSize)
        *outMaxSize = MAX_D32U;
    return ret;
}

DBOOL adm_fisexist(const DCHAR *inFileFullPathName)
{
    DBOOL ifExist = TRUE;

    if (access(inFileFullPathName, F_OK) == -1) {

        ifExist = FALSE;
    }

    return ifExist;
}

UTF8Str adm_fgets(void *outBuffer, D32U inBufferSize, void *inFile)
{
    DCHAR *ret = NULL;
    FILE *fp = (FILE *)inFile;
    if (fp) {
        ret =  fgets((DCHAR *)outBuffer, (int)inBufferSize, fp);
    }

    return (UTF8Str)ret;
}

ADM_ERROR adm_fgetworkingdir(DCHAR *outBuffer, D32U inBufferSize, D32U *outSize)
{
    ADM_ERROR ret = ADM_ERR_OK;
#if 0
    DCHAR *workingDir = DM_PL_Config_getStr("working_path");
    D32U  workingDirLen = 0;

    if (!workingDir)
        goto end;

    workingDirLen = adm_strlen((const DCHAR *) workingDir);
    if (!outBuffer || inBufferSize < workingDirLen) {
        ret = ADM_ERR_BUFFER_OVERFLOW;
        goto end;
    }

    adm_memcpy(outBuffer, (const void *)workingDir, workingDirLen);

end:
    if (outSize) {
        *outSize = workingDirLen;
    }
#endif
    return ret;
}

ADM_ERROR adm_fgetfilenameintree(DCHAR *outBuffer, D32U inBufferSize, D32U *outSize)
{
    ADM_ERROR ret = ADM_ERR_OK;
#if 0
    DCHAR *fileName = DM_PL_Config_getStr("tree_file_name");
    D32U  fileNameLen = adm_strlen((const DCHAR *) fileName);

    if (!outBuffer || inBufferSize < fileNameLen) {
        ret = ADM_ERR_BUFFER_OVERFLOW;
        goto end;
    }

    adm_memcpy(outBuffer, (const void *)fileName, fileNameLen);

end:
    if (outSize) {
        *outSize = fileNameLen;
    }
#endif
    return ret;
}

static ADM_ERROR _mkdir(const char *inFolderPath, const mode_t inMode)
{
    int rc;

    log_i("inFolderPath: %s\n", inFolderPath);

    rc = mkdir(inFolderPath, inMode);

	if(errno == EEXIST)
		;//log_i("mkdir path is already exist");
	else if(!rc)
		;//log_i("mkdir path create ok");
	else
    	log_i("mkdir result: %d errno: %d\n", rc, errno);

    return !rc || errno == EEXIST ? ADM_ERR_OK : ADM_ERR_UNSPECIFIC;
}

static ADM_ERROR recursiveMkdir(const DCHAR *inFolderPath, const mode_t inMode)
{
    ADM_ERROR ret = ADM_ERR_OK;
    DCHAR *path = NULL;
    DCHAR *slash = NULL;
	D32U pathLen = 0;

    log_i("path: %s\n", inFolderPath);

    if (!inFolderPath) {
        ret = ADM_ERR_BAD_INPUT;
        goto end;
    }

	pathLen = adm_strlen(inFolderPath) + 1;
    path = adm_malloc(pathLen);
    if (!path) {
        ret = ADM_ERR_MEMORY;
        goto end;
    }
   adm_strcpy(path, inFolderPath, pathLen);

    slash = path;

    while ((slash = adm_strchr(slash + 1, '/'))) {
        *slash = '\0';

        ret = _mkdir(path, inMode);
        if (ret != ADM_ERR_OK)
            break;

        *slash = '/';
    }

    ret = _mkdir(inFolderPath, inMode);

end:
    adm_free(path);
    return ret;
}



//TEST
#define ADM_WORKING_DIR "/opt/adm"
#define ADM_FS_FILENAME_TEST "admtest"
#define ADM_FS_FILENAME_SUFFIX_TEST ".bin"
#define ADM_FS_FULL_NAME_TEST           ADM_WORKING_DIR "/" ADM_FS_FILENAME_TEST ADM_FS_FILENAME_SUFFIX_TEST

D32S adm_write_file(DCHAR *fullname, D8U *buf, D32U len, DBOOL append)
{
    D32S isOK = 0;
	FILE *file_handle = NULL;
    const DCHAR *fileFullName = fullname;
    const DCHAR *directoryPath = NULL;

	if(!buf || len==0)
		return -1;

    if (!fileFullName)
        fileFullName = ADM_FS_FULL_NAME_TEST;
    

    if (!directoryPath)
        directoryPath = ADM_WORKING_DIR;

    // Create the folder if needed
    recursiveMkdir(directoryPath, 0777);

    if (append == 0) 
    {
        //write
        adm_fopen(fileFullName, "w+", (void **)&file_handle);
    }
    else
    {
        //append
        adm_fopen(fileFullName, "a+", (void **)&file_handle);
    }

    if (!file_handle) 
    {
//        log_i( "!!! The file '%s' cannot be opened, %m", fileFullName);
        isOK = -1;
    } 
    else 
    {
        D32U writon_len = 0;
  //      log_i( "************** output to file '%s' **************", fileFullName);

        adm_fwrite(buf, len, &writon_len, file_handle);

    //    log_i( "************** output to file: writon len: %d, want len:%d **************", writon_len, len);

        adm_fclose((void **)&file_handle);
    }

    return isOK;
}

D32S adm_read_file(DCHAR *fullname, D8U *buf, D32U len)
{
    D32S retval = 0;
	FILE *file_handle = NULL;
    const DCHAR *fileFullName = fullname;
    const DCHAR *directoryPath = NULL;

	if(!buf)
		return -1;

    if (!fileFullName)
        fileFullName = ADM_FS_FULL_NAME_TEST;
    

    if (!directoryPath)
        directoryPath = ADM_WORKING_DIR;

    adm_fopen(fileFullName, "r+", (void **)&file_handle);

    if (!file_handle) 
    {
      //  log_i( "!!! The file '%s' cannot be opened, %m", fileFullName);
        retval = -1;
    } 
    else 
    {
        D32U read_len = 0;
        //log_i( "************** read from file '%s' **************", fileFullName);

		retval = adm_fread(buf, len, &read_len, file_handle);
        if(retval != ADM_ERR_OK)
        {
	//		log_i( "read file: fail!!! retval:%d", retval);
        }
		else
        {
	//		log_i( "************** read from file: read len: %d, want len:%d **************", read_len, len);
            retval = read_len;
        }

        adm_fclose((void **)&file_handle);
    }

    return retval;
}

void adm_file_test_entry(void)
{
	DCHAR *str1 = "This is my file api test!\r\n";
	DCHAR *str2 = "This is my append content!";
	DCHAR *strOut[128] = {0};
	D32S ret = -1, readLen=0;
	
	ret = adm_write_file(ADM_FS_FULL_NAME_TEST, (D8U *)str1,adm_strlen(str1), 0);
	if(ret >= 0)
		log_i("write str1 ok");
	else
		log_i("write str1 fail!");
	
	ret = adm_write_file(ADM_FS_FULL_NAME_TEST, (D8U *)str2,adm_strlen(str1), 1);
	if(ret >= 0)
		log_i("write str2 ok");
	else
		log_i("write str2 fail!");


	readLen = adm_read_file(ADM_FS_FULL_NAME_TEST, (D8U *)strOut, 128);
	if(readLen > 0)
	{
		log_i("read str ok, len=%d", readLen);
		log_i("read data:\r\n%s", strOut);
	}
	else
		log_i("read str fail!");

}
