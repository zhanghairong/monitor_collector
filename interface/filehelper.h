#ifndef APE_COMMON_FILE_HELPER_H
#define APE_COMMON_FILE_HELPER_H
#include<string>
#include<stdio.h>
namespace ape {
namespace common {
bool ExistFile(const char* szfile);
int MakeDirectory(const char* szpath);
FILE *OpenFile(const char *filename, const char *mode);
int WriteToFile(FILE *file, const void *pbuf, unsigned int len);
int WriteToFile(const std::string &strfile, const char *buf, int len);
}
}
#endif
