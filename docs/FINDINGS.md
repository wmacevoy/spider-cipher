# Findings

What we learned the expensive way: surprises, dead ends, measurements, the attack that worked. The
only document reality writes
([SE1/W10](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/08-work.md#w10)).
Blameless: say what would have found it, not who should have asked
([SE1/H12](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/09-people.md#h12)).
Started 2026-10-29; earlier entries are reconstructed from the history.

## 2026-10-30 — `make theory` hung forever on its first figure

- **What happened:** `asy -tex pdflatex` keeps `pdflatex` open and asks it to measure each label,
  one at a time. With no TeX Live installed, `tools/pdflatex` (untracked) stood in for it by passing
  the job to tectonic, which first reads its whole input: each side waited on the other, at 0% CPU.
  Without the shim, `asy` failed at once (no `pdflatex`). The Makefile's `ASY_TEXPATH` was never
  passed to `asy`.
- **Kind:** K2 (surprise)
- **Cost:** the papers could not be built on the maintainer's machine; the figures and the edits
  to `theory.ltx` were uncommitted.
- **Findable by:** building the papers from a clean checkout, with a time limit.
- **Changed:** the papers build with latexmk + LuaLaTeX in Docker (`./build.sh`), as `../luck`
  does. The first clean build showed `figures/venn.asy` drawing at the wrong scale (no
  `unitsize`).

## 2026-10-30 — Decrypt ignores its own check for a changed packet

- **What happened:** the run of five 39s that ends every message is a check: change one card and the
  decks fall out of step, so the run does not come out, and `decodeIO` returns −1. The CLI ignores
  that result (`src/spider_cipher_main.c:145`): with one card of a 30-card packet changed, it printed
  `hlhe<1p<t` and exited 0. REQUIREMENTS_v1, reconstructed from the CLI's behavior, listed message
  authentication as a likely non-goal; the owner meant the 39s as the check.
- **Kind:** K2 (surprise)
- **Cost:** a changed or wrongly keyed packet decrypts to garbage with no error; v1 nearly recorded
  the gap as a decision.
- **Findable by:** the lying column of the inputs table (SE1/R6), run as a test that changes one card.
- **Changed:** REQUIREMENTS_v2 adds PK-2; decrypt now rejects such a packet (CLI and the 40-card
  deck), and `make check` decrypts a fixed packet with one card changed.

## 2026-10-29 — CI was red on every push for at least seven months

- **What happened:** every `C/C++ CI` run listed, back to 2026-03-05, failed. In the runs whose
  logs remain (2026-08-11 on; older logs have expired, so their cause is unknown), the facts all
  passed; CI's first configuration builds without optimization, where the default suite takes about
  15 s, and facts kills a run at 10 s by default. The truncated output then failed the diff against
  `expected/`. Optimized local builds take 6 s, so `make check` passed on the maintainer's machine.
- **Kind:** K2 (surprise)
- **Cost:** seven months with no signal from CI; every push looked broken, so a real break would
  not have been seen.
- **Findable by:** reading a failed run's log once; running CI's exact command
  (`make -B CDBG="" COPT="" check`) locally.
- **Changed:** `make check` passes `--facts_timeout=$(FACTS_TIMEOUT)` (default 300); CONTRIBUTING
  says to reproduce a CI failure with CI's command before diagnosing it.

## 2026-10-29 — The CI failure was first misdiagnosed as timestamps in the baseline

- **What happened:** the first diagnosis compared the facts output captured with `2>&1` against
  `expected/` and blamed timestamps and timings. They go to stderr, which `make check` does not
  compare; the real cause was the timeout above. The misdiagnosis reached the owner, who asked for
  the timestamps to be removed, before it was corrected.
- **Kind:** K3 (we were sure, and wrong)
- **Cost:** a wrong report, and an instruction based on it.
- **Findable by:** running the check exactly as the Makefile does, with the same streams, before
  explaining its failure.
- **Changed:** the CONTRIBUTING rule above.

## 2026-10-29 — Hex escape dropped the leading digit of powers of 16

- **What happened:** characters outside the shift tables are written in hex; the digit count used
  `(1<<s) < chr`, so U+0010, U+0100, U+1000, U+10000, and U+100000 lost their leading 1 ("Ā"
  decrypted as U+0000). Found while porting the encoder to the browser, by reading the loop.
- **Kind:** K2 (surprise)
- **Cost:** those characters never survived a round trip since the encoder was written.
- **Findable by:** a round-trip test over boundary values (exact powers of the base), which the
  `Encode` fact's strings did not include.
- **Changed:** `<=` in `src/spider_cipher.c` and `docs/forty/spider40.js`; the boundary code points
  added to the test strings (commit 8648919).

## 2026-10-29 — Editing the facts file silently breaks `make check`

- **What happened:** adding seven test strings shifted every line number in the facts output, and
  the commit was pushed without running `make check`, so the baseline in `expected/` no longer
  matched.
- **Kind:** K2 (surprise)
- **Cost:** a broken check on `main` until `make expected` was rerun.
- **Findable by:** running the check command before every commit (the definition of done).
- **Changed:** CONTRIBUTING's rule for `expected/`.

## 2026-10-29 — The Google Slides deck describes an older cipher

- **What happened:** the presentation's scramble step cuts on the tag card, shuffles, then cuts on
  the cut card, and its packet ends with random "C" cards; `src/` cuts once, on first ⊕ clear, and
  pads the message with 39s. `docs/demo.js` also shows the older step.
- **Kind:** K2 (surprise)
- **Cost:** two descriptions of one cipher; a reader following the slides gets a different cipher.
- **Findable by:** a check that the copies of the cipher agree (REQ1/IM-1).
- **Changed:** the 40-card deck in `docs/forty/` follows `src/`; updating the slides is queued.

## 2026-06-12 — The perfect variant in `core/` and `cpp/` swapped the wrong cards

- **What happened:** `SPIDER_CIPHER_PERFECT` swapped the first and last cards (0,39) instead of the
  design's (0,19); the (0,39) form is connected, but its shortest single-cut loop is 6, worse than
  the plain cipher's 9. Fixed in commit 204d0f6.
- **Kind:** K3 (we were sure, and wrong)
- **Cost:** results computed for the wrong variant; `core-tests-old/src/cycles.c` still holds that
  variant's table.
- **Findable by:** a fact comparing each implementation's perfect step with the human form (now
  `PerfectHumanForm`).
- **Changed:** core and cpp swap (0,19); the facts prove the equivalence.

## 2026-10-29 — The CLI crashes without `--key`

- **What happened:** `bin/spider_cipher --encrypt='x'` with no `--key` ends in a segmentation fault
  (exit 139): `DeckKey` calls `strlen` on a null key.
- **Kind:** K2 (surprise)
- **Cost:** none yet known.
- **Findable by:** the absent column of the inputs table (SE1/R6).
- **Changed:** recorded in REQUIREMENTS_v1's inputs table; a fix is queued.
