#pragma once

typedef double REAL;
typedef REAL* PREAL;

extern "C"
{
	// 8 bits unsigned
	extern	void im8PreUp   (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void im8PreDown (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPost8	(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd);
	extern	void icPost8c	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	// 8 bits signed
	extern	void im8sPreUp   (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void im8sPreDown (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPost8s	(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd);
	extern	void icPost8sc	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	// 16 bits unsigned
	extern	void im16PreUp  (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void im16PreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPost16	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	extern	void icPost16c	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	// 16 bits signed
	extern	void im16sPreUp  (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void im16sPreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPost16s	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	extern	void icPost16sc	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	// 32 bits unsigned
	extern	void im32PreUp  (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void im32PreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPost32	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	extern	void icPost32c( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	// 32 bits signed
	extern	void im32sPreUp  (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void im32sPreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPost32s	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	extern	void icPost32sc( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	// float
	extern	void imFPreUp  (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void imFPreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPostF	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	extern	void icPostFc( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	// double
	extern	void imDPreUp  (PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void imDPreDown(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
	extern	void icPostD	( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	extern	void icPostDc( LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
	
	extern	void icAdd   ( PREAL dst, PREAL src1, PREAL src2, int cnt );
	extern	void icSub   ( PREAL dst, PREAL src1, PREAL src2, int cnt );
	extern	void icMul   ( PREAL dst, PREAL src1, PREAL src2, int cnt );
	extern	void icDiv   ( PREAL dst, PREAL src1, PREAL src2, int cnt );
	extern	void icInv   ( PREAL dst, PREAL src1, PREAL src2, int cnt );

	extern	void GetFloatMinMax	(PREAL ftmp, int width, PREAL fmin, PREAL fmax );
//	extern	void GetFloatMinMaxD(LPVOID src, int cnt, double &_min_, double &_max_);
}

template <typename _Tx>
__forceinline void GetFloatMinMax_t(_Tx *src, int cnt, double &_min_, double &_max_)
{
	_Tx val;
//	_Tx *ptr = (_Tx*)src;
	for(int i = 0; i < cnt; i++)
	{
		val = src[i];
		if( val < _min_ ) _min_ = val;
		if( val > _max_ ) _max_ = val;
	}
}
