UPS=[('A','ALPHA'),  ('B','BRAVO'), ('C','CHARLIE'),('D','DELTA'), ('E','ECHO'),('F','FOXTROT'),('G','GOLF'),('H','HOTEL'),('I','INDIA'),('J','JULIETT'),
     ('K','KILO'),    ('L','LIMA'),  ('M','MIKE'),   ('N','NOVEMBER'), ('O','OSCAR'), ('P','PAPA'), ('Q','QUEBEC'),('R','ROMEO'),('S','SIERRA'),('T','TANGO'),
     ('U','UNIFORM'),('V','VICTOR'),('W','WHISKEY'),('X','XRAY'),('Y','YANKEE'),('Z','ZULU'),('{','{curly}'),('}','curly'),('[','[bracket]'),(']','[bracket]'),
     ('_','under'),  (':','colon'), ('!','bank'),   ('&#96;','&#96;back&#96;'),('heart-broken',''),('heart-full',''),('',''),('',''),('',''),('','')]


DOWNS=[('0','zero'),('1','one'),('2','two'),('3','three'),('4','four'),('5','five'),('6','six'),('7','seven'),('8','eight'),('9','nine'),
       ('A','ALPHA'),('B','BRAVO'),('C','CHARLIE'),('D','DELTA'),('E','ECHO'),('F','FOXTROT'),('@','at'),('=','equal'),('\\','backslash'),('~','tilde'),
       ('#','pound'),('$','dollar'),('%','percent'),('^','carat'),('&amp;','and'),('|','pipe'),('-','dash'),('+','plus'),('/','slash'),('*','astrisk'),
       ('↩','newline'),(';','semicolon'),('?','question'),('&#39;','&#39;single&#39;'),('sad',''),('happy',''),('',''),('',''),('',''),('','')]

PLAINS=[('a','alpha'),('b','bravo'),('c','charlie'),('d','delta'),('e','echo'),('f','foxtrot'),('g','golf'),('h','hotel'),('i','india'),('j','juliett'),
        ('k','kilo'),('l','lima'),('m','mike'),('n','november'),('o','oscar'),('p','papa'),('q','quebec'),('r','romeo'),('s','sierra'),('t','tango'),
        ('u','uniform'),('v','victor'),('w','whiskey'),('x','xray'),('y','yankee'),('z','zulu'),('&lt;','less'),('&gt;','more'),('(','(paren)'),(')','(paren)'),
        ('␣','space'),(',','comma'),('.','period'),('&#34;','&#34;double&#34;'),('thumb-down',''),('thumb-up',''),('↓',''),('↑',''),('⇊',''),('⇈','')]

SUITS=['suit-club','suit-diamond','suit-heart','suit-spade']

with open("card-template.svg") as f:
    svg = f.read()

def card(number):
    suit = number // 10
    face = number % 10
    up = UPS[number][0]
    hint_up = UPS[number][1]
    down = DOWNS[number][0]
    hint_down = DOWNS[number][1]
    plain = PLAINS[number][0]
    hint_plain = PLAINS[number][1]
    translate_type = "translate-text" if number != 34 and number != 35 else "translate-pics"
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
    return tmp

for number in range(40):
    cardsvg = card(number)
    with open(str(number//10) + str(number%10) + ".svg","w") as f:
        f.write(cardsvg)


