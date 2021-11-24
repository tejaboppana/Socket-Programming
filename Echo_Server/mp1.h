#include	<sys/socket.h>	
#include	<netinet/in.h>
#include        <arpa/inet.h>	
#include	<errno.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#define BACKLOG 4
#define MAXBUF 4096
static void	err_doit(int, int, const char *, va_list);
void err_sys(const char *fmt, ...);


