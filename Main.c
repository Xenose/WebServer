#define _GNU_SOURCE
#include<sys/types.h>
#include<sched.h>
#include<signal.h>
#include<sys/socket.h>
#include<netdb.h>
#include<poll.h>
#include<limits.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include"Settings.h"
#include"Types.h"
#include"Pages.h"

#define SERVER_FLAG_IS_RUNNING 0x1

atomic_uint __server_flags = SERVER_FLAG_IS_RUNNING;

/* Setting up the server information */
int setup_server_socket(server_sock* s)
{
   memset(&s->hints, 0, sizeof(s->hints));

   s->hints.ai_family	= AF_INET6;
   s->hints.ai_socktype = SOCK_STREAM;
   s->hints.ai_flags	= AI_PASSIVE;

   if (0 != getaddrinfo(0x0, s->port, &s->hints, &s->_result))
      goto ERROR_EXIT;

   s->fd = socket(s->_result->ai_family, s->_result->ai_socktype, s->_result->ai_protocol);

   if (0 > s->fd)
      goto ERROR_FREE_ADDRINFO;

   if (0 != setsockopt(s->fd, IPPROTO_IPV6, IPV6_V6ONLY, (void*) &s->option, sizeof(s->option)))
      goto ERROR_FREE_ADDRINFO;

   if (0 != bind(s->fd, s->_result->ai_addr, s->_result->ai_addrlen))
      goto ERROR_FREE_ADDRINFO;

   if (0 != listen(s->fd, BACKLOG))
      goto ERROR_FREE_ADDRINFO;

   freeaddrinfo(s->_result);
   return  0x0;

ERROR_FREE_ADDRINFO:
   freeaddrinfo(s->_result);
ERROR_EXIT:
   return -0x1;
}

int setup_client_sockets(client_sock* c)
{
   /* Setting the atomic values to the maximum size,
      indecating that any fd can be read or written. */
   c->w_index = UINT_MAX;
   c->r_index = UINT_MAX;
   c->length = INPUT_SOCK_COUNT;
   c->data = (struct pollfd*) malloc(sizeof(struct pollfd) * c->length);

   if (NULL == c->data)
      goto ERROR_EXIT;

   for (uint32_t i = 0; i < c->length; i++) {
      c->data[i].fd = -1;
   }

   return  0x0;
ERROR_EXIT:
   return -0x1;
}

/* We handle all the connections here */
void send_response(client_sock* client, uint32_t index)
{
   if (client->data[index].events & POLLIN) {
      puts("pollin");
   }
      
   if (client->data[index].events & POLLRDNORM) {
      puts("pollrdnorm");
   }
   
   if (client->data[index].events & POLLRDBAND) {
      puts("pollrdband");
   }
  
   if (client->data[index].events & POLLPRI) {
      puts("pollpri");
   }
   
   if (client->data[index].events & POLLOUT) {
      puts("pollout");

      long bytes = send(client->data[index].fd,
	       "HTTP/1.1 404 Not Found \r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n 404",
	       strlen("HTTP/1.1 404 Not Found \r\nConnection: close\r\nContent-Type: text/plain\r\n\r\n 404"), 0);

      printf("Sent to fd %d : %ld bytes\n", client->data[index].fd, bytes);

      close(client->data[index].fd);
      client->data[index].fd = -1;
   }
 
   if (client->data[index].events & POLLWRNORM) {
      puts("pollwrnorm");
   }
   
   if (client->data[index].events & POLLWRBAND) {
      puts("pollwrband");
   }
   
   if (client->data[index].events & POLLERR) {
      puts("pollerr");
   }
      
   if (client->data[index].events & POLLHUP) {
      puts("pollhup");
   }

   if (client->data[index].events & POLLNVAL) {
      puts("pollnval");
   }
}

/* this thread will handle output and input */
int response_thread(void* cl_sock)
{
   client_sock* client = (client_sock*) cl_sock;
   int pret;

   while(1) {
     
      pret = poll(client->data, client->length, 0);

      /// No data no need to check
      if (0 == pret)
	 continue;

      if (-1 == pret)
	 return -1;

      for (uint32_t i = 0; i < client->length; i++) {
	 if (-1 == client->data[i].fd)
	    continue;
	 
	 if (i == client->w_index)
	    continue;
	 
	 client->r_index = i;
	 
	 if (i == client->w_index) {
	    client->w_index = UINT_MAX;
	    continue;
	 }

	 /* we are finlly ready to read and write */
	 send_response(client, i); 
	 client->r_index = UINT_MAX;
      }
   }

   return 0x0;
}

int main(int arc, char** arv)
{
   char response_thread_stack[STACK_SIZE];
   
   struct sockaddr_storage client_info;
   socklen_t client_info_length = sizeof(client_info); 

   server_sock server;
   client_sock client;

   memset(response_thread_stack, 0, STACK_SIZE);
   server.option = 0x0;
   server.port = (char*) malloc(sizeof(char) * 5);
   strcpy(server.port, PORT_NUMBER);

   if (0x0 != setup_server_socket(&server))
      goto ERROR_EXIT;

   if (0x0 != setup_client_sockets(&client))
      goto ERROR_EXIT;

   if (0x0 != fetch_pages(&client))
      goto ERROR_EXIT;

   if (-1 == clone(&response_thread, response_thread_stack + STACK_SIZE, CLONE_FILES | CLONE_VM | SIGCHLD ,&client)) {
      goto ERROR_EXIT;
   }   

   while (1) {
     int tmp_fd = accept(server.fd, (struct sockaddr*)&client_info, &client_info_length);

     if (-0x1 == tmp_fd)
	continue;
      
     /* This will read the file discriptor nothing else */
     for (uint32_t i = 0; i < client.length; i++)
     {
	if (-1 != client.data[i].fd)
	   continue;

	/* Checking if the current index is in use */
	if (i == client.r_index)
	  continue;
	
	/* Writting is about to start setting the index */
	client.w_index = i;

	/* One extra check for safety */
	if (i == client.r_index) {
	   client.w_index = UINT_MAX;
	   continue;
	}

	client.data[i].fd = tmp_fd;

	/* Writting is over moving index back to the max value */
	client.w_index = UINT_MAX;
	break;
     }
   }

   return EXIT_SUCCESS;
ERROR_EXIT:
   perror(" \033[31mERROR\033[0m ");
   return EXIT_FAILURE;
}
