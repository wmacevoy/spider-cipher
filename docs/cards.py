UPS=['A','B','C','D','E','F','G','H','I','J',
     'K','L','M','N','O','P','Q','R','S','T',
     'U','V','W','X','Y','Z','{','}','[',']',
     '_',':','!','`','💔','❤️','','','','']

DOWNS=['0','1','2','3','4','5','6','7','8','9',
       'A','B','C','D','E','F','@','=','\\','~',
       '#','$','%','^','&amp;','|','-','+','/','*',
       '↩',';','?','\'','😢','😄','','','','']

PLAINS=['a','b','c','d','e','f','g','h','i','j',
        'k','l','m','n','o','p','q','r','s','t',
        'u','v','w','x','y','z','&lt;','&gt;','(',')',
        '☐',',','.','\"','👎','👍','↓','↑','⇊','⇈']

SUITS=['club','diamond','heart','spade']

with open("card-template.svg") as f:
    svg = f.read()

def card(number):
    suit = number // 10
    face = number % 10
    up = UPS[number]
    down = DOWNS[number]
    plain = PLAINS[number]
    tmp = svg
    tmp = tmp.replace("${suit}",SUITS[suit])
    tmp = tmp.replace("${number}",str(suit)+str(face))
    tmp = tmp.replace("${translate_down}",down)
    tmp = tmp.replace("${translate_up}",up)
    tmp = tmp.replace("${translate}",plain)
    return tmp

for number in range(40):
    cardsvg = card(number)
    with open(str(number//10) + str(number%10) + ".svg","w") as f:
        f.write(cardsvg)


