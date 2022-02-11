UPS=['A','B','C','D','E','F','G','H','I','J',
     'K','L','M','N','O','P','Q','R','S','T',
     'U','V','W','X','Y','Z','{','}','[',']',
     '_',':','!','&#96;','heartbroken','love️','','','','']

DOWNS=['0','1','2','3','4','5','6','7','8','9',
       'A','B','C','D','E','F','@','=','\\','~',
       '#','$','%','^','&amp;','|','-','+','/','*',
       '↩',';','?','&#39;','sad','smiley','','','','']

PLAINS=['a','b','c','d','e','f','g','h','i','j',
        'k','l','m','n','o','p','q','r','s','t',
        'u','v','w','x','y','z','&lt;','&gt;','(',')',
        '␣',',','.','&#34;','thumb-down','thumb-up','↓','↑','⇊','⇈']

SUITS=['club','diamond','heart','spade']

with open("card-template.svg") as f:
    svg = f.read()

def card(number):
    suit = number // 10
    face = number % 10
    up = UPS[number]
    down = DOWNS[number]
    plain = PLAINS[number]
    translate_type = "translate-text" if number != 34 and number != 35 else "translate-pics"
    tmp = svg
    tmp = tmp.replace("${suit}",SUITS[suit])
    tmp = tmp.replace("${number}",str(suit)+str(face))
    tmp = tmp.replace("${translate_type}",translate_type)
    tmp = tmp.replace("${translate_down}",down)
    tmp = tmp.replace("${translate_up}",up)
    tmp = tmp.replace("${translate}",plain)
    return tmp

for number in range(40):
    cardsvg = card(number)
    with open(str(number//10) + str(number%10) + ".svg","w") as f:
        f.write(cardsvg)


