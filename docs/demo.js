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
    
// Alright, this looks scary, but it's not that bad. It's just a recursive setTimeout.
// If you shouldn't be done, play the next frame, then do it all again after a delay,
// with an incremented step value. If you should be done, be done. Easy. This method
// pauses other execution, but that's really not much of an issue here.
function playAnimation(animation, dt=17, step=0) {
    ctx.clearRect(0, 0, width, height);
    animation.progress = ((dt / 1000) * step) / animation.duration;
    if(animation.progress < 1) {
        animation.nextFrame(step, dt);
        setTimeout(() => { playAnimation(animation, dt, step + 1) }, dt);
    } else return;
}

// simple interpolation from 0 to 1
function prop(whole, part) {
    return whole * part;
}

function xyProps(x, y) { return [prop(width, x), prop(height, y)]; }

var backFrontShuffleAnimation = {
    duration: 1,
    nextFrame: function(step) {
        var xy;
        ctx.beginPath();
        xy = xyProps(this.progress - 0.1, 0.5);
        ctx.moveTo(xy[0], xy[1]);
        xy = xyProps(this.progress, 0.5);
        ctx.lineTo(xy[0], xy[1]);
        ctx.closePath();
        ctx.stroke();
    },
}
