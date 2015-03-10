/*
 * main.c -- Main program for the GoAhead WebServer (LINUX version)
 *
 * Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
 *
 * See the file "license.txt" for usage and redistribution license requirements
 */

/******************************** Description *********************************/

/*
 *	Main program for for the GoAhead WebServer. This is a demonstration
 *	main program to initialize and configure the web server.
 */

/********************************* Includes ***********************************/

#include	"uemf.h"
#include	"wsIntrn.h"
#include	<signal.h>
#include	<unistd.h> 
#include	<sys/types.h>
#include	<sys/ioctl.h>
#include	<net/if.h>
#include	<sys/wait.h>
#include	<netdb.h>
#include	<syslog.h>
#include	<src/server.h>
#include	<src/mysyslog.h>
#include	<src/config_lock.h>
#include	<execinfo.h>
#include	<string.h>
#include	<time.h>

#ifdef WEBS_SSL_SUPPORT
#include	"websSSL.h"
#endif

#ifdef USER_MANAGEMENT_SUPPORT
#include	"um.h"
#ifndef qDmf
void	formDefineUserMgmt(void);
#endif
#endif

#ifdef qDmf
#include "../AlFrame/Dmf.h"
#endif

#include "../../Server/webApi.h"

#include <cyc.h>

struct cyc_info *cyc=NULL;

int qError = 1;
int qTrace = 0;
int qDebug = 0;
int executeFlg = 0;
int seclevel = 0;
int sslVer = 0;
char sslCipher[100];

extern int web_idle_timeout;
extern char_t kSidKey[15];

// array of logged-in web users
WebUserEntry webUsers[MAX_WEB_USERS];

/*********************************** Locals ***********************************/
/*
 *	Change configuration here
 */

static char_t		*rootWeb = T("new_web");			/* Root web directory */
static char_t		*password = T("");				/* Security password */
static int			port = 80;						/* Server port */
static int			retries = 1;					/* Server port retries */
static int			finished = 0;						/* Finished flag */

/****************************** Forward Declarations **************************/

extern void cycSetAspFunc(void);
static int 	initWebs();
#if 0
static int	aspTest(int eid, webs_t wp, int argc, char_t **argv);
static void formTest(webs_t wp, char_t *path, char_t *query);
#endif
static int  websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
				int arg, char_t *url, char_t *path, char_t *query);
extern void defaultErrorHandler(int etype, char_t *msg);
extern void defaultTraceHandler(int level, char_t *buf);
/* SIGINT handler... */
static void sig_int(int);
#if B_STATS
static void printMemStats(int handle, char_t *fmt, ...);
static void memLeaks();
#endif

#if MEMWATCH
void *my_ex_malloc(size_t size, const char *file, int line)
{
	return(mwMalloc( size, file, line ));
}

void *my_ex_realloc(void *old, size_t size, const char *file, int line)
{
	return(mwRealloc(old, size, file, line));
}

void my_ex_free(void *p)
{
	mwFree(p,__FILE__,__LINE__);
}
#endif

/*********************************** Code *************************************/
/*
 *	Main -- entry point from LINUX
 */


/* warning: `print_trace' defined but not used
static void print_trace (void)
{
        void *array[16];
        size_t size;
        char **strings;
        size_t i;
        //int port; warning: unused variable `port'

        size = backtrace(array, 16);
        strings = backtrace_symbols (array, size);

        printf("Stack Backtrace - Obtained %d stack frames", size);

        for (i = 0; i < size; i++) {
                printf("%s\n", strings[i]);
        }

        free(strings);
        exit(0);
}*/

void AcsWeb_atexit(void)
{
	libcyc_destroy(cyc);
	CycEndApi();
}

int main(int argc, char** argv)
{
	int opt, httpPort = 0, httpsPort = 0;
	/*warning: missing braces around initializer
	 struct sigaction sa = {NULL};*/
	struct sigaction sa = {{NULL}};
	extern char *optarg;
	extern int optind;
	int secs_toquit=10;

	/* Parse command-line arguments. */
	while ((opt = getopt(argc, argv, "s:p:P:T:t:v:c:de")) != EOF) {
		switch (opt) {
		case 'p':
            httpPort = atoi(optarg);
            break;
		case 'P':
            httpsPort = atoi(optarg);
            break;
		case 'd':
			qDebug = 1;
            break;
		case 'e':
			qError = 0;
            break;
		case 'T':
			web_idle_timeout = atoi(optarg);
            break;
		case 't':
			qTrace = atoi(optarg);
            break;
		case 's':
			seclevel = atoi(optarg);
            break;
		case 'v':
			sslVer = atoi(optarg);
            break;
		case 'c':
			strncpy(sslCipher,optarg,100);
	    break;
		default:
			printf("USAGE: %s [options]\n\n"
				"where options are:\n"
				"-p <n>    : HTTP  port. Default wwww  from /etc/services\n"
				"-P <n>    : HTTPS port. Default https from /etc/services\n"
				"-d        : Turns debug messages on\n"
				"-e        : Turns error messages off\n"
				"-T <n>    : Idle timeout in seconds\n"
				"-t <n>    : Sets trace level\n"
				"-s <n>    : Security level\n"
				"-v <n>    : SSL version 2=SSSV2, 3=SSLV3, otherwise SSLV23\n", argv[0]
				/*warning: too few arguments for format
				"-v <n>    : SSL version 2=SSSV2, 3=SSLV3, otherwise SSLV23\n"*/
			);
		}
	}

#if MEMWATCH
	mwInit();
	//mwAutoCheck(1);
	if (!CRYPTO_set_mem_ex_functions(
		my_ex_malloc,
		my_ex_realloc,
		my_ex_free)) {
	syslog(LOG_ALERT, "Web server could not set ssl malloc ex");
	}
#endif

/*
 *	Initialize the memory allocator. Allow use of malloc and start 
 *	with a 1 Mb heap.  For each page request approx 8KB is allocated.
 *	1 Mb allows for several concurrent page requests.  If more space
 *	is required, malloc will be used for the overflow.
 */
	bopen(NULL, (1024 * 1024), B_USE_MALLOC);

	sa.sa_handler = SIG_IGN;
	sigaction(SIGPIPE,	&sa, NULL);
//	sigaction(SIGCHLD,   &sa, NULL);

	sa.sa_handler = sig_int;
	sigaction(SIGUSRACK,  &sa, NULL);
	sigaction(SIGUSRKILL,  &sa, NULL);
	sigaction(SIGTERM,  &sa, NULL);
	sigaction(SIGINT,   &sa, NULL);
	sigaction(SIGHUP,   &sa, NULL);
#if B_STATS
	sigaction(40,   &sa, NULL);
	sigaction(41,   &sa, NULL);
	sigaction(42,   &sa, NULL);
	sigaction(43,   &sa, NULL);
#endif
	sigaction(51,   &sa, NULL);

	/* Initialize libcyc */
	while ((cyc = libcyc_init(CYC_USE_SYSLOG|(qDebug ? CYC_DEBUG : 0),
				  "AcsWeb","1.0", "/var/run",
				  "/tmp")) == FALSE) {
                sleep(1);
                secs_toquit--;
                if (secs_toquit <= 0) {
                        exit(ERROR);
                }
        }
	atexit(AcsWeb_atexit);

/*
 *	Initialize the web server
 */
	if (initWebs(httpPort) < 0) {
		return -1;
	}

#ifdef WEBS_SSL_SUPPORT
	if (httpsPort != -1) {
		if (websSSLOpen(httpsPort) < 0) {
			return(-1);
		}
	} 
#endif

	cycSetAspFunc();

/*
 *	Basic event loop. SocketReady returns true when a socket is ready for
 *	service. SocketSelect will block until an event occurs. SocketProcess
 *	will actually do the servicing.
 */
	while (!finished) {
		if (socketReady(-1) || socketSelect(-1, 1000)) {
			socketProcess(-1);
		}
		websCgiCleanup();
		emfSchedProcess();
	}


#ifdef qDmf
   dmfClose();
#endif
   
#ifdef WEBS_SSL_SUPPORT
	if (httpsPort != -1) {
		websSSLClose();
	}
#endif

#ifdef USER_MANAGEMENT_SUPPORT
	umClose();
#endif

/*
 *	Close the socket module, report memory leaks and close the memory allocator
 */
	websCloseServer();
	syslog(LOG_NOTICE, "%s: Web server stopped", TAG_DAEMON);
	socketClose();
#if B_STATS
	memLeaks();
#endif
	bclose();
#if MEMWATCH
	mwTerm();
#endif
	return 0;
}

/*
 *	Gets the local Ip address.
 */

static char *get_ip(void)
{
	char *ip_addr_str=NULL;

	if ((ip_addr_str=libcyc_iface_get_ip(cyc, "eth0")) == NULL) {
		return("127.0.0.1");
	}

	return(ip_addr_str);
}

/******************************************************************************/
/*
 *	Initialize the web server.
 */

static int initWebs(int httpPort)
{
	char			host[128], webdir[128];
	char			*ipaddr = get_ip();
#if 0
	char			*cp, dir[128];
#endif
	char_t			wbuf[128];
	struct servent  *sv;
#if 0
        struct hostent  *hp;
        struct in_addr  intaddr;
#endif

/*
 *	Initialize the socket subsystem
 */
	socketOpen();

#ifdef USER_MANAGEMENT_SUPPORT
/*
 *	Initialize the User Management database
 */
	umOpen();
	umRestore(T("umconfig.txt"));
#endif

/*
 *     Define the local Ip address, host name, default home page and the
 *     root web directory.
 */
	if (gethostname(host, sizeof(host)) < 0) {
		error(E_L, E_LOG, T("Can't get hostname"));
		strcpy(host, "localhost");
	}
#if 0
	if ((hp = gethostbyname(host)) == NULL) {
		error(E_L, E_LOG, T("Can't get host address, '127.0.0.1' assumed..."));
		intaddr.s_addr = 0x7f000001;
	} else {
		memcpy((char *) &intaddr, (char *) hp->h_addr_list[0],
			   (size_t) hp->h_length);
	}
#endif

/*
 *	Set ../web as the root web. Modify this to suit your needs
 */

#if 0
	getcwd(dir, sizeof(dir)); 
	if ((cp = strrchr(dir, '/'))) {
		*cp = '\0';
	}
	sprintf(webdir, "%s/%s", dir, rootWeb);
#else
	sprintf(webdir, "/%s", rootWeb);
#endif

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultDir(webdir);
	ascToUni(wbuf, ipaddr, min(sizeof(wbuf), strlen(ipaddr)+1));
	websSetIpaddr(wbuf);
	ascToUni(wbuf, host, min(sizeof(wbuf), strlen(host)+1));
	websSetHost(wbuf);

/*
 *	Configure the web server options before opening the web server
 */
	websSetDefaultPage(T("/login.asp"));
	websSetPassword(password);

/* 
 *	Open the web server on the given port. If that port is taken, try
 *	the next sequential port for up to "retries" attempts.
 */
	if (httpPort) {
		port = httpPort;
	} else {
		if ((sv = getservbyname("www", "tcp")) != NULL) {
			port = sv->s_port;
		}
	}

	if (websOpenServer(port, retries) < 0) {
		return(-1);
	}
	syslog(LOG_NOTICE, "%s: Web server started on port %d", TAG_DAEMON, port);

/*
 * 	First create the URL handlers. Note: handlers are called in sorted order
 *	with the longest path handler examined first. Here we define the security 
 *	handler, forms handler and the default web page handler.
 */
	websUrlHandlerDefine(T(""), NULL, 0, websSecurityHandler, 
		WEBS_HANDLER_FIRST);
	websUrlHandlerDefine(T("/goform"), NULL, 0, websFormHandler, 0);
	websUrlHandlerDefine(T("/cgi-bin"), NULL, 0, websCgiHandler, 0);
	websUrlHandlerDefine(T(""), NULL, 0, websDefaultHandler, 
		WEBS_HANDLER_LAST); 

/*
 *	Now define two test procedures. Replace these with your application
 *	relevant ASP script procedures and form functions.
 */
#if 0
	websAspDefine(T("aspTest"), aspTest);
	websFormDefine(T("formTest"), formTest);
#endif

/*
 *	Create the Form handlers for the User Management pages
 */
#ifdef USER_MANAGEMENT_SUPPORT
#ifndef qDmf
	formDefineUserMgmt();
#endif
#endif

/*
 *	Create a handler for the default home page
 */
	websUrlHandlerDefine(T("/"), NULL, 0, websHomePageHandler, 0); 

#ifdef qDmf
   return dmfInit();
#else
	return 0;
#endif

   
}

int signal_usr_flag = 0;

static void sig_int(int signo)
{
	switch (signo) {
#if B_STATS
	case 40: // the standard output must not be /dev/null 
		bstats(1, printMemStats);
		break;

	case 41: // the standard output must not be /dev/null 
		bstats1(1, printMemStats);
		break;

	case 42: // the standard output must not be /dev/null 
		bstats2(1, printMemStats);
		break;

	case 43: // the standard output must not be /dev/null 
		bstats3(1, printMemStats);
		break;
#endif

	case 51:
		// Debug stuff
#if B_STATS
		memLeaks();
#endif
#if MEMWATCH
		mwTerm();
#endif
		exit(1);
		break;
/* [EL]
	case SIGCHLD:
		wait(NULL);
		break;
*/

	case SIGUSRACK:
		signal_usr_flag |= 2;
		break;

	case SIGUSRKILL:
	case SIGHUP:
		syslog(LOG_NOTICE,"Session closed by another administrator (%d)", signo);
		signal_usr_flag |= 1;
		CycKickAdmin();
		break;

	case SIGINT:
	case SIGTERM:
	/* set the file-scope variable 'finished' to true, so our main loop
	 * will terminate, and we can clean up.
	 */
		CycKickAdmin();
		syslog(LOG_DEBUG, "%s: Caught SIGINT", TAG_DAEMON);
		finished = 1;
		break;

	default:
		syslog(LOG_NOTICE,"Receive Signal %d", signo);
		break;
	}
}

#if 0
/******************************************************************************/
/*
 *	Test Javascript binding for ASP. This will be invoked when "aspTest" is
 *	embedded in an ASP page. See web/asp.asp for usage. Set browser to 
 *	"localhost/asp.asp" to test.
 */

static int aspTest(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t	*name, *address;

	if (ejArgs(argc, argv, T("%s %s"), &name, &address) < 2) {
		websError(wp, 400, T("Insufficient args\n"));
		return -1;
	}
	return websWrite(wp, T("Name: %s, Address %s"), name, address);
}

/******************************************************************************/
/*
 *	Test form for posted data (in-memory CGI). This will be called when the
 *	form in web/asp.asp is invoked. Set browser to "localhost/asp.asp" to test.
 */

static void formTest(webs_t wp, char_t *path, char_t *query)
{
	char_t	*name, *address;

	name = websGetVar(wp, T("name"), T("Joe Smith")); 
	address = websGetVar(wp, T("address"), T("1212 Milky Way Ave.")); 

	websHeader(wp);
	websWrite(wp, T("<body><h2>Name: %s, Address: %s</h2>\n"), name, address);
	websFooter(wp);
	websDone(wp, 200);
}
#endif

/******************************************************************************/
/*
 *	Home page handler
 */

static int websHomePageHandler(webs_t wp, char_t *urlPrefix, char_t *webDir,
	int arg, char_t *url, char_t *path, char_t *query)
{
/*
 *	If the empty or "/" URL is invoked, redirect default URLs to the
 *      home page
 */
	char_t *homePage = NULL;
	char_t *p, *t;

//if (strstr(url, ".asp")) syslog(0, "websHomePageHandler URL=%s", url);

	if (*url == '\0' || gstrcmp(url, T("/")) == 0) {
		homePage = websGetDefaultPage();
		if (NULL != homePage) {
			websRedirect(wp, homePage);
		} else {
			websRedirect(wp, T("home.asp"));
		}
		return(1);
	}

	if (query && *query && (p = strstr(query, "SSID=")) != NULL) {
		if ((t = strchr(p, '&')) != NULL) *t = 0;
		p += 5;
		dmfSetParam(kSidKey, NULL, p, 0, -1);
		bfreeSafe(B_L, wp->ssid);
		wp->ssid = bstrdup(B_L, p);
		if (t) *t = '&';
		if (GetCycUserType(0) != kDmfSuccess) { /* invalid session */
			p = NULL;
		    fmtAlloc(&p, 256,
				"<html><head></head><body>\r\n"
				"<script language='JavaScript'>\r\n"
				"top.window.location.replace('%s');\r\n"
				"</script>\r\n"
				"</body></html>\r\n",
				websGetDefaultPage());
			websResponse(wp, 200, p, NULL);
		}
	}

	return(0);
}

/******************************************************************************/
/*
 *	Default error handler.  The developer should insert code to handle
 *	error messages in the desired manner.
 */

void defaultErrorHandler(int etype, char_t *msg)
{
	if (qError && msg) {
		write(2, msg, gstrlen(msg));
	}
}

/******************************************************************************/
/*
 *	Trace log. Customize this function to log trace output
 */

void defaultTraceHandler(int level, char_t *buf)
{
/*
 *	The following code would write all trace regardless of level
 *	to stdout.
 */
	if (qTrace >= level && buf) {
		write(1, buf, gstrlen(buf));
	}
}

/******************************************************************************/
/*
 *	Returns a pointer to an allocated qualified unique temporary file name.
 *	This filename must eventually be deleted with bfree();
 */

char_t *websGetCgiCommName()
{
	char_t	*pname1, *pname2;

	pname1 = tempnam(NULL, T("cgi"));
	pname2 = bstrdup(B_L, pname1);
	free(pname1);
	return pname2;
}

/******************************************************************************/
/*
 *	Launch the CGI process and return a handle to it.
 */

int websLaunchCgiProc(char_t *cgiPath, char_t **argp, char_t **envp,
					  char_t *stdIn, char_t *stdOut)
{
	int	pid, fdin, fdout, hstdin, hstdout, rc;

	fdin = fdout = hstdin = hstdout = rc = -1; 
	if ((fdin = open(stdIn, O_RDWR | O_CREAT, 0666)) < 0 ||
		(fdout = open(stdOut, O_RDWR | O_CREAT, 0666)) < 0 ||
		(hstdin = dup(0)) == -1 ||
		(hstdout = dup(1)) == -1 ||
		dup2(fdin, 0) == -1 ||
		dup2(fdout, 1) == -1) {
		goto DONE;
	}
		
syslog(LOG_INFO, "CGI: %s", cgiPath);

 	rc = pid = fork();
 	if (pid == 0) {
/*
 *		if pid == 0, then we are in the child process
 */
		if (execve(cgiPath, argp, envp) == -1) {
			printf("content-type: text/html\n\n"
				"Execution of cgi process failed\n");
		}
		exit (0);
	} 

DONE:
	if (hstdout >= 0) {
		dup2(hstdout, 1);
	}
	if (hstdin >= 0) {
		dup2(hstdin, 0);
	}
	if (fdout >= 0) {
		close(fdout);
	}
	if (fdin >= 0) {
		close(fdin);
	}
	return rc;
}

/******************************************************************************/
/*
 *	Check the CGI process.  Return 0 if it does not exist; non 0 if it does.
 */

int websCheckCgiProc(int handle)
{
/*
 *	Check to see if the CGI child process has terminated or not yet.  
 */
	if (waitpid(handle, NULL, WNOHANG) == handle) {
		return 0;
	} else {
		return 1;
	}
}

/******************************************************************************/

#if B_STATS
static void memLeaks() 
{
	int		fd;

	if ((fd = gopen(T("leak.txt"), O_CREAT | O_TRUNC | O_WRONLY, 0666)) >= 0) {
		bstats(fd, printMemStats);
		close(fd);
	}
}

/******************************************************************************/
/*
 *	Print memory usage / leaks
 */

static void printMemStats(int handle, char_t *fmt, ...)
{
	va_list		args;
	char_t		buf[BUF_MAX];

	va_start(args, fmt);
	vsprintf(buf, fmt, args);
	va_end(args);
	write(handle, buf, strlen(buf));
}
#endif

/******************************************************************************/

