#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <grp.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pwd.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <syslog.h>
#include <unistd.h>
#include <utmp.h>
#include <crypt.h>
#include <string.h>

#include "server.h"
#include <userinfo.h>

static int get_group(char *user, char *grp);

// return : 0 - allow  and 1 - deny
int check_users_access(char *users, char *user, int null_valid)
{
    char *p, ch[32];
    int  deny_access;

#ifdef ONS
	if (!strcmp(user, ADMIN_GROUP)) return(0);
#endif

    if (users == NULL  || *users == 0) {
        return(null_valid);
    }

    deny_access = strchr(users, '!') ? 1 : 0;

    for (p=ch; users; users++) {
        switch(*users) {
        case 0:
            users = (char *)-1;
            /* fall through */
        case '!':
        case ',':
        case ' ':
        case '\t':
            if (p != ch) {
                *p = 0;
                if (!strcmp(ch, user))
                    return(deny_access);
    			if (get_group(user, ch))
                    return(deny_access);
                p = ch;
            }
            break;
        default:
            *p++ = *users;
            break;
        }
    }

    // user not found only accept if users should be denied
    return(!deny_access);
}

static int get_group(char *user, char *grp)
{
    char *group, *p, *u, usr[32];

    group = mainconf.groups;

    while (group) {
        p = group+4;
        u = usr;
        for ( ; p; p++) {
            switch(*p) {
            case 0:
		p = (char *)-1;
                /* fall through */
            case ',':
            case ' ':
            case '\t':
                if (u != usr) {
                    *u = 0;
                    if (!strcmp(usr, user)) return(1);
                    u = usr;
                }
		if (p == (char *)-1) return(0);
                break;
            case ':':
                *u = 0;
		if (strcmp(usr, grp)) {
			p = (char *)-1;
		}
                u = usr;
                break;
            default:
                *u++ = *p;
                break;
            }
        }
        group = *(char **)group;
    }
    return(0);
}

//[RK]May/20/05 - Group Authorization Support
// verify the ACL using the username and/or group_name
// return : 0 - permit, 1 - deny
int check_group_name_access(char * username, char * group_name, char * pusers, char * padmin, int pnull)
{
	int flgU=0, flgA=0;
	char *grp,*grp_name;

	if (pusers) 
		flgU = strchr(pusers, '!') ? 1 : 0;
	if (padmin)
		flgA = strchr(padmin, '!') ? 1 : 0;

	// check username
	if (check_users_access(pusers,username, pnull)) {
		if (flgU) { 
			return(1);
		} else {
			if (check_users_access(padmin, username, 1)) {
				if (flgA)
					return(1);
			} else {
				if (!flgA) {
					return(0);
				}
			}
		}
	} else {
		if (!flgU)
			return(0);
	}

	// check group name
	grp_name = strdup(group_name);
	grp = strtok(grp_name," ,;");

	while (grp) {
		if (check_users_access(pusers,grp, pnull)) {
			if (flgU) { 
				free(grp_name);
				return(1);
			} else {
				if (check_users_access(padmin, grp, 1)) {
					if (flgA) {
						free(grp_name);
						return(1);
					}
				} else {
					if (!flgA) {
						free(grp_name);
						return(0);
					}
				}
			}
		} else {
			if (!flgU) {
				free(grp_name);
				return(0);
			}
		}	
		grp = strtok(NULL," ,:");
	}
	
	free(grp_name);
	if (flgU || flgA) {
		return (0);
	}

	return(1); 
}

// check admin_user
// return : 0 - is not admin user , 1 - is admin user
int check_admin_user(char * username, char * group_name, char * padmin)
{
	int flgA=0;
	char *grp,*grp_name;

	if (padmin)
		flgA = strchr(padmin, '!') ? 1 : 0;

	// check username
	if (check_users_access(padmin, username, 1)) {
		if (flgA)
			return(0);
	} else {
		if (!flgA) {
			return(1);
		}
	}

	// check group name
	grp_name = strdup(group_name);
	grp = strtok(grp_name," ,;");

	while (grp) {
		if (check_users_access(padmin, grp, 1)) {
			if (flgA) {
				free(grp_name);
				return(0);
			}
		} else {
			if (!flgA) {
				free(grp_name);
				return(1);
			}
		}
		grp = strtok(NULL," ,:");
	}
	free(grp_name);
	if (flgA) {
		return (1);
	}
	return(0); 
}
