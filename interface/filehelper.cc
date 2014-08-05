#include "filehelper.h"
#include <ape/loghelper.h>
#include <cstring>
#include <cerrno>
#include <cstdio>
#ifndef MAX_PATH_LEN
#define  MAX_PATH_LEN 256
#endif
#ifdef WIN32
#include <direct.h>   
#include <io.h>
#define DIRSPLITOR '\\'
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define DIRSPLITOR '/'
#endif
namespace ape {
namespace common {
static inline char* SplitFileName(const char *szpath, char *szdir, char *szfilename) {
    char szbuf[MAX_PATH_LEN+1] = {0};
    strncpy(szbuf, szpath, MAX_PATH_LEN);
    char *p = strrchr(szbuf,DIRSPLITOR);
    if (p) {
        if (szfilename)
            strcpy(szfilename, p + 1);    
        *p = 0;
        if (szdir)
            strcpy(szdir,szbuf);
        return szfilename;
    }
    return NULL;
}
bool ExistFile(const char* szfile) {
    return access(szfile,0) != -1;
}
int MakeDirectory(const char* szpath) {
    if (ExistFile(szpath))
        return 0;
    char szparentdir[MAX_PATH_LEN+1] = {0};
    SplitFileName(szpath,szparentdir,NULL);
    if (szparentdir[0]!=0 && !ExistFile(szparentdir)) {
        if (0 != MakeDirectory(szparentdir))
          return -1;
    }
    return (mkdir(szpath, 0755) == 0 || errno == EEXIST) ? 0 : -1;
}
FILE *OpenFile(const char *filename, const char *mode) {
    FILE *file = fopen(filename, mode);
    if (NULL != file) {
        return file;
    }
    char szbuf[MAX_PATH_LEN+1];
    strncpy(szbuf, filename, MAX_PATH_LEN);
    char *p = strrchr(szbuf, DIRSPLITOR);
    if (p != NULL) {
        *p = '\0';
        MakeDirectory(szbuf);
    }
    return fopen(filename, mode);
}

int WriteToFile(FILE *file, const void *pbuf, unsigned int len) {
    if (file == NULL) {
        BS_XLOG(XLOG_WARNING,"LogThread::%s, file[%0X] is not available, buf[%s]\n",__FUNCTION__, file, (const char *)pbuf);
        return -1;
    }
    if (0 > fwrite(pbuf, 1, len, file)) {
        BS_XLOG(XLOG_WARNING,"%s, write buf to file failed, buf[%s]\n", __FUNCTION__, (const char *)pbuf);
        return -1;
    }
    fflush(file);
    return 0;
}
int WriteToFile(const std::string &strfile, const char *buf, int len) {
    FILE *file = OpenFile(strfile.c_str(), "a");
    if (NULL == file) {
        BS_XLOG(XLOG_WARNING,"WatchDogThread::%s, open file[%s] failed.\n", __FUNCTION__, strfile.c_str());
        return -1;
    }
    int ret = WriteToFile(file, buf, len);
    fclose(file);
    return ret;
}


}
}