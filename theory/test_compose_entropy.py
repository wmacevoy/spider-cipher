import math
from group import Group, SymmetricGroup

def test_compose_entropy():
    S=SymmetricGroup(5)
    p=S.prob_random()
    q=S.prob_random()
    r=S.prob_compose(p,q)

    b1 = S.info_bits(r)

    b2 = 0
    for x in S.elements:
        for y in S.elements:
            pxy=p(x)*q(y)
            if pxy > 0.0:
                xy=S.compose(x,y)
                b2 += -pxy*math.log2(r(xy))
    assert abs(b1-b2) < 1e-12                
