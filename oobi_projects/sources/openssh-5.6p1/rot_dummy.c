#include "includes.h"

#include "buffer.h"

// EAS Mar/19/02 Serial port and external server authentication support

int cyc_ssh_tty_data(Buffer *Buf, int fd)
{
	return(write(fd, buffer_ptr(Buf), buffer_len(Buf)));
}

int cyc_ignore_data_buffer(int len)
{
	return(len);
}

#ifdef CY_DLA_ALERT
void cy_dla_alert(Buffer *Buf)
{
	return;
}
#endif //CY_DLA_ALERT
