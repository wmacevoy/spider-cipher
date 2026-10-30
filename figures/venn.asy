// Venn diagram for two generic sets A and B
import graph;
unitsize(1cm);

// Geometry
real r = 2.2;
pair A_center = (-1.7, 0);
pair B_center = ( 1.7, 0);

// Universal set rectangle
pair UL = (-5, -3.5);
pair UR = ( 5, -3.5);
pair UR2 = ( 5,  3.5);
pair UL2 = (-5,  3.5);
draw(UL--UR--UR2--UL2--cycle, black + 1bp);

// Circles and overlap
path cA = circle(A_center, r);
path cB = circle(B_center, r);

fill(cA, gray(0.93));
fill(cB, gray(0.93));

pair[] inter = intersectionpoints(cA, cB);
if(inter.length >= 2) {
  pair top = inter[0];
  pair bottom = inter[1];
  if(top.y < bottom.y) { pair tmp=top; top=bottom; bottom=tmp; }

  real a_top = degrees(top - A_center);
  real a_bottom = degrees(bottom - A_center);
  real b_top = degrees(top - B_center);
  real b_bottom = degrees(bottom - B_center);

  // clockwise: the short arcs that bound the overlap, not the long way around each circle
  path overlap = arc(A_center, r, a_top, a_bottom, CW) --
                 arc(B_center, r, b_bottom, b_top, CW) -- cycle;
  fill(overlap, gray(0.78));
}

pen outline = black+1.2bp;
draw(cA, outline);
draw(cB, outline);

// Labels
defaultpen(fontsize(14pt));

label("$U$", (4.8, 3.4), SW);
label("$A$", A_center + (0, r+0.3));
label("$B$", B_center + (0, r+0.3));

label("$A \cap B$", (0,0));
label("$A - B$", (-2.8, 0));
label("$B - A$", ( 2.8, 0));
