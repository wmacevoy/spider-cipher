"use strict";

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

function add(a, b) { return (a + b) % 40; }
// 40 is added to deal with the fact that % has odd behavior with negative numbers
function sub(a, b) { return ((a + 40) - b) % 40; }

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
