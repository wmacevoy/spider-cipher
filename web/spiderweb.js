"use strict";
// TODO TOMORROW deal with the fact that it needs to avoid shift ranges in certain situations

// If you're looking at this while this comment is still here, you're looking at an intermediate verison.
// ...Unless I forgot to remove this.
// If you see something really weird, it's probably a change I made just to test a simpler version of the thing while making this.

// javascript's const implementation is... screwy, but even just marking it as such is useful
const CARDS = 40;
const LOUD = false;

// Macros are used in your code, which avoids making stack frames, which has several advantages.
// Javascript is not so kind. This is slower, but as far as I can tell shouldn't introduce any security issues.
function add(a, b) { return (a + b) % CARDS; }
// 40 is added to deal with the fact that % has odd behavior with negative numbers
function sub(a, b) { return ((a + CARDS) - b) % CARDS; }
// You only use the cut pad once ish and it's really just the top card, and is defined by several layers of indirection.
// I'm gonna just leave it out unless there's a reason to re-include it.
// The mark is similarly used only once, in finding the tag. Seems like you just completely removed that terminology from the slides.
// I agree with this decision. I was thinking about making a note to suggest it already.
function noise(deck) {
    var tagVal = add(39, deck[2]); // same as subtracting 1
    var tagIndex = deckFindCard(deck, tagVal);
    return deck[add(tagIndex, 1)];
}

function deckFindCard(deck, card) {
    // TODO: test on more browsers
    // Works on: ([Y]es, [N]o, [P]ending testing)
    // Y Firefox
    // Y Chromium (so probably Chrome)
    // P Edge
    // P Opera
    // P Various mobile browsers
    // ? Wait, why didn't you do it this way before?
    //
    // Explanation:
    // The first part of this is quite similar to the C version.
    // ~0 is all 1's in binary, and in this case I don't need to worry about the truthiness of that. So a mask is selected based
    // on whether the current index is correct or not. It's anded with the index, which gives either the i & 0 (0) or i & 1 (i).
    // Since you've got 39 zeroes with (hopefully) an i in there, if you just or the pile together, the result is i or 0 or 0
    // or 0 ......... or 0. Which is still just i, and remains so in any order for any i. The difference here is that you should
    // be able to accumulate as you go without problems. As far as I can tell, this is constant time, constant space. There is an
    // implicit "if" here in the loop, but it runs the same every time. The only thing that changes between runs is which deck[i]
    // is the same as the card and therefore which entry is selected in the mask.
    //
    // Note that this is a pretty funny solution - the algorithm is O(N), but N is constant and we always take the worse case.
    // So it always takes the same amount of time to run for our purposes. You can definitely do better. I might try to.
    var acc = 0;
    // Seems like just testing equality to get an index doesn't actually work in js, I had to do something slightly different    
    var mask = {false: 0, true: ~0};
    for(var i = 0; i < CARDS; i++) acc = acc | (i & mask[deck[i]==card]);
    return acc;
}

// basically just pasted the code and changed some names
// with the exception that the output works differently
function deckCut(inputDeck, cutLoc) {
    var outputDeck = inputDeck.slice(0); // just a deep copy
    for (var i=0; i<CARDS; ++i) {
        outputDeck[i]=inputDeck[(i+cutLoc) % CARDS];
    }
    return outputDeck;
}

// same here
function deckBackFrontShuffle(inputDeck) {
    var outputDeck = inputDeck.slice(0); // just a deep copy    
    var back = CARDS/2;
    var front = CARDS/2-1;
    for (var i=0; i<CARDS; i += 2) {
        outputDeck[back]=inputDeck[i];
        outputDeck[front]=inputDeck[i+1];
        ++back;
        --front;
    }
    return outputDeck;    
}

// ditto
function deckBackFrontUnshuffle(inputDeck) {
    var outputDeck = inputDeck.slice(0); // just a deep copy
    var back = CARDS;
    var front = -1;
    for (var i=CARDS-1; i >= 0; --i) {
        if (i % 2 == 0) {
            --back;
            outputDeck[i]=inputDeck[back];
        } else {
            ++front;
            outputDeck[i]=inputDeck[front];
        }
    }
    return outputDeck;    
}

// slight modification to work with differently working cutting and shuffling
// ? Wait, why does the actual code use pseudo shuffle?
function deckPseudoShuffle(deck, cutLoc) {
    var temp;
    temp = deckCut(deck,cutLoc);
    temp = deckBackFrontShuffle(temp,deck);
    for (var i=0; i<CARDS; ++i) {
        temp[i]=0;
    }
}

// Some translation was needed; emoji are encoded slightly different here.
// ? You missed UP_CODE 33, '`'. This is an accident right?
var DOWN_CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  // + 0 club 
    'A', 'B', 'C', 'D', 'E', 'F', '@', '=','\\', '~',  // +10 diamond
    '#', '$', '%', '^', '&', '|', '-', '+', '/', '*',  // +20 heart
    '\n', ';', '?','\'',                               // +30 spade  
];

var CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',  // + 0 club   
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',  // +10 diamond
    'u', 'v', 'w', 'x', 'y', 'z', '<', '>', '(', ')',  // +20 heart  
    ' ', ',', '.','\"',                                // +30 spade  
];

var UP_CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',  // + 0 club   
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',  // +10 diamond
    'U', 'V', 'W', 'X', 'Y', 'Z', '{', '}', '[', ']',  // +20 heart  
    '_', ':', '!', '`',                                // +30 spade
];

var ALL_CODES = [DOWN_CODES, CODES, UP_CODES];
var EMOJICODES = [0xDE22, 0xDE04, 0xDC4E, 0xDC4D, 0xDC94, 0xDDA4];
// I could do this using some magical modular arithmetic but a parallel array
// is way easier to get right, easier to read, and not particularly worse here
var EMOJIINDICES = [[0, 34], [0, 35], [1, 34], [1, 35], [2, 34], [2, 35]];

// javascript's array find is terrible. it returns undefined when the value is not found
// undefined is ridiculously full of nonsense edge cases to work directly with, so
// I'm just avoiding it this way
function saneArrFind(arr, valToFind) {
    for(var i = 0; i < arr.length; i++) if(arr[i] === valToFind) return i;
    return -1;
}

// ? why is ord called ord in your code?
// ? and why does it return -1 when you use it with a shift state other than a normal one and it finds the code in the normal ones?
// ? Why does it perform this search at all in this case? Why not just return the code?
// I'm rewriting this to be significantly different. I just want to get the thing working for now.
// TODO: fix halt and catch fire
// ? TODO: implement this with a hash map?
// ? Your code is much harder to read than this, though more performant. Priorities for this version?
// 
// also: gotta deal with the annoyingness of emoji somehow
// for some reason the codes you use to get them to print aren't the same
// testing reveals that converting from char codes doesn't print right, and that
// the printable ones can't be looked with the same code
// maybe I'm missing something, but this should work
function translateChar(str, pos, shift) {
    var ch = str[pos];
    for(var searchShift = 0; searchShift < 3; searchShift++) {
        for(var index = 0; index < 36; index++) {
            if(ALL_CODES[searchShift][index] == ch) {
                // this approach is probably kinda slow but it's very readable
                // compared to other options I could think of
                var ret = new Object();
                ret.shift = searchShift;
                ret.ch = index;
                return ret;
            }
        }
    }
    var emojiIndex = -1;
    if(ch.charCodeAt(0) == 0xD83D) emojiIndex = saneArrFind(EMOJICODES, str[pos + 1].charCodeAt(0));
    if(emojiIndex != -1) {
        var ret = new Object();
        ret.shift = EMOJIINDICES[emojiIndex][0];
        ret.ch = EMOJIINDICES[emojiIndex][1];
        return ret;
    }
    throw `couldn't find character ${ch} (charcode1 ${ch.charCodeAt(0)}, 2 ${str[index + 1].charCodeAt(0)}); halt and catch fire`;
}

function detranslateChar(ch, shift) {
    if(ch == 34 || ch == 35) return String.fromCharCode(0xD83D, EMOJICODES[shift * 2 + (ch - 34)]);
    return ALL_CODES[shift][ch];
}

function readDeckString(deck) {
    return deck.split(",").map((x) => parseInt(x));
}

function translateString(str) {
    var shift = 0;
    var translated = [];
    for(var i = 0; i < str.length; i++) {
        var charInfo = translateChar(str, i, shift);
        if(shift < charInfo.shift) for(; shift < charInfo.shift; shift++) translated.push(39); // shift up
        if(shift > charInfo.shift) for(; shift > charInfo.shift; shift--) translated.push(38); // shift down
        translated.push(charInfo.ch);
        if(charInfo.ch == 34 || charInfo.ch == 35) i++;
    }
    return translated;
}

function detranslate(arr) {
    var shift = 0;
    var detranslated = "";
    for(var i = 0; i < arr.length; i++) {
        if(arr[i] == 38) shift--;
        else if(arr[i] == 39) shift++;
        else detranslated += detranslateChar(arr[i], shift);
    }
    return detranslated;
}

// these two could use refactoring to remove redundancy, but advanceDeck was pretty 
// hard to read so I'm still thinking about how best to do it
// TODO: that
function scramble(rawMsg, deckString) {
    var deck = readDeckString(deckString);
    var msg = translateString(rawMsg);
    var scrambled = [];
    for(var i = 0; i < msg.length; i++) {
        scrambled.push(add(msg[i], noise(deck)));
        deck = deckCut(deck, deckFindCard(deck, msg[i]));
        // TODO: add the front-back shuffle in
        // deck = deckBackFrontShuffle(deck);
    }
    return detranslate(scrambled);
}

function unscramble(rawMsg, deckString) {
    var deck = readDeckString(deckString);
    var msg = translateString(rawMsg);
    var unscrambled = [];
    for(var i = 0; i < msg.length; i++) {
        unscrambled.push(sub(msg[i], noise(deck)));
        deck = deckCut(deck, deckFindCard(deck, unscrambled[i]));
        // TODO: add the front-back shuffle in        
        // deck = deckBackFrontShuffle(deck);
    }
    return detranslate(unscrambled);
}


// [0-35]     39,     [0-33]     34,     [35,38,]                      [0-33, inc.A-F screwiness]        34,           [manually added section]
// [0-35]  34,39,34,  [0-33]  38,34,39,  [35,38,]  34,39,39,34,38,38,  [0-33, inc.A-F screwiness]  38,38,34,39,39,35,  [manually added section]

// [0-33]  34,  [35,39,]  [0-33] 34,35,38,  [0-33]  34,35,38,38
// [0-33]  35,  [35,39,]  [0-33] 38,35,35,  [0-33]  38,38,35,35  
