from svg import SVG

def small(x,dy=-3,s=0.75):
    size = 19.04*s
    return f'<tspan dy="{dy}" dx="0" style="font-size:{size}px">{x}</tspan>'

UPS=[('A','ALFA'),  ('B','BRAVO'), ('C','CHARLIE'),('D','DELTA'), ('E','ECHO'),('F','FOXTROT'),('G','GOLF'),('H','HOTEL'),('I','INDIA'),('J','JULIETT'),
     ('K','KILO'),    ('L','LIMA'),  ('M','MIKE'),   ('N','NOVEMBER'), ('O','OSCAR'), ('P','PAPA'), ('Q','QUEBEC'),('R','ROMEO'),('S','SIERRA'),('T','TANGO'),
     ('U','UNIFORM'),('V','VICTOR'),('W',"<tspan dy=\"-2\" x=\"0\">WHISKEY</tspan>"),('X','XRAY'),('Y','YANKEE'),('Z','ZULU'),(small('{'),'{curly'),(small('}'),'curly}'),(small('['),'[bracket'),(small(']'),'bracket]'),
     (small('_'),"<tspan dy=\"-4\" x=\"0\">UNDER</tspan><tspan x=\"0\" dy=\"3\">B_A_R</tspan>"),  (':','colon:'), ('!','bang!'),   ('&#96;','&#96;back&#96;'),('heart-broken',''),('heart-full',''),('',''),('',''),('',''),('','')]

DOWNS=[('0','zero'),('1','one'),('2','two'),('3','three'),('4','four'),('5','five'),('6','six'),('7','seven'),('8','eight'),('9','nine'),
       ('A','ALFA'),('B','BRAVO'),('C','CHARLIE'),('D','DELTA'),('E','ECHO'),('F','FOXTROT'),('@',"<tspan dy=\"-2\" x=\"0\">at</tspan>"),('=','equal'),('\\',"<tspan dy=\"-2\" x=\"0\">\\back\\</tspan><tspan x=\"0\" dy=\"3\">slash</tspan>"),('~','tilde'),
       ('#','hash'),('$','dollar'),('%',"<tspan dy=\"-2\" x=\"0\">percent</tspan>"),('^','^carat'),('&amp;','and'),('|','|pipe|'),('-','dash'),('+','plus'),('/',"<tspan dy=\"-2\" x=\"0\">/front/</tspan><tspan x=\"0\" dy=\"3\">slash</tspan>"),('*','astrisk*'),
       (small('↩',dy=3,s=1),"<tspan dy=\"-4\" x=\"0\">end</tspan><tspan x=\"0\" dy=\"3\">line</tspan>"),(';','semi;'),('?','ask?'),('&#39;','&#39;single&#39;'),('sad',''),('happy',''),('',''),('',''),('',''),('','')]

PLAINS=[('a','alfa'),('b','bravo'),('c','charlie'),('d','delta'),('e','echo'),('f','foxtrot'),('g','golf'),('h','hotel'),('i','india'),('j','juliett'),
        ('k','kilo'),('l','lima'),('m','mike'),('n','november'),('o','oscar'),('p','papa'),('q','quebec'),('r','romeo'),('s','sierra'),('t','tango'),
        ('u','uniform'),('v','victor'),('w',"<tspan dy=\"0\" x=\"0\">whiskey</tspan>"),('x','xray'),('y','yankee'),('z','zulu'),('&lt;','less'),('&gt;','more'),(small('('),'(paren'),(small(')'),'paren)'),
        (' ','s p a c e'),(',','comma,'),('.','period.'),('&#34;','&#34;double&#34;'),('thumb-down',"<tspan dy=\"22\" x=\"0\">BAD</tspan>"),('thumb-up',"<tspan dy=\"22\" x=\"0\">GOOD</tspan>"),
        ('',"<tspan dy=\"-7\" x=\"0\">dOWN sHIFT</tspan><tspan x=\"0\" dy=\"3.5\">oNCE</tspan>"),
        ('',"<tspan dy=\"-7\" x=\"0\">Up Shift</tspan><tspan x=\"0\" dy=\"3.5\">Once</tspan>"),
        ('',"<tspan dy=\"-9\" x=\"0\">down shift</tspan><tspan x=\"0\" dy=\"3.5\">lock</tspan>"),
        ('',"<tspan dy=\"-9\" x=\"0\">UP SHIFT</tspan><tspan x=\"0\" dy=\"3.5\">LOCK</tspan>")]

class Glyph(SVG):
    @staticmethod
    def build(params):
        number=int(params['number'])
        face = number % 10
        suit = number // 10
        shift = str(params['shift'])
        if number == 34:
            if shift == 'up': return GlyphHeartBad(params)
            if shift == 'none': return GlyphThumbBad(params)
            if shift == 'down': return GlyphFaceBad(params)

        if number == 35:
            if shift == 'up': return GlyphHeartGood(params)
            if shift == 'none': return GlyphThumbGood(params)
            if shift == 'down': return GlyphFaceGood(params)
        if number <= 34:
            if shift == 'up': return GlyphLetter(UPS[number],params)
            if shift == 'none': return GlyphLetter(PLAINS[number],params)
            if shift == 'down': return GlyphLetter(DOWNS[number],params)
            
    def __init__(self,params={}):
        SVG.__init__(self,params)
        self._params['@id']=f"{self.cardId()}-glyph-shift-{self.shift()}"
        self._params['@transform']=f"translate({self.x()},{self.y()})"
        self._params['tag']='g'
        
    def lines(self):
        return f"""<g style="stroke:#fff;stroke-width:0.25" id="{self.cardId()}-lines" transform="translate(-7,-14)">
<path d="m 0,0 14,0"/>
<path style="stroke-dasharray:0.25, 0.75"  d="m 1,7 12,0" />
<path d="m 0,14 14,0" />
</g>
"""
    def shift(self):
        return self._params['shift']
    
    def x(self):
        shift = self.shift()
        if shift == 'none': return 37.45
        if shift == 'up':   return 51
        if shift == 'down': return 51

    def y(self):
        center=34.0
        shift = self.shift()        
        if shift == 'none': return center
        if shift == 'up': return   center-14.0
        if shift == 'down': return center+14.0

class GlyphThumbGood(Glyph):
    def __init__(self,params={}):
        Glyph.__init__(self,params)
        self._params["@style"]="fill:none;stroke:#000;stroke-width:1.0"
    def parts(self):
        return f"""
{self.lines()}
<g transform="translate(-7,-14) translate(-0.10641977,-0.29657915)">{self.hand()}</g>
"""
    def hand(self):
        return f"""
<path
    d="m 6.1871597,13.485321 c -0.86873,-0.316667 -1.745357,-0.576073 -1.94806,-0.576468 -0.335622,-6.46e-4 -0.368552,-0.278773 -0.368552,-3.1126884 v -3.111973 l 1.415523,-1.699116 c 0.892948,-1.071843 1.575684,-2.12969 1.849356,-2.865441 0.575873,-1.54818795 0.661402,-1.65229395 1.35744,-1.65229395 1.501428,0 2.1336563,1.65433695 1.298337,3.39733595 -0.700765,1.462237 -0.700646,1.462354 1.4503803,1.462354 1.64411,0 1.953989,0.05407 2.337242,0.407813 0.352549,0.32541 0.414974,0.551277 0.308914,1.117732 -0.492867,2.513648 -0.533505,3.035154 -1.217172,5.9583704 -1.392301,1.458785 -3.5603873,1.739871 -6.4834083,0.674375 z"
    />
 <path
    d="m 8.6320627,0.53130865 c 0.971117,0.08316 0.52406,1.45114295 0.52406,1.45114295 l -1.289995,1.56277 c -1.547836,0.03925 -0.708151,-0.584561 -1.04812,-0.706968"
    />
 <rect
    width="3.5550032"
    height="8.3782034"
    x="0.30127177"
    y="6.6973658" />
"""

class GlyphThumbBad(Glyph):
    def __init__(self,params={}):
        Glyph.__init__(self,params)
        self._params["@style"]="fill:none;stroke:#000;stroke-width:1.0"
        
    def parts(self):
        return f"""
{self.lines()}
<g transform="translate(-7,-14) translate(-0.10642031,-0.64599116)">{self.hand()}</g>
"""
    def hand(self):
        return f"""
    <path
       d="m 6.1642365,1.456454 c -0.871272,0.316656 -1.750465,0.576052 -1.953762,0.576448 -0.336604,6.56e-4 -0.369631,0.278762 -0.369631,3.112571 v 3.111858 l 1.419668,1.699053 c 0.895561,1.071803 1.580294,2.129611 1.854768,2.865334 0.577558,1.548131 0.663337,1.652232 1.361413,1.652232 1.505822,0 2.1398995,-1.654275 1.302137,-3.39721 -0.702816,-1.462181 -0.702698,-1.462299 1.4546235,-1.462299 1.648923,0 1.959708,-0.05407 2.344084,-0.407798 0.35358,-0.325398 0.416189,-0.551256 0.309818,-1.11769 C 13.393046,5.575399 13.352289,5.053912 12.66662,2.130804 11.270244,0.67207403 9.0958115,0.39099803 6.1642365,1.456454 Z" />
    <path
       d="m 8.6162945,14.409985 c 0.97396,-0.08315 0.525595,-1.451089 0.525595,-1.451089 l -1.29377,-1.562712 c -1.552367,-0.03925 -0.710224,0.584539 -1.051189,0.706942" />    
       <rect
       width="3.5654082"
       height="8.3779297"
       x="0.30155382"
       y="2.0944207" />
"""

class GlyphHeartGood(Glyph):
    def __init__(self,params={}):
        Glyph.__init__(self,params)
        self._params["@style"]="fill:none;stroke:#000;stroke-width:1.0"
        
    def parts(self):
        return f"""
{self.lines()}
<g transform="translate(-7,-14)">
<path d="M 7,14 C -5.5,3.5 2,-3 7,2 12,-3 19.5,3.5 7,14 Z" />
</g>
"""

class GlyphHeartBad(Glyph):
    def __init__(self,params={}):
        Glyph.__init__(self,params)
        self._params["@style"]="fill:none;stroke:#000;stroke-width:1.0"
        
    def parts(self):
        return f"""
{self.lines()}
<g transform="translate(-7,-14)">
<path
  d="M 3.1192613,0.91816809 C -0.51575331,1.1343295 -2.3043291,6.5459524 6.5727643,13.572239 
     L 10.055526,6.1948296 5.8047553,7.7606133 6.7629245,2.3188525 
     C 5.5460584,1.2839481 4.244462,0.85125639 3.1192613,0.91816809 Z" />
<path
  d="M 11.5983,-0.03055905 C 10.626785,-0.0873111 9.5242056,0.22914926 8.4585571,0.98296888 
     L 6.9371668,5.7541131 12.359724,4.4696249 6.6548634,12.991543 
     c 0.14833,0.145957 0.2983343,0.292314 0.4533994,0.44009 
     C 17.163309,6.0077897 15.370964,0.189818 11.5983,-0.03055905 Z" />
</g>
"""

class GlyphFaceGood(Glyph):
    def __init__(self,params={}):
        Glyph.__init__(self,params)
        self._params["@style"]="fill:none;stroke:#000;stroke-width:1.0"
        
    def parts(self):
        return f"""
{self.lines()}
<g transform="translate(-7,-14)">
<ellipse cx="9.5" cy="4" rx="0.25" ry="0.75" />
<ellipse cx="4.5" cy="4" rx="0.25" ry="0.75" /> 
<circle cx="7" cy="7" r="7" />
<path d="m 3,8 c 1.5,4 6,4 8,0" />
</g>
"""

class GlyphFaceBad(Glyph):
    def __init__(self,params={}):
        Glyph.__init__(self,params)
        self._params["@style"]="fill:none;stroke:#000;stroke-width:1.0"
        
    def parts(self):
        return f"""
{self.lines()}
<g transform="translate(-7,-14)">
<ellipse cx="9.5" cy="4" rx="0.25" ry="0.75" />
<ellipse cx="4.5" cy="4" rx="0.25" ry="0.75" />
<circle cx="7" cy="7" r="7" />
<path d="m 3,10 c 1.5,-3 6,-3 8,0" />
</g>
"""

class GlyphLetter(Glyph):
    def __init__(self,letterHint,params={}):
        Glyph.__init__(self,params)
        self._params['letter']=letterHint[0]
        self._params['hint']=letterHint[1]        
    def letter(self):
        return f"""
<text xml:space=\"preserve\" text-anchor=\"middle\" >{self._params['letter']}</text>
"""
    def hint(self):
        return f"""
<text transform="translate(-8,-7) rotate(-90)" xml:space="preserve" text-anchor="middle" style="font-size:2.75px;line-height:1;font-family:Courier;fill:#ffffff;stroke:none">{self._params['hint']}</text>
"""
    def parts(self):
        return f"""
{self.lines()}
{self.hint()}
{self.letter()}
"""
