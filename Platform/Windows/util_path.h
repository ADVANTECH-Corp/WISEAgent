#ifndef _UTIL_PATH_H
#define _UTIL_PATH_H 

#include "stdbool.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FILE_SEPARATOR   '\\'

void util_split_path_file(char const *filepath, char* path, char* file);

void util_path_combine(char* destination, const char* path1, const char* path2);

int util_module_path_get(char * moudlePath);

bool util_create_directory(char* path);

unsigned long util_get_temp_path(char* lpBuffer, int nBufferLength);

bool util_is_file_exist(char const *filepath);

void util_remove_file(char const *filepath);

bool util_copy_file(char const *sourcefile, char const *targetfile);

#ifdef __cplusplus
}
#endif


#endif