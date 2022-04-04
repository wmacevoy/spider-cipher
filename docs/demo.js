// Note to all who find this code. "Fantastic quality" wasn't exactly prioritized,
// because I'm working on a semi-fixed time limit for a school project and this part's code
// isn't particularly critical. I don't hate it, it's just not the best I could do.

"use strict";

var canvas;
var width;
var height;
var ctx;
var canvasUnsupportedText = "Nope, canvas isn't supported here, you're probably using an ancient browser - you should fix that";

function loadStuff() {
    canvas = document.getElementById("demoCanvas");
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
const CARDS = 20;
// functionally a const
var UNSHUFFLED = [];
// an item in an array that stores its own position? seems weird but will make sense
for(var i = 0; i < CARDS; i++) UNSHUFFLED.push({loc: i});
var shuffled = UNSHUFFLED.slice(0); // deep copy

// if for whatever reason this ever gets more complicated, these need to
// get moved into an object for managing the animation state - this is only fine
// now because the thing I'm making is small; furthermore, I'm prioritizing getting
// this part done over making it "clean"
var drawables = [];
var deck = [];
drawables = deck;

var frame = 0;
var animStep = -1;
var stepFunc = null;

var auto = false;
var started = false;

function deckCut(cutLoc) {
    var newDeck = shuffled.slice(0); // deep copy
    var workingCard;
    for (var i = 0; i < CARDS; i++) {
        workingCard = shuffled[(i + cutLoc) % CARDS];
        workingCard.loc = i;
        newDeck[i] = workingCard;
    }
    shuffled = newDeck;
}

function deckCutConstantLoc() { deckCut(Math.floor(CARDS / 3)); }

function deckBackFrontShuffle() {
    var newDeck = shuffled.slice(0); // just a deep copy    
    var back = CARDS/2;
    var front = CARDS/2-1;
    var workingCard;
    for (var i=0; i<CARDS; i += 2) {
        workingCard = shuffled[i];
        workingCard.loc = back;
        newDeck[back] = workingCard;
        workingCard = shuffled[i + 1];
        workingCard.loc = front;
        newDeck[front] = workingCard;
        ++back;
        --front;
    }
    shuffled = newDeck;
}

function cardY(depthInDeck) { return (depthInDeck + 0.5) / (CARDS + 3); }

var bfsStep = function() {
    animStep++;
    // here's the payoff to the earlier setup; shuffled and unshuffled refer to
    // the same 40 cards, but in different orders. as a result, you can use
    // one to look into the other; the location needed to be a property
    // simply because otherwise it would be passing numbers around,
    // not pointers, which isn't very helpful
    drawables[animStep].moveTo([0.7, cardY(UNSHUFFLED[animStep].loc)], 60);
}

var cutStep = function() {
    animStep++;
    var cutLoc = Math.floor(CARDS / 3);
    var start = 0;
    var end = 0;
    if(animStep == 0) { start = 0; end = cutLoc; }
    if(animStep == 1) { start = cutLoc; end = CARDS; }    
    for(var i = start; i < end; i++) drawables[i].moveTo([0.7, cardY(UNSHUFFLED[i].loc)], 60);
}

function swapAuto() {
    auto = !auto;
    document.getElementById("autoButton").innerHTML = "Autoplay " + (auto?"ON":"OFF");
}

function bfsSetup() {
    setup(bfsStep, bfsColors)
    shuffled = UNSHUFFLED.slice(0);
    deckBackFrontShuffle();
}

function cutSetup() {
    setup(cutStep, cutColors)
    shuffled = UNSHUFFLED.slice(0);
    deckCutConstantLoc();
}

function bfsColors(i) { return toHexSmall(i / CARDS, 0, (CARDS - i) / CARDS); }
function cutColors(i) {
    if(i == Math.floor(CARDS / 3)) return toHexSmall(0, 0, 0);
    return (i < CARDS / 3) ? toHexSmall(1, 0, 0) : toHexSmall(0, 0, 1);
}

function setup(newAnim, colorFunc, shuffleFunc) {
    for(var i = 0; i < CARDS; i++) deck[i].setColor(colorFunc(i));    
    resetCardPos();
    stepFunc = newAnim;
    animStep = -1;
    if(!started) {
        started = true;
        updateLoop();
    }
}

function updateLoop() {
    frame++;    
    ctx.clearRect(0, 0, width, height);
    // welcome to the condition pile
    if(stepFunc != null &&
       frame % 80 == 0 &&
       animStep < CARDS &&
       auto
      ) stepFunc();
    drawables.map(x => x.move());
    drawables.map(x => x.draw());
    setTimeout(updateLoop, dt);
}

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
    var codify = function(digit) {
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

class Card {
    constructor(startPos, color) {
        this.pos = startPos;
        this.color = color;
        this.target = null;
        this.vel = [0, 0];
        this.frame = -1;
        this.endFrame = 0;
    }

    setPos(newPos) { this.pos = newPos; }
    setVel(newVel) { this.vel = newVel; }
    setDest(newDest) { this.dest = newDest; }
    setColor(newHexString) { this.color = newHexString; }

    draw() {
        ctx.beginPath();
        ctx.strokeStyle = this.color;
        var prop = xyProps(this.pos);
        ctx.moveTo(prop[0], prop[1]);
        prop = xyProps([this.pos[0] + 0.1, this.pos[1]]);
        ctx.lineTo(prop[0], prop[1]);
        ctx.closePath();
        ctx.stroke();
    }

    move() {
        if(this.frame != -1) {
            if(this.frame == this.endFrame) {
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

for(var i = 0; i < CARDS; i++) {
    deck.push(new Card([0, 0], toHexSmall(0, 0, 0)));
}

function resetCardPos() {
    for(var i = 0; i < CARDS; i++) {
        deck[i].setPos([0.1, cardY(i)]);
        deck[i].setVel([0, 0]);
        deck[i].setDest(null);
    }
}
