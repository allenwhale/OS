CC=g++
CFLAGS=-O2 --std=c++11 -Wall -Wshadow -Wno-unused-result

all: myhexdump.out myhexedit.out listall.out nightmare.out recovery.out

myhexdump.out: myhexdump.cpp
	$(CC) $(CFLAGS) -o myhexdump.out myhexdump.cpp

myhexedit.out: myhexedit.cpp
	$(CC) $(CFLAGS) -o myhexedit.out myhexedit.cpp

listall.out: listall.cpp
	$(CC) $(CFLAGS) -o listall.out listall.cpp

nightmare.out: nightmare.cpp
	$(CC) $(CFLAGS) -o nightmare.out nightmare.cpp

recovery.out: recovery.cpp
	$(CC) $(CFLAGS) -o recovery.out recovery.cpp
