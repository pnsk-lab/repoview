# $Id$

PWD = `pwd`
PLATFORM = generic

FLAGS = PWD=$(PWD) PLATFORM=$(PLATFORM) EXTOBJS="`./objs`" EXTLIBS="`./libs`"

.PHONY: all clean format ./CGI

all: ./CGI

./CGI:: check objs libs
	$(MAKE) -C $@ $(FLAGS)

objs:: objs.c check
	cc -o $@ objs.c

libs:: libs.c check
	cc -o $@ libs.c

check:: check.c config.h
	cc -o $@ check.c
	./check

clean:
	$(MAKE) -C ./CGI clean $(FLAGS)
	rm -f objs libs check

format:
	clang-format --verbose -i `find . -name "*.c" -or -name "*.h"`
