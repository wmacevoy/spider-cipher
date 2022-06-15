CARDS = 40
RULES = 7

def number(params):
    return int(params['number'] if 'number' in params else 0)

def faceNo(params):
    return number(params) % 10

def suitNo(params):
    return number(params) // 10

SUITS=['club','diamond','heart','spade']
def suit(params):
    return SUITS[suitNo(params)]

COLORS=['#208020','#202080','#802020','#202020']
def color(params):
    return COLORS[suitNo(params)]


def cardId(params):
    return f"card-{suitNo(params)}{faceNo(params)}"
