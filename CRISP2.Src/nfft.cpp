#include "StdAfx.h"

#include "commondef.h"

#pragma optimize ("agwty",on)

#include "Complex/complex_tmpl.h"
#define	__MEAT__
// #include "complex\complex.h"
#include "complex\ft2real.h"
#undef	__MEAT__

#include "ft32util.h"

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//- One dimensional FFT's ---------------------------------------------------
void	WINAPI	ft1CalcReal( LPDOUBLE data, int size )
{
	FFTD1R(data, 0, size, 1);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void	WINAPI	ft1CalcInverseReal( LPDOUBLE data, int size )
{
	IFFTD1R(data, 0, size, 1);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void	WINAPI	ft1CalcComplex( LPDOUBLE data, int size )
{
	FFTD1(data, 0, size, 1);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void	WINAPI	ft1CalcInverseComplex( LPDOUBLE data, int size )
{
	IFFTD1(data, 0, size, 1);
}
