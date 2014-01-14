#
# Makefile - The cutewiki's makefile
#
# Copyright 2002 Martin Doering
#
# This file is distributed under the GPL, version 2 or at your
# option any later version.  See doc/license.txt for details.
#

# for HP-UX
#CC = cc
#CFLAGS = -Ae -O +e -s -Wall


# For Linux
CC = gcc
CFLAGS = -g -Wall -Wno-format-y2k -std=gnu99
#CFLAGS = -O2 -s -Wall -Wno-format-y2k

# tcc for debugging
#CC = tcc
#CFLAGS = -g -b

LIBS = -lcrypt
INCS =

OBJS = cutewiki.o user.o misc.o page.o page_list.o menu.o cfg.o \
       parser.o out-htm.o out-prt.o out-rtf.o rss20.o var.o \
       http.o request.o svr.o tar.o create.o html.o rcs.o \
       hash.o array.o
       #robot.o out-rss.o 

all: cutewiki
cutewiki: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

cutewiki.o: cutewiki.c cutewiki.h svr.h http.h page.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

user.o: user.c user.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

misc.o: misc.c cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

page.o: page.c page.h parser.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

page_list.o: page_list.c page_list.h page.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

create.o: create.c create.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

menu.o: menu.c page.h cutewiki.h  config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

parser.o: parser.c parser.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

cfg.o: cfg.c cfg.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

html.o: html.c html.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

out-htm.o: out-htm.c  cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

out-prt.o: out-prt.c  cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

out-rtf.o: out-rtf.c  cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

rss20.o: rss20.c  cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

var.o: var.c  var.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

http.o: http.c  http.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

request.o: request.c request.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

svr.o: svr.c svr.h http.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

hash.o: hash.c hash.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

array.o: array.c array.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

tar.o: tar.c tar.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

rcs.o: rcs.c rcs.h cutewiki.h config.h
	$(CC) $(CFLAGS) $(INCS) -c $<

#wget.o: wget.c wget.h cutewiki.h config.h
#	$(CC) $(CFLAGS) $(INCS) -c $<

#robot.o: robot.c robot.h cutewiki.h config.h
#	$(CC) $(CFLAGS) $(INCS) -c $<
#
#replic.o: replic.c replic.h cutewiki.h config.h
#	$(CC) $(CFLAGS) $(INCS) -c $<

testlocale: testlocale.c 
	$(CC) $(CFLAGS) -o $@ testlocale.c


clean:
	rm -f cutewiki test core *.o *~

hpux: cutewiki
	scp cutewiki u22md@mmswr061:src/www/cutewiki/src
