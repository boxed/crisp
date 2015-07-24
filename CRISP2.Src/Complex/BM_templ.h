#pragma once

#include "complex_tmpl.h"

#ifndef	RDIS_ABS
#	define	RDIS_ABS	0x80000000
#	define	RDIS_AVR	0
#	define	RDIS_MIN	1
#	define	RDIS_MAX	2
#endif // RDIS_ABS

struct	SPOT
{
	FCOMPLEX p;
	float r;
	float a;
};

inline int SPOT_CmpA(const void * p1, const void * p2)
{
	if( ((SPOT*)p1)->a  < ((SPOT*)p2)->a) return 1;
	if( ((SPOT*)p1)->a  > ((SPOT*)p2)->a) return -1;
	return 0;
}

inline int SPOT_CmpAS(const void * p1, const void * p2)
{
	if( ((SPOT*)p1)->r  < ((SPOT*)p2)->r) return 1;
	if( ((SPOT*)p1)->r  > ((SPOT*)p2)->r) return -1;
	return 0;
}

inline int SPOT_CmpR(const void * p1, const void * p2)
{
	if( ((SPOT*)p1)->r  > ((SPOT*)p2)->r) return 1;
	if( ((SPOT*)p1)->r  < ((SPOT*)p2)->r) return -1;
	return 0;
}

template <typename _Tx>
class BM_templ
{
public:
	typedef _Tx					type;
	typedef type*				pointer;
	typedef BM_templ<float>		BMF;
	typedef BM_templ<double>	BMD;

	int		W;
	int		H;
	pointer	P;
	float	XC;
	float	YC;
	float	zoom;
	struct BMINFO
	{
		BITMAPINFOHEADER h;
		WORD index[256];
	} Info;

public:
	// ------------------ CONSTRUCTORS & DESTRUCTOR
	BM_templ()														{ Init();/*memset(this, 0, sizeof(BM));*/ }
	BM_templ(int w, pointer p=NULL)									{ Alloc( w, 1, 0.f, 0.f, p); }
	BM_templ(int w, int h, pointer p=NULL)							{ Alloc( w, h, 0.f, 0.f, p); }
	BM_templ(int w, int h, float xc, float yc, pointer p=NULL)		{ Alloc( w, h, xc, yc, p); }
	BM_templ(const ICOMPLEX& s, const FCOMPLEX& c, pointer p=NULL)	{ Alloc( s.i, s.j, c.i, c.j, p); }

	void Init()
	{
		W = H = 0;
		P = NULL;
		XC = YC = zoom = 0.0f;
		memset(&Info, 0, sizeof(Info));
	}

	virtual ~BM_templ() { Free(); }

	// ------------------ OPERATORS [], ()

	type&	operator[](int i)				{ return P[i]; }
	type&	operator[](const ICOMPLEX& x)	{ return P[(int)(x.i+XC) + (int)(x.j+YC) * W]; }
	type&	operator()(int x, int y)		{ return P[(int)(x+round(XC)) + (int)(y+round(YC)) * W]; }

	float	operator[](float x)
	{
		float	fontWidth = x + XC;
		int		ix = (int)fontWidth;
		float	a = P[ix], b = P[ix+1], kx = fontWidth-ix;
		return	a + (b-a)*kx;
	}
	float	operator()(float x, float y)
	{
		float	fontWidth = x+XC,	fontHeight = y+YC;
		int		ix = (int)fontWidth, iy = (int)fontHeight, p = ix + iy*W;
		float	a = P[p], b = P[p+1], c = P[p+W], d = P[p+W+1], kx = fontWidth-ix, ky = fontHeight-iy;
		a += (b-a)*kx;		c += (d-c)*kx;
		return  a + (c-a)*ky;
	}
	float	operator[](const FCOMPLEX& x)
	{
		return operator()(x.i, x.j);
	}
	void	PutPixel(const FCOMPLEX& x, float t)
	{
		float	fontWidth = x.i+XC,	fontHeight = x.j+YC;
		int		ix = (int)fontWidth, iy = (int)fontHeight, p = ix + iy*W;
		float	kx = fontWidth-ix, ky = fontHeight-iy;
		float	c = t*ky, a = t-c;
		P[p+1]	= t = a*ky;		P[p]  = a-t;
		P[p+1+W]= t = c*ky;		P[p+W]= c-t;
	}
	void	AddPixel(const FCOMPLEX& x, float t)
	{
		float	fontWidth = x.i+XC,	fontHeight = x.j+YC;
		int		ix = (int)fontWidth, iy = (int)fontHeight, p = ix + iy*W;
		float	kx = fontWidth-ix, ky = fontHeight-iy;
		float	c = t*ky, a = t-c;
		t = a*ky;	P[p+1]  += t;	P[p]  += a-t;
		t = c*ky;	P[p+1+W]+= t;	P[p+W]+= c-t;
	}
// #ifndef	_BMSP
// 	float	operator[](float x)
// 	{
// 		float	fontWidth = x+XC;
// 		int		ix = (int)fontWidth;
// 		float	a = P[ix], b = P[ix+1], kx = fontWidth-ix;
// 		return	a + (b-a)*kx;
// 	}
// 	float	operator()(float x, float y)
// 	{
// 		float	fontWidth = x+XC,	fontHeight = y+YC;
// 		int		ix = (int)fontWidth, iy = (int)fontHeight, p = ix + iy*W;
// 		float	a = P[p], b = P[p+1], c = P[p+W], d = P[p+W+1], kx = fontWidth-ix, ky = fontHeight-iy;
// 		a += (b-a)*kx;		c += (d-c)*kx;
// 		return  a + (c-a)*ky;
// 	}
// 	float	operator[](const FCOMPLEX& x)
// 	{
// 		return operator()(x.i, x.j);
// 	}
// 	void	PutPixel(const FCOMPLEX& x, float t)
// 	{
// 		float	fontWidth = x.i+XC,	fontHeight = x.j+YC;
// 		int		ix = (int)fontWidth, iy = (int)fontHeight, p = ix + iy*W;
// 		float	kx = fontWidth-ix, ky = fontHeight-iy;
// 		float	c = t*ky, a = t-c;
// 		P[p+1]	= t = a*ky;		P[p]  = a-t;
// 		P[p+1+W]= t = c*ky;		P[p+W]= c-t;
// 	}
// 	void	AddPixel(const FCOMPLEX& x, float t)
// 	{
// 		float	fontWidth = x.i+XC,	fontHeight = x.j+YC;
// 		int		ix = (int)fontWidth, iy = (int)fontHeight, p = ix + iy*W;
// 		float	kx = fontWidth-ix, ky = fontHeight-iy;
// 		float	c = t*ky, a = t-c;
// 		t = a*ky;	P[p+1]  += t;	P[p]  += a-t;
// 		t = c*ky;	P[p+1+W]+= t;	P[p+W]+= c-t;
// 	}
// #endif

	BOOL	Load(const BMF& bm, float gain = 1.0f);
	BOOL	Load(const BMD& bm, float gain = 1.0f);
	//----------------------------------------------------------------------
	BOOL	Alloc(int w, int h, float xc=0, float yc=0, void* p=NULL)
	{
		int i;
		W = w; H = h; XC = xc; YC = yc;	zoom=1;
		P = (NULL != p) ? (pointer)p : (pointer)GlobalAlloc(GPTR, W * H * sizeof(type));
// 		if(p)
// 			P = (B*)p;
// 		else
// 			P = (B*)GlobalAlloc(GPTR, W * H * sizeof(type));
		BITMAPINFOHEADER Bh = {40, W, -H, 1, 8, BI_RGB, W*H, 0, 0, 256, 256};
		Info.h = Bh;
		for(i = 0; i < 256; i++)
			Info.index[i] = i;
		return (P != NULL);
	}
	BOOL	ReAlloc(int w, int h, float xc=0, float yc=0, void* p=NULL)
	{
		Free();
		return Alloc(w, h, xc, yc, p);
	}
	void	Free(void)
	{
		if( NULL != P )
			GlobalFree(P);
		Init(); // memset(this, 0, sizeof(BM));
	}
	void	Clear(void){ memset(P, 0, W*H*sizeof(type));}

	BOOL	MulBlob(BMF& FLT, ICOMPLEX c0=0, int rmax=-1, int rflt=-1)
	{
		float	k, kr;
		ICOMPLEX p;
		pointer bp0, bp;
		int		x0,x1,y0,y1,r,xc=round(XC),yc=round(YC);

		if ((P==NULL)||(FLT.P==NULL)) return FALSE;
		if(rmax<0) rmax = round(sqrt((double)W*W+H*H)/2.);
		if(rflt<0) rflt = FLT.W-1; else rflt = __min(rflt, FLT.W-1);
		if(rmax) kr = (float)rflt/(float)rmax; else kr=0;

		x0 = __max(0-	  c0.i - xc, -rmax);
		y0 = __max(0-	  c0.j - yc, -rmax);
		x1 = __min(W-1- c0.i - xc, +rmax);
		y1 = __min(H-1- c0.j - yc, +rmax);
		bp0 = P + (y0+c0.j+yc)*W + (x0+c0.i+xc);
		for(p.j=y0; p.j<=y1; p.j++, bp0 += W)
		{
			for(bp=bp0, p.i=x0; p.i<=x1; p.i++, bp++)
			{
				if((k=abs(p)) > rmax) continue;
				r=round(kr*k);
				if(r>rflt) k = 0; else k = FLT.P[r];
				*bp *= k;
			}
		}
		return TRUE;
	}
	int		GetRDis (SPOT * Sp, int ri, const FCOMPLEX& pc,	SPOT* Sx=NULL)
	{
		int t,r,z;
		pointer ip;
		float a,as;
		FCOMPLEX p;

		if((P==NULL)||(Sp==NULL)) return 0;
		memset(Sp, 0, (ri+1) * sizeof(SPOT));
		p = round(pc + FCOMPLEX(XC,YC));
		r = __min( __min(p.i, W-p.i-1), __min(p.j, H-p.j-1) );
		r = __min(r, ri);
		if(r<=0) return 0;
		ip = P + round(p.i) + round(p.j-r)*W;

		for( p.j=-r; p.j<=r+0.1; p.j++, ip+=W)	for(p.i=-r; p.i<=r+0.1; p.i++){
			t = round(abs(p));
			if(t>r) continue;
			a = ip[round(p.i)];
			Sp[t].a += a;
			Sp[t].r += 1.;
			Sp[t].p += p*(double)a;

		}
		for(t=0; t<=r ; t++) { Sp[t].a /= Sp[t].r;	Sp[t].p /= Sp[t].r;}
		for(   ; t<=ri; t++) { Sp[t].a  = Sp[r].a;	Sp[t].p  = 0;}
		for(z=0; z< r;  z++)  if(Sp[z].a < Sp[z+1].a) break;
		for(p=0, as=0., t=0; t<z; t++){
			a   = Sp[t].a - Sp[z].a;
			p  += Sp[t].p * Sp[t].r;
			as += Sp[t].r * a;
			Sp[t].p /= a;
		}
		if(Sx != NULL) {Sx->a = Sp[0].a - Sp[z].a; Sx->r = as; Sx->p = p/as + pc;}
		return z;
	}
	float	GetRDis1(SPOT * Sp, int ri, const FCOMPLEX& pc)
	{
		int t,r,z;
		pointer ip;
		float a,as,r0=ri/2.;
		FCOMPLEX p;

		if((P==NULL)||(Sp==NULL)) return 0;
		memset(Sp, 0, (ri+1) * sizeof(SPOT));
		p = round(pc + FCOMPLEX(XC,YC));
		r = __min( __min(p.i, W-p.i-1), __min(p.j, H-p.j-1) );
		r = __min(r, ri);
		if(r<=0) return 0;
		ip = P + round(p.i) + round(p.j-r)*W;

		for( p.j=-r; p.j<=r+0.1; p.j++, ip+=W)	for(p.i=-r; p.i<=r+0.1; p.i++){
			t = round(abs(p));
			if(t>r) continue;
			if(t<r0) continue;
			a = ip[round(p.i)];
			Sp[t].a += a;
			Sp[t].r += 1.;
		}

		for(z=t=0; t<=r ; t++) { Sp[t].a /= Sp[t].r; z += Sp[t].r; Sp[t].r=0;}

		for( as=0, p.j=-r; p.j<=r+0.1; p.j++, ip+=W)	for(p.i=-r; p.i<=r+0.1; p.i++){
			t = round(abs(p));
			if(t>r) continue;
			if(t<r0) continue;
			a = ip[round(p.i)];
			as += fabs(Sp[t].a - a);
		}

		as /= z;
		return as;
	}

	int		CheckSpot(SPOT& p, int s, const ICOMPLEX& x0, int f=0)
	{
		float a;
		ICOMPLEX x,xc;

		for(p.a = 0.9*(*this)[x0], s++, x.j=-s; x.j<=s; x.j++)
			for(x.i =-s; x.i <= s; x.i++){
				a = (*this)[x+x0];
				if(a > p.a) { p.a = a; xc = x;}
			}
			s--;
			if((xc.i < -s) || (xc.j  <-s) || (xc.i >= s)  || (xc.j >= s) ) return FALSE;
			xc += x0;	p.p = xc;	p.r = abs(xc);
			if(f){
				a = p.a * (2*s+1)*(2*s+1);
				for(x.j=-s; x.j<=s; x.j++) for(x.i =-s; x.i <= s; x.i++) a -= (*this)[x+xc];
				if(a<=0)	return FALSE;
				p.a = a;
			}
			return TRUE;
	}
	int		CheckSpot1(SPOT& p, int s, const ICOMPLEX& x0, int f=0)
	{
		ICOMPLEX x,xc;
		int	q = s*s;
		p.a = (*this)[x0];	p.p = x0;
		for(x.j=-s; x.j<=s; x.j++)  for(x.i =-s; x.i <= s; x.i++){
			if(x.i*x.i+x.j*x.j > q) continue;
			if( ((x.i|x.j) != 0) && ((*this)[x+x0] > p.a) ) return FALSE;
		}
		return TRUE;
	}
	float	GetSpotSN(SPOT& p, int s)
	{
		ICOMPLEX x, x0 = round(p.p);
		float as=0, an=0, a = (*this)[x0];
		int q=0;

		for(x.j=-s; x.j<=s; x.j++)	for(x.i =-s; x.i <= s; x.i++)
			if(norm(x) > 3) { as += (*this)[x+x0]; q++; }
			as /= q;
			for(x.j=-s; x.j<=s; x.j++)	for(x.i =-s; x.i <= s; x.i++)
				if(norm(x) > 3) an += (as - (*this)[x+x0] ) * (as - (*this)[x+x0] );
				an /= q-1;
				an = sqrt(an);
				return fabs( (a-as) / an );
	}
	float	GetSpotSN1(SPOT& p, int s)
	{
		ICOMPLEX x, z, x0 = round(p.p);
		float as=0, a = (*this)[x0], a0, d;
		float q=0;

		for(x.j=-s; x.j<=s; x.j++)	for(x.i =-s; x.i <= s; x.i++){
			if(norm(x) < 2) continue;
			a0 = (*this)[x+x0];
			for(z.j=-s; z.j<=s; z.j++)	for(z.i =-s; z.i <= s; z.i++){
				if(norm(z) < 2) continue;
				d = a0 - (*this)[z+x0];
				as += abs(d);
				q += 1;
			}
		}
		if(q==0) return 0;
		as /= q;

		if(as < 1e-6) return 1e6;
		return fabs( a / as );
	}
	int		RefineSpotCenter(SPOT& Sp, int rm, int ra, float thr)
	{
		ICOMPLEX p,p0=Sp.p;
		FCOMPLEX x=0;
		float	a, at, as;
		int		i, n, f;

		f = __min(p0.i+XC, p0.j+YC);
		i = __min(W - p0.i - XC - 1, H - p0.j - YC - 1);
		i = __max(0, __min(rm, __min(i,f)));
		if(rm != i)
			rm = i;

		Sp.a = a = (*this)[p0];	at = a*thr;	as = a; n=1; f=1;
		for(p.i=1; (f>0)&&(p.i<rm); p.i++)
			for(f=0, p.j=-p.i; p.j<p.i; p.j++)
				if(sqr(p) <= rm*rm)
					for(i=0; i<4; i++, p *= ICOMPLEX(0,1)){

						if((a=(*this)[p+p0]) > at){
							x += p*a;
							as += a; n++; f++;
						}
					}
					if(n==0) return 0;
					x /= as; Sp.p = x+p0;
					if(ra){
						p0 = round(Sp.p);
						for(Sp.a=0, p.j=p0.j-ra; p.j<=p0.j+ra; p.j++)
							for(p.i=p0.i-ra; p.i<=p0.i+ra; p.i++) Sp.a += (*this)[p];
					}
					return n;
	}
};

// template specialization for FLOAT & DOUBLE binary matrix
// the following functions cannot be defined for FLOAT matrix

template<>
inline BOOL		BM_templ<float>::Load(const BMF& bm, float gain/* = 1.0f*/) { return FALSE; }

template<>
inline BOOL		BM_templ<double>::Load(const BMD& bm, float gain/* = 1.0f*/) { return FALSE; }

// template specialization for SPOT binary matrix
// the following functions cannot be defined for SPOT matrix

template<>
inline float	BM_templ<SPOT>::operator[](float x) { return 0.0f; }

template<>
inline float	BM_templ<SPOT>::operator()(float x, float y) { return 0.0f; }

template<>
inline float	BM_templ<SPOT>::operator[](const FCOMPLEX& x) { return 0.0f; }

template<>
inline void		BM_templ<SPOT>::PutPixel(const FCOMPLEX& x, float t) { }

template<>
inline void		BM_templ<SPOT>::AddPixel(const FCOMPLEX& x, float t) { }

template<>
inline BOOL		BM_templ<SPOT>::Load(const BMF& bm, float gain/* = 1.0f*/) { return FALSE; }

template<>
inline BOOL		BM_templ<SPOT>::Load(const BMD& bm, float gain/* = 1.0f*/) { return FALSE; }

template<>
inline BOOL		BM_templ<SPOT>::MulBlob(BMF& FLT, ICOMPLEX c0/*=0*/, int rmax/*=-1*/, int rflt/*=-1*/) { return FALSE; }

template<>
inline float	BM_templ<SPOT>::GetRDis1(SPOT * Sp, int r, const FCOMPLEX& pc) { return 0.0f; }

template<>
inline int		BM_templ<SPOT>::GetRDis (SPOT * Sp, int r, const FCOMPLEX& pc,	SPOT* Sx/*=NULL*/) { return 0; }

template<>
inline int		BM_templ<SPOT>::CheckSpot(SPOT& p, int s, const ICOMPLEX& x0, int f/*=0*/) { return 0; }

template<>
inline int		BM_templ<SPOT>::CheckSpot1(SPOT& p, int s, const ICOMPLEX& x0, int f/*=0*/) { return 0; }

template<>
inline float	BM_templ<SPOT>::GetSpotSN(SPOT& p, int s) { return 0.0f; }

template<>
inline float	BM_templ<SPOT>::GetSpotSN1(SPOT& p, int s) { return 0.0f; }

template<>
inline int		BM_templ<SPOT>::RefineSpotCenter(SPOT& Sp, int rm, int ra, float thr) { return 0; }

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

typedef BM_templ<float>::BMF	BMF;
typedef BM_templ<double>::BMD	BMD;
typedef BM_templ<BYTE>		BM8;
typedef BM_templ<CHAR>		BM8S;
typedef BM_templ<WORD>		BM16;
typedef BM_templ<SHORT>		BM16S;
typedef BM_templ<DWORD>		BM32;
typedef BM_templ<LONG>		BM32S;
typedef BM_templ<SPOT>		BMSP;
