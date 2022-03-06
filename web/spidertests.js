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

// It probably would be good to move this to a module or something. Unfortuantely, not all browsers
// (which can usually, and also in this case, be read as "all but IE") support them.
// ? Should I care about supporting IE? Will *anyone* - literally a single person - mind if I don't?
//
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

// This is a weird function mostly here to help with testing, its name isn't even very descriptive
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
