/****************************************************************************/
/* Copyright (c) 2008 Calidris **********************************************/
/****************************************************************************/
/* CRISP2.ImageCalculator functions * by Peter Oleynikov ********************/
/****************************************************************************/

#include "StdAfx.h"

#include "ICalc.h"

#ifdef max
	#undef max
	#undef min
#endif

template <typename _Tx>
void imPre_t(PREAL dst, _Tx *src, DWORD ofs, int iwid, int shft, int sub)
{
	typedef _Tx* pointer;
	const double _constMin = (double)(std::numeric_limits<_Tx>::min());
	const double _constMax = (double)(std::numeric_limits<_Tx>::max());
	pointer ptr = (pointer)(ofs + (LPBYTE)src);
	double tmp, mul, dsub;
	register int i;
	mul = pow(2.0, shft);
	dsub = (double)sub;
	if( 1.0 == mul )
	{
		for(i = 0; i < iwid; i++)
		{
			tmp = ((double)ptr[i]) - dsub;
			if( tmp < _constMin ) { tmp = _constMin; }
			if( tmp > _constMax ) { tmp = _constMax; }
			dst[i] = tmp;
		}
	}
	else
	{
		for(i = 0; i < iwid; i++)
		{
			tmp = mul * (((double)ptr[i]) - dsub);
			if( tmp < _constMin ) { tmp = _constMin; }
			if( tmp > _constMax ) { tmp = _constMax; }
			dst[i] = tmp;
		}
	}
}

template <typename _Tx>
void imPost_t(_Tx *dst, DWORD ofs, PREAL src, int iwid, REAL fAdd, REAL fMul)
{
	typedef _Tx* pointer;
	pointer ptr = (pointer)(ofs + (LPBYTE)dst);
	double tmp;
	register int i;
	if( 1.0 == fMul )
	{
		for(i = 0; i < iwid; i++)
		{
			ptr[i] = src[i] + fAdd;
		}
	}
	else
	{
		for(i = 0; i < iwid; i++)
		{
			ptr[i] = fMul * (src[i] + fAdd);
		}
	}
}

template <typename _Tx>
void imPostC_t(_Tx *dst, DWORD ofs, PREAL src, int iwid, REAL fAdd, REAL fMul, LONG lAdd)
{
	typedef _Tx* pointer;
	const double _constMin = (double)(std::numeric_limits<_Tx>::min());
	const double _constMax = (double)(std::numeric_limits<_Tx>::max());
	pointer ptr = (pointer)(ofs + (LPBYTE)dst);
	double tmp;
	register int i;
	if( 1.0 == fMul )
	{
		for(i = 0; i < iwid; i++)
		{
			tmp = src[i] + fAdd - lAdd;
			if( tmp < _constMin ) { tmp = _constMin; }
			if( tmp > _constMax ) { tmp = _constMax; }
			ptr[i] = tmp;
		}
	}
	else
	{
		for(i = 0; i < iwid; i++)
		{
			tmp = fMul * (src[i] + fAdd) - lAdd;
			if( tmp < _constMin ) { tmp = _constMin; }
			if( tmp > _constMax ) { tmp = _constMax; }
			ptr[i] = tmp;
		}
	}
}

#ifdef __cplusplus
extern "C" {
#endif

void icPost8sc(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd)
{
	imPostC_t((LPSTR)dst, ofs, src, wid, fAdd, fMul, lAdd);
}

void icPost16sc(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd)
{
	imPostC_t((short*)dst, ofs, src, wid, fAdd, fMul, lAdd);
}

//////////////////////////////////////////////////////////////////////////

void im32PreUp(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<DWORD>(dst, (LPDWORD)src, ofs, iwid, shft, sub);
}

void im32PreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<DWORD>(dst, (LPDWORD)src, ofs, iwid, -shft, sub);
}
//////////////////////////////////////////////////////////////////////////

void im32sPreUp(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<LONG>(dst, (LPLONG)src, ofs, iwid, shft, sub);
}

void im32sPreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<LONG>(dst, (LPLONG)src, ofs, iwid, -shft, sub);
}

void icPost32sc(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd)
{
	imPostC_t((LPLONG)dst, ofs, src, wid, fAdd, fMul, lAdd);
}

//////////////////////////////////////////////////////////////////////////

void imFPreUp(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<float>(dst, (float*)src, ofs, iwid, shft, sub);
}

void imFPreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<float>(dst, (float*)src, ofs, iwid, -shft, sub);
}

void icPostFc(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd)
{
	imPostC_t((float*)dst, ofs, src, wid, fAdd, fMul, lAdd);
}

//////////////////////////////////////////////////////////////////////////

void imDPreUp(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<double>(dst, (double*)src, ofs, iwid, shft, sub);
}

void imDPreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub)
{
	imPre_t<double>(dst, (double*)src, ofs, iwid, -shft, sub);
}

void icPostDc(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd)
{
	imPostC_t((double*)dst, ofs, src, wid, fAdd, fMul, lAdd);
}

//////////////////////////////////////////////////////////////////////////
/*
template <typename _Tx>
WORD imGetFrameDC_t(_Tx *src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid)
{
	typedef _Tx type;
	typedef type* pointer;
	DWORD pcnt = 0;
	WORD wRet = 0;
	pointer ptr = pointer(((LPBYTE)src) + soff * sizeof(type));
	for(;;)
	{
		int offset = msk[0];
		int cnt = msk[1];
		iwid = iwid;
		for(int i = offset; i < offset+cnt; i++)
		{
			
		}
		break;
	}
	return wRet;
}

WORD im16GetFrameDC(LPWORD src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid)
{
	return imGetFrameDC_t(src, soff, msk, iwid, fwid);
}

WORD im32GetFrameDC(LPDWORD src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid)
{
	DWORD pcnt;
	WORD wRet = 0;
	_asm
	{
		mov	esi,	[src]			;
		add	esi,	[soff]			;
		add	esi,	[soff]			;
		mov	ebx,	[msk]			;
		movzx	eax,	word ptr [ebx]	;
		lea	esi,	[esi+eax*4]		; First StosCnt
		xor	eax,	eax				;
		xor	ecx,	ecx				;
		mov	[pcnt],	ecx				;
Label_10:							;
		movzx	edi,	word ptr [ebx+2]; MovsCnt
		or	di,	di					;
		js	short	Label_90		;
		jz	short	Label_20		;
		movzx	edx,	dword ptr [esi]	;
		add	eax,	edx				;
		adc	ecx,	0				;
		inc	[pcnt]					;
		lea	esi,	[esi+edi*4]		;
		movzx	edx,	dword ptr [esi-4];
		add	eax,	edx				;
		adc	ecx,	0				;
		inc	[pcnt]					;
		;........................................
Label_20:							;
		mov	edx,	[iwid]			; OfsCnt
		sub	edx,	[fwid]			;
		add	dx,	[ebx+4]				;
		movsx	edx,	dx			;
		shl	edx,	2				;
		add	esi,	edx				;
		add	ebx,	4				;
		jmp	short	Label_10		;
Label_90:							;
		mov	edx,	ecx				;
		mov	ecx,	[pcnt]			;
		or	ecx,	ecx				;
		jz	short	Label_99		;
		div	ecx						;
Label_99:							;
		mov wRet, ax				;
	}
	return wRet;
}

WORD im32CountFrameDC(LPDWORD img, LPBYTE msk, DWORD cnt, DWORD mbt)
{
	WORD wRet;
	DWORD	eptr, xeax;
	_asm
	{
	 mov	edi,	[img]		;
	 mov	esi,	[msk]		;
	 mov	eax,	esi			;
	 add	eax,	[cnt]		;
	 mov	[eptr],	eax			;
	 mov	bl,	byte ptr [mbt]	;
	 xor	ecx,	ecx			;
	 xor	eax,	eax			;
	 xor	edx,	edx			;
	 mov	[xeax],	edx			;
Label_10:						;
	 cmp	bl,  byte ptr [esi]	;
	 jnz	Label_20			;
	 movzx	eax,	word ptr es:[edi];
	 add	[xeax],	eax			;
	 adc	edx,	0			;
	 inc	ecx					;
	 Label_20:					;
	 inc	esi					;
	 add	edi,	2			;
	 cmp	esi,	[eptr]		;
	 jnz short	Label_10		;
	 or	ecx,	ecx				;
	 jz	Label_99				;
	 mov	eax,	[xeax]		;
	 div	ecx					;
Label_99:						;
	mov wRet, ax			;
	}
	return wRet;
}

VOID im32MaskCopy(LPDWORD dst, LPDWORD src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc)
{
	_asm
	{
		mov	edi,	[dst]				;
		mov	esi,	[src]				;
		add	esi,	[soff]				;
		add	esi,	[soff]				;
		mov	ebx,	[msk]				;
		movzx eax,	word ptr [ebx]		;
		lea	esi,	[esi+eax*4]			; First StosCnt
		movzx	eax, dword ptr [imdc]	;
		imul	eax,	eax, 100000001h	;
Label_10:								;
		movzx	ecx,	word ptr [ebx]	; StosCnt
		rep	stos	dword ptr [edi]		;
		movzx	ecx,	word ptr [ebx+2]; MovsCnt
		or	cx,	cx						;
		js	short	Label_99			;
		rep	movs dword ptr [edi], [esi]	;
		;........................................
		mov	edx,	[iwid]				; OfsCnt
		sub	edx,	[fwid]				;
		add	dx,	[ebx+4]					;
		movsx	edx,	dx				;
		shl	edx,	2					;
		add	esi,	edx					;
		add	ebx,	4					;
		jmp	short	Label_10			;
Label_99:								;
		;-----------------------------------------------------------------------------
	}
}
*/
//////////////////////////////////////////////////////////////////////////
/*
WORD imFGetFrameDC(float* src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid)
{
	DWORD pcnt;
	WORD wRet;
	_asm
	{
		mov	esi,	[src]			;
		add	esi,	[soff]			;
		add	esi,	[soff]			;
		mov	ebx,	[msk]			;
		movzx	eax,	word ptr [ebx]	;
		lea	esi,	[esi+eax*4]		; First StosCnt
		xor	eax,	eax				;
		xor	ecx,	ecx				;
		mov	[pcnt],	ecx				;
Label_10:
		movzx	edi,	word ptr [ebx+2]; MovsCnt
		or	di,	di					;
		js	short	Label_90		;
		jz	short	Label_20		;
		movzx	edx,	dword ptr [esi]	;
		add	eax,	edx				;
		adc	ecx,	0				;
		inc	[pcnt]					;
		lea	esi,	[esi+edi*4]		;
		movzx	edx,	dword ptr [esi-4];
		add	eax,	edx				;
		adc	ecx,	0				;
		inc	[pcnt]					;
		;........................................
Label_20:
		mov	edx,	[iwid]			; OfsCnt
		sub	edx,	[fwid]			;
		add	dx,	[ebx+4]				;
		movsx	edx,	dx			;
		shl	edx,	2				;
		add	esi,	edx				;
		add	ebx,	4				;
		jmp	short	Label_10		;
Label_90:
		mov	edx,	ecx				;
		mov	ecx,	[pcnt]			;
		or	ecx,	ecx				;
		jz	short	Label_99		;
		div	ecx						;
Label_99:
		mov wRet, ax				;
	}
	return wRet;
}

WORD imFCountFrameDC(float* img, LPBYTE msk, DWORD cnt, DWORD mbt)
{
	WORD wRet;
	DWORD	eptr, xeax;
	_asm
	{
		mov	edi,	[img]		;
		mov	esi,	[msk]		;
		mov	eax,	esi			;
		add	eax,	[cnt]		;
		mov	[eptr],	eax			;
		mov	bl,	byte ptr [mbt]	;
		xor	ecx,	ecx			;
		xor	eax,	eax			;
		xor	edx,	edx			;
		mov	[xeax],	edx			;
Label_10:
		cmp	bl,  byte ptr [esi]	;
		jnz	Label_20			;
		movzx	eax,	word ptr es:[edi];
		add	[xeax],	eax			;
		adc	edx,	0			;
		inc	ecx					;
Label_20:
		inc	esi					;
		add	edi,	2			;
		cmp	esi,	[eptr]		;
		jnz short	Label_10		;
		or	ecx,	ecx				;
		jz	Label_99				;
		mov	eax,	[xeax]		;
		div	ecx					;
Label_99:
		mov wRet, ax			;
	}
	return wRet;
}

VOID imFMaskCopy(float* dst, float* src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc)
{
	_asm
	{
		mov	edi,	[dst]				;
		mov	esi,	[src]				;
		add	esi,	[soff]				;
		add	esi,	[soff]				;
		mov	ebx,	[msk]				;
		movzx eax,	word ptr [ebx]		;
		lea	esi,	[esi+eax*4]			; First StosCnt
		movzx	eax, dword ptr [imdc]	;
		imul	eax,	eax, 100000001h	;
Label_10:
		movzx	ecx,	word ptr [ebx]	; StosCnt
		rep	stos	dword ptr [edi]		;
		movzx	ecx,	word ptr [ebx+2]; MovsCnt
		or	cx,	cx						;
		js	short	Label_99			;
		rep	movs dword ptr [edi], [esi]	;
		;........................................
		mov	edx,	[iwid]				; OfsCnt
		sub	edx,	[fwid]				;
		add	dx,	[ebx+4]					;
		movsx	edx,	dx				;
		shl	edx,	2					;
		add	esi,	edx					;
		add	ebx,	4					;
		jmp	short	Label_10			;
Label_99:
		;-----------------------------------------------------------------------------
	}
}
*/
//////////////////////////////////////////////////////////////////////////
/*
WORD imDGetFrameDC(double* src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid)
{
	DWORD pcnt;
	WORD wRet;
	_asm
	{
		mov	esi,	[src]			;
		add	esi,	[soff]			;
		add	esi,	[soff]			;
		mov	ebx,	[msk]			;
		movzx	eax,	word ptr [ebx]	;
		lea	esi,	[esi+eax*4]		; First StosCnt
		xor	eax,	eax				;
		xor	ecx,	ecx				;
		mov	[pcnt],	ecx				;
Label_10:
		movzx	edi,	word ptr [ebx+2]; MovsCnt
		or	di,	di					;
		js	short	Label_90		;
		jz	short	Label_20		;
		movzx	edx,	dword ptr [esi]	;
		add	eax,	edx				;
		adc	ecx,	0				;
		inc	[pcnt]					;
		lea	esi,	[esi+edi*4]		;
		movzx	edx,	dword ptr [esi-4];
		add	eax,	edx				;
		adc	ecx,	0				;
		inc	[pcnt]					;
		;........................................
Label_20:
		mov	edx,	[iwid]			; OfsCnt
		sub	edx,	[fwid]			;
		add	dx,	[ebx+4]				;
		movsx	edx,	dx			;
		shl	edx,	2				;
		add	esi,	edx				;
		add	ebx,	4				;
		jmp	short	Label_10		;
Label_90:
		mov	edx,	ecx				;
		mov	ecx,	[pcnt]			;
		or	ecx,	ecx				;
		jz	short	Label_99		;
		div	ecx						;
Label_99:
		mov wRet, ax				;
	}
	return wRet;
}

WORD imDCountFrameDC(double* img, LPBYTE msk, DWORD cnt, DWORD mbt)
{
	WORD wRet;
	DWORD	eptr, xeax;
	_asm
	{
		mov	edi,	[img]		;
		mov	esi,	[msk]		;
		mov	eax,	esi			;
		add	eax,	[cnt]		;
		mov	[eptr],	eax			;
		mov	bl,	byte ptr [mbt]	;
		xor	ecx,	ecx			;
		xor	eax,	eax			;
		xor	edx,	edx			;
		mov	[xeax],	edx			;
Label_10:
		cmp	bl,  byte ptr [esi]	;
		jnz	Label_20			;
		movzx	eax,	word ptr es:[edi];
		add	[xeax],	eax			;
		adc	edx,	0			;
		inc	ecx					;
Label_20:
		inc	esi					;
		add	edi,	2			;
		cmp	esi,	[eptr]		;
		jnz short	Label_10		;
		or	ecx,	ecx				;
		jz	Label_99				;
		mov	eax,	[xeax]		;
		div	ecx					;
Label_99:
		mov wRet, ax			;
	}
	return wRet;
}

VOID imDMaskCopy(double* dst, double* src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc)
{
	_asm
	{
		mov	edi,	[dst]				;
		mov	esi,	[src]				;
		add	esi,	[soff]				;
		add	esi,	[soff]				;
		mov	ebx,	[msk]				;
		movzx eax,	word ptr [ebx]		;
		lea	esi,	[esi+eax*4]			; First StosCnt
		movzx	eax, dword ptr [imdc]	;
		imul	eax,	eax, 100000001h	;
Label_10:
		movzx	ecx,	word ptr [ebx]	; StosCnt
		rep	stos	dword ptr [edi]		;
		movzx	ecx,	word ptr [ebx+2]; MovsCnt
		or	cx,	cx						;
		js	short	Label_99			;
		rep	movs dword ptr [edi], [esi]	;
		;........................................
		mov	edx,	[iwid]				; OfsCnt
		sub	edx,	[fwid]				;
		add	dx,	[ebx+4]					;
		movsx	edx,	dx				;
		shl	edx,	2					;
		add	esi,	edx					;
		add	ebx,	4					;
		jmp	short	Label_10			;
Label_99:
		;-----------------------------------------------------------------------------
	}
}
*/
#ifdef __cplusplus
}
#endif
