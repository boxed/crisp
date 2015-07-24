#pragma once

#ifndef __CRISP_COMDEF_H__
#define __CRISP_COMDEF_H__

#include "trace.h"
#include "crash.h"
/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Common functions * by ML *************************************************/
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
/*- Useful typedefs --------------------------------------------------------*/
/*--------------------------------------------------------------------------*/
typedef	BYTE * LPBYTE;
typedef	WORD * LPWORD;
typedef	DWORD * LPDWORD;
typedef	LONG  * LPLONG;
typedef	float  * LPFLOAT;
typedef	double * LPDOUBLE;
typedef struct { double x,y; } POINTD, *LPPOINTD;

//#pragma	warning(disable:4136 4018 4135 4309)
#pragma	warning(disable:4244)

/****************************************************************************/
/* Definitions **************************************************************/
/****************************************************************************/

#define	R2(a)	((int)((a+1)&~1))
#define	R4(a)	((int)((a+3)&~3))
#define	R8(a)   ((int)((a+7)&~7))

#define	_MoveTo(a,b,c)	MoveToEx(a,b,c,NULL)

#define	BitSet(a,b)	a|=b
#define BitClr(a,b)	a&=~(b)
#define	Bit(a,b)	(a&(b))
#define	BitFromID(h,i,m,b) if(IsDlgButtonChecked(h,i)) BitSet(m,b); else BitClr(m,b)
#define	NotBitFromID(h,i,m,b) if(!IsDlgButtonChecked(h,i)) BitSet(m,b); else BitClr(m,b)

/**************************************************************************************/
//#ifndef M_PI
//#define M_PI	3.14159265358979323846
//#endif
#define	TINY	(1e-15)

#if 0
typedef short	PHASE;
	#define	PADD2	(PHASE)0x8000
	#define	PDIV2	32768.
//	#define	P_6d	1092
	#define P_PI	((PHASE)(0x8000))
#else
typedef int		PHASE;
	#define	PADD2	(PHASE)(0x80000000*0)
	#define	PDIV2	2147483648.
//	#define	P_6d	71582788
	#define P_PI	((PHASE)(0x80000000))
	#define	P_3PI2	((PHASE)(0x55555555))
#endif

#define R2D(p)	((p)*(180./M_PI))
#define	D2R(p)	((p)*(M_PI/180.))
#define P2D(p)	((double)((PHASE)(p)+PADD2) * 180./PDIV2)
#define P2R(p)  ((double)((PHASE)(p)+PADD2) * M_PI/PDIV2)
#define D2P(d)	((PHASE)round((d) * PDIV2/180.)-PADD2)
#define R2P(r)	((PHASE)round((r) * PDIV2/M_PI)-PADD2)

/*
#define R2D(p)	((p)*(180./M_PI))
#define	D2R(p)	((p)*(M_PI/180.))
#define P2D(p)	((double)((PHASE)p+0x80000000) * 180./2147483648.)
#define P2R(p)  ((double)((PHASE)p+0x80000000) * M_PI/2147483648.)
#define D2P(d)	((PHASE)round((d) * 2147483648./180.)-0x80000000)
#define R2P(r)	((PHASE)round((r) * 2147483648./M_PI)-0x80000000)
*/

/*
#define	___a 57.295779513082	// 180./M_PI;
#define	___b 0.017453292519943	// M_PI/180.;
#define	___c -0.005493164062500	// 180./32768.;
#define	___d -9.5873799242853e-5	// M_PI/32768.;
#define	___e -182.04444444444	// 32768./180.;
#define	___f -10430.378350470	// 32768./M_PI;

#define R2D(p)	((p)*___a)
#define	D2R(p)	((p)*___b)
#define P2D(p)	((float)(p) * ___c)
#define P2R(p)  ((float)(p) * ___d)
#define D2P(d)	((round((d) * ___e)))
#define R2P(r)	((round((r) * ___f)))

*/
#define	LOINT(a)	((short)LOWORD(a))
#define	HIINT(a)	((short)HIWORD(a))

#define	ATAN2(a,b)	atan2((double)(a),(double)(b))
#define	SQRT(a)		sqrt((double)(a))
#define	CABS(a,b)	sqrt((double)(a)*(double)(a)+(double)(b)*(double)(b))
#define	ABANGLE(ax,ay,bx,by) fabs(P2R(R2P( ATAN2(by, bx) - ATAN2(ay, ax) )))

#endif // __CRISP_COMDEF_H__