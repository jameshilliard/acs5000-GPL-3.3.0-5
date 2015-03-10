RADDIR=../src

CFLAGS+=-I$(pppd_DIR)/pppd -I../ -I../src

ALL: libpsr.so

libpsr.so: $(RADDIR)/librad.a libpsr.o
	$(CC) -shared -o libpsr.so libpsr.o $(RADDIR)/librad.a -lc

install:
	install -m 644 libpsr.so $(LIBDEST)
	${STRIP} $(LIBDEST)/libpsr.so

clean:
	rm -f *.o *.so

