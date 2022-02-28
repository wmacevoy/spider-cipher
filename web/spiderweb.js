"use strict";

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
    var outputDeck = inputDeck.splice(0); // just a deep copy
    for (var i=0; i<CARDS; ++i) {
        outputDeck[i]=inputDeck[(i+cutLoc) % CARDS];
    }
    return outputDeck;
}

// same here
function deckBackFrontShuffle(inputDeck) {
    var outputDeck = inputDeck.splice(0); // just a deep copy    
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
    var outputDeck = inputDeck.splice(0); // just a deep copy
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
function deckPseudoShuffle(deck, cutLoc) {
    var temp;
    temp = deckCut(deck,cutLoc);
    temp = deckBackFrontShuffle(temp,deck);
    for (var i=0; i<CARDS; ++i) {
        temp[i]=0;
    }
}

var DOWN_CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  // + 0 club 
    'A', 'B', 'C', 'D', 'E', 'F', '@', '=','\\', '~',  // +10 diamond
    '#', '$', '%', '^', '&', '|', '-', '+', '/', '*',  // +20 heart
    '\n', ';', '?','\'', '\u{1F622}', '\u{1F604}'      // +30 spade  
];

var CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',  // + 0 club   
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',  // +10 diamond
    'u', 'v', 'w', 'x', 'y', 'z', '<', '>', '(', ')',  // +20 heart  
    ' ', ',', '.','\"', '\u{1F44E}', '\u{1F44D}'       // +30 spade  
];

var UP_CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',  // + 0 club   
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',  // +10 diamond
    'U', 'V', 'W', 'X', 'Y', 'Z', '{', '}', '[', ']',  // +20 heart  
    '_', ':', '!', '`', '\u{1F494}', '\u{2764}'	       // +30 spade
];

var CODES = [DOWN_CODES, CODES, UP_CODES];

function showEmoji() {
    var s = "";
    CODES.map((x) => s += `${x[34]}${x[35]}`);
    console.log(s);
}

if(LOUD) {
    console.log("Emoji test:");
    showEmoji();
}

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

// messy binary decoder I threw together to see values for debugging
function bin(n) {
    if(n<=0) return "0 or less";
    else {
        var ret = "";
        var div = 1;
        while(div <= n) div *= 2;
        while(div != 1) {
            div /= 2;
            if(n >= div) {
                n -= div;
                ret += "1";
            } else ret += "0";
        }
        while(ret.length < 32) ret = "0" + ret;
        return ret;
    }
}

// correct is the function used to compare expected to result to determine if it's acceptable
class TestCase {
    constructor(msg, expected, func, args, check) {
        this.msg = msg;
        this.expected = expected;
        this.func = func;        
        this.args = args;
        this.check = check;
    }

    runTest(loud) {
        if(loud) console.log(`Testing ${this.msg}...`);
        // To pass a list as arguments, there's this cool thing called spread syntax. Unfortunately, IE doesn't support it.
        // So... apply() it is.
        var actual = this.func.apply(null, this.args);
        var ret = this.check(this.expected, actual);
        if(loud && ret) console.log("Passed!");
        if(!ret) console.log(`!!!Failed test; expected: ${this.expected}, actual: ${actual}`);
        return ret;
    }
}

class EqTestCase extends TestCase {
    constructor(msg, expected, func, args) {
        super(msg, expected, func, args, (a, b) => { return a == b; } );
    }
}

class BoolTestCase extends TestCase {
    // really just a degenerate case which ultimately checks if the function is true
    constructor(msg, func, args) {
        super(msg, true, func, args, (a, b) => { return a == b; } );
    }
}

function decksAreEqual(a, b) {
    for(var i = 0; i < CARDS; i++) if(a[i] != b[i]) return false;
    return true;
}

function deckToIndices(deck) {
    var indices = [];
    for(var i = 0; i < CARDS; i++) indices.push(deckFindCard(deck, i));
    return indices;    
}

var testDeck = [];
for(var i = 0; i < CARDS; i++) testDeck.push(i);
var testDeckShifted = [];
for(var i = 0; i < CARDS; i++) testDeckShifted.push((i + 1) % CARDS);
var cutTestDeck = [];
for(var i = 0; i < CARDS; i++) cutTestDeck.push((i + 20) % CARDS);

var tests = [
    new TestCase(
        "finding cards in testDeck",
        testDeck,
        deckToIndices,
        [testDeck],
        decksAreEqual
    ),
    new TestCase(
        "finding cards in testDeckShifted",
        testDeck.map((x) => sub(x, 1)),
        deckToIndices,
        [testDeckShifted],
        decksAreEqual
    ),
    new EqTestCase(
        'finding 23 in cutTestDeck',
        3,
        deckFindCard,
        [cutTestDeck, 23]
    ),
    new EqTestCase(
        'finding 23 in cutTestDeck',
        35,
        deckFindCard,
        [cutTestDeck, 15]
    ),   
];

function runTests() {
    var passed = true;
    tests.map((test) => {
        if(!test.runTest(LOUD)) passed = false;
    });
    if(passed) console.log("Passed all tests!");
}

runTests();
