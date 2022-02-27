"use strict";

// If you're looking at this while this comment is still here, you're looking at an intermediate verison.
// ...Unless I forgot to remove this.
// If you see something really weird, it's probably a change I made just to test a simpler version of the thing while making this.

// javascript's const implementation is... screwy, but even just marking it as such is useful
const CARDS = 40;
const LOUD = true;

// Macros are used in your code, which avoids making stack frames, which has several advantages.
// Javascript is not so kind. This is slower, but as far as I can tell shouldn't introduce any security issues.
function add(a, b) { return (a + b) % CARDS; }
// 40 is added to deal with the fact that % has odd behavior with negative numbers
function sub(a, b) { return ((a + CARDS) - b) % CARDS; }
// You only use the cut pad once ish and it's really just the top card, and is defined by several layers of indirection.
// I'm gonna just leave it out unless there's a reason to re-include it.
// The mark is similarly used only once, in finding the tag. Seems like you just completely removed that terminology from the slides.
// I agree with this decision. I was thinking about making a note to suggest it already.
function deckCipherPad(deck) {
    var tagVal = add(39, deck[2]); // same as subtracting 1
    var tagIndex = deckFindCard(deck, tagVal);
    return deck[add(tagIndex, 1)];
}

function deckFindCard(deck, card) {
    // Huh, gotta figure out how to do this O(1) in JS... alright
    // Seems like just testing equality to get an index doesn't actually work. I'll have to come up with something else.
    // TODO: test this code for browser differences
    // temp TODO: debug this lol
    // ? Wait, why didn't you do it this way before?
    var acc = 0;
    var mask = {false: ~0, true: 0};
    for(var i = 0; i < CARDS; i++) acc = acc ^ (i & mask[deck[i]==card]);
    return acc;
}

var translationTable = [
    //Q    A    2    3    4    5    6    7    8    9
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',  // + 0 club   
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',  // +10 diamond
    'u', 'v', 'w', 'x', 'y', 'z', ',', '.', '-', ' ',  // +20 heart  
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'   // +30 spade  
];

function parseDeck(deck) {
    return deck.split(",").map((x) => parseInt(x));
}

function encodeMsg(msg) {
    var encoded = [];        
    for(var i = 0; i < msg.length; i++) encoded.push(translationTable.findIndex((x) => msg[i] == x));
    return encoded;
}

function decodeMsg(msg) {
    var decoded = "";
    for(var i = 0; i < msg.length; i++) decoded += translationTable[msg[i]];
    return decoded;
}

function crypt(msg, deck, f) {
    deck = parseDeck(deck);
    msg = encodeMsg(msg);
    var cryptedMsg = [];
    for(var i = 0; i < msg.length; i++) cryptedMsg.push(f(msg[i], deck[i % 40]));
    var finalMsg = decodeMsg(cryptedMsg);
    document.getElementById("msg").value = finalMsg;
}

function scramble(msg, deck) { crypt(msg, deck, add); }
function unscramble(msg, deck) { crypt(msg, deck, sub); }

function assertEq(msg, expected, actual) {
    if(LOUD) console.log(`Testing ${msg}`);
    if(expected == actual && LOUD) console.log("Passed");
    else {
        if(!LOUD) console.log(`Testing ${msg}`);
        console.log(`!!!!Error, expected ${expected}, got ${actual}`);
    }
}

var testDeck = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39];
var testDeckShifted = [1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39, 0];
var cutTestDeck = [20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19];

for(var i = 0; i < CARDS; i++) {
    assertEq(`find ${i} in testDeck`, i, deckFindCard(testDeck, i));
    assertEq(`find ${i} in testDeckShifted`, sub(i, 1), deckFindCard(testDeck, i));    
}
assertEq(`find 23 in cutTestDeck`, 3, deckFindCard(cutTestDeck, 23));
assertEq(`find 15 in cutTestDeck`, 35, deckFindCard(cutTestDeck, 15));
