#include "StdAfx.h"

#include "complex_tmpl.h"
#include "BM_templ.h"

#define ffcvt }\
_asm	cdq					\
_asm	shr		edx,	1	\
_asm	xor		eax,	edx	\
_asm{
//-------------------------------
static	void	GetFloatMinMax(LPVOID src, int cnt, float& __min, float& __max)
{
	_asm{
		mov		eax,	[__max]	;
		mov		eax,	[eax]	;
		ffcvt					;
		mov		ebx,	eax		;
		mov		eax,	[__max]	;
		mov		eax,	[eax]	;
		ffcvt					;
		mov		edi,	eax		; edi=__min, ebx=__max
//------------------------------
		mov		esi,	[src]	;
		mov		ecx,	[cnt]	;
		jmp		t10				;
t8:								;
		mov		edi,	eax		;
t9:								;
		dec		ecx				;
		jz		t19				;
t10:							;
		mov		eax,	[esi]	;
		ffcvt					;
		add		esi,	4		;
		cmp		edi,	eax		;
		jg		t8				;
		cmp		ebx,	eax		;
		jge		t9				;
		mov		ebx,	eax		;
		jmp		t9				;
t19:							;
		mov		eax,	edi		;
		ffcvt					;
		mov		esi,	[__min]	;
		mov		[esi],	eax		;
		mov		eax,	ebx		;
		ffcvt					;
		mov		esi,	[__max]	;
		mov		[esi],	eax		;
	}//ASM
}
#undef	ffcvt
//------------------------------------------------
static	void	GetF_Img8(LPVOID src, LPVOID dst, int dim, float __min, float gain){
float	flt05 = 0.5f, flt255=255.f;
_asm{
		mov		esi, 	[src]		;
		mov		edi, 	[dst]		;
		mov		ecx,	[dim]		;
		fld		[flt05]				;
		fld		[flt255]			;
		fld		[__min]				;
		fld		[gain]				;
t10:
		fld		dword ptr[esi]		; D   sc   __min  255  0.5
		fsub	st,		st(2)		;
		add		esi,	4			;
		fmul	st,		st(1)		;
//----------------------------------------
		fcom	st(4)				;
		fstsw	ax					;
		sahf						;
		ja		t13					;
		fstp	st					;
		mov		byte ptr [edi],	0	;
		jmp		t20					;
t13:								;
		fcom	st(3)				;
		fstsw	ax					;
		sahf						;
		jbe		t14					;
		fstp	st					;
		mov	byte ptr [edi], 255		;
		jmp		t20					;
t14:								;
//----------------------------------------
		fistp	[gain]				;
		mov		eax,	[gain]		;
		mov		[edi],	al			;
t20:								;
		inc		edi					;
		dec		ecx					;
		jnz		t10					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
	}//ASM
}
//=========================================================================

BOOL	BM8::Load(const BMF& bm,float gain)
{
	float	__min=0., __max=1.;
	int		y;

	if( (bm.P==NULL) || (P==NULL) || (W != bm.W) || (H<bm.H) ) return FALSE;
	GetFloatMinMax(bm.P, bm.W*bm.H, __min=bm.P[0], __max=bm.P[0]);
	if ((__max-__min) < 1e-7) __max = __min+1.;                        
	__max = gain * 255. / (__max-__min);
	if(gain==0) {__min=0; __max=1.0;}
	
	for(y=0; y<bm.H; y++)
		GetF_Img8 (bm.P+y*bm.W,	P+y*W, __min(bm.W, W), __min, __max);
	return TRUE;		
}

