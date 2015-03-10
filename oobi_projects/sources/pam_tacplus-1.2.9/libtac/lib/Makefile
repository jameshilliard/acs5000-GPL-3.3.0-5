
include $(Linux-PAM)/Make.Rules

CFLAGS += -I../include
LIBRARY = libtac.a

LIBSRCS = 	connect.c \
			header.c \
			version.c \
			xalloc.c \
			messages.c \
			hdr_check.c \
			attrib.c \
			crypt.c \
			magic.c \
			md5.c \
			author_s.c \
			author_r.c \
			authen_s.c \
			authen_r.c \
			acct_r.c \
			acct_s.c


LIBOBJS = $(LIBSRCS:.c=.o)

.c.o:
	$(CC) $(CFLAGS) -c $<

$(LIBRARY): $(LIBOBJS) Makefile
	$(AR) rc $(LIBRARY) $(LIBOBJS)
	$(RANLIB) $(LIBRARY)

clean: 
	rm -f core $(LIBRARY) $(LIBOBJS)

