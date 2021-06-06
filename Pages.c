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

/* A container function to check for key words */
static inline int page_macros(page_tmp_object* obj, const char* page, uint32_t index, uint32_t flags)
{
	if (!strncmp("xweb_", &page[index], 5)) {
      switch(page[index + 5]) {
			case 'a':
				break;
			case 'b':
				break;
			case 'c':
				if (!strncmp("xweb_css", &page[index], 8)) {
					puts("\033[31mxweb_css\033[0m");
			 
					if (flags & PAGE_TYPE_DESKTOP) {
						obj->data = create_string("<link rel=\"stylesheet\" href=\"desktop.css\">");
						break;
					}
					
					if (flags & PAGE_TYPE_MOBILE) {
						obj->data = create_string("<link rel=\"stylesheet\" href=\"mobile.css\">");
						break;
					}
				}
				break;
			case 'd':
				break;
			case 'e':
				break;
			case 'f':
				break;
			case 'g':
				break;
			case 'h':
				break;
			case 'i':
				if (!strncmp("xweb_include", &page[index], 12)) {
					puts("\033[31mxweb_include\033[0m");
				}
				break;
			case 'j':
				if (!strncmp("xweb_javascript", &page[index], 15) && ~(flags & PAGE_TYPE_TERMINAL)) {
					puts("\033[31mxweb_javascript\033[0m");
				}
				break;
			case 'k':
				break;
			case 'l':
				if (!strncmp("xweb_lang", &page[index], 9)) {
					puts("\033[31mxweb_lang\033[0m");
					obj->data = create_string_va("lang=\"", "jp", "\"", NULL);
					break; 
				}
				break;
			case 'm':
				if (!strncmp("xweb_menu", &page[index], 9)) {
					puts("\033[31mxweb_menu\033[0m");
				}
				break;
			case 'n':
				break;
			case 'o':
				break;
			case 'p':
				if (!strncmp("xweb_parent", &page[index], 11)) {
					puts("\033[31mxweb_parent\033[0m");
				}
				break;
			case 'q':
				break;
			case 'r':
				break;
			case 's':
				if (!strncmp("xweb_script", &page[index], 11)) {
					puts("\033[31mxweb_script\033[0m");
				}
				break;
			case 't':
				break;
			case 'u':
				break;
			case 'v':
				break;
			case 'w':
				break;
			case 'x':
				break;
			case 'y':
				break;
			case 'z':
				break;
		}
		return index + strlen(obj->data);
	}
	
	return 0;
}

/* Parsing the pages to a useble format for transmission */
int parse_pages(int fd, webpage* pages_in, const char* lang)
{
   uint32_t page_length = lseek(fd, 0, SEEK_END);
   char* page = mmap(0, page_length, PROT_READ, MAP_PRIVATE, fd, 0);
   char* cpage = malloc(sizeof(char) * page_length);

   page_tmp_object* page_list = NULL;
   page_tmp_object* page_list_next = NULL; 

   if (MAP_FAILED == page)
      goto ERROR_EXIT;

   for (uint32_t i = 0, s = 0; i < page_length; i++) {

      page_tmp_object* tmp = (page_tmp_object*)malloc(sizeof(page_tmp_object));
      tmp->data = NULL;
		s += page_macros(page_list, page, i, PAGE_TYPE_MOBILE);

      /*if (0 != page_macros("jp", page, cpage, &j, &i, PAGE_TYPE_MOBILE))
	 continue;*/
   }
   

   if (0 != munmap(page, page_length))
      goto ERROR_EXIT;

   write(1, "\n", 1); 
   write(1, cpage, page_length);

   return 0;
ERROR_EXIT:
   return -1;
}

int fetch_pages(client_sock* client)
{
   int32_t page_count = 0;
   int32_t lang_count = 0;
   DIR* dir = opendir(FOLDER_PAGES);
   DIR* lang_dir;
   char work_dir[PATH_MAX];

   puts("Getting working directory...");  
   if (NULL == getcwd(work_dir, sizeof(work_dir))) 
      goto EXIT_ERROR;

   puts("Chancing directory...");  
   if (0 != chdir(FOLDER_PAGES))
      goto EXIT_ERROR; 
   
   lang_dir = opendir("assets");

   puts("Checking directories for \033[35mNULL\033[0m values...");  
   if (NULL == dir && NULL == lang_dir)
      goto EXIT_ERROR;

   client->a_pages.count = page_count = get_dir_count(dir, DT_REG); 
   client->a_pages.lang = get_dir_names(lang_dir, &lang_count, DT_DIR);
   client->a_pages.data = (webpage**)malloc(sizeof(webpage*) * lang_count);
   
   if (NULL == client->a_pages.data)
      goto EXIT_ERROR;

   printf("The page count is %u\n\n", page_count);
   printf("The lang page count is %u\n\n", lang_count);

   for (int i = 0; i < lang_count; i++) {
      puts(client->a_pages.lang[i]);
   }

   int fd = open("index.html", O_RDONLY);
   parse_pages(fd, client->a_pages.data[0], "jp");
   close(fd);

   if (0 != closedir(dir))
      goto EXIT_ERROR;

   if (0 != closedir(lang_dir))
      goto EXIT_ERROR;

   return  0x0;
EXIT_ERROR:
   return -0x1;
}
