# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

The Spider Cipher is a manual encryption scheme operated with a 40-card deck (values 0–39, mod-40 "base40" arithmetic). This repo holds several implementations of the cipher (C reference, minimal C core, C++ headers, Verilog hardware RNG, JS browser demo), tooling for the physical artifacts (card SVGs, 3D-printed dice), and the academic papers describing it.

## Build & test

### Reference C implementation (root `Makefile`, `src/`, `include/`)

**Requires sibling repos checked out next to this repo**: `../facts`, `../utf8`, `../cio` (github.com/wmacevoy/{facts,utf8,cio}). The Makefile includes their headers and compiles their sources directly; CI (`.github/workflows/c-cpp.yml`) checks them out the same way.

```sh
make all          # builds bin/spider_cipher (CLI) and bin/spider_cipher_facts (tests)
make check        # runs facts tests against expected/ and round-trips encrypt/decrypt in all 3 formats
make expected     # regenerates expected/ outputs (only when output changes are intentional)
```

Tests use the `facts` framework (test registry in `src/spider_cipher_facts.c`):

```sh
bin/spider_cipher_facts                                # default tests (minutes, ~4 GB RAM)
bin/spider_cipher_facts --facts_include='PseudoShuffle'  # run a single named fact
bin/spider_cipher_facts --facts_include='*'            # ALL tests incl. FACTS_EXCLUDE'd ones — ~a week of runtime, 16 GB RAM, 200 GB disk; never run casually
```

CLI usage: `bin/spider_cipher [--decimal|--base40|--cards] --key='key' --encrypt='plain' --decrypt='cipher'` (`-` reads stdin).

### C++ implementation (`cpp/`)

Header-only C++11 ports in `cpp/include/` (`spider_cipher_core.hpp`, `spider_cipher_rng.hpp`, `spider_cipher_hash.hpp`), each with a test executable:

```sh
make -C cpp all   # builds cpp/bin/test_spider_cipher_{core,rng,hash}; run them directly
```

### Minimal C core (`core/`, tested via `core-tests-old/`)

Dependency-free C core with a `spider_cipher_*` API. Deck size and shuffle variant are compile-time parameters: `make -C core-tests-old CARDS=40 PERFECT=0 ...` sets `-DSPIDER_CIPHER_CARDS` and `-DSPIDER_CIPHER_PERFECT` (perfect back-front shuffle swaps first/last cards).

### Papers (LaTeX)

```sh
make theory               # build/theory.pdf from theory.ltx via tectonic; figures from figures/*.asy via asymptote
make -C theory            # theory/ paper via latexmk + lualatex
```

### Cards & misc

- `cards/deck.py` generates `card-NN.svg` from the template; `cards/deck-export` renders 600 dpi PNGs via Inkscape (macOS app path hardcoded).
- `./setup.sh` creates a conda/mamba env at `.venv` from `environment.yml` (used for Python/doc tooling; see `context.sh` for the helper functions it sources).

## Architecture

**Cipher model.** The deck itself is the cipher state. Encrypting one card: derive a "pad"/"noise" card from the current deck order, add it mod 40 to the plaintext card, then advance the deck by a cut followed by a "back-front" shuffle (`deckPseudoShuffle` / `spider_cipher_update`). Decryption subtracts the pad and applies the same deck advance, so both sides' decks stay synchronized. Key terms used throughout the code: cut card (`deck[0]`), mark card (derived from `deck[2]`), pad/noise, back-front shuffle, pseudo-shuffle.

**Constant-time discipline.** Deck operations in the C implementations (cut, find, shuffle) are deliberately written fixed-time/branch-free with respect to secret values to resist side channels. Preserve this property when modifying them — an "obvious" simplification that branches on card values breaks the design intent.

**Three software generations**, intentionally coexisting:

1. `src/` + `include/` — the full-featured reference. I/O is layered on the `CIO` stream abstraction from the sibling `cio`/`utf8` repos (`CIOTranslate`, `CIOCardsFmt`, etc.), supporting decimal/base40/card-glyph text formats and envelope encryption with a deniable-output channel (`encryptEnvelopeIO`/`decryptEnvelopeIO`).
2. `core/` — a clean, dependency-free C core (`spider_cipher_init/update/scramble/unscramble`, OS-RNG shuffle in `spider_cipher_rng.c`), parameterized by deck size.
3. `cpp/` — header-only C++ ports of core, RNG, and a hash construction.

When changing cipher behavior, keep the implementations consistent with each other and with `expected/spider_cipher_facts.out` (that file is the regression baseline for `make check`).

**Hardware & physical artifacts.** `src/*.v` is a Verilog true-RNG (45 ring oscillators → 3-stage synchronizers → uniform 0..39 in `rng40.v`); `die/` holds OpenSCAD dice models; `cards/` generates the printable deck. `docs/` is a browser-based step-by-step animation of the cipher (`index.html` + `demo.js`).

**Papers.** `paper.ltx` (overview/history), `theory.ltx` + `theory/` (security analysis), bibliography in `spider.bib`.
