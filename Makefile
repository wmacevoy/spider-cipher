CSTD?=-std=c11 -D_POSIX_C_SOURCE=200809L
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

# -------------- LaTeX docs --------------
ASY?=asy
ASY_ENGINE?=pdflatex
ASY_TEXPATH?=$(CURDIR)/tools
ASYFLAGS?=-tex $(ASY_ENGINE)
TECTONIC?=tectonic
TEX_OUT?=build

FIGURES=$(TEX_OUT)/figures/venn.pdf

$(TEX_OUT)/figures/%.pdf: figures/%.asy
	mkdir -p $(dir $@)
	$(ASY) $(ASYFLAGS) -f pdf -o $@ $<

$(TEX_OUT)/theory.pdf: theory.ltx $(FIGURES)
	mkdir -p $(TEX_OUT)
	$(TECTONIC) -X compile --outdir $(TEX_OUT) $<

.PHONY: theory
theory: $(TEX_OUT)/theory.pdf

$(TEX_OUT)/perfect-theory.pdf: perfect-theory.ltx
	mkdir -p $(TEX_OUT)
	$(TECTONIC) -X compile --outdir $(TEX_OUT) $<

.PHONY: perfect-theory
perfect-theory: $(TEX_OUT)/perfect-theory.pdf

.PHONY: clean-theory
clean-theory:
	rm -rf $(TEX_OUT)
