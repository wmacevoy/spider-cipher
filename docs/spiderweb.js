"use strict";

// TODOS
// rewrite using typed arrays
// fix utf-16 support
// figure out how to do caching-safe translation and detranslation

/* Written primarily by me, Caleb Spiess
 * Based VERY heavily on Warren MacEvoy's C code for the same algorithm
 * I implemented the tricky constant-time parts other than the find.
 *
 * Front note: This code is written with timing attacks in mind. It is true that a
 * "normal" timing attack, using how long a request takes to go through, provides
 * some information. However, much more importantly, there is timing information
 * with no privileges needed: cache timing. If a malicious program can find code
 * which, due to its properties, pushes some of this code out of an instruction
 * or data cache, then this code will push theirs out in turn. If the attacker can
 * figure out which cache misses correspond to which lines of execution, shockingly
 * granular information can be gained. This can be used to steal a key quite
 * quickly in some situations. Because this is browser code, these side channel
 * attacks are particularly viable. As such, code will be written to attempt
 * to mitigate these attacks, as well as explanation for how that code works so
 * that this code base may be more safely adapted to other languages, and checked
 * for errors in this regard.
 *
 * Of note, the data itself is irrelevant. What matters is the location in memory.
 * Also, modern CPUs "stripe" their caches, making this much more practical - as an
 * example probably pretty numerically far off, but with the right idea,
 * accesses where the second to last byte in physical memory is an 8 might 
 * all take up the same set of cache slots. This striping provides a speed
 * increase, but makes this attack much easier.
 *
 * Though the instruction cache is much smaller and faster, it is also relevant here.
 * In basically exactly the same way, in fact. In other words, we need
 * secret-independent data accesses and secret-independent execution paths. To the 
 * unfamiliar, how this is actually achieved will make more sense when you see it.
 */

// like most languages, javascript's const implementation is... screwy, but even just marking something as such is useful
const CARDS = 40;
const LOUD = false;
const RANDFUNC = Math.random;

// Macros are used in your code, which avoids making stack frames, which has several advantages.
// Javascript is not so kind. This is slower, but as far as I can tell shouldn't introduce any security issues.
function add(a, b) { return (a + b) % CARDS; }
// CARDS is added to deal with the fact that % has odd behavior with negative numbers
function sub(a, b) { return ((a + CARDS) - b) % CARDS; }
// You only use the cut pad once ish and it's really just the top card, and is defined by several layers of indirection.
// I'm gonna just leave it out unless there's a reason to re-include it.
// The mark is similarly used only once, in finding the tag. Seems like you just completely removed that terminology from the slides.
// I agree with this decision. I was thinking about making a note to suggest it already.

// basically just pasted the code and changed some names
// with the exception that the output works differently
//
// No extra work needs to be done for timing attack mitigation,
// as all data is always accessed
function deckCut(inputDeck, cutLoc) {
    var outputDeck = inputDeck.slice(0);
    for (var i=0; i<CARDS; ++i) {
        outputDeck[i]=inputDeck[(i+cutLoc) % CARDS];
    }
    return outputDeck;
}

// same here
function deckBackFrontShuffle(inputDeck) {
    var outputDeck = inputDeck.slice(0);
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
    var outputDeck = inputDeck.slice(0);
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


// BEGIN QUARANTINE ZONE


// both in terms of consistent timing and unicode support
// maybe requires a hash table unfortunately

// WARNING: DO NOT ACCESS THESE ARRAYS DIRECTLY FOR TRANSLATION
// emoji are too weird to slap into a homogeneous data structure like this
// use translateChar or translateString

// Some translation was needed; emoji are encoded slightly different here.
var DOWN_CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',  // + 0 club
    'A', 'B', 'C', 'D', 'E', 'F', '@', '=','\\', '~',  // +10 diamond
    '#', '$', '%', '^', '&', '|', '-', '+', '/', '*',  // +20 heart
    '\n', ';', '?','\'',                               // +30 spade
];
// HEED THE ABOVE WARNING, LEST YE MEET A WATERY GRAVE
var CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',  // + 0 club
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',  // +10 diamond
    'u', 'v', 'w', 'x', 'y', 'z', '<', '>', '(', ')',  // +20 heart
    ' ', ',', '.','\"',                                // +30 spade
];
// HEED THE ABOVE WARNING, LEST YE MEET A WATERY GRAVE
var UP_CODES = [
    //Q    A    2    3    4    5    6    7    8    9
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',  // + 0 club
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',  // +10 diamond
    'U', 'V', 'W', 'X', 'Y', 'Z', '{', '}', '[', ']',  // +20 heart
    '_', ':', '!', '`',                                // +30 spade
];
// HEED THE ABOVE WARNING, LEST YE MEET A WATERY GRAVE
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

// I'm rewriting this to be significantly different. I just want to get the thing working for now.
// gotta deal with the annoyingness of emoji somehow
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
    if(shift < 0 || shift >= 3) throw `Invalid shift of ${shift} in detranslateChar`;
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
        else {
            if(shift < 0 || shift >= 3) throw `Invalid shift of ${shift} in detranslate, arr=${arr}`;
            detranslated += detranslateChar(arr[i], shift);
        }
    }
    return detranslated;
}


// END QUARANTINE ZONE

// does not include x
function zeroTo(x) {
    return Math.floor(RANDFUNC() * x);
}

function randCard() {
    return zeroTo(CARDS);
}

// packet and unpacket are actually independent of the contents of the plaintext and 
// key deck alike, so these very hefty-looking pieces of code are quite safe
function packet(msg) {
    for(var i = 0; i < 5; i++) msg.push(39);
    var len = msg.length;
    // geez, the splice function just has so much going on
    // anyway, the array is being modified while you're inserting, so this is a bit
    // weird to think about, but start by inserting at position 0, then 0 and 1 are
    // both occupied; to insert before the next unhandled plain card is to insert
    // at index 2, now.
    for(var i = 0; i < len; i++) msg.splice(i * 2, 0, randCard());
    // unshift is a bit obscure and weirdly named; it's analogous to push, but operates
    // on the front of an array
    for(var i = 0; i < 10; i++) msg.unshift(randCard());
    while((msg.length % 10) != 0) msg.push(randCard());
    return msg;
}

function unpacket(msg) {
    // find end of message
    var count = 0;
    // gotta start looking at 11; it's the first one that even might be it, since the
    // first 10 are random and *could* coincidentally look like EOM
    // index starts at 9 since it increases before things even start
    // we can assume that there are no 39s before the EOM marker since it doesn't
    // make sense to end by shifting, so the very first run of 5 39s is EOM for sure
    var index = 9;
    while(count < 5) {
        index += 2;
        if(index >= msg.length) throw "Message doesn't check out!";
        if(msg[index] == 39) count++;
        else count = 0;
    }
    // this lands index at the last 39
    // delete everything *after* the last 39
    msg.splice(index + 1);
    // now get rid of the junk at the start
    msg.splice(0, 10)
    // this might seem kinda confusing
    // at the start, index 0 has a junk card, so remove that
    // now index 0 is a real card, index 1 is the next junk
    // remove that, and it gets replaced by a real card
    var len = msg.length / 2;
    for(var i = 0; i < len; i++) msg.splice(i, 1);
    for(var i = 0; i < 5; i++) msg.pop();
    return msg;
}

// This one is tricky. Does accessing the second element of the deck leak information?
// The value we're accessing doesn't matter, but its address does. If this element has
// a consistent location, we have no problem. If, however, javascript's array is moreso
// a list of pointers to numbers... we have a problem. This is the reason we use typed
// arrays rather than just normal javascript arrays: sure, there's an efficiency gain,
// but this is the main thing.
function noise(deck) {
    var tagVal = add(39, deck[2]); // same as subtracting 1
    var tagIndex = deckFindCard(deck, tagVal);
    return deck[add(tagIndex, 1)];
}

/* Oh boy.
 * Explanation:
 * The first part of this is quite similar to the C version.
 * ~0 is all 1's in binary, and in this case I don't need to worry about the truthiness of that. So a mask is selected based
 * on whether the current index is correct or not. It's anded with the index, which gives either the i & 0 (0) or i & 1 (i).
 * Since you've got 39 zeroes with (hopefully) an i in there, if you just or the pile together, the result is i or 0 or 0
 * or 0 ......... or 0. Which is still just i, and remains so in any order for any i. The difference here is that you should
 * be able to accumulate as you go without problems. As far as I can tell, this is constant time, constant space. There is an
 * implicit "if" here in the loop, but it runs the same every time. The only thing that changes between runs is which deck[i]
 * is the same as the card and therefore which entry is selected in the mask.
 *
 * Note that this is a pretty funny solution - the algorithm is O(N), but N is constant and we always take the worse case.
 * So it always takes the same amount of time to run for our purposes. You can definitely do better in the general case, but
 * doing so requires keeping track of an inverse map of your deck which is definitely not a safe method against cache attacks.
 */
function deckFindCard(deck, card) {
    var acc = 0;
    var mask = {false: 0, true: ~0};
    for(var i = 0; i < CARDS; i++) acc = acc | (i & mask[deck[i]==card]);
    return acc;
}

// Here, there is a concept of a source and an output. The source text is the
// plaintext when encrypting and the ciphertext when decrypting. The output is the
// ciphertext when encrypting and the plaintext when decrypting.
//
// Making everything work requires a lot of ternary operator fiddling, but each instance
// is only once per encrypt/decrypt, so it's not a huge deal. I also think this is
// relatively readable for what it's doing.
//
// Finally, are these components secure against timing attacks? Most things are clearly
// fine. Most of this happens the same way no matter what the plaintext and deck are,
// in terms of the memory, thanks to the work in deckFindCard. However, the same problem
// as with noise occurs here. Fortunately, looking up a constant location per iteration
// is safe with a typed array, as it allows us to manipulate bits rather than shuffle
// around pointers to bits.
function spider(deck, source, mode) {
    // processing mode text to set doEncrypt
    var doEncrypt;
    mode = mode.toLowerCase().trim();
    if(mode == "encrypt") doEncrypt = true;
    else if(mode == "decrypt") doEncrypt = false;
    else throw "Not a valid mode. Valid modes are 'encrypt' and 'decrypt' only";     
    var doDecrypt = !doEncrypt;

    // encryption/decryption setup
    if(doEncrypt) source = packet(source);
    var output = [];
    var f = doEncrypt ? add : sub;
    var plain = doEncrypt ? source : output;

    // the workhorse
    for(var i = 0; i < source.length; i++) {
        output.push(f(source[i], noise(deck)));
        deck = deckCut(deck, deckFindCard(add(deck[0], plain[i])));
        deck = deckBackFrontShuffle(deck);
        deck = deckCut(deck, deckFindCard(sub(deck[2], 1)));
    }

    // cleanup
    if(doDecrypt) output = unpacket(output);
    return output;
}
