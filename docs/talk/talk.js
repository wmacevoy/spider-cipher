"use strict";

// ---- the cipher (same rules as core/src/spider_cipher.c) ----

const mod = (a, n) => ((a % n) + n) % n;

const Spider = {
  // move the top k cards to the bottom
  cut(d, k) { return d.map((_, i) => d[(i + k) % d.length]); },
  // back-front shuffle: card 2i -> n/2 + i, card 2i+1 -> n/2 - 1 - i
  shuffle(d) {
    const n = d.length, h = n / 2, o = new Array(n);
    for (let i = 0; i < h; ++i) { o[h + i] = d[2 * i]; o[h - 1 - i] = d[2 * i + 1]; }
    return o;
  },
  unshuffle(d) {
    const n = d.length, h = n / 2, o = new Array(n);
    for (let i = 0; i < h; ++i) { o[2 * i] = d[h + i]; o[2 * i + 1] = d[h - 1 - i]; }
    return o;
  },
  // pad: find (third card - 1); the next card is the pad
  pad(d) {
    const n = d.length, third = d[2], tag = mod(third - 1, n), at = d.indexOf(tag);
    return { third, tag, at, wraps: at === n - 1, pad: d[(at + 1) % n] };
  },
};

// ---- animated deck ----

const sleep = ms => new Promise(r => setTimeout(r, ms));
const CANCEL = Symbol("cancel");

class DeckView {
  constructor(el, values, opt = {}) {
    this.el = el;
    this.o = Object.assign({ w: 100, h: 134, gap: 14, font: 56, dur: 420, stagger: 110, hue: false,
      perRow: values.length, rowGap: 24, label: v => v, cls: () => "" }, opt);
    // a static deck only highlights; an animated one needs room to lift and deal
    this.lift = this.o.static ? 0 : Math.round(this.o.h * 0.45);
    this.dealY = this.lift + this.o.h + 28;
    this.rowH = (this.o.static ? this.o.h : this.dealY + this.o.h) + this.o.rowGap;
    this.initial = values.slice();
    this.epoch = 0;
    this.busy = false;
    el.classList.add("deckview");
    const cols = Math.min(values.length, this.o.perRow), rows = Math.ceil(values.length / this.o.perRow);
    el.style.width = `${cols * this.o.w + (cols - 1) * this.o.gap}px`;
    el.style.height = `${rows * this.rowH - this.o.rowGap}px`;
    this.build(values);
  }

  build(values) {
    this.el.innerHTML = "";
    this.order = values.slice();
    this.cards = new Map();
    for (const v of values) {
      const c = document.createElement("div");
      c.className = `card ${this.o.cls(v)}`.trim();
      c.textContent = this.o.label(v);
      c.dataset.v = v;
      Object.assign(c.style, {
        width: `${this.o.w}px`, height: `${this.o.h}px`,
        fontSize: `${this.o.font}px`, transition: "none",
      });
      if (this.o.hue) c.style.background = `hsl(${Math.round(v * 330 / values.length)}, 65%, 80%)`;
      this.el.appendChild(c);
      this.cards.set(v, c);
    }
    this.layout();
    this.el.offsetWidth; // commit positions before re-enabling transitions
    for (const c of this.cards.values()) {
      c.style.transition = "";
      c.style.setProperty("--dur", `${this.o.dur}ms`);
    }
  }

  x(i) { return (i % this.o.perRow) * (this.o.w + this.o.gap); }
  put(v, i, y) {
    const top = Math.floor(i / this.o.perRow) * this.rowH;
    this.cards.get(v).style.transform = `translate(${this.x(i)}px, ${top + y}px)`;
  }
  layout() { this.order.forEach((v, i) => this.put(v, i, this.lift)); }

  reset() { ++this.epoch; this.busy = false; this.build(this.initial); }
  mark(v, cls) { this.cards.get(v).classList.add(cls); }
  clear() { for (const c of this.cards.values()) c.classList.remove("third", "tag", "pad", "cutcard", "done"); }
  setSpeed(dur, stagger) {
    this.o.dur = dur; this.o.stagger = stagger;
    for (const c of this.cards.values()) c.style.setProperty("--dur", `${dur}ms`);
  }

  async wait(ms, ep) { await sleep(ms); if (ep !== this.epoch) throw CANCEL; }

  // run one animation at a time; a reset cancels whatever is in flight
  async run(fn) {
    if (this.busy) return false;
    this.busy = true;
    const ep = this.epoch;
    try { await fn(ep); }
    catch (e) { if (e !== CANCEL) throw e; return false; }
    finally { if (ep === this.epoch) this.busy = false; }
    return true;
  }

  // lift the top k cards, slide them to the bottom, drop them in
  async cut(k, ep) {
    const n = this.order.length, d = this.o.dur;
    k = mod(k, n);
    if (k === 0) return;
    const top = this.order.slice(0, k), lifted = new Set(top);
    top.forEach((v, i) => this.put(v, i, 0));
    await this.wait(d, ep);
    const next = Spider.cut(this.order, k);
    next.forEach((v, i) => this.put(v, i, lifted.has(v) ? 0 : this.lift));
    await this.wait(d * 1.4, ep);
    top.forEach(v => this.put(v, next.indexOf(v), this.lift));
    await this.wait(d, ep);
    this.order = next;
  }

  // deal from the top, one card at a time, into a new pile below; then lift the pile into place
  async deal(next, ep) {
    for (let j = 0; j < this.order.length; ++j) await this.dealOne(next, j, ep);
    await this.gather(next, ep);
  }
  // the j-th card from the top drops to its place in the new pile, which grows outward from the middle
  async dealOne(next, j, ep) {
    const v = this.order[j];
    this.put(v, next.indexOf(v), this.dealY);
    await this.wait(this.o.stagger, ep);
  }
  async gather(next, ep) {
    await this.wait(this.o.dur, ep);
    next.forEach((v, i) => this.put(v, i, this.lift));
    await this.wait(this.o.dur, ep);
    this.order = next;
  }

  shuffle(ep) { return this.deal(Spider.shuffle(this.order), ep); }
  unshuffle(ep) { return this.deal(Spider.unshuffle(this.order), ep); }
}

// ---- demos, one per slide that has data-demo ----

// the ten cards are Q, A, 2, …, 9 from one suit: the queen is 0 and the ace is 1
const face = v => (v === 0 ? "Q" : v === 1 ? "A" : `${v}`);
// a card's face with its value, for arithmetic in the narration: "Q (0)"
const faceVal = v => (v < 2 ? `${face(v)} (${v})` : `${v}`);
// position i (0 = top) as the audience counts it
const ordinal = i => ["1st", "2nd", "3rd", "4th", "5th", "6th", "7th", "8th", "9th", "10th"][i];

// the top card is not 3, so "the third card" can't be mistaken for "the card numbered 3"
const KEY = [4, 2, 8, 5, 0, 9, 6, 7, 3, 1];
const SORTED10 = [...Array(10).keys()];
const randomDeck = () => {
  const d = SORTED10.slice();
  for (let i = d.length - 1; i > 0; --i) {
    const j = Math.floor(Math.random() * (i + 1));
    [d[i], d[j]] = [d[j], d[i]];
  }
  return d;
};

function buttons(slide, handlers) {
  for (const b of slide.querySelectorAll("button[data-act]")) {
    b.addEventListener("click", () => { b.blur(); handlers[b.dataset.act](); });
  }
}

const Demos = {
  moves(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), SORTED10, { w: 96, h: 124, gap: 14, font: 52, label: face });
    const narr = slide.querySelector(".narr");
    // dealing: the shuffle being dealt one card at a time, as {next, j}; null between shuffles
    let cuts, shuffles, dealing;
    const cutOn = card => deck.run(async ep => {
      if (dealing) { narr.innerHTML = "Finish dealing the shuffle first."; return; }
      const at = deck.order.indexOf(card);
      deck.clear(); deck.mark(card, "cutcard");
      narr.innerHTML = at === 0
        ? `<b>${face(card)}</b> is already on top: the cut does nothing.`
        : `Cut on <b>${face(card)}</b>: the ${at} card${at > 1 ? "s" : ""} above it move to the bottom as one block.`;
      await deck.cut(at, ep);
      ++cuts;
    });
    // one card per press, so the room can deal along with real cards
    const dealOne = () => deck.run(async ep => {
      if (!dealing) { deck.clear(); dealing = { next: Spider.shuffle(deck.order), j: 0 }; }
      const { next, j } = dealing, v = deck.order[j];
      narr.innerHTML = j === 0 ? `<b>${face(v)}</b> starts the new pile.`
        : j % 2 === 1 ? `<b>${face(v)}</b> goes <b>on top</b> of the pile.`
        : `<b>${face(v)}</b> goes <b>underneath</b> the pile.`;
      await deck.dealOne(next, j, ep);
      if (++dealing.j === next.length) {
        await deck.gather(next, ep);
        dealing = null; ++shuffles;
        narr.innerHTML = "Pick up the pile: that is the shuffled deck.";
      }
    });
    const shuffle = () => deck.run(async ep => {
      if (dealing) { // finish the one being dealt
        for (; dealing.j < deck.order.length; ++dealing.j) await deck.dealOne(dealing.next, dealing.j, ep);
        await deck.gather(dealing.next, ep);
        dealing = null; ++shuffles;
        narr.innerHTML = "Pick up the pile: that is the shuffled deck.";
        return;
      }
      deck.clear();
      const sorted = deck.order.every((v, i) => v === i);
      narr.innerHTML = "Deal from the top: the first card starts a new pile, then alternate <b>on top</b>, <b>underneath</b>, on top, underneath…";
      await deck.shuffle(ep);
      ++shuffles;
      if (sorted) narr.innerHTML = "Odd cards rose to the top in reverse; even cards sank to the bottom in order.";
    });
    deck.el.classList.add("clickable");
    deck.el.addEventListener("click", e => {
      const c = e.target.closest(".card");
      if (c) cutOn(Number(c.dataset.v));
    });
    const reset = () => {
      deck.reset(); cuts = 0; shuffles = 0; dealing = null;
      narr.innerHTML = "Click any card to cut on it. <b>Deal one</b> steps through a shuffle; <b>Shuffle</b> does it all.";
    };
    buttons(slide, { cut3: () => cutOn(3), deal: dealOne, shuffle, reset });
    // Enter: cut on 3, deal the first shuffle card by card, then whole shuffles
    return { reset, next: () => (cuts === 0 ? cutOn(3) : shuffles === 0 ? dealOne() : shuffle()) };
  },

  pad(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), KEY, { w: 112, h: 150, gap: 16, font: 60, static: true, label: face });
    const steps = [...slide.querySelectorAll(".steps .ruled")];
    const narr = slide.querySelector(".narr");
    let info, phase;
    const show = [
      () => { deck.mark(info.third, "third"); narr.innerHTML = `Always start at the third card, whatever is on top. Here it is <b>${faceVal(info.third)}</b>.`; },
      () => { narr.innerHTML = `${info.third} − 1 ≡ <b>${info.tag}</b> (mod 10).`; },
      () => { deck.mark(info.tag, "tag"); narr.innerHTML = `Card <b>${face(info.tag)}</b> is the ${ordinal(info.at)} card.`; },
      () => {
        deck.mark(info.pad, "pad");
        narr.innerHTML = `The card after it${info.wraps ? " (wrapping to the top)" : ""} is <b>${faceVal(info.pad)}</b>. The pad is ${info.pad}.`;
      },
    ];
    const start = d => {
      deck.initial = d; deck.reset(); phase = 0;
      info = Spider.pad(d);
      steps.forEach(s => s.classList.remove("on"));
      narr.innerHTML = "Press <b>Next</b> to find the pad.";
    };
    const reset = () => start(KEY);
    const next = () => {
      if (phase >= show.length) return;
      steps[phase].classList.add("on");
      show[phase++]();
    };
    // a different deck gives a different pad
    buttons(slide, { next, random: () => start(randomDeck()), reset });
    return { reset, next };
  },

  example(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), KEY, { w: 92, h: 124, gap: 12, font: 52, label: face });
    const narr = slide.querySelector(".narr");
    const rows = [...slide.querySelectorAll("tbody tr")];
    const result = slide.querySelector(".result");
    const msg = [2, 0, 2, 6], n = 10;
    let digit, phase, info, out;

    const cell = (r, j, s) => { rows[r].children[j].textContent = s; };
    const start = key => {
      deck.initial = key; deck.reset(); digit = 0; phase = 0; out = [];
      rows.forEach((r, i) => { r.classList.remove("cur"); for (let j = 1; j < 4; ++j) cell(i, j, ""); });
      result.style.visibility = "hidden";
      narr.innerHTML = `Press <b>Next</b> (or Enter) to encrypt <b>2026</b> with the key deck ${key.map(face).join(" ")}.`;
    };
    const reset = () => start(KEY);

    const next = () => deck.run(async ep => {
      if (digit >= msg.length) return;
      const d = deck.order, p = msg[digit];
      rows.forEach((r, i) => r.classList.toggle("cur", i === digit));
      if (phase === 0) {
        info = Spider.pad(d);
        deck.clear(); deck.mark(info.third, "third");
        narr.innerHTML = `Digit <b><i>p</i> = ${p}</b>. The third card is <b>${faceVal(info.third)}</b>.`;
      } else if (phase === 1) {
        deck.mark(info.tag, "tag");
        narr.innerHTML = `${info.third} − 1 ≡ <b>${info.tag}</b>. Find card ${face(info.tag)}.`;
      } else if (phase === 2) {
        const c = mod(p + info.pad, n);
        deck.mark(info.pad, "pad");
        cell(digit, 1, info.pad); cell(digit, 2, c); out.push(c);
        narr.innerHTML = `The card after ${face(info.tag)} is <b>${faceVal(info.pad)}</b>${info.wraps ? " (wrapping to the top)" : ""}. ` +
          `Pad ${info.pad}, so <i>c</i> = ${p} + ${info.pad} ≡ <b>${c}</b>.`;
      } else if (phase === 3) {
        const top = d[0], card = mod(top + p, n), at = d.indexOf(card);
        const eq = top + p >= n ? "≡" : "=";
        deck.clear(); deck.mark(card, "cutcard");
        cell(digit, 3, `${top}+${p} ${eq} ${face(card)}`);
        narr.innerHTML = `Cut on top card + <i>p</i> = ${faceVal(top)} + ${p} ${eq} <b>${faceVal(card)}</b>` +
          (at === 0 ? ": it is already on top, so nothing moves." : ".");
        await deck.cut(at, ep);
      } else {
        deck.clear();
        narr.innerHTML = "Back-front shuffle.";
        await deck.shuffle(ep);
        ++digit; phase = -1;
        if (digit === msg.length) {
          rows.forEach(r => r.classList.remove("cur"));
          result.querySelector(".accent").textContent = out.join("");
          result.style.visibility = "visible";
          narr.innerHTML = `Done: <b>2026 → ${out.join("")}</b>. The receiver subtracts each pad and makes the same moves.`;
        } else {
          narr.innerHTML = `Ready for the next digit, <b>${msg[digit]}</b>.`;
        }
      }
      ++phase;
    });
    // a different key encrypts the same message differently
    buttons(slide, { next, random: () => start(randomDeck()), reset });
    return { reset, next };
  },

  reverse(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), SORTED10, { w: 100, h: 134, gap: 14, font: 56, label: face });
    const labels = [...slide.querySelectorAll(".reverse-steps p")];
    const narr = slide.querySelector(".narr");
    const acts = [
      ep => deck.shuffle(ep),
      ep => deck.cut(5, ep),
      ep => deck.unshuffle(ep),
    ];
    let k = 0;
    const reset = () => {
      deck.reset(); k = 0;
      labels.forEach(l => l.classList.remove("on", "done"));
      narr.innerHTML = "Press <b>Next</b>: shuffle, cut half, unshuffle.";
    };
    const next = () => {
      if (k >= acts.length) return reset();
      return deck.run(async ep => {
        labels.forEach((l, i) => { l.classList.toggle("on", i === k); l.classList.toggle("done", i < k); });
        narr.innerHTML = ["Back-front shuffle (β)…", "Cut 5: the top half moves to the bottom (τ<sup>5</sup>)…", "Unshuffle (β<sup>−1</sup>): deal it back…"][k];
        await acts[k](ep);
        labels[k].classList.replace("on", "done");
        if (++k === acts.length) {
          for (const v of deck.order) deck.mark(v, "done");
          narr.innerHTML = "<b>9 8 7 6 5 4 3 2 A Q</b>: the deck is reversed.";
        }
      });
    };
    buttons(slide, { next, reset });
    return { reset, next };
  },

  girth(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), [...Array(40).keys()],
      { w: 34, h: 54, gap: 7.6, font: 17, dur: 280, stagger: 22, hue: true });
    const counter = slide.querySelector(".counter");
    let k = 0;
    const show = () => {
      counter.textContent = k === 0 ? "0 steps · sorted"
        : `${k} step${k > 1 ? "s" : ""}${k % 9 === 0 ? " · sorted again" : ""}`;
    };
    const one = async ep => { await deck.cut(2, ep); await deck.shuffle(ep); ++k; show(); };
    const step = () => deck.run(one);
    const all = () => deck.run(async ep => {
      deck.setSpeed(180, 10);
      try { do { await one(ep); } while (k % 9 !== 0); }
      finally { if (ep === deck.epoch) deck.setSpeed(280, 22); }
    });
    const reset = () => { deck.reset(); deck.setSpeed(280, 22); k = 0; show(); };
    buttons(slide, { step, all, reset });
    return { reset, next: step };
  },
};

// ---- slides, scaling, keys ----

const stage = document.getElementById("stage");
const slides = [...stage.querySelectorAll(".slide")];
const notes = document.getElementById("notes");
const demos = new Map();
let cur = 0;

function fit() {
  const s = Math.min(innerWidth / 1920, innerHeight / 1080);
  stage.style.transform = `translate(${(innerWidth - 1920 * s) / 2}px, ${(innerHeight - 1080 * s) / 2}px) scale(${s})`;
}

function show(i) {
  cur = Math.max(0, Math.min(slides.length - 1, i));
  slides.forEach((s, j) => s.classList.toggle("active", j === cur));
  const demo = demos.get(slides[cur]);
  if (demo) demo.reset();
  const aside = slides[cur].querySelector("aside.notes");
  notes.innerHTML = `<b>${cur + 1} / ${slides.length}</b> &nbsp; ${aside ? aside.innerHTML : ""}`;
  if (location.hash !== `#${cur + 1}`) history.replaceState(null, "", `#${cur + 1}`);
}

slides.forEach((s, j) => {
  const no = document.createElement("div");
  no.className = "pageno";
  no.textContent = `${j + 1}`;
  s.appendChild(no);
});

addEventListener("keydown", e => {
  if (e.metaKey || e.ctrlKey || e.altKey) return;
  if (e.target.closest?.("input, textarea, select")) return; // typing in a demo
  const demo = demos.get(slides[cur]);
  switch (e.key) {
    case "ArrowRight": case "PageDown": case " ": show(cur + 1); break;
    case "ArrowLeft": case "PageUp": show(cur - 1); break;
    case "Home": show(0); break;
    case "End": show(slides.length - 1); break;
    case "Enter": case ".": if (demo) demo.next(); break;
    case "r": if (demo) demo.reset(); break;
    case "n": notes.classList.toggle("show"); break;
    case "f":
      if (document.fullscreenElement) document.exitFullscreen();
      else document.documentElement.requestFullscreen?.();
      break;
    default: return;
  }
  e.preventDefault();
});

document.getElementById("prev").addEventListener("click", () => show(cur - 1));
document.getElementById("next").addEventListener("click", () => show(cur + 1));
addEventListener("resize", fit);
addEventListener("hashchange", () => show(parseInt(location.hash.slice(1), 10) - 1 || 0));
fit();
// start after every script has loaded, so another deck can add to Demos
document.addEventListener("DOMContentLoaded", () => {
  slides.forEach(s => { if (s.dataset.demo) demos.set(s, Demos[s.dataset.demo](s)); });
  show(parseInt(location.hash.slice(1), 10) - 1 || 0);
});
