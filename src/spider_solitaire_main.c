#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "spider_solitaire.h"
#include "utf8.h"
#include "cio.h"

int CIOWriteUtf8(CIO *cio, const char *utf8, int utf8len) {
  while (utf8len > 0) {
    int declen = utf8declen(utf8,utf8len);
    if (declen > 0) {
      int status = CIOWrite(cio,utf8decval(utf8,declen));
      if (status == -1) {
	return -1;
      }
    } else {
      return -2;
    }
    utf8 += declen;
    utf8len -= declen;
  }
  return 0;
}

void CIOArraySetUtf8(CIOArray *array, const char *utf8) {
  array->position = 0;
  array->size = 0;
  int status = CIOWriteUtf8(&array->base,utf8,strlen(utf8));
  if (status != 0) {
    fprintf(stderr,"invalid utf8 (%s) or out of memory.",utf8);
    exit(1);
  }
  array->position=0;
}

void DeckKey(Deck deck, const char *utf8) {
  CIOArray key;
  CIOArrayWideCharInit(&key,NULL,0,0,0,INT_MAX);	
  CIOArraySetUtf8(&key,utf8);

  CIO null;
  CIOInit(&null);

  CIOTranslate trans;
  deckInit(deck);
  CIOTranslateInit(&trans,&null,deck,1);
  encodeIO(&key.base,&trans.base);
  CIOClose(&trans);
  CIOClose(&null);  
  CIOClose(&key);
}

struct CIOCardsFmtStruct;
typedef struct CIOCardsFmtStruct CIOCardsFmt;
struct CIOCardsFmtStruct {
  CIO base;
  FILE *out;
};

int CIOCardsFmtWrite(CIOCardsFmt *me, int card) {
  if (card < 0 || card >= CARDS) {
    fprintf(me->out,"?(%d)",card);
  }

  int faceNo = cardFaceNo(card);
  int suiteNo = cardSuiteNo(card);

  wchar_t face = cardFaceFromNo(faceNo);
  wchar_t suite = cardSuiteFromNo(suiteNo);

  wchar_t ws[2]={face,suite};
  unsigned char buf[8];
  for (int i=0; i<2; ++i) {
    int enclen = utf8enclen(ws[i]);
    utf8encval(buf,ws[i],enclen);
    buf[enclen]=0;
    if (fwrite(buf,1,enclen,me->out) != enclen) {
      fprintf(stderr,"write failed.");
      exit(1);
    }
  }
  fprintf(me->out,"(%02d)",card);

  int w = CIOGetWrites(me)+1;

  if (w % 5 == 0) {
    if (w % 10 == 0) {
      fprintf(me->out,"\n");
    } else {
      fprintf(me->out,"-");
    }
  }

  return 0;
}

void CIOCardsFmtInit(CIOCardsFmt *me,FILE *out) {
  CIOInit(&me->base);
  me->base.write = (CIOWritePtr) &CIOCardsFmtWrite;
  me->out=out;
}

int main(int argc, char *argv[]) {
  const char *plain = NULL;
  const char *key = NULL;
  for (int argi=1; argi<argc; ++argi) {
    const char *arg = argv[argi];
    {
      const char *op="--plain=";
      int oplen = strlen(op);
      if (strncmp(arg,op,oplen)==0) {
	plain = arg+oplen;
	continue;
      }
    }

    {
      const char *op="--key=";
      int oplen = strlen(op);
      if (strncmp(arg,op,oplen)==0) {
	key = arg+oplen;
	continue;
      }
    }

    {
      const char *op="--encrypt";
      if (strcmp(arg,op)==0) {
	Deck deck;
	DeckKey(deck,key);

	CIOArray wcPlain;
	CIOArrayWideCharInit(&wcPlain,NULL,0,0,0,INT_MAX);	
	CIOArraySetUtf8(&wcPlain,plain);

	CIORand rcg;
	CIORandInit(&rcg);

	CIOCardsFmt fmt;
	CIOCardsFmtInit(&fmt,stdout);
	encryptEnvelopeIO(deck,&wcPlain.base, &rcg.base, &fmt.base);

	CIOClose(&fmt);
	CIOClose(&rcg);
	CIOClose(&wcPlain);
	deckInit(deck);
      }
    }
  }
  return 0;
}
