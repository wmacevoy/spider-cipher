var testDeck = [];
for(var i = 0; i < CARDS; i++) testDeck.push(i);
var deckString = testDeck.join(",");

function pageScramble(scrambleMsg = true) {
    var msgBox = document.getElementById('msg');
    var msg = msgBox.value;
    if(scrambleMsg) msg = translateString(msg);
    else msg = readDeckString(msg);
    var deck = readDeckString(document.getElementById('deck').value);
    if(scrambleMsg) msgBox.value = scramble(msg, deck);
    else msgBox.value = detranslate(unscramble(msg, deck));
}

// only for testing! kinda obviously. 
function testScramble(scrambleMsg = true) {
    var deckBox = document.getElementById('deck');
    if(deckBox.value == "") deckBox.value = deckString;
    pageScramble(scrambleMsg);
}

function decksAreEqual(a, b) {
    for(var i = 0; i < CARDS; i++) if(a[i] != b[i]) return false;
    return true;
}

// This is a weird function mostly here to help with testing, its name isn't even very descriptive
function deckToIndices(deck) {
    var indices = [];
    for(var i = 0; i < CARDS; i++) indices.push(deckFindCard(deck, i));
    return indices;    
}

function arraysAreEqual(a, b) {
    if(a.length != b.length) return false;
    for(var i = 0; i < a.length; i++) if(a[i] != b[i]) return false;
    return true;
}

var testDeckShifted = [];
for(var i = 0; i < CARDS; i++) testDeckShifted.push((i + 1) % CARDS);
var testDeckShiftedBack = [];
for(var i = 0; i < CARDS; i++) testDeckShiftedBack.push((i + 39) % CARDS);
var cutTestDeck = [];
for(var i = 0; i < CARDS; i++) cutTestDeck.push((i + 20) % CARDS);
// manually shuffled just to check
var backFrontShuffledDeck = [39, 37, 35, 33, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3, 1, 0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38];

var tests = [
    new TestCase("finding cards in testDeck", testDeck, deckToIndices, [testDeck], decksAreEqual),
    new TestCase("finding cards in testDeckShifted", testDeck.map((x) => sub(x, 1)), deckToIndices, [testDeckShifted], decksAreEqual),
    new EqTestCase('finding 23 in cutTestDeck', 3, deckFindCard,[cutTestDeck, 23]),
    new EqTestCase('finding 23 in cutTestDeck', 35, deckFindCard, [cutTestDeck, 15]),
    new EqTestCase('add identity', 3, add, [3, 0]),
    new EqTestCase('add identity 2', 3, add, [3, CARDS]),
    new EqTestCase('add rollover', 2, add, [3, 39]),
    new EqTestCase('sub identity', 3, sub, [3, 0]),
    new EqTestCase('sub identity 2', 3, sub, [3, CARDS]),
    new EqTestCase('sub rollover', 4, sub, [3, 39]),
    new TestCase("that a cut at the start does nothing", testDeck, deckCut, [testDeck, 0], decksAreEqual),
    new TestCase("normal cut", cutTestDeck, deckCut, [testDeck, 20], decksAreEqual),
    new TestCase("that a cut at the end shifts", testDeckShiftedBack, deckCut, [testDeck, 39], decksAreEqual),
    new TestCase("basic back-front shuffle", backFrontShuffledDeck, deckBackFrontShuffle, [testDeck], decksAreEqual),
    new TestCase("back-front unshuffle", testDeck, deckBackFrontUnshuffle, [backFrontShuffledDeck], decksAreEqual),
    new TestCase("deckstring reading", [32, 9], readDeckString, ["32, 9"], arraysAreEqual),
];

var codeStringWithoutEmoji = DOWN_CODES.slice(0, 34).join("").concat(CODES.slice(0, 34).join("")).concat(UP_CODES.slice(0, 34).join("")) + "$Qh";
var codeArrWithoutEmoji = [];
for(var i = 0; i < 34; i++) codeArrWithoutEmoji.push(i);
codeArrWithoutEmoji.push(39);
for(var i = 0; i < 34; i++) codeArrWithoutEmoji.push(i);
// deal with A-F being weird
codeArrWithoutEmoji = codeArrWithoutEmoji.concat([38, 10, 11, 12, 13, 14, 15, 39, 39]);
for(var i = 6; i < 34; i++) codeArrWithoutEmoji.push(i);
codeArrWithoutEmoji = codeArrWithoutEmoji.concat([38, 38, 21, 39, 39, 16, 38, 7]);

var codeString = DOWN_CODES
    .join("")
    + String.fromCharCode(0xD83D, EMOJICODES[0])
    + String.fromCharCode(0xD83D, EMOJICODES[1])
    .concat(CODES.join(""))
    + String.fromCharCode(0xD83D, EMOJICODES[2])
    + String.fromCharCode(0xD83D, EMOJICODES[3])
    .concat(UP_CODES.join(""))
    + String.fromCharCode(0xD83D, EMOJICODES[4])
    + String.fromCharCode(0xD83D, EMOJICODES[5])
    + "$Qh";
var codeArr = [];
for(var i = 0; i < 34; i++) codeArr.push(i);
codeArr = codeArr.concat([34, 35, 39]);
for(var i = 0; i < 34; i++) codeArr.push(i);
// deal with A-F being weird
codeArr = codeArr.concat([34, 35, 38, 10, 11, 12, 13, 14, 15, 39, 39]);
for(var i = 6; i < 34; i++) codeArr.push(i);
codeArr = codeArr.concat([34, 35, 38, 38, 21, 39, 39, 16, 38, 7]);

tests.push(new TestCase("translating a string, no emoji", codeArrWithoutEmoji, translateString, [codeStringWithoutEmoji], arraysAreEqual));
tests.push(new EqTestCase("detranslating, no emoji", codeStringWithoutEmoji, detranslate, [codeArrWithoutEmoji]));

// TODO: fix emoji, get these cases working
tests.push(new TestCase("translating a string, with emoji", codeArr, translateString, [codeString], arraysAreEqual));
tests.push(new EqTestCase("detranslating, with emoji", codeString, detranslate, [codeArr]));

for(var test = 0; test < 100; test++) {
    // generating a random deck
    var randDeck = [];
    var pullDeck = testDeck.slice();    
    for(var i = CARDS; i > 0; i--) {
        var randy = Math.floor(Math.random() * i);
        randDeck.push(pullDeck.splice(randy, 1)[0]);
    }
    // generating a random string of characters in our character set
    var str = "";
    for(var len = 0; len < 100; len++) {
        var chIndex = Math.floor(Math.random() * 36);
        var thisShift = Math.floor(Math.random() * 3);
        str += detranslateChar(chIndex, thisShift);
    }
    // first we want to test that detranslate(translate(str)) == str
    tests.push(new EqTestCase(
        `translation invertibility of ${str}`,
        str, 
        (str) => detranslate(translateString(str)),
        [str]
    ));
    // and now the big test, that decrypt(encrypt(str)) == str
    tests.push(new EqTestCase(
        `encryption invertibility of ${str}`,
        str, 
        (str) => detranslate(unscramble(scramble(translateString(str), randDeck), randDeck)),
        [str]
    ));
}

// ------------------quick documentation-----------------
// TestCase(msg, expected, func, args, check)
// 
// EqTestCase(msg, expected, func, args)
//   not deep equality, it's just shallow equality
// 
// BoolTestCase(msg, func, args)
//   if func.apply(args) is true, the test passes
// 
// msg gets said if you're in loud mode or when it fails
// expected is the expected value
// func is the function to run when testing
// args is the list of arguments to func
// check takes expected and func.apply(arglist) as args
//   and returns a bool of whether the test passed
// 
// pass a list of tests to runTests to run them
// ------------------------------------------------------

runTests(tests, beLoud = false);

s = "";
testCodes = [0xDE22, 0xDE04, 0xDC4E, 0xDC4D, 0xDC94, 0xDDA4]; 
for(var i = 0; i < testCodes.length; i++) s += String.fromCharCode(0xD83D, testCodes[i]);
console.log("Emoji test: " + s);
