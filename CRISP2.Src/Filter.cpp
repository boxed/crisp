/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.FFTfilter * by ML *************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"

#include "fft.h"
#include "lattice.h"

#include "HelperFn.h"

#pragma optimize ("",off)

#define	MRS	2				// Marker size on plot
//--------------------------------------------------------------------------
// CTF filter
//--------------------------------------------------------------------------
typedef struct  {
	BOOL	bOn;			// Filter is ON
#define MAXELP		4
#define CTF_Capt	0x0001
#define CTF_On		0x0002
#define CTF_Done	0x0004
#define CTF_Tune	0x0008
#define CTF_Chg		0x0010
	int		f;
	int		ne;
	int		n;
	double	al;
	double	aw;
	double	ex;
	double	dl[MAXELP];
	double	Df0, Df1;//, Azimuth;
	double	Amplif;			// Maximum value for CTF correction
	BOOL	bElliptic;		// TRUE: elliptic aproximation, FALSE: mathmatical
	BOOL	bShow;			// Show elliptical rings on FFT
	int		iXoverU;		// Where first crossover along U
	int		iXoverV;		// Where first crossover along V
		// CTF root
		#define	rZero		0
		#define	rPi			1
		#define	rMinusPi	2
} CTFILTER, *LPCTFILTER;

//--------------------------------------------------------------------------
// Radial filter
//--------------------------------------------------------------------------
#define	ST		11			// Number of steps for radial filter
#define	MAXF	4			// Maximum number of R curves

typedef struct {
	double	ac[ST];		// Amplitude curve values
	double	ax[ST];		// Amplitude curve arguments
	char	name[20];	// Filter name
} RCURVE;

typedef struct {			// Radial filter curve struct
	BOOL	bOn;			// Filter is ON
	int		cnum;			// Curve number 0..MAXF-1
	RCURVE	rc[MAXF];		// Curve data
	int		mx[ST];			// Drag marker X
	int		my[ST];			// Drag marker Y
	BOOL	df;				// Drag flag
	int		di;				// Drag index
} RFILTER, * LPRFILTER;

RCURVE	FLIN = {{  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0},	// Amp curv
				{  0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},	// Amp mrk
				"Linear Filter"};
RCURVE	FLOW = {{  1.0, 0.9, 0.8, 0.7, 0.6, 0.5, 0.4, 0.3, 0.2, 0.1, 0.0},	// Amp curv
				{  0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},	// Amp mrk
				"Low pass"};
RCURVE	FHIGH= {{  0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},	// Amp curv
				{  0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},	// Amp mrk
				"High pass"};
RCURVE	FMID = {{  0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 0.8, 0.6, 0.4, 0.2, 0.0},	// Amp curv
				{  0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0},	// Amp mrk
				"Middle pass"};

//--------------------------------------------------------------------------
// Lattice filter
//--------------------------------------------------------------------------
#define	DEFRADIUS	5.		// Default radius
#define	MAXRADIUS	16		// Default radius
#define	MAXLPOINTS	1024	// Maximum number of user points
typedef struct {			// Substract lattice (user points)
	BOOL	bOn;			// Filter is ON
	int		n;				// number of defined points
	double	weight;			// Value of Weight control
	int		h[MAXLPOINTS];	// X coord.
	int		k[MAXLPOINTS];	// Y coord.
	int		r[MAXLPOINTS];	// Radiuses
	double	wgt[MAXLPOINTS];// Relative weight

	double	ax, ay, bx, by; // Lattice parameters
	int		rad;			// Radius
	BOOL	bShow;			// Show filter on FFT
	BOOL	bLattice;		// Lattice filter is ON
	BOOL	bUser;			// User points mode is ON
	BOOL	bInverse;		// Inverse, i.e. leave spots and remove background
	BOOL	bSmooth;		// Smooth spots edges
} LFILTER, * LPLFILTER;

//--------------------------------------------------------------------------
static	char	szFltHead[] = "Filter32";

//--------------------------------------------------------------------------
// Item to plot
#define	iRadial	0
#define	iCTF	1
#define	i1_CTF	2
#define	iRDis	3

//--------------------------------------------------------------------------
#define	GetCTF(O)	((LPCTFILTER)O->dp)
#define	SetCTF(O,C)	O->dp=(LPBYTE)C
#define	GetRAF(O)	((LPRFILTER) O->bp)
#define	SetRAF(O,R)	O->bp=(LPBYTE)R
#define	GetLAF(O)	((LPLFILTER) O->ebp)
#define	SetLAF(O,L)	O->ebp=(LPBYTE)L

/****************************************************************************/
// u in volts
double	WaveLength(double u)
{
	double	h	=	6.626176e-34;
	double	me	=	9.109534e-31;
	double	c	=	2.99792458e8;
	double	e	=	1.6021892e-19;
	double	E, m, l;

	E = e*u*.5;
	m = me + E/(c*c);
	l = h*.5/sqrt(m*E);

	return l;	// return in m
}
/****************************************************************************/
double	DfValue( double Vacc, double Cs, double u, double recipx, int flag)
{
	double	ewl, u1;
	double	dff = 0.;

	u1 = u*recipx;		// Coordinates
	u1 = u1*u1;

	ewl = WaveLength( Vacc );	// e- wavelength (m)

	switch (flag)
	{
		case  rZero:	dff = -0.5*Cs*ewl*ewl*u1; break;
		case  rMinusPi:	dff = - (1. + 0.5*Cs*ewl*ewl*ewl*u1*u1) / (ewl*u1); break;
		case  rPi:		dff =   (1. - 0.5*Cs*ewl*ewl*ewl*u1*u1) / (ewl*u1); break;
	}
	return ceil(dff*1e10) * 1e-10;
}
/****************************************************************************/
// CTF= sin(pi * dfu * wl  * u*u  +  pi * dfv * wl * v*v + 1/2 * pi * Cs * wl*wl*wl(u*u+v*v))
//
double	CTFValue( double Vacc, double df0, double df1, double azimuth, double Cs, int u, int v,
				  double recipx, double dfs, double conv)
{
	double	ewl;
	double	ca = cos( azimuth );
	double	sa = sin( azimuth );
	double	u1, v1;
	double	ctf, ctfa;
	double	ed, ec, b;
	double	a;

//	u1 = (u*ca - v*sa)*recipx;	// Coordinates rotation from original to df0^df1
//	v1 = (u*sa + v*ca)*recipx;	// in reciprocal units
	u1 = u*recipx;		// Coordinates
	v1 = v*recipx;		// in reciprocal units

	ewl = WaveLength( Vacc );	// e- wavelength (m)

	u1 = SQR(u1);
	v1 = SQR(v1);		// only squares

	a = SQR(ewl*(u1+v1));
	ctfa = ewl*(  df0*u1 + df1*v1 + 0.5*Cs*a );
	if ( ctfa > 20.)
		ctfa = 0.;
	ctf = sin(M_PI * ctfa );
	ed = exp(-0.5*SQR(M_PI*dfs)*a);
	b = 0.;
	if (u1 + v1 > 1e-10)
		b = (df0*u1+df1*v1)/(u1+v1)+Cs*SQR(ewl)*(u1+v1);
	ec = exp(-SQR(M_PI*conv*b)*(u1+v1));
	ctf = ctf * ed * ec;
	return	ctf;
}
/****************************************************************************/
static	void	Add_CTF_Filter( FT2R& fft, int xs, OBJ* O, LPCTFILTER C )
{
int		y,x,xs2 = xs>>1,xx;
double	ewl = WaveLength( O->NI.Vacc );
double	recipx = 1./O->NI.Scale/xs;
double	_Azimuth = M_PI*.5 - C->aw; // C->Azimuth
double	ca = cos( _Azimuth )*recipx;
double	sa = sin( _Azimuth )*recipx;
double	u0, v0, u, v, du, dv;
double	ctf, ctfa;
double	ed, ec;
double	a, b, amp=1./C->Amplif;
double	df0=C->Df0, df1=C->Df1, Cs=O->NI.Cs;
double	dfs = O->NI.Def_Spread,	conv = O->NI.Conver;

#ifdef _DEBUG
#define		X_MIN_PRINT		26
#define		X_MAX_PRINT		58
#define		Y_MIN_PRINT		-19
#define		Y_MAX_PRINT		19
	FILE *pCTF;
	pCTF = fopen("C:\\Temp\\CTF.txt", "wt");
	if(pCTF)
	{
		fprintf(pCTF, "%8s", "Val");
		for(x = X_MIN_PRINT; x <= X_MAX_PRINT; x++)
			fprintf(pCTF, "%8d", x);
	}
#endif

	du = ca;			// Increments for u and v
	dv = sa;			// in reciprocal units
	for(y = -xs2; y < xs2; y++)
	{
#ifdef _DEBUG
		if(pCTF && y <= Y_MAX_PRINT && y >= Y_MIN_PRINT)
			fprintf(pCTF, "\n%8d", y);
#endif
		u = u0 = -y*sa;				// Coordinates rotation from original to df0^df1
		v = v0 =  y*ca;				// in reciprocal units
		for(x = 0; x < xs2; x++)
		{
			if (x == 0 && y < 0 )
				xx = xs2;
			else
				xx = x;
			u = SQR(u0);
			v = SQR(v0);		// only squares
			a = SQR(ewl*(u+v));
			ctfa = ewl*( df0*u + df1*v + 0.5*Cs*a );
			if ( ctfa < 20. && (x!=0 || y!=0) )		// Less then 20 periods of oscillations and not (0,0)
			{
				ctf = sin(M_PI * ctfa);
//				if (1)			// Envelope functions
//				{
				ed = exp(-0.5*SQR(M_PI*dfs)*a);
				b = 0.;
				if (u + v > 1e-10)
					b = (df0*u + df1*v)/(u+v)+Cs*SQR(ewl)*(u+v);
				ec = exp(-SQR(M_PI*conv*b)*(u+v));
				ctf = ctf * ed * ec;
//				}
				if (fabs(ctf) < amp)
				{
					if (ctf < 0.)
						a = C->Amplif;
					else
						a = -C->Amplif;
				}
				else
					a = -1. / ctf;
			}
			else
			{
				a = 0.;
			}
			u0 += du;
			v0 += dv;
			fft(xx,y) *= a;
#ifdef _DEBUG
			if(pCTF && x <= X_MAX_PRINT && x >= X_MIN_PRINT && y <= Y_MAX_PRINT && y >= Y_MIN_PRINT)
				fprintf(pCTF, "%8.3f", a);
#endif
		}
#ifdef _DEBUG
//		if(pCTF && y <= Y_MAX_PRINT && y >= Y_MIN_PRINT)
//			fprintf(pCTF, "\n");
#endif
	}
#ifdef _DEBUG
	if(pCTF)
		fclose(pCTF);
#endif
}
/****************************************************************************/
static	void	Add_CTFE_Filter( FT2R& fft, int xs, OBJ* O, LPCTFILTER C )
{
int		y,x,xs2 = xs >> 1,i,xx;
double	ca = cos( C->aw ), cb, cc;
double	sa = sin( C->aw ), sb, ss;
double	a, v, rr0, rr1, rx, rr;

	for(y=-xs2; y<xs2; y++)
	{
		for(x=0; x<xs2; x++)
		{
			if (x==0 && y<0 ) xx=xs2;
			else              xx=x;
			rr = xx*xx+y*y;
			rx = sqrt(rr);				// Radius of the point
			cb = xx/rx; sb = y/rx;		// sin(b), cos(b)
			ss = sa*cb-ca*sb;
			cc = ca*cb+sa*sb;
			for(i=1,a=0,rr0=0.,v=C->al;i<=C->ne;i++)
			{
				rr1 = v*v*C->ex*C->ex/(cc*cc*C->ex*C->ex+ss*ss);
				if ((rr>rr0) && (rr<=rr1)) { a=(((i-1)&1)*-2)+1; /*a=i*100000;*/ break; }
				rr0=rr1;
				if (i<C->ne) v+=C->dl[i];
			}
			fft(xx,y) *= a;
		}
	}
}
/****************************************************************************/
static	void	Add_Radial_Filter(FT2R& fft, int xs, LPRFILTER R)
{
int			y, x, xx, xs2=xs/2, x0, x1, i , j;
LPFLOAT		lpf = (LPFLOAT)calloc(sizeof(float), xs);
LPDOUBLE	ax = R->rc[R->cnum].ax;
LPDOUBLE	ac = R->rc[R->cnum].ac;
double		dx, dy, d0;

	if (lpf==NULL) return;

	for (i=0; i<ST-1; i++) {
		x0 = ax[i]*xs2;
		x1 = ax[i+1]*xs2;
		dx = x1-x0;
		if (dx == 0.) continue;
		dy = (ac[i+1]-ac[i])/dx;
		d0 = ac[i];
		for (j=x0; j<x1; j++) {
			lpf[j] = d0+dy*(j-x0);
            if(fabs(lpf[j]) > 1.) lpf[j] /= fabs(lpf[j]);
		}
	}

	for(y=-xs2; y<xs2; y++) {
		for(x=0; x<xs2; x++) {
			if (x==0 && y==0) continue;
			if (x==0 && y<0 ) xx=xs2;
			else              xx=x;
			i = __min(xs2-1,round(sqrt((double)(xx*xx+y*y))));
			fft(xx,y) *= lpf[i];
		}
	}
	free( lpf );
}
/****************************************************************************/
static	void	Add_Lattice_Filter(FT2R& fft, int xs, LPLFILTER L)
{
	int		_H, _K;
	int		h, k, r;
	int		h0, k0, h1, k1, i, rr, min_h, min_k, max_h, max_k;
	int		xs2 = xs >> 1;
	double	ax, ay, bx, by, wgt;
	static	double	tbl[MAXRADIUS];

	for(i = 0; i < MAXRADIUS; i++)
	{
		if (L->bSmooth)
			tbl[i] = 1. - cos(M_PI/30.*i);
		else
			tbl[i] = 0;
	}

    if (L->bInverse)
	{
		for(i = 0; i < MAXRADIUS; i++)
			tbl[i] = 1. - tbl[i];
	}
	// added by Peter on 11 Sep 2001
	// calculate __min and max values for h and k
	min_h = -20/*0*/;
	min_k = -20;
	max_h = 20;
	max_k = 20;
	// lattice vectors
	ax = L->ax;
	ay = L->ay;
	bx = L->bx;
	by = L->by;
	max_h = round(xs2 * sqrt(2 / (ax*ax + ay*ay)));
	max_k = round(xs2 * sqrt(2 / (bx*bx + by*by)));
	min_k = -max_k;
	min_h = -max_h;

	if (L->bLattice)
	{
// Lattice subtraction
		r  = L->rad;
//		wgt = L->weight * 0.01;
		for(h1 = min_h; h1 <= max_h; h1++)
		{
			for(k1 = min_k; k1 < max_k; k1++)
			{
				h0 = round(ax*h1 + bx*k1);
				k0 = round(ay*h1 + by*k1);
				if ((abs(h0-r) > xs2) && (abs(h0+r) > xs2))
				{
					continue;	// out of FFT
				}
				if ((abs(k0-r) > xs2) && (abs(k0+r) > xs2))
				{
					continue;	// out of FFT
				}
				for(h = -r; h <= r; h++)
				{
					for(k = -r; k <= r; k++)
					{
						rr = round((double)sqrt((double)h*h + k*k));
						if (rr < r)
						{
							if (L->bInverse)
							{
								// CHANGED BY PETER ON 02 NOV 2005,
								// the class operator (h,k) IS BUGGY!!!
//*
								_H = h0+h;
								_K = k0+k;
								if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
								{
									if( _K < 0 ) _K = xs + _K;
									fft.BX.P[(_H + xs2*_K)<<1] = tbl[rr*MAXRADIUS/r];
								}
								_H = -_H;
								_K = -_K;
								if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
								{
									if( _K < 0 ) _K = xs + _K;
									fft.BX.P[(_H + xs2*_K)<<1] = tbl[rr*MAXRADIUS/r];
								}
/*/
								fft(  h0+h,  k0+k ) = fft( -h0-h, -k0-k ) = tbl[rr*MAXRADIUS/r];
								// weights added by Peter on 28 Aug 2001
//								fft(  h0+h,  k0+k ) = tbl[rr*MAXRADIUS/r];
//								fft( -h0-h, -k0-k ) = tbl[rr*MAXRADIUS/r];
//*/
							}
							else
							{
								_H = h0+h;
								_K = k0+k;
								if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
								{
									if( _K < 0 ) _K = xs + _K;
									fft.BX.P[(_H + xs2*_K)<<1] = 0;
								}
								_H = -_H;
								_K = -_K;
								if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
								{
									if( _K < 0 ) _K = xs + _K;
									fft.BX.P[(_H + xs2*_K)<<1] = 0;
								}
/*
								fft(  h0+h,  k0+k ) = 0;
								fft( -h0-h, -k0-k ) = 0;
*/
							}
						}
					}
				}
			}
		}
	}
	// User defined points
	if (L->bUser)
	{
		for(i = 0; i < L->n; i++)
		{
			r  = L->r[i];
			h0 = L->h[i];
			k0 = L->k[i];
			wgt = L->wgt[i];
			if ((abs(h0-r) > xs2) && (abs(h0+r) > xs2)) continue;	// out of FFT
			if ((abs(k0-r) > xs2) && (abs(k0+r) > xs2)) continue;	// out of FFT
			for(h = -r; h <= r; h++)
			{
				for(k = -r; k <= r; k++)
				{
					rr = round((double)sqrt((double)h*h+k*k));
					if (rr < r)
					{
						if (L->bInverse)
						{
							_H = h0+h;
							_K = k0+k;
							if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
							{
								if( _K < 0 ) _K = xs + _K;
								fft.BX.P[(_H + xs2*_K)<<1] = tbl[rr*MAXRADIUS/r]*wgt;
							}
							_H = -_H;
							_K = -_K;
							if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
							{
								if( _K < 0 ) _K = xs + _K;
								fft.BX.P[(_H + xs2*_K)<<1] = tbl[rr*MAXRADIUS/r]*wgt;
							}
/*
							fft(  h0+h,  k0+k ) = tbl[rr*MAXRADIUS/r]*wgt;
							fft( -h0-h, -k0-k ) = tbl[rr*MAXRADIUS/r]*wgt;
*/
						}
						else
						{
							_H = h0+h;
							_K = k0+k;
							if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
							{
								if( _K < 0 ) _K = xs + _K;
								fft.BX.P[(_H + xs2*_K)<<1] *= wgt;
							}
							_H = -_H;
							_K = -_K;
							if( _H >= 0 && _H < xs2 && _K >= -xs2 && _K < xs2 )
							{
								if( _K < 0 ) _K = xs + _K;
								fft.BX.P[(_H + xs2*_K)<<1] *= wgt;
							}
/*
							fft(  h0+h,  k0+k ) *= wgt;//tbl[rr*MAXRADIUS/r]*wgt;
							fft( -h0-h, -k0-k ) *= wgt;//tbl[rr*MAXRADIUS/r]*wgt;
*/
						}
					}
				}
			}
		}
	}
}
//*/
/****************************************************************************/
/*
{
	BM8		bmdst(Spr->fs,Spr->fs,Spr->img);
	Spr->ft2.DisplayFFT(bmdst);
	InvalidateRect(O->ParentWnd,NULL,FALSE);
}
*/
/****************************************************************************/
static	BOOL	Filter( OBJ* O )
{
	LPFFT		S = GetFFT(O);
	OBJ*		pr  = OBJ::GetOBJ(O->ParentWnd);	// The parent should be FFT type
	LPFFT		Spr = GetFFT(pr);
	HCURSOR		hoc;
	LPCTFILTER	C = GetCTF(O);
	LPRFILTER	R = GetRAF(O);
	LPLFILTER	L = GetLAF(O);
	int			xs = S->fs;
	int			xs2 = xs >> 1, i;
	LPFLOAT		lpf;
	BOOL		bMul = FALSE;
	float		v1 = 0.0;

	O->NI = pr->NI;		// Project Info structure

	tobSetInfoText( ID_INFO, "Filter");
	hoc = SetCursor( hcWait );

//	Init filter map, in case of Inverse&Lattice fill with Re=0., otherwise Re=1.
	v1 = ( L->bOn && L->bInverse ) ? 0.0 : 1.0;
	lpf = S->ft2.BX.P;
	for(i = 0; i < xs2 * xs; i++)
	{
		*lpf++ = v1;
		*lpf++ = 0.0;
	}
/*
	if ( L->bOn && L->bInverse )
	{
		lpf = S->ft2.BX.P;
		for(i=0;i<xs2*xs;i++)
		{
			*lpf++=0.0;
			*lpf++=0.0;
		}
	}
	else
	{
		lpf = S->ft2.BX.P;
		for(i=0;i<xs2*xs;i++)
		{
			*lpf++=1.0;
			*lpf++=0.0;
		}
	}
*/
	// Lattice subtraction filter preparation
	if ( L->bOn )
	{
		Add_Lattice_Filter(S->ft2, xs, L);
		bMul = TRUE;
	}

	// Radial filter
	if ( R->bOn )
	{
		Add_Radial_Filter( S->ft2, xs, R );
		bMul = TRUE;
	}

	// CTF filter preparation
	if ( C->bOn )
	{
		if (C->bElliptic)
		{
			Add_CTFE_Filter( S->ft2, xs, O, C );
		}
		else
		{
			Add_CTF_Filter ( S->ft2, xs, O, C );
		}
		bMul = TRUE;
	}
/*
	// DEBUG THE MASK
	FILE *pFile = fopen("d:\\latmask.txt", "wt");
//	FCOMPLEX *f = S->ft2.BX.P;
	FCOMPLEX *f = (FCOMPLEX*)S->ft2.BX.P;
	if( pFile )
	{
		for(int k = -S->ft2.S/2; k < S->ft2.S/2; k++)
//		for(int k = 0; k < S->ft2.S/2; k++)
		{
//			for(int h = -S->ft2.S/2; h < S->ft2.S/2; h++)
			for(int h = 0; h < S->ft2.S/2; h++, f++)
			{
//				FCOMPLEX &f = S->ft2(h, k);
//				fprintf(pFile, "%12.3f\t", sqrt(f.i*f.i + f.j*f.j));
				fprintf(pFile, "%12.3f\t", sqrt(f->i*f->i + f->j*f->j));
			}
//			if( k == S->ft2.S*(k/S->ft2.S) && k != 0 )
			{
				fprintf(pFile, "\n");
			}
		}
		fclose(pFile);
	}
*/
	if ( bMul )	S->ft2 *= Spr->ft2;	// Multiply by mask image
	else	    S->ft2  = Spr->ft2;	// Just copy it

	SetCursor( hoc );
	return	TRUE;
}
/****************************************************************************/
static	double	fpCalculateRDis( FT2R& fft, LPDOUBLE data, double alfa, int xs )
{
int		siz2 = xs >> 1;
double	cw, sw, amp;
int		h, k, hh, kk, r;
register int i;
int		* cnt = (int *)(data+xs);

#define	DK	3
#define	DH	1

	for (i=0; i<siz2; i++)
	{
		data[i] = 0.;
		cnt[i]  = 0;
	}

	cw = cos(alfa);	sw = sin(alfa);

	for (h=DH; h<siz2-DH; h++) for (k=-DK; k<=DK; k++) {
		hh = round(h*cw - k*sw);
		kk = round(h*sw + k*cw);
		r = round(sqrt((double)hh*hh+kk*kk));
		amp = fft.GetAmp(hh, kk);
		for(i=r-DH; i<=r+DH; i++ )
		{
			data[i] += amp;
			cnt[i]  += 1;
		}
	}
	amp = 0.;
	for(i=0;i<siz2;i++) if (cnt[i] != 0) {
		data[i] /= (double)cnt[i];
		if( amp < data[i] ) amp = data[i];
	}

	return	amp;
}
/****************************************************************************/
static	void	PaintFilter( LPVOID param )
{
	OBJ*	O = (OBJ*) param;
	HWND	hDlg = O->hWnd;
	HWND 	hw;
	HDC 	shDC, hDC;
	HBITMAP hbmp;
	HFONT	hof;
	int 	i, xs, ys;
	int		xsl, ysl;
	RECT 	r;
	int		x, y;
	LPRFILTER	R = GetRAF(O);
	LPFFT		S = GetFFT(OBJ::GetOBJ(O->ParentWnd));
	LPCTFILTER	C = GetCTF(O);
	HBRUSH	hob;

	double  *mx, *my;
	int		siz2 = S->fs>>1;	// FFT size /2
	double	inv_siz2 = 1. / (double)siz2;
	double	dd;
	int		item = SendDlgItemMessage( hDlg, IDC_FI_ITEMTOPLOT, CB_GETCURSEL, 0, 0);

	if (IsDlgButtonChecked( hDlg, IDC_FI_BIGPLOT) ) hw = O->hBigPlot;
	else											hw = GetDlgItem( hDlg, IDC_FI_PLOT);

	if (hw==NULL) return;

	shDC = GetDC( hw );
	hDC = CreateCompatibleDC( shDC );

	GetClientRect( hw, &r );
	xs = r.right - r.left;
	ys = r.bottom - r.top;
    xsl = xs - MRS*2;
	ysl = ys - MRS*2;
	hbmp = CreateCompatibleBitmap( shDC, xs, ys);
	SelectObject( hDC, hbmp );

	hob = SelectBrush( hDC, hPlotBrush );
	PatBlt( hDC, 0, 0, xs, ys, PATCOPY);

	SetROP2( hDC, R2_COPYPEN );

	// Grid
	SelectPen( hDC, hPlotPen );
	SetTextColor( hDC, rgbPlotText);
	SetBkMode( hDC, TRANSPARENT );
#ifdef USE_FONT_MANAGER
	int nFntSizeW, nFntSizeH;
	HFONT hFont = FontManager::GetFont(FM_SMALL_FONT, nFntSizeW, nFntSizeH);
#else
	int nFntSizeH = fnts_h;
	HFONT hFont = hfSmall;
#endif
	hof = SelectFont( hDC, hFont );
	_MoveTo( hDC, 0, MRS );                LineTo( hDC, xs, MRS);
	_MoveTo( hDC, 0, MRS+(ysl+1)/2 ); LineTo( hDC, xs, MRS+(ysl+1)/2);
	_MoveTo( hDC, 0, MRS+ysl );       LineTo( hDC, xs, MRS+ysl);
	for(i=0;i<ST;i++) {
		x = (xsl*i)/(ST-1)+MRS+1;
		_MoveTo( hDC, x, 0 ); LineTo( hDC, x, ys );
		sprintf( TMP, "%-4.2f", (double)i/(double)(ST-1)*.5/(O->NI.Scale*1e10));
		if (i&1) TextOut(hDC, x+1, MRS,           TMP, strlen(TMP));
		else     TextOut(hDC, x+1, ys-MRS-nFntSizeH, TMP, strlen(TMP));
	}
	SelectObject( hDC, hof );

	if (item == iRadial)
	{
		// Drawing Amplitude curve
		SelectObject( hDC, RedPen );
		for(i = 0;i < ST; i++)
		{
			x = (      xsl* R->rc[R->cnum].ax[i])+MRS;
//			y = (ysl - ysl*(R->rc[R->cnum].ac[i]+1.)*.5)+MRS;
			y = (ysl - ysl*(R->rc[R->cnum].ac[i]+1.)*.5)+MRS;
			if (i==0) _MoveTo( hDC, x, y );
			else	  LineTo( hDC, x, y );
		}
		// Drawing edit markers for Amplitude curve
		mx = R->rc[R->cnum].ax;
		my = R->rc[R->cnum].ac;
		SelectObject( hDC, RedPen );

		for(i = 0; i < ST; i++)
		{
			x =		  xsl* mx[i] + MRS;
//			y = ysl - ysl*(my[i] + 1.)*.5 + MRS;
			y = ysl - ysl*(my[i] + 1.)*.5 + MRS;
			Rectangle ( hDC, x-MRS, y-MRS, x+MRS+1, y+MRS+1 );
			R->mx[i] = x;
			R->my[i] = y;			// Save marker coord for draging
		}
	}
	if ( (item==iCTF) || (item == i1_CTF))
	{
		// Drawing u curve
		SelectObject( hDC, RedPen );
		for(i = 0;i < siz2; i++)
		{
			x = xsl*i/siz2 + MRS;
			// modified by Peter on 19 Sep 2001
//			dd = CTFValue(O->NI.Vacc,C->Df0,C->Df1,C->Azimuth,O->NI.Cs,i,0,1./O->NI.Scale*.5/(double)siz2,O->NI.Def_Spread,O->NI.Conver);
			dd = CTFValue(O->NI.Vacc,C->Df0,C->Df1,C->aw,O->NI.Cs,i,0,.5*inv_siz2/O->NI.Scale,O->NI.Def_Spread,O->NI.Conver);
			if (item == i1_CTF)
			{		// !!!
				if (fabs(dd) < 1./C->Amplif) dd = (dd<0.) ? 0. : 1.;
				else					     dd = (1./dd/C->Amplif+1.)*.5;
			} else						     dd = (dd+1.)*.5;
//				else					     dd = (1./dd/C->Amplif+1.)*.5;
//			} else						     dd = (dd+1.)*.5;
			y = (int)(ysl-ysl * dd +MRS);
			if (i==0) _MoveTo( hDC, x, y );
			else	  LineTo( hDC, x, y );
		}
		// Drawing v curve
		SelectObject( hDC, BluePen );
		for(i = 0; i < siz2; i++)
		{
			x = xsl*i/siz2 + MRS;
			// modified by Peter on 19 Sep 2001
//			dd = CTFValue(O->NI.Vacc,C->Df0,C->Df1,C->Azimuth,O->NI.Cs,0,i,1./O->NI.Scale*.5/(double)siz2,O->NI.Def_Spread,O->NI.Conver);
			dd = CTFValue(O->NI.Vacc,C->Df0,C->Df1,C->aw,O->NI.Cs,0,i,.5*inv_siz2/O->NI.Scale,O->NI.Def_Spread,O->NI.Conver);
			if (item == i1_CTF)
			{		// !!!
				if (fabs(dd) < 1./C->Amplif) dd = (dd<0.) ? 0. : 1.;
				else					     dd = (1./dd/C->Amplif+1)*.5;
			} else						     dd = (dd+1.)*.5;
//				else					     dd = (1./dd/C->Amplif+1)*.5;
//			} else						     dd = (dd+1.)*.5;
			y = (int)(ysl-ysl * dd +MRS);
			if (i==0) _MoveTo( hDC, x, y );
			else	  LineTo( hDC, x, y );
		}
	}
	if (item == iRDis) {
		LPDOUBLE rdis = (LPDOUBLE)malloc((sizeof(double)+sizeof(int))*4096);
		double	maxval;

		SelectObject( hDC, GreenPen );
		maxval = fpCalculateRDis( S->ft2, rdis, C->aw, S->fs );
		for(i=0;i<siz2;i++)
		{
			x = xsl*i/siz2 + MRS;
			y = ysl-ysl * rdis[i]/maxval +MRS;
			if (i==0) _MoveTo( hDC, x, y );
			else	  LineTo( hDC, x, y );
		}
		SelectObject( hDC, RedPen );
		maxval = C->al;
		for(i=0;i<C->ne;i++)
		{
			maxval += C->dl[i];
			x = (int)(xsl*maxval/(double)siz2)+MRS;
			_MoveTo( hDC, x, MRS );
			LineTo( hDC, x, MRS+ysl );
		}
		free(rdis);
	}

	BitBlt( shDC, 0, 0, xs, ys, hDC, 1, 1, SRCCOPY);
	SelectBrush( hDC, hob );
	DeleteDC( hDC );
	DeleteObject( hbmp );

	ReleaseDC( hw, shDC );
	if (IsDlgButtonChecked( hDlg, IDC_FI_BIGPLOT) )
	{
		ValidateRect( hw, NULL );
	}
	else
	{
		GetWindowRect( hw, &r );	// To validate window
		InflateRect( &r, -1, -1);
		MapWindowPoints( NULL, hDlg, (LPPOINT)&r, 2);
		ValidateRect( hDlg, &r);
	}
}
/****************************************************************************/
static	void	PrintRCurveXY( HWND hDlg, OBJ* O )
{
	LPRFILTER R = GetRAF(O);
	int		 i = R->di;

// Amplitude
	TextToDlgItemW(hDlg, IDC_FI_INFO, "R=%.2f 1/%c   A=%d%%",
		(double)R->rc[R->cnum].ax[i]*.5/(O->NI.Scale*1e10),
		ANGSTR_CHAR, (int)(R->rc[R->cnum].ac[i]*100));
//	sprintf(TMP, "R=%.2f 1/A   A=%d%%", (double)R->rc[R->cnum].ax[i]*.5/(O->NI.Scale*1e10),
//										(int)(R->rc[R->cnum].ac[i]*100));
//	SetDlgItemText( hDlg, IDC_FI_INFO, TMP);
}
/****************************************************************************/
static	void	PromptResolution( HWND hDlg, OBJ* O, LPARAM lParam)
{
POINT	p = {LOWORD(lParam),HIWORD(lParam)};
RECT	r;
HWND	hw = GetDlgItem( hDlg, IDC_FI_PLOT);
int		xs, x;

    MapWindowPoints(hDlg, hw, &p, 1);

	GetClientRect( hw, &r );
	InflateRect( &r, -MRS, -MRS);
	if ( ! PtInRect(&r, p) ) return;
	xs = r.right - r.left;
	x = p.x - r.left;

	TextToDlgItemW(hDlg, IDC_FI_INFO, "R=%.2f 1/%c",
		(double)x/(double)xs*.5/(O->NI.Scale*1e10), ANGSTR_CHAR);
//	sprintf(TMP, "R=%.2f 1/Angstrom", (double)x/(double)xs*.5/(O->NI.Scale*1e10));
//	SetDlgItemText( hDlg, IDC_FI_INFO, TMP);
}
/****************************************************************************/
static	BOOL	TestClickRCurve( HWND hDlg, OBJ* O, LPARAM lP )
{
LPRFILTER R = GetRAF(O);
int		i, x, y;
POINT	p = {LOWORD(lP),HIWORD(lP)}, p1;
RECT	r;
HWND	hw = GetDlgItem( hDlg, IDC_FI_PLOT);
int		xs, ys;

	if ( ! R->bOn ) return FALSE;
	MapWindowPoints( hDlg, hw, &p, 1);
	x = p.x; y = p.y;
	for( i=0; i<ST; i++)
		if (  (R->mx[i]-(MRS+3) < x) && (R->mx[i]+(MRS+3) > x)
		   && (R->my[i]-(MRS+3) < y) && (R->my[i]+(MRS+3) > y) ) break;

	if (i == ST)	return FALSE;

	// Save index
	R->di = i;

    p.x = R->mx[i]; p.y = R->my[i];
    MapWindowPoints(hw, NULL, &p, 1);
    SetCursorPos( p.x, p.y);

	GetClientRect( hw, &r );
	xs = r.right - r.left;
	ys = r.bottom - r.top;

	if      (i==0)   { p.x = p1.x = MRS; }
	else if (i==ST-1){ p.x = p1.x = xs-MRS; }
	else			 { p.x = R->mx[i-1]; p1.x = R->mx[i+1]; }
	p.y = MRS; p1.y = ys-MRS;

	MapWindowPoints(hw, NULL, &p, 1); MapWindowPoints(hw, NULL, &p1, 1);
	if (p.x>p1.x) { x=p.x; p.x=p1.x; p1.x=x; }
	if (p.y>p1.y) { y=p.y; p.y=p1.y; p1.y=y; }
	SetRect( &r, p.x, p.y, p1.x+1, p1.y+1);
	ClipCursor( &r );

	PrintRCurveXY( hDlg, O );
	return TRUE;
}
/****************************************************************************/
static	void	DragRCurve( HWND hDlg, OBJ* O, LPARAM lP )
{
LPRFILTER R = GetRAF(O);
POINT	p = {LOWORD(lP), HIWORD(lP)};
HWND	hw = GetDlgItem( hDlg, IDC_FI_PLOT);
RECT	r;
int		xs, ys;

	if ( ! R->bOn ) return;

	MapWindowPoints( hDlg, hw, &p, 1);
	GetClientRect( hw, &r );
	xs = r.right - r.left - MRS*2;
	ys = r.bottom - r.top - MRS*2;

	p.y = p.y-MRS;
	p.x = p.x-MRS;

	R->rc[R->cnum].ax[R->di] = (double)     p.x  / (double)xs;
	R->rc[R->cnum].ac[R->di] = ((double)(ys-p.y) / (double)ys)*2.-1.;

	PrintRCurveXY( hDlg, O );
}
/****************************************************************************/
static LRESULT ChangeItemColors( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC		hdc = (HDC)wParam;
	HWND	hwnd = (HWND)lParam;
	int		idc = GetDlgCtrlID(hwnd);
	LRESULT	ret;
	COLORREF	col  = RGB(0,0,0);
//	DWORD		bcol = GetSysColor(COLOR_WINDOW);
	static	BOOL brec = FALSE;
	BOOL	bApply = TRUE;

	if (brec)
		return FALSE;
	brec = TRUE;
	ret = DefDlgProc(hDlg, msg, wParam, lParam);
	switch ( idc )
	{
	case IDC_FI_C_AZIMUTH_TXT:
	case IDC_FI_C_ROOT1_TXT:
	case IDC_FI_C_DEFOCUS0:
	case IDC_FI_C_DEF0TXT:
		col = RGB(255,0,0);
//		bcol = RGB(192,192,192);
		break;
	case IDC_FI_C_ROOT2_TXT:
	case IDC_FI_C_DEFOCUS1:
	case IDC_FI_C_DEF1TXT:
		col = RGB(0,0,255);
//		bcol = RGB(192,192,192);
		break;
	default: bApply = FALSE; break;//goto xxx;
	}
	if(bApply)
	{
		SetTextColor(hdc, col);
	}
//	SetBkColor(hdc, bcol);
//xxx:
	brec = FALSE;
	return	ret;
}

/****************************************************************************/
static	void	DrawCTFEllipse( OBJ* O )
{
LPFFT F = GetFFT(O);
LPCTFILTER	 C = GetCTF(O);
double	a, b, ax, ay, bx, by, cw0, sw0, cw, sw, c, s, x, y;
double	sc, sx, sy, rx, ry;
int		i,n, orop;
#define NP 64
POINT	p[NP+1];
HDC		hDC;
OBJ*	pr;
HWND	hWndFFT;
HPEN	hop;
HBRUSH	hob;

// Searching back for FFT object
	hWndFFT = objFindParent( O, OT_FFT );
	pr = OBJ::GetOBJ( hWndFFT );

	hDC = GetDC( hWndFFT );

	orop = SetROP2( hDC, R2_COPYPEN );

	hob = SelectBrush( hDC, GetStockObject(NULL_BRUSH));
	hop = SelectPen( hDC, RedPen );
	c = 2.*M_PI/NP; s = sin(c); c = cos(c);
	cw0 = cos(C->aw);	sw0 = sin(C->aw);
	a = F->fs/1.4;
	x = a*cw0;	y = a*sw0;	Fc2Sc( &x, &y, pr );
	_MoveTo(hDC, round(x), round(y));
	x =-a*cw0;	y =-a*sw0;	Fc2Sc( &x, &y, pr );
	LineTo(hDC, round(x), round(y));

	x =-a*sw0;	y = a*cw0;	Fc2Sc( &x, &y, pr );
	_MoveTo(hDC, round(x), round(y));
	x = a*sw0;	y =-a*cw0;	Fc2Sc( &x, &y, pr );
	LineTo(hDC, round(x), round(y));

	sc = (double)pr->scu / (double)pr->scd;
	sx = F->fs*.5 - pr->xo;	sy = F->fs*.5 - pr->yo;
	rx = pr->scu*.5;		ry = pr->scu*.5;
	rx += 0.5;	ry += 0.5;
	sx *= sc;	sy *= sc;
	rx += sx;	ry += sy;

	a = C->al;
	for(n=0; n<C->ne; n++) {
		a += C->dl[n];	ax = a*cw0;	ay = a*sw0;
		b  = a*C->ex;	bx = b*sw0; by = b*cw0;
		cw = sc; sw = 0.;
		for(i=0; i<=NP; i++) {
			x = rx + ax*cw - bx*sw;
			y = ry + ay*cw + by*sw;
			p[i].x = (int)x;	p[i].y = (int)y;
			x = cw*c - sw*s; sw = cw*s + sw*c; cw = x;
		}
		Polyline(hDC, p, NP+1);
	}
	SetROP2( hDC, orop );
	SelectBrush( hDC, hob );
	SelectPen( hDC, hop );
	ReleaseDC( hWndFFT, hDC );
}
/****************************************************************************/
static	void	DrawLattFilter( OBJ* O )
{
OBJ*		pr;
HWND		hWndFFT;
HDC			hDC;
double		hh, kk;
int			orop;
register	int		h, k;
int			xs2, h0, k0, h1, k1, i, r, min_h, max_h, min_k, max_k;
double		ax, ay, bx, by;
LPLFILTER	S = GetLAF(O);

// Searching back for FFT object
	hWndFFT = objFindParent( O, OT_FFT );
	pr = OBJ::GetOBJ( hWndFFT );

	hDC = GetDC( hWndFFT );

	orop = SetROP2( hDC, R2_BLACK );
	if (S->bLattice)
	{
// Lattice subtraction
		ax = S->ax;
		ay = S->ay;
		bx = S->bx;
		by = S->by;
		xs2 = GetFFT(O)->fs>>1;
		min_h = -20;
		min_k = -20;
		max_h = 20;
		max_k = 20;
		max_h = round(xs2 * sqrt(2 / (ax*ax + ay*ay)));
		max_k = round(xs2 * sqrt(2 / (bx*bx + by*by)));
		min_k = -max_k;
		min_h = -max_h;
		if( ax == 0. && ay == 0. && bx == 0. && by == 0.)
			goto user;
		r = (int)(S->rad * (double)pr->scu / (double)pr->scd);
		for(h = min_h; h < max_h; h++)
		{
			for(k = min_k; k < max_k; k++)
			{
				hh = ax*h + bx*k;
				kk = ay*h + by*k;
//				if ((abs(h0-r) > xs2) && (abs(h0+r) > xs2)) continue;	// out of FFT
//				if ((abs(k0-r) > xs2) && (abs(k0+r) > xs2)) continue;	// out of FFT
				Fc2Sc( &hh, &kk, pr );
				h0 = round(hh-r); k0 = round(kk-r);
				h1 = round(hh+r); k1 = round(kk+r);
				Ellipse( hDC, h0, k0, h1, k1 );
			}
		}
	}
user:
	if (S->bUser)
	 for(i=0;i<S->n;i++) {
// User defined points --------------------------------------------------------------
		hh = S->h[i]; kk = S->k[i]; r = (int)(S->r[i] * (double)pr->scu / (double)pr->scd);
		Fc2Sc( &hh, &kk, pr );
		h0 = round(hh-r); k0 = round(kk-r);
		h1 = round(hh+r); k1 = round(kk+r);
		Ellipse( hDC, h0, k0, h1, k1 );

		hh = -S->h[i]; kk = -S->k[i];
		Fc2Sc( &hh, &kk, pr );
		h0 = round(hh-r); k0 = round(kk-r);
		h1 = round(hh+r); k1 = round(kk+r);
		Ellipse( hDC, h0, k0, h1, k1 );
	}
	SetROP2( hDC, orop );
	ReleaseDC( hWndFFT, hDC );
}
/****************************************************************************/
static	void	EstimateDfFromEllipse( HWND hDlg, OBJ* O )
{
	LPFFT	S = GetFFT(O);
	LPCTFILTER	C = GetCTF(O);

//	double dAzimuth = 90.0 - R2D(C->Azimuth);
	double dAzimuth = (90.0 - R2D(C->aw));

	// added by Peter on 23 Aug 2001
	while( dAzimuth < 0.0 )
		dAzimuth += 180.0;
	while( dAzimuth > 180.0 )
		dAzimuth -= 180.0;

	C->Df0 = DfValue(O->NI.Vacc, O->NI.Cs, C->al*C->ex, 1./O->NI.Scale/(double)S->fs, C->iXoverU );
	C->Df1 = DfValue(O->NI.Vacc, O->NI.Cs, C->al,       1./O->NI.Scale/(double)S->fs, C->iXoverV );
//	C->Df1 = DfValue(O->NI.Vacc, O->NI.Cs, C->al,       1./O->NI.Scale/(double)S->fs, C->iXoverU );
	SetDlgItemDouble( hDlg, IDC_FI_C_AZIMUTH,  dAzimuth, 0 );

	SetDlgItemDouble( hDlg, IDC_FI_C_DEFOCUS0, C->Df0*1e10,0 );
	SetDlgItemDouble( hDlg, IDC_FI_C_DEFOCUS1, C->Df1*1e10,0 );
}
/****************************************************************************/
static	BOOL	Set_EP(HWND hDlg, OBJ* O, LPARAM lParam)
{
LPFFT	S = GetFFT(O);
LPCTFILTER	C = GetCTF(O);
int 	i, n = C->n;
double	a, am, x, y, w, dw, l;
double	h = (double)(LOINT(lParam));
double	k = (double)(HIINT(lParam));

	if (! C->bShow ) return FALSE;
	if(n<0) return TRUE;
	if(fabs(h) > S->fs*.5) return TRUE;
	if(fabs(k) > S->fs*.5) return TRUE;
	l = sqrt(h*h+k*k);
	w = ATAN2(k,h);
	if (l < 8.) return TRUE;	// It was  l<(S->fs/16.)
	x = h*cos(C->aw) + k*sin(C->aw);
	y = k*cos(C->aw) - h*sin(C->aw);
	y /= C->ex;	x = sqrt(x*x+y*y);

	a = C->al; am = 1000.;
	if((C->f & CTF_Capt) == 0) {
		for(i=0; i<C->ne; i++) {
			a += C->dl[i];
			y = fabs(a-x); 	if(y<am) { am = y; n = i;}
		}
		C->f |= CTF_Capt; C->n = n;
	}
	if(n==0) {
		dw = fabs(w - C->aw); if(dw > M_PI*.5) dw = M_PI - dw;
		if(fabs(dw-M_PI/4.) < M_PI/8.) C->al *= x/C->al;
		else
			if(dw<M_PI/4.)	{ C->ex *= C->al / l;	C->al = l;	C->aw = w;}
			else			{ C->ex = l/C->al;		C->aw = w+M_PI*.5;}
		if(C->ex > 1.)	{ C->al *= C->ex; C->ex = 1./ C->ex; C->aw += M_PI*.5;}
		EstimateDfFromEllipse( hDlg, O );
	} else {
		a = C->al;	for(i=0; i<n; i++) a += C->dl[i];
		x -= a;
		if(x < 4.) x = 4.;
		if(fabs(C->dl[n] - x) < 0.01) return TRUE;
		C->dl[n] = x;
	}

	PaintFilter( O );
	InvalidateRect(objFindParent( O, OT_FFT ), NULL, FALSE);
	UpdateWindow(objFindParent( O, OT_FFT ));
	return TRUE;
}
/****************************************************************************/
static	void	PrepareRadial(OBJ* O, LPRFILTER R, LPFFT S, int siz2, double *fr )
{
double	dx, dy, d0;
int		i,j;
int		x[ST];

	for (i=0; i<siz2; i++) fr[i] = 1.;
// Amplitude part
	for (i=0; i<ST; i++) {
		x[i] = ((R->rc[R->cnum].ax[i]) * siz2);
	}
	for (i=0; i<ST-1; i++) {
		dx = (x[i+1]-x[i]);
		if (dx == 0.) continue;
		dy = (R->rc[R->cnum].ac[i+1]-R->rc[R->cnum].ac[i])/dx;
		d0 = R->rc[R->cnum].ac[i];
		for (j=x[i]; j<x[i+1]; j++) {
			fr[j] = d0+dy*(j-x[i]);
            if(fabs(fr[j]) > 1.)
            	fr[j] /= fabs(fr[j]);
		}
	}
}
/****************************************************************************/
static	void	GetCurveName( HWND hDlg, OBJ* O )
{
LPRFILTER   R = GetRAF(O);

	GetDlgItemText( hDlg, IDC_FI_R_LIST, R->rc[R->cnum].name, sizeof(R->rc[R->cnum].name));
}
/****************************************************************************/
static	void	FillCurveList( HWND hDlg, OBJ* O )
{
int		i, sel;
LPRFILTER R = GetRAF(O);

	sel = SendDlgItemMessage( hDlg, IDC_FI_R_LIST, CB_GETCURSEL, 0, 0);
	SendDlgItemMessage( hDlg, IDC_FI_R_LIST, CB_RESETCONTENT, 0, 0);
	for( i=0;i<MAXF;i++ ) {
		SendDlgItemMessage( hDlg, IDC_FI_R_LIST, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)R->rc[i].name);
	}
	if (sel<0) sel = 0;
	SendDlgItemMessage( hDlg, IDC_FI_R_LIST, CB_SETCURSEL, sel, 0);
}

/****************************************************************************/
static LRESULT CALLBACK RAFDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ( hDlg );
	LPRFILTER R;

	if (O) R=GetRAF(O);
	switch(message)
	{
		case WM_INITDIALOG:
			O=(OBJ*) lParam;
			OBJ::SetOBJ( hDlg, O );
			FillCurveList( hDlg, O );
			return TRUE;

		case WM_DESTROY:
			break;

		case WM_COMMAND:	switch (LOWORD(wParam)) {
			case IDC_FI_R_LIST:
				switch (HIWORD(wParam)) {
					case CBN_SELCHANGE:
						R->cnum = (int)SendDlgItemMessage(hDlg, IDC_FI_R_LIST, CB_GETCURSEL, 0, 0);
						PaintFilter( O );
						break;
					case CBN_KILLFOCUS:
					case CBN_DROPDOWN:
						GetCurveName( hDlg, O );
						FillCurveList( hDlg, O );
						break;
				}
				break;
			case IDC_FI_R_RESTORE:
				switch(R->cnum) {
					case 0:  R->rc[R->cnum] = FLOW;  break;
					case 1:  R->rc[R->cnum] = FHIGH; break;
					case 2:  R->rc[R->cnum] = FMID;  break;
					default: R->rc[R->cnum] = FLIN;  break;
				}
				FillCurveList( hDlg, O );
				PaintFilter( O );
				break;
		}
	}
	return FALSE;
}
/****************************************************************************/
static	void	UpdateLAFcontrols( HWND hDlg, OBJ* O )
{
LPLFILTER L = GetLAF(O);

	SetDlgItemDouble( hDlg, IDC_FI_L_RADIUS, L->rad, 0 );
	SetDlgItemDouble( hDlg, IDC_FI_L_WEIGHT, L->weight, 0 );
	CheckDlgButton( hDlg, IDC_FI_L_USER,	L->bUser );
	CheckDlgButton( hDlg, IDC_FI_L_LATTICE,	L->bLattice );
	CheckDlgButton( hDlg, IDC_FI_L_SHOW,	L->bShow );
	CheckDlgButton( hDlg, IDC_FI_L_INVERSE,	L->bInverse);
	CheckDlgButton( hDlg, IDC_FI_L_SMOOTH,	L->bSmooth);

	EnableWindow(GetDlgItem(hDlg, IDC_FI_L_CLEAR), L->bUser);
	EnableWindow(GetDlgItem(hDlg, IDC_FI_L_LIST), L->bLattice);
	sprintf(TMP, "Masks = %d", L->n);
	SetDlgItemText( hDlg, IDC_FI_L_INFO, TMP);
}
double GetDlgItemDbl( HWND hDlg, int IDC )
{
	char *p;

	GetDlgItemText(hDlg, IDC, TMP, 100);
	p = TMP;
	if (*p == 0)
		return 0;
	while(*p && *p!=' ')
	{
		if (!isdigit(*p) && *p!='.' && *p!='e' && *p!='E' && *p!='-' && *p!='+') return 0;
		p++;
	}
	return atof(TMP);
}
/****************************************************************************/
static	BOOL	TestLAFclick( HWND hDlg, OBJ* O, LPARAM lParam )
{
LPLFILTER L = GetLAF(O);
int		i,j;
double	r=L->rad;
double	w=L->weight;
int		h=LOINT(lParam), k=HIINT(lParam);


	if ( ! L->bUser || ! L->bShow )
		return FALSE;
	// get current data
    r = L->rad = GetDlgItemDbl( hDlg, IDC_FI_L_RADIUS );
	w = L->weight = GetDlgItemDbl( hDlg, IDC_FI_L_WEIGHT );

	// Changed by Peter on 28 Aug 2001
//	if (w<-1.) w=-1.0; if (w>1.0) w=1.0;
	if (w < -500.) w = -500.0;
	if (w >  500.) w =  500.0;
	// Try to remove
	if (L->n)
	{
		for(i=0;i<L->n;i++)
		{
			if ( abs(L->h[i] - h)<round((double)L->r[i]) &&
				 abs(L->k[i] - k)<round((double)L->r[i]))
			{
				for(j=i;j<L->n-1;j++)
				{
					L->h[j] = L->h[j+1];
					L->k[j] = L->k[j+1];
					L->r[j] = L->r[j+1];
				}
				L->n--;
				goto cont;
			}
		}
	}
	// Too close to previous
	if (   (L->n>0)
		&& abs(L->h[L->n-1] - h)<round(r*.5)
		&& abs(L->k[L->n-1] - k)<round(r*.5)) return TRUE;

	L->h[L->n] = h;
	L->k[L->n] = k;
	L->r[L->n] = round(r);
	L->wgt[L->n] = w * 0.01;	// from % to ratio
	if ((++L->n) >= MAXLPOINTS) L->n = MAXLPOINTS-1;	// Next Point
cont:
	sprintf(TMP, "Masks = %d", L->n);
	SetDlgItemText( hDlg, IDC_FI_L_INFO, TMP);
	return TRUE;
}
/****************************************************************************/
static	BOOL ScanLattice( HWND hDlg, OBJ* O, LPLFILTER L )
{
	float	ax=0., ay=0., bx=0., by=0.;
	int		i;

	if (!L->bLattice)
		return FALSE;
    if( (i = (int)SendDlgItemMessage(hDlg, IDC_FI_L_LIST, CB_GETCURSEL, 0, 0)) == CB_ERR )
		return FALSE;

	SendDlgItemMessage(hDlg, IDC_FI_L_LIST, CB_GETLBTEXT, i, (LPARAM)TMP);
	// scan the directions of a and b lattice vectors
	if (sscanf(TMP, "%f %f %f %f", &ax, &ay, &bx, &by) == 4)
	{
		L->ax = ax;
		L->ay = ay;
		L->bx = bx;
		L->by = by;
	}
	else
		return FALSE;
	return TRUE;
}
/****************************************************************************/
static	void	FillLatticeList( HWND hDlg, OBJ* O )
{
// Added by Peter
	LPLFILTER	LF = GetLAF(O);
	LPLATTICE	L = NULL;
	HWND	/*hLatRefWnd, */hFftWnd;
	OBJ*	pLatRefObj = NULL;
	OBJ* pFftObj = NULL;
	OBJ* pChiObj;

	if( O == NULL || LF == NULL )
		return;

	if ( !LF->bLattice )
		return;

	// try to find the FFT object
	if( ( hFftWnd = objFindParent( O, OT_FFT ) ) == NULL )
		return;
	// get it
	if( (pFftObj = OBJ::GetOBJ( hFftWnd ) ) == NULL )
		return;

	int		sel = (int)SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_GETCURSEL, 0, 0);

	// reset the lattice base list
	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, WM_SETREDRAW, 0, 0);
	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_RESETCONTENT, 0, 0);

	// try to find the lattice refinement object(s)
	// modified by Peter on 06 July 2006
	POSITION pos;
	pos = pFftObj->FindFirstChild(OT_LREF);
//	HwndVecIt it;
//	for(int i = 0; i < MAXCHI; i++)
//	for(it = pFftObj->m_vChildren.begin(); it != pFftObj->m_vChildren.end(); ++it)
	while( NULL != pos )
	{
//		HWND hChi = pFftObj->Chi[i];
//		HWND hChi = *it;
//		if( !hChi || !IsWindow(hChi) || ((pChiObj = OBJ::GetOBJ(hChi)) == NULL) )
//		{
//			continue;
//		}
		if( NULL != (pChiObj = pFftObj->FindNextChild(pos, OT_LREF)) )
//		if (pChiObj->ID == OT_LREF)
		{
//			if( (pChiObj = OBJ::GetOBJ( hChi ) ) == NULL )
//				continue;
			if( (L = GetLATTICE( pChiObj ) ) == NULL )
				continue;
			// add string to the list
			sprintf(TMP, "%.1f %.1f %.1f %.1f", L->ax, L->ay, L->bx, L->by);
			SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_ADDSTRING, -1, (LPARAM)TMP);
		}
	}
	// end of modification by Peter on 06 July 2006

	if (sel < 0)
		sel = 0;
	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_SETCURSEL, sel, 0);
	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, WM_SETREDRAW, 1, 0);
/*
	HWND	ob = GetWindow( back_hwnd, GW_CHILD );
	LPLATTICE L;
	int		sel = (int)SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_GETCURSEL, 0, 0);

	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, WM_SETREDRAW, 0, 0);
	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_RESETCONTENT, 0, 0);
	while ( ob && IsWindow(ob) )
	{
		if ( GetClassName( ob, TMP, 100) &&		// Retrieve class name
			 (strcmp( TMP, "LREFclass")==0) ) {	//  and it's Latref
			L = (LPLATTICE) &(OBJ::GetOBJ(ob))->dummy;		// LatRef struct ptr
			if (L->LR_Mode & LR_DONE) {					// Lattice must be refined
				sprintf( TMP, "%4.1f %4.1f %4.1f %4.1f", L->ax, L->ay, L->bx, L->by);
				SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_INSERTSTRING, -1, (LPARAM)(LPSTR)TMP);
			}
		}
		ob = GetWindow( ob, GW_HWNDNEXT );				// Get next window
	}

	if (sel<0) sel = 0;
	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_SETCURSEL, sel, 0);
	SendDlgItemMessage( hDlg, IDC_FI_L_LIST, WM_SETREDRAW, 1, 0);
*/
}
/****************************************************************************/
static LRESULT CALLBACK LAFDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ( hDlg );
	LPLFILTER L;
	BOOL	paredr = FALSE;
	double	val;
	UINT	id;

	if (O)
		L = GetLAF(O);
	switch(message)
	{
		case WM_INITDIALOG:
			O = (OBJ*) lParam;
			OBJ::SetOBJ( hDlg, O );
			UpdateLAFcontrols( hDlg, O );
			return TRUE;

		case WM_DESTROY:
			break;

		case FM_ParentInspect:
			if ( TestLAFclick( hDlg, O, lParam ) )
			{
				InvalidateRect( objFindParent( O, OT_FFT), NULL, FALSE );
				return TRUE;
			}
			else
				return FALSE;
			break;

		// TODO: update Lattice vectors list if activating this dialog window? Is it done correct here?
		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
		case WM_QUERYOPEN:
//			ActivateObj(hDlg);
//			FindLatticeRefinement( hDlg, O );
			break;

		case WM_COMMAND:
			// added by Peter on 9 Sep 2001
			if( HIWORD(wParam) == EN_KILLFOCUS )//EN_CHANGE )
			{
				id = (UINT) LOWORD(wParam);
				if( id == IDC_FI_L_WEIGHT )
				{
					val = 100;
					GetDlgItemDouble(hDlg, IDC_FI_L_WEIGHT, &val, FALSE);
				    if (val < -500.)
						val = -500.;
					if (val > 500.)
						val = 500.;
					L->weight = val;
				}
				else if( id == IDC_FI_L_RADIUS )
				{
					val = DEFRADIUS;
					GetDlgItemDouble(hDlg, IDC_FI_L_RADIUS, &val, FALSE);
				    if (val < 1.)
						val = 1.;
					if (val > MAXRADIUS)
						val = MAXRADIUS;
					L->rad = round(val);
				}
				// TODO: check - do we need this break here?
				break;
			}
			switch (LOWORD(wParam))
			{
			case IDC_FI_L_LIST:
					switch (HIWORD(wParam))
					{
						case CBN_SELCHANGE:
							goto glrad;
						case CBN_DROPDOWN:
							FillLatticeList( hDlg, O );
							break;
					}
					break;
			case IDC_FI_L_USER:
					L->bUser = IsDlgButtonChecked(hDlg, IDC_FI_L_USER);
					EnableWindow(GetDlgItem(hDlg, IDC_FI_L_CLEAR), L->bUser);
					paredr = TRUE;
					break;
			case IDC_FI_L_LATTICE:
					L->bLattice = IsDlgButtonChecked(hDlg, IDC_FI_L_LATTICE);
					EnableWindow(GetDlgItem(hDlg, IDC_FI_L_LIST), L->bLattice);
					if(!L->bLattice)
						SendDlgItemMessage( hDlg, IDC_FI_L_LIST, CB_RESETCONTENT, 0, 0);
					else
						FillLatticeList( hDlg, O );
glrad:				paredr = TRUE;
					ScanLattice( hDlg, O, L );
/*
					{
						double	fr = DEFRADIUS;
						double wgt = 100;
					    GetDlgItemDouble(hDlg, IDC_FI_L_RADIUS, &fr,0);
					    if (fr < 1.) fr = 1.; if (fr>MAXRADIUS) fr = MAXRADIUS;
						L->rad = round(fr);
					    GetDlgItemDouble(hDlg, IDC_FI_L_WEIGHT, &wgt,0);
					    if (wgt < -500.) wgt = -500.; if (wgt > 500.) wgt = 500.;
						L->weight = wgt;
//						SetDlgItemDouble( hDlg, IDC_FI_L_RADIUS, fr,0);
						// scan the basic a and b vectors from current selection in the list
					}
*/
					break;
			case IDC_FI_L_INVERSE:
					L->bInverse = IsDlgButtonChecked( hDlg, IDC_FI_L_INVERSE );
					break;
			case IDC_FI_L_SMOOTH:
					L->bSmooth = IsDlgButtonChecked( hDlg, IDC_FI_L_SMOOTH );
					break;
			case IDC_FI_L_CLEAR:
					L->n = 0;		// No points
					UpdateLAFcontrols( hDlg, O );
					paredr = TRUE;
					break;
			case IDC_FI_L_SHOW:
					paredr = TRUE;
					L->bShow = IsDlgButtonChecked(hDlg, IDC_FI_L_SHOW);
					if (L->bShow) GetCTF(O)->bShow = FALSE;
					break;
				break;
		}
	}
	if (paredr) InvalidateRect( objFindParent( O, OT_FFT), NULL, FALSE );
	return FALSE;
}
/****************************************************************************/
static	void	ChangeCTFType( HWND hDlg, OBJ* O )
{
	LPCTFILTER C = GetCTF(O);

	C->bElliptic = IsDlgButtonChecked( hDlg, IDC_FI_C_ELLIPTIC);

	EnableWindow( GetDlgItem(hDlg, IDC_FI_C_DEFOCUS0),	! C->bElliptic );
	EnableWindow( GetDlgItem(hDlg, IDC_FI_C_DEFOCUS1),	! C->bElliptic );
	EnableWindow( GetDlgItem(hDlg, IDC_FI_C_AZIMUTH),	! C->bElliptic );
	EnableWindow( GetDlgItem(hDlg, IDC_FI_C_AMP),		! C->bElliptic );

	C->bShow = C->bElliptic && C->bShow;
	CheckDlgButton( hDlg, IDC_FI_C_SHOW , C->bShow );
	if (C->bElliptic) CheckDlgButton( hDlg, IDC_FI_L_SHOW , FALSE );
	EnableWindow( GetDlgItem(hDlg, IDC_FI_C_SHOW), 		  C->bElliptic );
	EnableWindow( GetDlgItem(hDlg, IDC_FI_C_ROOT1),		  C->bElliptic );
	EnableWindow( GetDlgItem(hDlg, IDC_FI_C_ROOT2),		  C->bElliptic );
	if ( C->bElliptic )
		EstimateDfFromEllipse( hDlg, O );
}
/****************************************************************************/
static	void	UpdateCTFcontrols( HWND hDlg, OBJ* O )
{
	LPCTFILTER C = GetCTF(O);
//	double dAzimuth = 90.0 - R2D(C->Azimuth);
	double dAzimuth = (90.0 - R2D(C->aw));

	// added by Peter on 23 Aug 2001
	while( dAzimuth < 0.0 )
		dAzimuth += 180.0;
	while( dAzimuth > 180.0 )
		dAzimuth -= 180.0;

	// added by Peter on 10 Apr 2003
	CheckDlgButton( hDlg, IDC_FI_C_ELLIPTIC, C->bElliptic );
// CTF
	SetDlgItemDouble( hDlg, IDC_FI_C_DEFOCUS0, C->Df0*1e10, 0);
	// modified by Peter on 10 Apr 2003 - from Df0 to Df1
	SetDlgItemDouble( hDlg, IDC_FI_C_DEFOCUS1, C->Df1*1e10, 0);
	// modified by Peter on 23 Aug 2001
	SetDlgItemDouble( hDlg, IDC_FI_C_AZIMUTH,  dAzimuth, 0);
	SetDlgItemDouble( hDlg, IDC_FI_C_AMP,      C->Amplif, 0);
	// along U
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT1, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT1, CB_INSERTSTRING, rZero,	(LPARAM)(LPSTR)"Zero");
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT1, CB_INSERTSTRING, rPi, 	(LPARAM)(LPSTR)"Pi");
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT1, CB_INSERTSTRING, rMinusPi,(LPARAM)(LPSTR)"-Pi");
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT1, CB_SETCURSEL, C->iXoverU, 0);
	// along V
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT2, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT2, CB_INSERTSTRING, rZero,	(LPARAM)(LPSTR)"Zero");
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT2, CB_INSERTSTRING, rPi, 	(LPARAM)(LPSTR)"Pi");
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT2, CB_INSERTSTRING, rMinusPi,(LPARAM)(LPSTR)"-Pi");
	SendDlgItemMessage( hDlg, IDC_FI_C_ROOT2, CB_SETCURSEL, C->iXoverV, 0);
}
/****************************************************************************/
static	void	ScanCTF( HWND hDlg, OBJ* O )
{
	double	dd;
	LPCTFILTER C = GetCTF(O);

	if (GetDlgItemDouble( hDlg, IDC_FI_C_DEFOCUS0, &dd,0)) C->Df0		= dd*1e-10;  // Angstrom  -> m
	if (GetDlgItemDouble( hDlg, IDC_FI_C_DEFOCUS1, &dd,0)) C->Df1		= dd*1e-10;  // Angstrom  -> m
	if (GetDlgItemDouble( hDlg, IDC_FI_C_AZIMUTH,  &dd,0)) C->aw		= D2R(dd);	// deg -> rad
	// changed by Peter on 19 Sep 2001
//	if (GetDlgItemDouble( hDlg, IDC_FI_C_AZIMUTH,  &dd,0)) C->Azimuth	= D2R(dd);	// deg -> rad
	if (GetDlgItemDouble( hDlg, IDC_FI_C_AMP,	   &dd,0)) C->Amplif    = dd;

	if (C->Df0 <-50000e-10) { C->Df0=-50000e-10; SetDlgItemDouble(hDlg,IDC_FI_C_DEFOCUS0,C->Df0*1e10,0); }
	if (C->Df0 > 50000e-10) { C->Df0= 50000e-10; SetDlgItemDouble(hDlg,IDC_FI_C_DEFOCUS0,C->Df0*1e10,0); }
	if (C->Df1 <-50000e-10) { C->Df1=-50000e-10; SetDlgItemDouble(hDlg,IDC_FI_C_DEFOCUS1,C->Df1*1e10,0); }
	if (C->Df1 > 50000e-10) { C->Df1= 50000e-10; SetDlgItemDouble(hDlg,IDC_FI_C_DEFOCUS1,C->Df1*1e10,0); }
	if (C->Amplif<0)        { C->Amplif=0;       SetDlgItemDouble(hDlg,IDC_FI_C_AMP,C->Amplif,0); }
	if (C->Amplif>100)      { C->Amplif=100;     SetDlgItemDouble(hDlg,IDC_FI_C_AMP,C->Amplif,0); }
}
/****************************************************************************/
static LRESULT CALLBACK CTFDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	O = OBJ::GetOBJ( hDlg );
	LPCTFILTER C;
	BOOL	paredr = FALSE, repaint = FALSE;

	if (O)
		C = GetCTF(O);
	switch(message)
	{
		case WM_INITDIALOG:
			{
				O=(OBJ*) lParam;
				OBJ::SetOBJ( hDlg, O );
				// added and modified by Peter O. on 10 Apr 2003
				if (O)
				{
					C = GetCTF(O);
				}
				if( TRUE == C->bElliptic )
					CheckDlgButton( hDlg, IDC_FI_C_ELLIPTIC, TRUE );
				else
					CheckDlgButton( hDlg, IDC_FI_C_MATH, TRUE );
				UpdateCTFcontrols( hDlg, O );
				ChangeCTFType( hDlg, O );
#ifdef USE_FONT_MANAGER
				HFONT hFont = FontManager::GetFont(FM_NORMAL_FONT);
#else
				HFONT hFont = hfNormal;
#endif
				SendDlgItemMessage( hDlg, IDC_DEFOCUS_U, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage( hDlg, IDC_DEFOCUS_V, WM_SETFONT, (WPARAM)hFont, 0);
				TextToDlgItemW( hDlg, IDC_DEFOCUS_U, "%c", ANGSTR_CHAR );
				TextToDlgItemW( hDlg, IDC_DEFOCUS_V, "%c", ANGSTR_CHAR );
				//			SetDlgItemText( hDlg, IDC_DEFOCUS_U, "\302" );
				//			SetDlgItemText( hDlg, IDC_DEFOCUS_V, "\302" );
			}
			return TRUE;

		case WM_DESTROY:
			break;

		case FM_ParentLBUTTONUP:
			C->f &= ~CTF_Capt;
			return TRUE;

		case FM_ParentInspect:
			if (Set_EP(hDlg, O, lParam ))
			{
				EstimateDfFromEllipse( hDlg, O );
				return TRUE;
			}
			else
				return FALSE;
			break;

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLOREDIT:
			return ChangeItemColors( hDlg, message, wParam, lParam );

		case WM_COMMAND:	switch (LOWORD(wParam)) {
			case IDC_FI_C_DEFOCUS0:
			case IDC_FI_C_DEFOCUS1:
			case IDC_FI_C_AZIMUTH:
			case IDC_FI_C_AMP:
				if (HIWORD(wParam) == EN_KILLFOCUS)
					ScanCTF( hDlg, O );
				paredr = TRUE;
				repaint = TRUE;
				break;
			case IDC_FI_C_DEFAULT:
				UpdateCTFcontrols( hDlg, O );
				break;
			case IDC_FI_C_ELLIPTIC:
			case IDC_FI_C_MATH:
				ChangeCTFType( hDlg, O );
				paredr = TRUE;
				break;
			case IDC_FI_C_SHOW:
				paredr = TRUE;
				C->bShow = IsDlgButtonChecked(hDlg, IDC_FI_C_SHOW);
				if (C->bShow) GetLAF(O)->bShow = FALSE;
				break;
			case IDC_FI_C_ROOT1:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					C->iXoverU = SendDlgItemMessage( hDlg, IDC_FI_C_ROOT1, CB_GETCURSEL, 0, 0);
					EstimateDfFromEllipse( hDlg, O );
				}
				repaint = TRUE;
				break;
			case IDC_FI_C_ROOT2:
				if (HIWORD(wParam) == CBN_SELCHANGE)
				{
					C->iXoverV = SendDlgItemMessage( hDlg, IDC_FI_C_ROOT2, CB_GETCURSEL, 0, 0);
					EstimateDfFromEllipse( hDlg, O );
				}
				repaint = TRUE;
				break;
		}
	}
	if (paredr)  InvalidateRect( objFindParent( O, OT_FFT), NULL, FALSE );
	if (repaint) PaintFilter( O );
	return FALSE;
}
/****************************************************************************/
static	BOOL	SaveFILT( HWND hDlg, OBJ* O )
{
LPRFILTER   F = GetRAF(O);
LPLFILTER	L = GetLAF(O);
LPCTFILTER	C = GetCTF(O);
FILE		*out;
HCURSOR		hoc;

	_splitpath( O->fname, NULL, NULL, TMP, NULL );	// take file name only
	if (!fioGetFileNameDialog( MainhWnd, "Save filter settings", &ofNames[OFN_FILTER], TMP, TRUE)) return FALSE;

	if ((out = fopen( fioFileName(), "wb")) == NULL) return dlgError(IDS_ERRCREATEFILE, hDlg);

	hoc = SetCursor(hcWait);
	fwrite(szFltHead, strlen(szFltHead)+1, 1, out);
	fwrite(F, sizeof(RFILTER), 1, out );
	fwrite(L, sizeof(LFILTER), 1, out );
	fwrite(C, sizeof(CTFILTER),1, out );
	SetCursor(hoc);

	if (ferror(out)) { fclose(out); return dlgError(IDS_ERRWRITEFILE, hDlg); }
	fclose(out);
	return TRUE;
}
/****************************************************************************/
static	BOOL	LoadFILT( HWND hDlg, OBJ* O )
{
LPRFILTER   F = GetRAF(O);
LPLFILTER	L = GetLAF(O);
LPCTFILTER	C = GetCTF(O);;
FILE		*inf;

	_splitpath( O->fname, NULL, NULL, TMP, NULL );	// take file name only
	if (!fioGetFileNameDialog( MainhWnd, "Load filter settings", &ofNames[OFN_FILTER], TMP, FALSE)) return FALSE;

	if ((inf = fopen( fioFileName(), "rb")) == NULL) return dlgError(IDS_ERROPENFILE, hDlg);
	fread(TMP, strlen(szFltHead)+1, 1, inf);
	if (strcmp(TMP, szFltHead) != 0) return dlgError(IDS_ERRCRVHEADER, hDlg);
	fread(F, sizeof(RFILTER), 1, inf );
	fread(L, sizeof(LFILTER), 1, inf );
	fread(C, sizeof(CTFILTER),1, inf );
	if (feof(inf)) {
		fclose(inf);
		return dlgError(IDS_ERRREADFILE, hDlg);
	}
	fclose(inf);

	return TRUE;
}
/****************************************************************************/
typedef struct {
	LPSTR	lpTitle;
	LPSTR	lpDlgName;
	DLGPROC	dlgProc;
	UINT	idcButton;
} TABPAGE, * LPTABPAGE;

static TABPAGE	FilTab[3] = {
	{ "Radial",  "FI_RADIAL",  (DLGPROC)RAFDlgProc, IDC_FI_RADIAL},
	{ "Subtract lattice", "FI_LATTICE", (DLGPROC)LAFDlgProc, IDC_FI_LATTICE},
	{ "CTF",     "FI_CTF",	   (DLGPROC)CTFDlgProc, IDC_FI_CTF}
};
/****************************************************************************/
static void ChangeTab( HWND hDlg )
{
HWND	hwtc = GetDlgItem(hDlg, IDC_FI_TABS );
int		i = TabCtrl_GetCurSel(hwtc);
RECT	r, r0, rt;
int		x, y;
TCITEM	tci;
HWND	hwOver = (HWND)GetWindowLong(hwtc,GWL_USERDATA);

	SendMessage( hDlg, WM_SETREDRAW, FALSE, 0 );
	if (hwOver) DestroyWindow( hwOver ); hwOver = NULL;

	GetWindowRect(hwtc, &rt);
	MapWindowPoints(NULL,hDlg,(LPPOINT)&rt, 2);

	if (i>=0) {
		tci.mask = TCIF_PARAM;
		TabCtrl_GetItem( hwtc, i, &tci );
		i = tci.lParam;
		hwOver = CreateDialogParam( hInst, FilTab[i].lpDlgName, hDlg, FilTab[i].dlgProc, (LPARAM)OBJ::GetOBJ(hDlg) );
		GetWindowRect(hwOver, &r0);
		MapWindowPoints(NULL,hDlg,(LPPOINT)&r0, 2);
		r=r0;
		TabCtrl_AdjustRect( hwtc, FALSE, &r );
		x = rt.left+((rt.right-rt.left)-(r0.right-r0.left))/2;
		y = r.top+rt.top;
		SetRect(&r,x,y,x+(r0.right-r0.left),y+(r0.bottom-r0.top));
		SetWindowPos(hwOver, HWND_TOP, x,y, 0, 0, SWP_NOSIZE);
	} else {
		hwOver = CreateWindow("STATIC", "No active filter",
			WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE,
			0, 0, (rt.right-rt.left)/2, (rt.bottom-rt.top)/2,
			hDlg, NULL, hInst, NULL);
		SendMessage(hwOver,WM_SETFONT, (WPARAM)SendMessage(hwtc,WM_GETFONT,0,0),0);
		GetWindowRect(hwOver, &r0);
		MapWindowPoints(NULL,hDlg,(LPPOINT)&r0, 2);
		r=r0;
		x = rt.left+((rt.right-rt.left)-(r0.right-r0.left))/2;
		y = rt.top +((rt.bottom-rt.top)-(r0.bottom-r0.top))/2;
		SetRect(&r,x,y,x+(r0.right-r0.left),y+(r0.bottom-r0.top));
		SetWindowPos(hwOver, HWND_TOP, x,y, 0, 0, SWP_NOSIZE);
	}
	SetWindowLong(hwtc,GWL_USERDATA, (LONG)hwOver);
	SendMessage( hDlg, WM_SETREDRAW, TRUE, 0 );
	InvalidateRect( hDlg, &r, FALSE );
}
/****************************************************************************/
BOOL DispatchFilterTabsNotify( HWND hDlg, LPNMHDR lp )
{
	int		i = TabCtrl_GetCurSel(lp->hwndFrom);

	switch (lp->code) {
		case TCN_SELCHANGE:
			ChangeTab( hDlg );
			break;
		default: return FALSE;
	}
	return TRUE;
}
/****************************************************************************/
static void FillTab( HWND hDlg, UINT id )
{
	TCITEM		tci;
	int i;
	HWND hwtc=GetDlgItem(hDlg,IDC_FI_TABS);

	TabCtrl_DeleteAllItems( hwtc );
	for(i=0;i<3;i++) {
		tci.mask = TCIF_PARAM | TCIF_TEXT;
		tci.pszText = FilTab[i].lpTitle;
		tci.iImage = -1;
		tci.lParam = i;
		if (IsDlgButtonChecked(hDlg,FilTab[i].idcButton)) TabCtrl_InsertItem( hwtc, i, &tci);
	}
	tci.mask = TCIF_PARAM;
	for(i=0;i<3;i++) if (TabCtrl_GetItem( hwtc, i, &tci) && FilTab[tci.lParam].idcButton==id ) {
		TabCtrl_SetCurSel( hwtc, i );
		break;
	}
	ChangeTab( hDlg );
	InvalidateRect( hDlg, NULL, TRUE );
}
/****************************************************************************/
static BOOL InitFilterObject( HWND hDlg, OBJ* O )
{
// TODO: check here which FFT object we obtain!!!
	LPFFT		S = GetFFT(O);
	OBJ*		pr  = OBJ::GetOBJ( O->ParentWnd );	// The parent should be FFT type
	LPFFT		Spr = GetFFT(pr);
	LPCTFILTER	C;
	LPRFILTER	R;
	LPLFILTER	L;
	int			i;

	S->src = S->img = NULL;
	S->fs = Spr->fs;
	S->SB_Par = Spr->SB_Par;
	S->SB_Lin = Spr->SB_Lin;
	S->SB_DC  = Spr->SB_DC;
	S->SB_Sc  = Spr->SB_Sc;
	S->ft2.FT2R::FT2R(S->fs);	// FFT data
	// added by Peter on 13 Sep 2001
	S->ft2  = Spr->ft2;			// Just copy it

	O->bp  = O->dp  = NULL;		// To avoid freeing of parent memory which handles was copied
	O->ebp = O->edp = NULL;

	if ( (C = (LPCTFILTER)calloc(1, sizeof(CTFILTER))) == NULL) return FALSE;
	if ( (R = (LPRFILTER) calloc(1, sizeof(RFILTER)))  == NULL) return FALSE;
	if ( (L = (LPLFILTER) calloc(1, sizeof(LFILTER)))  == NULL) return FALSE;
	SetCTF(O, C);
	SetRAF(O, R);
	SetLAF(O, L);

	// CTF defaults
	C->Df0 = O->NI.Defocus0;
	C->Df1 = O->NI.Defocus1;
//	C->Azimuth = O->NI.Azimuth;
	C->Amplif = 1.;
	// Added by Peter on 10 Apr 2003
	C->bElliptic = 1;
	C->ne = MAXELP;
	C->n = 0;
	C->f = 0;
	C->al = S->fs * .25;
	C->ex = 0.8;
	C->aw = 0.2;
	C->dl[0] = 0.;
	C->iXoverU = rMinusPi;
	C->iXoverV = rMinusPi;
	for(i = 1; i < C->ne; i++)
		C->dl[i] = S->fs * .125;

	// Radial defaults
	R->cnum = 0;
	R->rc[0] = FLOW;
	R->rc[1] = FHIGH;
	R->rc[2] = FMID;
	for (i = 3; i < MAXF; i++)
		R->rc[i] = FLIN;

	// Lattice defaults
	L->n = 0;				// No points
	// Changed by Peter on 28 Aug 2001
	L->weight = 100.0;		//
	L->bUser = TRUE;		//
	L->bLattice = FALSE;	//
	L->rad = DEFRADIUS;		//

	SendDlgItemMessage( hDlg, IDC_FI_ITEMTOPLOT, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage( hDlg, IDC_FI_ITEMTOPLOT, CB_INSERTSTRING, iRadial,	(LPARAM)(LPSTR)"Radial");
	SendDlgItemMessage( hDlg, IDC_FI_ITEMTOPLOT, CB_INSERTSTRING, iCTF,		(LPARAM)(LPSTR)"CTF");
	SendDlgItemMessage( hDlg, IDC_FI_ITEMTOPLOT, CB_INSERTSTRING, i1_CTF,	(LPARAM)(LPSTR)"1/CTF");
	SendDlgItemMessage( hDlg, IDC_FI_ITEMTOPLOT, CB_INSERTSTRING, iRDis,	(LPARAM)(LPSTR)"R Distribution");
	SendDlgItemMessage( hDlg, IDC_FI_ITEMTOPLOT, CB_SETCURSEL, 0, 0);

	// Added by Peter to support Angstrems
#ifdef USE_FONT_MANAGER
	SendDlgItemMessage( hDlg, IDC_FI_INFO, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_NORMAL_FONT), 0);
#else
	SendDlgItemMessage( hDlg, IDC_FI_INFO, WM_SETFONT, (WPARAM)hfNormal, 0);
#endif
// 	SendDlgItemMessage( hDlg, IDC_DEFOCUS_U, WM_SETFONT, (WPARAM)hfNormal, 0);
// 	SendDlgItemMessage( hDlg, IDC_DEFOCUS_V, WM_SETFONT, (WPARAM)hfNormal, 0);
// 	TextToDlgItemW( hDlg, IDC_DEFOCUS_U, L"%c ", ANGSTR_CHAR );
// 	TextToDlgItemW( hDlg, IDC_DEFOCUS_V, L"%c ", ANGSTR_CHAR );

	return TRUE;
}
/****************************************************************************/
LRESULT CALLBACK FiltWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	try
	{
	OBJ*		O = OBJ::GetOBJ( hDlg );
	LPFFT		S;
	LPCTFILTER	C;
	LPRFILTER	R;
	LPLFILTER	L;
	if( NULL != O )
	{
		S = GetFFT(O);
		C = GetCTF(O);
		R = GetRAF(O);
		L = GetLAF(O);
	}
	HWND		hFiTabs = ::GetDlgItem(hDlg, IDC_FI_TABS);
	if( hFiTabs == NULL )
	{
		::OutputDebugString("Cannot get Tabs from filter window.\n");
		return FALSE;
	}
	HWND		hSubDlg = (HWND)GetWindowLong(hFiTabs, GWL_USERDATA);

	switch(message)
	{
		case WM_INITDIALOG:
			// ADDED BY PETER ON 29 JUNE 2004
			// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
			// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
			O = (OBJ*)lParam;
			S = GetFFT(O);
			C = GetCTF(O);
			R = GetRAF(O);
			L = GetLAF(O);
			InitializeObjectDialog(hDlg, O);

			if (!InitFilterObject( hDlg, O )) DestroyWindow(hDlg);
			FillTab( hDlg, 0 );
			return TRUE;

		case WM_NCDESTROY:
			S->ft2.FT2R::~FT2R();	// Free FFT storage
//			S->ff2.FT2R::~FT2R();	// Mask storage
			if ( GetCTF(O) ) free( GetCTF(O) );	SetCTF(O, NULL); // CTF Filter
			if ( GetRAF(O) ) free( GetRAF(O) );	SetRAF(O, NULL); // Radial filter
			if ( GetLAF(O) ) free( GetLAF(O) );	SetLAF(O, NULL);// Lattice filter
			if (O->ParentWnd) InvalidateRect( O->ParentWnd, NULL, FALSE );
			if (O->hBigPlot) DestroyWindow( O->hBigPlot ); O->hBigPlot = NULL;
			objFreeObj( hDlg );
			break;

		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
		case WM_QUERYOPEN:
			ActivateObj(hDlg);
			break;

		case FM_ParentLBUTTONUP:
		case FM_ParentInspect:
			if ( ! SendMessage( hSubDlg, message, wParam, lParam ) )
				objInformChildren( hDlg, message, wParam, lParam);
			break;

		case FM_ParentPaint:
			if ( L->bOn && L->bShow ) DrawLattFilter( O );
			if ( C->bOn && C->bShow ) DrawCTFEllipse( O );
			objInformChildren( hDlg, message, wParam, lParam);
			break;

		case FM_Update:
			Filter( O );
			objInformChildren( hDlg, FM_Update, (WPARAM)O, lParam);
			break;

		case FM_UpdateNI:
			O->NI = OBJ::GetOBJ(O->ParentWnd)->NI;
    		objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
    		break;

		case WM_PAINT:
			PaintFilter( O );
			break;

		case WM_NOTIFY:
			switch (((LPNMHDR) lParam)->idFrom)
			{
				case IDC_FI_TABS: return DispatchFilterTabsNotify( hDlg, (LPNMHDR)lParam );
			}
			break;

		case WM_LBUTTONUP:
			ClipCursor( NULL );
			if (R->df)
			{
				R->df = FALSE;
			}
			break;

		case WM_LBUTTONDOWN:
			R->df = TestClickRCurve( hDlg, O, lParam );
			break;

		case WM_MOUSEMOVE:
			if ((wParam & MK_LBUTTON) && R->df)
			{
				DragRCurve( hDlg, O, lParam );
				PaintFilter( O );
			} else PromptResolution( hDlg, O, lParam);
			break;

		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			case IDC_FI_APPLY:
  				Filter( O );
				objInformChildren( hDlg, FM_Update, (WPARAM)O, lParam);
				break;

			case IDC_FI_SAVE:
				SaveFILT( hDlg, O );
				break;

			case IDC_FI_LOAD:
				LoadFILT( hDlg, O );
				CheckDlgButton( hDlg, IDC_FI_RADIAL, R->bOn );
				CheckDlgButton( hDlg, IDC_FI_LATTICE, L->bOn );
				CheckDlgButton( hDlg, IDC_FI_CTF, C->bOn );
				Filter( O );
				objInformChildren( hDlg, FM_Update, (WPARAM)O, lParam);
				InvalidateRect( objFindParent( O, OT_FFT), NULL, FALSE );
				PaintFilter( O );
				// conntinue with fill tab
			case IDC_FI_RADIAL:
			case IDC_FI_LATTICE:
			case IDC_FI_CTF:
				R->bOn = IsDlgButtonChecked( hDlg, IDC_FI_RADIAL );
				L->bOn = IsDlgButtonChecked( hDlg, IDC_FI_LATTICE );
				C->bOn = IsDlgButtonChecked( hDlg, IDC_FI_CTF );
				FillTab( hDlg, LOWORD(wParam) );
				break;

			case IDC_FI_ITEMTOPLOT:
				PaintFilter( O );
				break;

			case IDC_FI_BIGPLOT:
				if (IsDlgButtonChecked(hDlg, IDC_FI_BIGPLOT))
				{
					O->PlotPaint = PaintFilter;
					O->hBigPlot = CreateWindowEx(WS_EX_WINDOWEDGE, "BIGPLOT", "Filter",
								WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS,
								100, 100, 200, 200,
								MDIhWnd, NULL, hInst, O);
					ShowWindow( O->hBigPlot, SW_SHOWNORMAL );
					InvalidateRect( hDlg, NULL, TRUE );
				}
				else
				{
					if (O->hBigPlot) DestroyWindow(O->hBigPlot);
					O->hBigPlot = NULL;
					PaintFilter( O );
				}
				break;

			case IDOK:
			case IDCANCEL:
				DestroyWindow( hDlg );
				return TRUE;
		}
	}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("FiltWndProc() -> message '%s': '%s'"), message, e.what()));
	}
	return FALSE;
}
