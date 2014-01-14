#
# Makefile - The wiky's makefile
#
# Copyright 2002 Martin Doering
#
# This file is distributed under the GPL, version 2 or at your
# option any later version.  See doc/license.txt for details.
#



#
# create a tgz archive named most-nnnnnn.tgz, where nnnnnn is the date.
#

HERE = $(shell pwd)
HEREDIR = $(shell basename $(HERE))
TGZ = $(shell echo $(HEREDIR)-`date +%y%m%d`|tr A-Z a-z).tgz

all: tgz
tgz: clean
	cd ..;\
	tar -cf - $(HEREDIR) | gzip -c -9 >$(TGZ)

utgz: clean
	cd ..;\
	tar -cf - cutewiki | gzip -c -9 >cutewiki.tgz

clean:
	rm -f *~

