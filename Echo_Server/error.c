#include <stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdarg.h>
#include<syslog.h>
#include<errno.h>
static void	err_doit(int, int, const char *, va_list);

void err_sys(const char *fmt, ...){
	va_list		ap;

	va_start(ap, fmt);
	err_doit(1, LOG_ERR, fmt, ap);
	va_end(ap);
	exit(1);
}

static void err_doit(int errnoflag, int level, const char *fmt, va_list ap){
	int		errno_save, n;
	char	buf[4096 + 1];

	errno_save = errno;		/* value caller might want printed */
    #ifdef	HAVE_VSNPRINTF
	  vsnprintf(buf, 4096, fmt, ap);	/* safe */
    #else
	  vsprintf(buf, fmt, ap);					/* not safe */
    #endif
	n = strlen(buf);
	if (errnoflag)
		snprintf(buf + n, 4096 - n, ": %s", strerror(errno_save));
	strcat(buf, "\n");

	fflush(stdout);		/* in case stdout and stderr are the same */
	fputs(buf, stderr);
	fflush(stderr);
	
	return;
}
