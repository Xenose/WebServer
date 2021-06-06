#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<stdarg.h>
#include"Utils.h"


/* A helper function to create strings */
char* create_string(const char* text)
{
   char* tmp = (char*)malloc(sizeof(char) * strlen(text));
   strcpy(tmp, text);
   return tmp;
}

/* when the normal create string isn't enough there is this function */
char* create_string_va(const char* text, ...)
{
   char* tmp = NULL;
   uint32_t index = 0;
   uint32_t bytes = strlen(text);
   va_list a, b;
   
   va_start(a, text);
   va_copy(b, a);

   for (char* i = (char*)text; NULL != i; i = va_arg(a, char*)) { 
      bytes += strlen(i);
   }

   if (NULL == (tmp = (char*)malloc(bytes))) {
      goto EXIT;
   }
  
   for (char* i = (char*)text; NULL != i; i = va_arg(b, char*)) {
      memcpy(&tmp[index], i, strlen(i));
      index += strlen(i);
   }

EXIT:
   return tmp;
}

/* A function to hget the number of directories */
int32_t get_dir_count(DIR* dir, uint32_t flags)
{
   uint32_t count = 0;

   if (NULL == dir) {
      count = -1;
      goto EXIT;
   }
   
   for (struct dirent* info = readdir(dir); NULL != info; info = readdir(dir)) {
      if (flags != info->d_type)
			continue;

      if (!strcmp(".", info->d_name) || !strcmp("..", info->d_name))
			continue;

      ++count; 
   }

   rewinddir(dir);
EXIT:
   return count;
}

char** get_dir_names(DIR* dir, int32_t* count, uint32_t flags)
{
   char** tmp = NULL;
   *count = get_dir_count(dir, flags);

   if (-1 == *count)
      goto ERROR_EXIT;

   tmp = (char**)malloc(sizeof(char*) * *count);

   if (NULL == tmp)
      goto ERROR_FREE_EXIT;

   for (int i = 0; i < *count; i++) { 
      struct dirent* info = readdir(dir);
      
      if (flags != info->d_type)
	 continue;

      if (!strcmp(".", info->d_name) || !strcmp("..", info->d_name))
	 continue;

      tmp[i] = create_string(info->d_name);
   }

   rewinddir(dir);
   return tmp;

ERROR_FREE_EXIT:
   free(tmp);
ERROR_EXIT:
   return tmp;
}
