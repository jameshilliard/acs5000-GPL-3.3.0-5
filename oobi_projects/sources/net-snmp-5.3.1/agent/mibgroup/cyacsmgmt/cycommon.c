
#include "cycommon.h"
#include "cystrings.h"

static FILE *fd_file;
static struct stat st;
static time_t cy_read_time = 0;
static unsigned char *cy_pfile = 0;
static unsigned char cy_new[255], cy_all[255], cy_pbuf[300];

extern int cy_numgroups;
extern int cy_numports;
extern struct gtable *group_table;

static int cy_line_parse(unsigned char *buffer,
			   unsigned char *string,
			   unsigned char **pos)
{
	unsigned char *temp, *line, arg_size, c;

	temp = buffer;

	while ( *temp && (isspace(*temp))) temp++; // find alpha

	if (!*temp) return 0; // line don't have alpha

	line = temp;

	while ( *temp && !(isspace(*temp))) temp++;  // skip first argument
	
	if (*temp) {
		c = *temp;
		*temp = 0x00;
	} else {
		return 0;
	}
	if (strcmp(line, string)) { // don't find string 
		*temp = c;
		return 0;
	} 
	*temp++ = c;

	while ( *temp && (isspace(*temp))) temp++; // find alpha
		
	*pos = temp;

	while ( *temp && !(isspace(*temp))) temp++;  // skip second argument

	if (*temp) *temp = 0x00;

	return 1;
}

// *
// * cy_rw_parse (filename, parse, buf, size, flag_rw)
// * open the filename, read, find parse,
// * if flag_rw == 0 (READ) return string after parse in buf
// * if flag_rw == 1 (WRITE) write buf after the parse 
// * return number of bytes read or write
// *
int cy_rw_parse (char *filename, char *parse, char *buf, 
		int size, int flag_rw)
{
	int result=0, res1, flag=0;
	unsigned char *line, *temp, *pos;
	unsigned char *pfile;
	
	if (flag_rw) { // write
		memset(cy_new,0,255);
		strncpy (cy_new, buf, size);
		*(cy_new+size) = 0x00;
	}

	/* we will first read the file in a buffer */
	if (stat(filename, &st)) return 0;
	pfile = (unsigned char *)malloc(st.st_size+1);
	if (!pfile) return 0;
	if (!(fd_file = fopen (filename, "r"))) {
		free(pfile);
		return 0;
	}
	fread(pfile, st.st_size, 1, fd_file);
	*(pfile + st.st_size) = 0;
	fclose(fd_file);

	/* flag_rw == 1 -> write / flag_rw == 0 ->read */
	if (flag_rw) {
		fd_file = fopen (filename, "w");
	}

	if (parse == NULL) { // read/write the file contents
		if (flag_rw == 0) { // read
			result = (st.st_size > size) ? size : st.st_size;
			*(pfile + result) = 0x00;
			strcpy(buf, pfile);
		} else { // write
			result = fwrite(buf, size, 1, fd_file);
			fclose(fd_file);
		}
		free(pfile);
		return(result);
	}
	
	if (parse[0] == '2') {
		flag = 1;
		parse++;
	}

	line = pfile;
	while (*line) {
		if (*line == '#') { // skip comments
			temp = line;
			while (*line && *line!='\n') line++;
			if (flag_rw) fwrite(temp, (line-temp), 1, fd_file); 
			continue;
		}
		temp = line;
		while (*line && (*line != '\n')) line++; // find new line
		if (*line) *line++=0x00;
		if (!cy_line_parse(temp,parse,&pos)) {
			if (flag_rw) {
				fprintf(fd_file,"%s\n",temp);
			}
			continue;
		}
		if (flag) {
			flag = 0;
			if (flag_rw) {
				fprintf(fd_file,"%s\n",temp);
			}
			continue;
		}
		if (flag_rw == 0) { // read
			strcpy(buf, pos);
			free(pfile);
			return strlen(buf);
		} 
		// write
		if (!result) {
			result = fprintf(fd_file,"%s %s\n",parse,cy_new);
		} else {
			fprintf(fd_file,"%s %s\n",parse,pos);
		}
	}
	if (!result && flag_rw) {
		result = fprintf(fd_file,"%s %s\n",parse,cy_new);
	}
	if (flag_rw) {
		fclose(fd_file);
	}
	free(pfile);
	return result;
}


static int cy_sp_parse(unsigned char *buffer,
			   unsigned char **prefix,
			   unsigned char **comd,
			   unsigned char **value)
{
	unsigned char *temp, *line;

	if (strlen(buffer) > 255) return 0;
	strcpy(cy_pbuf,buffer);

	temp = cy_pbuf;

	while ( *temp && !(isalpha(*temp))) temp++; // find alpha

	if (!*temp) return 0; // line don't have alpha

	line = temp;

	while ( *temp && (*temp != '.')) temp++;  // skip prefix
	
	if (!*temp) return 0;	// line don't have prefix

	*prefix = line;
	*temp++ = 0x00;	
	
	line = temp;
	while ( *temp && !(isspace(*temp))) temp++;  // skip command

	if (!*temp)	{ // line don't have command
		return 0;
	}

	*comd = line;
	*temp++ = 0x00;
	line = temp;

	while ( *temp && (isspace(*temp))) temp++; // find alpha
		
	*value = temp;

	return 1;
}

// *
// * cy_rwsp_conf (prefix, command, buf, size, flag_rw)
// * open the pslave.conf, read, find "<prefix>.<command>",
// * if flag_rw == 0 (READ) return string after parse in buf
// * if flag_rw == 1 (WRITE) write buf after the parse 
// * return number of bytes read or write
// *
int cy_rwsp_conf (char *prefix, char *command, unsigned char *buf, 
		int size, int flag_rw)
{
	int result=0, res1, flg_incl=0, flg_host=0;
	unsigned char *line, *inl, *pref, *cmd, *value;
	unsigned char filename[40] = "/etc/portslave/pslave.conf";
	unsigned char hostname[40];

	memset(cy_all,0,255);

	if (flag_rw) { // write
		memset(cy_new,0,255);
		strncpy (cy_new, buf, size);
		*(cy_new+size) = 0x00;
	}

	if (stat(filename, &st)) return ;
	if (!cy_read_time || (cy_read_time !=  st.st_mtime)) { 
		cy_read_time = st.st_mtime;
		if (cy_pfile) free(cy_pfile);
		cy_pfile = (unsigned char *)malloc(st.st_size+1);
		if (!cy_pfile) return ;
		if (!(fd_file = fopen (filename, "r"))) {
			free(cy_pfile);
			return ;
		}
		fread(cy_pfile, st.st_size, 1, fd_file);
		*(cy_pfile + st.st_size) = 0;
		fclose(fd_file);
	}

	/* flag_rw == 1 -> write / flag_rw == 0 ->read */
	if (flag_rw) {
		fd_file = fopen (filename, "w");
	}

	inl = line = cy_pfile;
  
	while (*line) {
		if (*line == '#') { // skip comments
			inl = line;
			while (*line && (*line != '\n')) line++;
			if (flag_rw) fwrite(inl, (line-inl), 1, fd_file); 
			inl = line;
			continue;
		}
		while (*line && (*line != '\n')) line++; // find new line
		if (*line) {
			*line++=0x00;
			pref = inl + strlen(inl);
			if ((pref > inl) && (*(pref-1) == '\\')) {
				// line ended with '\'
				*(line-1) = '\n';
				continue;
			}
		}
		if (!(cy_sp_parse(inl,&pref,&cmd,&value))) {
			if (flag_rw) {
				fprintf(fd_file,"%s\n",inl);
			}
			*(line-1) = '\n';
			inl = line;
			continue;
		}
		*(line-1) = '\n';
		if (flg_incl) { // it is a include file
			if (!strcmp(pref,"conf") && !strcmp(cmd,"host_config")) {
				if (flg_host) { 
					// ended the configuration of this host
					cy_read_time = 0;
					return(0);
				} else {
					if (!strcmp(value,hostname)) {
						flg_host = 1;
						continue;
					}
				}
			}
			if (!flg_host) continue;
		}

		if (!strcmp(pref,"conf")) {
			if (!strcmp(cmd,"include")) {
				strcpy(filename,value);
				if (flag_rw) { // close pslave.conf
					fprintf(fd_file,"%s.%s %s\n", pref,cmd,value);
					fclose(fd_file);
					return 0;	// include only get
				}
				free(cy_pfile);
				if (!hostname[0]) {
					fd_file = fopen("/etc/hostname","r");
					fgets(hostname,40,fd_file);
					fclose(fd_file);
				}
				if (stat(filename, &st)) return 0;
				cy_pfile = (unsigned char *)malloc(st.st_size+1);
				if (!cy_pfile) return 0;
				if (!(fd_file = fopen (filename, "r"))) {
					free(cy_pfile);
					return 0;
				}
				fread(cy_pfile, st.st_size, 1, fd_file);
				*(cy_pfile + st.st_size) = 0;
				fclose(fd_file);
		
				flg_incl = 1;
				inl = line = cy_pfile;
				continue;
			}
			if (!strcmp(cmd,"hostname")) {
				strcpy(hostname, value);
			}
		}	

		if (!strcmp(prefix,pref) && !strcmp(command, cmd)) { 
			// found out
			if (flag_rw == 0) { // read
				res1 = strlen(value) + 1;
				res1 = (res1 > size) ? size : res1;
				strncpy(buf, value, res1);
				*(buf + res1) = 0x00;
				return res1;
			} 
			// write
			result = fprintf(fd_file,"%s.%s %s\n",pref,cmd,cy_new);
			if (!strcmp(cmd,"hostname")) {
				strcpy(hostname, cy_new);
			}
			inl = line;
			continue;
		}
		if (flag_rw) fprintf(fd_file,"%s.%s %s\n", pref,cmd,value);

		if (!flag_rw && (*prefix == 's') && !strcmp(pref,"all") && 
		    !strcmp(command,cmd)) {
			strcpy(cy_all, value);	
		}

		inl = line;
	}
	if (!result && flag_rw) {
		result = fprintf(fd_file,"%s.%s %s\n",prefix,command,cy_new);
	}
	if (flag_rw) {
		cy_read_time = 0;
		fflush(fd_file);
		fclose(fd_file);
	} else {
		if (*cy_all) {
			res1 = strlen(cy_all) + 1;
			result = (res1 > size) ? size : res1;
			strncpy(buf, cy_all, result);
			*(buf + result) = 0x00;
		}
	}
	return result;
}


static int cy_sp_group(unsigned char *buffer,
		       unsigned char **gname, unsigned char **gusers)
{
	unsigned char *temp, *line;

	if (strlen(buffer) > 255) return 0;
	strcpy(cy_pbuf,buffer);

	temp = cy_pbuf;

	while (*temp && !(isalpha(*temp)))
		temp++;		// find alpha
	if (!*temp)
		return 0;	// line don't have alpha
	line = temp;
	while (*temp && (*temp != '.'))
		temp++;		// skip prefix
	if (!*temp)
		return 0;	// line don't have prefix
	*temp = 0x00;
	if (strcmp("conf", line)) {
		return 0;
	}
	line = temp;
	while (*temp && !(isspace(*temp)))
		temp++;		// skip command
	if (!*temp) {		// line don't have command
		return 0;
	}
	*temp++ = 0x00;
	if (!strcmp("include", line) || !strcmp(line, "host_config")) {
		// is conf.include or hostname
		line = temp;
		while (*temp && (isspace(*temp)))
			temp++;	// find alpha
		if (*temp)
			*temp = 0x00;
		*gusers = temp;
		return 1;
	}
	if (strcmp("group", line)) {
		return 0;
	}

	while (*temp && (isspace(*temp)))
		temp++;		// find gname 
	if (!*temp) {
		return 0;
	}

	line = temp;
	while (*temp && (*temp != ':'))
		temp++;		//skip gname 
	if (!*temp) {
		return 0;
	}

	*temp++ = 0x00;
	*gname = line;
	while (*temp && (isspace(*temp)))
		temp++;		// find users 
	line = temp;
	*gusers = line;
	return 2;
}

// *
// * cy_rwsp_group (flag_rw)
// * open the pslave.conf, read, 
// * if flag_rw == 0 (READ) read groups in the struct
// * if flag_rw == 1 (WRITE) write groups in the struct 
// * return number of bytes read or write
// *
int cy_rwsp_group(int flag_rw)
{
	int result = 0, res1, flg_incl = 0, flg_host = 0, index = 0;
	unsigned char *line, *inl, *gname, *gusers;
	unsigned char filename[40] = "/etc/portslave/pslave.conf";
	unsigned char hostname[40];
	struct gtable *ptable;

	if (stat(filename, &st)) return 0;
	if (!cy_read_time || (cy_read_time !=  st.st_mtime)) { 
		cy_read_time = st.st_mtime;
		if (cy_pfile) free(cy_pfile);
		cy_pfile = (unsigned char *)malloc(st.st_size+1);
		if (!cy_pfile) return 0;
		if (!(fd_file = fopen (filename, "r"))) {
			free(cy_pfile);
			return 0;
		}
		fread(cy_pfile, st.st_size, 1, fd_file);
		*(cy_pfile + st.st_size) = 0;
		fclose(fd_file);
	} else {
		if (cy_numgroups) return cy_numgroups;
	}

	ptable = group_table;

	/* flag_rw == 1 -> write / flag_rw == 0 ->read */
	if (flag_rw) {
		fd_file = fopen(filename, "w");
	} else {
		memset(ptable, 0, 20 * sizeof(struct gtable));
	}

	inl = line = cy_pfile;

	while (*line) {
		if (*line == '#') {	// skip comments
			inl = line;
			while (*line && (*line != '\n'))
				line++;
			if (flag_rw)
				fwrite(inl, (line - inl), 1, fd_file);
			inl = line;
			continue;
		}
		while (*line && (*line != '\n'))
			line++;	// find new line
		if (*line) {
			*line++ = 0x00;
			gname = inl + strlen(inl);
			if ((gname > inl) && (*(gname - 1) == '\\')) {
				// line ended with '\'
				*(line - 1) = '\n';
				continue;
			}
		}
		if (!(res1 = cy_sp_group(inl, &gname, &gusers))) {
			if (flag_rw) {
				fprintf(fd_file, "%s\n", inl);
			}
			*(line - 1) = '\n';
			inl = line;
			continue;
		}

		*(line - 1) = '\n';
		if (flg_incl) {	// it is a include file
			if (res1 == 1) {
				if (flg_host) {
					// ended the configuration of this host
					cy_read_time = 0;
					return (0);
				} else {
					if (!strcmp(gusers, hostname)) {
						flg_host = 1;
						continue;
					}
				}
			}
			if (!flg_host)
				continue;
		}

		if (res1 == 1) {
			strcpy(filename, gusers);
			if (flag_rw) {	// close pslave.conf
				fprintf(fd_file, "conf.include %s\n",
					gusers);
				fclose(fd_file);
				return 0;	// include only get
			}
			free(cy_pfile);
			if (!hostname[0]) {
				fd_file = fopen("/etc/hostname", "r");
				fgets(hostname, 40, fd_file);
				fclose(fd_file);
			}
			if (stat(filename, &st))
				return 0;
			cy_pfile = (unsigned char *) malloc(st.st_size + 1);
			if (!cy_pfile)
				return 0;
			if (!(fd_file = fopen(filename, "r"))) {
				free(cy_pfile);
				return 0;
			}
			fread(cy_pfile, st.st_size, 1, fd_file);
			*(cy_pfile + st.st_size) = 0;
			fclose(fd_file);
			flg_incl = 1;
			inl = line = cy_pfile;
			continue;
		}
		// found out conf.group
		if (flag_rw == 0) {	// read
			ptable->index = ++index;
			strcpy(ptable->gname, gname);
			strcpy(ptable->gusers, gusers);
			ptable++;
		} else {
			if (ptable->index == index+1) {
				fprintf(fd_file, "conf.group %s: %s\n",
					ptable->gname, ptable->gusers);
				ptable++;
			}
			index++;
		}
		inl = line;
	}
	if (flag_rw) {
		for (; ptable->index && ptable->gname[0]; ptable++) {
			fprintf(fd_file, "conf.group %s: %s\n",
				ptable->gname, ptable->gusers);
		}
		cy_read_time = 0;
		fflush(fd_file);
		fclose(fd_file);
	}
	return index;
}

#ifndef TS1H
// =========================================================================
// PCMCIA support

extern int cy_numcards;
extern CY_PCMCIA_IDENT *cy_pcmcia_ident;
extern CY_PCMCIA_CONF *cy_pcmcia_conf;
extern CY_PCMCIA_STATUS *cy_pcmcia_status;
static time_t cy_pcmcia_read_time = 0;

static void cy_pcmcia_read_ident(int socket)
{
	unsigned char command[100], *pb, *paux, size;
	unsigned char tmpfile[30]="/tmp/snmp_pcmcia";

	sprintf(command,"cardctl ident %d > %s",socket, tmpfile);

	if (system(command) == -1) return;

	if (!(fd_file = fopen (tmpfile, "r"))) {
		return; 
	}
	
	memset(cy_new,0,254);

	while (fgets(cy_new,254,fd_file)) {
		pb = cy_new;
		size = strlen(pb) - 1;
		if (pb[size] == '\n') pb[size]=0x00; 
		while ( *pb && (isspace(*pb))) pb++; // find alpha
		paux = pb;
		while ( *pb && (*pb != ':')) pb++;  // skip prefix
		*pb++ = 0x00;	
		if (!strcmp(paux,"product info")) {
			strcpy(cy_pcmcia_ident[socket].prod,pb);
			continue;
		} 
		if (!strcmp(paux,"manfid")) {
			strcpy(cy_pcmcia_ident[socket].man,pb);
			continue;
		}
		if (!strcmp(paux,"function")) {
			strcpy(cy_pcmcia_ident[socket].func,pb);
			continue;
		}
		if (!strcmp(paux,"PCI id")) {
			strcpy(cy_pcmcia_ident[socket].pci,pb);
		}
	}
	fclose(fd_file);
	sprintf(command,"rm -f %s",tmpfile);
	system(command);
}

static void cy_pcmcia_read_conf(int socket)
{
	unsigned char command[100], *pb, *paux, size;
	unsigned char tmpfile[30]="/tmp/snmp_pcmcia";

	sprintf(command,"cardctl config %d > %s",socket, tmpfile);

	if (system(command) == -1) return;

	if (!(fd_file = fopen (tmpfile, "r"))) {
		return; 
	}
	
	memset(cy_new,0,254);

	while (fgets(cy_new,254,fd_file)) {
		pb = cy_new;
		size = strlen(pb) - 1;
		if (pb[size] == '\n') pb[size]=0x00; 
		while ( *pb && (isspace(*pb))) pb++; // find alpha
		paux = pb;
		while ( *pb && (*pb != ' ')) pb++;  // skip prefix
		size = (unsigned char)(pb - paux);
		if (!strncmp(paux,"Vcc",size)) {
			strcpy(cy_pcmcia_conf[socket].power,paux);
			continue;
		} 
		if (!strncmp(paux,"interface",size)) {
			strcpy(cy_pcmcia_conf[socket].type,paux);
			continue;
		}
		if (!strncmp(paux,"irq",size)) {
			strcpy(cy_pcmcia_conf[socket].inter,paux);
		}
		if (!strncmp(paux,"DMA",size)) {
			size = strlen(cy_pcmcia_conf[socket].inter);
			strcpy(&cy_pcmcia_conf[socket].inter[size],"   ");
			size += 3;
			strcpy(&cy_pcmcia_conf[socket].inter[size],paux);
		}
		if (!strncmp(paux,"speaker",size)) {
			size = strlen(cy_pcmcia_conf[socket].inter);
			strcpy(&cy_pcmcia_conf[socket].inter[size],"   ");
			size += 3;
			strcpy(&cy_pcmcia_conf[socket].inter[size],paux);
		}
		if (!strncmp(paux,"function",size)) {
			strcpy(cy_pcmcia_conf[socket].func,paux);
			continue;
		}
		if (!strncmp(paux,"config",size)) {
			strcpy(cy_pcmcia_conf[socket].cardv,paux);
			continue;
		}
		if (!strncmp(paux,"option",size)) {
			size = strlen(cy_pcmcia_conf[socket].cardv);
			strcpy(&cy_pcmcia_conf[socket].cardv[size],"   ");
			size += 3;
			strcpy(&cy_pcmcia_conf[socket].cardv[size],paux);
		}
		if (!strncmp(paux,"io",size)) {
			if (cy_pcmcia_conf[socket].port1[0]) {
				strcpy(cy_pcmcia_conf[socket].port2,paux);
			} else {
				strcpy(cy_pcmcia_conf[socket].port1,paux);
			}
		}
	}
	fclose(fd_file);
	sprintf(command,"rm -f %s",tmpfile);
	system(command);
}

static void cy_pcmcia_read_status(int socket)
{
	unsigned char command[100], *pb, *paux, size;
	unsigned char tmpfile[30]="/tmp/snmp_pcmcia";

	sprintf(command,"cardctl status %d > %s",socket, tmpfile);

	if (system(command) == -1) return;

	if (!(fd_file = fopen (tmpfile, "r"))) {
		return; 
	}
	
	memset(cy_new,0,254);

	while (fgets(cy_new,254,fd_file)) {
		pb = cy_new;
		size = strlen(pb) - 1;
		if (pb[size] == '\n') pb[size]=0x00; 
		while ( *pb && (isspace(*pb))) pb++; // find alpha
		paux = pb;
		while ( *pb && (*pb != ' ')) pb++;  // skip prefix
		size = (unsigned char) (pb - paux);	
		if (!strncmp(paux,"function",size)) {
			strcpy(cy_pcmcia_status[socket].func,paux);
		} else {
			strcpy(cy_pcmcia_status[socket].card,paux);
		}
	}
	fclose(fd_file);
	sprintf(command,"rm -f %s",tmpfile);
	system(command);
}

void cy_pcmcia_read(void)
{
	unsigned char file_stat[] = "/var/lib/pcmcia/stab";
	unsigned char *pb;
	int counter;

	if (stat(file_stat, &st)) return ;
	if (cy_pcmcia_read_time && (cy_pcmcia_read_time >=  st.st_mtime)) {
		return;		
	} 
	// pcmcia configuration is changed --> need to read new configuration
	if (!cy_pcmcia_ident) {
		cy_pcmcia_ident = (CY_PCMCIA_IDENT *) malloc(sizeof(CY_PCMCIA_IDENT)*CY_PCMCIA_MAX);
		cy_pcmcia_conf = (CY_PCMCIA_CONF *) malloc(sizeof(CY_PCMCIA_CONF)*CY_PCMCIA_MAX);
		cy_pcmcia_status = (CY_PCMCIA_STATUS *)malloc(sizeof(CY_PCMCIA_STATUS)*CY_PCMCIA_MAX);
	} 
	memset(cy_pcmcia_ident,0, sizeof(CY_PCMCIA_IDENT) * CY_PCMCIA_MAX);
	memset(cy_pcmcia_conf,0, sizeof(CY_PCMCIA_CONF) * CY_PCMCIA_MAX);
	memset(cy_pcmcia_status,0, sizeof(CY_PCMCIA_STATUS) * CY_PCMCIA_MAX);
	memset(cy_new, 0, 134);

	cy_pcmcia_read_time = st.st_mtime;
	if (!(fd_file = fopen (file_stat, "r"))) {
		return ;
	}
	for (counter=0; fgets(cy_new,132,fd_file); ) {
		if (cy_new[0] == 'S') {
			pb = cy_new;
			while (*pb && (*pb != ':')) pb++;
			while (*pb && !isalpha(*pb)) pb++; 
			if (strncmp(pb,"empty",5)) {
				counter++;
			}
		}
	}
	fclose(fd_file);
	cy_numcards = counter;
	if (!cy_numcards) { // don't have card installed
		return;
	}
	
	for (counter=0; counter < cy_numcards ; counter++) {
		cy_pcmcia_read_ident(counter);
		cy_pcmcia_read_conf(counter);
		cy_pcmcia_read_status(counter);
	}
}
#endif // not TS1H


