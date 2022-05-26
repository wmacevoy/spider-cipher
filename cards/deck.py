#!/usr/bin/env python3

from svg import SVG
from suit import Suit
from glyph import Glyph

class Card(SVG):
    @staticmethod
    def build(params):
        return Card(params)
    
    def __init__(self,params={}):
        SVG.__init__(self,params)
        self._params['tag']='g'
        self._params['@id']=self.cardId()
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
<use xlink:href="#{self.cardId()}-morse" transform="translate(8,0) rotate(90)" />
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
    def __str__(self):
        return f"""<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<svg width="69mm" height="94mm" viewBox="-3 -3 69 94" version="1.1" xmlns:xlink="http://www.w3.org/1999/xlink" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg"><!-- {self.cardId()} -->
{self.defs()}
{SVG.__str__(self)}
</svg>
"""

for number in range(40):
    card = Card.build({'number':number})
    with open(card.cardId() + ".svg","w") as f:
        f.write(str(card))
