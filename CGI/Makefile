# $Id$

include $(PWD)/Platform/$(PLATFORM).mk

.PHONY: all clean
.SUFFIXES: .c .o

OBJS = main.o sanity.o version.o util.o query.o page.o sha512.o md5.o repo.o multipart.o $(EXTOBJS)

all: repoview.cgi

repoview.cgi: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) $(EXTLIBS)

.c.o:
	$(CC) $(EXTCFLAGS) -I $(PWD)/CGI $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.cgi *.o */*.o
