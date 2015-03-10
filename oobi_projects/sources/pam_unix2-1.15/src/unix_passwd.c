/*
 * Copyright (c) 1999, 2000, 2001, 2002, 2003 SuSE GmbH Nuernberg, Germany.
 * Author: Thorsten Kukuk <kukuk@suse.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, and the entire permission notice in its entirety,
 *    including the disclaimer of warranties.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * ALTERNATIVELY, this product may be distributed under the terms of
 * the GNU Public License, in which case the provisions of the GPL are
 * required INSTEAD OF the above restrictions.  (This clause is
 * necessary due to a potential bad interaction between the GPL and
 * the restrictions contained in a BSD-style copyright.)
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <shadow.h>
#include <dlfcn.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <rpc/types.h>
#include <nss.h>
#include <rpcsvc/yp_prot.h>
#include <rpcsvc/ypclnt.h>

#define PAM_SM_PASSWORD
#include <security/_pam_macros.h>
#include <security/pam_modules.h>

#if defined(HAVE_XCRYPT_H)
#include <xcrypt.h>
#elif defined(HAVE_CRYPT_H)
#include <crypt.h>
#endif

#include "read-files.h"
#include "yppasswd.h"
#include "public.h"
#include "passwd_nss.h"
#include "getuser.h"

#ifndef RANDOM_DEVICE
#define RANDOM_DEVICE "/dev/urandom"
#endif

#define MAX_LOCK_RETRIES 3 /* How often should we try to lock password file */
#define MAX_PASSWD_TRIES 3
#define OLD_PASSWORD_PROMPT "Old Password: "
#define NEW_PASSWORD_PROMPT "New password: "
#define AGAIN_PASSWORD_PROMPT "Re-enter new password: "

#define SCALE (24L*3600L)

static int __do_setpass (pam_handle_t *pamh, int flags, user_t *user,
			 options_t *options, struct crypt_data *output);

PAM_EXTERN int
pam_sm_chauthtok (pam_handle_t *pamh, int flags, int argc, const char **argv)
{
  struct crypt_data output;
  user_t *data;
  char *user, *oldpass, *newpass;
  int retval, trys;
  options_t *options;

  memset (&output, 0, sizeof (output));

  options = get_options ("password", argc, argv);
  if (options == NULL)
    {
      __pam_log (LOG_ERR, "cannot get options");
      return PAM_BUF_ERR;
    }


  if (options->debug)
    __pam_log (LOG_DEBUG, "pam_sm_chauthtok() called");


  /* If we use ldap, handle pam_ldap like "sufficient". If it returns
     success, we should also return success. Else ignore the call.  */
  if (options->use_ldap)
    {
      void *pam_ldap_handle;

      if (options->debug)
	__pam_log (LOG_DEBUG, "Load pam_ldap.so module");

      pam_ldap_handle = dlopen (PAMDIR"/pam_ldap.so", RTLD_NOW);
      if (pam_ldap_handle == NULL)
        {
          __pam_log (LOG_ERR, "dlopen(\""PAMDIR"/pam_ldap.so\") failed: %s",
                     dlerror ());
        }
      else
        {
          int (*pam_ldap_func) (pam_handle_t *, int, int, char **);
          char *error;
          int retval = PAM_IGNORE;

          pam_ldap_func = dlsym (pam_ldap_handle, "pam_sm_chauthtok");
          if ((error = dlerror ()) != NULL)
            __pam_log (LOG_ERR, "dlsym failed: %s", error);
          else
            {
	      char *new_argv[2] = {NULL, NULL};
	      int new_argc = 0;

	      if (options->use_first_pass)
		{
		  new_argv[0] = "try_first_pass";
		  ++new_argc;
		}

              retval = (*pam_ldap_func)(pamh, flags, new_argc, new_argv);
              if (options->debug)
                __pam_log (LOG_DEBUG, "pam_ldap returned %d", retval);
            }

#if 0
          dlclose (pam_ldap_handle);
#endif

          if (retval == PAM_SUCCESS)
            return PAM_SUCCESS;
        }
    }

  retval = pam_get_item (pamh, PAM_USER, (const void **) &user);
  if (retval != PAM_SUCCESS)
    return retval;

  if (user == NULL || strlen (user) == 0)
    {
      if (options->debug)
	__pam_log (LOG_DEBUG, "user (%s) unknown", user ? user : "NULL");
      /* The app is supposed to get us the username! */
      return PAM_USER_UNKNOWN;
    }

  if (flags & PAM_PRELIM_CHECK)
    {
      /* This indicates that the modules are being probed as to their
         ready status for altering the user's authentication token. If
         the module requires access to another system over some network
         it should attempt to verify it can connect to this system on
         receiving this flag. If a module cannot establish it is ready to
         update the user's authentication token it should return
         PAM_TRY_AGAIN, this information will be passed back to the
         application. */

      retval = PAM_SUCCESS;
      data = __do_getpwnam (user, options->nisdir);

      if (data == NULL || data->service == S_NONE)
	retval = PAM_USER_UNKNOWN;
      else if (data->is_expired)
	retval = PAM_ACCT_EXPIRED;

      if (data)
	free_user_t (data);
      return retval;
    }

  /* Now we have all the initial information we need from the app to
     set things up (we assume that getting the username succeeded...) */
  data = __do_getpwnam (user, options->nisdir);
  if (data == NULL || data->service == S_NONE)
    {
      if (data)
	free_user_t (data);
      return PAM_USER_UNKNOWN;
    }

  if ((flags & PAM_CHANGE_EXPIRED_AUTHTOK) || getuid () != 0)
    {
      if (data->is_tooearly)
	{
	  if (data->use_hp_aging)
	    __write_message (pamh, flags, PAM_TEXT_INFO,
			  "Sorry: less then %d weeks since the last change.",
			     data->hp_week);
	  else
	    __write_message (pamh, flags, PAM_TEXT_INFO,
			     "Sorry: less then %d days since the last change.",
			     data->spw.sp_min);
	  free_user_t (data);
	  return PAM_AUTHTOK_ERR;
	}
      else if (data->is_expired)
	{
	  free_user_t (data);
	  return PAM_ACCT_EXPIRED; /* If their account has expired, we
				      can't auth them to change their
				      password */
	}
    }
  if ((data->use_shadow || data->use_hp_aging) && !data->is_expiring &&
      (flags & PAM_CHANGE_EXPIRED_AUTHTOK))
    {
      free_user_t (data);
      return PAM_SUCCESS;
    }

  retval = pam_get_item (pamh, PAM_OLDAUTHTOK, (const void **) &oldpass);
  if (retval != PAM_SUCCESS)
    {
      free_user_t (data);
      return retval;
    }

  if (oldpass == NULL)
    {
      /* If this is being run by root and we change a local password,
	 we don't need to get the old password. The test for
	 PAM_CHANGE_EXPIRED_AUTHTOK is here, because login runs as
         root and we need the old password in this case.  */

      if (options->debug)
	__pam_log (LOG_DEBUG, "No old password found.");

      /* Ask for old password: if we are a normal user or if
	 we are called by login or if we are not a local user.
	 If we are a NIS user, but called by root for on the
	 master, change local password with query, too.  */
      if (getuid () || (flags & PAM_CHANGE_EXPIRED_AUTHTOK) ||
	  (data->service != S_LOCAL &&
	   !(data->service == S_YP && options->nisdir != NULL )))
	{
	  if (options->use_first_pass)
	    {
	      if (options->debug)
		__pam_log (LOG_DEBUG, "use_first_pass set -> abort.");

	      free_user_t (data);
	      if (getuid () == 0 && data->service != S_LOCAL)
		__write_message (pamh, flags, PAM_ERROR_MSG,
				 "You can only change local passwords.");
	      return PAM_AUTHTOK_RECOVER_ERR;
	    }

	  retval = __get_passwd (pamh, OLD_PASSWORD_PROMPT, &oldpass);
	  if (retval != PAM_SUCCESS)
	    {
	      free_user_t (data);
	      return retval;
	    }
	  if (oldpass == NULL)
	    {
	      if (options->debug)
		__pam_log (LOG_DEBUG, "old password is NULL -> abort.");
	      free_user_t (data);
	      return PAM_AUTH_ERR;
	    }

	  /* Empty password means: entry is locked */
	  if ((data->oldpassword == NULL || strlen (data->oldpassword) == 0)
	      && !options->nullok)
	    {
	      if (options->debug)
		__pam_log (LOG_DEBUG, "old password is empty which is not allowed -> abort.");
	      free_user_t (data);
	      return PAM_AUTH_ERR;
	    }

	  if (strcmp (data->oldpassword,
		      crypt_r (oldpass, data->oldpassword, &output)) != 0)
	    {
	      if (options->debug)
		__pam_log (LOG_DEBUG, "old password is wrong -> abort.");

	      free_user_t (data);
	      return PAM_AUTH_ERR;
	    }

	  if (!options->not_set_pass)
	    pam_set_item (pamh, PAM_OLDAUTHTOK, (void *) oldpass);
	}
      else
	oldpass = "";
    }

  /* create a copy which we can give free later */
  data->oldclearpwd = strdup (oldpass);

  /* If we haven't been given a password yet, prompt for one... */
  trys = 0;
  pam_get_item (pamh, PAM_AUTHTOK, (const void **) &newpass);
  if (newpass != NULL)
    newpass = strdup (newpass);

  while ((newpass == NULL) && (trys++ < MAX_PASSWD_TRIES) &&
	 !options->use_authtok)
    {
      const char *cmiscptr = NULL;

      retval = __get_passwd (pamh, NEW_PASSWORD_PROMPT, &newpass);
      if (retval != PAM_SUCCESS)
	{
	  free_user_t (data);
	  return retval;
	}

      if (newpass == NULL)
	{
	  /* We want to abort the password change */
	  __write_message (pamh, flags, PAM_ERROR_MSG,
			   "Password change aborted");
	  free_user_t (data);
	  return PAM_AUTHTOK_ERR;
	}

      if (cmiscptr != NULL)
	{
	  __write_message (pamh, flags, PAM_ERROR_MSG, cmiscptr);
	  if (newpass != NULL)
	    {
	      memset (newpass, '\0', strlen (newpass));
	      free (newpass);
	      newpass = NULL;
	    }
	  if (trys >= MAX_PASSWD_TRIES)
	    {
	      free_user_t (data);
	      return PAM_AUTHTOK_ERR;
	    }
	}
      else
	{
	  char *new2;
	  retval = __get_passwd (pamh, AGAIN_PASSWORD_PROMPT, &new2);
	  if (retval != PAM_SUCCESS)
	    {
	      free_user_t (data);
	      return retval;
	    }

	  if (new2 == NULL)
	    {			/* Aborting password change... */
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Password change aborted");
	      free_user_t (data);
	      return PAM_AUTHTOK_ERR;
	    }

	  if (strcmp (newpass, new2) == 0)
	    {
	      memset (new2, '\0', strlen (new2));
	      free (new2);
	      new2 = NULL;
	      break;
	    }
	  else
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "They don't match; try again.");
	      if (new2 != NULL)
		{
		  memset (new2, '\0', strlen (new2));
		  free (new2);
		  new2 = NULL;
		}
	      if (newpass != NULL)
		{
		  memset (newpass, '\0', strlen (newpass));
		  free (newpass);
		  newpass = NULL;
		}
	    }
	}
    }
  /* end while (newpass == NULL) */

  if (newpass == NULL)
    {
      free_user_t (data);
      return PAM_AUTHTOK_ERR;	/* They didn't seem to enter the right password
				   for three tries - error */
    }

  /* If we don't support passwords longer 8 characters, truncate them */
  if (options->use_crypt == CRYPT && strlen (newpass) > 8)
    newpass[8] = '\0';
  /* blowfish has a limit of 72 characters */
  if (options->use_crypt == BLOWFISH && strlen (newpass) > 72)
    newpass[72] = '\0';
  /* MD5 has a limit of 127 characters */
  if (options->use_crypt == MD5 && strlen (newpass) > 127)
    newpass[127] = '\0';
  data->newpassword = newpass;

  retval = __do_setpass (pamh, flags, data, options, &output);
  if (retval < 0)
    {
      free_user_t (data);
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "Error: Password NOT changed");
      return PAM_AUTHTOK_ERR;
    }
  else
    {
      if (!options->not_set_pass)
	pam_set_item (pamh, PAM_AUTHTOK, (void *) newpass);

      free_user_t (data);
      __write_message (pamh, flags, PAM_TEXT_INFO, "Password changed");
      return PAM_SUCCESS;
    }
}

static char *
getnismaster (pam_handle_t *pamh, int flags)
{

  char *master, *domainname;
  int port, err;

  yp_get_default_domain (&domainname);

  if ((err = yp_master (domainname, "passwd.byname", &master)) != 0)
    {
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "can't find the master ypserver: %s",
		       yperr_string (err));
      return NULL;
    }
  port = getrpcport (master, YPPASSWDPROG, YPPASSWDPROC_UPDATE, IPPROTO_UDP);
  if (port == 0)
    {
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "yppasswdd not running on NIS master %s", master);
      return NULL;
    }
  if (port >= IPPORT_RESERVED)
    {
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "yppasswd daemon running on illegal port.");
      return NULL;
    }

  return master;
}

#if defined(HAVE_CRYPT_GENSALT_RN)
static int
read_loop (int fd, char *buffer, int count)
{
  int offset, block;

  offset = 0;
  while (count > 0)
    {
      block = read(fd, &buffer[offset], count);

      if (block < 0)
	{
	  if (errno == EINTR)
	    continue;
	  return block;
	}
      if (!block)
	return offset;

      offset += block;
      count -= block;
    }

  return offset;
}

#endif

static char *
make_crypt_salt (const char *crypt_prefix, int crypt_rounds,
		 pam_handle_t *pamh, int flags)
{
#if defined(HAVE_CRYPT_GENSALT_RN)
#define CRYPT_GENSALT_OUTPUT_SIZE (7 + 22 + 1)
  int fd;
  char entropy[16];
  char *retval;
  char output[CRYPT_GENSALT_OUTPUT_SIZE];

  fd = open (RANDOM_DEVICE, O_RDONLY);
  if (fd < 0)
    {
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "Can't open %s for reading: %s\n",
		       RANDOM_DEVICE, strerror (errno));
      return NULL;
    }

  if (read_loop (fd, entropy, sizeof(entropy)) != sizeof(entropy))
    {
      close (fd);
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "Unable to obtain entropy from %s\n",
		       RANDOM_DEVICE);
      return NULL;
    }

  close (fd);

  retval = crypt_gensalt_rn (crypt_prefix, crypt_rounds, entropy,
			     sizeof (entropy), output, sizeof(output));

  memset (entropy, 0, sizeof (entropy));

  if (!retval)
    {
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "Unable to generate a salt, "
		       "check your crypt settings.\n");
      return NULL;
    }

  return strdup (retval);
#else
#define ascii_to_bin(c) ((c)>='a'?(c-59):(c)>='A'?((c)-53):(c)-'.')
#define bin_to_ascii(c) ((c)>=38?((c)-38+'a'):(c)>=12?((c)-12+'A'):(c)+'.')

//[RK]Sep/22/05 - included the crypt_prefix in the salt

  time_t tm;
  char salt[16];
  int x;

  strcpy(salt,crypt_prefix);

  x = strlen(salt);

  time (&tm);
  salt[x] = bin_to_ascii(tm & 0x3f);
  salt[x+1] = bin_to_ascii((tm >> 6) & 0x3f);
  if (x == 3) {
     salt[x+2] = '$';
     x++;
  }
  salt[x+2] = '\0';

  return strdup (salt);
#endif
}

static int
__do_setpass (pam_handle_t *pamh, int flags, user_t *data,
	      options_t *options, struct crypt_data *output)
{
  int retval = 0;
  char *salt;
  char *newpassword = NULL;

  switch (options->use_crypt)
    {
    case CRYPT:
      salt =  make_crypt_salt ("", 0, pamh, flags);
      if (salt != NULL)
	newpassword = crypt_r (data->newpassword, salt, output);
      else
	{
	  __write_message (pamh, flags, PAM_ERROR_MSG,
			   "Cannot create salt for standard crypt");
	  return -1;
	}
      free (salt);
      break;
    case MD5:
      salt = make_crypt_salt ("$1$", 0, pamh, flags);
      if (salt != NULL)
	newpassword = crypt_r (data->newpassword, salt, output);
      else
	{
	  __write_message (pamh, flags, PAM_ERROR_MSG,
			   "Cannot create salt for MD5 crypt");
	  return -1;
	}
      free (salt);
      break;
    case BIGCRYPT:
#if defined (HAVE_BIGCRYPT)
      salt = make_crypt_salt ("", 0, pamh, flags);
      if (salt != NULL)
	newpassword = bigcrypt (data->newpassword, salt);
      else
	{
	  __write_message (pamh, flags, PAM_ERROR_MSG,
			   "Cannot create salt for bigcrypt");
	  return -1;
	}
      free (salt);
#else
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "No support for bigcrypt compiled in");
      return -1;
#endif
      break;
    case BLOWFISH:
#if defined(HAVE_CRYPT_GENSALT_RN)
      salt = make_crypt_salt ("$2a$", options->crypt_rounds, pamh, flags);
      if (salt != NULL)
	newpassword = crypt_r (data->newpassword, salt, output);
      else
	{
	  __write_message (pamh, flags, PAM_ERROR_MSG,
			   "Cannot create salt for blowfish crypt");
	  return -1;
	}
      free (salt);
#else
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "No support for blowfish compiled in");
      return -1;
#endif
      break;
    default:
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "crypt_t: Don't know %d", options->use_crypt);
      return -1;
    }
  if (newpassword == NULL)
    {
      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "crypt_r() returns NULL pointer");
      return -1;
    }

  if (data->service == S_LOCAL || (data->service == S_YP && options->nisdir))
    {
      int retries = 0;
      char *etcdir;

      if (options->nisdir && data->service == S_YP)
	etcdir = options->nisdir;
      else
	etcdir = files_etc_dir;

      while (lckpwdf () && retries < MAX_LOCK_RETRIES)
        {
          sleep (1);
          ++retries;
        }

      if (retries == MAX_LOCK_RETRIES)
        {
	  __write_message (pamh, flags, PAM_ERROR_MSG,
			   "Canot lock password file: already locked");
	  retval = -1;
        }
      else if (data->use_shadow)
	{
	  const char *fn_shadow_tmp = "/shadow.tmpXXXXXX";
	  const char *fn_shadow_orig = "/shadow";
	  char *shadow_tmp =
	    alloca (strlen (etcdir) + strlen (fn_shadow_tmp) + 2);
	  char *shadow_orig =
	    alloca (strlen (etcdir) + strlen (fn_shadow_orig) + 2);
	  char *shadow_old =
	    alloca (strlen (etcdir) + strlen (fn_shadow_orig) + 6);
	  struct stat passwd_stat;
	  struct spwd *sp; /* shadow struct obtained from fgetspent() */
	  FILE *oldpf, *newpf;
	  int gotit, newpf_fd;

	  sprintf (shadow_tmp, "%s%s", etcdir, fn_shadow_tmp);
	  sprintf (shadow_orig, "%s%s", etcdir, fn_shadow_orig);
	  sprintf (shadow_old, "%s%s.old", etcdir, fn_shadow_orig);

	  /* Open the shadow file for reading. We can't use getspent and
	     friends here, because they go through the YP maps, too. */
	  if ((oldpf = fopen (shadow_orig, "r")) == NULL)
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Can't open %s: %m", shadow_orig);
	      retval = -1;
	      goto error_shadow;
	    }
	  if (fstat (fileno (oldpf), &passwd_stat) < 0)
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Can't stat %s: %m", shadow_orig);
	      fclose (oldpf);
	      retval = -1;
	      goto error_shadow;
	    }

	  /* Open a temp shadow file */
	  newpf_fd = mkstemp (shadow_tmp);
	  if (newpf_fd == -1)
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Can't create temp file (%s): %m", shadow_tmp);
	      fclose (oldpf);
	      retval = -1;
	      goto error_shadow;
	    }
	  fchmod (newpf_fd, passwd_stat.st_mode);
	  fchown (newpf_fd, passwd_stat.st_uid, passwd_stat.st_gid);
	  newpf = fdopen (newpf_fd, "w+");
	  if (newpf == NULL)
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Can't open %s: %m", shadow_tmp);
	      fclose (oldpf);
	      close (newpf_fd);
	      retval = -1;
	      goto error_shadow;
	    }

	  gotit = 0;

	  /* Loop over all passwd entries */
	  while ((sp = fgetspent (oldpf)) != NULL)
	    {
	      /* check if this is the uid we want to change. A few
		 sanity checks added for consistency. */
	      if (!gotit && strcmp (data->pwd.pw_name, sp->sp_namp) == 0)
		{
		  time_t now;

		  time(&now);
		  /* set the new passwd */
		  sp->sp_pwdp = newpassword;
		  sp->sp_lstchg = (long int)now / (24L*3600L);
		  gotit = 1;
		}

	      /* write the passwd entry to tmp file */
	      if (putspent (sp, newpf) < 0)
		{
		  __write_message (pamh, flags, PAM_ERROR_MSG,
			    "Error while writing new shadow file: %m");
		  fclose (oldpf);
		  fclose (newpf);
		  retval = -1;
		  goto error_shadow;
		}
	    }
          if (fclose (oldpf) != 0)
            {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Error while closing old shadow file: %m");
              fclose (newpf);
              retval = -1;
              goto error_passwd;
            }
          if (fclose (newpf) != 0)
            {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "Error while closing temporary shadow file: %m");
              retval = -1;
              goto error_passwd;
            }
	  unlink (shadow_old);
	  link (shadow_orig, shadow_old);
	  rename (shadow_tmp, shadow_orig);
	error_shadow:
	  unlink (shadow_tmp);
	}
      else
	{
	  const char *fn_passwd_tmp = "/passwd.tmpXXXXXX";
	  const char *fn_passwd_orig = "/passwd";
	  char *passwd_tmp =
	    alloca (strlen (etcdir) + strlen (fn_passwd_tmp) + 2);
	  char *passwd_orig =
	    alloca (strlen (etcdir) + strlen (fn_passwd_orig) + 2);
	  char *passwd_old =
	    alloca (strlen (etcdir) + strlen (fn_passwd_orig) + 6);
	  struct stat passwd_stat;
	  struct passwd *pw; /* passwd struct obtained from fgetpwent() */
	  FILE *oldpf, *newpf;
	  int gotit, newpf_fd;

	  sprintf (passwd_tmp, "%s%s", etcdir, fn_passwd_tmp);
	  sprintf (passwd_orig, "%s%s", etcdir, fn_passwd_orig);
	  sprintf (passwd_old, "%s%s.old", etcdir, fn_passwd_orig);

	  if ((oldpf = fopen (passwd_orig, "r")) == NULL)
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Can't open %s: %m", passwd_orig);
	      retval = -1;
	      goto error_passwd;
	    }
          if (fstat (fileno (oldpf), &passwd_stat) < 0)
            {
              __write_message (pamh, flags, PAM_ERROR_MSG,
                               "Can't stat %s: %m", passwd_orig);
              fclose (oldpf);
	      retval = -1;
              goto error_passwd;
            }

	  /* Open a temp passwd file */
	  newpf_fd = mkstemp (passwd_tmp);
	  if (newpf_fd == -1)
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Can't create temp file (%s): %m", passwd_tmp);
	      fclose (oldpf);
	      retval = -1;
	      goto error_passwd;
	    }
          fchmod (newpf_fd, passwd_stat.st_mode);
          fchown (newpf_fd, passwd_stat.st_uid, passwd_stat.st_gid);
	  newpf = fdopen (newpf_fd, "w+");
	  if (newpf == NULL)
	    {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Can't open %s: %m", passwd_tmp);
	      fclose (oldpf);
	      close (newpf_fd);
	      retval = -1;
	      goto error_passwd;
	    }

	  gotit = 0;

	  /* Loop over all passwd entries */
	  while ((pw = fgetpwent (oldpf)) != NULL)
	    {
	      /* check if this is the uid we want to change. A few
		 sanity checks added for consistency. */
	      if (data->pwd.pw_uid == pw->pw_uid &&
		  data->pwd.pw_gid == pw->pw_gid &&
		  !strcmp (data->pwd.pw_name, pw->pw_name) && !gotit)
		{
		  pw->pw_passwd = newpassword;
		  gotit = 1;
		}

	      /* write the passwd entry to tmp file */
	      if (putpwent (pw, newpf) < 0)
		{
		  __write_message (pamh, flags, PAM_ERROR_MSG,
				"Error while writing new password file: %m");
		  fclose (oldpf);
		  fclose (newpf);
		  retval = -1;
		  goto error_passwd;
		}
	    }
          if (fclose (oldpf) != 0)
            {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
			       "Error while closing old password file: %m");
              fclose (newpf);
              retval = -1;
              goto error_passwd;
            }
          if (fclose (newpf) != 0)
            {
	      __write_message (pamh, flags, PAM_ERROR_MSG,
		       "Error while closing temporary password file: %m");
              retval = -1;
              goto error_passwd;
            }
	  unlink (passwd_old);
	  link (passwd_orig, passwd_old);
	  rename (passwd_tmp, passwd_orig);
	error_passwd:
	  unlink (passwd_tmp);
	}

      ulckpwdf ();
    }
  else if (data->service == S_YP)
    {
      struct yppasswd yppwd;
      CLIENT *clnt;
      char *master = getnismaster(pamh, flags);
      struct timeval TIMEOUT = {25, 0}; /* total timeout */
      int error, status;

      if (master == NULL)
	return -1;

      /* Initialize password information */
      memset (&yppwd, '\0', sizeof (yppwd));
      yppwd.newpw.pw_passwd = newpassword;
      yppwd.newpw.pw_name = data->pwd.pw_name;
      yppwd.newpw.pw_uid = data->pwd.pw_uid;
      yppwd.newpw.pw_gid = data->pwd.pw_gid;
      yppwd.newpw.pw_gecos = data->pwd.pw_gecos;
      yppwd.newpw.pw_dir = data->pwd.pw_dir;
      yppwd.newpw.pw_shell = data->pwd.pw_shell;
      yppwd.oldpass = data->oldclearpwd;

      __write_message (pamh, flags, PAM_TEXT_INFO,
		       "Changing NIS password for %s on %s.",
		       data->pwd.pw_name, master);

      clnt = clnt_create (master, YPPASSWDPROG, YPPASSWDVERS, "udp");
      clnt->cl_auth = authunix_create_default ();
      memset (&status, '\0', sizeof (status));
      error = clnt_call (clnt, YPPASSWDPROC_UPDATE,
			 (xdrproc_t) xdr_yppasswd, (caddr_t) &yppwd,
			 (xdrproc_t) xdr_int, (caddr_t) &status, TIMEOUT);
      if (error || status)
	{
	  if (error)
	    clnt_perrno (error);
	  else
	    __write_message (pamh, flags, PAM_ERROR_MSG,
			     "Error while changing the NIS password.");
	  retval = -1;
	}
    }
  else if (data->service == S_NISPLUS)
    {
      retval = npd_upd_pwd (pamh, flags, data);
      if (retval != 0)
	{
	  __write_message (pamh, flags, PAM_ERROR_MSG,
			   "Error while changing the NIS+ password.");
	  retval = -1;
	}
    }

  return retval;
}

