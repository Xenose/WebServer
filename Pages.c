#define __GNU_SOURCE
#include<sys/mman.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<dirent.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include"Pages.h"
#include"Settings.h"
#include"Utils.h"

int parse_pages(int fd, webpage* pages_in)
{
   uint32_t page_length = lseek(fd, 0, SEEK_END);
   char* page = mmap(0, page_length, PROT_READ, MAP_PRIVATE, fd, 0);
   char* cpage = malloc(sizeof(char) * page_length);

   if (MAP_FAILED == page)
      goto ERROR_EXIT;

   for (uint32_t i = 0; i < page_length; i++) {

      if (' ' !=  cpage[i]) {
	 if ('\n' !=  cpage[i]) {
	    if (!strncmp("xweb_lang", &page[i], 9)) {
	       memcpy(&cpage[i], "lang=\"jp\"", sizeof(char) * strlen("lang=\"jp\""));
	       i += strlen("xweb_lang") - 1;
	       continue;
	    }
	 }
      }

      cpage[i] = page[i];
   }
   

   if (0 != munmap(page, page_length))
      goto ERROR_EXIT;
   
   write(1, cpage, page_length);

   return 0;
ERROR_EXIT:
   return -1;
}

int fetch_pages(client_sock* client)
{
   uint32_t page_count = 0;
   DIR* dir = opendir(FOLDER_PAGES);
   char work_dir[PATH_MAX];

   if (NULL == getcwd(work_dir, sizeof(work_dir)))
      goto EXIT_ERROR;

   if (0 != chdir(FOLDER_PAGES))
      goto EXIT_ERROR;

   if (NULL == dir)
      goto EXIT_ERROR;

   for (struct dirent* info = readdir(dir); NULL != info; info = readdir(dir)) {

      if (DT_REG != info->d_type)
	 continue;

      puts(info->d_name);
      ++page_count; 
   }

   // TODO remove this
   printf("The page count is %u\n\n", page_count);

   client->a_pages.count = page_count;
   client->a_pages.data = (webpage*)malloc(sizeof(webpage) * page_count);
   rewinddir(dir);

   if (NULL == client->a_pages.data)
      goto EXIT_ERROR;
   
   struct dirent* info = readdir(dir);
   for (uint32_t i = 0; NULL != info; i++, info = readdir(dir)) {

      if (DT_REG != info->d_type)
	 continue;

      client->a_pages.data[i].name = create_string(info->d_name);
      int fd = open(info->d_name, O_RDONLY);
      
      if (-1 == fd) {
	 free(client->a_pages.data[i].name);
	 client->a_pages.data[i].name = NULL;
	 continue;
      }

      parse_pages(fd, &client->a_pages.data[i]);
      close(fd);
   }

   if (0 != closedir(dir))
      goto EXIT_ERROR;

   return  0x0;
EXIT_ERROR:
   return -0x1;
}
