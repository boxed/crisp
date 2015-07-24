/****************************************************************************/
/* Copyright© 1996 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* MARX Crypto-Box Interface by ML ******************************************/
/****************************************************************************/
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#define	MARX_IMPORT
#include "cbn_.h"

#include "shcb.h"

static	DWORD IDc[] = {0, 0x022AB0, 0x2247B0, 0x79B4B0, 0x012287, 0x8BAF10, 0x7FA1B8, 0x384A5F, 0x375BAE };
static	DWORD IDr[] = {0,   0x420D,   0x00CA,   0xCD22,   0x1725,   0xBA80,   0x7A8B,   0x9A7B,   0xEA75 }; 

static	LONG PW1 = 0x22880B00;

static short iPort;
static	USER user;
static	WORD wBoxName;

#pragma	warning(disable:4133 )

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static short rn13(void)
{
int i = rand()&3;
	if (i==0) i++;
	return i;
}
/****************************************************************************/
WORD scbLookup( void )
{   
short	i;
DWORD	ww;

	for(iPort=11;iPort<14;iPort++)
		if (CbN_BoxReady(iPort,&wBoxName)==0) break;
	if (iPort==14) return 0;

	if (CbN_ReadID1(iPort, &IDc[1], &ww) || ww!=IDr[1]) return 0;
	if (CbN_ReadID2(iPort, &IDc[2], &ww) || ww!=IDr[2]) return 0;
	if (CbN_ReadID3(iPort, &IDc[3], &ww) || ww!=IDr[3]) return 0;
	if (CbN_ReadID4(iPort, &IDc[4], &ww) || ww!=IDr[4]) return 0;
	if (CbN_ReadID5(iPort, &IDc[5], &ww) || ww!=IDr[5]) return 0;
	if (CbN_ReadID6(iPort, &IDc[6], &ww) || ww!=IDr[6]) return 0;
	if (CbN_ReadID7(iPort, &IDc[7], &ww) || ww!=IDr[7]) return 0;
	if (CbN_ReadID8(iPort, &IDc[8], &ww) || ww!=IDr[8]) return 0;

	srand((int)GetTickCount());
	i = rn13(); if (CbN_ReadRAM1( iPort, i, &IDc[i], &PW1, 0, 
								 sizeof(user.dwPW2)+sizeof(user.wSRN)+sizeof(user.UserName), &user )) return 0;
	return user.wSRN;

}
/****************************************************************************/
BOOL CheckCRC( LPUSER u, int app )
{
DWORD	crc;
LPDWORD a;
short	i;

	switch(app) {
		case APP_CRISP:
			crc = u->dwPW2 + u->wSRN;
			for(i=0,a=(LPDWORD)&u->c;i<APP_SIZE/4-1;i++,a++) crc+=*a;
			if (u->c.crc != crc) return FALSE; break;
		case APP_PENTACLE:
			crc = u->dwPW2 + u->wSRN;
			for(i=0,a=(LPDWORD)&u->p;i<APP_SIZE/4-1;i++,a++) crc+=*a;
			if (u->p.crc != crc) return FALSE; break;
		case APP_TRIX:
			crc = u->dwPW2 + u->wSRN;
			for(i=0,a=(LPDWORD)&u->t;i<APP_SIZE/4-1;i++,a++) crc+=*a;
			if (u->t.crc != crc) return FALSE; break;
		default: return FALSE;
	}
	return TRUE;
}
/****************************************************************************/
LPVOID scbGetAppData( short app )
{
short	i;

	switch(app) {
		case APP_CRISP:
			i = rn13(); if (CbN_ReadRAM2( iPort, i, &IDc[i], &user.dwPW2, (short)(app*APP_SIZE), APP_SIZE, &user.c )) return NULL;
			i = rn13(); if (CbN_IDEA_Decrypt( iPort, i, &IDc[i], &user.c, sizeof(user.c))) return NULL; break;
		case APP_TRIX:
			i = rn13(); if (CbN_ReadRAM2( iPort, i, &IDc[i], &user.dwPW2, (short)(app*APP_SIZE), APP_SIZE, &user.t )) return NULL;
			i = rn13(); if (CbN_IDEA_Decrypt( iPort, i, &IDc[i], &user.t, sizeof(user.t))) return NULL; break;
		case APP_PENTACLE:
			i = rn13(); if (CbN_ReadRAM2( iPort, i, &IDc[i], &user.dwPW2, (short)(app*APP_SIZE), APP_SIZE, &user.p )) return NULL;
			i = rn13(); if (CbN_IDEA_Decrypt( iPort, 1, &IDc[1], &user.p, sizeof(user.p))) return NULL; break;
		default:  return NULL;
	}		
	if (!CheckCRC(&user, app)) app = -1;
	switch( app ) {
		case APP_CRISP:		return (LPVOID) &user.c;
		case APP_TRIX: 		return (LPVOID) &user.t;
		case APP_PENTACLE:	return (LPVOID) &user.p;
	}
	return NULL;
}
/****************************************************************************/
/****************************************************************************/
BOOL scbGetUserName( LPSTR s )
{
short	i;
	if (user.wSRN == 0) return FALSE;
	memset(s,0,sizeof(user.UserName)+1);
	memcpy(s,user.UserName,sizeof(user.UserName));
	i = rn13(); CbN_IDEA_Decrypt(iPort, i, &IDc[i], s, sizeof(user.UserName));
	return TRUE;
}
/****************************************************************************/
