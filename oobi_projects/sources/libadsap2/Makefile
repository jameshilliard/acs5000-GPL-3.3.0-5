LIBADSAP2_OBJS:= api_adsap2.o

TARGETS:=libadsap2.a

all: $(TARGETS)

libadsap2.a: $(LIBADSAP2_OBJS)
	$(AR) rv $@ $(LIBADSAP2_OBJS)
	$(RANLIB) $@

