#ifndef ROT_SHM_H
#define ROT_SHM_H 1

#define DB_ST_INACTIVE		0
#define DB_ST_ACTIVE		1
#define DB_ST_STOPPING		2
#define DB_ST_STOPPED		3
#define DB_ST_RESTART		4
#define DB_ST_SOCK_REST		5

#define TYPE_SHM_STR	0
#define TYPE_SHM_REALM	1
#define TYPE_SHM_SESS	2
#define TYPE_SHM_LSTR	3
#define TYPE_SHM_MAX	4

struct _CY_SHM_STR;

typedef struct _CY_SHM_STR CY_SHM_STR;

struct _CY_SHM_STR {
	int  flag;
	int  count;
	int  size;
	CY_SHM_STR *next;
};

void shm_inc_count(char *p);
void shm_free_str(char *p);
char *shm_malloc(int size, int flag);
char *shm_strdup(char *s);
int shm_init(int shm_id);
void shm_set_st_data_buffering(int port, int st);
int shm_get_st_data_buffering(int port);
int shm_get_set_val(int val, int *pval, int flag, int sem_num);
void shm_signal(int sem_num);
void shm_unsignal(int sem_num);

#endif
