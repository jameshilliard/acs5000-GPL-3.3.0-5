INSTALL=/usr/bin/install -c

LIBS=$(RAS_DIR)/src/librad.a

all: inetd install

install:
	 ${INSTALL} -m 755 inetd ${PREFIX}/bin  
	 $(STRIP) ${PREFIX}/bin/inetd

inetd: inetd.o daemon.o rot_inetd.o
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

inetd.o daemon.o: pathnames.h inetd.h

clean:
	rm -f *.o inetd

