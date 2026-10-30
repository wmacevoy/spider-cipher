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
    for (const v of this.order) {
      this.put(v, next.indexOf(v), this.dealY);
      await this.wait(this.o.stagger, ep);
    }
    await this.wait(this.o.dur, ep);
    next.forEach((v, i) => this.put(v, i, this.lift));
    await this.wait(this.o.dur, ep);
    this.order = next;
  }

  shuffle(ep) { return this.deal(Spider.shuffle(this.order), ep); }
  unshuffle(ep) { return this.deal(Spider.unshuffle(this.order), ep); }
}

// ---- demos, one per slide that has data-demo ----

const KEY = [3, 7, 0, 9, 4, 1, 8, 5, 2, 6];
const SORTED10 = [...Array(10).keys()];

function buttons(slide, handlers) {
  for (const b of slide.querySelectorAll("button[data-act]")) {
    b.addEventListener("click", () => { b.blur(); handlers[b.dataset.act](); });
  }
}

const Demos = {
  moves(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), SORTED10, { w: 96, h: 124, gap: 14, font: 52 });
    const narr = slide.querySelector(".narr");
    let cuts = 0;
    const cutOn = card => deck.run(async ep => {
      const at = deck.order.indexOf(card);
      deck.clear(); deck.mark(card, "cutcard");
      narr.innerHTML = at === 0
        ? `<b>${card}</b> is already on top: the cut does nothing.`
        : `Cut on <b>${card}</b>: the ${at} card${at > 1 ? "s" : ""} above it move to the bottom as one block.`;
      await deck.cut(at, ep);
      ++cuts;
    });
    const shuffle = () => deck.run(async ep => {
      deck.clear();
      const sorted = deck.order.every((v, i) => v === i);
      narr.innerHTML = "Deal from the top: the first card starts a new pile, then alternate <b>on top</b>, <b>underneath</b>, on top, underneath…";
      await deck.shuffle(ep);
      if (sorted) narr.innerHTML = "Odd cards rose to the top in reverse; even cards sank to the bottom in order.";
    });
    deck.el.classList.add("clickable");
    deck.el.addEventListener("click", e => {
      const c = e.target.closest(".card");
      if (c) cutOn(Number(c.dataset.v));
    });
    const reset = () => { deck.reset(); cuts = 0; narr.innerHTML = "Click any card to cut on it, or press <b>Shuffle</b>."; };
    buttons(slide, { cut3: () => cutOn(3), shuffle, reset });
    return { reset, next: () => (cuts === 0 ? cutOn(3) : shuffle()) };
  },

  pad(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), KEY, { w: 112, h: 150, gap: 16, font: 60, static: true });
    const steps = [...slide.querySelectorAll(".steps .ruled")];
    const narr = slide.querySelector(".narr");
    const info = Spider.pad(KEY);
    let phase = 0;
    const show = [
      () => { deck.mark(info.third, "third"); narr.innerHTML = `The third card is <b>${info.third}</b>.`; },
      () => { narr.innerHTML = `${info.third} − 1 ≡ <b>${info.tag}</b> (mod 10).`; },
      () => { deck.mark(info.tag, "tag"); narr.innerHTML = `Card <b>${info.tag}</b> is in position ${info.at}.`; },
      () => { deck.mark(info.pad, "pad"); narr.innerHTML = `The card after it is <b>${info.pad}</b>. The pad is ${info.pad}.`; },
    ];
    const reset = () => {
      deck.reset(); phase = 0;
      steps.forEach(s => s.classList.remove("on"));
      narr.innerHTML = "Press <b>Next</b> to find the pad.";
    };
    const next = () => {
      if (phase >= show.length) return;
      steps[phase].classList.add("on");
      show[phase++]();
    };
    buttons(slide, { next, reset });
    return { reset, next };
  },

  example(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), KEY, { w: 92, h: 124, gap: 12, font: 52 });
    const narr = slide.querySelector(".narr");
    const rows = [...slide.querySelectorAll("tbody tr")];
    const result = slide.querySelector(".result");
    const msg = [2, 0, 2, 6], n = 10;
    let digit, phase, info, out;

    const cell = (r, j, s) => { rows[r].children[j].textContent = s; };
    const reset = () => {
      deck.reset(); digit = 0; phase = 0; out = [];
      rows.forEach((r, i) => { r.classList.remove("cur"); for (let j = 1; j < 4; ++j) cell(i, j, ""); });
      result.style.visibility = "hidden";
      narr.innerHTML = "Press <b>Next</b> (or Enter) to encrypt <b>2026</b> with the key deck.";
    };

    const next = () => deck.run(async ep => {
      if (digit >= msg.length) return;
      const d = deck.order, x = msg[digit];
      rows.forEach((r, i) => r.classList.toggle("cur", i === digit));
      if (phase === 0) {
        info = Spider.pad(d);
        deck.clear(); deck.mark(info.third, "third");
        narr.innerHTML = `Digit <b>x = ${x}</b>. The third card is <b>${info.third}</b>.`;
      } else if (phase === 1) {
        deck.mark(info.tag, "tag");
        narr.innerHTML = `${info.third} − 1 ≡ <b>${info.tag}</b>. Find card ${info.tag}.`;
      } else if (phase === 2) {
        const y = mod(x + info.pad, n);
        deck.mark(info.pad, "pad");
        cell(digit, 1, info.pad); cell(digit, 2, y); out.push(y);
        narr.innerHTML = `The card after ${info.tag} is <b>${info.pad}</b>${info.wraps ? " (wrapping to the top)" : ""}. ` +
          `Pad ${info.pad}, so y = ${x} + ${info.pad} ≡ <b>${y}</b>.`;
      } else if (phase === 3) {
        const top = d[0], c = mod(top + x, n), at = d.indexOf(c);
        const eq = top + x >= n ? "≡" : "=";
        deck.clear(); deck.mark(c, "cutcard");
        cell(digit, 3, `${top}+${x} ${eq} ${c}`);
        narr.innerHTML = at === 0
          ? `Cut on top card + x = ${top} + ${x} ${eq} <b>${c}</b>: it is already on top, so nothing moves.`
          : `Cut on top card + x = ${top} + ${x} ${eq} <b>${c}</b>.`;
        await deck.cut(at, ep);
      } else {
        deck.clear();
        narr.innerHTML = "Back-front shuffle.";
        await deck.shuffle(ep);
        ++digit; phase = -1;
        if (digit === msg.length) {
          rows.forEach(r => r.classList.remove("cur"));
          result.style.visibility = "visible";
          narr.innerHTML = `Done: <b>2026 → ${out.join("")}</b>. The receiver subtracts each pad and makes the same moves.`;
        } else {
          narr.innerHTML = `Ready for the next digit, <b>${msg[digit]}</b>.`;
        }
      }
      ++phase;
    });
    buttons(slide, { next, reset });
    return { reset, next };
  },

  reverse(slide) {
    const deck = new DeckView(slide.querySelector(".deckview"), SORTED10, { w: 100, h: 134, gap: 14, font: 56 });
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
          narr.innerHTML = "<b>9876543210</b>: the deck is reversed.";
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
