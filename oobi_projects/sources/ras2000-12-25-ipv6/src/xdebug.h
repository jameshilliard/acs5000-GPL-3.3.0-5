#define XDEBUG_FORCE 0x80000000

#define XPRINTF( P... ) xprintf(XDEBUG_FORCE, ##P )

#ifdef XDEBUG
#define XPRINTF8( P... ) xprintf(8, ##P )
#define XPRINTF4( P... ) xprintf(4, ##P )
#define XPRINTF2( P... ) xprintf(2, ##P )
#define XPRINTF1( P... ) xprintf(1, ##P )
#else
#define XPRINTF1(P...)
#define XPRINTF2(P...)
#define XPRINTF4(P...)
#define XPRINTF8(P...)
#endif

int test_xflag(int bit);
int set_xprintf(int flag);
void xprintf(int flag, const char *fmt, ...);

