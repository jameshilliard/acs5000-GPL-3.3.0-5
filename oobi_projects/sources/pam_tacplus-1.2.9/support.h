#include <security/pam_modules.h>

/* support.c */
//[RK]Jun/02/05 - Group Authorization Support - removed the get_str
extern int _pam_parse (int acct, int port, int argc, const char **argv);
extern const char * _resolve_name (char *serv, struct sockaddr_storage *addr);
extern int tacacs_get_password (pam_handle_t * pamh, int flags
		     ,int ctrl, char **password);
extern int converse (pam_handle_t * pamh, int nargs
	  ,struct pam_message **message
	  ,struct pam_response **response);
extern void _pam_log (int err, const char *format,...);
extern void *_xcalloc (size_t size);
extern char *_pam_get_terminal(pam_handle_t *pamh);

extern char *xinet_ntoa(struct sockaddr *server, char *buffer, int b_len);
extern const char *xinet_aton(const char *buffer, struct sockaddr_storage *server);
