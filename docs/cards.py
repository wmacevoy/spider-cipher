#!/usr/bin/env python3

UPS=[('A','ALFA'),  ('B','BRAVO'), ('C','CHARLIE'),('D','DELTA'), ('E','ECHO'),('F','FOXTROT'),('G','GOLF'),('H','HOTEL'),('I','INDIA'),('J','JULIETT'),
     ('K','KILO'),    ('L','LIMA'),  ('M','MIKE'),   ('N','NOVEMBER'), ('O','OSCAR'), ('P','PAPA'), ('Q','QUEBEC'),('R','ROMEO'),('S','SIERRA'),('T','TANGO'),
     ('U','UNIFORM'),('V','VICTOR'),('W','WHISKEY'),('X','XRAY'),('Y','YANKEE'),('Z','ZULU'),('{','{curly'),('}','curly}'),('[','[bracket'),(']','bracket]'),
     ('_','under_bar'),  (':','colon:'), ('!','bang!'),   ('&#96;','&#96;back&#96;'),('heart-broken',''),('heart-full',''),('',''),('',''),('',''),('','')]

DOWNS=[('0','zero'),('1','one'),('2','two'),('3','three'),('4','four'),('5','five'),('6','six'),('7','seven'),('8','eight'),('9','nine'),
       ('A','ALFA'),('B','BRAVO'),('C','CHARLIE'),('D','DELTA'),('E','ECHO'),('F','FOXTROT'),('@','at'),('=','equal'),('\\','backslash'),('~','tilde'),
       ('#','pound'),('$','dollar'),('%','percent'),('^','carat'),('&amp;','and'),('|','pipe'),('-','dash'),('+','plus'),('/','slash'),('*','astrisk*'),
       ('↩',"<tspan dy=\"-4\" x=\"0\">end↩</tspan><tspan x=\"0\" dy=\"3\">line↩</tspan>"),(';','semi;'),('?','ask?'),('&#39;','&#39;single&#39;'),('sad',''),('happy',''),('',''),('',''),('',''),('','')]

PLAINS=[('a','alfa'),('b','bravo'),('c','charlie'),('d','delta'),('e','echo'),('f','foxtrot'),('g','golf'),('h','hotel'),('i','india'),('j','juliett'),
        ('k','kilo'),('l','lima'),('m','mike'),('n','november'),('o','oscar'),('p','papa'),('q','quebec'),('r','romeo'),('s','sierra'),('t','tango'),
        ('u','uniform'),('v','victor'),('w','whiskey'),('x','xray'),('y','yankee'),('z','zulu'),('&lt;','less'),('&gt;','more'),('(','(paren'),(')','paren)'),
        ('␣','s p a c e'),(',','comma,'),('.','period.'),('&#34;','&#34;double&#34;'),('thumb-down',''),('thumb-up',''),
        ('',"<tspan dy=\"-7\" x=\"0\">- Shift -</tspan><tspan x=\"0\" dy=\"3.5\">once</tspan>"),
        ('',"<tspan dy=\"-7\" x=\"0\">+ Shift +</tspan><tspan x=\"0\" dy=\"3.5\">once</tspan>"),
        ('',"<tspan dy=\"-7\" x=\"0\">- SHIFT -</tspan><tspan x=\"0\" dy=\"3.5\">LOCK</tspan>"),
        ('',"<tspan dy=\"-7\" x=\"0\">+ SHIFT +</tspan><tspan x=\"0\" dy=\"3.5\">LOCK</tspan>")]

SUITS=['suit-club','suit-diamond','suit-heart','suit-spade']
ARROWS=['↓','↑','⇟','⇞']

with open("card-template.svg") as f:
    svg = f.read()

def spans(str):
    lines = str.split("\n")
    if len(lines) == 1:
        return lines[0]
    else:
        return "<tspan>"+("</tspan><tspan>".join(str.split("\n"))) + "</tspan>"
    
def card(number):
    suit = number // 10
    face = number % 10
    up = UPS[number][0]
    hint_up = spans(UPS[number][1])
    down = DOWNS[number][0]
    hint_down = spans(DOWNS[number][1])
    plain = PLAINS[number][0]
    hint_plain = spans(PLAINS[number][1])
    translate_type = "translate-text" if number != 34 and number != 35 else "translate-pics"
    formats="formats" if number < 36 else "formats-dark"

    arrow="" if number < 36 else ARROWS[number-36]

    tmp = svg
    tmp = tmp.replace("${suit}",SUITS[suit])
    tmp = tmp.replace("${number}",str(suit)+str(face))
    tmp = tmp.replace("${translate_type}",translate_type)
    tmp = tmp.replace("${translate_down}",down)
    tmp = tmp.replace("${translate_up}",up)
    tmp = tmp.replace("${translate_plain}",plain)
    tmp = tmp.replace("${hint_down}",hint_down)
    tmp = tmp.replace("${hint_up}",hint_up)
    tmp = tmp.replace("${hint_plain}",hint_plain)
    tmp = tmp.replace("${formats}",formats)
    tmp = tmp.replace("${arrow}",arrow)    
    return tmp

for number in range(40):
    cardsvg = card(number)
    with open(str(number//10) + str(number%10) + ".svg","w") as f:
        f.write(cardsvg)
