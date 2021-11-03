#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "facts.h"

#include "spider_solitaire.h"

const wchar_t* TEST_STRINGS [] =
  {
   L"",
   L"x",
   L"xy",
   L"X",
   L"XY",
   L"1",
   L"12",
   L"♣",
   L"♣♠",
   L"xX1♣xyXY12♣♠",
   L"Spider Solitare",
   L"int main(int argc, char** argv) {\n" 
   L"  ::testing::InitGoogleTest(&argc, argv);\n" 
   L"  return RUN_ALL_TESTS();\n" 
   L"}\n",
   L"Q(0)A(1)23456789♣+0a0AQ♣0/0b1BA♣1/-39c2C2♣2/-38d3D3♣3/-37e4E4♣4/-36f5F5♣5/-35g6G6♣6/-34h7H7♣7/-33i8I8♣8/-32j9J9♣9/-31♦+10kAKQ♦10/-30lBLA♦11/-29mCM2♦12/-28nDN3♦13/-27oEO4♦14/-26pFP5♦15/-25q@Q6♦16/-24r=R7♦17/-23s\\S8♦18/-22t~T9♦19/-21♥+20u#UQ♥20/-20v$VA♥21/-19w%W2♥22/-18x^X3♥23/-17y&Y4♥24/-16z|Z5♥25/-15<-{6♥26/-14>+}7♥27/-13(/[8♥28/-12)*]9♥29/-11♠+30☐↩_Q♠30/-10,;:A♠31/-9.?!2♠32/-8\"\'`3♠33/-7👍😄❤️4♠34/-6👎😢💔5♠35/-5↓6♠36/-4↑7♠37/-3⇊8♠38/-2⇈9♠39/-1",
};


#define DECK_EQ(a,b) { int i; for (i=0; i<CARDS; ++i) { FACT(a[i],==,b[i]); } }

struct CardNotRandIOStruct;
typedef struct CardNotRandIOStruct
CardNotRandIO;

struct CardNotRandIOStruct {
  CardIO base;
  int state;
};

int CardNotRandIORead(CardIO *me) {
  CardNotRandIO *my=(CardNotRandIO *)me;
  int ans=0;
  if (my->state < PREFIX) {
    ans = my->state + 1; // 1--10 for prefix */
  } else {
    ans = CARDS-1-((my->state-PREFIX) % CARDS);
  }
  ++my->state;
  return ans;
}

void CardNotRandIOClose(CardIO *me) {}

void CardNotRandIOInit(CardNotRandIO *me) {
  me->base.read = &CardNotRandIORead;
  me->base.write = NULL;
  me->base.peek = NULL;
  me->base.close = &CardNotRandIOClose;
  me->state = 0;
}

FACTS(RandStats) {
  int n = 10*1000*1000;
  int counts1[CARDS];
  int counts2[CARDS][CARDS];
  CardRandIO rcg;
  CardRandIOInit(&rcg);

  for (int i=0; i<CARDS; ++i) {
    counts1[i]=0;
    for (int j=0; j<CARDS; ++j) {
      counts2[i][j]=0;
    }
  }

  int c1=rcg.base.read((CardIO*)&rcg);
  for (int i=0; i<n; ++i) {
    int c0=rcg.base.read((CardIO*)&rcg);
    ++counts1[c0];
    ++counts2[c0][c1];
    c1=c0;
  }

  double p1 = 1.0/CARDS;
  double q1 = 1-p1;
  double mu1 = n*p1;
  double inv_sigma1 = 1/sqrt(mu1);
  double z1=0.0;

  for (int i=0; i<CARDS; ++i) {
    double zi=inv_sigma1*(counts1[i]-mu1);
    FACT(fabs(zi),<=,6.0);
    z1 += pow(zi,2);
  }

  z1 = sqrt(2.0)*(sqrt(z1)-sqrt(CARDS-1.5));

  double p2 = pow(1.0/CARDS,2);
  double q2 = 1-p2;
  double mu2 = n*p2;
  double inv_sigma2 = 1/sqrt(mu2);
  double z2=0.0;

  for (int i=0; i<CARDS; ++i) {
    for (int j=0; j<CARDS; ++j) {
      double zij=inv_sigma2*(counts2[i][j]-mu2);
      FACT(fabs(zij),<=,6.0);
      z2 += pow(zij,2);
    }
  }

  z2 = sqrt(2.0)*(sqrt(z2)-sqrt(pow(CARDS,2)-1.5));

  FACT(fabs(z1),<=,6.0);
  FACT(fabs(z2),<=,6.0);  
}


void testBackFrontShuffle(const Deck in, Deck out)
{
  int n=0;
  for (int i=0; i<CARDS; ++i) {
    int j=(i%2 == 0) ? n : 0;
    for (int k=n; k>j; --k) {
      out[k]=out[k-1];
    }
    out[j]=in[i];
    ++n;
  }
}

void testShuffle(const Deck in, Deck out) {
  for (int i=0; i<CARDS; ++i) {
    out[i]=in[i];
  }
  for (int i=0; i<CARDS; ++i) {
    int n=(CARDS-i);
    int r = (17*n+3) % n;
    int j = i + r;
    int tmp=out[i];
    out[i]=out[j];
    out[j]=tmp;
  }
}

void testCut(const Deck in, int cutPos, Deck out) {
  for (int i=0; i<CARDS; ++i) {
    out[i]=in[(i+cutPos) % CARDS];
  }
}

int testFindCard(const Deck deck, Card card) {
  for (int i=0; i<CARDS; ++i) {
    if (deck[i] == card) {
      return i;
    }
  }
  return -1;
}

Card testCutPad(const Deck deck) {
  return deck[CUT_ZTH];
}

Card testCipherPad(const Deck deck) {
  int markCard = (deck[MARK_ZTH]+MARK_ADD) % CARDS;
  int markLoc = testFindCard(deck, markCard);
  return deck[(markLoc + 1) % CARDS];
}

FACTS(SpiderFaceAndSuiteNo) {
  for (int suiteNo = 0; suiteNo <= 3; ++suiteNo) {
    for (int faceNo = 0; faceNo <= 9; ++faceNo) {
      Card card = 10*suiteNo+faceNo;
      FACT(cardFaceNo(card),==,faceNo);
      FACT(cardSuiteNo(card),==,suiteNo);
    }
  }
}

FACTS(SpiderFaceFromNo) {
  FACT(cardFaceFromNo(0),==,L'Q');
  FACT(cardFaceFromNo(1),==,L'A');
  FACT(cardFaceFromNo(2),==,L'2');
  FACT(cardFaceFromNo(3),==,L'3');
  FACT(cardFaceFromNo(4),==,L'4');
  FACT(cardFaceFromNo(5),==,L'5');
  FACT(cardFaceFromNo(6),==,L'6');
  FACT(cardFaceFromNo(7),==,L'7');
  FACT(cardFaceFromNo(8),==,L'8');
  FACT(cardFaceFromNo(9),==,L'9');
}

FACTS(SpiderSuiteFromNo) {
  FACT(cardSuiteFromNo(0),==,0x2663); // Unicode club
  FACT(cardSuiteFromNo(1),==,0x2665); // Unicdoe diamond
  FACT(cardSuiteFromNo(2),==,0x2667); // Unicode heart
  FACT(cardSuiteFromNo(3),==,0x2669); // Unicode spade
}

FACTS(SpiderCardFromFaceSuiteNo) {
  for (int suiteNo = -1; suiteNo <= 4; ++suiteNo) {
    for (int faceNo = -1; faceNo <= 10; ++faceNo) {
      int card = 10*suiteNo+faceNo;
      if (suiteNo < 0 || suiteNo >= 4) { card = -1; }
      if (faceNo < 0 || faceNo >= 10) { card = -1; }      
      FACT(cardFromFaceSuiteNo(faceNo,suiteNo),==,card);
    }
  }
}

FACTS(SpiderAdd) {
  for (int x = 0; x <= CARDS; ++x) {
    for (int y = 0; y <= CARDS; ++y) {
      int z = x + y;
      while (z < 0) z += CARDS;
      while (z >= CARDS) z -= CARDS;
      int result =cardAdd(x,y);
      FACT(z,==,result);
    }
  }
}

FACTS(SpiderSubtract) {
  for (int x = 0; x <= CARDS; ++x) {
    for (int y = 0; y <= CARDS; ++y) {
      int z = x - y;
      while (z < 0) z += CARDS;
      while (z >= CARDS) z -= CARDS;
      int result = cardSubtract(x,y);
      FACT(z,==,result);
    }
  }
}

FACTS(SpiderInit) {
  Deck deck;
  deckInit(deck);
  for (int i=0; i<CARDS; ++i) {
    FACT(deck[i],==,i);
  }
}

FACTS(SpiderCut) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,expect;
    deckInit(input);
    deckInit(output);
    deckInit(expect);
    testCut(input,cutLoc,expect);
    deckCut(input,cutLoc,output);
    DECK_EQ(output,expect);
  }
}

FACTS(SpiderBackFrontShuffle) {
  Deck input,output,expect;
  deckInit(input);
  deckInit(output);
  deckInit(expect);
  testBackFrontShuffle(input,expect);
  deckBackFrontShuffle(input,output);
  DECK_EQ(output,expect);
}

FACTS(SpiderFindCard) {
  Deck deck,shuffled;
  deckInit(deck);
  deckInit(shuffled);
  testShuffle(deck,shuffled);
  for (int i=0; i<CARDS; ++i) {
    FACT(testFindCard(deck,i),==,deckFindCard(deck,i));
    FACT(testFindCard(shuffled,i),==,deckFindCard(shuffled,i));    
  }
}

FACTS(SpiderPseudoShuffle) {
  for (int cutLoc = 0; cutLoc < CARDS; ++cutLoc) {
    Deck input,output,tmp,expect;
    deckInit(input);
    deckInit(output);
    deckInit(tmp);
    deckInit(expect);
    testCut(input,cutLoc,tmp);
    testBackFrontShuffle(tmp,expect);
    deckPseudoShuffle(output,cutLoc);

    DECK_EQ(output,expect);
  }
}

FACTS_FINISHED

FACTS_MAIN
