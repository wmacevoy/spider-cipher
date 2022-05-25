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

    def suit(self): return str(self._suit)


    
    def defs(self):
        return f"""
<defs>
{self.suit()}
{self.glyphs()}
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
<use xlink:href="#{self.cardId()}-glyphs" />
<g transform="rotate(180,31.5,44)">
  <use xlink:href="#{self.cardId()}-glyphs" />
</g>
</g><!-- {self.cardId()}-fg -->
"""

    def suit(self): return str(self._suit)
    def glyphs(self):
        ans = f'<g id="{self.cardId()}-glyphs" >'
        for glyph in self._glyphs:
            ans = ans + str(self._glyphs[glyph])
        ans = ans + f'</g><!-- {self.cardId()}-glyphs -->'
        return ans
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
