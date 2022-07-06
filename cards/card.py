import info
from svg import SVG
from geom import Point
from suit import Suit
from glyph import Glyph,GlyphArrow

class Card(SVG):
    @staticmethod
    def build(params):
        number = int(params['number'])
        if number < 40: return CardPlay(params)
        if number < 42: return CardCodes(params)
        if number == 42: return CardDefs(params)
        if number == 43: return CardTranslate(params)
        if number == 44: return CardPacket(params)
        if number == 45: return CardScramble(params)
        if number == 46: return CardClear(params)        
    
    def __init__(self,params={}):
        SVG.__init__(self,params)
        if not 'tag' in self._params:
            self._params['tag']='g'
        if not '@id' in self._params:
            self._params['@id']=self.cardId()

    def number(self): return info.number(self._params)

    def faceNo(self): return info.faceNo(self._params)

    def suitNo(self): return info.suitNo(self._params)

    def suit(self): return info.suit(self._params)

    def color(self): return info.color(self._params)

    def cardId(self): return info.cardId(self._params)

class CardPlay(Card):
    def __init__(self,params={}):
        Card.__init__(self,params)
        self._suit = Suit.build(params)
        self._glyphs={}
        
        if self.number() < 36:
            for shift in ['none','up','down']:
                params = self._params.copy()
                params['shift']=shift
                self._glyphs[shift]=Glyph.build(params)
        else:
            self._glyphs['shifter']=Glyph.build(params)
           
    def suit(self): return str(self._suit)

    def glyphs(self):
        ans = f'<g id="{self.cardId()}-glyphs" >'
        for glyph in self._glyphs:
            ans = ans + str(self._glyphs[glyph])
        ans = ans + f'</g><!-- {self.cardId()}-glyphs -->'
        return ans

    def code(self):
        return f"""
<g>
<text x="16" y="20" id="{self.cardId()}-code" style="font-family:Helvetica;font-size:20px;fill:#fff" text-anchor="middle">{self.suitNo()}{self.faceNo()}</text>
</g>
"""
    def dot(self):
        return f"""
<g id="{self.cardId()}-morse-dot">
  <circle cx="1" cy="0" r="0.5" style="stroke:none;fill:#fff" />
</g>
"""
    
    def dash(self):
        return f"""
<g id="{self.cardId()}-morse-dash">
  <rect x="0.125" y="-0.5" width="1.75" height="1" style="stroke:none;fill:#fff" />
</g>
"""

    def morse(self,x):
        codes = []
        if x < 5:
            codes = [ 'dot' if i < x else 'dash' for i in range(5)]
        else:
            codes = [ 'dot' if i >= x-5 else 'dash' for i in range(5)]
        
        x = 0
        ans = ""
        for code in codes:
            ans = ans + f"""<use xlink:href="#{self.cardId()}-morse-{code}" transform="translate({x},0)" />"""
            x = x + 2
        return ans

    def top(self):
        return f"""<g id="{self.cardId()}-top">
<use xlink:href="#{self.cardId()}-morse" />
<use xlink:href="#{self.cardId()}-morse" transform="translate(0,33) rotate(-90)" />
<use xlink:href="#{self.cardId()}-code" />
<use xlink:href="#{self.cardId()}-glyphs" />
</g>
"""
    def defs(self):
        return f"""
<defs>
{self.dot()}
{self.dash()}
<g id="{self.cardId()}-morse" transform="translate(6,4)">
  <g>{self.morse(self.suitNo())}</g>
  <g transform="translate(11,0)">{self.morse(self.faceNo())}</g>
</g>
{self.suit()}
{self.code()}
{self.glyphs()}
{self.top()}
</defs>
"""
    def bg(self):
        return f"""<g id="{self.cardId()}-bg">
<rect x="-3" y="-3" width="69" height="94" style="fill:#808080" />
<use xlink:href="#{self.cardId()}-suit" />
<use xlink:href="#{self.cardId()}-formats" />

<g transform="rotate(180,31.5,44)">
  <use xlink:href="#{self.cardId()}-suit" />
  <use xlink:href="#{self.cardId()}-formats" />
</g>
<rect x="-1.5" y="-1.5" width="66" height="91" rx="6.35" opacity="1.0" style="fill:none;stroke:#ffffff;stroke-width:9.0" />
<path style="stroke:#ffffff;stroke-width:1.75;fill:none;" d="M -3.0,30 C 18.5,30 44.5,58 66,58" />
</g><!-- {self.cardId()}-bg -->
"""
    def fg(self):
        return f"""<g id="{self.cardId()}-fg">
<use xlink:href="#{self.cardId()}-top" />
<g transform="rotate(180,31.5,44)">
  <use xlink:href="#{self.cardId()}-top" />
</g>
</g><!-- {self.cardId()}-fg -->
"""
    def parts(self):
        return f"""
{self.bg()}
{self.fg()}
"""
#    def xml(self):
#        return f"""<?xml version="1.0" encoding="UTF-8" standalone="no"?>"""
#    def osvg(self):
#        return f"""<svg width="69mm" height="94mm" viewBox="-3 -3 69 94" version="1.1" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg">"""
#    def csvg(self):
#        return f"""</svg>"""
    
#    def __str__(self):
#        return f"""{self.xml()}{self.osvg()}{self.defs()}{SVG.__str__(self)}{self.csvg()}"""

class CardRules(Card):
    def __init__(self,params={}):
        Card.__init__(self,params)
        self._params['tag']='g'

    def bg(self):
        return f"""<g id="{self.cardId()}-bg">
<rect x="-3" y="-3" width="69" height="94" style="fill:#808080" />
<rect x="-1.5" y="-1.5" width="66" height="91" rx="6.35" opacity="1.0" style="fill:none;stroke:#ffffff;stroke-width:9.0" />
</g><!-- {self.cardId()}-bg -->
"""

    def typeset(self,words):
        lines=words.split("<br/>")
        ans = ""
        dy = 0
        for line in lines:
            lh = 5
            line=line.replace("\r","")
            line=line.replace("\n","")
            h1 = False
            if line.startswith('<h1>'):
                line = line.replace("<h1>",f"""<tspan style="fill:#000" font-size="5px" xml:space="preserve">""").replace("</h1>","""</tspan>""")
                lh = max(5.5,lh)
            line = line.replace("<sup>",f"""<tspan dy="-2" font-size="3px" font-style="italic" xml:space="preserve">""").replace("</sup>","""</tspan><tspan dy="2">&#8203;</tspan>""")
            if "<b>" in line:
                line = line.replace("<b>",f"""<tspan style="fill:#000" xml:space="preserve">""").replace("</b>","""</tspan>""")
            if "<t2>" in line:
                line = line.replace("<t2>",f"""<tspan style="font-family:Menlo;font-size:3.9px;fill:#fff" xml:space="preserve">""").replace("</t2>","""</tspan>""")
                lh = max(4.5,lh)
            if "<t1>" in line:                
                line = line.replace("<t1>",f"""<tspan style="font-family:Menlo;font-size:6px;fill:#000" xml:space="preserve">""").replace("</t1>","""</tspan>""")
                lh = max(6.0,lh)
            ans = ans + f"""<text dy="{dy}" style="font-family:Helvetica;font-size:4px;fill:#fff" xml:space="preserve">{line}</text>"""
            dy = dy + lh
        return ans


    def rotparts(self): return ""

    def parts(self):
        return f"""{self.bg()}
<g transform="translate(54,4.5) rotate(90)">{self.rotparts()}</g>
"""
    def defs(self):
        return f"""
<defs>
</defs>
"""

class CardCodes(CardRules):
    def __init__(self,params={}):
        CardRules.__init__(self,params)
        self._params['height']=6
        self._params['gap']=2

        if self.number() == 40:
            self._params['n'] = [30,20]
        elif self.number() == 41:
            self._params['n'] = [10,0]

    def line(self):
        height=self._params['height']                                 
        return f"""<g style="stroke:#fff;stroke-width:0.25">
<path d="m -{height/2},0 0,94"/>
<path style="stroke-dasharray:0.25, 0.75"  d="m 0,0 0,94" />
<path d="m {height/2},0 0,94" />
</g>
"""
    def x(self,code,s):
        center=31.5
        gap=self._params['gap']
        height=self._params['height']
        n=self._params['n']
        
        if n[0] <= code and code < n[0]+10:
            return center-2.5*gap-1.5*height+(height+gap)*s-height/2
        if n[1] <= code and code < n[1]+10:
            return center+2.5*gap+1.5*height+(height+gap)*s-height/2

        return None
    
    def y(self,code,s):
        center=44+2
        gap=self._params['gap']
        height=self._params['height']        
        return center+(height+gap)*((code % 10)-4.5)


    def glyphs(self):
        ans = ""
        shifts = ['down','none','up']
        for n in self._params['n']:
            for k in range(10):
                nk = n+k                
                if nk >= 36:
                    for s in [-1,1]:
                        up = 1 if nk in [37,39] else -1
                        lock = nk >= 38
                        y = self.y(nk,0)
                        
                        xa = self.x(nk,0+0.5)
                        xb = self.x(nk,s+0.5)
                        xh = xa if s*up == -1 else xb
                        xt = xb if s*up == -1 else xa
                        head = Point(xh,y)
                        tail = Point(xt,y)
                        tail = tail.scale(0.8,head)

                        params = {'head':head,
                                  'tail':tail,
                                  'lock':lock,
                                  'tipAngle':90,
                                  'tipWidth':3,
                                  'shaftWidth':1,
                                  '@style':"stroke:none;fill:#fff"}
                        ans = ans + str(GlyphArrow.build(params)) + f" <!-- arrow {n+k} s={s} parms={params} -->"
                else:
                    for s in [-1,0,1]:
                        x = self.x(n+k,s)
                        y = self.y(n+k,s)
                        shift = shifts[s+1]
                        glyph=Glyph.build({'number':n+k,'shift':shift,'lines':False, 'hints':True})
                        ans = ans + f"""<g transform="translate({x},{y}) scale(0.40) rotate(90)" {glyph.attr('style')}>{glyph.parts()}</g>"""
        return ans;

    def numbers(self):
        gap=self._params['gap']        
        ans=""
        for n in self._params['n']:
            for k in range(10):
                nk=n+k
                face = nk % 10
                suit = nk // 10
                x = self.x(nk,2)-gap
                y = self.y(nk,0)
                ans = ans + f"""<g transform="translate({x},{y}) rotate(90)"><text style="font-family:Courier;font-size:4px;fill:#fff" text-anchor="middle">{suit}{face}</text></g>"""
        return ans
    
    def lines(self):
        center=31.5
        gap=self._params['gap']
        height=self._params['height']
                                 
        return f"""
<g transform="translate({center-2.5*gap-1.5*height},0)">
<g transform="translate({height+gap},0)">{self.line()}</g>
<g transform="translate(0,0)">{self.line()}</g>
<g transform="translate(-{height+gap},0)">{self.line()}</g>
</g>
<g transform="translate({center+2.5*gap+1.5*height},0)">
<g transform="translate({height+gap},0)">{self.line()}</g>
<g transform="translate(0,0)">{self.line()}</g>
<g transform="translate(-{height+gap},0)">{self.line()}</g>
</g>
"""
    def parts(self):
        return f"""
{self.bg()}
{self.lines()}
{self.numbers()}
{self.glyphs()}
"""

class CardDefs(CardRules):
    def __init__(self,params={}):
        CardRules.__init__(self,params)

    def shuffle(self):
        words=f"""<h1>BACK FRONT SHUFFLE</h1><br/>
 <b>S1</b> Separate cards into two stacks:<br/>
   BACK is 1<sup>st</sup>,3<sup>rd</sup>,5<sup>th</sup>,...,35<sup>th</sup>,37<sup>th</sup>,39<sup>th</sup>.<br/>
   FRONT is 2<sup>nd</sup>,4<sup>th</sup>,6<sup>th</sup>,...,34<sup>th</sup>,36<sup>th</sup>,38<sup>th</sup>.<br/>
 <b>S2</b> Flip BACK (39<sup>th</sup>,37<sup>th</sup>,35<sup>th</sup>,...).<br/>
 <b>S3</b> Stack FRONT on BACK to combine.<br/>
"""
        return self.typeset(words)
        
    def cut(self):
        words=f"""<h1>CUT DECK ON CARD</h1><br/>
 <b>C1</b> Move any cards above <b>card</b> to back.<br/>
"""
        return self.typeset(words)

    def rotparts(self):
        return f"""
<g transform="translate(0,0)">{self.shuffle()}</g>
<g transform="translate(0,35)">{self.cut()}</g>
"""

class CardTranslate(CardRules):
    def __init__(self,params={}):
        CardRules.__init__(self,params)

    def overview(self):
        words=f"""<h1>1 - TRANSLATE RULES</h1><br/>
<b>T1</b> Shift is <t2>none</t2> to start.<br/>
<b>T2</b> <t2>00-35</t2> translate according to shift.<br/>
     Ex: <t2>03</t2> is <b>d</b> (<t2>none</t2>) <b>D</b> (<t2>up</t2>) or <b>3</b> (<t2>down</t2>).<br/>
<b>T3</b> Cards 36-39 change shift.<br/>
     <t2>36</t2><t1>↓</t1>/<t2>37</t2><t1>↑</t1> down/up once<br/>
     <t2>38</t2><t1>⤈</t1>/<t2>39</t2><t1>⤉</t1> down/up lock.<br/>
<h1>EXAMPLE: I♥3.14</h1><br/>
   <t1>⤉ I ♥ ⤈ ⤈ 3 ↑ . 1 4</t1><br/>
   <t2>39 08 35 38 38 03 36 32 01 04</t2>
"""
        return self.typeset(words)
    
    def rotparts(self):
        return f"""
<g transform="translate(0,0)">{self.overview()}</g>
"""

class CardPacket(CardRules):
    def __init__(self,params={}):
        CardRules.__init__(self,params)

    def overview(self):
        words=f"""<h1>2 - PACKET</h1><br/>
<b>P1</b> Add 5x <t2>39</t2> cards last: <t2>39 39 39 39 39</t2>.<br/>
<b>P2</b> Add die-roll cards so length is 10,20,30,...<br/>
<b>P3</b> Add die-roll cards before each card.<br/>
<b>P4</b> Add 10 random cards first.<br/>
<br/>
<h1>EXAMPLE hi.</h1>/<t2>07 08 32</t2><br/>
 <t2>P40 P41 P42 P43 P44 P45 P46 P47 P48 P49</t2><br/>
 <t2>P30  07 P31  08 P32  32 P33  39 P34  39</t2><br/>
 <t2>P35  39 P36  39 P37  39 P38 P20 P39 P21</t2><br/>
 """
        tsw=self.typeset(words)
        thumbsUp=str(Glyph.build({'number':35,'shift':'none','lines':False}))
        return f"""{tsw}<g transform="translate(22,20) scale(0.30)">{thumbsUp}</g>"""
    
    def rotparts(self):
        return f"""
<g transform="translate(0,0)">{self.overview()}</g>
"""
    
class CardCipher(CardRules):
    def __init__(self,params={}):
        CardRules.__init__(self,params)
        self._item = 0
        self._refs = {}

    def title(self):
        return '3 - ENCRYPT/SCRAMBLE' if self._params['mode'] == 'scramble' else '4 - DECRYPT/CLEAR'
    def source(self):
        return 'PLAIN' if self._params['mode'] == 'scramble' else 'SCRAMBLE'
    def dest(self):
        return 'SCRAMBLE' if self._params['mode'] == 'scramble' else 'PLAIN'
    def prefix(self):
        return 'ES' if self._params['mode'] == 'scramble' else 'DC'
    def pm(self):
        return 'plus' if self._params['mode'] == 'scramble' else 'minus'

    def li(self,ref=None):
        self._item = self._item+1
        loc = f"""{self.prefix()}{self._item}"""
        if ref != None:
            self._refs[ref]=loc
        return f"""<b>{loc}</b>"""

    def overview(self):
        words=f"""<h1>{self.title()}</h1><br/>
Start with key deck and repeat:<br/>
{self.li()} <b>{self.source()}</b> is next card in packet.<br/>
{self.li("tag")} <b>TAG</b> is 3<sup>rd</sup> card in deck minus 1.<br/>
{self.li()} <b>NOISE</b> is card in deck after <b>TAG</b> card.<br/>
{self.li()} <b>{self.dest()}</b> is <b>{self.source()}</b> {self.pm()} <b>NOISE</b>.<br/>
{self.li("cut")} <b>CUT</b> is 1<sup>st</sup> card in deck plus PLAIN.<br/>
{self.li()} Cut deck on <b>TAG</b> card (<b>{self._refs["tag"]}</b>)<br/>
{self.li()} Back-front shuffle<br/>
{self.li()} Cut deck on <b>CUT</b> card (<b>{self._refs["cut"]}</b>)<br/>
"""
        return self.typeset(words)
    
    def rotparts(self):
        return f"""
<g transform="translate(0,0)">{self.overview()}</g>
"""

class CardScramble(CardCipher):
    def __init__(self,params={}):
        CardCipher.__init__(self,params)
        self._params['mode']='scramble'

class CardClear(CardCipher):
    def __init__(self,params={}):
        CardCipher.__init__(self,params)
        self._params['mode']='clear'
