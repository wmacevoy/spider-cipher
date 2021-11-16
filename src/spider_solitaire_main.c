#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>


#include "spider_solitaire.h"
#include "utf8.h"
#include "cio.h"

void DeckKey(Deck deck, const char *key) {
  CIOArray u8Key;
  int keylen = strlen(key);
  CIOArrayConstU8Init(&u8Key,key,0,keylen);
  CIOUTF8 wcKey;
  CIOUTF8Init(&wcKey,&u8Key.base);
  
  CIO null;
  CIOInit(&null);

  CIOTranslate trans;
  deckInit(deck);
  CIOTranslateInit(&trans,&null,deck,1);
  encodeIO(&wcKey.base,&trans.base);
  CIOClose(&trans);
  CIOClose(&null);  
  CIOClose(&wcKey);
  CIOClose(&u8Key);
}

int main(int argc, char *argv[]) {
  CIOFILE  cioStdin;
  CIOUTF8  cioWCStdin;
  CIOFILE  cioStdout;
  CIOUTF8  cioWCStdout;

  CIOFILEInit(&cioStdin,stdin,0);
  CIOUTF8Init(&cioWCStdin,&cioStdin.base);
  CIOFILEInit(&cioStdout,stdout,0);
  CIOUTF8Init(&cioWCStdout,&cioStdout.base);

  int mode = '#';
  const char *key = NULL;
  for (int argi=1; argi<argc; ++argi) {
    const char *arg=argv[argi];
    
    {
      const char *op="--key=";
      int oplen = strlen(op);
      if (strncmp(arg,op,oplen)==0) {
	key = arg+oplen;
	continue;
      }
    }

    {
      const char *op="--decimal";
      if (strcmp(arg,op)==0) {
	mode = '#';
	continue;
      }
    }
    
    {
      const char *op="--base40";
      if (strcmp(arg,op)==0) {
	mode = 'b';
	continue;
      }
    }

    {
      const char *op="--cards";
      if (strcmp(arg,op)==0) {
	mode = 'c';
	continue;
      }
    }
    
    {
      const char *op="--encrypt=";
      int oplen = strlen(op);      
      if (strncmp(arg,op,oplen)==0) {
	const char *plain=arg+oplen;
	Deck deck;
	DeckKey(deck,key);

	CIO *wcIn = NULL;

	CIOArray memPlain;
	CIOArrayConstU8Init(&memPlain,plain,0,strlen(plain));
	CIOUTF8 memWCPlain;
	CIOUTF8Init(&memWCPlain,&memPlain.base);
	
	if (strcmp(plain,"-") != 0) {
	  wcIn=&memWCPlain.base;
	} else {
	  wcIn=&cioWCStdin.base;
	}

	CIORand rcg;
	CIORandInit(&rcg);

	CIOCardsFmt cardOut;

	CIOCardsFmtInit(&cardOut,&cioWCStdout.base,mode);
	encryptEnvelopeIO(deck,wcIn, &rcg.base, &cardOut.base);
	printf("\n");

	CIOClose(&cardOut);
	CIOClose(&rcg);
	CIOClose(&memWCPlain);
	CIOClose(&memPlain);
	deckInit(deck);
	continue;
      }
    }

    {
      const char *op="--decrypt=";
      int oplen = strlen(op);      
      if (strncmp(arg,op,oplen)==0) {
	const char *cipher=arg+oplen;
	Deck deck;
	DeckKey(deck,key);

	CIO *wcIn = NULL;

	CIOArray memCipher;
	CIOArrayConstU8Init(&memCipher,cipher,0,strlen(cipher));
	CIOUTF8 memWCCipher;
	CIOUTF8Init(&memWCCipher,&memCipher.base);
	if (strcmp(cipher,"-") != 0) {
	  wcIn=&memWCCipher.base;
	} else {
	  wcIn=&cioWCStdin.base;
	}

	CIOCardsFmt cardIn;
	CIOCardsFmtInit(&cardIn,wcIn,mode);

	CIO deny;
	CIOInit(&deny);

	decryptEnvelopeIO(deck,&cardIn.base, &deny, &cioWCStdout.base);

	CIOClose(&deny);
	CIOClose(&cardIn);
	CIOClose(&memWCCipher);
	CIOClose(&memCipher);	
	deckInit(deck);
	printf("\n");
	continue;
      }
    }
  }

  CIOClose(&cioWCStdin);
  CIOClose(&cioWCStdout);  
  CIOClose(&cioStdin);
  CIOClose(&cioStdout);
  
  return 0;
}
