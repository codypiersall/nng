all: issuex

run: issuex
	./issuex tcp://127.0.0.1 1000

issuex: issuex.c
	gcc -O0 -Wall issuex.c -Iinclude -lnng -lpthread -Lbuild -o issuex


