# $Id: generic.mk 3 2024-08-20 21:05:24Z nishi $

CC = cc
CFLAGS = -std=c99 -D_DEFAULT_SOURCE -I /usr/include/GraphicsMagick
LDFLAGS =
LIBS = -lcrypto
