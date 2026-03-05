#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>


#include "spider_cipher.h"
#include "utf8.h"
#include "cio.h"

void DeckKey(Deck deck, const char *key) {
  CIOArray u8Key;
  int keylen = strlen(key);
  CIOArrayU8Init(&u8Key,(uint8_t*)key,0,keylen,keylen,keylen);
  CIOUTF8 u32Key;
  CIOUTF8Init(&u32Key,&u8Key.base,0);
  
  CIO null;
  CIOInit(&null);

  CIOTranslate trans;
  deckInit(deck);
  CIOTranslateInit(&trans,&null,deck,1);
  encodeIO(&u32Key.base,&trans.base);
  CIOClose(&trans);
  CIOClose(&null);  
  CIOClose(&u32Key);
  CIOClose(&u8Key);
}

int main(int argc, char *argv[]) {
  CIOFILE  cioStdin;
  CIOUTF8  cioU32Stdin;
  CIOFILE  cioStdout;
  CIOUTF8  cioU32Stdout;

  CIOFILEInit(&cioStdin,stdin,0);
  CIOUTF8Init(&cioU32Stdin,&cioStdin.base,0);
  CIOFILEInit(&cioStdout,stdout,0);
  CIOUTF8Init(&cioU32Stdout,&cioStdout.base,0);

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
	CIOArrayU8Init(&memPlain,(uint8_t*)plain,0,strlen(plain),strlen(plain),strlen(plain));
	CIOUTF8 memU32Plain;
	CIOUTF8Init(&memU32Plain,&memPlain.base,0);
	
	if (strcmp(plain,"-") != 0) {
	  wcIn=&memU32Plain.base;
	} else {
	  wcIn=&cioU32Stdin.base;
	}

	CIORand rcg;
	CIORandInit(&rcg);

	CIOCardsFmt cardOut;

	CIOCardsFmtInit(&cardOut,&cioU32Stdout.base,mode);
	encryptEnvelopeIO(deck,wcIn, &rcg.base, &cardOut.base);
	printf("\n");

	CIOClose(&cardOut);
	CIOClose(&rcg);
	CIOClose(&memU32Plain);
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
	CIOArrayU8Init(&memCipher,(uint8_t*)cipher,0,strlen(cipher),strlen(cipher),strlen(cipher));
	CIOUTF8 memU32Cipher;
	CIOUTF8Init(&memU32Cipher,&memCipher.base,0);
	if (strcmp(cipher,"-") != 0) {
	  wcIn=&memU32Cipher.base;
	} else {
	  wcIn=&cioU32Stdin.base;
	}

	CIOCardsFmt cardIn;
	CIOCardsFmtInit(&cardIn,wcIn,mode);

	CIO deny;
	CIOInit(&deny);

	decryptEnvelopeIO(deck,&cardIn.base, &deny, &cioU32Stdout.base);

	CIOClose(&deny);
	CIOClose(&cardIn);
	CIOClose(&memU32Cipher);
	CIOClose(&memCipher);	
	deckInit(deck);
	printf("\n");
	continue;
      }
    }
  }

  CIOClose(&cioU32Stdin);
  CIOClose(&cioU32Stdout);  
  CIOClose(&cioStdin);
  CIOClose(&cioStdout);
  
  return 0;
}
