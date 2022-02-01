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

tmp/spider_solitaire.o: src/spider_solitaire.c include/spider_solitaire.h ../cio/include/cio.h ../utf8/include/utf8.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/spider_solitaire_facts.o : src/spider_solitaire_facts.c include/spider_solitaire.h ../cio/include/cio.h ../utf8/include/utf8.h ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<


tmp/spider_solitaire_main.o : src/spider_solitaire_main.c include/spider_solitaire.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

bin/spider_solitaire_facts : tmp/spider_solitaire_facts.o tmp/spider_solitaire.o tmp/cio.o tmp/utf8.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/spider_solitaire : tmp/spider_solitaire_main.o tmp/spider_solitaire.o tmp/cio.o tmp/utf8.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: all
all : bin/spider_solitaire bin/spider_solitaire_facts

.PHONY: check
check : all
	bin/spider_solitaire_facts | diff - expected/spider_solitaire_facts.out
	bin/spider_solitaire --decimal --key='secret' --encrypt='message' | bin/spider_solitaire --key='secret' --decrypt=- | diff - expected/message.out
	bin/spider_solitaire --base40 --key='secret' --encrypt='message' | bin/spider_solitaire --key='secret' --decrypt=- | diff - expected/message.out
	bin/spider_solitaire --cards --key='secret' --encrypt='message' | bin/spider_solitaire --key='secret' --decrypt=- | diff - expected/message.out

.PHONY: expected
expected : all
	bin/spider_solitaire_facts >expected/spider_solitaire_facts.out
	echo "message" >expected/message.out
