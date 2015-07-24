//		BLOWFISH.H

#ifndef __BLOWFISH_H
#define __BLOWFISH_H

extern "C" {                                                       

void  Blowfish_Init(unsigned char pKey[], unsigned int unKeySize);  
void  Blowfish_ECBEncrypt(unsigned long *pDst, unsigned long *pSrc, unsigned int unCount);
void  Blowfish_ECBDecrypt(unsigned long *pDst, unsigned long *pSrc, unsigned int unCount);
void  Blowfish_CBCEncrypt(unsigned long *pDst, unsigned long *pSrc, unsigned int unCount,
                                        unsigned long *ulCBCLeft, unsigned long *ulCBCRight);
void  Blowfish_CBCDecrypt(unsigned long *pDst, unsigned long *pSrc, unsigned int unCount,
                                        unsigned long *ulCBCLeft, unsigned long *ulCBCRight);
void  Blowfish_Done();
void  Blowfish_SetRounds(unsigned int unRounds);
void*  Blowfish_GetBoxPointer();
char  Blowfish_WeakKey();

};

#endif


