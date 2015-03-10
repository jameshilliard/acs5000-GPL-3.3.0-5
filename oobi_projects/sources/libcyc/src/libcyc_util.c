/* -*- linux-c -*- */
/*****************************************************************
 * File: libcyc_util.c
 *
 * Copyright (C) 2003-2006 Cyclades Corporation
 *
 * www.cyclades.com
 *
 * Description: libcyc_util
 *
 ****************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/mman.h>

#include "md5.h"

#include <cyc.h>

struct system_str_info {
	char *str;
	int len;
};

static pid_t get_pid_from_runfile(const char *);
static void read_from_pipe(struct cyc_info *,
			   void (*data_fn)(struct cyc_info *, char *,
					   int, void *),
			   void *);
static void fill_str(struct cyc_info *, char *, int, void *);
static void system_quiet(struct cyc_info *, char *, int, void *);
static const char *parse_dev_name(const char *);
static void dev_add(struct cyc_info *, const char *, const char *, int, int);
static void dev_remove(struct cyc_info *, const char *);
static struct dev_info *dev_get_by_name(struct cyc_info *, const char *);
static struct dev_info *dev_get_by_fd(struct cyc_info *, int);
static int dev_check_open(struct cyc_info *, const char *);
                        
static void read_from_pipe(struct cyc_info *cyc, void (*data_fn)
			   (struct cyc_info *, char *, int, void *),
			   void *handle)
{                    
        FILE *stream;
	char *buf = NULL;
	ssize_t buf_sz = 0;
	size_t len = 0;

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return;
                
        close(cyc->e_pipe[1]);
        stream = fdopen(cyc->e_pipe[0], "r");

/*
	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_str[MAX_STR_LEN];
		
		snprintf(debug_str, MAX_STR_LEN,
			 "%s entering", __func__);
q		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
	}
*/
	
	while ((buf_sz = getline(&buf, &len, stream)) != -1) {
/*
		if (libcyc_get_flags(cyc) & CYC_DEBUG) {
			char debug_str[MAX_STR_LEN];

			snprintf(debug_str, MAX_STR_LEN,
				 "%s buf_sz: %d buf: %s",
				 __func__, buf_sz, buf);
			debug_str[MAX_STR_LEN-1] = '\0';
			libcyc_debug(cyc, debug_str);
		}
*/
		if (buf) data_fn(cyc, buf, len, handle);
        }

	/* Free the buffer allocated by getline */
	if (buf) {
		free(buf);
		buf = NULL;
	}

        fclose(stream);
}

int libcyc_system_pipe(struct cyc_info *cyc, char *cmd,
		       char **e_argv, void (*data_fn)
		       (struct cyc_info *, char *, int, void *),
		       void *handle)
{               
        pid_t pid;
        int status;
	char debug_str[MAX_STR_LEN];

	if (cmd == NULL) return(ERROR);
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		int i;

		snprintf(debug_str, MAX_STR_LEN, "%s begin execution of %s",
			 __func__, cmd);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		for (i = 0; e_argv[i]; i++) {
			snprintf(debug_str, MAX_STR_LEN,
				 "%s %s e_argv[%d]: %s", __func__, cmd,
				 i, e_argv[i]);
			debug_str[MAX_STR_LEN-1] = '\0';
			libcyc_debug(cyc, debug_str);	
		}
	}
        
        pipe(cyc->e_pipe);
                
        pid=fork();
        if (pid == 0) {
                /* Child process */
                close(cyc->e_pipe[0]);
                dup2(cyc->e_pipe[1],STDOUT_FILENO);
                execv(cmd,e_argv);
		/* Should never get here */
		perror("libcyc_system_pipe");
        } else {
                /* Parent process */
                waitpid(pid,&status,0);
                read_from_pipe(cyc, data_fn, handle);
                if (WIFEXITED(status)) {
                        return(WEXITSTATUS(status));
                }
        }
        
        return(ERROR);
}

static void fill_str(struct cyc_info *cyc, char *buf, int buf_sz, void *handle)
{
	struct system_str_info *strinfo = (struct system_str_info *)handle;

	if (strinfo->str == NULL) return;

        if (buf_sz < strinfo->len) strinfo->len = buf_sz;

	strncpy(strinfo->str, buf, strinfo->len);
	strinfo->str[strinfo->len - 1] = '\0';
}

int libcyc_system_str(struct cyc_info *cyc, char *cmd, char **e_argv,
		      char *str, int *len)
{
	struct system_str_info strinfo;
	int ret = ERROR;

	strinfo.str = str;
	strinfo.len = *len;

	ret = libcyc_system_pipe(cyc, cmd, e_argv, fill_str, &strinfo);

	*len = strlen(strinfo.str);

	return(ret);
}

int libcyc_system(struct cyc_info *cyc, char *cmd, char **e_argv)
{
        pid_t pid;
        int status;
	char debug_str[MAX_STR_LEN];

	if (cmd == NULL) return(ERROR);
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		int i;

		snprintf(debug_str, MAX_STR_LEN, "%s begin execution of %s",
			 __func__, cmd);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		for (i = 0; e_argv[i]; i++) {
			snprintf(debug_str, MAX_STR_LEN,
				 "%s %s e_argv[%d]: %s", __func__, cmd,
				 i, e_argv[i]);
			debug_str[MAX_STR_LEN-1] = '\0';
			libcyc_debug(cyc, debug_str);	
		}
	}

        pid=fork();
        if (pid == 0) {
                /* Child process */
                execv(cmd,e_argv);
	        /* Should never get here */
	        perror("libcyc_system");
        } else {
                /* Parent process: wait for child to complete */
                waitpid(pid,&status,0);
                if (WIFEXITED(status)) {
                        return(WEXITSTATUS(status));
                }
        }
        
        return(ERROR);
}

static void system_quiet(struct cyc_info *cyc, char *buf, int buf_sz,
			 void *handle)
{
	/* Do nothing */
}

int libcyc_system_quiet(struct cyc_info *cyc, char *cmd, char **e_argv)
{    
	return(libcyc_system_pipe(cyc, cmd, e_argv, system_quiet, NULL));
}

int libcyc_system_nowait(struct cyc_info *cyc, char *cmd, char **e_argv)
{
        pid_t pid;
	char debug_str[MAX_STR_LEN];

	if (cmd == NULL) return(ERROR);
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		int i;

		snprintf(debug_str, MAX_STR_LEN, "%s begin execution of %s",
			 __func__, cmd);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		for (i = 0; e_argv[i]; i++) {
			snprintf(debug_str, MAX_STR_LEN,
				 "%s %s e_argv[%d]: %s", __func__, cmd,
				 i, e_argv[i]);
			debug_str[MAX_STR_LEN-1] = '\0';
			libcyc_debug(cyc, debug_str);	
		}
	}

        pid=fork();
        if (pid == 0) {
                /* Child process */
                execv(cmd,e_argv);
	        /* Should never get here */
	        perror("libcyc_system_nowait");
        } else {
                /* Parent process: don't wait for child */
		return(0);
        }
        
        return(ERROR);
}

pid_t libcyc_get_pid_from_runfile(const char *filename)
{
	return(get_pid_from_runfile(filename));
}

static pid_t get_pid_from_runfile(const char *filename)
{
	FILE *fp=NULL;
	pid_t pid=ERROR;

	if (filename == NULL) return(ERROR);

	if (libcyc_file_exist(filename) == FALSE) {
		return(ERROR);
	}

	if ((fp=fopen(filename, "r")) != NULL) {
		if (fscanf(fp, "%d", (pid_t *)&pid) != 1) pid = ERROR;
		fclose(fp);
	}

	return(pid);
}

pid_t libcyc_get_daemon_pid(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(get_pid_from_runfile(cyc->lock_daemon_filepath));
}

pid_t libcyc_get_cgi_pid(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(get_pid_from_runfile(cyc->lock_cgi_filepath));
}

pid_t libcyc_get_pid(struct cyc_info *cyc)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);

	return(get_pid_from_runfile(cyc->lock_filepath));
}

char *libcyc_get_sock_filepath(struct cyc_info *cyc)
{
	return(cyc->sock_filepath);	
}

pid_t libcyc_update_pid(struct cyc_info *cyc)
{
        int fd;
	char pid_str[MAX_STR_LEN];
	pid_t pid=getpid();
	char *filepath = NULL;
	
	if ((cyc == NULL) || (pid <= 0)) return(ERROR);
	
	if (cyc->flags & CYC_DAEMON) {
		filepath = cyc->lock_daemon_filepath;
	} else if (cyc->flags & CYC_CGI) {
		filepath = cyc->lock_cgi_filepath;
	} else {
		filepath = cyc->lock_filepath;
	}

	if ((fd=open(filepath, O_CREAT|O_WRONLY, 0644)) == -1) {
		perror("libcyc_update_pid: open");
		return(ERROR);
	}

	snprintf(pid_str, MAX_STR_LEN, "%d\n", pid);
	write(fd, pid_str, strlen(pid_str));
        close(fd);

	return(pid);
}

int libcyc_file_exist(const char *filename)
{
        struct stat buf;

	if (filename == NULL) return(ERROR);

        if ((stat(filename,&buf)) == -1) {
                return(FALSE);
        }

        return(TRUE);
}

char *libcyc_create_filepath(struct cyc_info *cyc,
			     const char *dir,
			     const char *filename,
			     const char *ext)
{
	char *str;
        const int str_len = MAX_STR_LEN*3;

	if (cyc == NULL) return(NULL);

	if (dir && filename && ext) {
		str=libcyc_malloc(cyc,str_len);
	} else {
		return(NULL);
	}

	snprintf(str, str_len, "%s/%s.%s",
		 dir, filename, ext);

	/* Make sure the string is null terminated */
	str[str_len-1]='\0';

	return(str);
}

/* Returns the IP addres associated with a given interface (e.g. eth0) */
const char *libcyc_iface_get_ip(struct cyc_info *cyc, const char *ifname)
{
	struct ifreq ifr;
	struct sockaddr_in *sin;
	int fd;
	char *ip_addr_str=NULL;
	char debug_str[MAX_STR_LEN];

	if ((cyc == NULL) || (ifname == NULL)) return(NULL);
	
	if ((fd = socket(PF_INET,SOCK_DGRAM,IPPROTO_IP)) < 0) {
		perror("libcyc_iface_get_ip");
		return(NULL);
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(fd, SIOCGIFADDR, &ifr) == 0) {
		sin=(struct sockaddr_in *)&ifr.ifr_addr;
		ip_addr_str = inet_ntoa(sin->sin_addr);
	}
	close(fd);

	if (ip_addr_str == NULL) return(NULL);

	snprintf(debug_str, MAX_STR_LEN,
		 "%s iface: %s IP: %s",
		 __func__,ifr.ifr_name,ip_addr_str);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);
	
	return(ip_addr_str);
}

/* Returns the MAC address associated with a given interface (e.g. eth0) */
char *libcyc_iface_get_mac_delim(struct cyc_info *cyc,
				 const char *ifname, char delim)
{
	char debug_str[MAX_STR_LEN];
	static char mac_addr_str[64];
	int size;
	char *mac_addr = NULL;

	if ((cyc == NULL) || (ifname == NULL)) return(NULL);
	
	if (libcyc_iface_get_mac_raw(cyc, ifname, &mac_addr, &size) == ERROR) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s Could not obtain MAC address", __func__);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		return(NULL);
	}
	if (size < 6) {
		snprintf(debug_str, MAX_STR_LEN,
			 "%s MAC address string too short", __func__);
		debug_str[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_str);
		return(NULL);
	}

	snprintf(mac_addr_str, sizeof(mac_addr_str),
		 "%02X%c%02X%c%02X%c%02X%c%02X%c%02X",
		 (mac_addr[0] & 0377),
		 delim,
		 (mac_addr[1] & 0377),
		 delim,
		 (mac_addr[2] & 0377),		 
		 delim,
		 (mac_addr[3] & 0377),
		 delim,
		 (mac_addr[4] & 0377),
		 delim,
		 (mac_addr[5] & 0377));
	mac_addr_str[sizeof(mac_addr_str)-1] = '\0';

	libcyc_free(cyc, mac_addr);
	mac_addr = NULL;

	snprintf(debug_str, MAX_STR_LEN,
		 "%s iface: %s MAC addr: %s",
		 __func__, ifname, mac_addr_str);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);
	
	return(libcyc_strdup(cyc, mac_addr_str));
}

/* Returns the MAC address with the octets separated by colons */
char *libcyc_iface_get_mac(struct cyc_info *cyc, const char *ifname)
{
	return(libcyc_iface_get_mac_delim(cyc, ifname, ':'));
}

/* Returns the MAC address in raw format */
int libcyc_iface_get_mac_raw(struct cyc_info *cyc, const char *ifname,
			     char **mac_addr, int *size)
{
	struct ifreq ifr;
	int fd;
	char debug_str[MAX_STR_LEN];

	if ((cyc == NULL) || (ifname == NULL)) return(ERROR);
	
	if ((fd = socket(PF_INET,SOCK_DGRAM,IPPROTO_IP)) < 0) {
		perror("libcyc_iface_get_mac_raw");
		return(ERROR);
	}

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ-1] = '\0';
	ifr.ifr_addr.sa_family = AF_INET;
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
		perror("libcyc_iface_get_mac_raw");
		close(fd);
		return(ERROR);
	}
	close(fd);

	*size = sizeof(ifr.ifr_hwaddr.sa_data);
	if (*size < 6) return(ERROR);

	*mac_addr = libcyc_malloc(cyc, *size);
	memcpy(*mac_addr, ifr.ifr_hwaddr.sa_data, *size);

	snprintf(debug_str, MAX_STR_LEN,
		 "%s iface: %s size: %d MAC (raw): %02x%02x%02x%02x%02x%02x",
		 __func__, ifname, *size,
		 ((*mac_addr)[0] & 0377),
		 ((*mac_addr)[1] & 0377),
		 ((*mac_addr)[2] & 0377),
		 ((*mac_addr)[3] & 0377),
		 ((*mac_addr)[4] & 0377),
		 ((*mac_addr)[5] & 0377));
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	return(0);
}

char *libcyc_get_lockfile(struct cyc_info *cyc,
			  enum libcyc_flags flags)
{
	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(NULL);

	if (CYC_CLEAR || (flags & CYC_CLIENT)) {
		return(cyc->lock_filepath);
	} else if ((flags & CYC_SERVER) || (flags & CYC_DAEMON)) {
		return(cyc->lock_daemon_filepath);
	} else if (flags & CYC_CGI) {
		return(cyc->lock_cgi_filepath);
	} else {
		return(cyc->lock_filepath);
	}
}

double libcyc_get_uptime(void)
{
	FILE *fp=NULL;
	double uptime=ERROR, idletime=ERROR;
	char *filename = PROC_UPTIME_FILE;

	if (libcyc_file_exist(filename) == FALSE) {
		return(ERROR);
	}

	if ((fp=fopen(filename,"r")) == NULL) return(ERROR);
	
	fscanf(fp,"%le %le\n", &uptime, &idletime);
	fclose(fp);

	return(uptime);
}

long libcyc_get_uptime_ms(void)
{
	return((long)(libcyc_get_uptime() * 1000));
}

/* Device open/close implementation with locking */
static const char *parse_dev_name(const char *filepath)
{
        char *name = NULL;
	
	if (filepath == NULL) return(NULL);

        name = strrchr(filepath, '/');
	name++;

        return(name);
}

static void dev_add(struct cyc_info *cyc, const char *name,
		    const char *dev_filepath, int fd, int semid)
{
	struct dev_info *dev = NULL;

	if ((cyc == NULL) || (name == NULL)) return;

	if (libcyc_file_exist(dev_filepath) == FALSE) return;

	dev = libcyc_malloc(cyc, sizeof(struct dev_info));

	strncpy(dev->name, name, MAX_STR_LEN);
	dev->name[MAX_STR_LEN-1] = '\0';
	strncpy(dev->dev_filepath, dev_filepath, MAX_STR_LEN);
	dev->dev_filepath[MAX_STR_LEN-1] = '\0';
	dev->fd = fd;
	dev->semid = semid;

	cyc->devs = libcyc_ll_append(cyc->devs, dev);
}

static void dev_remove(struct cyc_info *cyc, const char *name)
{
	struct dev_info *dev = NULL;
	struct dev_info *tmp = NULL;

	if ((cyc == NULL) || (name == NULL)) return;

	while ((tmp = libcyc_ll_iterate(cyc->devs))) {
		if (strncmp(tmp->name, name, MAX_STR_LEN) == 0) dev = tmp;
	}

	if (dev) {
		cyc->devs = libcyc_ll_remove(cyc->devs, dev);
		libcyc_free(cyc, dev);
	}
}

static struct dev_info *dev_get_by_name(struct cyc_info *cyc, const char *name)
{
	struct dev_info *dev = NULL;
	struct dev_info *tmp = NULL;

	if ((cyc == NULL) || (name == NULL)) return(NULL);

	while ((tmp = libcyc_ll_iterate(cyc->devs))) {
		if (strncmp(tmp->name, name, MAX_STR_LEN) == 0) dev = tmp;
	}

	return(dev);
}

static struct dev_info *dev_get_by_fd(struct cyc_info *cyc, int fd)
{
	struct dev_info *dev = NULL;
	struct dev_info *tmp = NULL;

	if (cyc == NULL) return(NULL);

	while ((tmp = libcyc_ll_iterate(cyc->devs))) {
		if (tmp->fd == fd) dev = tmp;
	}

	return(dev);
}

static int dev_check_open(struct cyc_info *cyc, const char *name)
{
	struct dev_info *dev = NULL;

	if ((cyc == NULL) || (name == NULL)) return(ERROR);

	if ((dev = dev_get_by_name(cyc, name))) {
		return(dev->fd);
	}

	return(ERROR);
}

int libcyc_open_dev(struct cyc_info *cyc, const char *devfile, int flags)
{
	int fd;
	const char *name = NULL;
	int semid = 0;
	int semkey = 0;

	if ((name = parse_dev_name(devfile)) == NULL) return(ERROR);

/*
	if (libcyc_get_flags(cyc) & CYC_DEBUG) {
		char debug_msg[MAX_STR_LEN];

		snprintf(debug_msg, MAX_STR_LEN,
			 "%s devfile: %s flags: %d name: %s",
			 __func__, devfile, flags, name);
		debug_msg[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_msg);
	}
*/

	if ((fd = dev_check_open(cyc, name)) != ERROR) return(fd);

	/* Sem code so two apps don't try to open a device file at the
	   same time; we don't remove this sem since we don't know how
	   many apps will connect to it.  Sems don't take much
	   space, so we'll let it hang around */

	/* Get the sem key -- it's the same for every devfile */
	semkey = libcyc_ipc_key_get(cyc, devfile, 0);
	/* This is a new sem, initialize it */
	if ((semid = libcyc_ipc_sem_get(cyc, semkey, 1, IPC_CREAT|IPC_EXCL)) != ERROR) {
		libcyc_ipc_sem_set_all(cyc, semid, 0);
	} else {
		/* Attach to existing sem */
		if ((semid = libcyc_ipc_sem_get(cyc, semkey, 1, 0)) == ERROR) {
			return(ERROR);
		}
		
		/* Wait up to 10 seconds to try to get the sem */
		if (libcyc_ipc_sem_timed_op(cyc, semid, 0, -1, 10000) == ERROR) {
			return(ERROR);
		}
	}
	
	if ((fd = open(devfile, flags)) == -1) {
		perror("libcyc_open_dev open");
		return(ERROR);
	}

	dev_add(cyc, name, devfile, fd, semid);

	return(fd);
}

void libcyc_close_dev(struct cyc_info *cyc, int fd)
{
	struct dev_info *dev = NULL;

	if (cyc == NULL) return;
	if (libcyc_get_flags(cyc) & CYC_DEV_PERSIST) return;

	if ((dev = dev_get_by_fd(cyc, fd))) {
		close(fd);
		dev_remove(cyc, dev->name);
		libcyc_ipc_sem_op(cyc, dev->semid, 0, 1);
	}
}

/* Simple lockfile implementation (lock/unlock/status) */
int libcyc_lock(struct cyc_info *cyc, const char *lock_id)
{
        int fd;
	char lock_filepath[MAX_STR_LEN];

	if ((cyc == NULL) || (! (cyc->flags & CYC_INIT))) return(ERROR);
	if (lock_id == NULL) return(ERROR);
	
	snprintf(lock_filepath, MAX_STR_LEN, "%s/%s", cyc->tmpdir, lock_id);
	lock_filepath[MAX_STR_LEN-1] = '\0';

	if ((fd = open(lock_filepath, O_CREAT|O_EXCL|O_WRONLY, 0644)) == -1) {
		char debug_msg[MAX_STR_LEN];

		snprintf(debug_msg, MAX_STR_LEN, "%s appname: %s can't obtain lock",
		         __func__, cyc->appname);
		debug_msg[MAX_STR_LEN-1] = '\0';
		libcyc_debug(cyc, debug_msg);

		return(ERROR);
	}
	
	write(fd, &fd, sizeof(int));
        close(fd);

	return(0);
}

int libcyc_unlock(struct cyc_info *cyc, const char *lock_id)
{
	char lock_filepath[MAX_STR_LEN];

	if (cyc == NULL) return(ERROR);
	if (lock_id == NULL) return(ERROR);

	snprintf(lock_filepath, MAX_STR_LEN, "%s/%s", cyc->tmpdir, lock_id);
	lock_filepath[MAX_STR_LEN-1] = '\0';

	if (unlink(lock_filepath) == -1) {
		return(ERROR);
	}
	
	return(0);
}

int libcyc_lock_status(struct cyc_info *cyc, const char *lock_id)
{
	char lock_filepath[MAX_STR_LEN];

	snprintf(lock_filepath, MAX_STR_LEN, "%s/%s", cyc->tmpdir, lock_id);
	lock_filepath[MAX_STR_LEN-1] = '\0';

	return(libcyc_file_exist(lock_filepath));
}

/* This function pings the host given as an argument and returns
   TRUE if the host is alive and FALSE if the host is dead */
int libcyc_ping(struct cyc_info *cyc, const char *ip_str)
{
	/* Command for busybox ping */
	char *p_argv[] = { "ping", "-q", "-c", "1", (char *)ip_str, NULL };
	/* Command for a "real" ping
	char *p_argv[] = { "ping", "-q", "-c", "1", "-w", "5", NULL }; */
	int ret = FALSE;
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return(ERROR);
	if (ip_str == NULL) return(ERROR);

	if (libcyc_system_quiet(cyc, PING_COMMAND, p_argv) == 0) ret = TRUE;

	snprintf(debug_str, MAX_STR_LEN, "%s host %s is %s",
		 __func__, ip_str, ret ? "alive" : "dead");
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	return(ret);
}

char *libcyc_md5sum_hexstr_get(struct cyc_info *cyc, const char *filename)
{
	md5_state_t state;
	md5_byte_t digest[MD5_DIGEST_LEN];
	char hex_output[MD5_DIGEST_LEN*2 + 1];
	int di;
	struct stat fileinfo;
	char *filebuf = NULL;
	int fd;
	char debug_str[MAX_STR_LEN];

	if (cyc == NULL) return(NULL);
	if (filename == NULL) return(NULL);

        if ((stat(filename, &fileinfo)) == -1) {
		perror("libcyc_md5_hexstr_get");
		return(NULL);
        }

	if ((fd = open(filename, O_RDONLY)) == -1) {
		perror("libcyc_md5_hexstr_get");
		return(NULL);
	}

	filebuf = mmap((void *)0, fileinfo.st_size, PROT_READ,
		       MAP_SHARED, fd, 0);
	if (filebuf == MAP_FAILED) {
		perror("libcyc_md5_hexstr_get");
		close(fd);
		return(NULL);
	}

	md5_init(&state);
	md5_append(&state, (const md5_byte_t *)filebuf, fileinfo.st_size);
	md5_finish(&state, digest);

	for (di = 0; di < MD5_DIGEST_LEN; ++di) {
		sprintf(hex_output + (di*2), "%02x", digest[di]);
	}

	munmap(filebuf, fileinfo.st_size);
	close(fd);

	snprintf(debug_str, MAX_STR_LEN, "%s filename: %s md5sum: %s",
		 __func__, filename, hex_output);
	debug_str[MAX_STR_LEN-1] = '\0';
	libcyc_debug(cyc, debug_str);

	return(libcyc_strdup(cyc, hex_output));
}

int libcyc_update_list(struct cyc_info *cyc, const char *conf_file,
		       char **md5sum_ptr,
		       void (*parse_cmd)(struct cyc_info *, libcyc_list **),
		       libcyc_list **data)
{
	char *md5sum = NULL;

	if (cyc == NULL) return(ERROR);
	if (conf_file == NULL) return(ERROR);
	if (md5sum_ptr == NULL) return(ERROR);
	if (parse_cmd == NULL) return(ERROR);
	if (data == NULL) return(ERROR);
	
	if (libcyc_get_flags(cyc) & CYC_CONF_NOUPDATE) return(0);
	
	md5sum = libcyc_md5sum_hexstr_get(cyc, conf_file);
	if (md5sum == NULL) return(ERROR);
	
	/* First time: no checksum and no data */
	if ((*md5sum_ptr == NULL) || (*data == NULL)) {
		parse_cmd(cyc, data);
		*md5sum_ptr = md5sum;
	}
	
	/* If the memcmp fails, we have an out-of-date checksum;
	   re-read the conf file */
	if (memcmp(*md5sum_ptr, md5sum, MD5SUM_HEXSTR_LEN) != 0) {
		parse_cmd(cyc, data);
		/* Free the old md5sum string */
		libcyc_free(cyc, *md5sum_ptr);
		/* Assign the new md5sum string */
		*md5sum_ptr = md5sum;
	}

	return(0);
}
