"use strict";
// The 40-card Spider cipher: translate, packet, scramble.
// A port of src/spider_cipher.c (encodeIO, decodeIO, the envelope, deckAdvance)
// for the browser; it runs under node too, for testing against bin/spider_cipher.

(function (root) {
  const CARDS = 40, PREFIX = 10;
  const SHIFT_DOWN = 36, SHIFT_UP = 37, SHIFT_LOCK_DOWN = 38, SHIFT_LOCK_UP = 39;

  const cp = s => [...s].map(c => c.codePointAt(0));
  //            Q    A    2    3    4    5    6    7    8    9
  const DOWN = cp("0123456789" + "ABCDEF@=\\~" + "#$%^&|-+/*" + "\n;?'").concat([0x1F622, 0x1F604]);
  const NORMAL = cp("abcdefghij" + "klmnopqrst" + "uvwxyz<>()" + " ,.\"").concat([0x1F44E, 0x1F44D]);
  const UP = cp("ABCDEFGHIJ" + "KLMNOPQRST" + "UVWXYZ{}[]" + "_:!`").concat([0x1F494, 0x2764]);
  const CODES = [DOWN, NORMAL, UP];

  const FACES = "QA23456789";
  const SUITS = ["♣", "♦", "♥", "♠"]; // clubs, diamonds, hearts, spades

  const mod = (a, n) => ((a % n) + n) % n;

  // card position of code point `code` in shift state `shift`, or -1;
  // shift -1 is the hex state, which takes any code point not in a table
  function ord(shift, code) {
    if (code === undefined || code < 0) return -1;
    if (shift >= 0) return CODES[shift].indexOf(code);
    for (const t of CODES) if (t.includes(code)) return -1;
    return code;
  }

  // text -> cards, including the end-of-message 39s
  function encode(text) {
    const s = typeof text === "string" ? cp(text) : text;
    const out = [];
    let i = 0, shift = 1, next = 1;
    const peek = k => (i + k < s.length ? s[i + k] : -1);
    for (;;) {
      const chr = peek(0);
      if (chr < 0) break;
      const pos = ord(shift, chr);
      if (pos >= 0) {
        ++i;
        if (shift === -1) {
          let b = 4;
          while (2 ** b <= chr) b += 4;
          while (b >= 4) { b -= 4; out.push(Math.floor(chr / 2 ** b) % 16); }
          if (ord(-1, peek(1)) >= 0) { out.push(SHIFT_LOCK_DOWN); next = -1; }
          else { out.push(SHIFT_LOCK_UP); next = 0; }
        } else {
          out.push(pos);
        }
        shift = next;
        continue;
      }
      // how far each shift state can run (up to PREFIX characters while tied)
      const en = [0, 0, 0, 0];
      for (let el = 0; el < PREFIX; ++el) {
        let ties = 0;
        for (let q = -1; q < 3; ++q) {
          if (en[q + 1] === el && ord(q, peek(el)) >= 0) { ++en[q + 1]; ++ties; }
        }
        if (ties < 2) break;
      }
      let m = -1;
      for (let q = 0; q < 3; ++q) {
        if (en[m + 1] < en[q + 1] || (en[m + 1] === en[q + 1] && Math.abs(q - next) <= Math.abs(m - next))) m = q;
      }
      const lock = m === -1 || en[m + 1] > 1 || ord(next, peek(1)) < 0;
      while (shift !== m) {
        if (shift < m) { ++shift; out.push(lock ? SHIFT_LOCK_UP : SHIFT_UP); }
        if (m < shift) { --shift; out.push(lock ? SHIFT_LOCK_DOWN : SHIFT_DOWN); }
      }
      if (lock) next = shift;
    }
    for (let k = 0; k < PREFIX / 2 || out.length % PREFIX !== 0; ++k) out.push(CARDS - 1);
    return out;
  }

  // cards -> text; stops at five 39s in a row
  function decode(cards) {
    let text = "", shift = 1, next = 1, unicode = -1, pad = 0, on = true;
    const emit = c => { if (on && c <= 0x10FFFF) text += String.fromCodePoint(c); shift = next; };
    for (const card of cards) {
      if (card === CARDS - 1) { if (++pad >= PREFIX / 2) on = false; }
      else if (pad < PREFIX / 2) pad = 0;
      if (shift === -1) {
        if (card >= 0 && card < 16) unicode = (unicode > 0 ? 16 * unicode : 0) + card;
        else if (card >= 20 && card < 30) unicode = (unicode > 0 ? 10 * unicode : 0) + (card - 20);
        else { if (unicode >= 0) emit(unicode); unicode = -1; }
      }
      if (shift >= 0 && card >= 0 && card < 36) emit(CODES[shift][card]);
      if ((card === SHIFT_UP || card === SHIFT_LOCK_UP) && shift <= 1) ++shift;
      if ((card === SHIFT_DOWN || card === SHIFT_LOCK_DOWN) && shift >= 0) --shift;
      if (card === SHIFT_LOCK_DOWN || card === SHIFT_LOCK_UP) next = shift;
    }
    return text;
  }

  // ---- the deck ----

  const sorted = () => [...Array(CARDS).keys()];
  const cut = (d, k) => d.map((_, i) => d[(i + k) % d.length]);
  function shuffle(d) {
    const n = d.length, h = n / 2, o = new Array(n);
    for (let i = 0; i < h; ++i) { o[h + i] = d[2 * i]; o[h - 1 - i] = d[2 * i + 1]; }
    return o;
  }
  // tag = third card minus 1; noise = the card after the tag; cut card = first card plus the clear card
  function look(d) {
    const tag = mod(d[2] - 1, CARDS), at = d.indexOf(tag);
    return { tag, at, noise: d[(at + 1) % CARDS] };
  }
  // one step: returns [output card, next deck]; mode 1 scrambles, -1 unscrambles
  function step(d, card, mode) {
    const { noise } = look(d);
    const clear = mode === 1 ? card : mod(card - noise, CARDS);
    const out = mode === 1 ? mod(card + noise, CARDS) : clear;
    const cutCard = mod(d[0] + clear, CARDS);
    return [out, shuffle(cut(d, d.indexOf(cutCard)))];
  }
  function run(d, cards, mode) {
    const out = [];
    for (const c of cards) { const [o, nd] = step(d, c, mode); out.push(o); d = nd; }
    return [out, d];
  }

  // a passphrase key: scramble the encoded passphrase through a sorted deck
  const keyDeck = phrase => run(sorted(), encode(phrase), 1)[1];

  function randomCard() {
    const b = new Uint8Array(1);
    for (;;) { crypto.getRandomValues(b); if (b[0] < 240) return b[0] % CARDS; }
  }

  // packet: ten random cards, then a random card before each message card
  function packet(msg, rand = randomCard) {
    const p = [];
    for (let k = 0; k < PREFIX; ++k) p.push(rand());
    for (const c of msg) p.push(rand(), c);
    return p;
  }
  const unpacket = p => p.slice(PREFIX).filter((_, i) => i % 2 === 1);

  function encrypt(deck, text, rand) {
    const msg = encode(text), pkt = packet(msg, rand), [cipher] = run(deck, pkt, 1);
    return { msg, pkt, cipher };
  }
  function decrypt(deck, cipher) {
    const [pkt] = run(deck, cipher, -1), msg = unpacket(pkt);
    return { pkt, msg, text: decode(msg) };
  }

  // ---- formats ----

  const two = c => String(c).padStart(2, "0");
  const glyph = c => FACES[c % 10] + SUITS[Math.floor(c / 10)];
  // groups of five, rows of twenty, like the CLI
  function format(cards, as = two) {
    return cards.map((c, i) => (i === 0 ? "" : i % 20 === 0 ? "\n" : i % 5 === 0 ? "|" : " ") + as(c)).join("");
  }
  // reads decimal (07 31 …) or card glyphs (7♣ A♠ …)
  function parse(s) {
    const out = [];
    for (const m of s.matchAll(/([QA2-9])\s*([♣♦♥♠])|(\d{1,2})/g)) {
      const c = m[3] !== undefined ? Number(m[3]) : SUITS.indexOf(m[2]) * 10 + FACES.indexOf(m[1]);
      if (c >= 0 && c < CARDS) out.push(c);
    }
    return out;
  }

  const api = { CARDS, PREFIX, CODES, FACES, SUITS, mod, encode, decode, sorted, cut, shuffle, look, step, run,
    keyDeck, randomCard, packet, unpacket, encrypt, decrypt, two, glyph, format, parse };
  if (typeof module !== "undefined") module.exports = api;
  else root.Spider40 = api;
})(this);
