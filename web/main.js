var defaultDeck = "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39";

function pageScramble(scrambleMsg = true) {
    var msgBox = document.getElementById('msg');
    var deckBox = document.getElementById('deck');
    if(scrambleMsg) msgBox.value = scramble(msgBox.value, deckBox.value);
    else msgBox.value = unscramble(msgBox.value, deckBox.value);
}

// only for testing! kinda obviously. 
function testScramble(scrambleMsg = true) {
    var msgBox = document.getElementById('msg');
    var deckBox = document.getElementById('deck');
    if(deckBox.value = "") deckBox.value = defaultDeck;
    if(scrambleMsg) msgBox.value = scramble(msgBox.value, deckBox.value);
    else msgBox.value = unscramble(msgBox.value, deckBox.value);
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

runTests(tests);
