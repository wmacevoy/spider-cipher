#!/usr/bin/env python3

import info
from card import Card
    
for number in range(info.CARDS+info.RULES):
    print(f"build card {number}")
    params={'number':number}
    card = Card.build(params)

    with open(card.cardId() + ".svg","w") as fd:
        fd.write(card.svg())
