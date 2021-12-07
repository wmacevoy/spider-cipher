

CSTD?=-std=c11
CDBG?=-g
COPT?=
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


tmp/spider_solitaire_big_facts.o : src/spider_solitaire_big_facts.c include/spider_solitaire.h ../cio/include/cio.h ../utf8/include/utf8.h ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<



tmp/spider_solitaire_main.o : src/spider_solitaire_main.c include/spider_solitaire.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

bin/spider_solitaire_facts : tmp/spider_solitaire_facts.o tmp/spider_solitaire.o tmp/cio.o tmp/utf8.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/spider_solitaire_big_facts : tmp/spider_solitaire_big_facts.o tmp/spider_solitaire.o tmp/cio.o tmp/utf8.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/spider_solitaire : tmp/spider_solitaire_main.o tmp/spider_solitaire.o tmp/cio.o tmp/utf8.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)


all : bin/spider_solitaire bin/spider_solitaire_facts bin/spider_solitaire_big_facts
