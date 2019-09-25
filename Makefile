all: issuex

run: issuex
	./issuex | python3 parse_output.py

issuex: issuex.c
	gcc -O0 -Wall issuex.c -Iinclude -lnng -lpthread -Lbuild -o issuex


