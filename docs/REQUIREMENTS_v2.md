# Requirements — v2

What must be observably true, and what is not being built. Changes only with the client; a change is
a new version (`REQUIREMENTS_v3.md`), never an edit to this one. See
[SE1/R1–R11](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/04-requirements.md).

Client: Warren MacEvoy, who designed the cipher and maintains the repository. The users who are never
in the room — people who will encrypt by hand, readers of the papers and talks, students — have no
proxy yet besides the issue tracker *(inferred)*
([SE1/H5](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/09-people.md#h5)).
· Drafted: 2026-10-29, reconstructed from the code, the papers, the slides, and the history
([SE1/W12](https://github.com/wmacevoy/software-engineering/blob/v1.3/guide/08-work.md#w12)); not
yet read back. Every requirement below is *(inferred)* until the client confirms it. *Tell us what is
wrong with it.*
· v2: 2026-10-30, from v1 with the client's answers about the end of message. Changed: TR-2
confirmed; PK-2 added (a changed packet is reported, not decrypted); the inputs table, the security
section, the scope decisions, and the non-goals follow from them; the integrity question is answered.
Everything else is as in v1.

## Glossary

| Term | Meaning |
| ---- | ------- |
| card | A value 0–39: suit × 10 + face, faces Q A 2–9 = 0–9, suits ♣ ♦ ♥ ♠ = 0–3. |
| deck | An order of the 40 cards, front (position 0) to back; the cipher's whole state. |
| key deck | The deck both sides start every message from. |
| tag card | The third card (position 2) minus 1. The code calls it the *mark card*; see *Not yet answered*. |
| noise | The card after the tag card, wrapping to the front. The code and the math talk also say *pad* / *cipher pad*. |
| cut card | The first card (position 0) plus the clear card. |
| cut | Move the cards before a chosen card to the back, as one block, so it is first. |
| back-front shuffle | Deal from the front into a new pile, alternately to its back and its front; card 2i goes to position 20 + i, card 2i+1 to 19 − i. |
| step | One card's work: noise, add (or subtract), cut on the cut card, back-front shuffle. The code calls the deck move a *pseudo-shuffle*. |
| clear card / scrambled card | A packet card before and after its step. |
| translate | Text to cards: three shift states (normal, up, down), a hex state for other Unicode, and shift cards 36–39. |
| shift card | 36 ↓ and 37 ↑ shift for one character; 38 ⤓ and 39 ⤒ lock. |
| end of message | Five or more 39s ending a translated message, padding it to a multiple of 10. It is also the reader's check that the packet arrived unchanged (PK-2). |
| packet | Ten random cards, then a random card before each message card. The code calls it the *envelope*. |
| deniable channel | The random cards of a packet, which decrypt to a second stream the reader may ignore. |
| passphrase key | The key deck made by scrambling a translated passphrase through a sorted deck. |
| plain / perfect variant | The step without / with a swap of shuffled positions 0 and 19 after the shuffle. |
| facts | The test framework (`../facts`) and the machine-checked claims it runs. |
| free depth, girth, relator | Graph quantities of the deck walk; defined in `foundations.ltx`. |

## The pain

Modern ciphers are invisible: almost nobody can watch one work, the software and hardware that run
them age and can be compromised, and keys outlive the tools that made them. Today, someone who wants
a cipher they can audit by hand, or that their heirs can still run in a hundred years, has only
historical hand ciphers, which are broken or impractical *(inferred, from the original slides and
`paper.ltx`)*.

## People this touches

- People who encrypt and decrypt by hand, with cards and two-digit arithmetic.
- People who use the command-line tool or the browser demos.
- Readers of the papers, who rely on the machine-checked facts the papers cite.
- Students and audiences of the talks.
- Anyone who receives a message: a desynchronized deck garbles everything after the error.

## Requirements

- **DK-1** *(since v1)* *(inferred)* Cards are numbered 0–39 as suit × 10 + face (Q A 2–9 = 0–9;
  ♣ ♦ ♥ ♠ = 0–3), and all card arithmetic is mod 40.
- **ST-1** *(since v1)* *(inferred)* Scrambling a clear card x with deck d outputs (x + noise) mod 40,
  where noise is the card after (d[2] − 1); the deck then becomes the back-front shuffle of d cut on
  the card (d[0] + x) mod 40. For a given deck and card, the output and the next deck are fixed.
- **ST-2** *(since v1)* *(inferred)* Unscrambling the outputs of ST-1, starting from the same deck,
  returns every clear card, and leaves the reader's deck equal to the writer's after every card.
- **TR-1** *(since v1)* *(inferred)* Translating any Unicode string to cards and back returns the
  same string, including characters outside the three shift tables (written in hex).
- **TR-2** *(since v1)* A translated message ends with at least five 39s, and its length is a
  multiple of 10; reading stops at the first run of five 39s. Confirmed by the client on 2026-10-30.
- **PK-1** *(since v1)* *(inferred)* A packet is ten random cards, then a random card before each
  message card; its length is 10 + 2 × (message length). Decrypting discards the random cards from
  the message (they go to the deniable channel).
- **PK-2** *(since v2)* Decrypting a packet whose message does not reach a run of five 39s prints no
  message, says on stderr that the packet was changed or the key is wrong, and exits nonzero.
  Changing any one card before that run is rejected this way, except by chance about once in 40⁵
  (about 10⁸) tries *(estimated, assuming the garbled cards behave like random ones)*: the decks fall
  out of step, and the run does not come out. A card changed after the run changes nothing in the
  message. Decided by the client on 2026-10-30.
- **KY-1** *(since v1)* *(inferred)* `--key='phrase'` makes the key deck by scrambling the translated
  phrase (with its end of message) through a sorted deck; the same phrase always gives the same deck.
- **CL-1** *(since v1)* *(inferred)* `bin/spider_cipher` encrypts with `--encrypt=` and decrypts with
  `--decrypt=` (`-` reads stdin) in `--decimal`, `--base40`, or `--cards` format; output is in groups
  of five, rows of twenty; a message encrypted and decrypted with the same key comes back unchanged.
- **CT-1** *(since v1)* *(inferred)* In the C implementations, the find, cut, noise, and shuffle
  operations take the same branches and memory accesses whatever the card values are.
- **IM-1** *(since v1)* *(inferred)* `core/`, `cpp/`, and `docs/forty/spider40.js` give the same output
  as `src/` for the same deck, input, and variant.
- **HD-1** *(since v1)* *(inferred)* Every step can be done by one person with the cards: reading the
  first and third cards, finding a card, one cut, one back-front shuffle, and two-digit arithmetic.
- **TH-1** *(since v1)* *(inferred)* Every fact the papers cite as machine-checked (`FACTS(...)`)
  exists in `src/spider_cipher_facts.c` and passes, in the default suite or by the command the paper
  or the guide names for it.

## Inputs: absent, late, wrong, lying

| Input | Absent | Late | Wrong | Lying |
| ----- | ------ | ---- | ----- | ----- |
| Message text | Empty text still makes a 30-card packet of end-of-message cards (seen). | n/a | Any Unicode is accepted (TR-1). | n/a: the writer chooses it. |
| Scrambled packet | Decrypt prints an empty line (seen). | n/a | A changed card garbles the rest of the message, and the run of five 39s does not come out; decrypt must reject it (PK-2). Today the CLI prints the garbage and exits 0 (seen 2026-10-30; queued). | An altered packet is rejected the same way (PK-2), except by chance about once in 40⁵ tries. |
| Key passphrase | Without `--key`, the CLI crashes (segmentation fault, exit 139; seen 2026-10-29). | n/a | A different phrase gives a different deck and garbage, with no error (seen). | n/a |
| Random source (`/dev/urandom`) | `fopen` is not checked; a missing device is undefined behavior *(read in the code, not tested)*. | Blocks. | A biased source weakens the packet's randomness, undetected. | Same as wrong. |
| Physical deck (by hand) | n/a | n/a | One wrong cut or shuffle desynchronizes every card after it. | n/a |

## Operating envelope

- **Scale:** one message at a time, by hand or one process; messages of a few thousand cards. Not
  designed for bulk or streaming encryption *(inferred)*.
- **Speed:** by hand, about a minute per card *(invented)*; the default facts suite in under
  5 minutes on CI (`FACTS_TIMEOUT=300`); measured 6 s optimized and 15 s unoptimized on 2026-10-29.
- **Recovery:** a desynchronized deck is recovered only by starting again from the key deck.
- **Availability:** n/a — no service.
- **Resources:** the default suite's memory was not measured (an earlier note says ~4 GB); the full
  suite needs about a week, 16 GB RAM, and 200 GB disk (README).
- **Interruption:** an interrupted hand session can resume only from a written-down deck order.

## Security, law, and ethics

Threat model *(inferred)*: an adversary sees every scrambled packet, and may know or choose some
plaintext; the key deck is shared out of band. The key space is 40! ≈ 2¹⁵⁹. Side channels are
addressed in the C code (CT-1). Altered packets are detected by the end of message (PK-2), except by
chance about once in 40⁵ tries; the CLI does not yet act on it. No data
about people is collected or kept by any part of this repository. The papers disclose AI assistance.

## Assumptions (K4s), read back to the client

- "We're assuming the C reference in `src/` defines the cipher, and the Google Slides deck's older
  step (cut on the tag card, shuffle, cut on the cut card) is retired. True?" — *not yet asked*
- "We're assuming the plain variant is the cipher of record for `src/` and the CLI, and the perfect
  variant is an option in `core/` and `cpp/`. True?" — *not yet asked*

## Scope decisions

- The cipher must stay operable by hand (HD-1), which rules out any step a person cannot do with
  cards *(inferred)*.
- Three implementations coexist (`src/`, `core/`, `cpp/`) plus the browser port; `src/` is the
  reference *(inferred, from CLAUDE.md)*.
- The working set lives in `docs/` and is published with the Pages site — decided by the owner on
  2026-10-29.
- `make check` is the one check command, run the same way in CI — decided by the owner on 2026-10-29.
- The end of message stays five or more 39s to a multiple of 10, as in `src/`; the original slides'
  form (four or more 39s to a multiple of 5, then random cards to a multiple of 10) is retired — five
  39s make the check 40 times stronger — decided by the owner on 2026-10-30.
- A changed packet is a gap, not a non-goal: it is reported (PK-2) — decided by the owner on
  2026-10-30.

## Non-goals

- Not a key-exchange scheme: the key deck is shared out of band *(inferred)*.
- Not speed: the design favors hand operation over throughput *(inferred)*.

## Not yet answered

- One word each for *tag* / *mark* and *noise* / *pad* (SE1/D9): the code, the Google Slides, and the
  talks use both. — Warren
- Should the perfect variant replace the plain one in `src/` and the CLI? — Warren

## Deferred

- Not yet: a tested answer for a missing `/dev/urandom` — no one has hit it; a port to a system
  without it would trigger it.
