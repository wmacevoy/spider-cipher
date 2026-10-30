# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

@AGENTS.md
@CONTRIBUTING.md

The two imports above load the working agreement (a copy of the software-engineering guide's
AGENTS.md, not edited here) and the repository guide (how to build, run, check, and reset; local
rules; where the working set is). This file adds the architecture.

## What this is

The Spider Cipher is a manual encryption scheme operated with a 40-card deck (values 0–39, mod-40 "base40" arithmetic). This repo holds several implementations of the cipher (C reference, minimal C core, C++ headers, Verilog hardware RNG, JS browser demos), tooling for the physical artifacts (card SVGs, 3D-printed dice), and the academic papers describing it.

## Architecture

**Cipher model.** The deck itself is the cipher state. Encrypting one card: derive a "pad"/"noise" card from the current deck order, add it mod 40 to the plaintext card, then advance the deck by a cut followed by a "back-front" shuffle (`deckPseudoShuffle` / `spider_cipher_update`). Decryption subtracts the pad and applies the same deck advance, so both sides' decks stay synchronized. Key terms used throughout the code: cut card (`deck[0]`), mark card (derived from `deck[2]`), pad/noise, back-front shuffle, pseudo-shuffle.

**Constant-time discipline.** Deck operations in the C implementations (cut, find, shuffle) are deliberately written fixed-time/branch-free with respect to secret values to resist side channels. Preserve this property when modifying them — an "obvious" simplification that branches on card values breaks the design intent.

**Three software generations**, intentionally coexisting:

1. `src/` + `include/` — the full-featured reference. I/O is layered on the `CIO` stream abstraction from the sibling `cio`/`utf8` repos (`CIOTranslate`, `CIOCardsFmt`, etc.), supporting decimal/base40/card-glyph text formats and envelope encryption with a deniable-output channel (`encryptEnvelopeIO`/`decryptEnvelopeIO`).
2. `core/` — a clean, dependency-free C core (`spider_cipher_init/update/scramble/unscramble`, OS-RNG shuffle in `spider_cipher_rng.c`), parameterized by deck size and variant (`SPIDER_CIPHER_PERFECT` swaps shuffled positions 0 and 19).
3. `cpp/` — header-only C++ ports of core, RNG, and a hash construction.

When changing cipher behavior, keep the implementations consistent with each other and with `expected/spider_cipher_facts.out` (that file is the regression baseline for `make check`).

**Hardware & physical artifacts.** `src/*.v` is a Verilog true-RNG (45 ring oscillators → 3-stage synchronizers → uniform 0..39 in `rng40.v`); `die/` holds OpenSCAD dice models; `cards/` generates the printable deck.

**Browser.** `docs/` is the GitHub Pages site: the overview (`index.html`, the talk's QR code target), the older Spiderweb tool (`spiderweb.html`) and its step-by-step animation (`explanation.html` + `demo.js`, an older two-cut form of the cipher), the ten-card math talk (`docs/talk/`, whose `talk.js` is the shared slide engine), and the 40-card system deck (`docs/forty/`, whose `spider40.js` ports `src/spider_cipher.c`). The working set (`REQUIREMENTS_v3`, `DESIGN_v1`, `QUEUE`, `FINDINGS`) also lives in `docs/`.

**Papers.** `paper.ltx` (overview/history), `theory.ltx` + `theory/` (security analysis), `foundations.ltx` (vertex transitivity, connectivity, free depth), `perfect-theory.ltx` (the perfect variant), bibliography in `spider.bib`.
