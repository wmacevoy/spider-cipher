CSTD?=-std=c11
CDBG?=-g
COPT?=-O2
CINC?=-Iinclude -I../facts/include

CFLAGS=$(CDBG) $(COPT) $(CSTD) $(CINC)

LDLIBS=-lm

tmp/facts.o: ../facts/src/facts.c ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/spider_solitaire.o: src/spider_solitaire.c
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/spider_solitaire_factcheck.o : src/spider_solitaire_factcheck.c include/spider_solitaire.h ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

bin/spider_solitaire_factcheck : tmp/spider_solitaire_factcheck.o tmp/spider_solitaire.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

all : bin/spider_solitaire_factcheck
