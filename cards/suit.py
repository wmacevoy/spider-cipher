import math
import info
from maybe import Maybe
from geom import Point
from svg import SVG

def pc(r,theta):
    p=Point.polar(r,theta)
    return f"{p.x},{p.y}"

def cc(x,y):
    p=Point.cartesian(x,y)
    return f"{p.x},{p.y}"

class Suit(SVG):
    @staticmethod
    def build(params):
        suit = info.suitNo(params)
        if suit == 0: return SuitClub(params)
        if suit == 1: return SuitDiamond(params)
        if suit == 2: return SuitHeart(params)
        if suit == 3: return SuitSpade(params)
        raise ValueError("invalid params")

    def __init__(self,params={}):
        SVG.__init__(self,params)
        self._params['@id']=f"{info.cardId(params)}-suit"
        self._params['tag']='g'
        self._params['@style']=f"stroke:none;fill:{info.color(params)}"
        self._params['@transform']="matrix(2,0,0,2,-6,-6)"
        
class SuitClub(Suit):
    def __init__(self,params={}):
        Suit.__init__(self,params)
        
    @property
    def r(self): return float(getattr(self._params,'r',4.0))

    @property
    def R(self): return float(getattr(self._params,'R',13.0))

    @property
    def delta(self): return float(getattr(self._params,'delta',15.0))

    @property
    def h(self): return float(getattr(self._params,'h',7.0))

    @property
    def w(self): return float(getattr(self._params,'w',3.0))

    def parts(self):
        return f"""<g transform="translate(7,7)">
{self.flower()}
{self.stem()}
</g>
"""

    def flower(self):
        a=[270,270+120,270+240]
        r=self.r
        R=self.R
        delta=self.delta
        
        return f"""
<path d="M {pc(r,a[0])}
        C {pc(R,a[0]+delta)} {pc(R,a[1]-delta)} {pc(r,a[1])} 
        C {pc(R,a[1]+delta)} {pc(R,a[2]-delta)} {pc(r,a[2])} 
        C {pc(R,a[2]+delta)} {pc(R,a[0]-delta)} {pc(r,a[0])} 
        Z " />
"""

    def stem(self):
        h=self.h
        w=self.w
        return f"""
<path d="m 0,{h} h -{w/2} 
        c {w/2},{-w/2} {self.w/2},{-h} {w/2},{-h} 
        c 0,0 {0},{h-w/2} {w/2},{h} 
        z" />
"""
    
class SuitDiamond(Suit):
    def __init__(self,params={}):
        Suit.__init__(self,params)
    
    def parts(self): return f"{self.path()}"
    def path(self): return f"""
      <path d="m 7,14 -7,-10 7,-4 7,4 z" />
"""

class SuitHeart(Suit):
    def __init__(self,params={}):
        Suit.__init__(self,params)
    def parts(self): return f"{self.path()}"
    def path(self): return f"""
<path d="M 7,14 C -5.5,3.5 2,-3 7,2 12,-3 19.5,3.5 7,14 Z" />
"""

class SuitSpade(Suit):
    def __init__(self,params={}):
        Suit.__init__(self,params)
    def parts(self): return f"{self.point()}{self.stem()}"

    def point(self):
        return f"<path d=\"M 7,0 C -5.5,10.5 2,17 7,12 12,17 19.5,10.5 7,0 Z\" />"
    def stem(self):
        return f"<path d=\"m 7,14 h -1.5 c 1.5,-1.5 1.5,-7.0 1.5,-7.0 c 0,0 0,5.5 1.5,7.0 z \" />"

    
