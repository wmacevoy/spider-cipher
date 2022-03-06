// ------------------quick documentation-----------------
// pass a list of tests to runTests to run them
// normal test constructor takes:
// msg, expected, func, arglist, checkfunc
// msg gets said if you're in loud mode or when it fails
// expected is the expected value
// func is the function to run when testing
// arglist is the list of arguments to func
// checkfunc takes expected and func.apply(arglist)
//   and returns a bool of whether the test passed
// EqTestCase(msg, expected, func, args)
//   not deep equality, it's just shallow equality
// BoolTestCase(msg, func, args)
//   if func.apply(args) is true, the test passes
// ------------------------------------------------------

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

function runTests(tests, beLoud = true) {
    var passed = true;
    try {
        tests.map((test) => {
            if(!test.runTest(beLoud)) passed = false;
        });
    } catch(e) {
        console.error(e);
        passed = false;
    }
    if(passed) console.log("Passed all tests!");
}

