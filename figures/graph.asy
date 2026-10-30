// Tiny directed graph example for the Graph chapter:
// V = {0,1,2}, E = {1->0, 1->2, 2->0, 2->2}
unitsize(1cm);

real r = 0.5;              // vertex radius
pair v0 = (0, 3.2);
pair v1 = (3.5, 1.6);
pair v2 = (0.4, 0);

pen outline = black + 1.2bp;
pen edgepen = black + 1bp;
pen fillpen = gray(0.93);

defaultpen(fontsize(12pt));

void vertex(pair p, string s) {
  fill(circle(p, r), fillpen);
  draw(circle(p, r), outline);
  label(s, p);
}

// straight directed edge trimmed to the vertex circles
void edge(pair a, pair b) {
  pair d = unit(b - a);
  draw((a + r*d) -- (b - r*d), edgepen, Arrow(6bp));
}

edge(v1, v0);   // 1 -> 0
edge(v1, v2);   // 1 -> 2
edge(v2, v0);   // 2 -> 0

// self loop 2 -> 2, hanging below the vertex
pair pA = v2 + r*dir(240);
pair pB = v2 + r*dir(300);
draw(pA .. controls (v2 + 1.7*dir(220)) and (v2 + 1.7*dir(320)) .. pB,
     edgepen, Arrow(6bp));

vertex(v0, "$0$");
vertex(v1, "$1$");
vertex(v2, "$2$");
