/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.LatRef * by ML ****************************************************/
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
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"

//#pragma warning(disable:4090)

#define	Rm	3
#define Rn	4
#define	Rg	1
#define	Td1	3.0
#define	Td2	0.71
#define	Tn1	12.
#define	Tn2	8.
#define	GD	0.075

#define	A90	1.570796327
#define Swap(a,b) {x=a; a=b; b=x;}

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

//#define	MaxRfl	10000
/****************************************************************************/
/****************************************************************************/
BOOL lrCGPeakPos( FT2R& ft2, LPDOUBLE xc, LPDOUBLE yc, double r, LPDOUBLE shift, LPDOUBLE aver )
{
double	a, ax, ay, as, xx = *xc, yy = *yc;
int     x, y,x0, y0, n;

	x0 = round(xx); y0 = round(yy);
	ax = ay = as = 0.; n=0;
	for(x=x0-r; x<=x0+r; x++) for(y=y0-r; y<=y0+r; y++) {
		if (ft2.ValidHK(x,y)) {
			a = ft2.GetAmp(x, y);
			ax += x*a; ay += y*a; as +=a; n++;
		}
	}
	if(as < 1.) return FALSE;
	*xc = ax/as; *yc = ay/as;
	ax = *xc-xx; ay = *yc-yy;

	if (shift) *shift = CABS(ax,ay);
	if (aver && n) *aver = as/n;
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
BOOL lrMaxPeakPos( FT2R& ft2, LPDOUBLE xc, LPDOUBLE yc, double r, LPDOUBLE shift, LPDOUBLE aver )
{
double	a, ax, ay, as, am;
int     x, y,x0, y0, n;

	x0 = round(*xc); y0 = round(*yc);
	am = as = 0.; n=0;
	for(x=x0-r; x<=x0+r; x++) for(y=y0-r; y<=y0+r; y++) {
		if (ft2.ValidHK(x,y)) {
			a = ft2.GetAmp(x, y);
			as +=a; n++;
			if (a>am) { ax = x; ay = y; am=a; }
		}
	}
	if (as < 1.) return FALSE;
//	if ((fabs(ax-r-*xc)<1.5) || (fabs(ax+r-*xc)<1.5) ||
//		(fabs(ay-r-*yc)<1.5) || (fabs(ay+r-*yc)<1.5)) return FALSE;
	*xc = ax; *yc = ay;
	ax = *xc-ax; ay = *yc-ay;

	if (shift) *shift = CABS(ax,ay);
	if (aver && n) *aver = as/n;
	return TRUE;
}
/****************************************************************************/
double lrAreaESD( FT2R& ft2, double xc, double yc, double r, double aver )
{
double	a, as;
int     x, y, x0, y0, n;

	x0 = round(xc); y0 = round(yc);
	as = 0.; n=0;
	for(x=x0-r; x<=x0+r; x++) for(y=y0-r; y<=y0+r; y++) {
		if (ft2.ValidHK(x,y)) {
			a = ft2.GetAmp(x, y);
			a = fabs(a-aver);
			as += a*a; n++;
		}
	}
	if (n>1) return sqrt(as/(n-1));
	else     return 0;
}
/****************************************************************************/
BOOL lrCheckRefl( FT2R& ft2, LPDOUBLE xc, LPDOUBLE yc, double rmax )
{
double shift,aver,esd, a;

	if (CABS(*xc,*yc)<3) return FALSE;
	if (rmax>0) {
		if (!lrCGPeakPos( ft2, xc, yc, rmax, &shift, &aver)) return FALSE;
		if (shift>=rmax-1) return FALSE;
		return TRUE;
	} else {
		if (!lrMaxPeakPos( ft2, xc, yc, 5, &shift, &aver)) return FALSE;
		if (!lrCGPeakPos( ft2, xc, yc, 3, &shift, &aver)) return FALSE;
		if (shift>1.5) return FALSE;
		esd = lrAreaESD( ft2, *xc, *yc, 3, aver );
		a = ft2.GetAmp( round(*xc), round(*yc) );
		if (a-aver>3*esd) return TRUE;
		return FALSE;
	}
}
/****************************************************************************/
static	void	PaintRefls( LPLATTICE L, double fontWidth, double fontHeight)
{
	HDC		hDC;
	int     x, y;
	HPEN	hop;

// added by Peter on 13 Sep 2001
	OBJ* pFFT = OBJ::GetOBJ(L->hFFTwnd);
	if( pFFT == NULL)
	{
		// L->hFFTwnd must be NULL then
		return;
	}
	hDC = GetDC( L->hFFTwnd );
	hop = SelectPen( hDC, RedPen );

// changed by Peter on 13 Sep 2001
//	Fc2Sc( &fontWidth, &fontHeight, L->F );
	Fc2Sc( &fontWidth, &fontHeight, pFFT );
	x = round(fontWidth);
	y = round(fontHeight);
	SelectObject( hDC, RedPen );
	_MoveTo( hDC, x-1,  y-5 ); LineTo( hDC, x-1,  y-1 ); LineTo( hDC, x-6,  y-1 );
	_MoveTo( hDC, x+1,  y-5 ); LineTo( hDC, x+1,  y-1 ); LineTo( hDC, x+6,  y-1 );
	_MoveTo( hDC, x-5,  y+1 ); LineTo( hDC, x-1,  y+1 ); LineTo( hDC, x-1,  y+6 );
	_MoveTo( hDC, x+1,  y+5 ); LineTo( hDC, x+1,  y+1 ); LineTo( hDC, x+6,  y+1 );

	SelectObject(hDC, hop);
	ReleaseDC( L->hFFTwnd, hDC );
}
/****************************************************************************/
int		Get_SCenter( FT2R& ft2, int xs, LPDOUBLE xc, LPDOUBLE yc)
{
double	a, b, c, d, p, q, x1, y1;
int		x, y, x0 = round(*xc), y0 = round(*yc), err = 0;
static	double	kk[8];
static	double	aa[5][5];
LPDOUBLE ap = &aa[0][0];

#define	A0	kk[0]
#define	B0	kk[1]
#define	C0	kk[2]
#define	D0	kk[3]
#define	A1	kk[4]
#define	B1	kk[5]
#define	C1	kk[6]
#define	D1	kk[7]

	for(y=-2; y<=2; y++) for(x=-2; x<=2; x++) *(ap++) = ft2.GetAmp(x0+x, y0+y);

	Conv5x5(&aa[0][0], &kk[0]);

	if( fabs(B0) < 0.3*fabs(aa[2][2]) ) return(0);
	if( fabs(C1) < 0.3*fabs(aa[2][2]) ) return(0);

	a = C0*D1 - C1*D0;
	b = A0*D1 - A1*D0 - B0*C1 + B1*C0;
	c = A0*B1 - A1*B0;

	x1 = y1 = 1234.;

	if(fabs(a) < 0.0001) {
		if(fabs(b) < 0.0001) err = 1;
		else	{ d = 0.;	y1 = -c/b;}
	} else {
		y1 = -b/a*.5;	d  = y1*y1 - c/a;
		if(d < 0) err = 2;
		else	{ d = sqrt(d);	y1-= d;}
	}

	if(fabs(y1) > fabs(y1+2.*d)) y1 += 2.*d;

	p = (B0 + B1) + (D0 + D1) * y1;
	q = (A0 + A1) + (C0 + C1) * y1;
	if(fabs(p) < 0.0001) err = 3; else	x1 = -q/p;

	if(fabs(x1) > 3) err = 4;
	if(fabs(y1) > 3) err = 5;
	if(err) return(0);

	if(fabs(x1) > 1.6) err = 6; else *xc = x0 + x1*.5;
	if(fabs(y1) > 1.6) err = 7; else *yc = y0 + y1*.5;

	return 1;
}

/****************************************************************************/
/****************************************************************************/
double	Cr_Noise(FT2R& ft2, int r, double xc, double yc)
{
int		x, y, x0, y0;
double	an, as, nn, ns;

	x0 = round(xc); y0 = round(yc);	an = as = nn = ns = 0.;

	for(x=x0-1; x<=x0+1; x++) for(y=y0-1; y<=y0+1; y++) {
		if (ft2.ValidHK(x,y)) { as+=ft2.GetAmp(x, y); an += 1.; }
	}
	for(x=x0-r; x<=x0+r; x++) {
		if (ft2.ValidHK(x, y0+r)) { ns += ft2.GetAmp(x, y0+r); nn++; }
		if (ft2.ValidHK(x, y0-r)) { ns += ft2.GetAmp(x, y0-r); nn++; }
	}
	for(y=y0-(r-1); y<=y0+(r-1); y++) {
		if (ft2.ValidHK(x0+r, y)) { ns += ft2.GetAmp(x0+r, y); nn++; }
		if (ft2.ValidHK(x0-r, y)) { ns += ft2.GetAmp(x0-r, y); nn++; }
	}
	as /= an;	ns /= nn;	as -= ns;
	if ( as < 0. )		return 0.;
	if ( ns < 0.0001)	return 1000.;
	return 9.*as/ns;
}
/****************************************************************************/
/*
double	Get_ReflC(LPLATTICE L, LPREFL R)
{
	int		x, y, x0, y0;
	double	a, an, as, ns, nn;
	FT2R&	ft2 = L->ft2;

	x0 = round(R->x0);
	y0 = round(R->y0);
	an = as = ns = nn = 0.;

	// Get Noise as 9x9 (Rn == 4)
	for(x=x0-Rn; x<=x0+Rn; x++)
	{
		a = ft2.GetAmp(x, y0+Rn); ns += a*a;
		a = ft2.GetAmp(x, y0-Rn); ns += a*a;
		nn += 2.;
	}
	for(y=y0-(Rn-1); y<=y0+(Rn-1); y++)
	{
		a = ft2.GetAmp(x0+Rn, y); ns += a*a;
		a = ft2.GetAmp(x0-Rn, y); ns += a*a;
		nn += 2.;
	}
	// Get Amplitude as Mean 3x3
	for(x=x0-1; x<= x0+1; x++)
	{
		a = ft2.GetAmp(x, y0+1); as += a*a;
		a = ft2.GetAmp(x, y0  ); as += a*a;
		a = ft2.GetAmp(x, y0-1); as += a*a;
		an += 3.;
	}
	as /= an; ns /= nn;
	nn = sqrt(as/(ns+1.))*10.;
	R->qq = __min(255,(int)nn);

	// Phase extraction
	R->p0 = R2P(ft2.GetPha(x0, y0));

	a = as - ns;
	if(a>0)	R->a0 = sqrt(a);
	else  { R->a0 = 0.; R->f |= BAMP;}
	return R->a0;
}
*/
/****************************************************************************/
/*
double	Get_Refl1(LPLATTICE L, LPREFL R)
{
	int		r, x, y, x0, y0;
	double	a, an, as, ns, nn, dir_x, dir_y;
	FT2R&	ft2 = L->ft2;

	// Modified by Peter on 26 Aug 2001
//	r  = round((L->al + L->bl) / 4.);
	r  = round((L->al + L->bl) * .25);
	x0 = round(R->x0);
	y0 = round(R->y0);
	an = as = ns = nn = 0.;

	// Get Noise as Halfway
	for(x=x0-r; x<=x0+r; x++)
	{
		a = ft2.GetAmp(x, y0+r); ns += a*a;
		a = ft2.GetAmp(x, y0-r); ns += a*a;
		nn += 2.;
	}
	for(y=y0-(r-1); y<=y0+(r-1); y++)
	{
		a = ft2.GetAmp(x0+r, y); ns += a*a;
		a = ft2.GetAmp(x0-r, y); ns += a*a;
		nn += 2.;
	}
	// Get Amplitude as Mean 3x3
	for(x=x0-1; x<= x0+1; x++)
	{
		a = ft2.GetAmp(x, y0+1); as += a*a;
		a = ft2.GetAmp(x, y0  ); as += a*a;
		a = ft2.GetAmp(x, y0-1); as += a*a;
		an += 3.;
	}
	as /= an; ns /= nn;
	a = as - ns;
//	sprintf(TMP, "%f\t%f\n", a, ns);
//	::OutputDebugString(TMP);

	dir_x = (L->ax + L->bx)*.5;
	dir_y = (L->ay + L->by)*.5;
	ns  = GetAmplAvg3x3(ft2, R->x0 + dir_x, R->y0 + dir_y);
	ns += GetAmplAvg3x3(ft2, R->x0 + dir_x, R->y0 - dir_y);
	ns += GetAmplAvg3x3(ft2, R->x0 - dir_x, R->y0 - dir_y);
	ns += GetAmplAvg3x3(ft2, R->x0 - dir_x, R->y0 + dir_y);
	ns *= .25;
	a = as - ns;

//	sprintf(TMP, "%f\t%f\n", a, ns);
//	::OutputDebugString(TMP);

	nn = sqrt(as/(ns+1.))*10.;
	R->qq = __min(255,(int)nn);

	// Phase extraction
	R->p0 = R2P(ft2.GetPha(x0, y0));

	if(a>0)	R->a0 = sqrt(a);
	else  { R->a0 = 0.; R->f |= BAMP;}
	return R->a0;
}
*/
/****************************************************************************/
/*
double	Get_Refl2(LPLATTICE L, LPREFL R)
{
	int		r, x, y, x0, y0;
	double	a, an, as, ns, nn, amax;
	FT2R&	ft2 = L->ft2;
	FCOMPLEX	fontWidth=0;

	// Modified by Peter on 26 Aug 2001
//	r  = round((L->al + L->bl) / 4.);
	r  = round((L->al + L->bl) * .25);
	x0 = round(R->x0);
	y0 = round(R->y0);
	an = as = ns = nn = amax = 0.;

	// Modified by Peter on 26 Aug 2001
//	for(x=0; x<=1; x++)
//	{
//		a = ft2.GetAmp(x0+r, y0+r); ns += a*a;
//		a = ft2.GetAmp(x0+r, y0-r); ns += a*a;
//		a = ft2.GetAmp(x0-r, y0+r); ns += a*a;
//		a = ft2.GetAmp(x0-r, y0-r); ns += a*a;
//		nn += 4.; r++;
//	}

	// Get Noise as Halfway
	for(x = x0-r; x <= x0+r; x++)
	{
		a = ft2.GetAmp(x, y0+r); ns += a*a;
		a = ft2.GetAmp(x, y0-r); ns += a*a;
		nn += 2.;
	}
	for(y = y0-(r-1); y <= y0+(r-1); y++)
	{
		a = ft2.GetAmp(x0+r, y); ns += a*a;
		a = ft2.GetAmp(x0-r, y); ns += a*a;
		nn += 2.;
	}
	// calculate __max amplitude and average amplitude also
	for(x = x0-1; x <= x0+1; x++)
	{
		for(y = y0-1; y <= y0+1; y++)
		{
			a = ft2.GetAmp(x, y);
			as += a*a;
			an += 1.;
			if(a > amax)
				amax = a;
		}
	}
	// calc. average phase
	for(x = x0-1; x <= x0+1; x++)
	{
		for(y = y0-1; y <= y0+1; y++)
		{
			a = ft2.GetAmp(x, y);
			if(a > 0.6*amax)
			{
				fontWidth += ft2.GetVal(x, y);
			}
		}
	}
	as /= an; ns /= nn;
	nn = sqrt(as/(ns+1.))*10.;
	R->qq = __min(255,(int)nn);

	// Phase extraction from average
	if(abs(fontWidth) > 0.1)	R->p0 = R2P(atan2(fontWidth));
	else                R->p0 = R2P(ft2.GetPha(x0, y0));

	a = as - ns;
	if(a>0)	R->a0 = sqrt(a);
	else  { R->a0 = 0.; R->f |= BAMP;}
	return R->a0;
}
*/
/****************************************************************************/
/*
double	Get_Refl3(LPLATTICE L, LPREFL R)
{
	int		xx0, yy0, x, y, sx, sy, r;
	double	x2, y2, ar2, nr2, x0 = R->x0, y0 = R->y0;
	double	sn=0., sa=0., nr=0., nn=0.;
	double	a, amax=0., nmax = 0.;
	FT2R&	ft2 = L->ft2;
	FCOMPLEX	fontWidth, ffx;

	ar2 = CABS(L->al,L->bl) / 6.;
	if(ar2<1.14) ar2 = 1.14;
	else if(ar2<1.6) ar2 = 1.6;
		 else ar2 = 2.7;

	nr2 = ar2*2.;
	r = (int)ceil(nr2+0.5); ar2 *= ar2; nr2 *= nr2;

	xx0 = round(x0);
	yy0 = round(y0);

	// calculate maximum amplitude
	for(x = xx0-1; x <= xx0+1; x++)
	{
		for(y = yy0-1; y <= yy0+1; y++)
		{
			a = ft2.GetAmp(x, y);
			if(a > amax)
				amax = a;
		}
	}

	fontWidth=0;
	for(sx=-r; sx<=r; sx++)
	{
		x = xx0 + sx;
		x2 = (double)x - x0;
		x2 *= x2;
		for(sy=-r; sy<=r; sy++)
		{
			y = yy0+sy;
			y2 = (double)y - y0;
			y2 = y2*y2 + x2;
			if(y2 <= ar2)
			{
				a = ft2.GetAmp(x, y);
				if(a > 0.6*amax)
				{
					fontWidth += ft2.GetVal(x, y);
				}
				sa += a*a;
				nr += 1.;
			}
			else if(y2<=nr2)
			{
				a = ft2.GetAmp(x, y);
				sn += a*a;
				nn += 1.;
				if(a > nmax)
					nmax = a;
			}
		}
	}
	if(amax < 0.7*nmax)		R->f |= BSN;
	sn /= nn*2;	sa /= nr;
	nn = sqrt(sa/(sn+1.))*10.;
	R->qq = __min(255,(int)nn);
	if(nn < 7)	R->f |= BSN;	// <0.7

	// Phase extraction
	if(abs(fontWidth) > 0.1)	R->p0 = R2P(atan2(fontWidth));
	else				R->p0 = R2P(ft2.GetPha(xx0, yy0));

	a =  sa -sn;
	if(a > 1.) R->a0 = sqrt(a);
	else 	 { R->a0 = 0.; R->f |= BAMP;}
	return R->a0;
}
*/
static double GetAmplAvg3x3(LPLATTICE L, double dPosX, double dPosY)
{
	int x, x0, y0;
	double as = .0, a;
	FT2R&	ft2 = L->ft2;

	x0 = round(dPosX);
	y0 = round(dPosY);
	// Get Amplitude as Mean 3x3
	for(x = x0 - 1; x <= x0 + 1; x++)
	{
		a = ft2.GetAmp(x, y0+1); as += a*a;
		a = ft2.GetAmp(x, y0  ); as += a*a;
		a = ft2.GetAmp(x, y0-1); as += a*a;
	}
	return as / 9.;
}

static double GetNoise9x9(LPLATTICE L, LPREFL pRefl)
{
	int		x, y, x0, y0;
	double	a, ns, nn;
	FT2R&	ft2 = L->ft2;

	x0 = round(pRefl->x0);
	y0 = round(pRefl->y0);
	ns = nn = 0.;

	// Get Noise as 9x9 (Rn == 4)
	for(x = x0 - Rn; x <= x0 + Rn; x++)
	{
		a = ft2.GetAmp(x, y0 + Rn); ns += a*a;
		a = ft2.GetAmp(x, y0 - Rn); ns += a*a;
		nn += 2.;
	}
	for(y = y0 - (Rn - 1); y <= y0 + (Rn - 1); y++)
	{
		a = ft2.GetAmp(x0+Rn, y); ns += a*a;
		a = ft2.GetAmp(x0-Rn, y); ns += a*a;
		nn += 2.;
	}
	ns /= nn;
	return ns;
}
static double GetNoiseHalfway(LPLATTICE L, LPREFL pRefl)
{
	double	ns, x, y;
	FT2R&	ft2 = L->ft2;

	// Get at the box corners
	x = pRefl->x0 + (L->ax + L->bx)*.5;		y = pRefl->y0 + (L->ay + L->by)*.5;
	ns  = GetAmplAvg3x3(L, x, y);
//	sprintf(TMP, "x = %f\tY = %f\n", x, y);		::OutputDebugString(TMP);

	x = pRefl->x0 + (L->ax - L->bx)*.5;		y = pRefl->y0 + (L->ay - L->by)*.5;
	ns += GetAmplAvg3x3(L, x, y);
//	sprintf(TMP, "x = %f\tY = %f\n", x, y);		::OutputDebugString(TMP);

	x = pRefl->x0 + (-L->ax - L->bx)*.5;	y = pRefl->y0 + (-L->ay - L->by)*.5;
	ns += GetAmplAvg3x3(L, x, y);
//	sprintf(TMP, "x = %f\tY = %f\n", x, y);		::OutputDebugString(TMP);

	x = pRefl->x0 + (-L->ax + L->bx)*.5;	y = pRefl->y0 + (-L->ay + L->by)*.5;
	ns += GetAmplAvg3x3(L, x, y);
//	sprintf(TMP, "x = %f\tY = %f\n", x, y);		::OutputDebugString(TMP);

	ns *= .25;
	return ns;
}
static double GetNoiseBox(LPLATTICE L, LPREFL pRefl)
{
	int i, len;
	double	x0, y0, x1, y1, a, nn, ns, min_len, dir_ax, dir_ay, dir_bx, dir_by, dx, dy;
	FT2R&	ft2 = L->ft2;

	min_len = __min(L->al,L->bl);
	len = round(min_len);

	dir_ax = L->ax * min_len * .5 / L->al;
	dir_ay = L->ay * min_len * .5 / L->al;
	dir_bx = L->bx * min_len * .5 / L->bl;
	dir_by = L->by * min_len * .5 / L->bl;

	// trace perimeter of the box
	nn = ns = 0.;
	// 1. top line
	x0 = pRefl->x0 + (-dir_ax + dir_bx);
	y0 = pRefl->y0 + (-dir_ay + dir_by);
	x1 = pRefl->x0 + (dir_ax + dir_bx);
	y1 = pRefl->y0 + (dir_ay + dir_by);
	dx = (x1 - x0) / len;
	dy = (y1 - y0) / len;
	for(i = 1; i <= len; x0 += dx, y0 += dy, i++)
	{
		a = ft2.GetAmp(round(x0), round(y0)); ns += a*a;
		nn += 1.0;
//		sprintf(TMP, "x = %f\tY = %f\n", x0, y0);		::OutputDebugString(TMP);
	}
	// 1. right line
	x0 = pRefl->x0 + (dir_ax + dir_bx);
	y0 = pRefl->y0 + (dir_ay + dir_by);
	x1 = pRefl->x0 + (dir_ax - dir_bx);
	y1 = pRefl->y0 + (dir_ay - dir_by);
	dx = (x1 - x0) / len;
	dy = (y1 - y0) / len;
	for(i = 1; i <= len; x0 += dx, y0 += dy, i++)
	{
		a = ft2.GetAmp(round(x0), round(y0)); ns += a*a;
		nn += 1.0;
//		sprintf(TMP, "x = %f\tY = %f\n", x0, y0);		::OutputDebugString(TMP);
	}
	// 1. bottom line
	x0 = pRefl->x0 + (dir_ax - dir_bx);
	y0 = pRefl->y0 + (dir_ay - dir_by);
	x1 = pRefl->x0 + (-dir_ax - dir_bx);
	y1 = pRefl->y0 + (-dir_ay - dir_by);
	dx = (x1 - x0) / len;
	dy = (y1 - y0) / len;
	for(i = 1; i <= len; x0 += dx, y0 += dy, i++)
	{
		a = ft2.GetAmp(round(x0), round(y0)); ns += a*a;
		nn += 1.0;
//		sprintf(TMP, "x = %f\tY = %f\n", x0, y0);		::OutputDebugString(TMP);
	}
	// 1. left line
	x0 = pRefl->x0 + (-dir_ax - dir_bx);
	y0 = pRefl->y0 + (-dir_ay - dir_by);
	x1 = pRefl->x0 + (-dir_ax + dir_bx);
	y1 = pRefl->y0 + (-dir_ay + dir_by);
	dx = (x1 - x0) / len;
	dy = (y1 - y0) / len;
	for(i = 1; i <= len; x0 += dx, y0 += dy, i++)
	{
		a = ft2.GetAmp(round(x0), round(y0)); ns += a*a;
		nn += 1.0;
//		sprintf(TMP, "x = %f\tY = %f\n", x0, y0);		::OutputDebugString(TMP);
	}
	return ns / nn;
}
static float GetPixelPhase(LPLATTICE L, LPREFL pRefl)
{
	int x0, y0;
	FT2R&	ft2 = L->ft2;

	x0 = round(pRefl->x0);
	y0 = round(pRefl->y0);

	return R2P(ft2.GetPha(x0, y0));
}
// returns the Phase value calculated as an ATAN of vector,
// which is a sum of complex values of points in area 3x3.
// Only those points, for which A > Amax
// in this 3x3 area, are take into account.
static float GetPhasAvg3x3(LPLATTICE L, LPREFL pRefl)
{
	int x, y, x0, y0, i, j;
	double amax = .0, a[3][3], val;
	FCOMPLEX fontWidth[3][3], fc;
	FT2R&	ft2 = L->ft2;

	x0 = round(pRefl->x0);
	y0 = round(pRefl->y0);
	// calculate __max amplitude
	for(y = y0 - 1, j = 0; j < 3; y++, j++)
	{
		for(x = x0 - 1, i = 0; i < 3; x++, i++)
		{
			fontWidth[i][j] = fc = ft2.GetVal(x, y);
			a[i][j] = val = sqrt(fc.i*fc.i + fc.j*fc.j);
			if(val > amax)
				amax = val;
		}
	}
	// calc. phase
	fc = 0.;
	for(j = 0; j < 3; j++)
	{
		for(i = 0; i < 3; i++)
		{
			if(a[i][j] > 0.6*amax)
			{
				fc += fontWidth[i][j];
			}
		}
	}
	// Phase extraction from average
	if(abs(fc) > 0.1)
		return R2P(atan2(fc));

	return R2P(ft2.GetPha(x0, y0));
}
static double GetReflValues(LPLATTICE L, LPREFL R, int iPhaMode, int iNoiseMode)
{
	double a, as, ns, nn, p0;

	// get Avg3x3 value
	as = GetAmplAvg3x3(L, R->x0, R->y0);
	// get noise
	switch (iNoiseMode)
	{
		case 0 :	ns = GetNoise9x9(L, R); break;
		case 2 :	ns = GetNoiseHalfway(L, R); break;
		case 4 :
		default :	ns = GetNoiseBox(L, R); break;
	}
	// value to noise ratio
	nn = sqrt(as / (ns + 1.)) * 10.;
	R->qq = __min(255,(int)nn);

	// get phase
	switch (iPhaMode)
	{
		case 0 :	p0 = GetPixelPhase(L, R); break;
		case 1 :	p0 = GetPhasAvg3x3(L, R); break;
		default :	p0 = GetPixelPhase(L, R); break;
	}
	R->p0 = p0;

	// subtract noise
	a = as - ns;
	if(a > 0)
		R->a0 = sqrt(a);
	else
	{
		R->a0 = 0.;
		R->f |= BAMP;
	}
	return R->a0;
}
/****************************************************************************/
// #pragma optimize ("",off)
BOOL	rflGetRefls(LPLATTICE L)
{
//	int		i, mode = L->Ex_Mode & 3;
	int		i, iPhaMode = L->Ex_Mode & EM_PHA_MODE, iNoiseMode = L->Ex_Mode & EM_NOISE_MODE;
	double	amax=0, a=0;//, ns, p0;

	double	lr = L->rad, rm = L->ft2.S*.5-4., al = L->al, bl = L->bl;
	double	x, y, ax, ay, bx, by, cx, cy, dx, dy, x0, y0;
	int		h, k, l, m, hp, km, kp, lm, mm, lp, mp, xs = L->ft2.S;
	static	int nrefl;
	LPREFL	R;
//FT2R&	ft2 = L->ft2;

	ax = L->ax; ay = L->ay; bx = L->bx; by = L->by;
	cx = L->cx; cy = L->cy; dx = L->dx; dy = L->dy;

	L->nrefl = 0;
	l = m = 1;
	km = lm = mm = 0;
	kp = lp = mp = 1;
	hp = (int)(lr/al)+1;
	if(L->num_dimensions >= 2)
	{
		hp += abs(round(hp*sin(M_PI*.5-L->gamma)));
		km = (int)(lr/bl)+1;
		km += abs(round(km*sin(M_PI*.5-L->gamma)));
		kp = km;
		if(L->cl > 3.) l = round(al/L->cl);	if(!l) l=1;
		if(L->dl > 3.) m = round(bl/L->dl); if(!m) m=1;
		lp = l/2+1;	lm = l-lp;	mp = m/2+1;	mm = m-mp;
	}

	nrefl = hp*(km+kp)*(lm+lp)*(mm+mp);
	if ((L->rptr = (LPREFL)realloc(L->rptr, nrefl*sizeof(REFL)))==NULL) return FALSE;
	memset(L->rptr, 0, nrefl*sizeof(REFL));
	R = L->rptr;

	for(h =   0; h < hp; h++)	for(k = -km; k < kp; k++)
	for(l = -lm; l < lp; l++)	for(m = -mm; m < mp; m++) {
		if((h|k|l|m) == 0) continue;
		if((h==0) && (k<0)) continue;
		x = x0 = h*ax + k*bx + l*cx + m*dx;
		y = y0 = h*ay + k*by + l*cy + m*dy;
		if(CABS(x, y) >= lr) continue;
		if((fabs(x) >= rm) || (fabs(y) >= rm)) continue;
		R->f = 0;
//		if(L->Ex_Mode & 3)
		// TODO: check what if (L->Ex_Mode & EM_MODE == 0)?
		if(L->Ex_Mode & EM_MODE)
		{
//			if(Get_SCenter(fft, xs, &x, &y)==0)	R->f |= BCN;
			if(CABS(x-x0, y-y0) > 1.4 )			R->f |= BCD;
		}
		R->x0 = x0;	R->y0 = y0;
		R->h0 = R->h = h;	R->k0 = R->k = k;
		R->l0 = R->l = l;	R->m0 = R->m = m;
/*
		switch (mode)
		{
			case 0 :	a = Get_ReflC(L, R); break;
			case 1 :	a = Get_Refl1(L, R); break;
			case 2 :	a = Get_Refl2(L, R); break;
			case 3 :	a = Get_Refl3(L, R); break;
		}
*/
		a = GetReflValues(L, R, iPhaMode, iNoiseMode);

		if(LM0(R)==0) if(a > amax) amax = a;
		R->p = R->p0;

//		sprintf(TMP, "A = %f\tPha = %f\n", R->a0, R->p0);
//		::OutputDebugString(TMP);

		PaintRefls( L, x0, y0 );
		R++;
		L->nrefl++;
		if(L->nrefl >= nrefl) break;
	}

	L->rptr = (LPREFL)realloc( L->rptr, sizeof(REFL)*L->nrefl );
	R = L->rptr; amax /= 10000.;
	amax= __max(1e-6,amax);
	L->MaxAmp = 10000.;
	for(i = L->nrefl; i>0; i--) {
		a = R->a0 / amax;
		R->a0 = R->a = a;
		R++;
	}
	return TRUE;
}
/****************************************************************************/
int		Lin_Equ(int dm, int sm, int dv, LPDOUBLE MC, LPDOUBLE MS)
{
#define	A(i,j) MC[(i)*sm+(j)]
#define	V(i,j) MS[(i)*dv+(j)]
static	int	idc[64], idr[64], ipv[64];
int		i, j, k, ic, ir;
double	a, __max;

	memset( idc, 0, sizeof(idc) );
	memset( idr, 0, sizeof(idr) );
	memset( ipv, 0, sizeof(ipv) );

	for(i=0; i<dm; i++) {
		__max = 0.;
		for(j=0; j<dm; j++) if(ipv[j] != 1) for(k=0; k<dm; k++) {
			if(ipv[k] == 0){ if((a=fabs(A(j,k)))>=__max){ __max=a; ir=j; ic=k;}}
			else if(ipv[k] > 1) return 0;
		}
		ipv[ic]++;
		if(ir != ic) {
			for(j=0; j<dm; j++){ a=A(ir,j); A(ir,j)=A(ic,j); A(ic,j)=a;}
			for(j=0; j<dv; j++){ a=V(ir,j); V(ir,j)=V(ic,j); V(ic,j)=a;}
		}
		idr[i] = ir;	idc[i] = ic;
		if( fabs(a=A(ic,ic)) < 1e-5) return -i;
		A(ic,ic) = 1.;
		for(j=0; j<dm; j++) A(ic,j) /= a;
		for(j=0; j<dv; j++) V(ic,j) /= a;
		for(j=0; j<dm; j++) if(j != ic) {
			a = A(j,ic); A(j,ic) = 0.;
			for(k=0; k<dm; k++) A(j,k) -= A(ic,k) * a;
			for(k=0; k<dv; k++) V(j,k) -= V(ic,k) * a;
		}
	}
	return i;
#undef	A
#undef	V
}
/****************************************************************************/
int	Calc_Axes( LPLATTICE L, int f)
{
double	x,y;

	L->al = CABS(L->ax, L->ay);
	L->bl = CABS(L->bx, L->by);
	L->cl = CABS(L->cx, L->cy);
	L->dl = CABS(L->dx, L->dy);
	L->gamma = L->cdgamma = 0.;
	if(L->al < 3.) return 0;
	if(L->bl < 3.) return 1;

	x = ABANGLE(L->ax, L->ay, L->bx, L->by);
	if(f && (x > A90+GD)) {L->by = -L->by; L->bx = -L->bx; x = M_PI - x; }

	L->gamma = x;

	if(L->cl >= 3.) {
		if(L->dl >= 3.) {
			x = ABANGLE(L->cx, L->cy, L->dx, L->dy);
			if(f && (x > A90+GD)) {L->dy = -L->dy; L->dx = -L->dx; x = M_PI - x; }
			L->cdgamma = x;
			if (f) {
				x = ABANGLE(L->ax, L->ay, L->cx, L->cy);
				x = fabs(fabs(x - A90) - A90);
				y = ABANGLE(L->ax, L->ay, L->dx, L->dy);
				y = fabs(fabs(y - A90) - A90);
				if(x>y) {
					Swap(L->cx,L->dx);
					Swap(L->cy,L->dy);
					Swap(L->cl,L->dl);
				}
			}
			return 4;
		}
		return	3;
	}
	return	2;
}
/****************************************************************************/
int		Check_Latt( LPLATTICE L )
{
double	MC[4][4];
double	MS[4][2];
int		i, j, k, n=0;
	k = 0;
	for(i=0; i<4; i++) MS[i][0] = MS[i][1] = 0.;
	for(i=0; i<4; i++) {
		MC[k][0] = L->H[i];	MC[k][1] = L->K[i];
		MC[k][2] = L->h[i];	MC[k][3] = L->k[i];
		if (L->bDefRefl[i]) {
			MS[k][0] = L->pDefRefl[i].x;
			MS[k][1] = L->pDefRefl[i].y;
		} else {
			MS[k][0] = 0;
			MS[k][1] = 0;
		}
		if(	L->bDefRefl[i] && lrCheckRefl( L->ft2, &MS[k][0], &MS[k][1], -1 ))
			k++;
	}
	if(k >= 1) j = Lin_Equ(k, 4, 2, MC[0], MS[0]); else j=0;
	if(j<0) {i = -j; j = 0;} else {i=j; j=1;}
	if(i>=2) j=1;
	L->ax = MS[0][0];	L->ay = MS[0][1];
	L->bx = MS[1][0];	L->by = MS[1][1];
	L->cx = MS[2][0];	L->cy = MS[2][1];
	L->dx = MS[3][0];	L->dy = MS[3][1];
	L->num_dimensions = Calc_Axes(L, 0);
	return (j * L->num_dimensions);
}
/****************************************************************************/
static	int		h, k, l, m, rnoi, tcnt, gcnt, dcnt, lcnt;
static	double	fs, rm, maxd;
static	double	ax, ay, bx, by, cx, cy, dx, dy;
static	double	shx, shy, skx, sky, shh, skk, shk;

static _inline int Add_RP(LPLATTICE L, int f)
{
double	a, x0, y0, x, y;

	x = x0 = h*ax + k*bx + l*cx + m*dx;
	y = y0 = h*ay + k*by + l*cy + m*dy;
	if((fabs(x) >= fs) || (fabs(y) >= fs) )	return FALSE;
	if( ((a=CABS(x,y)) < 3.) || (a>rm) )	return FALSE;
	tcnt++;	lcnt++;
	lrMaxPeakPos(L->ft2, &x, &y, 2, NULL, NULL);
	if(Cr_Noise	  (L->ft2, rnoi,  x,  y) < 8.) return FALSE;
	lrCGPeakPos(L->ft2, &x, &y, 1, NULL, NULL);
	if(CABS(x-x0, y-y0) > maxd) { dcnt++; return FALSE;}
	if(f) {
		if(l|m) {
			x -= h*ax + k*bx; y -= h*ay + k*by;
			shx += l*x*a;	skx += m*x*a;	shy += l*y*a;	sky += m*y*a;
			shh += l*l*a;	skk += m*m*a;	shk += l*m*a;
		} else {
			shx += h*x*a;	skx += k*x*a;	shy += h*y*a;	sky += k*y*a;
			shh += h*h*a;	skk += k*k*a;	shk += h*k*a;
		}
//		PaintRefls( L, x, y);
//		PaintRefls( L, -x, -y);
		gcnt++;
	}
	return TRUE;
}
/****************************************************************************/
int		Latt_RefE(LPLATTICE L)
{
int		r,rx;
BOOL	bDtrm;	// Non zero determinanat
int		ndim = L->num_dimensions;
double	s;

	fs = L->ft2.S*.5-4;	rm = L->rad;
	ax  = L->ax; bx = L->bx; cx = L->cx; dx = L->dx;
	ay  = L->ay; by = L->by; cy = L->cy; dy = L->dy;
	if(ndim < 1) return FALSE;

	// The maximum misspalcement is 1/8 of the shortest axis
	// The rnoi (radius for noise estimation) is a 2/5 of the shortest axis
	maxd = L->al;
	if (L->bl>3.) maxd = __min(maxd, L->bl);
	if (L->cl>3.) maxd = __min(maxd, L->cl);
	if (L->dl>3.) maxd = __min(maxd, L->dl);
	rnoi = __max(round(maxd*.55),2);
	maxd *= .125; maxd = __max(maxd,2.);

	// The radius (measured as a diffraction order) at which a new lattice parameters
	// will be calculated after the cycle of refls collection.
	// If we have user define reflections, than start after the maximum order
	rx = 0;
	for(r=0; r<4; r++) if (L->bDefRefl[r]){
		rx = __max(L->H[r],rx);
		rx = __max(L->K[r],rx);
	}
	if(rx > 5) rx--;	// Seems to be a patch from SK

	h = k = l = m = tcnt = gcnt = dcnt = 0;
	r = 0;
	shx = shy = skx = sky = shh = skk = shk = 0.;
	do {
		r++;
		if(r>rm) break;
		lcnt = 0;
		k=-r;

		for(h=-r,k=-r; h<r;  h++) Add_RP(L, 1);
		for(h= r,k=-r; k<=r; k++) Add_RP(L, 1);

		s  = shk*shk - shh*skk;
		if(ndim>1) {
			bDtrm = (fabs(s) > 1.);
			// Calculating a new lattice parameters
			if( bDtrm && ((r>=rx) || (lcnt==0)) ) {
				ax = (skx*shk - shx*skk) / s;	bx = (shx*shk - skx*shh) / s;
				ay = (sky*shk - shy*skk) / s;	by = (shy*shk - sky*shh) / s;
				s = s;
			}
			if((CABS(ax,ay)<3.) || (CABS(bx,by)<3.)) return FALSE;
		} else {
			bDtrm = (fabs(shh) > 0.5);
			if (bDtrm) { ax = shx/shh;	ay = shy/shh;}
			bx = by = 0;
			if(CABS(ax,ay) < 3.) return FALSE;
		}
	} while(lcnt);

	if(!bDtrm) return FALSE;

	if(r>5) r=5;
	if(ndim>2) {
		shx = shy = skx = sky = shh = skk = shk = 0.;
		for(h=0; h<r; h++) for(k=-r; k<r; k++) {
			if((h==0) && (k<0)) continue;
			if(Add_RP(L, 0)) for(l=-1; l<=1; l++) for(m=-1; m<=1; m++) {
				if((l|m)==0) continue;
				Add_RP(L, 1);
			}
			if(ndim>3) {
				s  = shk*shk - shh*skk;
				bDtrm = (fabs(s) > 1.);
				if ( bDtrm ) {
					cx = (skx*shk - shx*skk) / s;	dx = (shx*shk - skx*shh) / s;
					cy = (sky*shk - shy*skk) / s;	dy = (shy*shk - sky*shh) / s;
				}
			}
			else if (shh > 0.5) { cx = shx/shh;	cy = shy/shh;}
		}
		L->cx = cx; L->cy = cy; L->dx = dx; L->dy = dy;
	}
	L->ax = ax; L->ay = ay; L->bx = bx; L->by = by;
	return Calc_Axes(L, 0);
}
/****************************************************************************/
int	Check_Centred(LPDOUBLE axp, LPDOUBLE ayp, LPDOUBLE bxp, LPDOUBLE byp, int * ns)
{
double	ax=*axp, bx=*bxp, ay=*ayp, by=*byp;
double	x, rax, rbx, ray, rby;
double	a[3];
int		v[3] = {0,1,2};
int		nx;
#define SwapA(i,j) {x=a[i]; a[i]=a[j]; a[j]=x; nx=v[i]; v[i]=v[j]; v[j]=nx;}

	a[0] = ABANGLE(2.*bx-ax, 2.*by-ay, ax,	   ay);
	a[1] = ABANGLE(2.*ax-bx, 2.*ay-by, bx,    by);
	a[2] = ABANGLE(ax+bx,	  ay+by,	ax-bx, ay-by);

	a[0] = fabs(A90-a[0]);	a[1] = fabs(A90-a[1]);	a[2] = fabs(A90-a[2]);

	if(a[0] > a[1]) SwapA(0,1);	if(a[0] > a[2]) SwapA(0,2);
	if(a[1] > a[2]) SwapA(1,2);

	for(nx=0; nx<3; nx++) if(a[nx]>GD) break;
	if (*ns > nx) *ns = nx;
	if (nx==0) return(0); /* nx : [1,2,3] */
	switch (v[(*ns) - 1])
	{
		case 0:	rax = ax*.5;		rbx	= bx-ax*.5;
				ray = ay*.5;		rby = by-ay*.5;	break;
		case 1:	rax = bx*.5;		rbx = ax-bx*.5;
				ray = by*.5;		rby = ay-by*.5;	break;
		case 2: rax = (ax-bx)*.5;	rbx = (ax+bx)*.5;
				ray = (ay-by)*.5;	rby = (ay+by)*.5;
	}

	if(CABS(rax, ray) > CABS(rbx, rby))
			{ ax = rbx; ay = rby; bx = rax; by = ray; }
	else	{ ax = rax; ay = ray; bx = rbx; by = rby; }
	if(ax < 0) {ax = -ax; ay = -ay;}
	if(by < 0) {by = -by; bx = -bx;}
	x = ABANGLE(ax, ay, bx, by);
	if(x > A90+GD) {by = -by; bx = -bx;}
	*axp = ax; *ayp = ay;
	*bxp = bx; *byp = by;
	return nx;
}
/****************************************************************************/
typedef struct {
	double x,y;
} AREFL, * LPAREFL;

typedef struct {
	double	dist;	// distance
	double	ang;	// Azimuth
	int		flg;	// Flags: 1- deleted due to high ESD
} ADIST, * LPADIST;

typedef struct {
	int		count;	// number of matches
	double	length;	// vector length
	double	ang;	// vector azimuth
	int		x,y;
} AHIST, * LPAHIST;

#define	MAXAREFL	128

static AREFL	aref[MAXAREFL];
static ADIST	adist[MAXAREFL*MAXAREFL];
static AHIST	ahist[MAXAREFL*MAXAREFL];
static double maxampl;
static int	maxcount;

static	double	mm[128][128*2];
/****************************************************************************/
static void _inline laAddRefl( LPLATTICE L, int h, int k, LPAREFL aref, LPINT cnt )
{
	double	x0,x, y0,y;
	double	shift,aver,esd, a;

	x0=x=h;
	y0=y=k;
	if(*cnt>=MAXAREFL)
		return;
	if (CABS(x,y)<3)
		return;
	if (!lrMaxPeakPos( L->ft2, &x, &y, 2, NULL, &aver))
		return;
	if (fabs(x-x0)>1. || fabs(y-y0)>1.)
		return;
	a = L->ft2.GetAmp( round(x), round(y));
	if (a<maxampl/25)
		return;
	if (a>maxampl)
		maxampl=a;
	if (!lrCGPeakPos( L->ft2, &x, &y, 3, &shift, &aver))
		return;
	if (shift>1.5)
		return;
	esd = lrAreaESD( L->ft2, x, y, 3, aver );
	if (a-aver>3.5*esd)	// 3*esd
	{
		aref[*cnt].x = x;
		aref[*cnt].y = y;
		(*cnt)++;
	}
}

/****************************************************************************/
int     adcmp( const void *va, const void *vb )
{
static int dd;
const LPADIST a = (LPADIST)va;
const LPADIST b = (LPADIST)vb;
static double	fd, fp, fa, fb;

	fd = (a->dist-b->dist);
	if (fabs(fd)<2.0) dd=0;
	else if (fd<0) dd=-1;
	else           dd=+1;
	if (dd) return dd;
	fa = sin(a->ang-b->ang)*(a->dist);
	if (fabs(sin(a->ang-b->ang)*(a->dist)) < 2.5) return 0;	// 2.5
	return round(fa);
}
/****************************************************************************/
int     adcmplen( const void *va, const void *vb )
{
const LPADIST a = (LPADIST)va;
const LPADIST b = (LPADIST)vb;
double	fd;

	fd = (a->dist-b->dist);
	return (round(fd*1e3));
}
/****************************************************************************/
int     adcmpang( const void *va, const void *vb )
{
const LPADIST a = (LPADIST)va;
const LPADIST b = (LPADIST)vb;

	return round(R2D(a->ang)-R2D(b->ang));
}

/****************************************************************************/
int     hicmp( const void *va, const void *vb )
{
const LPAHIST a = (LPAHIST)va;
const LPAHIST b = (LPAHIST)vb;
int		thr = maxcount/2;

	if (b->count>thr && a->count>thr)
		return round((a->length-b->length)*1e4);
	if (b->count>thr)
		return 1;
	if (a->count>thr)
		return -1;
	return 0;
}
/****************************************************************************/
static BOOL Latt_Auto( LPLATTICE L )
{
	static int		h,k,r, tcnt,i,j;
	ADIST	ad;
	AHIST	ah;
	double	phase;
	double	alen;
	int		count;
	int		minrad;
	double	xa, ya;
	double	ma, mi;
//double	esd, n, as;

	static int xx[MAXAREFL*MAXAREFL];

	//////////////////////////////////////////////////////////////////////////
	// Added by Peter on 17 Mar 2006
	int maxrad = __min(L->rad, L->ft2.S/2-4);
	//////////////////////////////////////////////////////////////////////////

	maxampl=0.; tcnt=0;
	minrad = 10;	// 15*L->ft2.S/256;
//	for(r=minrad;r<L->ft2.S/2-4;r+=3) {
	for(r = minrad; r < maxrad; r+=3)
	{
		k=-r;
		for(h=-r;   h<r;  h+=3) laAddRefl(L,h,k,aref,&tcnt);
		for(k=-r;   k<=r; k+=3) laAddRefl(L,h,k,aref,&tcnt);
		if(tcnt==MAXAREFL) break;
	}

	tcnt=0;
//	for(r=minrad;r<L->ft2.S/2-4;r+=3) {
	for(r = minrad; r < maxrad; r+=3)
	{
		k=-r;
		for(h=-r;   h<r;  h+=3) laAddRefl(L,h,k,aref,&tcnt);
		for(k=-r;   k<=r; k+=3) laAddRefl(L,h,k,aref,&tcnt);
		if(tcnt==MAXAREFL) break;
	}

	for(h=0;h<tcnt;h++) PaintRefls( L, aref[h].x, aref[h].y);

	memset( adist, 0, sizeof(adist) );
	for(i=0,r=0,h=0;i<tcnt;i++) for(j=i+1;j<tcnt;j++) {
		adist[r].dist = CABS(aref[i].x-aref[j].x, aref[i].y-aref[j].y);

		if (adist[r].dist < 4) continue;

//		phase = acos(fabs( (aref[i].x*aref[j].x + aref[i].y*aref[j].y) /
//			          (CABS(aref[i].x,aref[i].y) * CABS(aref[j].x,aref[j].y)) ));
//		if (phase>M_PI/6.) continue;

		// Skip vectors which crosses any reflection
		for(k=0;k<tcnt;k++) {
			if (k==i || k==j) continue;
			phase = fabs((aref[j].x-aref[i].x)*(aref[k].y-aref[i].y)-
					 (aref[j].y-aref[i].y)*(aref[k].x-aref[i].x));

			if (fabs((aref[j].x-aref[i].x)*(aref[k].y-aref[i].y)-
					 (aref[j].y-aref[i].y)*(aref[k].x-aref[i].x)) < 10) {
				if (fabs(aref[j].x-aref[i].x) > fabs(aref[j].y-aref[i].y)) {
					ma = __max(__max(aref[j].x,aref[i].x),aref[k].x);
					mi = __min(__min(aref[j].x,aref[i].x),aref[k].x);
					if (ma>aref[k].x && mi<aref[k].x) break;
				} else {
					ma = __max(__max(aref[j].y,aref[i].y),aref[k].y);
					mi = __min(__min(aref[j].y,aref[i].y),aref[k].y);
					if (ma>aref[k].y && mi<aref[k].y) break;
				}
			}
		}
		if (k<tcnt) continue;

		phase = ATAN2(aref[i].y-aref[j].y, aref[i].x-aref[j].x);
		while (phase<0) phase+=M_PI;
		while (phase>M_PI) phase-=M_PI;
		adist[r].ang  = phase;
		r++;
	}

	// Sorting on vector length
	qsort(adist,r,sizeof(ADIST),adcmplen);

	if (0) {
		FILE *ff=fopen("C:/cc.txt","w");
		for(i=0;i<r;i++)
			fprintf(ff,"%8.2f %8.0f\n", adist[i].dist, R2D(adist[i].ang));
		fclose(ff);
	}

	// Cut into pieces with ~equal length
	for(i=0,h=0,k=1;i<r-1;i++) {
		if (fabs(adist[i].dist-adist[i+1].dist)<0.5) {
			k++;
		} else {
			xx[h]=k;
			h++; k=1;
		}
	}

/*	// Remove all which are too far from average
	for(i=0,j=0,k=0;i<h;i++) {
		if (xx[i]>1) {
			for(k=j,alen=0;k<j+xx[i];k++) alen+=adist[k].dist;
			alen/=xx[i];
			n=0; esd=0;
			for(k=j;k<j+xx[i];k++) { as = fabs(alen-adist[k].dist); esd += as*as; n++; }
			esd = sqrt(esd/(n-1));
			for(k=j;k<j+xx[i];k++) if (esd*3<fabs(alen-adist[k].dist)) adist[k].flg=1;
		} else {
			// only one vector
			adist[j].flg=1;
		}
		j+=xx[i];
	}
*/
	// Sorting cuts on angle
	for(i=0,j=0;i<h;i++) {
		qsort(&adist[j],xx[i],sizeof(ADIST),adcmpang);
		j+=xx[i];
	}

	if (0) {
		FILE *ff=fopen("C:/bb.txt","w");
		for(i=0;i<r;i++)
			fprintf(ff,"%8.2f %8.0f %1d\n", adist[i].dist, R2D(adist[i].ang), adist[i].flg);
		fclose(ff);
	}

	// Collecting histogramm
	memset(ahist,0,sizeof(ahist));
	memset(&ad,0,sizeof(ADIST));

	maxcount=0;
	for(i=0,j=0,r=0;i<h;i++) {
bgn:
		for(k=j;k<j+xx[i];k++) if (adist[k].flg == 0) break;
		if (k==j+xx[i]) goto nxt;

		ad=adist[k];
		alen=adist[k].dist; count=1;
		xa = cos(adist[k].ang)*adist[k].dist;
		ya = sin(adist[k].ang)*adist[k].dist;
		for(k=j;k<j+xx[i];k++) if (adist[k].flg==0) {
			if (adcmp(&ad,&adist[k])==0) {
				adist[k].flg=1;	// used
				count++;
				alen+=adist[k].dist;
				if ( fabs( fabs(R2D(adist[k].ang)) - fabs(R2D(ATAN2(ya,xa))) ) <90) {
					xa += cos(adist[k].ang)*adist[k].dist;
					ya += sin(adist[k].ang)*adist[k].dist;
				} else {
					xa -= cos(adist[k].ang)*adist[k].dist;
					ya -= sin(adist[k].ang)*adist[k].dist;
				}
//				ad = adist[k];
			}
		}
		if (count) {
			maxcount=__max(maxcount,count);
			ahist[r].count=count;
			ahist[r].length=alen/count;
			ahist[r].ang = ATAN2(ya,xa);
			r++;
		}
		goto bgn;

nxt:	j+=xx[i];
	}

	// Sorting histogram
	qsort(ahist,r,sizeof(AHIST),hicmp);

	if (0) {
		FILE *ff=fopen("C:/aa.txt","w");
		for(i=0;i<j;i++)
			fprintf(ff,"%4d %8.2f %8.0f\n", ahist[i].count, ahist[i].length,
				R2D(ahist[i].ang));
		fclose(ff);
	}

	i=1;
	// Choose vectors with gamma > 10 degree
	ah = ahist[0];
	for(i=1;i<j;i++) {
		phase = fabs(P2D(R2P(ahist[i].ang)*2-R2P(ahist[0].ang)*2));
		if (phase>20.) break;
	}

	// Choose 'a' as shortest
	j=0;
	if (ahist[j].length > ahist[i].length) {
		r=j; j=i; i=r;
	}

	L->ax = ahist[j].length*cos(ahist[j].ang);
	L->ay = ahist[j].length*sin(ahist[j].ang);

	L->bx = ahist[i].length*cos(ahist[i].ang);
	L->by = ahist[i].length*sin(ahist[i].ang);

	L->cx = 0;	L->cy = 0;
	L->dx = 0;	L->dy = 0;

//	PaintRefls( L, ax, ay);
//	PaintRefls( L, bx, by);

	L->num_dimensions = Calc_Axes(L, 0);

	return FALSE;
}
/****************************************************************************/
int		rflLatticeRefine(LPLATTICE L, int go, int Mode)
{
	double	x;
	int		n=0, n1=0, n2=0;
	int 	f=100;
	#define	LMode L->LR_Mode

	if(go)
	{
		LMode &= ~LR_DONE;	L->num_dimensions = 0;
		if(LMode & LR_AUTO)	Latt_Auto(L);
		else	if((f=Check_Latt(L)) == 0) return FALSE;

		if(f>1 && !Latt_RefE(L))
		{
			if(LMode & LR_AUTO)	return FALSE;
			else MessageBox( MainhWnd, "Lattice not refined, will use user defined", "LATREF", MB_OK|MB_ICONINFORMATION );
		}
		LMode |= LR_DONE;

		if(LMode & LR_AUTO) {
			if(L->al > L->bl) { Swap(L->ax,L->bx); Swap(L->ay,L->by);}
			if(L->ax < 0) {L->ax = -L->ax; L->ay = -L->ay;}
			// changed by Peter on 10 Oct 2001
			if(L->by > 0) {L->by = -L->by; L->bx = -L->bx;}
//			if(L->by < 0) {L->by = -L->by; L->bx = -L->bx;}
			n = 1;
		}
		Calc_Axes(L, n);
		L->ax0 = L->ax; L->ay0 = L->ay; L->bx0 = L->bx; L->by0 = L->by;
		L->cx0 = L->cx; L->cy0 = L->cy; L->dx0 = L->dx; L->dy0 = L->dy;
	}
	if (!(LMode & LR_DONE)) return FALSE;
	L->ax = L->ax0; L->ay = L->ay0; L->bx = L->bx0; L->by = L->by0;
	L->cx = L->cx0; L->cy = L->cy0; L->dx = L->dx0; L->dy = L->dy0;

	if(Mode & ML_CENTAB) {
		n1 = (Mode & (ML_CENAB0 | ML_CENAB1))/ML_CENAB0;	/* 1,2,3 */
		n  = Check_Centred(&L->ax, &L->ay, &L->bx, &L->by, &n1);
		LMode = (LMode & ~(LR_CEXAB0 | LR_CEXAB1)) | (n*LR_CEXAB0);
	}
	if((Mode & ML_CENTab) && (L->cdgamma > 0.1)) {
		n2 = (Mode & (ML_CENab0 | ML_CENab1))/ML_CENab0;	/* 1,2,3 */
		n  = Check_Centred(&L->cx, &L->cy, &L->dx, &L->dy, &n2);
		LMode = (LMode & ~(LR_CEXab0 | LR_CEXab1)) | (n*LR_CEXab0);
	}

	if(Mode & ML_SWAPAB) { Swap(L->ax,L->bx); Swap(L->ay,L->by);}
	if(Mode & ML_SWAPab) { Swap(L->cx,L->dx); Swap(L->cy,L->dy);}
	if(Mode & ML_NEGA)	 { L->ax = -L->ax; L->ay = -L->ay;}
	if(Mode & ML_NEGB)	 { L->by = -L->by; L->bx = -L->bx;}
	if(Mode & ML_NEGa)	 { L->cx = -L->cx; L->cy = -L->cy;}
	if(Mode & ML_NEGb)	 { L->dy = -L->dy; L->dx = -L->dx;}

	L->num_dimensions = Calc_Axes(L, 0);

	rflGetRefls(L);

	return n1 | (n2<<2) | 0x80;
}
