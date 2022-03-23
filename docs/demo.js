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

var drawables = [];
var frame = 0;
var dt = 17;
var cardToSend = 0;

function play() {
    ctx.clearRect(0, 0, width, height);
    if(frame % 80 == 0 && cardToSend < 40) {
        var offset = (cardToSend + 1) / 60;
        if(cardToSend % 2 != 0) offset = -offset;
        drawables[cardToSend].moveTo([0.9, (offset/2) + 0.5], 60);
        cardToSend++;
    }
    drawables.map(x => x.move());        
    drawables.map(x => x.draw());
    frame++;
    setTimeout(play, dt);
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

var deck = [];

var CARDS = 40;
for(var i = 0; i < CARDS; i++) {
    deck.push(new Card([0.1, (i + 0.1) / (CARDS + 3)],
                       toHexSmall(i/CARDS, (CARDS-i)/CARDS, 0.5), 
                       [[0.7, (i + 0.1) / (CARDS+3)]],
                       ));
}

function moveCards() {
    drawables = deck;
    drawables.push(new Card([0.9, 0.5], toHexSmall(0, 0, 0)));
    play();
}
