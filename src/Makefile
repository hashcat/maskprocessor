##
##  Makefile for mp
##

CFLAGS = -W -Wall -std=c99 -O2 -s
#CFLAGS = -W -Wall -std=c99 -g

#CC_LINUX32        = /opt/hashcat-toolchain/linux32/bin/i686-hashcat-linux-gnu-gcc
#CC_LINUX64        = /opt/hashcat-toolchain/linux64/bin/x86_64-hashcat-linux-gnu-gcc
CC_LINUX32        = gcc
CC_LINUX64        = gcc
CC_WINDOWS32      = /usr/bin/i686-w64-mingw32-gcc
CC_WINDOWS64      = /usr/bin/x86_64-w64-mingw32-gcc
CC_OSX32          = /usr/bin/i686-apple-darwin10-gcc
CC_OSX64          = /usr/bin/i686-apple-darwin10-gcc

CFLAGS_LINUX32    = $(CFLAGS) -m32 -DLINUX
CFLAGS_LINUX64    = $(CFLAGS) -m64 -DLINUX
CFLAGS_WINDOWS32  = $(CFLAGS) -m32 -DWINDOWS
CFLAGS_WINDOWS64  = $(CFLAGS) -m64 -DWINDOWS
CFLAGS_OSX32      = $(CFLAGS) -m32 -DOSX
CFLAGS_OSX64      = $(CFLAGS) -m64 -DOSX

all: mp64.bin

mp32: mp32.bin mp32.exe mp32.app
mp64: mp64.bin mp64.exe mp64.app

clean:
	rm -f mp32.bin mp64.bin mp32.exe mp64.exe mp32.app mp64.app

mp32.bin: mp.c
	$(CC_LINUX32)   $(CFLAGS_LINUX32)   -o $@ $^

mp64.bin: mp.c
	$(CC_LINUX64)   $(CFLAGS_LINUX64)   -o $@ $^

mp32.exe: mp.c
	$(CC_WINDOWS32) $(CFLAGS_WINDOWS32) -o $@ $^

mp64.exe: mp.c
	$(CC_WINDOWS64) $(CFLAGS_WINDOWS64) -o $@ $^

mp32.app: mp.c
	$(CC_OSX32)     $(CFLAGS_OSX32)     -o $@ $^

mp64.app: mp.c
	$(CC_OSX64)     $(CFLAGS_OSX64)     -o $@ $^
