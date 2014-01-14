# pc/Makefile
# This file is part of RCS.
#
# RCS is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# RCS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with RCS; see the file COPYING.  If not, write to
# the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
#
# Report problems and direct all questions to:
#
#    rcs-bugs@cs.purdue.edu

# Supported MAKE programs:
# - IBM or Microsoft NMAKE
# - DMAKE 3.8 or 4.0
# - GNU make 3.64 or higher

# Supported compilers and target environments:
# - emx+gcc 0.9c for OS/2, DOS or Win95/NT 32-bit (with RSXNT 1.3)
# - Microsoft C 10.0 (Visual C++ 4.0) for Win95/NT 32-bit
# - Microsoft C 6.00A for OS/2 or DOS 16-bit
# - Watcom C/C++ 10.6 for OS/2, DOS or Win95/NT 32-bit
# - Watcom C/C++ 10.6 for OS/2 or DOS 16-bit

# The PC-specific source code subdirectory.
D = pc

# default target
default:
	@echo "Available targets:"
	@echo "	emxos2 emxos2-static emxos2-debug emxdos"
	@echo "	emxnt emxnt-static emxnt-debug"
	@echo "	mingw32 mingw32-static mingw32-debug"
	@echo " ibmos2 ibmos2-debug ibmnt ibmnt-debug"
	@echo "	mscos2 mscos2-debug mscdos mscdos-debug"
	@echo "	mscnt mscnt-static mscnt-debug"
	@echo "	w32os2 w32os2-static w32os2-debug"
	@echo "	w32nt w32nt-static w32nt-debug"
	@echo "	w16os2 w16os2-debug w16dos w16dos-debug"

# emx 0.9c (gcc) -> 32-bit OS/2 executables
emxos2:
	$(MAKE) all -f $D/Makefile EMXO="" OS="os2" \
	CC="gcc -Zomf" CCFLAGS="-O -D__OS2__ -DRCSDLL" \
	LDFLAGS="-Zstack 512 -Zcrtdll -s -o ./" \
	DLDFLAGS="-Zdll -Zcrtdll -s -o ./"
emxos2-static:
	$(MAKE) all -f $D/Makefile GCCS="" OS="os2" \
	CC="gcc -Zomf" CCFLAGS="-O -D__OS2__" \
	LDFLAGS="-Zstack 512 -Zsys -s -o ./"
emxos2-debug:
	$(MAKE) all -f $D/Makefile GCCS="" OS="os2" \
	CC="gcc -g" CCFLAGS="-D__OS2__" \
	LDFLAGS="-o ./" O=".o" A=".a" AR="ar"

# emx 0.9c (gcc) -> 32-bit DOS executables
emxdos:
	$(MAKE) all -f $D/Makefile GCCS="" OS="dos" \
	CC="gcc" CCFLAGS="-O -D__MSDOS__" \
	LDFLAGS="-s -o ./" O=".o" A=".a" AR="ar"

# emx 0.9c (gcc) -> 32-bit Win95/NT executables
emxnt:
	$(MAKE) all -f $D/Makefile EMXN="" OS="nt" \
	CC="gcc -Zwin32 -Zcrtdll=crtrsxnt" CCFLAGS="-O -D__WIN32__ -DRCSDLL" \
	LDFLAGS="-s -o ./" O=".o" A=".a" AR="ar" \
	DLDFLAGS="-Zdll -ladvapi32 -s -o ./"
emxnt-static:
	$(MAKE) all -f $D/Makefile GCCS="" OS="nt" \
	CC="gcc -Zwin32" CCFLAGS="-O -D__WIN32__" \
	LDFLAGS="-Zsys -ladvapi32 -s -o ./" O=".o" A=".a" AR="ar"
emxnt-debug:
	$(MAKE) all -f $D/Makefile GCCS="" OS="nt" \
	CC="gcc -Zwin32 -g" CCFLAGS="-D__WIN32__" \
	LDFLAGS="-ladvapi32 -o ./" O=".o" A=".a" AR="ar"

# mingw32 (gcc) -> 32-bit Win95/NT executables
mingw32:
	$(MAKE) all -f $D/Makefile MINGW32="" OS="nt" \
	CC="gcc" CCFLAGS="-O -D__WIN32__ -D__32BIT__ -DRCSDLL" \
	LDFLAGS="-s -o ./" O=".o" A=".a" AR="ar" \
	DLDFLAGS="-ladvapi32 -s -o ./"
mingw32-static:
	$(MAKE) all -f $D/Makefile GCCS="" OS="nt" \
	CC="gcc" CCFLAGS="-O -D__WIN32__ -D__32BIT__" \
	LDFLAGS="-ladvapi32 -s -o ./" O=".o" A=".a" AR="ar"
mingw32-debug:
	$(MAKE) all -f $D/Makefile GCCS="" OS="nt" \
	CC="gcc -g" CCFLAGS="-D__WIN32__ -D__32BIT__ -DDEBUG" \
	LDFLAGS="-ladvapi32 -o ./" O=".o" A=".a" AR="ar"

# IBM VisualAge C/C++ -> 32-bit OS/2 executables
ibmos2:
	$(MAKE) all -f $D/Makefile IBM="" OS="os2" EXTRA="mktemp.obj" \
	CC="icc -q" CCFLAGS="-wpro- -wcnd- -Gs -O" \
	LDFLAGS="-B/ST:0x50000 -B/NOE setargv.obj -Fe "
ibmos2-debug:
	$(MAKE) all -f $D/Makefile IBM="" OS="os2" EXTRA="mktemp.obj" \
	CC="icc -q -Ti" CCFLAGS="-wpro- -wcnd-" \
	LDFLAGS="-B/ST:0x50000 -B/NOE setargv.obj -Fe "

# IBM VisualAge C/C++ -> 32-bit Win32 executables
ibmnt:
	$(MAKE) all -f $D/Makefile IBM="" OS="nt" EXTRA="mktemp.obj" \
	CC="icc -q" CCFLAGS="-wpro- -wcnd- -Gs -O -D__WIN32__ -D__32BIT__" \
	LDFLAGS="advapi32.lib -B/ST:0x50000 -B/NOE setargv.obj -Fe "
ibmnt-debug:
	$(MAKE) all -f $D/Makefile IBM="" OS="nt" EXTRA="mktemp.obj" \
	CC="icc -q -Ti" CCFLAGS="-wpro- -wcnd- -DDEBUG -D__WIN32__ -D__32BIT__" \
	LDFLAGS="-B/ST:0x50000 -B/NOE setargv.obj -Fe "

# Microsoft C 6.00A -> 16-bit OS/2 executables
# remove the -Lp option if the default libraries are the protected mode ones
mscos2:
	$(MAKE) all -f $D/Makefile MSC="" OS="os2" \
	CC="cl -nologo -AS" CCFLAGS="-Zp -G2s -Ocegit -D__OS2__" \
	LDFLAGS="-F 4000 -Lp setargv.obj $D\\rcs.def -Fe" LINK="/NOE"
mscos2-debug:
	$(MAKE) all -f $D/Makefile MSC="" OS="os2" \
	CC="cl -nologo -Zi -AM" CCFLAGS="-Zp -G2 -Od -D__OS2__" \
	LDFLAGS="-F 4000 -Lp setargv.obj $D\\rcs.def -Fe" LINK="/NOE"

# Microsoft C 6.00A -> 16-bit DOS executables
# remove the -Lr option if the default libraries are the real mode ones
mscdos:
	$(MAKE) all -f $D/Makefile MSC="" OS="dos" \
	CC="cl -nologo -AS" CCFLAGS="-Zp -G2s -Ocegit -D__MSDOS__" \
	LDFLAGS="-F 4000 -Lr setargv.obj -Fe" LINK="/NOE"
mscdos-debug:
	$(MAKE) all -f $D/Makefile MSC="" OS="dos" \
	CC="cl -nologo -Zi -AM" CCFLAGS="-Zp -G2s -Od -D__MSDOS__" \
	LDFLAGS="-F 4000 -Lr setargv.obj -Fe" LINK="/NOE"

# Microsoft C 10.00 -> 32-bit Win32 executables
mscnt:
	$(MAKE) lib -f $D/Makefile MSCD="" OS="nt" \
	CC="cl -nologo -MD" DLDFLAGS="advapi32.lib -Fe" \
	CCFLAGS="-Zp -Gs -Ogti -D__WIN32__ -D__32BIT__"
	$(MAKE) exe -f $D/Makefile MSCD="" OS="nt" \
	CC="cl -nologo -MD" LDFLAGS="-F 50000 setargv.obj -Fe" \
	CCFLAGS="-Zp -Gs -Ogti -D__WIN32__ -D__32BIT__ -DRCSDLL -DRCSIMP"
mscnt-static:
	$(MAKE) all -f $D/Makefile MSCS="" OS="nt" \
	CC="cl -nologo -MD" CCFLAGS="-Zp -Gs -Ogti -D__WIN32__ -D__32BIT__" \
	LDFLAGS="advapi32.lib -F 50000 setargv.obj -Fe"
mscnt-debug:
	$(MAKE) all -f $D/Makefile MSCS="" OS="nt" \
	CC="cl -nologo -Zi" CCFLAGS="-Zp -Gs -Od -D__WIN32__ -D__32BIT__" \
	LDFLAGS="advapi32.lib -F 50000 setargv.obj -Fe"

# Watcom C/C++ 10.6 -> 32-bit OS/2 executables
w32os2:
	$(MAKE) lib -f $D/Makefile WCD="" OS="os2" \
	CC="wcl386 -bd -bt=os2v2 -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__OS2__ -D__32BIT__" EXTRA="mktemp.obj" \
	DLDFLAGS="-l=os2v2dll @pc\rcs.lnk -Fe="
	$(MAKE) exe -f $D/Makefile WCD="" OS="os2" \
	CC="wcl386 -bt=os2v2 -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__OS2__ -D__32BIT__ -DRCSDLL" \
	LDFLAGS="-l=os2v2exe -k0x50000 @pc\rcs.lnk -Fe="
w32os2-static:
	$(MAKE) all -f $D/Makefile WCS="" OS="os2" \
	CC="wcl386 -bt=os2v2 -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__OS2__ -D__32BIT__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=os2v2exe -k0x50000 @pc\rcs.lnk -Fe="
w32os2-debug:
	$(MAKE) all -f $D/Makefile WCS="" OS="os2" \
	CC="wcl386 -bt=os2v2 -zq -d2" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Od -D__OS2__ -D__32BIT__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=os2v2exe -k0x50000 @pc\rcs.lnk -Fe="

# Watcom C/C++ 10.6 -> 32-bit Win95/NT executables
w32nt:
	$(MAKE) lib -f $D/Makefile WCD="" OS="nt" \
	CC="wcl386 -bd -bt=nt -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__WIN32__ -D__32BIT__" EXTRA="mktemp.obj" \
	DLDFLAGS="-l=nt_dll -Fe="
	$(MAKE) exe -f $D/Makefile WCD="" OS="nt" \
	CC="wcl386 -bt=nt -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__WIN32__ -D__32BIT__ -DRCSDLL" \
	LDFLAGS="-l=nt -k0x50000 -Fe="
w32nt-static:
	$(MAKE) all -f $D/Makefile WCS="" OS="nt" \
	CC="wcl386 -bt=nt -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__WIN32__ -D__32BIT__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=nt -k0x50000 -Fe="
w32nt-debug:
	$(MAKE) all -f $D/Makefile WCS="" OS="nt" \
	CC="wcl386 -bt=nt -zq -d2" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Od -D__WIN32__ -D__32BIT__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=nt -k0x50000 -Fe="

# Watcom C/C++ 10.6 -> 16-bit OS/2 executables
w16os2:
	$(MAKE) all -f $D/Makefile WCS="" OS="os2" \
	CC="wcl -mm -bt=os2 -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__OS2__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=os2v1exe -k0x4000 @pc\rcs.lnk -Fe="
w16os2-debug:
	$(MAKE) all -f $D/Makefile WCS="" OS="os2" \
	CC="wcl -mm -bt=os2 -zq -d2" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Od -D__OS2__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=os2v1exe -k0x4000 @pc\rcs.lnk -Fe="

# Watcom C/C++ 10.6 -> 16-bit DOS executables
w16dos:
	$(MAKE) all -f $D/Makefile WCS="" OS="dos" \
	CC="wcl -mm -bt=dos -zq" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Ox -D__MSDOS__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=dos -k0x4000 -Fe="
w16dos-debug:
	$(MAKE) all -f $D/Makefile WCS="" OS="dos" \
	CC="wcl -mm -bt=dos -zq -d2" DIRSEP="\\" \
	CCFLAGS="-Zp1 -s -Od -D__MSDOS__" EXTRA="mktemp.obj" \
	LDFLAGS="-l=dos -k0x4000 -Fe="


libcos2:
	$(MAKE) all -f $D/Makefile GCCS="" OS="os2" \
	CC="gcc" CCFLAGS="-O -D__OS2__" \
	LDFLAGS="-s -o ./"


# defaults:
O = .o
A = .l
X = .exe
AR = ar
DIRSEP = /

# target hiding macros, one of them is always pre-set to be empty
GCCS = gccs
EMXO = emxo
EMXN = emxn
MINGW32 = mingw32
IBM = ibm
MSC = msc
MSCS = mscs
MSCD = mscd
WCS = wcs
WCD = wcd

# other macros:
CFLAGS = $(CCFLAGS) -I$D -Dmain=rcsmain -Dhas_conf_h
LIBOBJS = getcwdsl$O spawnvpq$O getlogin$O $(EXTRA)
REMOVE = del

# ----- end of configuration section -----
# You shouldn't have to change anything past this point.

# all commands
PROGRAMS = ci.exe co.exe ident.exe merge.exe rcs.exe \
	rcsclean.exe rcsdiff.exe rcsmerge.exe rlog.exe

.SUFFIXES: .c $O .exe

.c$O:
	$(CC) $(CFLAGS) -c $<

$O.exe:
	$(CC) $< libinit$O rcslib$A $(LDFLAGS)$@

all: lib exe
exe: $(PROGRAMS)
lib: rcslib$A
$(PROGRAMS): libinit$O rcslib$A

RCSLIB1 = rcslex$O rcssyn$O rcsgen$O rcsedit$O rcskeys$O rcsmap$O
RCSLIB2 = rcsrev$O rcsutil$O rcsfnms$O partime$O maketime$O
RCSLIB3 = rcskeep$O rcsfcmp$O rcstime$O merger$O version$O
RCSLIB = $(RCSLIB1) $(RCSLIB2) $(RCSLIB3) $(LIBOBJS)

# For the library targets to work under DOS too, we need to get around
# the stupid DOS command line length limit by using multiple commands.

# dynamic/shared library variant, emx, OS/2 only
$(EMXO)rcslib$A: $D/rcslib.def $(EMXO)rcslib.dll
	-$(REMOVE) $@
	emximp -o $@ $D/rcslib.def
$(EMXO)rcslib.dll: dllmain$O $(RCSLIB) $D/rcslib.def
	$(CC) dllmain$O $(RCSLIB) $(DLDFLAGS)$@ $D/rcslib.def

# dynamic/shared library variant, emx, NT only
$(EMXN)rcslib$A: $(EMXN)rcslib.dll
	-$(REMOVE) $@
	makelib $(EMXN)rcslib.dll -o $@
$(EMXN)rcslib.dll: dllmain$O $(RCSLIB) $D/rcslib.def
	$(CC) dllmain$O $(RCSLIB) $(DLDFLAGS)$@ $D/rcslib.def

# dynamic/shared library variant, MinGW32, NT only
$(MINGW32)rcslib$A: $D/rcslib.def $(MINGW32)rcslib.dll
	-$(REMOVE) $@
	dlltool -d $D/rcslib.def -D $(MINGW32)rcslib.dll -l $@
$(MINGW32)rcslib.dll: dllmain$O $(RCSLIB) $D/rcslib.def
	dllwrap dllmain$O $(RCSLIB) $(DLDFLAGS)$@ --def $D/rcslib.def

# static library variant, gcc
$(GCCS)rcslib$A: $(RCSLIB)
	-$(REMOVE) $@
	$(AR) cr $@ $(RCSLIB1)
	$(AR) cr $@ $(RCSLIB2)
	$(AR) cr $@ $(RCSLIB3)
	$(AR) cr $@ $(LIBOBJS)
	$(AR) s $@

# static library variant, IBM VisualAge C/C++
$(IBM)rcslib$A: $(RCSLIB)
	-$(REMOVE) $@
	ilib /nologo /out:$@ $(RCSLIB1) $(RCSLIB2) $(RCSLIB3) $(LIBOBJS)

# static library variant, MS C 6.00
$(MSC)rcslib$A: $(RCSLIB)
	-$(REMOVE) $@
	lib /nologo /noi $@ $(RCSLIB1);
	lib /nologo /noi $@ $(RCSLIB2);
	lib /nologo /noi $@ $(RCSLIB3);
	lib /nologo /noi $@ $(LIBOBJS);

# dynamic/shared library variant, MS C 10.00
$(MSCD)rcslib$A: $D/rcslib.def $(MSCD)rcslib.dll
	-$(REMOVE) $@
	lib /out:$@ /def:$D/rcslib.def
$(MSCD)rcslib.dll: dllmain$O $(RCSLIB) $D/rcslib.def
	$(CC) dllmain$O $(RCSLIB) $(DLDFLAGS)$@ $D/rcslib.def

# static library variant, MS C 10.00
$(MSCS)rcslib$A: $(RCSLIB)
	-$(REMOVE) $@
	lib /nologo /out:$@ $(RCSLIB1) $(RCSLIB2) $(RCSLIB3) $(LIBOBJS)

# dynamic/shared library variant, Watcom C/C++ OS/2 and Win95/NT only
$(WCD)rcslib.lib: $D/rcslib.lnk $(WCD)rcslib.dll
	-$(REMOVE) $@
	wlib -q -c $@ +rcslib.dll
$(WCD)rcslib.dll: dllmain$O $(RCSLIB) $D/rcslib.lnk
	$(CC) dllmain$O $(RCSLIB) $(DLDFLAGS)$@ @$D$(DIRSEP)rcslib.lnk

# static library variant, Watcom C/C++
$(WCS)rcslib$A: $(RCSLIB)
	-$(REMOVE) $@
	wlib -q -c -b $@ + $(RCSLIB1)
	wlib -q -c -b $@ + $(RCSLIB2)
	wlib -q -c -b $@ + $(RCSLIB3)
	wlib -q -c -b $@ + $(LIBOBJS)

OBJECTS=ci$O co$O ident$O maketime$O merge$O merger$O partime$O \
	rcs$O rcsclean$O rcsdiff$O rcsedit$O rcsfcmp$O rcsfnms$O \
	rcsgen$O rcskeep$O rcskeys$O rcslex$O rcsmap$O rcsmerge$O \
	rcsrev$O rcssyn$O rcsutil$O rlog$O version$O

$(OBJECTS): $D/conf.h rcsbase.h

ci$O: ci.c
	$(CC) $(CFLAGS) -DRCSMAIN -c ci.c
co$O: co.c
	$(CC) $(CFLAGS) -DRCSMAIN -c co.c
ident$O: ident.c
	$(CC) $(CFLAGS) -DRCSMAIN -c ident.c
merge$O: merge.c
	$(CC) $(CFLAGS) -DRCSMAIN -c merge.c
rcs$O: rcs.c
	$(CC) $(CFLAGS) -DRCSMAIN -c rcs.c
rcsclean$O: rcsclean.c
	$(CC) $(CFLAGS) -DRCSMAIN -c rcsclean.c
rcsdiff$O: rcsdiff.c
	$(CC) $(CFLAGS) -DRCSMAIN -c rcsdiff.c
rcsmerge$O: rcsmerge.c
	$(CC) $(CFLAGS) -DRCSMAIN -c rcsmerge.c
rlog$O: rlog.c
	$(CC) $(CFLAGS) -DRCSMAIN -c rlog.c

getcwdsl$O: $D/getcwdsl.c
	$(CC) $(CFLAGS) -c $D$(DIRSEP)getcwdsl.c
spawnvpq$O: $D/spawnvpq.c
	$(CC) $(CFLAGS) -c $D$(DIRSEP)spawnvpq.c
getlogin$O: $D/getlogin.c
	$(CC) $(CFLAGS) -c $D$(DIRSEP)getlogin.c
mktemp$O: $D/mktemp.c
	$(CC) $(CFLAGS) -c $D$(DIRSEP)mktemp.c

libinit$O: $D/libinit.c
	$(CC) $(CFLAGS) -c $D$(DIRSEP)libinit.c
dllmain$O: $D/dllmain.c
	$(CC) $(CFLAGS) -c $D$(DIRSEP)dllmain.c

clean:
	-$(REMOVE) *.o *.a *.obj *.lib *.exp *.bak conf.err $(PROGRAMS)
