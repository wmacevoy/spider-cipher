# Queue

What is being worked, in what order, by whom, and what would make it done. The shared cursor
([SE1/W10](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/08-work.md#w10)).
Nothing enters without a *done* sentence that can be checked from outside — "it works" is not one.
An item that contradicts the requirements is a question for the client, not a task; mark it blocked,
and say on what. Only a human marks an item *done*.

| # | Item | Versioned requirement IDs (or *chore*) | Owner | Done when | Estimate (number / range / time box) | Status | Since |
| - | ---- | ---------------------------- | ----- | --------- | ------------------------------------ | ------ | ----- |
| 1 | Read back REQUIREMENTS_v1, which is entirely *(inferred)*; answer *Not yet answered* (integrity, tag/mark and noise/pad, perfect in `src/`) | REQ1/DK-1 through REQ1/TH-1 | Warren | each requirement is confirmed, changed in a `REQUIREMENTS_v2`, or dropped, and each open question has a dated answer | time box: 1 hour | ready | 2026-10-29 |
| 2 | The CLI crashes without `--key` | REQ1/CL-1 | unassigned | `bin/spider_cipher --encrypt=x` with no `--key` exits nonzero with a message on stderr, and `make check` runs that case and fails without the fix | time box: 1 hour | ready | 2026-10-29 |
| 3 | Commit the cross-check of `docs/forty/spider40.js` against `bin/spider_cipher` and run it in CI | REQ1/IM-1, REQ1/TR-1 | unassigned | a script in the repository encrypts with each and decrypts with the other in both formats, compares the translated cards, and CI runs it; it fails if one side's encoder changes | 2–4 hours | blocked: on Warren — it adds `node` to CI (a new build step) | 2026-10-29 |
| 4 | Check that `core/` and `cpp/` agree with `src/` | REQ1/IM-1 | unassigned | `make check` steps the same decks and inputs through `src/`, `core/` and `cpp/` (plain and perfect where each has it) and fails on any difference | 1–2 days | ready | 2026-10-29 |
| 5 | Make the facts name the requirements they assert | REQ1/ST-1, REQ1/TR-1, REQ1/TH-1 | unassigned | each REQ1 ID with a test is named, versioned, in the fact that asserts it, and `./se --doctor`'s trace row sees them | 2–4 hours | blocked: on Warren — `se` reads only paths containing "test", and `src/spider_cipher_facts.c` has none: rename it (a move) or accept the warning | 2026-10-29 |
| 6 | Update the Google Slides deck and `docs/demo.js` to the current step and packet | REQ1/ST-1, REQ1/PK-1 | Warren | the scramble slide shows one cut on first ⊕ clear then the shuffle, the packet slide pads with 39s, and `docs/demo.js` does the same | time box: 2 hours | ready | 2026-10-29 |
| 7 | A test for constant-time deck operations | REQ1/CT-1 | unassigned | a test runs find, cut, and noise on different card values and fails when a branch on a card value is added to any of them | time box: 1 day | ready | 2026-10-29 |
| 8 | Remove the timestamps from the facts output | chore | Warren | the facts binary prints no wall-clock time or duration | 30 minutes | blocked: on Warren — asked for on 2026-10-29 while the CI failure was misdiagnosed; they go to stderr, which the check ignores, and they report progress in the week-long runs | 2026-10-29 |
| 9 | Move CI to `actions/checkout@v4` | chore | unassigned | the workflow uses `actions/checkout@v4` and its runs show no deprecation warning for the action | 15 minutes | ready | 2026-10-29 |
