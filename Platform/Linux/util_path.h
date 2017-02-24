#ifndef _PATH_UTIL_H
#define _PATH_UTIL_H 

#include "stdbool.h"

#define FILE_SEPARATOR	'/'
#define MAX_PATH	260

#ifdef __cplusplus
extern "C" {
#endif

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
