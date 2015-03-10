CPP		= $CC -E

INSTALL=/usr/bin/install -c

all: configts install

install:
	 ${INSTALL} -m 755 configts ${PREFIX}/bin  
	 $(STRIP) ${PREFIX}/bin/configts
	 cd ${PREFIX}/bin; ln -s -f configts config

configts:	main.o clparse.o ktermio.o $(KVMOBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@

configts.o main.o ktermio.o clparse.o: clparse.h

kvmcfg.o: $(KVMD_DIR)/kvmcfg.c $(KVMD_DIR)/kvmcfg.h $(KVMD_DIR)/kvmdefs.h
	$(CC) $(CFLAGS) -DKVM_CLI_CFG -c -o kvmcfg.o $(KVMD_DIR)/kvmcfg.c

kvmconfig.o: $(KVMD_DIR)/kvmconfig.c $(KVMD_DIR)/kvmconfig.h $(KVMD_DIR)/kvmdefs.h
	$(CC) $(CFLAGS) -DKVM_CLI_CFG -c -o kvmconfig.o $(KVMD_DIR)/kvmconfig.c

kvmuser.o: $(KVMD_DIR)/kvmuser.c $(KVMD_DIR)/kvmuser.h $(KVMD_DIR)/kvmdefs.h
	$(CC) $(CFLAGS) -DKVM_CLI_CFG -c -o kvmuser.o $(KVMD_DIR)/kvmuser.c

clean:
	rm -f *.o configts

