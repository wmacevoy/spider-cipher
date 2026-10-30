# Design — v1

How we intend to satisfy the requirements, and what we rejected. Every mechanism here should be
replaceable without renegotiating a requirement
([SE1/R5](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/04-requirements.md)).
Reconstructed on 2026-10-29 from the code and history; reasons nobody wrote down are *(inferred)*
([SE1/W12](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/08-work.md#w12)).

## Components and the dependency rule

| Component | Layer | Kind | Contract (one line) | Holds state? |
| --------- | ----- | ---- | ------------------- | ------------ |
| `../cio`, `../utf8`, `../facts` | 0 | sibling repositories, compiled in | character streams, UTF-8, the test framework | no |
| `include/spider_cipher.h`, `src/spider_cipher.c` | 1 | C library (reference) | deck operations, step, translate, packet (envelope), card formats | the caller's deck |
| `src/spider_cipher_main.c` | 2 | CLI | `--key`, `--encrypt`, `--decrypt`, three formats (REQ1/CL-1) | no |
| `src/spider_cipher_facts.c` | 2 | tests | the facts, including those the papers cite (REQ1/TH-1) | no |
| `core/` | 1 | C library (minimal) | `spider_cipher_init/update/scramble/unscramble`, deck size and variant at compile time | the caller's deck |
| `cpp/include/` | 1 | C++ headers | ports of core, RNG, and a hash | the caller's deck |
| `docs/forty/spider40.js` | 1 | JS module | a port of `src/spider_cipher.c`'s translate, packet, step, and passphrase key | no |
| `docs/talk/talk.js` | 2 | browser | slide engine and animated deck; both decks load it | the page |
| `docs/talk/`, `docs/forty/` | 3 | browser decks | the ten-card talk and the 40-card summary | no |
| `src/*.v` | — | Verilog | a hardware RNG for cards 0–39 | registers |
| `*.ltx`, `theory/` | — | papers | proofs; cite facts by name | no |

The rule that a component depends only on lower layers is not enforced by any check
([SE1/D10](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/05-design.md#d10)); it holds
today by reading the includes *(inferred)*.

## State

| State | Owner (authoritative) | Copies | Max staleness | While wrong, the user sees |
| ----- | --------------------- | ------ | ------------- | -------------------------- |
| The deck during a message | the writer | the reader's deck | zero: one missed or wrong step desynchronizes every card after it | garbled text after the error, with no error message |
| The key deck | the two people who share it | written-down orders, physical decks | none: must be identical | garbage from the first card |
| The cipher definition | `src/spider_cipher.c` | `core/`, `cpp/`, `spider40.js`, the Google Slides deck, `paper.ltx`, `docs/demo.js` | until someone changes one; no check compares them except the 2026-10-29 cross-check run by hand | demos or slides that disagree with the tool |
| The regression baseline | the current facts output | `expected/spider_cipher_facts.out` | any edit to `src/spider_cipher_facts.c` (line numbers are in it) | a red `make check` |
| The working agreement | the guide's release (v1.3.2) | `AGENTS.md`, `se.py` | until the next guide release; `./se --doctor` reports it | rules that differ from the guide's |

## Essential complexity

- The deck is the only state, so writer and reader stay in lockstep only if the cut uses the clear
  card, which the reader has just recovered (REQ1/ST-1, REQ1/ST-2).
- Forty cards carry 0–39 only, so text needs shift states and a hex escape (REQ1/TR-1).
- The same message must not scramble the same way twice, so random cards precede and interleave it
  (REQ1/PK-1).
- A person must be able to do every step (REQ1/HD-1), which limits each step to a find, a cut, a
  shuffle, and two-digit arithmetic.
- Side channels in software: operations on secret cards are branch-free (REQ1/CT-1).
- Several copies of one cipher must agree (REQ1/IM-1).

## One-way doors

- **The cipher definition** (step, tag/noise rule, shuffle, variant): changing it makes every
  existing packet, printed instruction, and slide wrong. Reviewed by: nobody recorded *(inferred)*.
- **The translation table and end-of-message rule:** the same, for every translated message.
- **The packet format:** the same, for every packet.

## Decisions

- **One cut per step, on first ⊕ clear, then the shuffle** — REQ1/ST-1, REQ1/HD-1 — fewer moves per
  card than the older step (cut on the tag card, shuffle, cut on the cut card) still shown in the
  Google Slides deck and `docs/demo.js` — rejected: the older step, for its extra cut *(inferred;
  no source found)*.
- **Branch-free deck operations** — REQ1/CT-1 — cache and timing side channels; the JS port is not
  hardened — source: `paper.ltx`, *Implementation*.
- **Perfect variant = swap shuffled positions 0 and 19** — REQ1/IM-1 — keeps the shortest
  single-cut loop at 30; the first/last (0,39) swap dropped it to 6 — source: commit 204d0f6,
  2026-06-12.
- **Pad the message to a multiple of 10 with at least five 39s** — REQ2/TR-2, REQ2/PK-2 — the run
  is also the reader's check that nothing changed, so it ends every message; rather than four 39s to
  a multiple of 5 and then random cards (the Google Slides' "C" cards), which made the check 40 times
  weaker — source: the owner, 2026-10-30.
- **Hex digit count: s bits must hold the code point (`<=`)** — REQ1/TR-1 — fixed 2026-10-29,
  commit 8648919; see FINDINGS.
- **`make check` passes `--facts_timeout=$(FACTS_TIMEOUT)`, default 300 s** — REQ1/TH-1 — facts' own
  10 s default is shorter than an unoptimized run — source: 2026-10-29, see FINDINGS.
- **Browser decks share `docs/talk/talk.js`** — rather than a copy per deck — source: 2026-10-29.
- **Papers build with latexmk + LuaLaTeX in Docker on `devcontainer-latex`, as `../luck` does** —
  REQ1/TH-1 — one toolchain across the owner's TeX projects, and `tools/check-log.sh` fails a build
  that silently loses content; rather than tectonic, whose `pdflatex` shim cannot answer `asy`'s
  interactive label queries, so `make theory` hung — source: the owner, 2026-10-30.
- **Decrypt holds the message until the run of 39s arrives** — REQ2/PK-2 — the CLI decodes into a
  buffer and prints it only if `decodeIO` succeeds, so a changed packet prints nothing; rather than
  printing as it reads and reporting at the end, which shows the garbage — source: 2026-10-30. The
  browser port does the same (`complete` in `spider40.js`).

## Deferred

- Not yet: a check that the copies of the cipher agree (REQ1/IM-1) — the cross-check exists only as a
  script run by hand on 2026-10-29; see QUEUE.
- Not yet: a test for REQ1/CT-1 — none exists.
