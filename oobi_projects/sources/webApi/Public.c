/*****************************************************************
 * File: public.c
 *
 * Copyright (C) 2003 Cyclades Corporation
 *
 * Maintainer: www.cyclades.com
 *
 * Description: This file contains generic functions that can be 
 * called by anyone.             
 *
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <src/server.h>
#include <sys/types.h>
#include <fcntl.h>
#include "AITypes.h"
#include <webApi.h>
#include "WebApi2.h"
#include <assert.h>
#define XOR_ENCRYPT  "ioHrEw(L#yZfjsfSdfHfSgGjMjq;Ss;d)(*&^#@$avsig"

#define STDIN_FD  0
#define STDOUT_FD 1


#define RDEND 0
#define WREND 1

const int FW_LINE_SIZE=128;

#ifdef KVM
    char flashDir[] = "proc";
#else
    char flashDir[] = "mnt";
#endif

/* Code used in the appliance image to identify the product.
 * This code is used to validate the image downloaded to the
 * flash memory. We will only accept the right product image
 * to be downloaded to the image.
 * Obs: A blank product code means that this product will not
 * do the validation.
 */
#if   defined(KVM)
	const char prdCode[] = "kv";
#elif defined(ONS)
	const char prdCode[] = "";
#elif defined(TARGET_AVCS)
	const char prdCode[] = "as";
#elif defined(TARGET_ACS5K)
	const char prdCode[] = "a5";
#else
	const char prdCode[] = "1k";
#endif

/*************************************************************************
   Description:
   	Check if firmware image downloaded is the right one.

   Parameters:
   	fileName: full path to the firmware image file.

   Return:
   	 0 = OK, this is the right firmware image for the product.
   	 1 = Wrong firmware image.
   	-1 = Error opening the file.
   	-2 = Error seeking file to the product code position.
   	-3 = Error reading product code from file.
*************************************************************************/
int checkFwImage(char *fileName)
{
	FILE *fp;
	int ret = 1;
	char buf[3];

	/* verify if need to check firmware image for this product */
	if (!strlen(prdCode)) return(0);

	/* open firmware image file */
	if ((fp = fopen(fileName, "r")) == NULL) return(-1);

	/* seek to the product code position */
	if (fseek(fp, 0x1A, SEEK_SET) == -1) return(-2);

	/* read product code */
	if (fread(buf, 1, 2, fp) != 2) return(-3);
	buf[2] = 0;

	/* compares product code */
	if (!strncmp(prdCode, buf, 2)) ret = 0;

	/* close file */
	fclose(fp);

	/* return the result */
	return(ret);
}

char currentAdmin[kUserNameLength];
// ParseLineComArg - Routine for parsing the most common 
// configuration file syntax (host.conf, resolv.conf, cyras.conf).
// The syntax of a configuration line is: 
// <parameter name> <parameter value> 
//========================================================================
int ParseLineComArg(unsigned char *buf, unsigned char **argument)
{
    unsigned char *a, *s;

    s=buf;
    if (*s==0 || isspace(*s)) return 0;
    while (*s && !isspace(*s)) s++;
    if (*s) *s++=0;
    while (isspace(*s)) s++;
    if (*s==0) return 0;
    a=s;
    while (*s) s++;
    while (*s!=0 || isspace(*s)) s--;
    if (*s) *(s+1)=0;
    *argument=a;
    return 1;
}

//========================================================================
// ParseLinePslave - Parses line. The syntax of a configuration line is:
// <Word>.<Command> <Argument>
// Lines which start with # or empty are disregarded; lines which end 
// with "\" will be linked with the next line.
//========================================================================
int ParseLinePslave (unsigned char *buf, unsigned char **word,
                            unsigned char **command, unsigned char **argument)
{
	unsigned char *p, *s, *w, *c, *a;
	p = buf;

	//if ((p[0] == '#') || p[0] == '\n') return 0;
	//[RK]Jun/19/06 - fixed problem with DSView
	if ((p[0] == '#' && (p[1] =='#' || isspace(p[1]) || p[1] == 0)) || p[0] == '\n') return 0;
	//printf("ParseLinePslave(): P: %s\n", p);
	s = p + strlen(p);
	//printf("ParseLinePslave(): S: %s\n", s);
	if (s > p && *(s-1) == '\\') 
	{
		//printf("ParseLinePslave(): Returning 2...\n");
		return 2; //keep adding the line
	}
	if (buf[0] == '\n' || buf[0] == 0) 
	{
		//printf("ParseLinePslave(): 2 Returning 0...\n");
		return 0;
	}
	// Parse Line - w=word c=command a=argument 
	s = buf;
	for (w = s; *w; w++);
	while (--w > s && *w == '\n') *w = 0;
	while isspace (*s) s++;
	w = s;
	//printf("ParseLinePslave(): W: %s\n", w);
	while (*s && !isspace(*s)) s++;
	if (*w==0) return 0;
	if (*s) *s++=0;
	while (isspace(*s)) s++;
	a = s;
	if ((c = strchr(w,'.')) == NULL || c[1] == 0) return 0;
	*c++ = 0;
	*word=w;
	*command=c;
	*argument=a;
	//printf("ParseLinePslave(): W: [%s], C: [%s]\n", w, c);
	return 1;
}

//========================================================================
// ParseLineSnmpd - Parses line. The syntax of a configuration line is:
// <Word>  <Argument>
// Lines which start with # and then there is a spcae afterwards, or if
// the line is empty are disregarded.
//========================================================================
int ParseLineSnmpd(unsigned char *buf, unsigned char **word,
                            unsigned char **argument)
{
	unsigned char *p, *s, *w, *a;
	p = buf;

	//if ((p[0] == '#') || p[0] == '\n') return 0;
	if ((p[0] == '#' && (isspace(p[1]) || p[1] == 0)) || p[0] == '\n') return 0;
	s = p + strlen(p);
	if (buf[0] == '\n' || buf[0] == 0) 
	{
		return 0;
	}
	s = buf;
	while (isspace(*s)) s++;
	w = s;
	//printf("ParseLinePslave(): W: %s\n", w);
	while (*s && !isspace(*s)) s++;
	if (*w==0) return 0;
	if (*s) *s++=0;
	while (isspace(*s)) s++;
	a = s;
	*word=w;
	*argument=a;
	//printf("ParseLineSnmpd(): W: [%s], C: [%s]\n", w, a);
	return 1;
}

//========================================================================
// ParseLineNis - Parses line. The syntax of a configuration line is:
// <Word>:  <Argument>
// Lines which start with # or empty are disregarded.
//========================================================================
int ParseLineNis(unsigned char *buf, unsigned char **word,
                            unsigned char **argument)
{
	unsigned char *p, *s, *w, *a, *c;
	p = buf;

	if ((p[0] == '#') || p[0] == '\n') return 0;
	s = p + strlen(p);
	if (buf[0] == '\n' || buf[0] == 0) 
	{
		return 0;
	}
	s = buf;
	while (isspace(*s)) s++;
  w = s;
  while (*s && !isspace(*s)) s++;
  if (*w==0) return 0;
  if (*s) *s++=0;
  //printf("ParseLineNis(): W: %s\n", w);
  a = s;
	while(*s && !isspace(*s)) s++;
  if ((c = strchr(w,':')) != NULL)
		*c=0;
  *word=w;
  *argument=a;
  //printf("ParseLineNis(): W: [%s], C: [%s]\n", w, a);
  return 1;
}

//========================================================================
// ParseLineLoginConfig - Parses line. The syntax of a configuration line 
// is: <username> <userid> <utmp_entry> <login_program> [arguments]"
// Lines which start with # or empty are disregarded.
//========================================================================
int ParseLineLoginConfig(unsigned char *buf, unsigned char **username,
	unsigned char **userid, unsigned char **utmpEntry, 
	unsigned char **loginPrgm, unsigned char **arguments)
{
	unsigned char *p, *s;
	unsigned char *name, *id, *utmp, *prog, *arg;
	p = buf;

	if ((p[0] == '#') || p[0] == '\n') return 0;
	s = p + strlen(p);
	if (buf[0] == '\n' || buf[0] == 0) 
	{
		return 0;
	}
	s = buf;
	while (isspace(*s)) s++;
  name = s;
  while (*s && !isspace(*s)) s++;
  if (*name==0) return 0;
  if (*s) *s++=0;
  //printf("ParseLineLoginConfig(): Name: %s\n", name);
	if (*s==0) return 0;

	while (*s && isspace(*s)) s++;	
  id = s;
  while (*s && !isspace(*s)) s++;
  if (*id==0) return 0;
	if (*s) *s++=0;

	while (*s && isspace(*s)) s++;	
  utmp = s;
  while (*s && !isspace(*s)) s++;
  if (*utmp==0) return 0;
	if (*s) *s++=0;

	while (*s && isspace(*s)) s++;	
  prog = s;
  while (*s && !isspace(*s)) s++;
  if (*prog==0) return 0;
	if (*s) *s++=0;

	while (*s && isspace(*s)) s++;	
  arg = s;
  *username=name;
  *userid=id;
  *utmpEntry=utmp;
  *loginPrgm=prog;
	*arguments=arg;
  //printf("ParseLineWireless(): W: [%s], C: [%s]\n", w, a);
  return 1;
}

//========================================================================
// PrintLinePslave - Prints the [word.command  argument] to pslave.conf.
//========================================================================
void PrintLinePslave (FILE *fp, unsigned char *w, unsigned char *c, unsigned char *a)
{
  unsigned char *s;

	if (*a) 
	{
		for (s=a; *s; s++) 
		{
			if (*s=='\r') *s='\\';
		}
		fprintf (fp, "%s.%s\t%s\n", w, c, a);
	}
}

//========================================================================
// Min - Returns the minimum number
//========================================================================
int Min(int x, int y)
{
  if(x<y){
    return x;
  }
  return y;
}

void LogError(char *fmt, ...)
{
	FILE *logfile;
  va_list ap;

  va_start(ap, fmt);

	logfile=fopen(ERROR_LOG_FILE, "a");
  vfprintf(logfile, fmt, ap);

  va_end(ap);
	if (fmt[0] != '\0' && fmt[strlen(fmt)-1] == ':')
		fprintf(logfile, " %s", strerror(errno));
	fprintf(logfile, "\n");
	fclose(logfile);
}

char *read_file_return_buf(char *fn)
{
   struct stat st;
   int fd;
   char *buf;

   // we will first read the file in a buffer
   if (stat(fn, &st)) {
      return(NULL);
   }

   if ((buf = malloc(st.st_size+1)) == NULL) {
      return(NULL);
   }
   if ((fd = open (fn, O_RDONLY)) < 0) {
      free(buf);
      return(NULL);
   }

   if (read(fd, buf, st.st_size) != st.st_size) {
      free(buf);
      close(fd);
      return(NULL);
   }

   *(buf+st.st_size) = 0;
   close(fd);
   return(buf);
}


//========================================================================
// retrieve_fw_size - returns the firmware file size in buf
//========================================================================
long retrieve_fw_size(char buf[FW_LINE_SIZE]) 
{
    long size = 0L;

    size = atol(&buf[4]);
    
    return(size);
}

//========================================================================
// check_buffer_size - writes the linebuffer to file when it is full
//========================================================================

void check_buffer_size(char linebuffer[FW_LINE_SIZE],int fd) 
{
    if (strlen(linebuffer) == FW_LINE_SIZE) {
        write(fd,linebuffer,strlen(linebuffer));
        strcpy(linebuffer,"");
    }

}


//========================================================================
// monitor_ftp - write the ftp script to ftp STDIN and 
// read ftp STDOUT to read the number of hashes
//========================================================================
int monitor_ftp(pid_t child_pid, int stout, int stin, long int ramSize,
		int *result)
{
    char INITFTPMESSAGE[] = "150 ";
    char SIZEFTPMESSAGE[] = "213 ";
    char *msg;
    char buf[1];
    char buf_num[8];
    char linebuffer[FW_LINE_SIZE];

    int ftp_status_file;
    int hashcount=0;
    int finished =0;
    int previous_download_ratio=0;
    long int fw_size=0;

    float download_ratio=0;
    int ftplogfile;
    int flags, status;
    int ftpactions;
    ssize_t rdcnt;

    int ftpStarted = 0, ftpExited = 0;
    int countNoFTPMessages = 0;
    const int RX_TIMEOUT = 15;

    /* Put stin in non-blocking mode */
    flags = fcntl(stin, F_GETFL);
    fcntl(stin, F_SETFL, flags | O_NONBLOCK);

    /* Put stdin in non-blocking mode */
    flags = fcntl(STDIN_FD, F_GETFL);
    fcntl(STDIN_FD, F_SETFL, flags | O_NONBLOCK);

    /* reset linebuffer and buf variables */
    bzero(linebuffer,sizeof(linebuffer));
    bzero(buf,sizeof(buf));

    if ((ftp_status_file = open("/tmp/fwup",O_WRONLY | O_CREAT | O_TRUNC,0644 )) < 0) {
		LogError("cannot open /tmp/fwlog file");
		return(ftpExited);
    }

    if ((ftplogfile = open("/tmp/webtemp2",O_WRONLY | O_CREAT | O_TRUNC,0644 )) < 0) {
		LogError("cannot open webtemp2 file");
		return(ftpExited);
    }

    if ((ftpactions = open("/tmp/webtemp",O_RDONLY)) < 0) {
		LogError("cannot open /tmp/webtemp file.");
		return(ftpExited);
    }

    // write webtemp to ftp stdin pipe
    for(;;) {

        rdcnt =  read(ftpactions,linebuffer,sizeof(linebuffer));

        if (rdcnt == -1) {
		  LogError("cannot read ftpactions file");

        } else if (rdcnt==0) {
            break;

        } else {
            write(stout,linebuffer,sizeof(linebuffer));
        }
    }

    close(ftpactions);
    strcpy(linebuffer,"");

    // read ftp messages
    for (;;) {

	/* read one byte from ftp pipe */
        rdcnt = read(stin, buf, sizeof(buf));

	/* error reading from ftp input */
        if(rdcnt == -1) {

		/* stop reading status from ftp pipe */
        	if(errno != EAGAIN) {
	        	LogError("Read error from firmware ftp pipe.");
		        break;
	        }

		/* count qtd of errors and wait a second for next read */
		++countNoFTPMessages;
		sleep(1);

        } else if (rdcnt == 0) {

		/* return if ftp process has finished */
        	if (ftpExited == 1) break;

		/* count qtd of errors and wait a second for next read */
		++countNoFTPMessages;
		sleep(1);

        } else {

		/* clean qtd of errors */
		countNoFTPMessages = 0;

		/* wait for a line to process */
		if (*buf != '\n') {
			strncat(linebuffer, buf, sizeof(buf));
			check_buffer_size(linebuffer, ftplogfile);

		/* received a line, so process */
		} else {
			strncat(linebuffer, buf, sizeof(buf));
			write(ftplogfile, linebuffer, strlen(linebuffer));
          
			if ((msg = strstr(linebuffer, SIZEFTPMESSAGE)) != NULL) {
        	        	fw_size = retrieve_fw_size(linebuffer);
				if (((fw_size / 1000) + 1024) > ramSize) {
					LogError("No space in RAM for firmware upgrade.");
					*result = -3;
					break;
				}
			}

                        if ((msg = strstr(linebuffer, INITFTPMESSAGE)) != NULL) {
        	        	ftpStarted = 1;
			}
			
		        strcpy(linebuffer, "");
	        }

        	if (fw_size && ftpStarted) {
	        // start counting hashes

        		if (*buf == '#') {
		        	hashcount++;
				
				if((fw_size - hashcount * 1024) < 1024) {
        		        	finished = 1;
					*result = 0;
		                	download_ratio=100;
	                	} else {
			                download_ratio= ((float)hashcount/fw_size) * 1024 * 100;
                		}

				if (previous_download_ratio != (int) download_ratio) {
					previous_download_ratio = download_ratio;
			                sprintf(buf_num,"%03.0f",download_ratio);
        	        		lseek(ftp_status_file,0,SEEK_SET);
                	    		write(ftp_status_file,buf_num,3);
			                sprintf(buf_num,"%d0",finished);
        	        		write(ftp_status_file,buf_num,2);
	                	}
            		}
        	}
    	}

	/* check if ftp process has already stopped */
	if(waitpid(child_pid, &status, WNOHANG) > 0) {
		ftpExited = 1;
	}

	/* check rx timeout */
	if (countNoFTPMessages > RX_TIMEOUT) break;
    }

    /* close files */ 
    close(ftplogfile);
    close(ftp_status_file);

    /* return */
    return(ftpExited);
}

//========================================================================
// get ram space available for firmware upgrade
//========================================================================
long int getRamSpaceAvail()
{
	long int ramSize = 0;
	int error = 0, qtd, fp;
	char buf[20];
	
	// read ram space available and save in a file
	system ("df -k | awk '/ram0/ { print $4}' > /tmp/space.tmp");

	// open a file for reading
	if ((fp = open("/tmp/space.tmp", O_RDONLY)) == NULL) {
		error = 1;
	}
	else {
		if ((qtd = read(fp, buf, sizeof(buf) - 1)) <= 0) {
			error = 2;
		}
		else {
			buf[qtd] = '\0';
			if ((ramSize = atol(buf)) <= 0)
				error = 3;
		}
		
	}

	if (error) {
		LogError("Error reading RAM space avaliable (%d)\n", error);
	}

	// release resources
	close(fp);
	remove("/tmp/space.tmp");

	return(ramSize);
}


//========================================================================
// execute_firmware_ftp - forks a child and execute firmware ftp
//
// Returns:
//	 0 = FTP process executed (need to check FTP result)
//	-1 = error trying to execute FTP
//	-2 = error reading ram space
//	-3 = no space in RAM for firmware upgrade
//========================================================================
int execute_firmware_ftp(char *ftpSite)
{
	long int ramSize = 0;
	pid_t child_pid;
	int result = -1;	

	int p0[2];
	int p1[2];

    	assert(pipe(p0) == 0);
    	assert(pipe(p1) == 0);
                               
	/* get space avail in RAM */       
	if ((ramSize = getRamSpaceAvail()) <= 0) {
		LogError("RAM size = %ld", ramSize);
		return(-2);
	}

	switch(child_pid = fork()) {

		/* error forking child process */
		case -1: 
			LogError("Cannot fork child for firmware upgrade.");	
		  	return (1);
    	  
		/* child process */
		case 0:
			close(p0[WREND]);
			close(p1[RDEND]);

			dup2(p0[RDEND], 0);  /* Replace stdin */
			dup2(p1[WREND], 1);  /* Replace stdout */
			dup2(p1[WREND], 2);  /* Replace stderr */

			close(p0[RDEND]);
			close(p1[WREND]);
			close(p1[WREND]);

			execl("/usr/bin/ftp","ftp","-nv",ftpSite,(char *)NULL);

			break;
    
		/* parent process */
		default:
			close(p0[RDEND]);
			close(p1[WREND]);
			if (!monitor_ftp(child_pid, p0[WREND], p1[RDEND],
					ramSize, &result)) 
				kill(child_pid, SIGINT);
			break;
	}
	
	return(result);
}


//========================================================================
// UpGradeFirmware - Immediate API that upgrades the firmware
//========================================================================
int UpGradeFirmware(UpGradeInfo *pUpgrade)
{
	FILE *fp;
	unsigned char buffer[200];
	int ret;
	int transfer = 0;
	int abort = 0;
	int login = 0;
        int remote_file = 0;  //[GY]2006/Jun/14  BUG#7323
        int service_unavail = 0;  //[GY]2006/Jun/20  BUG#7323
        char copyCmd[50];
        int ftp_status_file, i;
        
#ifdef UBOOT_PARAM
	unsigned char *s;
#else
	char s[100], t[100];
	unsigned char *pS, *pT, *strS, *strT;
#endif

	dolog("UpGradeFirmware()");
        
	if (*(pUpgrade->ftpSite) == 0) return(kFileTransferredFail);

	if ((fp = fopen("/tmp/webtemp", "w")) == NULL) {
		strcpy(pUpgrade->sysMsg, "FTP command error.");
		return(kFileTransferredFail);
	}

	// remove temp files
	remove("/tmp/fwup");
	remove("/tmp/zImage");
	remove("/tmp/temp.md5");
	
	fprintf(fp, "user %s %s\nbin\n", pUpgrade->userName, pUpgrade->passWd);
	fprintf(fp, "hash\n");
	fprintf(fp, "size %s\n", pUpgrade->filePathName);
	
#ifdef UBOOT_PARAM
	fprintf(fp, "get %s /tmp/uImage\n", pUpgrade->filePathName);
#else
	fprintf(fp, "get %s /tmp/zImage\n", pUpgrade->filePathName);
	if (pUpgrade->checkSum == kYes) {
		fprintf(fp, "get %s.md5 /tmp/temp.md5\n", pUpgrade->filePathName);
	}
#endif
	fprintf(fp, "quit\n");  //[GY] 2005/Sep/21  BUG fix 4728
	fclose(fp);
	chmod("/tmp/webtemp", 0600);

	/* execute firmware upgrade */
	ret = execute_firmware_ftp(pUpgrade->ftpSite);

	/* check results */
	switch(ret) {

		/* ftp command executed ok, check results later */
		case 0:
			break;

		/* error executing ftp command or error reading ram space */
		case -1:
		case -2:
			remove("/tmp/zImage");
			dolog("File was not successfully transferred.");
			LogError("File %s was unsuccessfully transferred.", pUpgrade->filePathName);
			strcpy(pUpgrade->sysMsg, "Transfer Error: Please check "ERROR_LOG_FILE" file for details.");
			return(kFileTransferredFail);
			break;

		/* no space in ram for upgrade */
		case -3:
			remove("/tmp/zImage");
			dolog("File was not successfully transferred.");
			LogError("File %s was unsuccessfully transferred.", pUpgrade->filePathName);
			strcpy(pUpgrade->sysMsg, "Error: No space in RAM for firmware upgrade.");
			return(kFileRamSpaceError);
			break;
	}

	if ((fp = fopen("/tmp/webtemp2", "r")) != NULL) {
		
		while (fgets(buffer, sizeof(buffer),fp)) {
			
			if (strncmp(buffer,"530", 3) == 0) {  //Login incorrect.
				login = 1;
				break;
			}
			
			if (strncmp(buffer,"426", 3) == 0) { //Transfer aborted.
				abort = 1;
				break;
			}
			
                        if( strncmp(buffer, "550", 3) == 0 ) { //No such file or directory
                                remote_file = 1;
				break;
                        }
                        
                        if( strncmp(buffer, "421", 3) == 0 ) { //Service not available
                                service_unavail = 1;
				break;
                        }

			if (strncmp(buffer,"226", 3) == 0) { //Transfer complete.
				transfer = 1;
				break;
			}
		}
		
		fclose(fp);
	}
        
	/* Comment this out if you want to debug and see 
	   what actually got printed to the file */
	unlink("/tmp/webtemp2"); 
	unlink("/tmp/webtemp");

	if (transfer == 0) {
		if (abort == 1) {
			dolog("File was not successfully transferred.");
			LogError("File, %s, was unsuccessfully transferred.", pUpgrade->filePathName);	
			LogError("FTP result:\n%s", buffer);
			strcpy(pUpgrade->sysMsg, "Transfer Error: Please check "ERROR_LOG_FILE" file for details.");
			return(kFileTransferredFail);
		} else if (login == 1) {
			dolog("FTP login incorrect.");
			LogError("Login Incorrect %s.", pUpgrade->userName);	
			LogError("FTP result:\n%s", buffer);
			strcpy(pUpgrade->sysMsg, "Login Incorrect.");
			return(kLoginFailed);
                //[GY]2006/Jun/14  BUG#7323
                } else if( remote_file == 1 ) {
                        dolog("No such file or directory.");
                        LogError("File/directory %s wasn't found.", pUpgrade->filePathName);
                        LogError("FTP result:\n%s", buffer);
                        strcpy(pUpgrade->sysMsg, "No such file or directory.");
                        return(kFileDirUnavail);
                //[GY]2006/Jun/20  BUG#7323
                } else if( service_unavail == 1 ) {
                        dolog("Service not available.");
                        LogError("Service not available in %s server.", pUpgrade->ftpSite);
                        LogError("FTP result:\n%s", buffer);
                        strcpy(pUpgrade->sysMsg, "Service not available.");
                        return(kFileTransferredFail);
		} else {
                        dolog("Transfer error occurred.");
                        LogError("Transfer error occurred: ret=%d .", ret);
                        LogError("FTP result:\n%s", buffer);
                        strcpy(pUpgrade->sysMsg, "Transfer error occurred.");
                        return(kFileTransferredFail);
                }
	}

#ifdef UBOOT_PARAM
	snprintf(buffer, sizeof(buffer), 
		"createimage %s >%s 2>&1", "/tmp/uImage", "/tmp/webtemp2");
	ret = system(buffer);
	/* Comment this out if you want to debug and see 
	   what actually got printed to the file */
	unlink("/tmp/uImage");
	fp = fopen("/tmp/webtemp2", "r");
	if (fp == NULL) {
		strcpy(pUpgrade->sysMsg, "Result file could not be opened.");
	    	return(kFileTransferredFail);
	}
	pUpgrade->sysMsg[0] = 0;
	while (fgets(buffer, 200, fp)) {
		/* Remove \r and \n */
		for (s = buffer + strlen(buffer) - 1; 
			s != buffer && (*s == '\r' || *s == '\n'); s --);
		*(s + 1) = 0;
		if (strstr(buffer, "[ERROR]")) {
			strncpy(pUpgrade->sysMsg, buffer, kInfoLength);
			break;
		}
		if (strstr(buffer, "[WARNING]")) {
			if (pUpgrade->sysMsg[0]) {
				strncat(pUpgrade->sysMsg, "\\n", kInfoLength);
			}
			strncat(pUpgrade->sysMsg, buffer, kInfoLength);
		}
	}
	unlink("/tmp/webtemp2");
	if (ret != 0) {
	    	return(kFileTransferredFail);
	}
	if (pUpgrade->sysMsg[0]) {
		strncat(pUpgrade->sysMsg, "\\n", kInfoLength);
	}
	strncat(pUpgrade->sysMsg, "Upgrade was successful. Reboot the appliance to make the upgrade effective.", kInfoLength);
	return(kFileTransferred);
	
#else
	ret = kFileTransferred;
	
	/* i = 0: verify checksum for file downloaded in RAM
	   i = 1: verify checksum for file saved in flash */
	for (i = 0; i < 2; i++) {
		
		if (pUpgrade->checkSum == kYes) {
			
			// read checksum in md5 file
			dolog("Checksum Yes");
			if ((fp = fopen("/tmp/temp.md5", "r")) == NULL)
			{
				strcpy(pUpgrade->sysMsg, "Checksum check failed "
					"- Checksum file not found");
				ret = kCheckSumCheckFailed;
				break;
			}
			fgets(s, sizeof(s), fp);
			dolog("Read in S: %s", s);
			fclose(fp);

			// calculate checksum from image
			if (!i)
				snprintf(buffer, sizeof(buffer), "md5sum "
					"/tmp/zImage > /tmp/webtemp");
			else
				snprintf(buffer, sizeof(buffer), "md5sum "
					"/%s/flash/zImage > /tmp/webtemp", flashDir);
                        system(buffer);
			fp = fopen("/tmp/webtemp", "r");
			fgets(t, sizeof(t), fp); 
			dolog("Read in T: %s", t);

			/* compares checksum */
			pS = s;
			strS = pS;
			while(*pS && !isspace(*pS)) pS++;
			if (*pS) *pS++=0;
			pT = t;
			strT = pT;
			while (*pT && !isspace(*pT)) pT++;			
			if (*pT) *pT++=0;
			dolog("str1 : %s", strS);
			dolog("str2 : %s", strT);
			if (strcmp(strS, strT))
			{
				strcpy(pUpgrade->sysMsg, "Checksum of files did not match");
				system("rm -f /tmp/webtemp");
				fclose(fp);
				ret = kCheckSumCheckFailed;
				break;
			}

			// free resources
			system("rm -f /tmp/webtemp");
			fclose(fp);
		}

		/* verify image and copy to flash */
		if (!i) {

			/* verify if it is the right image for the product */
			if (checkFwImage("/tmp/zImage")) {
				strcpy(pUpgrade->sysMsg, "Wrong firmware image for this product");
				ret = kFileWrongProductError;
				break;
			}

			/* copy image to flash */
			sprintf(copyCmd, "cp -f /tmp/zImage /%s/flash/zImage", flashDir);
			if (system(copyCmd)) {
				strcpy(pUpgrade->sysMsg, "Error copying file to flash");
				ret = kFileCopyFlashError;
				break;
			}
		}
	}

	// set upgrade status
	ftp_status_file = open("/tmp/fwup", O_WRONLY | O_CREAT | O_TRUNC, 0644);
	sprintf(copyCmd, "10011");
	lseek(ftp_status_file, 0, SEEK_SET);
	write(ftp_status_file, copyCmd, 5);
	close(ftp_status_file);

	// remove temp files
	system("rm -f /tmp/zImage");
	system("rm -f /tmp/temp.md5");

	if (ret == kFileTransferred) {
		dolog("File was successfully transferred");
		strncpy(pUpgrade->sysMsg, "Upgrade was successful. Reboot the "
			"appliance to make the upgrade effective.", kInfoLength);
	}
	
	return(ret);
#endif
}

//========================================================================
// CycSetCurrentAdmin - Sets the current admin. Can be called from CLI
//========================================================================
void CycSetCurrentAdmin(char_t *user)
{
   strcpy(currentAdmin, user);
}

//========================================================================
// CycGetCurrentAdmin - Gets the current admin. 
//========================================================================
char *CycGetCurrentAdmin()
{
	char *name;
	name = currentAdmin;
   return(name);
}

//========================================================================
// itox - Converts int value (of a char) to its hex value
//========================================================================
char* itox(int n) {
	static char temp[16];
	char *buf = &temp[0];
	int pos = 0, i;

	for (i = 2*sizeof(int) - 1; i >= 0; i--) {
		buf[pos] = "0123456789abcdef"[((n >> i*4) & 0xf)];
		pos++;
	}
	buf[pos] = '\0';

	return buf;
}

//========================================================================
// EncryptString - Encrypts passwd or any string using a key. def is
// true (1) or false(0). If 1, it uses the default XOR_ENCRYPT string as 
// the key. If 0, then it uses the key passed in. The XOR_ENCRYPT matches
// up with the xor string defined in the java applet code used to decrypt.
// So if you change the XOR_ENCRYPT string here, please synchronize with
// java applet. 
//========================================================================
int EncryptString(char *key, char *textString, int def)
{
   char_t   *enMask;
   char_t   enChar;
   int      numChars;

	if (def) 
		key = XOR_ENCRYPT;

	enMask = key;
   numChars = 0;

   while (*textString) {
		dolog("Encrypt text and key before: [%x][%c] ^ [%x][%c]", *textString, *textString, *enMask, *enMask);
      enChar = *textString ^ *enMask;
		dolog("XORed character: [%x][%c]", enChar, enChar);
/*
 *    Do not produce encrypted text with embedded linefeeds or tabs.
 *       Simply use existing character.
 */
      *textString = enChar;
/*
 *    Increment all pointers.
 */
      enMask++;
      textString++;
      numChars++;
/*
 *    Break if key is shorter than text
 */
      if (*enMask == '\0') {
	enMask = key;
      }
   }
   return numChars;
}

//========================================================================
// HexEncode - Converts a string to its hex value
//========================================================================
char *HexEncode(char *textString, int xorLen)
{
	char temp[3];
	char strTemp[kUserNameLength*2] = {0};
	char *p;

   while (xorLen) {
/*
 *    Do not produce encrypted text with embedded linefeeds or tabs.
 *       Simply use existing character.
 */
			p = itox((int)(*textString));
			dolog("Itox: %s", p);	
			sscanf(p, "000000%s", temp);
			dolog("Hex val of [%c]: [%s]", *textString, temp);
			strcat(strTemp, temp);
/*
 *    Increment all pointers.
 */
      textString++;
		xorLen--;
	}
	textString = strTemp;

   return (textString);
}
