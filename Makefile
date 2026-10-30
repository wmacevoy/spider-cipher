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

tmp/spider_cipher_generic_facts.o : src/spider_cipher_generic_facts.c ../facts/include/facts.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

tmp/spider_cipher_main.o : src/spider_cipher_main.c include/spider_cipher.h
	mkdir -p tmp
	$(CC) -c $(CFLAGS) $(LDFLAGS) -o $@ $<

bin/spider_cipher_facts : tmp/spider_cipher_facts.o tmp/spider_cipher.o tmp/cio.o tmp/utf8.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/spider_cipher_generic_facts : tmp/spider_cipher_generic_facts.o tmp/facts.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

bin/spider_cipher : tmp/spider_cipher_main.o tmp/spider_cipher.o tmp/cio.o tmp/utf8.o
	mkdir -p bin
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: all
all : bin/spider_cipher bin/spider_cipher_facts

# The default suite takes ~15 s unoptimized (CI's plain build), past facts' 10 s default timeout.
FACTS_TIMEOUT?=300

.PHONY: check
check : all
	bin/spider_cipher_facts --facts_timeout=$(FACTS_TIMEOUT) | diff - expected/spider_cipher_facts.out
	bin/spider_cipher --decimal --key='secret' --encrypt='message' | bin/spider_cipher --key='secret' --decrypt=- | diff - expected/message.out
	bin/spider_cipher --base40 --key='secret' --encrypt='message' | bin/spider_cipher --key='secret' --decrypt=- | diff - expected/message.out
	bin/spider_cipher --cards --key='secret' --encrypt='message' | bin/spider_cipher --key='secret' --decrypt=- | diff - expected/message.out
# REQ2/PK-2: a fixed packet decrypts and exits 0; the same packet with its first message card
# changed prints nothing, says why on stderr, and exits nonzero.
	bin/spider_cipher --key='secret' --decrypt=- <expected/message.packet >tmp/message.out && diff tmp/message.out expected/message.out
	! bin/spider_cipher --key='secret' --decrypt=- <expected/message-changed.packet >tmp/changed.out 2>tmp/changed.err
	test ! -s tmp/changed.out && test -s tmp/changed.err

.PHONY: expected
expected : all
	bin/spider_cipher_facts --facts_timeout=$(FACTS_TIMEOUT) >expected/spider_cipher_facts.out
	echo "message" >expected/message.out

# -------------- LaTeX docs --------------
# The papers build with latexmk + LuaLaTeX, and figures with plain asy, as in ../luck. Without a
# local TeX Live, ./build.sh runs these targets in the devcontainer-latex Docker image.
LATEXMK?=latexmk
LUALATEX_FLAGS?=-lualatex -interaction=nonstopmode -halt-on-error -file-line-error -shell-escape
TEX_OUT?=build

FIGURES=$(TEX_OUT)/figures/venn.pdf $(TEX_OUT)/figures/graph.pdf $(TEX_OUT)/figures/tree.pdf

$(TEX_OUT)/figures/%.pdf: figures/%.asy
	mkdir -p $(dir $@)
	asy -f pdf -o $@ $<

# check-log.sh fails the build, and deletes the PDF, on silent content loss (missing glyphs,
# undefined references or fonts)
$(TEX_OUT)/%.pdf: %.ltx
	mkdir -p $(TEX_OUT)
	$(LATEXMK) $(LUALATEX_FLAGS) -outdir=$(TEX_OUT) $<
	@'$(CURDIR)/tools/check-log.sh' '$(TEX_OUT)/$*.log' '$@'

$(TEX_OUT)/theory.pdf: $(FIGURES)

.PHONY: theory perfect-theory foundations papers
theory: $(TEX_OUT)/theory.pdf
perfect-theory: $(TEX_OUT)/perfect-theory.pdf
foundations: $(TEX_OUT)/foundations.pdf
papers: theory perfect-theory foundations

.PHONY: clean-theory
clean-theory:
	rm -rf $(TEX_OUT)
