#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include "facts.h"

#include "utf8.h"
#include "spider_solitaire.h"

const char* const UTF8_TEST_STRINGS [] =
  {
   u8"",
   u8"x",
   u8"xy",
   u8"X",
   u8"XY",
   u8"1",
   u8"12",
   u8"♣",
   u8"♣♠",
   u8"xX1♣xyXY12♣♠",
   u8"Spider Solitare",
   u8"int main(int argc, char** argv) {\n" 
   u8"  ::testing::InitGoogleTest(&argc, argv);\n" 
   u8"  return RUN_ALL_TESTS();\n" 
   u8"}\n",
   u8"Q(0)A(1)23456789♣+0a0AQ♣0/0b1BA♣1/-39c2C2♣2/-38d3D3♣3/-37e4E4♣4/-36f5F5♣5/-35g6G6♣6/-34h7H7♣7/-33i8I8♣8/-32j9J9♣9/-31♦+10kAKQ♦10/-30lBLA♦11/-29mCM2♦12/-28nDN3♦13/-27oEO4♦14/-26pFP5♦15/-25q@Q6♦16/-24r=R7♦17/-23s\\S8♦18/-22t~T9♦19/-21♥+20u#UQ♥20/-20v$VA♥21/-19w%W2♥22/-18x^X3♥23/-17y&Y4♥24/-16z|Z5♥25/-15<-{6♥26/-14>+}7♥27/-13(/[8♥28/-12)*]9♥29/-11♠+30☐↩_Q♠30/-10,;:A♠31/-9.?!2♠32/-8\"\'`3♠33/-7👍😄❤️4♠34/-6👎😢💔5♠35/-5↓6♠36/-4↑7♠37/-3⇊8♠38/-2⇈9♠39/-1",
};

const uint32_t* testString(int i) {
  const char *utf8=UTF8_TEST_STRINGS[i];
  int u32len=utf8decode(utf8, strlen(utf8), NULL, 0);
  uint32_t *u32 = (uint32_t*)malloc(sizeof(uint32_t)*(u32len+1));
  utf8decode(utf8, strlen(utf8), u32, u32len);
  u32[u32len]=0;
  return u32;
}

#define DECK_EQ(a,b) { int i; for (i=0; i<CARDS; ++i) { FACT(a[i],==,b[i]); } }
#define DECK_NE(a,b) { int i, ne=0; for (i=0; i<CARDS; ++i) { ne = ne || (a[i] != b[i]); }; FACT(ne,==,1); }


struct CIONotRandStruct;
typedef struct CIONotRandStruct CIONotRand;

struct CIONotRandStruct {
  CIO base;
  int state;
};

int CIONotRandRead(CIONotRand *me) {
  int ans=0;
  if (me->state < PREFIX) {
    ans = me->state + 1; // 1--10 for prefix */
  } else {
    ans = CARDS-1-((me->state-PREFIX) % CARDS);
  }
  ++me->state;
  return ans;
}

void CIONotRandInit(CIONotRand *me) {
  CIOInit(&me->base);
  me->base.read = (CIOReadPtr) &CIONotRandRead;
  me->state = 0;
}

FACTS(RandStats) {
  int n = 10*1000*1000;
  int counts1[CARDS];
  int counts2[CARDS][CARDS];
  CIORand rcg;
  CIORandInit(&rcg);

  for (int i=0; i<CARDS; ++i) {
    counts1[i]=0;
    for (int j=0; j<CARDS; ++j) {
      counts2[i][j]=0;
    }
  }

  int c1=CIORead(&rcg);
  for (int i=0; i<n; ++i) {
    int c0=CIORead(&rcg);
    ++counts1[c0];
    ++counts2[c0][c1];
    c1=c0;
  }
  CIOClose(&rcg);

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

FACTS(FaceAndSuiteNo) {
  for (int suiteNo = 0; suiteNo <= 3; ++suiteNo) {
    for (int faceNo = 0; faceNo <= 9; ++faceNo) {
      Card card = 10*suiteNo+faceNo;
      FACT(cardFaceNo(card),==,faceNo);
      FACT(cardSuiteNo(card),==,suiteNo);
    }
  }
}

FACTS(FaceFromNo) {
  FACT(cardFaceFromNo(0),==,'Q');
  FACT(cardFaceFromNo(1),==,'A');
  FACT(cardFaceFromNo(2),==,'2');
  FACT(cardFaceFromNo(3),==,'3');
  FACT(cardFaceFromNo(4),==,'4');
  FACT(cardFaceFromNo(5),==,'5');
  FACT(cardFaceFromNo(6),==,'6');
  FACT(cardFaceFromNo(7),==,'7');
  FACT(cardFaceFromNo(8),==,'8');
  FACT(cardFaceFromNo(9),==,'9');
}

FACTS(SuiteFromNo) {
  const uint32_t u32[]={0x2663,0x2666,0x2665,0x2660};
  const char *utf8[]={u8"♣",u8"♦",u8"♥",u8"♠"};
  for (int i=0; i<4; ++i) {
    FACT(utf8decval(utf8[i],utf8declen(utf8[i],strlen(utf8[i]))),==,u32[i]);
    FACT(cardSuiteFromNo(i),==,u32[i]);
  }
}

FACTS(CardFromFaceSuiteNo) {
  for (int suiteNo = -1; suiteNo <= 4; ++suiteNo) {
    for (int faceNo = -1; faceNo <= 10; ++faceNo) {
      int card = 10*suiteNo+faceNo;
      if (suiteNo < 0 || suiteNo >= 4) { card = -1; }
      if (faceNo < 0 || faceNo >= 10) { card = -1; }      
      FACT(cardFromFaceSuiteNo(faceNo,suiteNo),==,card);
    }
  }
}

FACTS(Add) {
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

FACTS(Subtract) {
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

FACTS(Init) {
  Deck deck;
  deckInit(deck);
  for (int i=0; i<CARDS; ++i) {
    FACT(deck[i],==,i);
  }
}

FACTS(Cut) {
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

FACTS(BackFrontShuffle) {
  Deck input,output,expect;
  deckInit(input);
  deckInit(output);
  deckInit(expect);
  testBackFrontShuffle(input,expect);
  deckBackFrontShuffle(input,output);
  DECK_EQ(output,expect);
}

FACTS(FindCard) {
  Deck deck,shuffled;
  deckInit(deck);
  deckInit(shuffled);
  testShuffle(deck,shuffled);
  for (int i=0; i<CARDS; ++i) {
    FACT(testFindCard(deck,i),==,deckFindCard(deck,i));
    FACT(testFindCard(shuffled,i),==,deckFindCard(shuffled,i));    
  }
}

FACTS(PseudoShuffle) {
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

FACTS(Pads) {
  const char *testString = "spidersolitare";
  Deck testDeck,tmp,deck;
  deckInit(testDeck);
  deckInit(tmp);
  deckInit(deck);
  
  for (int i=0; testString[i] != 0; ++i) {
    DECK_EQ(testDeck,deck);
    FACT(testCutPad(testDeck),==,deckCutPad(deck));
    FACT(testCipherPad(testDeck),==,deckCipherPad(deck));
    Card plain = testString[i]-'a';
    Card cutCard = cardAdd(plain,deckCutPad(deck));
    int cutLoc = deckFindCard(deck,cutCard);
    
    testCut(testDeck,cutLoc,tmp);
    testBackFrontShuffle(tmp,testDeck);
    deckPseudoShuffle(deck,cutLoc);
  }
}

FACTS(Ciphers) {
  const char *testString = "spidersolitare";
  Deck testDeck,tmp,deck;
  deckInit(testDeck);
  deckInit(tmp);
  deckInit(deck);

  for (int k=0; k<10; ++k) {
    for (int i=0; testString[i] != 0; ++i) {
      DECK_EQ(testDeck,deck);
      for (int j=0; j<CARDS; ++j) {
	tmp[j]=deck[j];
      }
      Card plainCard = testString[i]-'a';
      Card cipherCard = cardAdd(plainCard,testCipherPad(testDeck));
      Card cutCard = cardAdd(plainCard,testCutPad(testDeck));
      int cutLoc = testFindCard(testDeck,cutCard);
      for (int j=0; j<CARDS; ++j) {
	deck[j]=tmp[j];
      }
      FACT(testCipherPad(testDeck),==,deckCipherPad(deck));
      FACT(testCutPad(testDeck),==,deckCutPad(deck));      
      int cipher2 = deckEncryptCard(deck,plainCard);
      FACT(cipherCard,==,cipher2);
      
      for (int j=0; j<CARDS; ++j) {
	deck[j]=tmp[j];
      }
      FACT(testCipherPad(testDeck),==,deckCipherPad(deck));
      FACT(testCutPad(testDeck),==,deckCutPad(deck));      
      int plain2 = deckDecryptCard(deck,cipherCard);
      FACT(plainCard,==,plain2);

      deckPseudoShuffle(testDeck,cutLoc);
      DECK_EQ(testDeck,deck);
    }
  }
}

int wstrlen(const uint32_t *s) {  int n=0; while (*s++ != 0) ++n; return n; }

FACTS(Encode) {
  for (int i=0; i<sizeof(UTF8_TEST_STRINGS)/sizeof(char*); ++i) {
    const uint32_t *str=testString(i);
    int strLen = wstrlen(str);
    CIOArray plain;
    CIOArray encoded;
    CIOArray decoded;
    CIOArrayConstU32Init(&plain,str,0,strLen);
    CIOArrayU8Init(&encoded,NULL,0,0,0,INT_MAX);
    CIOArrayU32Init(&decoded,NULL,0,0,0,INT_MAX);    
    encodeIO(&plain.base,&encoded.base);

    int encLen = CIOGetWrites(&encoded);
    encoded.position=0;
    decodeIO(&encoded.base,&decoded.base);
    int decLen = CIOGetWrites(&decoded);
    decoded.position=0;
    int maxLen = strLen > decLen ? strLen : decLen;
    for (int j=0; j<=maxLen; ++j) {
      FACT(j < strLen ? str[j] : -1,==,CIOPeek(&decoded,j));
    }
    CIOClose(&plain);
    CIOClose(&encoded);
    CIOClose(&decoded);
    free((void*)str);
  }
}

FACTS(Envelope) {
  for (int i=0; i<sizeof(UTF8_TEST_STRINGS)/sizeof(char*); ++i) {
    const uint32_t *str=testString(i);    
    int strLen = wstrlen(str);

    CIOArray plain;
    CIOArrayConstU32Init(&plain,str,0,strLen);
    CIONotRand nrcg;
    CIONotRandInit(&nrcg);
    Deck deck;
    deckInit(deck);

    CIOArray envelope;
    CIOArrayU8Init(&envelope,NULL,0,0,0,INT_MAX);
    encryptEnvelopeIO(deck,&plain.base,&nrcg.base,&envelope.base);
    int envLen = CIOGetWrites(&envelope);

    // reset envelope for decryption
    envelope.position = 0;
    
    CIOArray decrypted;
    CIOArrayU32Init(&decrypted,NULL,0,0,0,INT_MAX);
    deckInit(deck);
    decryptEnvelopeIO(deck,&envelope.base,NULL,&decrypted.base);
    int decLen = CIOGetWrites(&decrypted);
    decrypted.position = 0;

    int maxLen = strLen > decLen ? strLen : decLen;
    for (int j=0; j<=maxLen; ++j) {
      FACT(j < strLen ? str[j] : -1,==,CIOPeek(&decrypted,j));
    }

    CIOClose(&decrypted);
    CIOClose(&envelope);
    CIOClose(&plain);
    CIOClose(&nrcg);

    free((void*)str);
  }
}

FACTS(BackFrontUnshuffle) {
  Deck deck;
  Deck tmp;
  Deck id;
  deckInit(deck);
  deckInit(tmp);
  deckInit(id);
  deckBackFrontShuffle(deck,tmp);
  for (int i=0; i<CARDS; ++i) deck[i]=0;
  deckBackFrontUnshuffle(tmp,deck);
  DECK_EQ(deck,id);
}

// pseudo-shuffle on cut location c
void P(Deck deck,int c) {
  Deck tmp;
  deckCut(deck,c,tmp);
  deckBackFrontShuffle(tmp,deck);
}

FACTS(P) {
  for (int c=0; c<CARDS; ++c) {
    Deck a;
    Deck b;
    deckInit(a);
    deckInit(b);
    deckPseudoShuffle(a,c);
    P(b,c);
    DECK_EQ(a,b);
  }
}

FACTS(PReachable) {
  for (int c=0; c<CARDS; ++c) {
    Deck p;
    deckInit(p);
    P(p,c);
    
    Deck reach;
    deckInit(reach);
    deckPseudoShuffle(reach,c);

    DECK_EQ(reach,p);
  }
}

void I(Deck deck) {
  //id;
}

FACTS(I) {
  Deck i;
  deckInit(i);
  I(i);
  for (int k=0; k<CARDS; ++k) i[k] == k;
}

FACTS(IReachable) {
  Deck id;
  deckInit(id);
  I(id);

  Deck reach;
  deckInit(reach);
  for (int k=0; k<9; ++k) {
    P(reach,2);
  }

  DECK_EQ(reach,id);
}

// inverse pseudo-shuffle on cut location c
void Q(Deck deck,int c) {
  Deck tmp;
  deckBackFrontUnshuffle(deck,tmp);
  deckCut(tmp,CARDS-c,deck);
}

FACTS(Q) {
  for (int c=0; c<CARDS; ++c) {
    Deck deck;
    Deck id;
    deckInit(deck);
    deckInit(id);
    P(deck,c);
    Q(deck,c);
    DECK_EQ(deck,id);
  }
}

FACTS(QReachable) {
  for (int c=0; c<CARDS; ++c) {
    Deck q;
    deckInit(q);
    Q(q,c);
	  
    Deck reach;
    deckInit(reach);

    for (int i=0; i<17; ++i) {
      deckPseudoShuffle(reach,(i == 8) ? (4+(CARDS-c))%CARDS : 2);
    }
    DECK_EQ(reach,q);
  }
}

void T(Deck deck, int c) {
  Deck tmp;
  deckCut(deck,c,tmp);
  for (int i=0; i<CARDS; ++i) {
    deck[i]=tmp[i];
  }
}

FACTS(T) {
  for (int c=0; c<CARDS; ++c) {
    Deck t;
    deckInit(t);
    T(t,c);

    for (int i=0; i<CARDS; ++i) {
      t[i]=(i+c) % CARDS;
    }
  }
}

FACTS(TReachable) {
  for (int c=0; c<CARDS; ++c) {
    Deck t;
    deckInit(t);
    T(t,c);

    Deck reach;
    deckInit(reach);
    for (int i=0; i<9; ++i) {
      deckPseudoShuffle(reach,(i == 0) ? (2+c)%CARDS : 2);
    }
    
    DECK_EQ(reach,t);
  }
}

// reverse deck
void R(Deck deck) {
  Deck tmp;
  for (int i=0; i<CARDS; ++i) {
    tmp[i]=deck[(CARDS-1)-i];
  }
  for (int i=0; i<CARDS; ++i) {
    deck[i]=tmp[i];
  }
}

FACTS(R) {
  Deck r;
  deckInit(r);
  R(r);
  for (int i=0; i<CARDS; ++i) {
    FACT(r[i],==,CARDS-1 - i);
  }
}


FACTS(RReachable) {
  Deck r;
  deckInit(r);
  R(r);

  Deck reach;
  deckInit(reach);
  P(reach,0);
  T(reach,20);
  Q(reach,0);

  DECK_EQ(reach,r);
}

void X(Deck deck) {
  Deck tmp;
  for (int i=0; i<CARDS; i += 2) {
    int j=i+1;
    tmp[i]=deck[j];
    tmp[j]=deck[i];
  }
  for (int i=0; i<CARDS; ++i) {
    deck[i]=tmp[i];
  }
}

FACTS(X) {
  Deck x;
  deckInit(x);
  X(x);
  for (int i=0; i<CARDS; ++i) {
    FACT(x[i],==,(i % 2 == 0) ? i+1 : i-1);
  }
}

FACTS(XReachable) {
  Deck x;
  deckInit(x);
  X(x);

  Deck reach;
  deckInit(reach);

  P(reach,0);
  R(reach);
  Q(reach,0);

  DECK_EQ(reach,x);
}

void B(Deck deck, int i) {
  int j = (i+1)%CARDS;
  int tmp = deck[i];
  deck[i]=deck[j];
  deck[j]=tmp;
}

FACTS(B) {
  for (int i=0; i<CARDS; ++i) {
    Deck b;
    deckInit(b);
    B(b,i);
    for (int k=0; k<CARDS; ++k) {
      int j=(i+1) % CARDS;
      if (k == i) FACT(b[k],==,j);
      if (k == j) FACT(b[k],==,i);
      if (k != i && k != j) FACT(b[k],==,k);
    }
  }
}

FACTS(BReachable) {
  for (int i=0; i<CARDS; ++i) {
    Deck b;
    deckInit(b);
    B(b,i);

    Deck reach;
    deckInit(reach);

    T(reach,(i+1)%CARDS);
    P(reach,0);
    T(reach,21);
    Q(reach,0);
    R(reach);
    X(reach);
    T(reach,1);
    X(reach);
    T(reach,CARDS-1);
    T(reach,(2*CARDS-(i+1)) % CARDS);
    
    DECK_EQ(reach,b);
  }
}

void S(Deck deck, int i, int j) {
  int tmp = deck[i];
  deck[i]=deck[j];
  deck[j]=tmp;
}

FACTS(S) {
  for (int i=0; i<CARDS; ++i) {
    for (int j=0; j<CARDS; ++j) {
      Deck s;
      deckInit(s);
      S(s,i,j);
      
      for (int k=0; k<CARDS; ++k) {
	if (k == i) FACT(s[k],==,j);
	if (k == j) FACT(s[k],==,i);
	if (k != i && k != j) FACT(s[k],==,k);
      }
    }
  }
}

FACTS(SReachable) {
  for (int i=0; i<CARDS; ++i) {
    for (int j=0; j<CARDS; ++j) {
      Deck s;
      deckInit(s);
      S(s,i,j);
      
      Deck reach;
      deckInit(reach);

      if (i != j) {
	int a = (i < j) ? i : j;
	int b = (i > j) ? i : j;

	T(reach,a);	
	for (int k=0; k<b-a; ++k) {
	  B(reach,k);
	}
	for (int k=b-a-2; k>=0; --k) {
	  B(reach,k);
	}
	T(reach,(CARDS-a));
      }
      DECK_EQ(reach,s);
    }
  }
}

static int CYCLE_LENGTHS [] =
  {
   27,  30,   9, 110,  12,  90,  99, 234, 105,  12,
   60, 126, 115,  56,  20, 174,  12,  66, 105,  20,
   39,  40,  39,  60,  72, 150,  60,  40,  39, 264,
   36, 380,  75,  20,  60,  40, 182, 190, 440,  24
  };

FACTS(Cycles) {
  for (int c = 0; c < CARDS; ++c) {
    Deck deck;
    Deck id;
    deckInit(deck);
    deckInit(id);
    int n=CYCLE_LENGTHS[c];
    for (int i=0; i<n; ++i) {
      if (i != 0) { DECK_NE(deck,id); }
      else { DECK_EQ(deck,id); }
      P(deck,c);
      if (i != n-1) { DECK_NE(deck,id); }
      else { DECK_EQ(deck,id); }
    }
  }
}

FACTS(InverseCycles) {
  for (int c = 0; c < CARDS; ++c) {
    Deck deck;
    Deck id;
    deckInit(deck);
    deckInit(id);
    int n=CYCLE_LENGTHS[c];
    for (int i=0; i<n; ++i) {
      if (i != 0) { DECK_NE(deck,id); }
      else { DECK_EQ(deck,id); }
      Q(deck,c);
      if (i != n-1) { DECK_NE(deck,id); }
      else { DECK_EQ(deck,id); }
    }
  }
}


FACTS_FINISHED
FACTS_MAIN
