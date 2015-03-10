/* vi: set sw=4 ts=4: */
/*
 * adduser - add users to /etc/passwd and /etc/shadow
 *
 * Copyright (C) 1999 by Lineo, inc. and John Beppu
 * Copyright (C) 1999,2000,2001 by John Beppu <beppu@codepoet.org>
 *
 * Licensed under the GPL v2 or later, see the file LICENSE in this tarball.
 */

#include "libbb.h"

#ifdef CYCLADES

#include <syslog.h>
#define OPT_DONT_SET_PASS  (1 << 7)
#define OPT_DONT_MAKE_HOME (1 << 9)

#else /* <--  CYCLADES */

#define OPT_DONT_SET_PASS  (1 << 4)
#define OPT_DONT_MAKE_HOME (1 << 6)

#endif /* <-- !CYCLADES */


#ifdef CYCLADES
static char *passwd_line = NULL;
extern void writeevt(int, const char *, ...); /* [RK]May/08/06 event notification */
#endif

/* remix */
/* EDR recoded such that the uid may be passed in *p */
static int passwd_study(const char *filename, struct passwd *p)
{
	enum { min = 500, max = 65000 };
	FILE *passwd;
	/* We are using reentrant fgetpwent_r() in order to avoid
	 * pulling in static buffers from libc (think static build here) */
	char buffer[256];
	struct passwd pw;
	struct passwd *result;
#ifdef CYCLADES
	int check_uid = (p->pw_uid == -1) ? 1 : 0;
	int uid_ok = 0;
#endif

	passwd = xfopen(filename, "r");

#ifdef CYCLADES
	if (check_uid)
		p->pw_uid = min;
#else
	/* EDR if uid is out of bounds, set to min */
	if ((p->pw_uid > max) || (p->pw_uid < min))
		p->pw_uid = min;
#endif

	/* stuff to do:
	 * make sure login isn't taken;
	 * find free uid and gid;
	 */
	while (!fgetpwent_r(passwd, &pw, buffer, sizeof(buffer), &result)) {
		if (strcmp(pw.pw_name, p->pw_name) == 0) {
			/* return 0; */
			return 1;
		}
#ifdef CYCLADES
		if (check_uid) { /* [RK]May/07/03 */
#endif
		if ((pw.pw_uid >= p->pw_uid) && (pw.pw_uid < max)
			&& (pw.pw_uid >= min)) {
			p->pw_uid = pw.pw_uid + 1;
		}
#ifdef CYCLADES
		} else {
			if (p->pw_uid == pw.pw_uid)
				uid_ok = 1;
		}
#endif
	}

#ifdef CYCLADES
	if (p->pw_gid == -1)	/* [RK]May/07/03 */
#else
	if (p->pw_gid == 0)
#endif
	{
		/* EDR check for an already existing gid */
		while (getgrgid(p->pw_uid) != NULL)
			p->pw_uid++;

		/* EDR also check for an existing group definition */
		if (getgrnam(p->pw_name) != NULL)
			return 3;

		/* EDR create new gid always = uid */
		p->pw_gid = p->pw_uid;
	}

	/* EDR bounds check */
#ifdef CYCLADES
	if (check_uid) {
		if ((p->pw_uid > max) || (p->pw_uid < min))
			return 2;
	} else {
		if (!uid_ok) return 2;
	}
#else
	if ((p->pw_uid > max) || (p->pw_uid < min))
		return 2;
#endif

	/* return 1; */
	return 0;
}

static void addgroup_wrapper(struct passwd *p)
{
	char *cmd;

	cmd = xasprintf("addgroup -g %d \"%s\"", p->pw_gid, p->pw_name);
	system(cmd);
	free(cmd);
}

#ifdef CYCLADES
static void passwd_wrapper(const char *login)
{
	char *cmd;
	static const char prog[] ALIGN1 = "passwd";

	if (passwd_line) {
		cmd = xasprintf("passwd -P %s %s", passwd_line, login);
		system(cmd);
		free(cmd);
	} else {
		execlp(prog, prog, login, NULL);
		bb_error_msg_and_die("Failed to execute '%s', you must set the password for '%s' manually", prog, login);
	}
}
#else
static void passwd_wrapper(const char *login) ATTRIBUTE_NORETURN;
static void passwd_wrapper(const char *login)
{
	static const char prog[] ALIGN1 = "passwd";

	BB_EXECLP(prog, prog, login, NULL);
	bb_error_msg_and_die("failed to execute '%s', you must set the password for '%s' manually", prog, login);
}
#endif

/* putpwent(3) remix */
static int adduser(struct passwd *p)
{
	FILE *file;
#ifdef CYCLADES
	int addgroup = (p->pw_gid == -1);
#else
	int addgroup = !p->pw_gid;
#endif


	/* make sure everything is kosher and setup uid && gid */
	file = xfopen(bb_path_passwd_file, "a");
	fseek(file, 0, SEEK_END);

	switch (passwd_study(bb_path_passwd_file, p)) {
		case 1:
			bb_error_msg_and_die("%s: login already in use", p->pw_name);
		case 2:
			bb_error_msg_and_die("illegal uid or no uids left");
		case 3:
			bb_error_msg_and_die("%s: group name already in use", p->pw_name);
	}

	/* add to passwd */
	if (putpwent(p, file) == -1) {
		bb_perror_nomsg_and_die();
	}
	/* Do fclose even if !ENABLE_FEATURE_CLEAN_UP.
	 * We will exec passwd, files must be flushed & closed before that! */
	fclose(file);

#if ENABLE_FEATURE_SHADOWPASSWDS
	/* add to shadow if necessary */
	file = fopen_or_warn(bb_path_shadow_file, "a");
	if (file) {
		fseek(file, 0, SEEK_END);
		fprintf(file, "%s:!:%ld:%d:%d:%d:::\n",
				p->pw_name,             /* username */
				time(NULL) / 86400,     /* sp->sp_lstchg */
				0,                      /* sp->sp_min */
				99999,                  /* sp->sp_max */
				7);                     /* sp->sp_warn */
		fclose(file);
	}
#endif

	/* add to group */
	/* addgroup should be responsible for dealing w/ gshadow */
	/* if using a pre-existing group, don't create one */
	if (addgroup) addgroup_wrapper(p);

	/* Clear the umask for this process so it doesn't
	 * screw up the permissions on the mkdir and chown. */
	umask(0);
	if (!(option_mask32 & OPT_DONT_MAKE_HOME)) {
		/* Set the owner and group so it is owned by the new user,
		   then fix up the permissions to 2755. Can't do it before
		   since chown will clear the setgid bit */
		if (mkdir(p->pw_dir, 0755)
		|| chown(p->pw_dir, p->pw_uid, p->pw_gid)
		|| chmod(p->pw_dir, 02755)) {
			bb_perror_msg("%s", p->pw_dir);
		}
	}

#ifdef CYCLADES
	writeevt(16, "Us", p->pw_name);
#endif
	if (!(option_mask32 & OPT_DONT_SET_PASS)) {
		/* interactively set passwd */
		passwd_wrapper(p->pw_name);
	}

	return 0;
}

#ifdef CYCLADES
/*
 * Regina Kodato - Nov/19/03
 * Add username in the supplementary group.
 *
 */
static void add_username_supgroup(char *groupname, char *username)
{
	FILE *grp;
	size_t len, len1;
	char *buffer, *a, *b, name[256];
	struct stat statb;

	if ((grp = fopen(bb_path_group_file, "r")) == 0) {
		syslog(4, "add_username %m");
		return;
	}
	stat(bb_path_group_file, &statb);
	len = statb.st_size;
	buffer = (char *) malloc(len);
	if (!buffer) return;
	fread(buffer, len, sizeof(char), grp);
	fclose(grp);

	if (strcmp(groupname, "root")) {
		sprintf(name, "\n%s:", groupname);
		a = strstr(buffer, name);
		if (!a) return;
		a++;	// skip \n
	} else {
		sprintf(name, "%s:", groupname);
		a = strstr(buffer, name);
	}

	b = index(a, '\n');
	len1 = b - buffer;

	//[RK]May/29 - verify if username is in the group line
	memset(name, 0, 256);
	strncpy(name, a, (len1 > 255) ? 255 : (len1 - 1));
	b = index(name, ':');
	if ((b = strstr(b, username)) != NULL) {
		if ((*(b-1) == ':' || *(b-1) == ',') &&
		    ((*(b+strlen(username)) == ',') ||
		     (*(b+strlen(username)) == ' ') ||
		     (*(b+strlen(username)) == 0x0a))) {
			// username has already had in the group
			return;
		}
	}

	grp = fopen(bb_path_group_file, "w");
	fwrite(buffer, len1, sizeof(char), grp);
	if (buffer[len1-1]==':') {
		fprintf(grp, "%s", username);
	} else {
		fprintf(grp, ",%s", username);
	}
	fwrite(&buffer[len1], (len - len1), sizeof(char), grp);
	fclose(grp);
}
#endif

/*
 * adduser will take a login_name as its first parameter.
 *
 * home
 * shell
 * gecos
 *
 * can be customized via command-line parameters.
 */
int adduser_main(int argc, char **argv);
int adduser_main(int argc, char **argv)
{
	struct passwd pw;
	const char *usegroup = NULL;
#ifdef CYCLADES
	char *userid = NULL;	/* Regina Kodato - May/07/03 */
	char *sup_group = NULL; /* Regina Kodato - Nov/19/03 */
#endif

	/* got root? */
	if (geteuid()) {
		bb_error_msg_and_die(bb_msg_perm_denied_are_you_root);
	}

	pw.pw_gecos = (char *)"Linux User,,,";
	pw.pw_shell = (char *)DEFAULT_SHELL;
	pw.pw_dir = NULL;

	/* exactly one non-option arg */
	opt_complementary = "=1";
#ifdef CYCLADES
	// -h <home> -c <gecos> -s <shell> -g <usergroup> -u <userid> -G <supl.group> -P <password>
	getopt32(argv, "h:c:s:g:u:G:P:DSH", 
		 &pw.pw_dir, &pw.pw_gecos, &pw.pw_shell, &usegroup,
		 &userid, &sup_group, &passwd_line);
#else
	getopt32(argv, "h:g:s:G:DSH", &pw.pw_dir, &pw.pw_gecos, &pw.pw_shell, &usegroup);
#endif
	argv += optind;

	/* create a passwd struct */
	pw.pw_name = argv[0];
	if (!pw.pw_dir) {
		/* create string for $HOME if not specified already */
		pw.pw_dir = xasprintf("/home/%s", argv[0]);
	}
	pw.pw_passwd = (char *)"x";
#ifndef CYCLADES
	pw.pw_uid = 0;
	pw.pw_gid = usegroup ? xgroup2gid(usegroup) : 0; /* exits on failure */
#else
	pw.pw_uid = -1;
	pw.pw_gid = -1;
	{
		long gid;

		if (usegroup) {
			/* Add user to a group that already exists */
			if (isdigit(*usegroup)) { /* group id  */
				gid = strtol(usegroup, NULL, 10);
				bb_getgrgid(NULL, -1, gid);
				pw.pw_gid = gid;
			} else {
				pw.pw_gid = xgroup2gid(usegroup);
			}
		}
		/* Regina Kodato - Nov/19/03 */
		if (sup_group) {
			/* supplementary group */
			unsigned char grname[50];
			while (*sup_group) {
				usegroup = sup_group;
				while (*sup_group && *sup_group != ',')
					sup_group++;
				if (*sup_group) *sup_group++ = '\0';
				if (isdigit(*usegroup)) {
					/* group id */
					gid = strtol(usegroup, NULL, 10);
					bb_getgrgid(grname, -1, gid);
				} else {
					/* group name */
					xgroup2gid(usegroup);
					strcpy(grname, usegroup);
				}
				/* add the username in the group definition */
				add_username_supgroup(grname, pw.pw_name);
			}
		}
	}
#endif

	/* grand finale */
	return adduser(&pw);
}
