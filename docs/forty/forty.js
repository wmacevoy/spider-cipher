"use strict";
// Demos for the 40-card deck; uses DeckView, Demos, buttons from ../talk/talk.js and Spider40 from spider40.js.

const S40 = Spider40;
const SUIT_NAMES = ["clubs", "diamonds", "hearts", "spades"];
const isRed = c => Math.floor(c / 10) === 1 || Math.floor(c / 10) === 2;
const glyphHTML = c => `<span class="${isRed(c) ? "red" : ""}">${S40.glyph(c)}</span>`;
const SHOW = { " ": "␣", "\n": "⏎" };
const shown = ch => SHOW[ch] ?? ch;
const SHIFT_NAMES = { 36: "↓", 37: "↑", 38: "⤓", 39: "⤒" };

function el(tag, cls, html) {
  const e = document.createElement(tag);
  if (cls) e.className = cls;
  if (html !== undefined) e.innerHTML = html;
  return e;
}

// what each translated card means, for labeling chips: walks the same state machine as decode
function annotate(cards) {
  const notes = [];
  let shift = 1, next = 1;
  for (const c of cards) {
    // in the hex state, cards 0-15 are hex digits of a code point
    if (shift === -1 && c < 16) { notes.push({ kind: "hex", ch: c.toString(16).toUpperCase() }); continue; }
    if (shift === -1) shift = next;
    if (shift >= 0 && c < 36) { notes.push({ kind: "", ch: shown(String.fromCodePoint(S40.CODES[shift][c])) }); shift = next; }
    else notes.push({ kind: "shift", ch: SHIFT_NAMES[c] });
    if ((c === 37 || c === 39) && shift <= 1) ++shift;
    if ((c === 36 || c === 38) && shift >= 0) --shift;
    if (c === 38 || c === 39) next = shift;
  }
  // the trailing run of 39s is the end-of-message marker
  for (let i = cards.length - 1; i >= 0 && cards[i] === 39; --i) notes[i].kind = "eom";
  return notes;
}

function chip(c, note) {
  const d = el("div", `chip ${note.kind}`);
  d.append(el("span", `g ${isRed(c) ? "red" : ""}`, S40.glyph(c)), el("span", "n", S40.two(c)), el("span", "ch", note.ch));
  return d;
}

Object.assign(Demos, {
  cards40(slide) {
    const g = slide.querySelector(".grid40");
    g.append(el("div"));
    for (const f of S40.FACES) g.append(el("div", "head", f));
    for (let s = 0; s < 4; ++s) {
      g.append(el("div", "suit", `<b>${S40.SUITS[s]}</b> ${SUIT_NAMES[s]}<br><span class="soft">${s}0s</span>`));
      for (let f = 0; f < 10; ++f) {
        const c = 10 * s + f, d = el("div", `card ${isRed(c) ? "red" : ""}`);
        d.append(S40.glyph(c), el("small", "", S40.two(c)));
        g.append(d);
      }
    }
    return { reset() {}, next() {} };
  },

  codes(slide) {
    const g = slide.querySelector(".codes");
    const [DOWN, NORMAL, UP] = S40.CODES;
    g.append(el("div"));
    for (const f of S40.FACES) g.append(el("div", "head", f));
    for (let s = 0; s < 4; ++s) {
      g.append(el("div", "suit", `${S40.SUITS[s]}<small>${s}0s</small>`));
      for (let f = 0; f < 10; ++f) {
        const c = 10 * s + f;
        if (c >= 36) {
          const d = el("div", "code shift");
          d.append(el("span", "c", SHIFT_NAMES[c]), el("span", "ud", ["shift down", "shift up", "lock down", "lock up"][c - 36]));
          g.append(d);
          continue;
        }
        const ch = n => shown(String.fromCodePoint(n));
        const d = el("div", "code");
        d.append(el("span", "c", ch(NORMAL[c])));
        const ud = el("span", "ud");
        ud.append(el("span", "u", ch(UP[c])), el("span", "d", ch(DOWN[c])));
        d.append(ud);
        g.append(d);
      }
    }
    return { reset() {}, next() {} };
  },

  translate(slide) {
    const input = slide.querySelector("input"), chips = slide.querySelector(".chips"), info = slide.querySelector(".narr");
    const render = () => {
      const cards = S40.encode(input.value), notes = annotate(cards);
      chips.innerHTML = "";
      cards.forEach((c, i) => chips.append(chip(c, notes[i])));
      const eom = notes.filter(n => n.kind === "eom").length;
      info.innerHTML = `${cards.length} cards: ${cards.length - eom} for the text, then <b>${eom} × ⤒</b> to end the message on a multiple of 10. ` +
        `Decodes to “${S40.decode(cards).replace(/\n/g, "⏎")}”.`;
    };
    input.addEventListener("input", render);
    const reset = () => { if (!input.value) input.value = "#SPIDER Cipher."; render(); };
    return { reset, next() {} };
  },

  scramble40(slide) {
    const key = S40.keyDeck("math club");
    // fixed "random" cards so the demo is the same every time
    const RAND = [17, 4, 33, 26, 9, 38, 12, 1, 30, 21, 7, 25, 14, 36, 3, 28, 19, 10, 35, 22];
    let r = 0;
    const msg = S40.encode("Hi"), pkt = S40.packet(msg, () => RAND[r++ % RAND.length]);
    const role = i => (i < 10 ? "A" : i % 2 === 0 ? "B" : "M");
    const deck = new DeckView(slide.querySelector(".deckview"), key,
      { w: 72, h: 80, gap: 10, font: 30, perRow: 20, rowGap: 16, dur: 320, stagger: 28,
        label: S40.glyph, cls: c => (isRed(c) ? "red" : "") });
    const strip = slide.querySelector(".strip"), narr = slide.querySelector(".narr");
    const cells = pkt.map((c, i) => {
      const d = el("div", `cell ${role(i)}`);
      d.append(el("span", isRed(c) ? "red" : "", S40.glyph(c)), el("span", "o", ""));
      strip.append(d);
      return d;
    });
    let j, ph, info;
    const out = [];

    const phase = async ep => {
      if (j >= pkt.length) return;
      const d = deck.order, x = pkt[j];
      cells.forEach((c, i) => c.classList.toggle("cur", i === j));
      if (ph === 0) {
        info = S40.look(d);
        deck.clear(); deck.mark(d[2], "third"); deck.mark(info.tag, "tag");
        narr.innerHTML = `Packet card ${j + 1}: ${glyphHTML(x)} = ${S40.two(x)}. Tag = third card ${glyphHTML(d[2])} ⊖ 1 = <b>${S40.glyph(info.tag)}</b>.`;
      } else if (ph === 1) {
        const y = S40.mod(x + info.noise, 40);
        deck.mark(info.noise, "pad");
        out[j] = y;
        cells[j].querySelector(".o").innerHTML = glyphHTML(y);
        narr.innerHTML = `Noise = the card after the tag, ${glyphHTML(info.noise)}. Scrambled = ${S40.two(x)} ⊕ ${S40.two(info.noise)} = <b>${S40.two(y)}</b> ${glyphHTML(y)}.`;
      } else if (ph === 2) {
        const cutCard = S40.mod(d[0] + x, 40);
        deck.clear(); deck.mark(cutCard, "cutcard");
        narr.innerHTML = `Cut card = first ${glyphHTML(d[0])} ⊕ clear ${glyphHTML(x)} = <b>${S40.glyph(cutCard)}</b>. Cut it to the front.`;
        await deck.cut(d.indexOf(cutCard), ep);
      } else {
        deck.clear();
        narr.innerHTML = "Back-front shuffle.";
        await deck.shuffle(ep);
        ++j; ph = -1;
        if (j === pkt.length) {
          cells.forEach(c => c.classList.remove("cur"));
          const back = S40.decrypt(key, out).text;
          narr.innerHTML = `Done: ${pkt.length} cards scrambled. Unscrambling with the key deck gives back “<b>${back}</b>”.`;
        }
      }
      ++ph;
    };
    const step = () => deck.run(phase);
    const card = () => deck.run(async ep => {
      do { await phase(ep); await deck.wait(450, ep); } while (ph !== 0 && j < pkt.length);
    });
    const all = () => deck.run(async ep => {
      deck.setSpeed(120, 4);
      try { while (j < pkt.length) { await phase(ep); } }
      finally { if (ep === deck.epoch) deck.setSpeed(320, 28); }
    });
    const reset = () => {
      deck.reset(); deck.setSpeed(320, 28); j = 0; ph = 0; out.length = 0;
      cells.forEach(c => { c.classList.remove("cur"); c.querySelector(".o").innerHTML = ""; });
      narr.innerHTML = `Key: the passphrase “math club”. Message “Hi” → ${msg.length} cards → a ${pkt.length}-card packet. Press <b>Step</b>.`;
    };
    buttons(slide, { step, card, all, reset });
    return { reset, next: step };
  },

  tryit(slide) {
    const $ = s => slide.querySelector(s);
    const key = $("#k"), msg = $("#m"), cipher = $("#c"), plain = $("#p"), stats = $("#stats");
    let fmt = "cards";
    const as = () => (fmt === "cards" ? S40.glyph : S40.two);
    const encrypt = () => {
      const { msg: m, pkt, cipher: c } = S40.encrypt(S40.keyDeck(key.value), msg.value);
      cipher.value = S40.format(c, as());
      stats.textContent = `${m.length} message cards → ${pkt.length}-card packet (10 random + a random card before each).`;
      decrypt();
    };
    const decrypt = () => {
      const cards = S40.parse(cipher.value);
      plain.textContent = cards.length ? S40.decrypt(S40.keyDeck(key.value), cards).text : "";
    };
    const setFmt = f => {
      fmt = f;
      slide.querySelectorAll(".toggle button").forEach(b => b.classList.toggle("on", b.dataset.act === f));
      const cards = S40.parse(cipher.value);
      if (cards.length) cipher.value = S40.format(cards, as());
    };
    buttons(slide, { encrypt, decrypt, cards: () => setFmt("cards"), decimal: () => setFmt("decimal") });
    cipher.addEventListener("input", decrypt);
    key.addEventListener("input", decrypt);
    const reset = () => { if (!cipher.value) encrypt(); };
    return { reset, next: encrypt };
  },
});
