#ifndef __types_header__
#define __types_header__
#include<netdb.h>
#include<stdatomic.h>
#include<stdint.h>
#include<poll.h>
#include<uchar.h>

#define PAGE_TYPE_TERMINAL	0x1
#define PAGE_TYPE_DESKTOP	0x2
#define PAGE_TYPE_MOBILE	0x4

typedef struct __page_tmp_object {
   struct page_tmp_object* next;
   char* data;
} page_tmp_object;

typedef struct __webpage {
   char* name;

   uint32_t desktop_length;
   char16_t* desktop;

   uint32_t mobile_length;
   char16_t* mobile;

   uint32_t terminal_length;
   char16_t* terminal;
} webpage;

typedef struct __available_pages {
   uint32_t count;
   char** lang;
   webpage** data;
} available_pages;

typedef struct __server_sock {
   int fd;
   int option;
   char* port;
   uint32_t flags;
   
   struct addrinfo  hints;
   struct addrinfo* _result; 
} server_sock;

typedef struct __client_sock {
   atomic_uint w_index;
   atomic_uint r_index;

   uint32_t length;
   struct pollfd* data;

   available_pages a_pages;
} client_sock;


#endif //__types_header__
