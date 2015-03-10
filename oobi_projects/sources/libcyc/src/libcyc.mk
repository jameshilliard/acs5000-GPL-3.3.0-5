CC=$(CC_CROSS)
CXX=$(CXX_CROSS)
CFLAGS+=-O2 -Wall -Werror
CXXFLAGS+=-O2 -Wall -Werror
DEBUG_CFLAGS=-g -ggdb

%.so:
	$(CC) -shared -o $@ $^ $(LDFLAGS)

%.a:
	$(AR) -rc $@ $^
	$(RANLIB) $@

%.o: %.c
	$(CC) -fPIC -c $(CFLAGS) $< -o $@

%.o: %.cc
	$(CXX) -fPIC -c $(CXXFLAGS) $< -o $@

%.o: %.cpp
	$(CXX) -fPIC -c $(CXXFLAGS) $< -o $@

%.d: %.c
	$(SHELL) -ec '$(CC) -MM $(CFLAGS) $< \
		| sed "s/\($*\)\.o[ :]*/\1.o $@ : /g" > $@; \
		[ -s $@ ] || rm -f $@'

%.d: %.cc
	$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
		| sed "s/\($*\)\.o[ :]*/\1.o $@ : /g" > $@; \
		[ -s $@ ] || rm -f $@'


%.d: %.cpp
	$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
		| sed "s/\($*\)\.o[ :]*/\1.o $@ : /g" > $@; \
		[ -s $@ ] || rm -f $@'
