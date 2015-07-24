/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.DMapUtil * by ML **************************************************/
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

// TODO: move to header file
#define ALIGN_DWORD(x)		((x+3)&0xFFFC)

// commented by Peter on 10 Apr 2002 (optim - off everywhere here)
// since in Release mode it affects in strange way on the Create_DMap() rdr flag!
//#ifdef _DEBUG
#pragma optimize ("",off)
//#endif

/****************************************************************************/
void	DMap_To_Img( LPSGINFO SI, LPVOID img, int imgw, int imgh)
{
	LPBYTE	map = SI->dmap;
	int		neg, x_a, x_b, y_a, y_b, a0, b0, imgx;
	double	k, ka, kb, ks, g = SI->rgamma;
	double	n = SI->scale;
	double	cg	= cos(g);
	double	sg	= sin(g);
	double	inv_sg	= 1.0 / sg;
	double	w	= SI->rot;
	double	shx	= SI->shx;
	double	shy = SI->shy;
	double	cw	= cos(w);
	double	sw	= sin(w);
	double	imgs;

	// DWORD align
	imgx = ALIGN_DWORD(imgw);
	// changed by Peter on 27 Aug 2001
	imgs = __min(imgw, imgh);
//	if(imgw < imgh) imgs = (double)imgw; else imgs = (double)imgh;

	ks = n * 64. / (double)imgs;
	if(SI->ral > SI->rbl) { ka = 1.; kb = SI->ral / SI->rbl;}
	else				  { kb = 1.; ka = SI->rbl / SI->ral;}

	k = SI->ral/64. * ka * ks; ks *= 256.;
	shx	/= k;
	shy /= k;

	x_a =   round(ks*ka*( cw - sw * cg * inv_sg));
	x_b =   round(ks*kb*( sw * inv_sg));
	y_a =  -round(ks*ka*( sw + cw * cg * inv_sg));
	y_b =   round(ks*kb*( cw * inv_sg ));	// signs convoluted by Peter on 27 Aug 2001
//	y_b =  -round(ks*kb*(-cw * inv_sg));

	a0 = 32*256-x_a * (imgw/2 + round(shx)) - y_a*(imgh/2 + round(shy));
	b0 = 32*256-x_b * (imgw/2 + round(shx)) - y_b*(imgh/2 + round(shy));
	if (SI->flg & DM_NEG) neg = 255; else neg = 0;

	X64_Copy(map, img, imgx, imgh, x_a, y_a, x_b, y_b, a0, b0, neg);
}
/****************************************************************************/
void	DMap_To_Img256(LPSGINFO pSI, LPVOID img, int imgw, int imgh)
{
	LPBYTE	map = pSI->dmap;
	int		neg, x_a, x_b, y_a, y_b, a0, b0, imgx, imgw_2 = imgw >> 1, imgh_2 = imgh >> 1;
	double	k, ka, kb, ks, g = pSI->rgamma;
	double	scale = pSI->scale;
	double	cos_g	= cos(g);
	double	sin_g	= sin(g), inv_sin_g = 1.0 / sin_g;
	double	rot_a	= pSI->rot;
	double	shx	= pSI->shx;
	double	shy = pSI->shy;
	double	cos_rot	= cos(rot_a);
	double	sin_rot	= sin(rot_a);
	double	imgs;

	// DWORD align
	imgx = (imgw + 3)&0xFFFC;
	// changed by Peter on 27 Aug 2001
	imgs = __min(imgw, imgh);

//	if(imgw < imgh) imgs = (double)imgw; else imgs = (double)imgh;

	// mul by 256 to make fixed point
	ks = scale * 256. / (double)imgs;

	if(pSI->ral > pSI->rbl)
	{
		ka = 1.;
		kb = pSI->ral / pSI->rbl;
	}
	else
	{
		kb = 1.;
		ka = pSI->rbl / pSI->ral;
	}

	k = pSI->ral * ka * ks / 256.;
	ks *= 256.;
	shx	/= k;
	shy /= k;

	// increment-vector (1, 0) along a*-axis
	x_a =   round(ks*ka*( cos_rot - sin_rot * cos_g * inv_sin_g));
	x_b =   round(ks*kb*( sin_rot * inv_sin_g));
	// increment-vector (0, 1) along b*-axis
	y_a =  -round(ks*ka*( sin_rot + cos_rot * cos_g * inv_sin_g));
	y_b =   round(ks*kb*( cos_rot * inv_sin_g ));	// signs convoluted by Peter on 27 Aug 2001
//	y_b =  -round(ks*kb*(-cos_rot / sin_g));

	a0 = (int)(0x8000 - x_a * (imgw_2 + round(shx)) - y_a*(imgh_2 + round(shy)));
	b0 = (int)(0x8000 - x_b * (imgw_2 + round(shx)) - y_b*(imgh_2 + round(shy)));

	// changed by Peter on 27 Aug 2001
//	if (pSI->flg & DM_NEG) neg = 255; else neg = 0;
	neg = (pSI->flg & DM_NEG) ? 255 : 0;

	X256_Copy(map, img, imgx, imgh, x_a, y_a, x_b, y_b, a0, b0, neg);
}
/****************************************************************************/
void	RMap_To_Img(LPLATTICE L, LPSGINFO SI, LPVOID img, int imgw, int imgh)
{
LPBYTE	map = SI->rmap;
int		imgx, x_a, x_b, y_a, y_b, a0, b0;
double	kx, ky;
double	n = 1.;

	imgx = (imgw+3)&0xFFFC;

	if(map==NULL) return;
	kx = n * 256. * 64. / (double)imgw;
	ky = n * 256. * 64. / (double)imgh;

	x_a =  round(kx);
	y_a =  0;
	x_b =  0;
	y_b =  round(ky);

	a0 = -(x_a*imgw + y_a*imgh) / 2;
	b0 = -(x_b*imgw + y_b*imgh) / 2;

	X64_Copy(map, img, imgx, imgh, x_a, y_a, x_b, y_b, a0, b0, 0);
}
/****************************************************************************/
void	Get_CSym(LPLATTICE L, int w, int h, int * xc, int * yc)
{
LPSGINFO SI = L->SI+L->Spg;
double	x0=0., y0=0.;
double	kx, ky;
PHASE   bh = SI->Bh, bk = SI->Bk;

	if(SI->flg & OR_MAN) { bh += SI->Sh; bk += SI->Sk; }

	kx = (double)w/360.; ky = (double)h/360.;
	x0  = P2D(bh);      y0  = P2D(bk);
	x0 += P2D(SI->Ch);	y0 += P2D(SI->Ck);
	x0  = P2D(D2P(x0));	y0  = P2D(D2P(y0));

	*xc = round(w*.5 + x0*kx);
	*yc = round(h*.5 + y0*ky);
}
/****************************************************************************/
int		Set_CSym(LPLATTICE L, int w, int h, int xc, int yc, BOOL any)
{
static	int scn[2][NUMSYMM+1] ={
	{0, 0, 4, 1,1, 1,1, 1,1, 4, 4,4, 4, 4, 4, 4, 4, 3, 3, 0, 0, 0},
	{0, 0, 4, 1,1, 1,1, 1,1, 4, 4,4, 4, 4, 2, 2, 2, 3, 3, 0, 0, 0}
};

LPSGINFO SI = L->SI+L->Spg;
int		f = 0;
double	x0=0., y0=0.;
double	kx, ky;
int m = 0;

	if(L->OR_Mode & OR_RELAT) m++;
	kx = (double)w/360.; ky = (double)h/360.;
	x0  = P2D(SI->Bh);  y0  = P2D(SI->Bk);
	x0 += P2D(SI->Ch);	y0 += P2D(SI->Ck);
	x0 += (w/2-xc)/kx;	y0 += (h/2-yc)/ky;
	x0  = P2D(D2P(x0));	y0  = P2D(D2P(y0));

	if (any) {
		SI->Ck = 0; SI->Ch = 0;
		SI->Bh = -D2P((w/2-xc)/kx);
		SI->Bk = -D2P((h/2-yc)/ky); f++;
	} else {
		if(scn[m][L->Spg] == 0) return FALSE;
		switch(scn[m][L->Spg]){
			case 1:	if(fabs(x0) > 90.) SI->Ch += P_PI;
					SI->Ck -= D2P(y0); f++;
					break;
			case 2:	if((fabs(x0) > 90.) && (fabs(y0) > 90.))
					  { SI->Ch += P_PI; SI->Ck += P_PI; f++;}
					break;
			case 3:	if((fabs(x0) > 60.) && (fabs(y0) > 60.))
					{
						if((x0>0) && (y0<0)) { SI->Ch -= P_3PI2; SI->Ck += P_3PI2; f++;}
						if((x0<0) && (y0>0)) { SI->Ch += P_3PI2; SI->Ck -= P_3PI2; f++;}
					}
					break;
			case 4:	if(fabs(x0) > 90.) { SI->Ch += P_PI; f++;}
					if(fabs(y0) > 90.) { SI->Ck += P_PI; f++;}
					break;
		}
	}
	return f;
}
/****************************************************************************/
/*
void	Expand_All( LPLATTICE L)
{
	REFL	R;
	double	a, p;

	LPVOID	ifft = L->ifft;
	int		xs	 = L->ixs;
	int		h, k, xs2 = xs/2;

	spg = L->Spg-1;	if(spg <= 1) return;
	wstar = L->ws;
	for(h=0; h<xs2; h++)	for(k=-xs2+1; k<xs2; k++)
	{
		if((h==0) && (k<=0)) continue;
		Get_Amp(ifft, xs, h, k, &a);
		if(a < 1.)
		{
			R.h = h;	R.k = k;	R.f = 0;
			asym1(&R);
			if( (abs(R.h) < xs2) && (abs(R.k) < xs2) )
			{
				Get_AP(ifft, xs, R.h, R.k, &a, &p);
				if(a > 1.)
				{
					if(R.f & RIP1) p = -p;
					if(R.f & RIP2) p = p + M_PI;
					Put_AP(ifft, xs, h, k, a, p);
				}
			}
		}
	}
}
*/
/***************************************************************/
BOOL CALLBACK fnSetIFFTdata( UINT y, UINT cnt, LPVOID lpSrc, LPARAM lParam )
{
LPSGINFO	SI = (LPSGINFO)lParam;

    memcpy( SI->dmap+y*SI->dxs, lpSrc, cnt);
    return TRUE;
}
/****************************************************************************/
void	Create_DMap( LPLATTICE L, int flg)
{
LPSGINFO pSI = L->SI+L->Spg;
LPREFL	R = L->rptr;
int		h, k, i;
int		bh = pSI->Bh+pSI->Ch, bk = pSI->Bk+pSI->Ck;

double	thr		= L->Thr /* * L->MaxAmp/100.*/ ;
double	scale	= 1000.;
int		xs		= pSI->dxs;
// removed by Peter on 10 Apr 2002 - reorder the map flag must be always
// present since this places origin in the correct place
//int		rdr = 1;
PHASE	p, p0;
int		xs2 = xs/2, f;
FT2R	ft2;
FCOMPLEX	fontWidth;

// removed by Peter on 9 Aug 2001 - reorder the map flag
//	if(pSI->flg & DM_REOR) rdr = 0; else rdr = 1;
	if(pSI->flg & OR_MAN) { bh += pSI->Sh; bk += pSI->Sk; }
	if((pSI->flg & OR_DONE) == 0) return;
	ft2 = FT2R(xs);// return;
	memset(ft2.BX.P, 0, xs*xs*sizeof(float));

//	if (L->rflMaxAmp>1.) scale = 160000000./L->rflMaxAmp;
//	else scale = 1000000.;
//	scale *= (xs/64)*(xs/64);
//	if (scale*L->rflMaxAmp > 256000000.) scale = 256000000./L->rflMaxAmp;

	if(flg)
	{
		for(i=0;i<L->nrefl;i++,R++)
		{
			if(LM0(R)) continue;
			h = R->h0; k = R->k0;
			if( (abs(h) >= xs2) || (abs(k) >= xs2) ) continue;
			if( (f=R->f) & (SABS | UDEL) )	continue;
			if( (R->a < thr) && ((f & UADD)==0) )   continue;
			p = R->p;
			if(f & RIP1) p = -p;	if(f & RIP2) p += P_PI;
// rdr removed by Peter on 10 Apr 2002 - reorder the map flag is always 1
			if((h ^ k) & 1/*rdr*/) p += P_PI;
			ft2.PutAP( h, k, R->a*scale, P2R(p));
		}
	}
	else
	{
		for(i=0;i<L->nrefl;i++,R++)
		{
			if(LM0(R)) continue;
			h = R->h0; k = R->k0;
			if( (h >= xs2) || (k >= xs2) || (k <= -xs2) ) continue;
			if( R->a0 < thr)     continue;
			p0 = R->p0 + h*bh + k*bk;
// rdr removed by Peter on 10 Apr 2002 - reorder the map flag is always 1
			if((h ^ k) & 1/*rdr*/) p0 += P_PI;
			ft2.PutAP( h, k, R->a0*scale, P2R(p0));
//		fontWidth.i = R->a0*scale*sin(P2R(p0));
//		fontWidth.j = R->a0*scale*cos(P2R(p0));
//		ft2.PutVal( h, k, fontWidth );
		}
	}

//------------------------------------------------------
// TODO: something is missed here
	if(flg)	; // Expand_All(L);
	else	pSI = L->SI+1;
//------------------------------------------------------
	BM8	bmdst(pSI->dxs,pSI->dxs,pSI->dmap);
	ft2.IFT( );
	ft2.DisplayIFT( bmdst, 1. );
	bmdst.P = NULL;
}
/*======================================================*/
/*======================================================*/
void	Fract2Client(LPSGINFO SI, int imgw, int imgh, double af, double bf, LPLONG xc, LPLONG yc)
{
double	x_a, x_b, y_a, y_b, a0, b0, as, bs;
double	k, ka, kb, ks, g = SI->rgamma;
double	n = SI->scale;
double	cg	= cos(g);
double	sg	= sin(g);
double	w	= SI->rot;
double	shx	= SI->shx;
double	shy = SI->shy;
double	cw	= cos(w);
double	sw	= sin(w);
double	imgs;
int		dd = SI->dxs;

	if(imgw < imgh) imgs = (double)imgw; else imgs = (double)imgh;

	ks = n * dd / (double)imgs;
	if(SI->ral > SI->rbl) { ka = 1.; kb = SI->ral / SI->rbl;}
	else				  { kb = 1.; ka = SI->rbl / SI->ral;}

	k = SI->ral/dd * ka * ks;	shx	/= k; shy /= k;

	x_a = ks*ka*( cw - sw * cg / sg);
	y_a = -ks*ka*( sw + cw * cg / sg);
	x_b = ks*kb*( sw / sg);
	y_b = -ks*kb*(-cw / sg);

	a0 = - ( x_a*(imgw*.5 + shx) + y_a*(imgh*.5 + shy) );
	b0 = - ( x_b*(imgw*.5 + shx) + y_b*(imgh*.5 + shy) );

	as = (af-0.5)*dd - a0; bs =  (bf-0.5)*dd - b0;
	a0 = (as*y_b - bs*y_a) / (x_a*y_b - x_b*y_a);
	b0 = (as*x_b - bs*x_a) / (x_b*y_a - x_a*y_b);

	*xc = round(a0 + 1e-4);
	*yc = round(b0 + 1e-4);
}
/*=========================================================*/
/*=========================================================*/
/*=========================================================*/
#define	AAs	2
#define	AAm	4
static	double	aa[AAs*2+1][AAs*2+1];
/*---------------------------------------------------------*/
int		Get_DSCenter(double * xc, double * yc)
{
extern  void    Conv5x5(LPDOUBLE inp, LPDOUBLE out);

double	a, b, c, d, p, q, x1, y1;

int		x0 = round(*xc), y0 = round(*yc), err = 0;

static	double	kk[8];

#define	A0	kk[0]
#define	B0	kk[1]
#define	C0	kk[2]
#define	D0	kk[3]
#define	A1	kk[4]
#define	B1	kk[5]
#define	C1	kk[6]
#define	D1	kk[7]


	Conv5x5(&aa[0][0], &kk[0]);

	if( fabs(B0) < 0.03*fabs(aa[AAs][AAs]) ) return(0);
	if( fabs(C1) < 0.03*fabs(aa[AAs][AAs]) ) return(0);

	a = C0*D1 - C1*D0;
	b = A0*D1 - A1*D0 - B0*C1 + B1*C0;
	c = A0*B1 - A1*B0;

	x1 = y1 = 1234.;

	if(fabs(a) < 0.0001)
	{
		if(fabs(b) < 0.0001) err = 1;
		else	{ d = 0.;	y1 = -c/b;}
	}
	else
	{
		y1 = -b/a*.5;	d  = y1*y1 - c/a;
		if(d < 0) err = 2;
		else	{ d = sqrt(d);	y1-= d;}
	}

	if(fabs(y1) > fabs(y1+2.*d)) y1 += 2.*d;

	p = (B0 + B1) + (D0 + D1) * y1;
	q = (A0 + A1) + (C0 + C1) * y1;
	if(fabs(p) < 0.0001) err = 3; else	x1 = -q/p;

	if(fabs(x1) > 2.5) err = 4;
	if(fabs(y1) > 2.5) err = 5;
	if(err) return(0);

	if(fabs(x1) > 1.4) x1 *= (0.7/fabs(x1) + 0.5);
	if(fabs(y1) > 1.4) y1 *= (0.7/fabs(y1) + 0.5);
	*xc = x0 + (x1*.5)*AAm;	*yc = y0 + (y1*.5)*AAm;

	return(1);
}
/*======================================================*/
/*======================================================*/
double	Get_Density(LPBYTE map, int neg, double a, double b, int dd)
{
#define	dens(b, a) (double)(map[(((b)+(dd/2))&(dd-1))*dd + (((a)+(dd/2))&(dd-1))] ^ neg)
int		a0 = (int)floor(a), b0 = (int)floor(b);
double	d00 = dens(b0,   a0  );
double	d01 = dens(b0,   a0+1);
double	d10 = dens(b0+1, a0  );
double	d11 = dens(b0+1, a0+1);
double	s0, s1, d;

	s0 = d00 + (d01-d00) * (a-a0);
	s1 = d10 + (d11-d10) * (a-a0);
	d  = s0  + (s1 - s0) * (b-b0);
	return(d*999./255.);
}
/*======================================================*/
/*======================================================*/
int		Get_DCenter(LPSGINFO SI, int imgw, int imgh,
					double * xc, double * yc, double * af, double * bf,
					double * dns0, double * dns1, int flg)
{
LPBYTE	map = SI->dmap;
int		neg = 0, i = 0, j = 0, a = 0, b = 0, am = 0, bm = 0, r=4;
double	__max, x, y, d, x_a, x_b, y_a, y_b, a0, b0, as, bs;
double	k, ka, kb, ks, g = SI->rgamma;
double	n = SI->scale;
double	cg	= cos(g);
double	sg	= sin(g);
double	w	= SI->rot;
double	shx	= SI->shx;
double	shy = SI->shy;
double	cw	= cos(w);
double	sw	= sin(w);
double	imgs;
double * ap = &aa[0][0];
int		dd = SI->dxs;

	if(imgw < imgh) imgs = (double)imgw; else imgs = (double)imgh;

	ks = n * dd / (double)imgs;
	if(SI->ral > SI->rbl) { ka = 1.; kb = SI->ral / SI->rbl;}
	else				{ kb = 1.; ka = SI->rbl / SI->ral;}

	k = SI->ral/dd * ka * ks;
	shx	/= k; shy /= k;

	x_a = ks*ka*( cw - sw * cg / sg);
	y_a = -ks*ka*( sw + cw * cg / sg);
	x_b = ks*kb*( sw / sg);
	y_b = -ks*kb*(-cw / sg);

	a0 = - ( x_a*(imgw*.5 + shx) + y_a*(imgh*.5 + shy) );
	b0 = - ( x_b*(imgw*.5 + shx) + y_b*(imgh*.5 + shy) );

	if (SI->flg & DM_NEG) neg = 0; else neg = 255;
	x = *xc; y = *yc; __max = 0.;
	as = x * x_a + y * y_a + a0;
	bs = x * x_b + y * y_b + b0;
	*af = as/dd+0.5; *bf = bs/dd+0.5;
	a = round(as); b = round(bs);
	*dns0 = *dns1 = Get_Density(map, neg, as, bs, dd);


	for(i=b-r; i<=b+r; i++)
		for(j=a-r; j<=a+r; j++)
		{
			d = (double)(map[((i+(dd/2))&(dd-1))*dd + ((j+(dd/2))&(dd-1))] ^ neg);
			if(d > __max) {__max = d; am = j; bm = i;}
            if((d==__max)&&(abs(j)<abs(am))) am = j;
            if((d==__max)&&(abs(i)<abs(bm))) bm = i;
		}
	for(i=bm-AAs*AAm; i<=bm+AAs*AAm; i+=AAm)
		for(j=am-AAs*AAm; j<=am+AAs*AAm; j+=AAm)
			*(ap++) = (double)( map[((i+dd/2)&(dd-1))*dd + ((j+dd/2)&(dd-1))] ^ neg);
/*
	for(i=0; i<=4; i++) { aa[i][0] /=2.; aa[i][4] /=2.; aa[0][i] /=2.; aa[4][i] /=2.;}
*/
	as = am; bs = bm;
	if(flg) {
		i = Get_DSCenter(&as, &bs);
		*dns1 = Get_Density(map, neg, as, bs, dd);
		*af = as/dd+0.5; *bf = bs/dd+0.5;
		as -= a0; bs -= b0;
		x = (as*y_b - bs*y_a) / (x_a*y_b - x_b*y_a);
		y = (as*x_b - bs*x_a) / (x_b*y_a - x_a*y_b);
		*xc = x; *yc = y;
		return	i;
	}
	return FALSE;
}
