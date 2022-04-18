"use strict";

let canvas;
let width;
let height;
let ctx;
let canvasUnsupportedText = "Nope, canvas isn't supported here, you're probably using an ancient browser - you should fix that";
let mouseX = 0;
let mouseY = 0;

function loadStuff() {
    canvas = document.getElementById("demoCanvas");
    canvas.addEventListener("mousemove", (e) => {
        let canvasRect = canvas.getBoundingClientRect();
        mouseX = Math.round(e.clientX - canvasRect.left);
        mouseY = Math.round(e.clientY - canvasRect.top);    
    });    
    width = document.documentElement.clientWidth;
    height = document.documentElement.clientHeight;
    canvas.width = width;
    canvas.height = height;
    if(canvas.getContext) ctx = canvas.getContext("2d");
    else {
        alert(canvasUnsupportedText);    
        throw canvasUnsupportedText;
    }
    console.log("Loaded!")
}
window.addEventListener("load", loadStuff);

const dt = 17;
const CARD_COUNT = 40;
const THIRD_THROUGH_DECK = Math.floor(CARD_COUNT / 3);

function cardY(depthInDeck) { return (depthInDeck + 0.5) / (CARD_COUNT + 3); }

// simple interpolation from 0 to 1
function prop(whole, part) {
    return whole * part;
}

function xyProps(xy) { return [prop(width, xy[0]), prop(height, xy[1])]; }

// proper lerp
function lerp(p1, p2, prop) { return [p1[0] + ((p2[0] - p1[0]) * prop),
                                      p1[1] + ((p2[1] - p1[1]) * prop)]; }

// can't believe JS doesn't come with anything like this
// necessary to generate colors on a gradient
// (without it being horribly tedious)
function toHex(r, g, b) {
    let codify = function(digit) {
        if(digit < 0) digit = 0;
        if(digit >= 16) digit = 15;
        if(digit < 10) return digit + 48;
        if(digit < 16) return digit + 55;
    }
    return "#" + [Math.floor(r/16),r%16,Math.floor(g/16),g%16,Math.floor(b/16),b%16]
        .map(x => String.fromCharCode(codify(x)))
        .join("");
}

function vecAdd(v1, v2) { return [v1[0] + v2[0], v1[1] + v2[1]]; }
function vecSub(v1, v2) { return [v1[0] - v2[0], v1[1] - v2[1]]; }
function vecScalarMult(v, s) { return [v[0] * s, v[1] * s]; }

function toHexSmall(r, g, b) { return toHex(r * 255, g * 255, b * 255); }

function cut(cutLoc, deck) {
    let newDeck = deck.slice(0);
    let workingCard;
    for (let i = 0; i < CARD_COUNT; i++) {
        workingCard = deck[(i + cutLoc) % CARD_COUNT];
        newDeck[i] = workingCard;
        workingCard.locInDeck = i;
    }
    return newDeck;
}

function backFrontShuffle(deck) {
    let newDeck = deck.slice(0);
    let back = CARD_COUNT / 2;
    let front = (CARD_COUNT / 2) - 1;
    let workingCard;
    for (let i = 0; i < CARD_COUNT; i += 2) {
        workingCard = deck[i];
        newDeck[back] = workingCard;
        workingCard.locInDeck = back;
        workingCard = deck[i + 1];
        newDeck[front] = workingCard;
        workingCard.locInDeck = front;
        ++back;
        --front;
    }
    return newDeck;
}

class Card {
    constructor(val, startPos, color, loc) {
        this.numericVal = val;
        this.pos = startPos;
        this.color = color;
        this.target = null;
        this.vel = [0, 0];
        this.frame = -1;
        this.endFrame = 0;
        this.locInDeck = loc;

        this.img = new Image();
        let src = `${val}`;
        while(src.length < 2) src = "0" + src;
        this.img.src = src + ".svg";
    }

    setPos(newPos) { this.pos = newPos; }
    setVel(newVel) { this.vel = newVel; }
    setDest(newDest) { this.dest = newDest; }
    setColor(newHexString) { this.color = newHexString; }

    draw() {
        ctx.beginPath();
        ctx.strokeStyle = this.color;
        let start = xyProps(this.pos);
        ctx.moveTo(start[0], start[1]);
        let end = xyProps([this.pos[0] + 0.1, this.pos[1]]);
        ctx.lineTo(end[0], end[1]);
        ctx.closePath();
        ctx.lineWidth = 4;
        ctx.stroke();

        if(start[0] <= mouseX &&
           mouseX <= end[0] &&
           Math.abs(start[1] - mouseY) < 10) {
            this.drawFront(mouseX, mouseY);
        }
    }

    drawFront(x, y) {
        let scale = 0.6;
        let imgWidth = this.img.width * scale;
        let imgHeight = this.img.height * scale;
        let xShifted = x - Math.max(0, (x + imgWidth) - canvas.width);
        let yShifted = y - Math.max(0, (y + imgHeight) - canvas.height);
        ctx.drawImage(this.img, xShifted, yShifted, this.img.width * 0.6, this.img.height * 0.6);
    }

    move() {
        if(this.frame != -1) {
            if(this.frame >= this.endFrame) {
                // end the animation
                this.pos = this.target;
                this.frame = -1;
            }
            else {
                this.pos = vecAdd(this.pos, this.vel);
                this.frame += 1;
            }
        }
    }

    moveTo(dest, framesToReach) {
        this.target = dest;
        this.frame = 0;
        this.endFrame = framesToReach;
        this.vel = vecScalarMult(vecSub(this.target, this.pos), (1/framesToReach));
    }
}

// this started as more but it's barely anything anymore
class Deck {
    constructor(colorFunc) {
        this.cards = [];
        for(let i = 0; i < CARD_COUNT; i++) {
            this.cards.push(new Card(
                i,
                [0, cardY(i)],
                (colorFunc ? colorFunc(i) : toHexSmall(0, 0, 0)),
                i
            ));
        }
    }
}

class DeckAnimator {
    constructor(colorFunc, inDeck) {
        this.deck = inDeck ? inDeck : new Deck(colorFunc);
        this.step = null;
        this.animStep = -1;
        this.auto = false;
        this.frame = 0;
        this.drawables = this.deck.cards.slice(0);
        this.transform = new Array(CARD_COUNT);
        this.oldLocations = [];
        this.done = false;
        // whether cards' display positions adjust
        // to their positions in the deck
        this.adaptiveCards = false;
    }

    setStep(stepFunc) {
        this.step = stepFunc;
        this.frame = 0;
        this.animStep = -1;
    }

    swapAuto() {
        this.auto = !this.auto;
        document.getElementById("autoButton").innerHTML =
            "Autoplay "
            + (this.auto ? "ON" : "OFF");
    }

    resetPositions() {
        for(let i = 0; i < CARD_COUNT; i++) {
            let card = this.deck.cards[i];
            card.setPos([0.1, cardY(i)]);
            card.setVel([0, 0]);
            card.setDest(null);
        }        
    }

    setTransform(transformFunc) {
        let cards = this.deck.cards;
        this.oldLocations = cards.map(card => card.locInDeck);
        this.transform = cards.slice(0);
        // IMPORTANT: updates their data about locations in the deck, but not their
        // display locations!
        cards = transformFunc(cards);
    }

    advance() {
        this.animStep++;
        this.step();
    }

    update() {
        this.frame++;
        ctx.clearRect(0, 0, width, height);
        // welcome to the condition pile
        if(this.step != null &&
           this.frame % 80 == 0 &&
           this.auto
          ) this.advance();
        this.deck.cards.map(x => x.move());
        this.drawables.map(x => x.draw());
        // Javascript is weird. "this" is weird by Javascript standards.
        // "this" is just the object calling a function, so unfortunately, setTimeout
        // sets "this" to the window unless we keep it bound from here. It's a tiny fix,
        // but a pain to figure out if you didn't know that intricacy, like I didn't.
        if(!this.done) setTimeout(this.update.bind(this), dt);
        if(this.adaptiveCards) {
            for(let i = 0; i < CARD_COUNT; i++) {
                let card = this.deck.cards[i];
                card.pos = [card.pos[0], cardY(card.locInDeck)];
            }
        }
    }

    start() {
        this.update();
    }

    bfsStep() {
        if(this.animStep < CARD_COUNT) {
            this.deck
                .cards[this.oldLocations[this.animStep]]
                .moveTo([0.7, cardY(this.transform[this.animStep].locInDeck)], 60);
        }
    }

    cutStep() {
        let cutLoc = THIRD_THROUGH_DECK;
        let start = 0;
        let end = 0;
        if(this.animStep == 0) { start = 0; end = cutLoc; }
        if(this.animStep == 1) { start = cutLoc; end = CARD_COUNT; }    
        for(let i = start; i < end; i++) {
            this.deck.cards[i].moveTo([0.7, cardY(this.transform[i].locInDeck)], 60);
        }
    }
}

function bfsColors(i) { return toHexSmall(i / CARD_COUNT, 0, (CARD_COUNT - i) / CARD_COUNT); }
function cutColors(i) {
    if(i == THIRD_THROUGH_DECK) return toHexSmall(0, 0, 0);
    return (i < CARD_COUNT / 3) ? toHexSmall(1, 0, 0) : toHexSmall(0, 0, 1);
}

function drawText(text, x, y) {
    ctx.font = "30px serif";
    let realPos = xyProps([x, y]);
    ctx.fillText(text, realPos[0], realPos[1]);
}

let anim;
function bfsSetup() {
    if(anim) anim.done = true;
    anim = new DeckAnimator(bfsColors);
    anim.setStep(anim.bfsStep);
    anim.setTransform(backFrontShuffle);
    anim.start();
}
function cutSetup() {
    if(anim) anim.done = true;    
    anim = new DeckAnimator(cutColors);
    anim.setStep(anim.cutStep);
    anim.setTransform((deck) => cut(THIRD_THROUGH_DECK, deck));
    anim.start();
}
function fullSetup() {
    if(anim) anim.done = true;
    anim = new DeckAnimator(() => { return toHexSmall(0, 0, 0); });
    anim.adaptiveCards = true;
    let shuff = () => { anim.deck.cards = backFrontShuffle(anim.deck.cards); };
    let render = () => { anim.deck.cards[2].drawFront(200, 200); };
    let stuff = { change: shuff, draw: render, };
    let steps = [
        
    ];
    for(let i = 0; i < 9999; i++) steps.push(stuff);
    let firstRun = true;
    let fullStepFunc = function() {
        if(!firstRun) anim.drawables.pop();
        else firstRun = false;
        // oh geez, parentheses everywhere
        if(steps.length > 0) {
            let elem = steps.shift();
            if(elem.change) elem.change();
            if(elem.draw) anim.drawables.push(elem);
        }
    }
    anim.setStep(fullStepFunc);
    anim.start();
}
function advance() { if(anim) anim.advance(); }
function autoClickHandler() { anim.swapAuto(); }

/*let instructions = "Before anything else, shuffle the deck.\n\
However, we will keep it unshuffled to make the demonstration a bit easier.\n\
Give your friend a copy of the deck you're using.\n\
They will be able to use it to see your message!\n\
Our message will be 'Cat'.\n\
We have work to do before we even touch the deck!\n\
First we translate the message.\n\
To translate a character, find the card with the right character.\n\
Or just look it up in a table if you only have a normal deck.\n\
\n\
Now, we need to surround the message with garbage.\n\
Put five 39s \n\"
*/
