// Decision tree for a coin toss followed by a die roll
unitsize(1cm);

real bw = 1.5;   // box width
real bh = 0.8;   // box height

pen outline = black + 1bp;
pen edgepen = black + 0.8bp;
pen fillpen = gray(0.93);

defaultpen(fontsize(14pt));

void node(pair p, string s) {
  path b = shift(p - (bw/2, bh/2)) * scale(bw, bh) * unitsquare;
  fill(b, fillpen);
  draw(b, outline);
  label(s, p);
}

// link from bottom of parent to top of child
void link(pair parent, pair child) {
  draw((parent - (0, bh/2)) -- (child + (0, bh/2)), edgepen, Arrow(5bp));
}

pair root = (0, 4);
pair cT = (-4.2, 2);
pair cH = ( 4.2, 2);

pair[] leavesT = {(-6.9, 0), (-5.2, 0), (-1.8, 0)};
pair[] leavesH = {( 1.8, 0), ( 3.5, 0), ( 6.9, 0)};

link(root, cT);
link(root, cH);
for (pair p : leavesT) link(cT, p);
for (pair p : leavesH) link(cH, p);

node(root, "$[?,?]$");
node(cT, "$[T,?]$");
node(cH, "$[H,?]$");

node(leavesT[0], "$[T,1]$");
node(leavesT[1], "$[T,2]$");
label("$\cdots$", (-3.5, 0));
node(leavesT[2], "$[T,6]$");

node(leavesH[0], "$[H,1]$");
node(leavesH[1], "$[H,2]$");
label("$\cdots$", (5.2, 0));
node(leavesH[2], "$[H,6]$");
