# Contributing

How to work on this repository — for a new teammate and for an agent, who both arrive knowing
nothing
([SE1/A3](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/10-agents.md#a3)). Keep
it short and true: a change that makes a line here untrue updates it in the same change
([SE1/W11](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/08-work.md#w11)).
`CLAUDE.md` holds the architecture; this file holds the mechanics.

## Run it

- **Prerequisites:** a C11 compiler and `make`; the sibling repositories
  [facts](https://github.com/wmacevoy/facts), [utf8](https://github.com/wmacevoy/utf8), and
  [cio](https://github.com/wmacevoy/cio) checked out next to this one (`../facts`, `../utf8`,
  `../cio`). The Makefile compiles their sources directly. Optional: Docker and
  [devcontainer-latex](https://github.com/wmacevoy/devcontainer-latex) checked out at
  `../devcontainer-latex` for the papers (or a local TeX Live with `latexmk`, LuaLaTeX, and `asy`),
  `node` for the browser demos' cross-check, Python 3.9+ for `./se`.
- **Build:** `make all` — builds `bin/spider_cipher` (the CLI) and `bin/spider_cipher_facts` (the
  tests).
- **Run:** `bin/spider_cipher --cards --key='secret' --encrypt='message'` prints a scrambled packet
  in card glyphs; `bin/spider_cipher --key='secret' --decrypt='…'` reads it back (`--decimal`,
  `--base40`, `--cards` pick the format; `-` reads stdin).
- **Run the browser decks:** `python3 -m http.server -d docs 8000`, then open the overview at
  `http://localhost:8000/`, and the decks at `http://localhost:8000/talk/` and
  `http://localhost:8000/forty/`; the talk's handout is `talk/worksheet.html`, printed from the
  browser (two letter pages). GitHub Pages serves `docs/` from `main` at
  https://wmacevoy.github.io/spider-cipher/, the address in the talk's QR code.
- **Reset to a known state:** `rm -rf tmp bin && make all` — removes every object and binary,
  including ones `all` does not rebuild (`bin/spider_cipher_generic_facts`: `make` it by name);
  rebuilds in a few seconds. There is no `make clean` for the C build. The tests keep no state
  between runs.
- **Other implementations:** `make -C cpp all` builds `cpp/bin/test_spider_cipher_{core,rng,hash}`;
  run each directly. `make -C core-tests-old CARDS=40 PERFECT=0 …` builds the minimal C core with a
  chosen deck size and variant. Neither is part of the check yet (see `docs/QUEUE.md`).
- **Papers:** `./build.sh` builds `build/theory.pdf`, `build/perfect-theory.pdf`, and
  `build/foundations.pdf` in Docker, on the `devcontainer-latex` image, as `../luck` does
  (`./build.sh theory` for one). With a local TeX Live, `make papers` (or `make theory`) does the
  same. Each runs `latexmk -lualatex`, then `tools/check-log.sh`, which fails the build on silent
  content loss. `make -C theory` builds the `theory/` paper with `latexmk`.
- **Cards and tooling:** `cards/deck.py` generates `card-NN.svg` from the template;
  `cards/deck-export` renders 600 dpi PNGs with Inkscape (macOS app path hard-coded). `./setup.sh`
  creates a conda/mamba environment at `.venv` from `environment.yml` for the Python and document
  tooling (`context.sh` holds the helpers it sources).

## Check it

- **The full check:** `make check` — the same one CI runs
  ([SE1/O3](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/07-operating.md#o3))
- **Success prints:** no `diff` output and no `make: *** … Error`; `make` exits 0. The facts summary
  it compares ends with `… truths and 0 (0.0%) fictions checked.`
- **What it runs:** the default facts suite, its stdout diffed against
  `expected/spider_cipher_facts.out`, then an encrypt/decrypt round trip of `message` in all three
  formats, then two fixed packets: `expected/message.packet` must decrypt to `message`, and
  `expected/message-changed.packet` (one card changed) must be rejected. About 6 s optimized and 15 s unoptimized on an M-series Mac; `FACTS_TIMEOUT` (default 300 s)
  raises facts' own 10 s timeout, which the unoptimized build exceeds.
- **Reproduce CI exactly:** `make -B CDBG="" COPT="" check`, then the `-g` and `-O2` variants in
  `.github/workflows/c-cpp.yml`. A failure in CI is reproduced with CI's command before it is
  diagnosed.
- **One test:** `bin/spider_cipher_facts --facts_include='Encode'`. Everything, including the
  excluded week-long searches: `--facts_include='*'` with a raised `--facts_timeout` (a week, 16 GB
  RAM, 200 GB disk; never run casually).
- **Instruments:** none yet — no sanitizer, coverage, or linter runs.

## Rules for this repository

- **`expected/` is the regression baseline.** Facts output names source line numbers, so any edit
  to `src/spider_cipher_facts.c` changes it: run `make expected`, read the diff, and commit the new
  baseline only if every change is one you meant.
- **Constant time.** Deck operations on secret values (find, cut, noise, shuffle) are written
  without branches or memory accesses that depend on card values. An "obvious" simplification that
  branches on a card breaks the design; keep the property when editing them.
- **One cipher, several copies.** `src/` is the reference. `core/`, `cpp/`, and
  `docs/forty/spider40.js` must give the same output for the same deck and input; a change to the
  cipher changes all of them, or says which it leaves behind.
- **Perfect variant** means the post-shuffle swap of positions 0 and 19 (equivalently: cut, swap
  the second and bottom cards, then shuffle). The first/last (0,39) swap is retired.
- **Requirements are versioned, never edited in place:** `docs/REQUIREMENTS_v3.md` is current; it is
  followed by `_v4`, not rewritten, and earlier versions stay as they were.
- **The copy of AGENTS.md is not edited.** Local rules go here; this file wins where they disagree.

## Where things are

- **Working set:** `docs/` — `REQUIREMENTS_v3`, `DESIGN_v1`, `QUEUE`, `FINDINGS`
  ([SE1/W10](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/08-work.md#w10)).
  `docs/` is also the GitHub Pages site, so these are published too; that was chosen on 2026-10-29.
- **Agents:** read `AGENTS.md` in this repository (copied from the
  [guide](https://github.com/wmacevoy/software-engineering/blob/v1.3/AGENTS.md) at v1.3.2) and this
  file before acting. `CLAUDE.md` imports both, so Claude Code loads them at every start.
- **Code:** `src/` + `include/` (reference C and its facts), `core/` (dependency-free C core),
  `cpp/` (header-only C++), `src/*.v` (Verilog RNG), `docs/talk/` and `docs/forty/` (browser decks),
  `cards/` and `die/` (physical artifacts), `*.ltx`, `theory/` (papers).

### This repository's names for AGENTS.md's nouns

| AGENTS.md says                 | In this repository                                                       |
| ------------------------------ | ------------------------------------------------------------------------ |
| the repository guide           | `CONTRIBUTING.md` (with the architecture in `CLAUDE.md`)                 |
| the queue                      | `docs/QUEUE.md`                                                          |
| findings                       | `docs/FINDINGS.md`                                                       |
| the check command              | `make check`; success: `make` exits 0 with no diff output                |
| the handoff check              | `./se --post`, a `SessionStart` hook (startup, resume, compact) in `.claude/settings.json` |
| `Non-goals`, `Scope decisions` | sections of `docs/REQUIREMENTS_v3.md`                                    |

### Done

A change is done when the [working checklist](https://github.com/wmacevoy/software-engineering/blob/v1.3/CHECKLISTS.md#working)
holds: `make check` passes locally and in CI, a new test was seen to fail first, the change names
its requirement (`REQ3/…`) or says *chore*, and what it made untrue here or in `docs/` is updated in
the same change. Only the owner marks a queue item *done*.
