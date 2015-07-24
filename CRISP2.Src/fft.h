#pragma once

#include "complex\complex_tmpl.h"
#include "complex\ft2real.h"

#define	FF_LOG	0x001			// Logarithm control
#define	FF_RES	0x002			// Status line in 1/Angstrom

typedef struct {
	LPBYTE	src;				// IMG buffer ptr
	LPBYTE	img;				// FFT bitmap buffer ptr
	int		fs;					// FFT dimension (i.e. 128,256,...)
	SCROLLBAR	SB_Par;			// Parabolic control
	SCROLLBAR	SB_Lin;			// Linear control
	SCROLLBAR	SB_DC;			// Zero shift control
	SCROLLBAR	SB_Sc;			// Scale control
	DWORD	flag;
// 	int		pix;				// Pixel size in bytes
	PIXEL_FORMAT npix;			// Pixel format
	FT2R	ft2;				// FT2R element
	FT2R	ff2;				// FT2R element for filter
} FFT, * LPFFT;

#define	GetFFT(O)	((LPFFT)(O->dummy))

void	Fc2Sc( LPDOUBLE x, LPDOUBLE y, OBJ* O);

// One dimensional FFT's
// input 	data -> double  img[size]
//			size = fft size, size = 2**n, where n>5
// output	data -> complex fft[size/2];
void	WINAPI	ft1CalcReal( LPDOUBLE data, int size );

// input 	data -> complex fft[size/2]
//			size = fft size, size = 2**n, where n>5
// output	data -> double  img[size];
void	WINAPI	ft1CalcInverseReal( LPDOUBLE data, int size );

// input 	data -> complex img[size]
//			size = fft size, size = 2**n, where n>4
// output	data -> complex fft[size];
void	WINAPI	ft1CalcComplex( LPDOUBLE data, int size );

// input 	data -> complex fft[size]
//			size = fft size, size = 2**n, where n>4
// output	data -> complex img[size];
void	WINAPI	ft1CalcInverseComplex( LPDOUBLE data, int size );


// Old staff
#if 0
#define		FILTERSIZE	256		// Filter length
#define		CTFSIZE	(FILTERSIZE*3*4)

typedef	struct	{
#define	MAXELP		4
#define	CTF_Capt	0x0001
#define	CTF_On		0x0002
#define	CTF_Done	0x0004
#define	CTF_Tune	0x0008
#define	CTF_Chg		0x0010

	LPFLOAT fcsk;
	LPFLOAT	frad;
	int		f;
	int		ne;
	int		n;
	float	al;
	float	aw;
	float	ex;
	float	dl[MAXELP];
} CTF;
typedef	CTF * LPCTF;

/*---------- FFTSUB.C  --------*/
/*
extern	void	ExpandFFT (lpFFTCom S);
extern	BOOL	IFFT2     (lpFFTCom S);
extern	BOOL	FFT2      (lpFFTCom S);
extern	BOOL	GetAmpMap8(lpFFTCom S);
*/
#define FFTSIZE(xs) (xs*xs*4L)
#define COSSIZE(xs) (xs*4L)

#endif
