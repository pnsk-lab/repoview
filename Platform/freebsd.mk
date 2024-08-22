# $Id$

CC = cc
CFLAGS = -std=c99 -I /usr/local/include -I /usr/local/include/GraphicsMagick -DOPENSSL_SUPPRESS_DEPRECATED
LDFLAGS = -L /usr/local/lib
LIBS = -lcrypto
