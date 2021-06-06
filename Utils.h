#ifndef __utils_header__
#define __utils_header__
#include<stdint.h>
#include<uchar.h>
#include<dirent.h>

extern char* create_string(const char* text);
extern char* create_string_va(const char* text, ...);
extern int32_t get_dir_count(DIR* dir, uint32_t flags);
extern char** get_dir_names(DIR* dir, int32_t* count, uint32_t flags);

#endif //__utils_header__
