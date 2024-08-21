# $Id$

CC = cc
CFLAGS = -std=c99 -I /usr/pkg/include -I /usr/pkg/include/GraphicsMagick
LDFLAGS = -L /usr/pkg/lib -Wl,-R/usr/pkg/lib
LIBS = -lcrypto
