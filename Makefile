CSTD?=-std=c11  -D_POSIX_C_SOURCE=199309L
CDBG?=-g
COPT?=-O2
CINC?=-Iinclude -I../utf8/include -I../cio/include -I../facts/include

CFLAGS=$(CDBG) $(COPT) $(CSTD) $(CINC)

LDLIBS=-lm

tmp/facts.o: ../facts/src/facts.c ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/utf8.o: ../utf8/src/utf8.c ../utf8/include/utf8.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/cio.o: ../cio/src/cio.c ../cio/include/cio.h ../utf8/include/utf8.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

deps : tmp/utf8.o tmp/cio.o tmp/facts.o

tmp/spider_cipher.o: src/spider_cipher.c include/spider_cipher.h ../cio/include/cio.h ../utf8/include/utf8.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/spider_cipher_facts.o : src/spider_cipher_facts.c include/spider_cipher.h ../cio/include/cio.h ../utf8/include/utf8.h ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<


tmp/spider_cipher_main.o : src/spider_cipher_main.c include/spider_cipher.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

bin/spider_cipher_facts : tmp/spider_cipher_facts.o tmp/spider_cipher.o tmp/cio.o tmp/utf8.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/spider_cipher : tmp/spider_cipher_main.o tmp/spider_cipher.o tmp/cio.o tmp/utf8.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: all
all : bin/spider_cipher bin/spider_cipher_facts

.PHONY: check
check : all
	bin/spider_cipher_facts | diff - expected/spider_cipher_facts.out
	bin/spider_cipher --decimal --key='secret' --encrypt='message' | bin/spider_cipher --key='secret' --decrypt=- | diff - expected/message.out
	bin/spider_cipher --base40 --key='secret' --encrypt='message' | bin/spider_cipher --key='secret' --decrypt=- | diff - expected/message.out
	bin/spider_cipher --cards --key='secret' --encrypt='message' | bin/spider_cipher --key='secret' --decrypt=- | diff - expected/message.out

.PHONY: expected
expected : all
	bin/spider_cipher_facts >expected/spider_cipher_facts.out
	echo "message" >expected/message.out
