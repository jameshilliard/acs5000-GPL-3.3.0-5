all:    install

%.o:	%.c
	$(CC) $(CFLAGS) -c $+

install: libdlog.a dlogcfg
	#install -m 755 dlogcfg $(TARGET_DIR)/bin
	#$(STRIP) $(TARGET_DIR)/bin/dlogcfg	
	#install -m 644 libdlog.so $(TARGET_DIR)/lib
	#$(STRIP) $(TARGET_DIR)/lib/libdlog.so

libdlog.a: dlog.o
	$(AR) rvu $@ dlog.o
	$(RANLIB) $@

libdlog.so: dlog.o
	$(CC) -o $@ $(LDFLAGS) -shared dlog.o

dlogcfg: dlog.o dlogcfg.o
	$(CC) -o $@ $(LDFLAGS)  dlog.o dlogcfg.o

clean:
	-rm *.o libdlog.a libdlog.so dlogcfg
