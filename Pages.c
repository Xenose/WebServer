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
			case 'b':
			case 'c':
				if (!strncmp("xweb_css", &page[index], 8)) {
					puts("\033[31mxweb_css\033[0m");
			 
					if (flags & PAGE_TYPE_DESKTOP) {
						obj->data = create_string("<link rel=\"stylesheet\" href=\"desktop.css\">");
						goto CSS_RETURN;
					}
					
					if (flags & PAGE_TYPE_MOBILE) {
						obj->data = create_string("<link rel=\"stylesheet\" href=\"mobile.css\">");
						goto CSS_RETURN;
					}

					obj->data = create_string("");
					
CSS_RETURN:
					return strlen("xweb_css");
				}
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
				if (!strncmp("xweb_include", &page[index], 12)) {
					puts("\033[31mxweb_include\033[0m");
					obj->data = create_string("");
					return strlen("xweb_include");
				}
			case 'j':
				if (!strncmp("xweb_javascript", &page[index], 15) && ~(flags & PAGE_TYPE_TERMINAL)) {
					puts("\033[31mxweb_javascript\033[0m");
					return strlen("xweb_javascript");
				}
			case 'k':
			case 'l':
				if (!strncmp("xweb_lang", &page[index], 9)) {
					puts("\033[31mxweb_lang\033[0m");
					obj->data = create_string_va("lang=\"", "jp", "\"", NULL);
					return strlen("xweb_lang");
				}
			case 'm':
				if (!strncmp("xweb_menu", &page[index], 9)) {
					puts("\033[31mxweb_menu\033[0m");
					obj->data = create_string("");
					return strlen("xweb_menu");
				}
			case 'n':
			case 'o':
			case 'p':
				if (!strncmp("xweb_parent", &page[index], 11)) {
					puts("\033[31mxweb_parent\033[0m");
					obj->data = create_string("");
					printf("%lu\n", strlen("xweb_parent"));	
					return strlen("xweb_parent");
				}
			case 'q':
			case 'r':
			case 's':
				if (!strncmp("xweb_script", &page[index], 11)) {
					puts("\033[31mxweb_script\033[0m");
					return strlen("xweb_script");
				}
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
				{}
		}
	}
	return 0;
}

/* Parsing the pages to a useble format for transmission */
int parse_pages(int fd, webpage* pages_in, const char* lang, uint32_t flags)
{
   uint32_t page_length = lseek(fd, 0, SEEK_END);
	uint32_t page_bytes = 0;
   char* page = mmap(0, page_length, PROT_READ, MAP_PRIVATE, fd, 0);
   char* rpage = NULL;
	
	/* loop varibles index, start, result */
	int32_t i, s, r;

	/* if mmap failed return with -1 */
   if (MAP_FAILED == page)
      goto ERROR_EXIT;

	/* Pre-Allocating data, we don't want a segfault */
   page_tmp_object* tmp = (page_tmp_object*)malloc(sizeof(page_tmp_object));
   page_tmp_object* page_list = tmp;
	page_tmp_object* page_next = NULL;
   
	tmp->data = NULL;
	tmp->next = (page_tmp_object*)malloc(sizeof(page_tmp_object));

   for (i = 0, s = 0, r = 0; i < page_length; i++) {

		if (0 == (r = page_macros(tmp->next, page, i, flags)))
			continue;

		/* Magic time! */
		int32_t html_length = i - s;
		tmp->data = malloc(sizeof(char) * html_length);
		memcpy(tmp->data, &page[s], html_length);
		
		page_bytes += strlen(tmp->data) + strlen(tmp->next->data);
		tmp->next->next = (page_tmp_object*)malloc(sizeof(page_tmp_object));
		tmp->next->next->next = (page_tmp_object*)malloc(sizeof(page_tmp_object));
		tmp = tmp->next->next;

		s = i + r;
   }

	printf("s %d i %d\n", s, i);

	tmp->data = malloc(sizeof(char) * (i - s));
	memcpy(tmp->data, &page[s], i - s);
	page_bytes += i - s;

	rpage = (char*)malloc(sizeof(char) * page_bytes);
	page_next = page_list;

	for (i = 0; NULL != page_next->next;  ) {
		page_tmp_object* old = page_next;
		memcpy(&rpage[i], page_next->data, strlen(page_next->data));
		i += strlen(page_next->data);
		
		page_next = page_next->next;
		free(old);
	}
   

   if (0 != munmap(page, page_length))
      goto ERROR_EXIT;

	write(1, rpage, page_bytes);

	printf("\rPage is %u bytes long\n", page_bytes);

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
   parse_pages(fd, client->a_pages.data[0], "jp", PAGE_TYPE_MOBILE);
   close(fd);

   if (0 != closedir(dir))
      goto EXIT_ERROR;

   if (0 != closedir(lang_dir))
      goto EXIT_ERROR;

   return  0x0;
EXIT_ERROR:
   return -0x1;
}
