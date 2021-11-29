#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <arpa/inet.h>

#include "facts.h"
#include "spider_solitaire.h"

uint8_t PERMS1[1][1]=
  {
   {0}
  };


uint8_t PERMS2[2][2]=
  {
   {0,1},
   {1,0}
  };

uint8_t PERMS3[6][3]=
  {
   {0,1,2},{0,2,1},
   {1,0,2},{1,2,0},
   {2,0,1},{2,1,0}
  };

uint8_t PERMS4[24][4]=
  {
   {0,1,2,3},{0,1,3,2},{0,2,1,3},{0,2,3,1},{0,3,1,2},{0,3,2,1},
   {1,0,2,3},{1,0,3,2},{1,2,0,3},{1,2,3,0},{1,3,0,2},{1,3,2,0},
   {2,0,1,3},{2,0,3,1},{2,1,0,3},{2,1,3,0},{2,3,0,1},{2,3,1,0},
   {3,0,1,2},{3,0,2,1},{3,1,0,2},{3,1,2,0},{3,2,0,1},{3,2,1,0}
  };

// A set of (1*2*6*24)^4 distinct permutations
// to test the big deck set with.
// They are otherwise not important.

void Z(Deck deck, int64_t i) {
  Deck tmp;
  for (int j=0; j<4; ++j) {
    int i1 = 0;
    int i2 = i % 2;
    i = i/2;
    int i3 = i % 6;
    i = i/6;
    int i4 = i % 24;
    i = i/24;
    
    tmp[10*j+0+0]=deck[10*j+0+PERMS1[i1][0]];
    tmp[10*j+1+0]=deck[10*j+1+PERMS2[i2][0]];
    tmp[10*j+1+1]=deck[10*j+1+PERMS2[i2][1]];
    tmp[10*j+3+0]=deck[10*j+3+PERMS3[i3][0]];
    tmp[10*j+3+1]=deck[10*j+3+PERMS3[i3][1]];
    tmp[10*j+3+2]=deck[10*j+3+PERMS3[i3][2]];  
    tmp[10*j+6+0]=deck[10*j+6+PERMS4[i4][0]];
    tmp[10*j+6+1]=deck[10*j+6+PERMS4[i4][1]];
    tmp[10*j+6+2]=deck[10*j+6+PERMS4[i4][2]];
    tmp[10*j+6+3]=deck[10*j+6+PERMS4[i4][3]];
  }

  for (int k=0; k<CARDS; ++k) {
    deck[k]=tmp[k];
  }
}

FACTS(Z) {
  for (int64_t j0=0; j0<4; ++j0) {
    for (int64_t j1=0; j1<4; ++j1) {
      for (int64_t i0=0; i0<2*6*24; ++i0) {
	for (int64_t i1=0; i1<2*6*24; ++i1) {
	  int64_t k0 = i0+pow(2*6*24,j0);
	  int64_t k1 = i1+pow(2*6*24,j1);
	  Deck d0,d1;
	  deckInit(d0);
	  deckInit(d1);	    
	  Z(d0,k0);
	  Z(d1,k1);
	  FACT(memcmp(d0,d1,CARDS)!=0,==,k0!=k1);
	    
	  int b0[40],b1[40];
	  for (int i=0; i<CARDS; ++i) {
	    b0[i]=0;
	    b1[i]=0;
	  }
	  for (int i=0; i<CARDS; ++i) {
	    FACT(0,<=,d0[i]);
	    FACT(d0[i],<,CARDS);
	    FACT(0,<=,d1[i]);
	    FACT(d1[i],<,CARDS);
	    ++b0[d0[i]];
	    ++b1[d1[i]];
	  }
	  for (int i=0; i<CARDS; ++i) {
	    FACT(b0[i],==,1);
	    FACT(b1[i],==,1);
	  }
	}
      }
    }
  }  
}

typedef struct {
  int pbins;
  uint32_t nbins;
  uint32_t *counts;
  uint32_t *offsets;
  Card *cards;
  FILE *file;
} DeckSet;

void DeckSetInit(DeckSet *me, int pbins, FILE *file) {
  me->pbins = pbins;
  me->nbins = 1;
  for (int i=0; i<pbins; ++i) {
    me->nbins  *= CARDS;
  }

  me->counts = (uint32_t*)calloc(sizeof(uint32_t),me->nbins);
  me->offsets = (uint32_t*)calloc(sizeof(uint32_t),me->nbins);
  me->cards = NULL;
  me->file = file;
}

void DeckSetCount(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<me->pbins; ++i) {
    k=CARDS*k+deck[i];
  }
  ++me->counts[k];
}

void DeckSetCounted(DeckSet *me) {
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i] = ((i > 0) ? me->offsets[i-1] : 0) + me->counts[i];
  }
  if (me->file == NULL) {
    me->cards = (Card*) calloc(me->offsets[me->nbins-1],CARDS);
    assert(me->cards != NULL);
  }
  memset(me->counts,0,me->nbins*sizeof(uint32_t));  
}

void DeckSetSave(DeckSet *me) {
  if (me->file == NULL) return;
  
  int seekOk = fseek(me->file,0L,SEEK_SET);
  assert(seekOk==0);
  for (int i=0; i<me->nbins; ++i) {
    me->counts[i]=htonl(me->counts[i]);
  }
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i]=htonl(me->offsets[i]);
  }
  int writeOk = fwrite(me->counts,sizeof(uint32_t),me->nbins,me->file);
  assert(writeOk==me->nbins);
  writeOk = fwrite(me->offsets,sizeof(uint32_t),me->nbins,me->file);
  assert(writeOk==me->nbins);
  for (int i=0; i<me->nbins; ++i) {
    me->counts[i]=ntohl(me->counts[i]);
  }
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i]=ntohl(me->offsets[i]);
  }
}

void DeckSetLoad(DeckSet *me) {
  if (me->file == NULL) return;
  int seekOk = fseek(me->file,0L,SEEK_SET);
  assert(seekOk==0);
  int readOk = fread(me->counts,sizeof(uint32_t),me->nbins,me->file);
  assert(readOk==me->nbins);
  readOk = fread(me->offsets,sizeof(uint32_t),me->nbins,me->file);
  assert(readOk==me->nbins);

  for (int i=0; i<me->nbins; ++i) {
    me->counts[i]=ntohl(me->counts[i]);
  }
  for (int i=0; i<me->nbins; ++i) {
    me->offsets[i]=ntohl(me->offsets[i]);
  }
}

void DeckSetAdd(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<me->pbins; ++i) {
    k=CARDS*k+deck[i];
  }
  uint64_t offset = ((k > 0) ? me->offsets[k-1] : 0)+me->counts[k];
  if (me->file == NULL) {
    memcpy(me->cards+CARDS*offset,deck,CARDS);
  } else {
    offset = CARDS*offset + me->nbins*sizeof(uint32_t);
    int seekOk = fseek(me->file,offset,SEEK_SET);
    assert(seekOk==0);
    int writeOk = fwrite(deck,CARDS,1,me->file);
    assert(writeOk==1);
  }
  ++me->counts[k];
}

int deckComp(Card *a, Card *b) {
  return memcmp(a,b,CARDS);
}

uint32_t unique(uint32_t n, Card *cards) {
  uint32_t i=0,j=1;
  while (j < n) {
    if (memcmp(cards+i*CARDS,cards+j*CARDS,CARDS)==0) {
      ++j;
    } else {
      ++i;
      if (i != j) {
	memcpy(cards+i*CARDS,cards+j*CARDS,CARDS);
      }
      ++j;
    }
  }
  return i+1;
}

uint32_t DeckSetSort(DeckSet *me) {
  uint32_t dups  = 0;
  int maxCount = 0;
  for (int k=0; k<me->nbins; ++k) {
    if (me->counts[k] > maxCount) maxCount = me->counts[k];
  }

  if (maxCount < 2) {
    return dups;
  }

  Card *cards = NULL;
  if (me->file != NULL) {
    cards = (Card*) malloc(CARDS*maxCount);
    assert(cards != NULL);
  }
  
  for (int k=0; k<me->nbins; ++k) {
    if (me->counts[k] < 2) continue;
    uint64_t offset = ((k > 0) ? me->offsets[k-1] : 0);
    if (me->file == NULL) {
      qsort(me->cards+CARDS*offset,me->counts[k],CARDS,
	    (int (*)(const void *, const void *))deckComp);
      uint32_t count=unique(me->counts[k],me->cards+CARDS*offset);
      dups += (me->counts[k]-count);
      me->counts[k]=count;
    } else {
      offset = CARDS*offset + me->nbins*sizeof(uint32_t);
      int seekOk = fseek(me->file,offset,SEEK_SET);
      assert(seekOk==0);
      int readOk = fread(cards,CARDS,me->counts[k],me->file);
      assert(readOk==me->counts[k]);
      qsort(cards,me->counts[k],CARDS,
	    (int (*)(const void *, const void *))deckComp);
      uint32_t count=unique(me->counts[k],cards);      
      dups += (me->counts[k]-count);
      me->counts[k]=count;
      seekOk = fseek(me->file,offset,SEEK_SET);
      assert(seekOk==0);    
      int writeOk = fwrite(cards,CARDS,me->counts[k],me->file);
      assert(writeOk==me->counts[k]);
    }
  }
  
  free(cards);

  return dups;
  
}

void DeckSetClose(DeckSet *me) {
  DeckSetSave(me);
  free(me->cards);
  free(me->counts);
  free(me->offsets);
}


int DeckSetContains(DeckSet *me, Deck deck) {
  int k=0;
  for (int i=0; i<me->pbins; ++i) {
    k = CARDS*k + deck[i];
  }
  if (me->counts[k] == 0) return 0;

  int64_t lo = ((k > 0) ? ((int64_t)me->offsets[k-1]) : ((int64_t)0))  - 1;
  int64_t hi = lo + me->counts[k] + 1;

  while (hi-lo >= 2) {
    int64_t mid = (lo+hi)/2;
    Deck tmp;
    uint64_t offset = mid;
    int cmp = 0;
    if (me->file == NULL) {
      cmp=deckComp(deck,me->cards+offset*CARDS);
    } else {
      offset = CARDS*offset + me->nbins*sizeof(uint32_t);
      int seekOk = fseek(me->file,offset,SEEK_SET);
      assert(seekOk==0);
      int readOk = fread(tmp,CARDS,1,me->file);
      assert(readOk==1);
      cmp = deckComp(deck,tmp);
    }
    if (cmp == 0) return 1;
    if (cmp < 0) {
      hi = mid;
    } else {
      lo = mid;
    }
  }
  return 0;
}




FACTS(DeckSet) {
  for (int tmp = 0; tmp<2; ++tmp) {
    for (int pbins = 1; pbins < 4; ++pbins) {
      DeckSet *ds = (DeckSet*) malloc(sizeof(DeckSet));
      assert (ds != NULL);
      FILE *file = NULL;
      if (tmp) {
	file=tmpfile();
	assert(file != NULL);
      }
      DeckSetInit(ds,pbins,file);

      for (int64_t j=0; j<4; ++j) {
	for (int64_t i=0; i<2*6*24; ++i) {
	  int64_t k = i+pow(2*6*24,j);
	  if (k % 17 == 0) continue;
	  Deck deck;

	  deckInit(deck);
	  Z(deck,k);
	  DeckSetCount(ds,deck);
	  if (k % 19 == 0) {
	    DeckSetCount(ds,deck);
	  }
	}
      }


      DeckSetCounted(ds);

      for (int64_t j=0; j<4; ++j) {
	for (int64_t i=0; i<2*6*24; ++i) {
	  int64_t k = i+pow(2*6*24,j);
	  if (k % 17 == 0) continue;
	  Deck deck;
	  deckInit(deck);
	  Z(deck,k);
	  DeckSetAdd(ds,deck);
	  if (k % 19 == 0) {
	    DeckSetAdd(ds,deck);
	  }
	}
      }

      DeckSetSort(ds);

      for (int64_t j=0; j<4; ++j) {
	for (int64_t i=0; i<2*6*24; ++i) {
	  int64_t k = i+pow(2*6*24,j);
	  Deck deck;
	  deckInit(deck);
	  Z(deck,k);
	  int ans = DeckSetContains(ds,deck);
	  FACT(ans,==,k % 17 != 0);
	}
      }

      DeckSetClose(ds);
      if (file != NULL) {
	fclose(file);
      }
    }
  }
}

void Neighbors(DeckSet *ds,Deck deck, int dir, int count, int dist) {
  if (dist > 0) {
    Deck tmp,next;
    for (int c=0; c<CARDS; ++c) {
      Deck next;
      if (dir == 1) {
	deckCut(deck,c,tmp);
	deckBackFrontShuffle(tmp,next);
      } else {
        deckBackFrontUnshuffle(deck,tmp);
	deckCut(tmp,CARDS-c,next);
      }

      if (count) {
	DeckSetCount(ds,next);
      } else {
	DeckSetAdd(ds,next);
      }
      Neighbors(ds,next,dir,count,dist-1);
    }
  }
}

int dups(int dir, int dist) {
  Deck deck;
  deckInit(deck);

  FILE *file = dist > 5 ? tmpfile() : NULL;
  int pbins = dist > 5 ? 5 : 4;

  DeckSet *ds = (DeckSet*) malloc(sizeof(DeckSet));
  DeckSetInit(ds,pbins,file);

  int count = 1;
  Neighbors(ds,deck,dir,count,dist);

  DeckSetCounted(ds);
  count = 0;
  Neighbors(ds,deck,dir,count,dist);

  int dups = DeckSetSort(ds);

  DeckSetClose(ds);
  fclose(file);
  free(ds);

  return dups;
}

FACTS(Neigbors) {
  int fdup = dups(1,5);
  int bdup = dups(-1,5);
  FACT(fdup,==,0);
  FACT(bdup,==,0);
}
      
FACTS_FINISHED
FACTS_MAIN
