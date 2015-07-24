/**************************************************************************************/
#ifndef _FT2REAL_
#define _FT2REAL_

#include "complex_tmpl.h"
#include "bm_templ.h"
#include "ICalc.h"
//	#define _Cd __cdecl // stdcall cdecl fastcall emit
//	#pragma intrinsic(atan, exp, log10, sqrt, atan2, log, sin, tan, cos, fabs)
//	#pragma optimize("agwsy",on)

//class FT2R;
//####################################################################################
class  FT2R
{
public:
	int		S;
	int		Ln2S;
	float	MaxVal;
	int		iDC;
	int		iScale;
	int		iLinear;
	int		iParab;
	int		fDoMaxVal:1;
	int		iScaling:1;
	DCOMPLEX*		cst;
	BMF		BX;
// ------------------ CONSTRUCTORS & DESTRUCTOR
	FT2R() { memset(this, 0, sizeof(FT2R));}
	FT2R(int s, void * p=NULL)	{ Alloc(s, p);}

	~FT2R(){ Free();}

// ------------------ OPERATORS
	FT2R& operator=(const FT2R& FT)
	{
		if( (S != FT.S) || (cst==NULL) || (BX.P==NULL) ){
			Free();
			Alloc(FT.S, NULL);
		}
		if(BX.P) memcpy(BX.P, FT.BX.P, S * S * sizeof(float));
		return *this;
	}
//*
	FCOMPLEX&	operator()(int h, int k) {
		if( h<0 ) {h = -h; k = -k;}
		if((h==0)&&(k<0))  k = -k;
		if( h==S/2 ) { h=0; if (k>0) k=-k; }
		h += ((k&(S-1)) << (Ln2S-1));
		return *( ((FCOMPLEX*)BX.P) + h );
	}
/*/
	FCOMPLEX&	operator()(int h, int k) {
		if( h < 0 ) h = S/2+h;
		if( k < 0 ) k = S+k;
//		if((h==0)&&(k<0))  k = -k;
//		if( h==S/2 ) { h=0; if (k>0) k=-k; }
		h += ((k&(S-1)) << (Ln2S-1));
		return *( ((FCOMPLEX*)BX.P) + h );
	}
//*/
	FCOMPLEX&	operator[](const ICOMPLEX& c) { return operator()(c.i, c.j); }
//=======================================================================
	FT2R&	operator +=(const FT2R& FT){ ADD (FT); return *this;}
	FT2R&	operator -=(const FT2R& FT){ SUB (FT); return *this;}
	FT2R&	operator *=(const FT2R& FT){ MUL (FT); return *this;}
	FT2R&	operator ^=(const FT2R& FT){ MULC(FT); return *this;}
	FT2R&	operator /=(const FT2R& FT){ DIV (FT); return *this;}
//=======================================================================
//=======================================================================
//=======================================================================
	BOOL	Alloc(int s, void * p);
	void	Free();
	BOOL	ReAlloc(int s, void * p){ Free(); return Alloc(s, p); }
	void	Init_CS_Tbl();
//=======================================================================
	BOOL	Load(BM8& BM);
	BOOL	Load(BM8& BM, int xc, int yc);
	BOOL	Load(BM8S& BM);
	BOOL	Load(BM8S& BM, int xc, int yc);
	BOOL	Load(BM16& BM);
	BOOL	Load(BM16& BM, int xc, int yc);
	BOOL	Load(BM16S& BM);
	BOOL	Load(BM16S& BM, int xc, int yc);
	BOOL	Load(BM32& BM);
	BOOL	Load(BM32& BM, int xc, int yc);
	BOOL	Load(BM32S& BM);
	BOOL	Load(BM32S& BM, int xc, int yc);
	BOOL	Load(BMF& BM);
	BOOL	Load(BMF& BM, int xc, int yc);
	BOOL	Load(BMD& BM);
	BOOL	Load(BMD& BM, int xc, int yc);
	//////////////////////////////////////////////////////////////////////////
	BOOL	FFT(void);
	BOOL	IFT(void);
	BOOL	DisplayFFT(BM8& BM, float amax=0);
	BOOL	DisplayFFT(BMF& BM, float amax=0);
	//////////////////////////////////////////////////////////////////////////
	BOOL	DisplayIFT(BM8& BM, float gain=0);
	BOOL	DisplayIFT(BM8S& BM, float gain=0);
	BOOL	DisplayIFT(BM16& BM, float gain=0);
	BOOL	DisplayIFT(BM16S& BM, float gain=0);
	BOOL	DisplayIFT(BM32& BM, float gain=0);
	BOOL	DisplayIFT(BM32S& BM, float gain=0);
	BOOL	DisplayIFT(BMF& BM, float gain=0);
	BOOL	DisplayIFT(BMD& BM, float gain=0);
	//////////////////////////////////////////////////////////////////////////
	void	PutVal(int h, int k, FCOMPLEX val);
	void	PutAP(int h, int k, float a, float p);
	void	PutVal(ICOMPLEX hk, FCOMPLEX val);
	FCOMPLEX		GetVal(int h, int k);
	FCOMPLEX		GetVal(ICOMPLEX hk);
	float	GetAmp(int h, int k) {return abs(GetVal(h,k));}
	float	GetAmp(ICOMPLEX hk)        {return abs(GetVal(hk));}
	float	GetPha(int h, int k) {return atan2(GetVal(h,k));}
	float	GetPha(ICOMPLEX hk)        {return atan2(GetVal(hk));}
	BOOL	ValidHK(int h, int k) { return ((h>=-S/2)&&(h<=S/2-1)&&(k>=-S/2)&&(k<=S/2));}
//=======================================================================
	BOOL	ADD (const FT2R& F2);
	BOOL	SUB (const FT2R& F2);
	BOOL	SUBR(const FT2R& F2);
	BOOL	MUL (const FT2R& F2);
	BOOL	MULC(const FT2R& F2);
	BOOL	DIV (const FT2R& F2);
//=======================================================================
	BOOL	MulBlob(BMF& BM, float rflt, int xc=0, int yc=0);
	BOOL	MulBlob(BMD& BM, float rflt, int xc=0, int yc=0);
//=======================================================================
	void	reord_x  (FCOMPLEX * p, int n);
	void	bitrev_x2(FCOMPLEX * p1, FCOMPLEX * p2, int n);
	void	brev2    (FCOMPLEX * p, int n, int w);
	void	decod	 (FCOMPLEX *Fp, DCOMPLEX *fwt, int s);
	void	encod	 (FCOMPLEX *Fp, DCOMPLEX *fwt, int s);
	void	fft2T(FCOMPLEX * Fp, DCOMPLEX * cst, int S); // Time Domain	2D-FFT
	void	fft2F(FCOMPLEX * Fp, DCOMPLEX * cst, int S); // Freq Domain 2D-FFT
	float	GetF_MaxVal(void * fft_adr, int xs);
	void	GetFAmp1Str8(LPVOID fft_adr, LPBYTE img_adr,int dim,int y,float dc,float kr0,float kr1,float kr2);
	void	GetFAmp1StrF(LPVOID fft_adr, float* img_adr,int dim,int y,float dc,float kr0,float kr1,float kr2);
// 	void	GetFloatMinMax(LPVOID src, int cnt, float& _min_, float& _max_);
	void	GetF_Img8(LPVOID src, LPVOID dst, int dim, double _min_, double gain);
	void	GetF_Img16(LPVOID src, LPVOID dst, int dim, double _min_, double gain);
	void	GetF_Img32(LPVOID src, LPVOID dst, int dim, double _min_, double gain);
	void	GetF_ImgF(LPVOID src, LPVOID dst, int dim, double _min_, double gain);
	void	GetF_ImgD(LPVOID src, LPVOID dst, int dim, double _min_, double gain);
	//=======================================================================
	__forceinline int set_FPUCW(int cw)
	{
		int		ocw;
		_asm
		{
			fstcw	[ocw]		;
			fldcw	[cw]		;
		}
		return	ocw;
	}
	//=======================================================================
};

//####################################################################################
//####################################################################################
// Class FT2R::Implementation=======================================
#ifdef __MEAT__
//####################################################################################
//####################################################################################
//-------------------------------------------------------------------------

//####################################################################################
BOOL	FT2R::ADD(const FT2R& F2){
		int	x,y;
		FCOMPLEX *f1=(FCOMPLEX*)BX.P, *f2=(FCOMPLEX*)F2.BX.P;
		if((f1==NULL)||(f2==NULL)||(S!=F2.S)) return FALSE;
		for(y=0; y<S; y++) for(x=0; x<S/2; x++){ *f1 += *f2; f1++; f2++;}
		return TRUE;
}//-------------------------------------------------------------------------
BOOL	FT2R::SUB(const FT2R& F2){
		int	x,y;
		FCOMPLEX *f1=(FCOMPLEX*)BX.P, *f2=(FCOMPLEX*)F2.BX.P;
		if((f1==NULL)||(f2==NULL)||(S!=F2.S)) return FALSE;
		for(y=0; y<S; y++) for(x=0; x<S/2; x++){ *f1 -= *f2; f1++; f2++;}
		return TRUE;
}//-------------------------------------------------------------------------
BOOL	FT2R::SUBR(const FT2R& F2){
		int	x,y;
		FCOMPLEX *f1=(FCOMPLEX*)BX.P, *f2=(FCOMPLEX*)F2.BX.P;
		if((f1==NULL)||(f2==NULL)||(S!=F2.S)) return FALSE;
		for(y=0; y<S; y++) for(x=0; x<S/2; x++){ *f1 = *f2 - *f1; f1++; f2++;}
		return TRUE;
}//-------------------------------------------------------------------------
BOOL	FT2R::MUL(const FT2R& F2){
		int	x,y,i,s;
		FCOMPLEX *f1=(FCOMPLEX*)BX.P, *f2=(FCOMPLEX*)F2.BX.P;
		if((f1==NULL)||(f2==NULL)||(S!=F2.S)) return FALSE;

		for(i=0; i<2; i++){
		(*f1).i *= (*f2).i;
		(*f1).j *= (*f2).j;
		f1++;	f2++;
		for(s=1,y=0; y<S/2; y++, s=0)
			for(x=s; x<S/2; x++){ *f1 *= *f2; f1++; f2++;}
		}
		return TRUE;
}//-------------------------------------------------------------------------
BOOL	FT2R::MULC(const FT2R& F2){
		int	x,y,i,s;
		FCOMPLEX *f1=(FCOMPLEX*)BX.P, *f2=(FCOMPLEX*)F2.BX.P;
		if((f1==NULL)||(f2==NULL)||(S!=F2.S)) return FALSE;
		for(i=0; i<2; i++){
		(*f1).i *= (*f2).i;
		(*f1).j *= (*f2).j;
		f1++;	f2++;
		for(s=1,y=0; y<S/2; y++, s=0)
			for(x=s; x<S/2; x++){ *f1 *= ~*f2; f1++; f2++;}
		}
		return TRUE;
}//-------------------------------------------------------------------------
BOOL	FT2R::DIV(const FT2R& F2){
		int	x,y;
		FCOMPLEX *f1=(FCOMPLEX*)BX.P, *f2=(FCOMPLEX*)F2.BX.P;
		if((f1==NULL)||(f2==NULL)||(S!=F2.S)) return FALSE;
		for(y=0; y<S; y++) for(x=0; x<S/2; x++){ *f1 /= *f2; f1++; f2++;}
		return TRUE;
}
//-------------------------------------------------------------------------
BOOL	FT2R::MulBlob(BMF& FLT, float rflt, int xc, int yc)
{
	int		xs=S, xs2=S/2, rf = round((float)S * rflt);
	float	d, df, kr = (float)FLT.W/(float)rf;
	ICOMPLEX x, p, x0(xc,yc);

	if ((BX.P==NULL)||(FLT.P==NULL)) return FALSE;

	if( x0.i<0) x0 = -x0;
	if((x0.i==0)&&(x0.j<0)) x0.j = -x0.j;

	for(x.j=-rf; x.j<=rf; x.j++) for(x.i=-rf; x.i<=rf; x.i++)	{
		if ((x.i<=-xs2) || (x.j<-xs2) || (x.i>=xs2) || (x.j>=xs2)) continue;
		if((d=abs(x)) >= rf) continue;
		p = x+x0;
		if( (df=abs(x0+p)) <= rf){
			if((p.i<0) || ((p.i==0) && (p.j<0)) ) continue;
			if(df<d) d=df;
		}
		d = __min(d*kr, FLT.W-1);
		(*this)[p] *= FLT[d];
	}
	return TRUE;
}
//-------------------------------------------------------------------------
BOOL	FT2R::MulBlob(BMD& FLT, float rflt, int xc, int yc)
{
	int		xs=S, xs2=S/2, rf = round((float)S * rflt);
	float	d, df, kr = (float)FLT.W/(float)rf;
	ICOMPLEX x, p, x0(xc,yc);

	if ((BX.P==NULL)||(FLT.P==NULL)) return FALSE;

	if( x0.i<0) x0 = -x0;
	if((x0.i==0)&&(x0.j<0)) x0.j = -x0.j;

	for(x.j=-rf; x.j<=rf; x.j++) for(x.i=-rf; x.i<=rf; x.i++)	{
		if ((x.i<=-xs2) || (x.j<-xs2) || (x.i>=xs2) || (x.j>=xs2)) continue;
		if((d=abs(x)) >= rf) continue;
		p = x+x0;
		if( (df=abs(x0+p)) <= rf){
			if((p.i<0) || ((p.i==0) && (p.j<0)) ) continue;
			if(df<d) d=df;
		}
		d = __min(d*kr, FLT.W-1);
		(*this)[p] *= FLT[d];
	}
	return TRUE;
}
//####################################################################################
BOOL	FT2R::Load(BM8& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC)), aval=128;
	float* fp;
	LPBYTE ip;
	if((BX.P==NULL) || (BM.P==NULL)) return FALSE;
	for(fp=BX.P, ip=BM.P+(int)BM.XC+(int)BM.YC*BM.W, y=0; y<ny; y++, fp+=S,	ip+=BM.W){
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for( ; x<S; x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
BOOL	FT2R::Load(BM8S& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC)), aval=0;
	float* fp;
	LPSTR ip;
	if((BX.P==NULL) || (BM.P==NULL)) return FALSE;
	for(fp=BX.P, ip=BM.P+(int)BM.XC+(int)BM.YC*BM.W, y=0; y<ny; y++, fp+=S,	ip+=BM.W){
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for( ; x<S; x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
//-------------------------------------------------------------------------
BOOL	FT2R::Load(BM8& BM, int xc, int yc)
{
	float	*fp,am;
	BYTE	*ip;
	int		hs=S/2, y,ix,ix0,ix1,fontWidth,fx0,fx1,iy0,iy1,fy0,fy1, a, n;

	if((BX.P==NULL) || (BM.P==NULL)) return FALSE;
	ix0 = __max(0, xc-hs);	ix1 = __min(BM.W, xc+hs);
	fx0 = ix0 - xc + hs;	fx1 = ix1 - xc + hs;
	iy0 = __max(0, yc-hs);	iy1 = __min(BM.H, yc+hs);
	fy0 = iy0 - yc + hs;	fy1 = iy1 - yc + hs;

	a = n = am = 0;
	for(ip=BM.P+iy0*BM.W, fp =BX.P+fy0*BX.W, y=fy0; y<fy1; y++, fp+=BX.W, ip+=BM.W)
		for(ix=ix0, fontWidth=fx0; fontWidth<fx1; fontWidth++, ix++){
			fp[fontWidth] = ip[ix]; a += ip[ix]; n++;
		}
	if(n){
		am = (float)a/(float)n;

		for(y=0; y<fy0; y++) for(fp=BX.P+y*BX.W, fontWidth=0; fontWidth<S; fontWidth++) fp[fontWidth] = am;
		for(y=fy0; y<fy1; y++){
			fp=BX.P+y*BX.W;
			for(fontWidth=0; fontWidth<fx0; fontWidth++) fp[fontWidth] = am;
			for(fontWidth=fx1; fontWidth<S; fontWidth++) fp[fontWidth] = am;
		}
		for(y=fy1; y<S; y++) for(fp=BX.P+y*BX.W, fontWidth=0; fontWidth<S; fontWidth++) fp[fontWidth] = am;
	}
	else memset(BX.P, 0, S*S*4);
	return TRUE;
}
//-------------------------------------------------------------------------
//####################################################################################
BOOL	FT2R::Load(BM16& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC)), aval=32768;
	LPFLOAT fp;
	LPWORD ip;
	if((BX.P==NULL) || (BM.P==NULL)) return FALSE;
	for(fp=BX.P, ip=BM.P+(int)BM.XC+(int)BM.YC*BM.W, y=0; y<ny; y++, fp+=S,	ip+=BM.W){
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for(   ; x<S;  x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
BOOL	FT2R::Load(BM16S& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC)), aval=0;
	LPFLOAT fp;
	SHORT* ip;
	if((BX.P==NULL) || (BM.P==NULL)) return FALSE;
	for(fp=BX.P, ip=BM.P+(int)BM.XC+(int)BM.YC*BM.W, y=0; y<ny; y++, fp+=S,	ip+=BM.W){
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for(   ; x<S;  x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
//-------------------------------------------------------------------------
BOOL	FT2R::Load(BM16& BM, int xc, int yc)
{
	float	*fp,am;
	WORD	*ip;
	int		hs=S/2, y,ix,ix0,ix1,fontWidth,fx0,fx1,iy0,iy1,fy0,fy1, a, n;

	if((BX.P==NULL) || (BM.P==NULL)) return FALSE;
	ix0 = __max(0, xc-hs);	ix1 = __min(BM.W, xc+hs);
	fx0 = ix0 - xc + hs;	fx1 = ix1 - xc + hs;
	iy0 = __max(0, yc-hs);	iy1 = __min(BM.H, yc+hs);
	fy0 = iy0 - yc + hs;	fy1 = iy1 - yc + hs;

	a = n = am = 0;
	for(ip=BM.P+iy0*BM.W, fp =BX.P+fy0*BX.W, y=fy0; y<fy1; y++, fp+=BX.W, ip+=BM.W)
		for(ix=ix0, fontWidth=fx0; fontWidth<fx1; fontWidth++, ix++){
			fp[fontWidth] = ip[ix]; a += ip[ix]; n++;
		}
	if(n){
		am = (float)a/(float)n;

		for(y=0; y<fy0; y++) for(fp=BX.P+y*BX.W, fontWidth=0; fontWidth<S; fontWidth++) fp[fontWidth] = am;
		for(y=fy0; y<fy1; y++){
			fp=BX.P+y*BX.W;
			for(fontWidth=0; fontWidth<fx0; fontWidth++) fp[fontWidth] = am;
			for(fontWidth=fx1; fontWidth<S; fontWidth++) fp[fontWidth] = am;
		}
		for(y=fy1; y<S; y++) for(fp=BX.P+y*BX.W, fontWidth=0; fontWidth<S; fontWidth++) fp[fontWidth] = am;
	}
	else memset(BX.P, 0, S*S*4);
	return TRUE;
}
//-------------------------------------------------------------------------
//####################################################################################
BOOL	FT2R::Load(BM32& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC));
	DWORD aval = 0x80000000;
	LPFLOAT fp;
	LPDWORD ip;
	if( (BX.P==NULL) || (BM.P==NULL) ) return FALSE;
	for(fp = BX.P, ip = BM.P + (int)BM.XC + ((int)BM.YC)*BM.W, y=0; y<ny; y++, fp+=S, ip+=BM.W)
	{
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for(   ; x<S;  x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
BOOL	FT2R::Load(BM32S& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC));
	LONG aval = 0;
	LPFLOAT fp;
	LPLONG ip;
	if( (BX.P==NULL) || (BM.P==NULL) ) return FALSE;
	for(fp = BX.P, ip = BM.P + (int)BM.XC + ((int)BM.YC)*BM.W, y=0; y<ny; y++, fp+=S, ip+=BM.W)
	{
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for(   ; x<S;  x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
//-------------------------------------------------------------------------
BOOL	FT2R::Load(BM32& BM, int xc, int yc)
{
	float	*fp,am;
	DWORD	*ip;
	int		hs=S/2, y,ix,ix0,ix1,fontWidth,fx0,fx1,iy0,iy1,fy0,fy1, a, n;

	if((BX.P==NULL) || (BM.P==NULL)) return FALSE;
	ix0 = __max(0, xc-hs);	ix1 = __min(BM.W, xc+hs);
	fx0 = ix0 - xc + hs;	fx1 = ix1 - xc + hs;
	iy0 = __max(0, yc-hs);	iy1 = __min(BM.H, yc+hs);
	fy0 = iy0 - yc + hs;	fy1 = iy1 - yc + hs;

	a = n = am = 0;
	for(ip=BM.P+iy0*BM.W, fp =BX.P+fy0*BX.W, y=fy0; y<fy1; y++, fp+=BX.W, ip+=BM.W)
		for(ix=ix0, fontWidth=fx0; fontWidth<fx1; fontWidth++, ix++){
			fp[fontWidth] = ip[ix]; a += ip[ix]; n++;
		}
		if(n){
			am = (float)a/(float)n;

			for(y=0; y<fy0; y++) for(fp=BX.P+y*BX.W, fontWidth=0; fontWidth<S; fontWidth++) fp[fontWidth] = am;
			for(y=fy0; y<fy1; y++){
				fp=BX.P+y*BX.W;
				for(fontWidth=0; fontWidth<fx0; fontWidth++) fp[fontWidth] = am;
				for(fontWidth=fx1; fontWidth<S; fontWidth++) fp[fontWidth] = am;
			}
			for(y=fy1; y<S; y++) for(fp=BX.P+y*BX.W, fontWidth=0; fontWidth<S; fontWidth++) fp[fontWidth] = am;
		}
		else memset(BX.P, 0, S*S*4);
		return TRUE;
}
//-------------------------------------------------------------------------
//####################################################################################
BOOL	FT2R::Load(BMF& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC));
	// TODO: check aval
	const float aval = 0;
	LPFLOAT fp;
	LPFLOAT ip;
	if( (BX.P==NULL) || (BM.P==NULL) ) return FALSE;
	for(fp = BX.P, ip = BM.P + (int)BM.XC + ((int)BM.YC)*BM.W, y=0; y<ny; y++, fp+=S, ip+=BM.W)
	{
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for(   ; x<S;  x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
///-------------------------------------------------------------------------
//####################################################################################
BOOL	FT2R::Load(BMD& BM)
{
	int x,y, nx=__min(S, __max(0,BM.W-BM.XC)), ny=__min(S, __max(0,BM.H-BM.YC));
	// TODO: check aval
	const double aval = 0;
	LPFLOAT fp;
	LPDOUBLE ip;
	if( (BX.P==NULL) || (BM.P==NULL) ) return FALSE;
	for(fp = BX.P, ip = BM.P + (int)BM.XC + ((int)BM.YC)*BM.W, y=0; y<ny; y++, fp+=S, ip+=BM.W)
	{
		for(x=0; x<nx; x++) fp[x] = ip[x];
		for(   ; x<S;  x++) fp[x] = aval;
	}
	for( ; y<S; y++, fp+=S) for(x=0 ; x<S; x++) fp[x] = aval;
	return TRUE;
}
//-------------------------------------------------------------------------
//####################################################################################
BOOL	FT2R::Alloc(int s, void * p)
{
	int	i;
	memset(this, 0, sizeof(FT2R));
	if((cst = (DCOMPLEX*) GlobalAlloc(GPTR, s*sizeof(DCOMPLEX)))==NULL) return FALSE;
	if(p)	BX.P = (float*)p;
	else	BX.P = (float*)GlobalAlloc(GPTR, s * s * sizeof(float));
	if(BX.P==NULL) return FALSE;
	S = BX.W = BX.H = s;
	BX.XC = BX.YC = s/2;
	for(i=1; i<s; i*=2) Ln2S++;
	MaxVal	= 1e6;
	iDC		= 0;	iScale	= 0;	iLinear	= 0;	iParab	= 0;
	iScaling = TRUE;
	fDoMaxVal = TRUE;
	Init_CS_Tbl();
	return TRUE;
}
//-------------------------------------------------------------------------
void	FT2R::Free()
{
	if(BX.P)GlobalFree(BX.P);
	if(cst)	GlobalFree(cst);
	memset(this, 0, sizeof(FT2R));
}
//-------------------------------------------------------------------------
void	FT2R::Init_CS_Tbl()
{
	DCOMPLEX * C = cst;
	int		ocw, s = S;
	if(C==NULL) return;
	ocw = set_FPUCW(0x37F);
	_asm {
		mov		edi,	[C]			;
		mov		ecx,	[s]			;
		fldpi						;
		fidiv	[s]					;
		fadd	st,		st			;
		fsincos						;	cw		sw
		fxch	st(1)				;	sw		cw
//		fimul	[sg]				;
		fld1						;	st0		st1		st2		st3		st4		st5		st6		st7
		fldz						;
ict_4:	;---------------------------;	s		c		sw		cw
		fld		st(1)				;	c		s		c		sw		cw
		fst		qword ptr[edi]		;
		fmul	st,		st(4)		;	c*cw	s		c		sw		cw
		fld		st(1)				;	s		c*cw	s		c		sw		cw
		fst		qword ptr[edi+8]	;
		fmul	st,		st(4)		;	s*sw	c*cw	s		c		sw		cw
		fxch	st(3)				;	c		c*cw	s		s*sw	sw		cw
		fmul	st,		st(4)		;	c*sw	c*cw	s		s*sw	sw		cw
		fxch	st(2)				;	s		c*cw	c*sw	s*sw	sw		cw
		fmul	st,		st(5)		;	s*cw	c*cw	c*sw	s*sw	sw		cw
		fxch	st(1)				;	c*cw	s*cw	c*sw	s*sw	sw		cw
		fsubrp	st(3),	st			;	s*cw	c*sw	C		sw		cw
		faddp	st(1),	st			;	S		C		sw		cw
		add		edi,	2*8			;
		dec		ecx					;
		jnz		ict_4				;
		fstp st						;
		fstp st						;
		fstp st						;
		fstp st						;
	}
	ocw = set_FPUCW(ocw);
}
//-------------------------------------------------------------------------
//####################################################################################
//-------------------------------------------------------------------------
void FT2R::reord_x(FCOMPLEX * p, int n)
{
	int i,m,j;
	FCOMPLEX t;
	for(j=i=0; i<n-1; i++) {
		if(j>i) { t=p[i]; p[i]=p[j]; p[j]=t;}
		m = n/2; while(m >=1 && j>=m) {j-=m; m /= 2;}
		j += m;
	}
}//-------------------------------------------------------------------
void FT2R::bitrev_x2(FCOMPLEX * p1, FCOMPLEX * p2, int n){
_asm{
		mov		edi,	[p1]
		mov		esi,	[p2]
		xor		ebx,	ebx
		mov		ecx,	[n]
		lea		ecx,	[edi+ecx*8]
br2_0:
		mov		eax,	[edi]
		mov		edx,	[esi+ebx*8]
		mov		[esi+ebx*8],eax
		mov		[edi],	edx
		mov		eax,	[edi+4]
		mov		edx,	[esi+ebx*8+4]
		mov		[esi+ebx*8+4],eax
		mov		[edi+4],	edx

		mov		eax,	[n]
		add		edi,	8
br2_2:	shr		eax,	1
		jz		br2_4
		sub		ebx,	eax
		jns		br2_2
		lea		ebx,	[ebx+eax*2]
br2_4:
		cmp		edi,	ecx
		jnz		br2_0
	}
}//-------------------------------------------------------------------
void FT2R::brev2(FCOMPLEX * p, int n, int w){
int i,m,j;
int n2=n/2, s = w*sizeof(FCOMPLEX);
FCOMPLEX *p1, *p2;
	for(j=i=0; i<n; i++) {
		p1 = p+i*w;	p2 = p+j*w;
		if(j >i) bitrev_x2(p1, p2, n2);
		if(i==j) reord_x(p1, n2);
		m = n/2; while(m >=1 && j>=m) {j-=m; m /= 2;}
		j += m;
	}
}
//####################################################################################

void	FT2R::decod(FCOMPLEX *Fp, DCOMPLEX *fwt, int s){
int	y, n=s/2, nn=n*n;
FCOMPLEX	b,c;
static	int	s1,s2,s3;
static	FCOMPLEX *p1,*p2;
static	DCOMPLEX *cst, *cx0;//, *cx;
static	float	f05;
	f05 = 0.5f;
	s1 = (n/2+1)*2*4; s2 = n*n*2*4; s3 = (n-1)*2*8;
	p1 = Fp + n*n - n/2 - 1;
	p2 = Fp +   n + n/2 + 1;
	cst = fwt; cx0 = &fwt[n/2-1];

#if(0)
static	DCOMPLEX *cx;
FCOMPLEX	t,u,v,a,d,A,B,C,D,w,z,e,f,g,h,zw;

	for(y=n-1; y>0; y--){
yy0:	z = cst[y];

		b = *(p1+nn+1) * z;	a = *(p1+1);
		c = a + b;
		d = a - b;
		*(p1+1) = c;
		*(p1+nn+1) = d;

		cx = cx0;
		for(cx=cx0; cx>cst; cx--){
			A = *p1; B = *(p1+nn);		C = *p2; D = *(p2+nn);

			b.i = 0.5*(B.i + D.i);		b.j = 0.5*(B.j - D.j);
			c.i = 0.5*(A.i - C.i);		c.j = 0.5*(A.j + C.j);
			d.i = 0.5*(B.i - D.i);		d.j = 0.5*(B.j + D.j);

			w = *cx; zw = cx[y];

			b *= z;	c *= w;	d *= zw;

			a.i = 0.5*(A.i + C.i);		a.j = 0.5*(A.j - C.j);

			A.i = (a.i+b.i) + (c.j+d.j);
			B.i = (a.i+b.i) - (c.j+d.j);
			C.i = (a.i-b.i) + (c.j-d.j);
			D.i = (a.i-b.i) - (c.j-d.j);

			A.j = (a.j+b.j) - (c.i+d.i);
			B.j =-(a.j+b.j) - (c.i+d.i);
			C.j = (a.j-b.j) + (d.i-c.i);
			D.j =-(a.j-b.j) + (d.i-c.i);

			*p1 = A; *(p1+nn) = C;
			if(y){ *(p2+nn) =B;	*p2 = D;} else {*p2 = B; *(p2+nn) = D;}
			p1--;
			p2++;
		}
		b = *(p1+nn) * z;
		a = *p1;
		c = a + b;
		d = a - b;
		a.i = c.i+c.j; a.j = c.i-c.j;
		b.i = d.i+d.j; b.j = d.i-d.j;
		*p1 = a;
		*(p1+nn) = b;

		p1 -= n/2 + 1;
		p2 += n/2 + 1;
	}
	p2 -= nn;
	if(y>=0) goto yy0;


#else

_asm{	//===============================
		push	ebp						;
		mov		ebp,	[s2]			; n*n*sizeof(FCOMPLEX)
		mov		esi,	[p1]			; [esi] -> A,	[esi+ebp] -> B
		mov		edi,	[p2]			; [edi] -> C,	[edi+ebp] -> D
		mov		edx,	[s3]			; y = (n-1)*sizeof(DCOMPLEX)
		mov		ebx,	[cst]			; [ebx+edx] -> Z

xr_y:	//=============================================================================================
//		d = *(p1+nn+1) * z;	c = *(p1+1);	*(p1+nn+1) = c - d;	*(p1+1) = c + d;
		fld		dword ptr[esi+ebp+8]	;	st0		st1		st2		st3		st4		st5		st6		st7
		fmul	qword ptr[ebx+edx]		;	di*zi
		fld		dword ptr[esi+ebp+12]	;
		fmul	qword ptr[ebx+edx+8]	;	dj*zj	di*zi
		fld		dword ptr[esi+ebp+12]	;
		fmul	qword ptr[ebx+edx]		;	dj*zi	dj*zj	di*zi
		fld		dword ptr[esi+ebp+8]	;
		fmul	qword ptr[ebx+edx+8]	;	di*zj	dj*zi	dj*zj	di*zi
		fxch	st(2)					;	dj*zj	dj*zi	di*zj	di*zi
		fsubp	st(3),	st				;	dj*zi	di*zj	ti
		fld		dword ptr[esi+8]		;	ci		dj*zi	di*zj	ti
		fld		st						;	ci		ci		dj*zi	di*zj	ti
		fadd	st,		st(4)			;	Ci		ci		dj*zi	di*zj	ti
		fxch	st(1)					;	ci		Ci		dj*zi	di*zj	ti
		fsubrp	st(4),	st				;	Ci		dj*zi	di*zj	Di
		fstp	[esi+8]					;	dj*zi	di*zj	Di
		faddp	st(1),	st				;	tj		Di
		fld		dword ptr[esi+12]		;	cj		tj		Di
		fld		st						;	cj		cj		tj		Di
		fadd	st,		st(2)			;	Cj		cj		tj		Di
		fxch	st(1)					;	cj		Cj		tj		Di
		fsubrp	st(2),	st				;	Cj		Dj		Di
		fstp	dword ptr[esi+12]		;
		fstp	dword ptr[esi+ebp+12]	;
		fstp	dword ptr[esi+ebp+8]	;

		mov		ecx,	[cx0]			;	[ecx] -> W,		[ecx+edx] -> ZW
xr_x:	;===============================;	st0		st1		st2		st3		st4		st5		st6		st7

//		A = *p1; B = *(p1+nn);		C = *p2; D = *(p2+nn);

		fld		dword ptr[esi+ebp]		;//	b.i = 0.5*(B.i + D.i);	b.j = 0.5*(B.j - D.j);
		fadd	dword ptr[edi+ebp]		;
		fld		dword ptr[esi+ebp+4]	;
		fsub	dword ptr[edi+ebp+4]	;
		fxch	st(1)					;
		fmul	[f05]					;
		fxch	st(1)					;
		fmul	[f05]					;	bj		bi

		fld		dword ptr[esi]			;//	c.i = 0.5*(A.i - C.i);	c.j = 0.5*(A.j + C.j);
		fsub	dword ptr[edi]			;
		fld		dword ptr[esi+4]		;
		fadd	dword ptr[edi+4]		;
		fxch	st(1)					;
		fmul	[f05]					;
		fxch	st(1)					;
		fmul	[f05]					;	cj		ci		bj		bi

		fld		dword ptr[esi+ebp]		;
		fsub	dword ptr[edi+ebp]		;//	d.i = 0.5*(B.i - D.i);	d.j = 0.5*(B.j + D.j);
		fld		dword ptr[esi+ebp+4]	;
		fadd	dword ptr[edi+ebp+4]	;
		fxch	st(1)					;
		fmul	[f05]					;
		fxch	st(1)					;
		fmul	[f05]					;	dj		di		cj		ci		bj		bi
//	b *= z;		c *= w;		d *= zw;	;	st0		st1		st2		st3		st4		st5		st6		st7
		fld		st(5)					;	bi		dj		di		cj		ci		bj		bi
		fmul	qword ptr[ebx+edx]		;	bi*zi	dj		di		cj		ci		bj		bi
		fld		st(5)					;	bj		bi*zi	dj		di		cj		ci		bj		bi
		fmul	qword ptr[ebx+edx+8]	;	bj*zj	bi*zi	dj		di		cj		ci		bj		bi
		fxch	st(6)					;	bj		bi*zi	dj		di		cj		ci		bj*zj	bi
		fmul	qword ptr[ebx+edx]		;	bj*zi	bi*zi	dj		di		cj		ci		bj*zj	bi
		fxch	st(7)					;	bi		bi*zi	dj		di		cj		ci		bj*zj	bj*zi
		fmul	qword ptr[ebx+edx+8]	;	bi*zj	bi*zi	dj		di		cj		ci		bj*zj	bj*zi
		fxch	st(1)					;	bi*zi	bi*zj	dj		di		cj		ci		bj*zj	bj*zi
		fsubrp	st(6),	st				;	bi*zj	dj		di		cj		ci		Bi		bj*zi
		fld		st(4)					;	ci		bi*zj	dj		di		cj		ci		Bi		bj*zi
		fmul	qword ptr[ecx]			;	ci*wi	bi*zj	dj		di		cj		ci		Bi		bj*zi
		fxch	st(1)					;	bi*zj	ci*wi	dj		di		cj		ci		Bi		bj*zi
		faddp	st(7),	st				;	ci*wi	dj		di		cj		ci		Bi		Bj
		fld		st(3)					;	cj		ci*wi	dj		di		cj		ci		Bi		Bj
		fmul	qword ptr[ecx+8]		;	cj*wj	ci*wi	dj		di		cj		ci		Bi		Bj
		fxch	st(5)					;	ci		ci*wi	dj		di		cj		cj*wj	Bi		Bj
		fmul	qword ptr[ecx+8]		;	ci*wj	ci*wi	dj		di		cj		cj*wj	Bi		Bj
		fxch	st(4)					;	cj		ci*wi	dj		di		ci*wj	cj*wj	Bi		Bj
		fmul	qword ptr[ecx]			;	cj*wi	ci*wi	dj		di		ci*wj	cj*wj	Bi		Bj
		fxch	st(1)					;	ci*wi	cj*wi	dj		di		ci*wj	cj*wj	Bi		Bj
		fsubrp	st(5),	st				;	cj*wi	dj		di		ci*wj	Ci		Bi		Bj
		fld		st(2)					;	di		cj*wi	dj		di		ci*wj	Ci		Bi		Bj
		fmul	qword ptr[ecx+edx]		;	di*zwi	cj*wi	dj		di		ci*wj	Ci		Bi		Bj
		fxch	st(1)					;	cj*wi	di*zwi	dj		di		ci*wj	Ci		Bi		Bj
		faddp	st(4),	st				;	di*zwi	dj		di		Cj		Ci		Bi		Bj
		fld		st(1)					;	dj		di*zwi	dj		di		Cj		Ci		Bi		Bj
		fmul	qword ptr[ecx+edx+8]	;	dj*zwj	di*zwi	dj		di		Cj		Ci		Bi		Bj
		fxch	st(2)					;	dj		di*zwi	dj*zwj	di		Cj		Ci		Bi		Bj
		fmul	qword ptr[ecx+edx]		;	dj*zwi	di*zwi	dj*zwj	di		Cj		Ci		Bi		Bj
		fxch	st(3)					;	di		di*zwi	dj*zwj	dj*zwi	Cj		Ci		Bi		Bj
		fmul	qword ptr[ecx+edx+8]	;	di*zwj	di*zwi	dj*zwj	dj*zwi	Cj		Ci		Bi		Bj
		fxch	st(1)					;	di*zwi	di*zwj	dj*zwj	dj*zwi	Cj		Ci		Bi		Bj
		fsubrp	st(2),	st				;	di*zwj	Di		dj*zwi	Cj		Ci		Bi		Bj
		fld		dword ptr[esi]			;//	a.i = 0.5*(A.i + C.i);
		fadd	dword ptr[edi]			;	ai'		di*zwj	Di		dj*zwi	Cj		Ci		Bi		Bj
		fxch	st(1)					;	di*zwj	ai'		Di		dj*zwi	Cj		Ci		Bi		Bj
		faddp	st(3),	st				;	ai'		Di		Dj		Cj		Ci		Bi		Bj
		fmul	[f05]					;	ai		di		dj		cj		ci		bi		bj
;--------------------------------------//	st0		st1		st2		st3		st4		st5		st6		st7
//		Ai = (ai+bi) + (cj + dj);
//		Bi = (ai+bi) - (cj + dj);			ai		di		dj		cj		ci		bi		bj
		fld		st(3)					;	cj		ai		di		dj		cj		ci		bi		bj
		fadd	st,		st(3)			;	cj+dj	ai		di		dj		cj		ci		bi		bj
		fxch	st(1)					;	ai		cj+dj	di		dj		cj		ci		bi		bj
		fadd	st,		st(6)			;	ai+bi	cj+dj	di		dj		cj		ci		bi		bj
		fxch	st(3)					;	dj		cj+dj	di		ai+bi	cj		ci		bi		bj
		fsubp	st(4),	st				;	cj+dj	di		ai+bi	cj-dj	ci		bi		bj
		fld		st(2)					;	ai+bi	cj+dj	di		ai+bi	cj-dj	ci		bi		bj
		fsub	st,		st(1)			;	Bi		cj+dj	di		ai+bi	cj-dj	ci		bi		bj
		fxch	st(6)					;	bi		cj+dj	di		ai+bi	cj-dj	ci		Bi		bj
		fadd	st,		st				;	2bi		cj+dj	di		ai+bi	cj-dj	ci		Bi		bj
		fxch	st(3)					;	ai+bi	cj+dj	di		2bi		cj-dj	ci		Bi		bj
		fadd	st(1),	st				;	ai+bi	Ai		di		2bi		cj-dj	ci		Bi		bj
		fsubrp	st(3),	st				;	Ai		di		ai-bi	cj-dj	ci		Bi		bj
		fstp	dword ptr[esi]			;	di		ai-bi	cj-dj	ci		Bi		bj
//		C.i = (a.i-b.i) + (c.j-d.j);
//		D.i = (a.i-b.i) - (c.j-d.j);		st0		st1		st2		st3		st4		st5		st6		st7
		fld		dword ptr[esi+4]		;
		fsub	dword ptr[edi+4]		;	aj'		di		ai-bi	cj-dj	ci		Bi		bj
		fld		st(2)					;	ai-bi	aj'		di		ai-bi	cj-dj	ci		Bi		bj
		fsub	st,		st(4)			;	Di		aj'		di		ai-bi	cj-dj	ci		Bi		bj
		fxch	st(4)					;	cj-dj	aj'		di		ai-bi	Di		ci		Bi		bj
		faddp	st(3),	st				;	aj'		di		Ci		Di		ci		Bi		bj
		fmul	[f05]					;	aj		di		Ci		Di		ci		Bi		bj
		fxch	st(2)					;	Ci		di		aj		Di		ci		Bi		bj
		fstp	dword ptr[esi+ebp]		;	di		aj		Di		ci		Bi		bj
//		A.j = (a.j+b.j) - (c.i+d.i);
//		B.j =-(a.j+b.j) - (c.i+d.i);		st0		st1		st2		st3		st4		st5		st6		st7
		fld		st						;	di		di		aj		Di		ci		Bi		bj
		fadd	st,		st(4)			;	di+ci	di		aj		Di		ci		Bi		bj
		fxch	st(1)					;	di		di+ci	aj		Di		ci		Bi		bj
		fsubrp	st(4),	st				;	di+ci	aj		Di		di-ci	Bi		bj
		fld		st(1)					;	aj		di+ci	aj		Di		di-ci	Bi		bj
		fsub	st,		st(6)			;	aj-bj	di+ci	aj		Di		di-ci	Bi		bj
		fxch	st(6)					;	bj		di+ci	aj		Di		di-ci	Bi		aj-bj
		faddp	st(2),	st				;	di+ci	aj+bj	Di		di-ci	Bi		aj-bj
		fsubr	st,		st(1)			;	Aj		aj+bj	Di		di-ci	Bi		aj-bj
		fxch	st(1)					;	aj+bj	Aj		Di		di-ci	Bi		aj-bj
		fadd	st,		st				;	2(aj+bj)Aj		Di		di-ci	Bi		aj-bj
//		C.j = (a.j-b.j) + (d.i-c.i);
//		D.j =-(a.j-b.j) + (d.i-c.i);		st0		st1		st2		st3		st4		st5		st6		st7
		fld		st(5)					;	aj-bj	2(aj+bj)Aj		Di		di-ci	Bi		aj-bj
		fsubr	st,		st(4)			;	Dj		2(aj+bj)Aj		Di		di-ci	Bi		aj-bj
		fxch	st(6)					;	aj-bj	2(aj+bj)Aj		Di		di-ci	Bi		Dj
		faddp	st(4),	st				;	2(aj+bj)Aj		Di		Cj		Bi		Dj
		fsubr	st,		st(1)			;	Bj		Aj		Di		Cj		Bi		Dj
		fxch	st(3)					;	Cj		Aj		Di		Bj		Bi		Dj
//		*p1 = A; *(p1+nn) = C;
		fstp	dword ptr[esi+ebp+4]	;	Aj		Di		Bj		Bi		Dj
		fstp	dword ptr[esi+4]		;	Di		Bj		Bi		Dj
//		if(!y) {*p2 = B; *(p2+nn) = D;}		Di		Bj		Bi		Dj
		or		edx,	edx				;
		jnz		xr_x2					;
		fstp	dword ptr[edi+ebp]		;	Bj		Bi		Dj
		fstp	dword ptr[edi+4]		;	Bi		Dj
		fstp	dword ptr[edi]			;	Dj
		fstp	dword ptr[edi+ebp+4]	;
		jmp		xr_x4					;
//		else	{ *(p2+nn) =B;	*p2 = D;}		Di		Bj		Bi		Dj
xr_x2:	fstp	dword ptr[edi]			;	Bj		Bi		Dj
		fstp	dword ptr[edi+ebp+4]	;	Bi		Dj
		fstp	dword ptr[edi+ebp]		;	Dj
		fstp	dword ptr[edi+4]		;
;----------------------------------------
xr_x4:	sub		ecx,	2*8				; cx--
		sub		esi,	2*4				; p1--
		add		edi,	2*4				; p2++
		cmp		ecx,	ebx				;
		jg		xr_x					;
;==========================================
//		b = *(p1+nn) * z;	a = *p1;	*(p1+nn) = a - b;	*p1 = a + b;

		fld		dword ptr[esi+ebp]		;	st0		st1		st2		st3		st4		st5		st6		st7
		fmul	qword ptr[ebx+edx]		;	di*zi
		fld		dword ptr[esi+ebp+4]	;
		fmul	qword ptr[ebx+edx+8]	;	dj*zj	di*zi
		fld		dword ptr[esi+ebp+4]	;
		fmul	qword ptr[ebx+edx]		;	dj*zi	dj*zj	di*zi
		fld		dword ptr[esi+ebp]		;
		fmul	qword ptr[ebx+edx+8]	;	di*zj	dj*zi	dj*zj	di*zi
		fxch	st(2)					;	dj*zj	dj*zi	di*zj	di*zi
		fsubp	st(3),	st				;	dj*zi	di*zj	ti
		fld		dword ptr[esi]			;	ci		dj*zi	di*zj	ti
		fld		st						;	ci		ci		dj*zi	di*zj	ti
		fadd	st,		st(4)			;	Ci		ci		dj*zi	di*zj	ti
		fxch	st(1)					;	ci		Ci		dj*zi	di*zj	ti
		fsubrp	st(4),	st				;	Ci		dj*zi	di*zj	Di
		fxch	st(1)					;	dj*zi	Ci		di*zj	Di
		faddp	st(2),	st				;	Ci		tj		Di
		fld		dword ptr[esi+4]		;	cj		Ci		tj		Di
		fld		st						;	cj		cj		Ci		tj		Di
		fadd	st,		st(3)			;	Cj		cj		Ci		tj		Di
		fxch	st(1)					;	cj		Cj		Ci		tj		Di
		fsubrp	st(3),	st				;	Cj		Ci		Dj		Di
		fld		st						;	Cj		Cj		Ci		Dj		Di
		fadd	st,		st(2)			;	CI		Cj		Ci		Dj		Di
		fxch	st(1)					;	Cj		CI		Ci		Dj		Di
		fsubp	st(2),	st				;	CI		CJ		Dj		Di
		fld		st(2)					;	Dj		CI		CJ		Dj		Di
		fadd	st,		st(4)			;	DI		CI		CJ		Dj		Di
		fxch	st(3)					;	Dj		CI		CJ		DI		Di
		fsubp	st(4),	st				;	CI		CJ		DI		DJ

		fstp	dword ptr[esi]			;
		fstp	dword ptr[esi+4]		;
		fstp	dword ptr[esi+ebp]		;
		fstp	dword ptr[esi+ebp+4]	;

;----------------------------------------
		sub		esi,	[s1]			;
		add		edi,	[s1]			;
		sub		edx,	2*8				; y--
		jg		xr_y					;
		lea		edi,	[esi+2*2*4]
		jge		xr_y					;

		pop		ebp
}// ASM	===================================

#endif

	p1 = Fp + n*n - n;	p2 = p1+2*n;

	for(y=1; y<n; y++){
		b = *p2;	c = *p1;

		*p1 = 0.5*FCOMPLEX( b.i + c.i , b.j - c.j );
		*p2 = 0.5*FCOMPLEX( b.j + c.j , b.i - c.i );

		p1 -= n; p2 += n;
	}

}

//####################################################################################
void	FT2R::encod(FCOMPLEX *Fp, DCOMPLEX *fwt, int s){
int	y, n=s/2, nn=n*n;
FCOMPLEX	c,d;
static	int	s1,s2,s3;
static	FCOMPLEX *p1,*p2;
static	DCOMPLEX *cst, *cx0;//, *cx1;

	p1 = Fp + n*n - n;	p2 = p1+2*n;
	for(y=1; y<n; y++){
		c.i = p1->i;	d.j = p1->j;	c.j = p2->i;	d.i = p2->j;
		*p2 = c+d;		*p1 = c-d;
		p1 -= n; p2 += n;
	}

	s1 = (n/2+1)*2*4; s2 = n*n*2*4; s3 = (n-1)*2*8;
	p1 = Fp + n*n - n/2 - 1;
	p2 = Fp +   n + n/2 + 1;
	cst = fwt; cx0 = &fwt[n/2-1];

#if(0)
FCOMPLEX	t,u,v,a,b,A,B,C,D,w,z,e,f,g,h,zw;
	for(y=n-1; y>0; y--){
yy0:	z = cst[y];
		c = *(p1+1) * 2.;		d = *(p1+nn+1) * 2.;
		C = c+d;	D=(c-d);	D*=~z;
		*(p1+1) = C; 		*(p1+nn+1) =D;
//===============================================
		for(cx1=cx0; cx1>cst; cx1--){

			A = *p1; C = *(p1+nn);
			if(y){ B = *(p2+nn);	D = *p2;}
			else { B = *p2;			D = *(p2+nn);}

			e.i = A.i+B.i;		f.j = (A.i-B.i);
			e.j = A.j-B.j;		f.i =-(A.j+B.j);
			g.i = C.i+D.i;		h.j = (C.i-D.i);
			g.j = C.j-D.j;		h.i =-(C.j+D.j);

			a = e+g; b = e-g; c = f+h; d = f-h;

			w = *cx1; zw = cx1[y];

			b *= ~z;	c *= ~w;	d *= ~zw;

			A.i = a.i+c.i;		C.i = a.i-c.i;
			A.j = a.j+c.j;		C.j = c.j-a.j;
			B.i = b.i+d.i;		D.i = b.i-d.i;
			B.j = b.j+d.j;		D.j = d.j-b.j;
			*p1		 = A;	*(p1+nn) = B;	*p2		 = C;	*(p2+nn) = D;
			p1--;
			p2++;
		}
//=================================================
		a = *p1;	b = *(p1+nn);
		d.i = b.i+b.j;		d.j = b.i-b.j;
		c.i = a.i+a.j;		c.j = a.i-a.j;
		a = c+d;			b = c-d;
		*(p1+nn) = b * ~z;	*p1 = a;

		p1 -= n/2 + 1;
		p2 += n/2 + 1;
	}
	p2 -= nn;
	if(y>=0) goto yy0;
#else

_asm{	//===============================
		push	ebp						;
		mov		ebp,	[s2]			; n*n*sizeof(FCOMPLEX)
		mov		esi,	[p1]			; [esi] -> A,	[esi+ebp] -> B
		mov		edi,	[p2]			; [edi] -> C,	[edi+ebp] -> D
		mov		edx,	[s3]			; y = (n-1)*sizeof(DCOMPLEX)
		mov		ebx,	[cst]			; [ebx+edx] -> Z

xr_y:	//=============================================================================================
//		c = *(p1+1) * 2.;	d = *(p1+nn+1) * 2.;
		fld		dword ptr[esi+8]		;	st0		st1		st2		st3		st4		st5		st6		st7
		fadd	st,		st				;
		fld		dword ptr[esi+12]		;
		fadd	st,		st				;
		fld		dword ptr[esi+ebp+8]	;
		fadd	st,		st				;
		fld		dword ptr[esi+ebp+12]	;	dj		di		cj		ci
		fadd	st,		st				;	dj		di		cj		ci
		fld		st(3)					;	ci		dj		di		cj		ci
		fsub	st,		st(2)			;	ci-di	dj		di		cj		ci
		fld		st(3)					;	cj		ci-di	dj		di		cj		ci
		fsub	st,		st(2)			;	cj-dj	ci-di	dj		di		cj		ci
		fxch	st(2)					;	dj		ci-di	cj-dj	di		cj		ci
		faddp	st(4),	st				;	ci-di	cj-dj	di		cj+dj	ci
		fxch	st(2)					;	di		cj-dj	ci-di	cj+dj	ci
		faddp	st(4),	st				;	Dj		Di		cj+dj	ci+di
//		*(p1+1) = c+d;
//		*(p1+nn+1) = (c-d)*z;			;	st0		st1		st2		st3		st4		st5		st6		st7
		fld		st(1)					;	Di		Dj		Di		cj+dj	ci+di
		fmul	qword ptr[ebx+edx]		;	Di*zi	Dj		Di		cj+dj	ci+di
		fld		st(1)					;	Dj		Di*zi	Dj		Di		cj+dj	ci+di
		fmul	qword ptr[ebx+edx+8]	;	Dj*zj	Di*zi	Dj		Di		cj+dj	ci+di
		fxch	st(2)					;	Dj		Di*zi	Dj*zj	Di		cj+dj	ci+di
		fmul	qword ptr[ebx+edx]		;	Dj*zi	Di*zi	Dj*zj	Di		cj+dj	ci+di
		fxch	st(3)					;	Di		Di*zi	Dj*zj	Dj*zi	cj+dj	ci+di
		fmul	qword ptr[ebx+edx+8]	;	Di*zj	Di*zi	Dj*zj	Dj*zi	cj+dj	ci+di
		fxch	st(5)					;	ci+di	Di*zi	Dj*zj	Dj*zi	cj+dj	Di*zj
		fstp	dword ptr[esi+8]		;	Di*zi	Dj*zj	Dj*zi	cj+dj	Di*zj
		faddp	st(1),	st				;	D"i		Dj*zi	cj+dj	Di*zj
		fxch	st(2)					;	cj+dj	Dj*zi	D"i		Di*zj
		fstp	dword ptr[esi+12]		;	Dj*zi	D"i		Di*zj
		fsubrp	st(2),	st				;	D"i		D"j
		fstp	dword ptr[esi+ebp+8]	;
		fstp	dword ptr[esi+ebp+12]	;

		mov		ecx,	[cx0]			;	[ecx] -> W,		[ecx+edx] -> ZW
xr_x:	;//==============================	st0		st1		st2		st3		st4		st5		st6		st7
		or		edx,	edx				;
		jnz		xr_x2					;
//		if(y==0){ B = *p2;	D = *(p2+nn);}
		fld		dword ptr[edi]			;
		fld		dword ptr[edi+4]		;
		fld		dword ptr[edi+ebp]		;
		fld		dword ptr[edi+ebp+4]	;	Dj		Di		Bj		Bi
		jmp		xr_x4
//		else	{ B = *(p2+nn);	D = *p2;}
xr_x2:
		fld		dword ptr[edi+ebp]		;
		fld		dword ptr[edi+ebp+4]	;
		fld		dword ptr[edi]			;
		fld		dword ptr[edi+4]		;	Dj		Di		Bj		Bi
xr_x4:	;//		A = *p1; C = *(p1+nn);
		fld		dword ptr[esi]			;
		fld		dword ptr[esi+4]		;
		fld		dword ptr[esi+ebp]		;
		fld		dword ptr[esi+ebp+4]	;	Cj		Ci		Aj		Ai		Dj		Di		Bj		Bi
//			e.i = A.i+B.i;		f.j = (A.i-B.i);
//			e.j = A.j-B.j;		f.i =-(A.j+B.j);
//			g.i = C.i+D.i;		h.j = (C.i-D.i);
//			g.j = C.j-D.j;		h.i =-(C.j+D.j);
//			a = e+g; b = e-g; c = f+h; d = f-h;
		fsubr	st(4),	st				;//	st0		st1		st2		st3		st4		st5		st6		st7
		fadd	st,		st				;	2Cj		Ci		Aj		Ai		Cj-Dj	Di		Bj		Bi
		fxch	st(1)					;	Ci		2Cj		Aj		Ai		Cj-Dj	Di		Bj		Bi
		fsubr	st(5),	st				;
		fadd	st,		st				;	2Ci		2Cj		Aj		Ai		Cj-Dj	Ci-Di	Bj		Bi
		fxch	st(2)					;	Aj		2Cj		2Ci		Ai		Cj-Dj	Ci-Di	Bj		Bi
		fsubr	st(6),	st				;
		fadd	st,		st				;	2Aj		2Cj		2Ci		Ai		Cj-Dj	Ci-Di	Aj-Bj	Bi
		fxch	st(3)					;	Ai		2Cj		2Ci		2Aj		Cj-Dj	Ci-Di	Aj-Bj	Bi
		fsubr	st(7),	st				;//	2Ai		2Cj		2Ci		2Aj		Cj-Dj	Ci-Di	Aj-Bj	Ai-Bi
		fadd	st,		st				;//	2Ai		2Cj		2Ci		2Aj		gj		hj		ej		fj
		fxch	st(6)					;	ej		2Cj		2Ci		2Aj		gj		hj		2Ai		fj
		fsubr	st(3),	st				;	ej		2Cj		2Ci		fi		gj		hj		2Ai		fj
		fadd	st,		st(4)			;	ej+gj	2Cj		2Ci		fi		gj		hj		2Ai		fj
		fxch	st(4)					;	gj		2Cj		2Ci		fi		ej+gj	hj		2Ai		fj
		fsubr	st(1),	st				;	gj		hi		2Ci		fi		ej+gj	hj		2Ai		fj
		fadd	st,		st				;	2gj		hi		2Ci		fi		ej+gj	hj		2Ai		fj
		fxch	st(7)					;	fj		hi		2Ci		fi		ej+gj	hj		2Ai		2gj
		fsub	st(6),	st				;	fj		hi		2Ci		fi		ej+gj	hj		ei		2gj
		fadd	st,		st(5)			;	fj+hj	hi		2Ci		fi		ej+gj	hj		ei		2gj
		fxch	st(5)					;	hj		hi		2Ci		fi		ej+gj	fj+hj	ei		2gj
		fsub	st(2),	st				;	hj		hi		gi		fi		ej+gj	fj+hj	ei		2gj
		fadd	st,		st				;	2hj		hi		gi		fi		ej+gj	fj+hj	ei		2gj
		fxch	st(1)					;	hi		2hj		gi		fi		ej+gj	fj+hj	ei		2gj
		fadd	st(3),	st				;
		fadd	st,		st				;	2hi		2hj		gi		fi+hi	ej+gj	fj+hj	ei		2gj
		fxch	st(2)					;	gi		2hj		2hi		fi+hi	ej+gj	fj+hj	ei		2gj
		fadd	st(6),	st				;
		fadd	st,		st				;	2gi		2hj		2hi		fi+hi	ej+gj	fj+hj	ei+gi	2gj
		fxch	st(4)					;	ej+gj	2hj		2hi		fi+hi	2gi		fj+hj	ei+gi	2gj
		fsubr	st(7),	st				;	aj		2hj		2hi		fi+hi	2gi		fj+hj	ei+gi	bj
		fstp	dword ptr[esi+4]		;	2hj		2hi		fi+hi	2gi		fj+hj	ei+gi	bj
		fsubr	st,		st(4)			;	dj		2hi		fi+hi	2gi		cj		ei+gi	bj
		fxch	st(5)					;	ei+gi	2hi		fi+hi	2gi		cj		dj		bj
		fsubr	st(3),	st				;	ai		2hi		fi+hi	bi		cj		dj		bj
		fstp	dword ptr[esi]			;	2hi		fi+hi	bi		cj		dj		bj
		fsubr	st,	st(1)				;	di		ci		bi		cj		dj		bj
//		b *= z;	c *= w;	d *= zw			;//	st0		st1		st2		st3		st4		st5		st6		st7
		fld		st(2)					;	bi		di		ci		bi		cj		dj		bj
		fmul	qword ptr[ebx+edx]		;	bi*zi	di		ci		bi		cj		dj		bj
		fld		st(6)					;	bj		bi*zi	di		ci		bi		cj		dj		bj
		fmul	qword ptr[ebx+edx+8]	;	bj*zj	bi*zi	di		ci		bi		cj		dj		bj
		fxch	st(7)					;	bj		bi*zi	di		ci		bi		cj		dj		bj*zj
		fmul	qword ptr[ebx+edx]		;	bj*zi	bi*zi	di		ci		bi		cj		dj		bj*zj
		fxch	st(4)					;	bi		bi*zi	di		ci		bj*zi	cj		dj		bj*zj
		fmul	qword ptr[ebx+edx+8]	;	bi*zj	bi*zi	di		ci		bj*zi	cj		dj		bj*zj
		fxch	st(1)					;	bi*zi	bi*zj	di		ci		bj*zi	cj		dj		bj*zj
		faddp	st(7),	st				;	bi*zj	di		ci		bj*zi	cj		dj		Bi
		fld		st(2)					;	ci		bi*zj	di		ci		bj*zi	cj		dj		Bi
		fmul	qword ptr[ecx]			;	ci*wi	bi*zj	di		ci		bj*zi	cj		dj		Bi
		fxch	st(1)					;	bi*zj	ci*wi	di		ci		bj*zi	cj		dj		Bi
		fsubp	st(4),	st				;	ci*wi	di		ci		Bj		cj		dj		Bi
		fld		st(4)					;	cj		ci*wi	di		ci		Bj		cj		dj		Bi
		fmul	qword ptr[ecx+8]		;	cj*wj	ci*wi	di		ci		Bj		cj		dj		Bi
		fxch	st(3)					;	ci		ci*wi	di		cj*wj	Bj		cj		dj		Bi
		fmul	qword ptr[ecx+8]		;	ci*wj	ci*wi	di		cj*wj	Bj		cj		dj		Bi
		fxch	st(5)					;	cj		ci*wi	di		cj*wj	Bj		ci*wj	dj		Bi
		fmul	qword ptr[ecx]			;	cj*wi	ci*wi	di		cj*wj	Bj		ci*wj	dj		Bi
		fxch	st(1)					;	ci*wi	cj*wi	di		cj*wj	Bj		ci*wj	dj		Bi
		faddp	st(3),	st				;	cj*wi	di		Ci		Bj		ci*wj	dj		Bi
		fld		st(1)					;	di		cj*wi	di		Ci		Bj		ci*wj	dj		Bi
		fmul	qword ptr[ecx+edx]		;	di*zwi	cj*wi	di		Ci		Bj		ci*wj	dj		Bi
		fxch	st(1)					;	cj*wi	di*zwi	di		Ci		Bj		ci*wj	dj		Bi
		fsubrp	st(5),	st				;	di*zwi	di		Ci		Bj		Cj		dj		Bi
		fld		st(5)					;	dj		di*zwi	di		Ci		Bj		Cj		dj		Bi
		fmul	qword ptr[ecx+edx+8]	;	dj*zwj	di*zwi	di		Ci		Bj		Cj		dj		Bi
		fxch	st(6)					;	dj		di*zwi	di		Ci		Bj		Cj		dj*zwj	Bi
		fmul	qword ptr[ecx+edx]		;	dj*zwi	di*zwi	di		Ci		Bj		Cj		dj*zwj	Bi
		fxch	st(2)					;	di		di*zwi	dj*zwi	Ci		Bj		Cj		dj*zwj	Bi
		fmul	qword ptr[ecx+edx+8]	;	di*zwj	di*zwi	dj*zwi	Ci		Bj		Cj		dj*zwj	Bi
		fxch	st(1)					;	di*zwi	di*zwj	dj*zwi	Ci		Bj		Cj		dj*zwj	Bi
		faddp	st(6),	st				;	di*zwj	dj*zwi	Ci		Bj		Cj		Di		Bi
		fld		dword ptr[esi]			;	Ai		di*zwj	dj*zwi	Ci		Bj		Cj		Di		Bi
//	a.i = A.i + C.i;	c.i = A.i-C.i;
//	a.j = A.j + C.j;	c.j = C.j-A.j;
//	b.i = B.i + D.i;	d.i = B.i-D.i;
//	b.j = B.j + D.j;	d.j = D.j-B.j	;//	st0		st1		st2		st3		st4		st5		st6		st7
		fadd	st(3),	st				;	Ai		di*zwj	dj*zwi	Ai+Ci	Bj		Cj		Di		Bi
		fadd	st,		st				;	2Ai		di*zwj	dj*zwi	Ai+Ci	Bj		Cj		Di		Bi
		fxch	st(1)					;	di*zwj	2Ai		dj*zwi	Ai+Ci	Bj		Cj		Di		Bi
		fsubp	st(2),	st				;	2Ai		Dj		Ai+Ci	Bj		Cj		Di		Bi
		fsub	st,		st(2)			;	Ai-Ci	Dj		Ai+Ci	Bj		Cj		Di		Bi
		fld		dword ptr[esi+4]		;	Aj		Ai-Ci	Dj		Ai+Ci	Bj		Cj		Di		Bi
		fadd	st(5),	st				;
		fadd	st,		st				;	2Aj		Ai-Ci	Dj		Ai+Ci	Bj		Aj+Cj	Di		Bi
		fxch	st(2)					;	Dj		Ai-Ci	2Aj		Ai+Ci	Bj		Aj+Cj	Di		Bi
		fadd	st(4),	st				;
		fadd	st,		st				;	2Dj		Ai-Ci	2Aj		Ai+Ci	Bj+Dj	Aj+Cj	Di		Bi
		fxch	st(2)					;	2Aj		Ai-Ci	2Dj		Ai+Ci	Bj+Dj	Aj+Cj	Di		Bi
		fsubr	st,		st(5)			;	Aj-Cj	Ai-Ci	2Dj		Ai+Ci	Bj+Dj	Aj+Cj	Di		Bi
		fxch	st(2)					;	2Dj		Ai-Ci	Aj-Cj	Ai+Ci	Bj+Dj	Aj+Cj	Di		Bi
		fsub	st,		st(4)			;	Bj-Dj	Ai-Ci	Aj-Cj	Ai+Ci	Bj+Dj	Aj+Cj	Di		Bi
		fxch	st(6)					;	Di		Ai-Ci	Aj-Cj	Ai+Ci	Bj+Dj	Aj+Cj	Bj-Dj	Bi
		fadd	st(7),	st				;
		fadd	st,		st				;	2Di		Ai-Ci	Aj-Cj	Ai+Ci	Bj+Dj	Aj+Cj	Bj-Dj	Bi+Di
		fxch	st(5)					;	Aj+Cj	Ai-Ci	Aj-Cj	Ai+Ci	Bj+Dj	2Di		Bj-Dj	Bi+Di
//	*p1 = A; *(p1+nn) = B;				;	aj		ci		cj		ai		bj				dj
//	*p2 = C; *(p2+nn) = D;
		fstp	dword ptr[esi+4]		;
		fstp	dword ptr[edi]			;
		fstp	dword ptr[edi+4]		;
		fstp	dword ptr[esi]			;
		fstp	dword ptr[esi+ebp+4]	;	2Di		Bj-Dj	Bi+Di
		fsubr	st,		st(2)			;
		fxch	st(2)					;	Bi+Di	Bj-Dj	Bi-Di
		fstp	dword ptr[esi+ebp]		;
		fstp	dword ptr[edi+ebp+4]	;
		fstp	dword ptr[edi+ebp]		;
;----------------------------------------------
		sub		ecx,	2*8				; cx--
		sub		esi,	2*4				; p1--
		add		edi,	2*4				; p2++
		cmp		ecx,	ebx				;
		jg		xr_x					;
;==========================================
//		a = *p1;	b = *(p1+nn);
//		d.i = b.i+b.j;		d.j = b.i-b.j;
//		c.i = a.i+a.j;		c.j = a.i-a.j;
//		a = c+d;			b = c-d;
//		*(p1+nn) = b * z;	*p1 = a;
		fld		dword ptr[esi]			;	st0		st1		st2		st3		st4		st5		st6		st7
		fadd	dword ptr[esi+4]		;	ci
		fld		dword ptr[esi]			;
		fsub	dword ptr[esi+4]		;	cj		ci
		fld		dword ptr[esi+ebp]		;
		fadd	dword ptr[esi+ebp+4]	;	di		cj		ci
		fld		dword ptr[esi+ebp]		;
		fsub	dword ptr[esi+ebp+4]	;	dj		di		cj		ci
		fld		st(3)					;	ci		dj		di		cj		ci
		fadd	st,		st(2)			;	ai		dj		di		cj		ci
		fld		st(3)					;	cj		ai		dj		di		cj		ci
		fadd	st,		st(2)			;	aj		ai		dj		di		cj		ci
		fxch	st(5)					;	ci		ai		dj		di		cj		aj
		fsubrp	st(3),	st				;	ai		dj		bi		cj		aj
		fxch	st(3)					;	cj		dj		bi		ai		aj
		fsubrp	st(1),	st				;	bj		bi		ai		aj
		fld		st(1)					;	bi		bj		bi		ai		aj
		fmul	qword ptr[ebx+edx]		;	bi*zi	bj		bi		ai		aj
		fld		st(1)					;	bj		bi*zi	bj		bi		ai		aj
		fmul	qword ptr[ebx+edx+8]	;	bj*zj	bi*zi	bj		bi		ai		aj
		fxch	st(2)					;	bj		bi*zi	bj*zj	bi		ai		aj
		fmul	qword ptr[ebx+edx]		;	bj*zi	bi*zi	bj*zj	bi		ai		aj
		fxch	st(3)					;	bi		bi*zi	bj*zj	bj*zi	ai		aj
		fmul	qword ptr[ebx+edx+8]	;	bi*zj	bi*zi	bj*zj	bj*zi	ai		aj
		fxch	st(1)					;	bi*zi	bi*zj	bj*zj	bj*zi	ai		aj
		faddp	st(2),	st				;	bi*zj	Bi		bj*zi	ai		aj
		fxch	st(4)					;	aj		Bi		bj*zi	ai		bi*zj
		fstp	dword ptr[esi+4]		;
		fstp	dword ptr[esi+ebp]		;	bj*zi	ai		bi*zj
		fsubrp	st(2),	st				;	ai		Bj
		fstp	dword ptr[esi]			;
		fstp	dword ptr[esi+ebp+4]	;

;----------------------------------------
		sub		esi,	[s1]			;
		add		edi,	[s1]			;
		sub		edx,	2*8				; y--
		jg		xr_y					;
		lea		edi,	[esi+2*2*4]
		jge		xr_y					;

		pop		ebp
}// ASM	===================================

#endif
}

//####################################################################################
void FT2R::fft2F(FCOMPLEX * Fp, DCOMPLEX * cst, int S){ //Freq Domain FFT
static	DCOMPLEX  v;//,*su;
static	FCOMPLEX *f1, *f2;
static	int	n, s, ct, cv, tx;
float	k = 0.25/S/S;
int		jv,kv;

#if(0)
int ju,ku;
FCOMPLEX		a,b,c,d,A,B,C,D;
	for(tx = 8*(1-S), n=1, ct=16*2, s=S/2; s>1; s/=2, ct *= 2, n *= 2)
	 for(cv=kv=0; kv<s; kv++,cv+=ct)
	  for(v = cst[cv/16], jv=0; jv<S; jv+=s*2) {
		f1 = Fp+(jv+kv)*S;	f2 = f1 + s*S;
		for(su=cst, ku=s; ku>0; ku--, f1 += tx/8, f2 += tx/8, su+=ct/16)
			for(ju=n; ju>0; ju--){
			a = f1[0];	b = f1[s];	c = f2[0];	d = f2[s];
			A = a+c;	C = a-c;	B = b+d;	D = b-d;
			a = A+B;	b = A-B;	c = C+D;	d = C-D;
			b*= ~su[0];	c *= ~v; d*= ~su[cv/16];
			f1[0] = a;	f1[s] = b;	f2[0] = c;	f2[s] = d;
			f1 += s+s;	f2+=s+s;
		}
	}
#else
	for(tx = 8*(1-S), n=1, ct=16*2, s=S/2; s>1; s/=2, ct *= 2, n *= 2)
	 for(cv=kv=0; kv<s; kv++,cv+=ct)
	  for(v = cst[cv/16], jv=0; jv<S; jv+=s*2) {
		f1 = Fp+(jv+kv)*S;	f2 = f1 + s*S;
_asm{	//===============================
		mov		esi,	[f1]			;
		mov		edi,	[f2]			;
		mov		ebx,	[s]				;
		shl		ebx,	3				;
		mov		ecx,	[cst]			;
		mov		edx,	[cv]			;
		push	ebp						;
		mov		ebp,	[s]				;
ift1_k:	//=============================================================================================
		mov		eax,	[n]				;
ift1_j:									;//	st0		st1		st2		st3		st4		st5		st6		st7

//		a = f1[0];	b = f1[s];	c = f2[0];	d = f2[s];
		fld		dword ptr[esi]			;
		fld		dword ptr[esi+4]		;
		fld		dword ptr[esi+ebx]		;
		fld		dword ptr[esi+ebx+4]	;
		fld		dword ptr[edi]			;
		fld		dword ptr[edi+4]		;
		fld		dword ptr[edi+ebx]		;
		fld		dword ptr[edi+ebx+4]	;	dj		di		cj		ci		bj		bi		aj		ai
//		A = a+c;	C = a-c;	B = b+d;	D = b-d;
//		a = A+B;	b = A-B;	c = C+D;	d = C-D;

		fsub	st(4),	st				;
		fadd	st,		st				;	2dj		di		cj		ci		bj-dj	bi		aj		ai
		fxch	st(1)					;	di		2dj		cj		ci		bj-dj	bi		aj		ai
		fsub	st(5),	st				;
		fadd	st,		st				;	2di		2dj		cj		ci		bj-dj	bi-di	aj		ai
		fxch	st(2)					;	cj		2dj		2di		ci		bj-dj	bi-di	aj		ai
		fsub	st(6),	st				;
		fadd	st,		st				;	2cj		2dj		2di		ci		bj-dj	bi-di	aj-cj	ai
		fxch	st(3)					;	ci		2dj		2di		2cj		bj-dj	bi-di	aj-cj	ai
		fsub	st(7),	st				;
		fadd	st,		st				;	2ci		2dj		2di		2cj		Dj		Di		Cj		Ci
		fxch	st(6)					;	Cj		2dj		2di		2cj		Dj		Di		2ci		Ci
		fadd	st(3),	st				;	Cj		2dj		2di		Aj		Dj		Di		2ci		Ci
		fadd	st,		st(4)			;	Cj+Dj	2dj		2di		Aj		Dj		Di		2ci		Ci
		fxch	st(4)					;	Dj		2dj		2di		Aj		Cj+Dj	Di		2ci		Ci
		fadd	st(1),	st				;
		fadd	st,		st				;	2Dj		Bj		2di		Aj		Cj+Dj	Di		2ci		Ci
		fxch	st(7)					;	Ci		Bj		2di		Aj		Cj+Dj	Di		2ci		2Dj
		fadd	st(6),	st				;	Ci		Bj		2di		Aj		Cj+Dj	Di		Ai		2Dj
		fadd	st,		st(5)			;	Ci+Di	Bj		2di		Aj		Cj+Dj	Di		Ai		2Dj
		fxch	st(5)					;	Di		Bj		2di		Aj		Cj+Dj	Ci+Di	Ai		2Dj
		fadd	st(2),	st				;
		fadd	st,		st				;	2Di		Bj		Bi		Aj		Cj+Dj	Ci+Di	Ai		2Dj
		fxch	st(2)					;	Bi		Bj		2Di		Aj		Cj+Dj	Ci+Di	Ai		2Dj

		fsub	st(6),	st				;
		fadd	st,		st				;	2Bi		Bj		2Di		Aj		Cj+Dj	Ci+Di	Ai-Bi	2Dj
		fxch	st(1)					;	Bj		2Bi		2Di		Aj		Cj+Dj	Ci+Di	Ai-Bi	2Dj
		fadd	st(3),	st				;
		fadd	st,		st				;	2Bj		2Bi		2Di		Aj+Bj	Cj+Dj	Ci+Di	Ai-Bi	2Dj
		fxch	st(6)					;	Ai-Bi	2Bi		2Di		Aj+Bj	Cj+Dj	Ci+Di	2Bj		2Dj
		fadd	st(1),	st				;	Ai-Bi	Ai+Bi	2Di		Aj+Bj	Cj+Dj	Ci+Di	2Bj		2Dj
		fxch	st(3)					;	Aj+Bj	Ai+Bi	2Di		Ai-Bi	Cj+Dj	Ci+Di	2Bj		2Dj
		fsubr	st(6),	st				;	Aj+Bj	Ai+Bi	2Di		Ai-Bi	Cj+Dj	Ci+Di	Aj-Bj	2Dj
		fstp	dword ptr[esi+4]		;//	st0		st1		st2		st3		st4		st5		st6		st7
		fstp	dword ptr[esi]			;	2Di		Ai-Bi	Cj+Dj	Ci+Di	Aj-Bj	2Dj
		fsubr	st,		st(3)			;	Ci-Di	Ai-Bi	Cj+Dj	Ci+Di	Aj-Bj	2Dj
		fxch	st(2)					;	Cj+Dj	Ai-Bi	Ci-Di	Ci+Di	Aj-Bj	2Dj
		fsubr	st(5),	st				;	cj		bi		di		ci		bj		dj
//	b*= ~su[0];	c *= ~v; d*= ~su[cv/16]	;	cj		bi		di		ci		bj		dj
		fld		st(1)					;	bi		cj		bi		di		ci		bj		dj
		fmul	qword ptr[ecx]			;	bi*cu	cj		bi		di		ci		bj		dj
		fld		st(5)					;	bj		bi*cu	cj		bi		di		ci		bj		dj
		fmul	qword ptr[ecx+8]		;	bj*su	bi*cu	cj		bi		di		ci		bj		dj
		fxch	st(6)					;	bj		bi*cu	cj		bi		di		ci		bj*su	dj
		fmul	qword ptr[ecx]			;	bj*cu	bi*cu	cj		bi		di		ci		bj*su	dj
		fxch	st(3)					;	bi		bi*cu	cj		bj*cu	di		ci		bj*su	dj
		fmul	qword ptr[ecx+8]		;	bi*su	bi*cu	cj		bj*cu	di		ci		bj*su	dj
		fxch	st(1)					;	bi*cu	bi*su	cj		bj*cu	di		ci		bj*su	dj
		faddp	st(6),	st				;	bi*su	cj		bj*cu	di		ci		Bi		dj
		fld		st(4)					;	ci		bi*su	cj		bj*cu	di		ci		Bi		dj
		fmul	qword ptr[v]			;	ci*cv	bi*su	cj		bj*cu	di		ci		Bi		dj
		fxch	st(1)					;	bi*su	ci*cv	cj		bj*cu	di		ci		Bi		dj
		fsubp	st(3),	st				;	ci*cv	cj		Bj		di		ci		Bi		dj
		fld		st(1)					;	cj		ci*cv	cj		Bj		di		ci		Bi		dj
		fmul	qword ptr[v+8]			;	cj*sv	ci*cv	cj		Bj		di		ci		Bi		dj
		fxch	st(5)					;	ci		ci*cv	cj		Bj		di		cj*sv	Bi		dj
		fmul	qword ptr[v+8]			;	ci*sv	ci*cv	cj		Bj		di		cj*sv	Bi		dj
		fxch	st(2)					;	cj		ci*cv	ci*sv	Bj		di		cj*sv	Bi		dj
		fmul	qword ptr[v]			;	cj*cv	ci*cv	ci*sv	Bj		di		cj*sv	Bi		dj
		fxch	st(1)					;	ci*cv	cj*cv	ci*sv	Bj		di		cj*sv	Bi		dj
		faddp	st(5),	st				;	cj*cv	ci*sv	Bj		di		Ci		Bi		dj
		fld		st(3)					;	di		cj*cv	ci*sv	Bj		di		Ci		Bi		dj
		fmul	qword ptr[ecx+edx]		;	di*cx	cj*cv	ci*sv	Bj		di		Ci		Bi		dj
		fxch	st(1)					;	cj*cv	di*cx	ci*sv	Bj		di		Ci		Bi		dj
		fsubrp	st(2),	st				;	di*cx	Cj		Bj		di		Ci		Bi		dj
		fld		st(6)					;	dj		di*cx	Cj		Bj		di		Ci		Bi		dj
		fmul	qword ptr[ecx+edx+8]	;	dj*sx	di*cx	Cj		Bj		di		Ci		Bi		dj
		fxch	st(4)					;	di		di*cx	Cj		Bj		dj*sx	Ci		Bi		dj
		fmul	qword ptr[ecx+edx+8]	;	di*sx	di*cx	Cj		Bj		dj*sx	Ci		Bi		dj
		fxch	st(7)					;	dj		di*cx	Cj		Bj		dj*sx	Ci		Bi		di*sx
		fmul	qword ptr[ecx+edx]		;	dj*cx	di*cx	Cj		Bj		dj*sx	Ci		Bi		di*sx
		fxch	st(1)					;	di*cx	dj*cx	Cj		Bj		dj*sx	Ci		Bi		di*sx
		faddp	st(4),	st				;	dj*cx	Cj		Bj		Di		Ci		Bi		di*sx
		fxch	st(4)					;	Ci		Cj		Bj		Di		dj*cx	Bi		di*sx
//		f1[s] = b;	f2[0] = c;	f2[s]=d ;//	st0		st1		st2		st3		st4		st5		st6		st7
		fstp	dword ptr[edi]			;
		fstp	dword ptr[edi+4]		;
		fstp	dword ptr[esi+ebx+4]	;
		fstp	dword ptr[edi+ebx]		;	dj*cx	Bi		di*sx
		fsubrp	st(2),	st				;	Bi		Dj
		fstp	dword ptr[esi+ebx]		;
		fstp	dword ptr[edi+ebx+4]	;

		lea		esi,	[esi+ebx*2]		; f1 += 2*s
		lea		edi,	[edi+ebx*2]		; f2 += 2*s
		dec		eax						; ju--
		jnz		ift1_j					;
;--------------------------------------------------
		add		ecx,	[ct]			; su += ct
		add		esi,	[tx]			; f1 += 1-S
		add		edi,	[tx]			; f2 += 1-S
		dec		ebp						; ku--
		jnz		ift1_k					;
		pop		ebp						;
		}// ASM	===================================
	}
#endif
//================================================
#if(0)
	for(f1=Fp, jv=0; jv<S; jv+=2) {
			for(ju=n; ju>0; ju--){
			A = f1[0]*k;	B = f1[1]*k;	C = f1[S]*k;	D = f1[S+1]*k;
			a = A+C;	c = A-C;	b = B+D;	d = B-D;

			f1[0] = c-d;	f1[1] = c+d;	f1[S] = a-b;	f1[S+1] = a+b;
			f1 += 2;
		}
		f1+=S;
	}
#else
_asm{
		mov		esi,	[Fp]			;
		mov		edx,	[S]				;
		lea		edi,	[edx*8]			;
		shr		edx,	1				;
		fld		[k]						;	k
ift2_y:									;
		mov		ecx,	[S]				;
		shr		ecx,	1				;
ift2_x:									;
		fld		dword ptr[esi]			;//	st0		st1		st2		st3		st4		st5		st6		st7
		fmul	st,		st(1)			;	A		k
		fld		dword ptr[esi+8]		;
		fmul	st,		st(2)			;	B		A		k
		fld		dword ptr[esi+edi]		;
		fmul	st,		st(3)			;	C		B		A		k
		fld		dword ptr[esi+edi+8]	;
		fmul	st,		st(4)			;	D		C		B		A		k
		fld		st(3)					;	A		D		C		B		A		k
		fadd	st,		st(2)			;	a		D		C		B		A		k
		fxch	st(4)					;	A		D		C		B		a		k
		fsubrp	st(2),	st				;	D		c		B		a		k
		fld		st(2)					;	B		D		c		B		a		k
		fadd	st,		st(1)			;	b		D		c		B		a		k
		fxch	st(3)					;	B		D		c		b		a		k
		fsubrp	st(1),	st				;	d		c		b		a		k
		fld		st(3)					;	a		d		c		b		a		k
		fadd	st,		st(3)			;	a+b		d		c		b		a		k
		fxch	st(4)					;	a		d		c		b		a+b		k
		fsubrp	st(3),	st				;	d		c		a-b		a+b		k
		fld		st						;	d		d		c		a-b		a+b		k
		fadd	st,		st(2)			;	c+d		d		c		a-b		a+b		k
		fxch	st(1)					;	d		c+d		c		a-b		a+b		k
		fsubp	st(2),	st				;	c+d		c-d		a-b		a+b		k
		fstp	dword ptr[esi+8]		;
		fstp	dword ptr[esi]			;
		fstp	dword ptr[esi+edi]		;
		fstp	dword ptr[esi+edi+8]	;
		fld		dword ptr[esi+4]		;//	st0		st1		st2		st3		st4		st5		st6		st7
		fmul	st,		st(1)			;	A		k
		fld		dword ptr[esi+8+4]		;
		fmul	st,		st(2)			;	B		A		k
		fld		dword ptr[esi+edi+4]	;
		fmul	st,		st(3)			;	C		B		A		k
		fld		dword ptr[esi+edi+8+4]	;
		fmul	st,		st(4)			;	D		C		B		A		k
		fld		st(3)					;	A		D		C		B		A		k
		fadd	st,		st(2)			;	a		D		C		B		A		k
		fxch	st(4)					;	A		D		C		B		a		k
		fsubrp	st(2),	st				;	D		c		B		a		k
		fld		st(2)					;	B		D		c		B		a		k
		fadd	st,		st(1)			;	b		D		c		B		a		k
		fxch	st(3)					;	B		D		c		b		a		k
		fsubrp	st(1),	st				;	d		c		b		a		k
		fld		st(3)					;	a		d		c		b		a		k
		fadd	st,		st(3)			;	a+b		d		c		b		a		k
		fxch	st(4)					;	a		d		c		b		a+b		k
		fsubrp	st(3),	st				;	d		c		a-b		a+b		k
		fld		st						;	d		d		c		a-b		a+b		k
		fadd	st,		st(2)			;	c+d		d		c		a-b		a+b		k
		fxch	st(1)					;	d		c+d		c		a-b		a+b		k
		fsubp	st(2),	st				;	c+d		c-d		a-b		a+b		k
		fstp	dword ptr[esi+8+4]		;
		fstp	dword ptr[esi+4]		;
		fstp	dword ptr[esi+edi+4]	;
		fstp	dword ptr[esi+edi+8+4]	;
		add		esi,	2*8				;
		dec		ecx						;
		jnz		ift2_x					;
		add		esi,	edi				;
		dec		edx						;
		jnz		ift2_y					;
		fstp	st						;
	}
#endif
}
#define C_FFT
//####################################################################################
//-------------------------------------------------------------------
void FT2R::fft2T(FCOMPLEX * Fp, DCOMPLEX * cst, int S){ // Time Domain	FFT
static int jv,kv,s,ct,cv,n,tx;//,ju,ku;
static	DCOMPLEX  v;//,*su;
static	FCOMPLEX *f1=NULL, *f2=NULL, i(0,1.);
#if(1)
FCOMPLEX	a,b,c,d,e,f,g,h,aa,bb,cc,dd,ee,ff,gg,hh;
#endif
#ifdef C_FFT
	static int ju,ku;
	tx=(1-S);	s=1; f1 = Fp;
	for(jv=S/2; jv>0; jv--) {
		  for(ju=S/4; ju>0; ju--){
			aa = f1[S+1];	bb = f1[S+0];	ee = f1[S+3];	ff = f1[S+2];
			cc = f1[1];		dd = f1[0];		gg = f1[3];		hh = f1[2];

			a=aa+cc;		b = bb+dd;		e = ee+gg;		f = ff+hh;
			c=aa-cc;		d = bb-dd;		g = ee-gg;		h = ff-hh;

			f1[0]	= a+b + (e+f);
			f1[2]	= a+b - (e+f);
			f1[S]	= c+d + (g+h);
			f1[S+2]	= c+d - (g+h);

			f1[1].j		= a.j-b.j + (e.i-f.i);
			f1[3].j		= a.j-b.j - (e.i-f.i);
			f1[S+1].j	= c.j-d.j + (g.i-h.i);
			f1[S+3].j	= c.j-d.j - (g.i-h.i);
			f1[1].i		= a.i-b.i - (e.j-f.j);
			f1[3].i		= a.i-b.i + (e.j-f.j);
			f1[S+1].i	= c.i-d.i - (g.j-h.j);
			f1[S+3].i	= c.i-d.i + (g.j-h.j);
			f1 += 4;
		}
		f1 += S;
	}
#else
_asm {
		mov		esi,	[Fp]			;
		mov		edi,	[S]				;
		mov		edx,	edi				;
		shl		edi,	3				;
		shr		edx,	1				;
f2t1_y:									;
		mov		ecx,	[S]				;
		shr		ecx,	2				;
f2t1_x:
		fld		dword ptr[esi+edi]		;	st0		st1		st2		st3		st4		st5		st6		st7
		fld		dword ptr[esi+edi+8]	;
		fld		dword ptr[esi+edi+8*2]	;
		fld		dword ptr[esi+edi+8*3]	;	eei		ffi		aai		bbi
		fld		dword ptr[esi]			;
		fld		dword ptr[esi+8]		;
		fld		dword ptr[esi+8*2]		;
		fld		dword ptr[esi+8*3]		;	ggi		hhi		cci		ddi		eei		ffi		aai		bbi
		fxch	st(3)					;	ddi		hhi		cci		ggi		eei		ffi		aai		bbi
		fsub	st(7),	st				;
		fadd	st,		st				;	2ddi	hhi		cci		ggi		eei		ffi		aai		di
		fxch	st(2)					;	cci		hhi		2ddi	ggi		eei		ffi		aai		di
		fsub	st(6),	st				;
		fadd	st,		st				;	2cci	hhi		2ddi	ggi		eei		ffi		ci		di
		fxch	st(3)					;	ggi		hhi		2ddi	2cci	eei		ffi		ci		di
		fsub	st(4),	st				;
		fadd	st,		st				;	2ggi	hhi		2ddi	2cci	gi		ffi		ci		di
		fxch	st(1)					;	hhi		2ggi	2ddi	2cci	gi		ffi		ci		di
		fsub	st(5),	st				;
		fadd	st,		st				;	2hhi	2ggi	2ddi	2cci	gi		hi		ci		di
		fxch	st(1)					;	2ggi	2hhi	2ddi	2cci	gi		hi		ci		di
		fadd	st,		st(4)			;	ei		2hhi	2ddi	2cci	gi		hi		ci		di
		fxch	st(1)					;	2hhi	ei		2ddi	2cci	gi		hi		ci		di
		fadd	st,		st(5)			;	fi		ei		2ddi	2cci	gi		hi		ci		di
		fxch	st(2)					;	2ddi	ei		fi		2cci	gi		hi		ci		di
		fadd	st,		st(7)			;	bi		ei		fi		2cci	gi		hi		ci		di
		fxch	st(6)					;	ci		ei		fi		2cci	gi		hi		bi		di
		fadd	st(3),	st				;	ci		ei		fi		ai		gi		hi		bi		di
		fadd	st(7),	st				;	ci		ei		fi		ai		gi		hi		bi		ci+di
		fadd	st,		st				;	2ci		ei		fi		ai		gi		hi		bi		ci+di
		fxch	st(3)					;	ai		ei		fi		2ci		gi		hi		bi		ci+di
		fadd	st(6),	st				;	ai		ei		fi		2ci		gi		hi		ai+bi	ci+di
		fadd	st,		st				;	2ai		ei		fi		2ci		gi		hi		ai+bi	ci+di
		fxch	st(1)					;	ei		2ai		fi		2ci		gi		hi		ai+bi	ci+di
		fadd	st(2),	st				;	ei		2ai		ei+fi	2ci		gi		hi		ai+bi	ci+di
		fadd	st,		st				;	2ei		2ai		ei+fi	2ci		gi		hi		ai+bi	ci+di
		fxch	st(4)					;	gi		2ai		ei+fi	2ci		2ei		hi		ai+bi	ci+di
		fadd	st(5),	st				;	gi		2ai		ei+fi	2ci		2ei		gi+hi	ai+bi	ci+di
		fadd	st,		st				;	2gi		2ai		ei+fi	2ci		2ei		gi+hi	ai+bi	ci+di
		fxch	st(6)					;	ai+bi	2ai		ei+fi	2ci		2ei		gi+hi	2gi		ci+di
		fsub	st(1),	st				;	ai+bi	ai-bi	ei+fi	2ci		2ei		gi+hi	2gi		ci+di
		fxch	st(3)					;	2ci		ai-bi	ei+fi	ai+bi	2ei		gi+hi	2gi		ci+di
		fsub	st,		st(7)			;	ci-di	ai-bi	ei+fi	ai+bi	2ei		gi+hi	2gi		ci+di
		fxch	st(4)					;	2ei		ai-bi	ei+fi	ai+bi	ci-di	gi+hi	2gi		ci+di
		fsub	st,		st(2)			;	ei-fi	ai-bi	ei+fi	ai+bi	ci-di	gi+hi	2gi		ci+di
		fxch	st(6)					;	2gi		ai-bi	ei+fi	ai+bi	ci-di	gi+hi	ei-fi	ci+di
		fsub	st,		st(5)			;	gi-hi	ai-bi	ei+fi	ai+bi	ci-di	gi+hi	ei-fi	ci+di
		fxch	st(2)					;	ei+fi	ai-bi	gi-hi	ai+bi	ci-di	gi+hi	ei-fi	ci+di
//			f1[0] = a+b + (e+f);
//			f1[2] = a+b - (e+f);
//			f1[S] = c+d + (g+h);
//			f1[S+2]=c+d - (g+h);		;	st0		st1		st2		st3		st4		st5		st6		st7
		fsub	st(3),	st				;	ei+fi	ai-bi	gi-hi	f[2].i	ci-di	gi+hi	ei-fi	ci+di
		fadd	st,		st				;	2ei+fi	ai-bi	gi-hi	f[2].i	ci-di	gi+hi	ei-fi	ci+di
		fxch	st(7)					;	ci+di	ai-bi	gi-hi	f[2].i	ci-di	gi+hi	ei-fi	2(ei+fi)
		fadd	st(5),	st				;	ci+di	ai-bi	gi-hi	f[2].i	ci-di	f[S].i	ei-fi	2(ei+fi)
		fadd	st,		st				;	2(ci+di)ai-bi	gi-hi	f[2].i	ci-di	f[S].i	ei-fi	2(ei+fi)
		fxch	st(3)					;	f[2].i	ai-bi	gi-hi	2(ci+di)ci-di	f[S].i	ei-fi	2(ei+fi)
		fadd	st(7),	st				;	F[2].i	ai-bi	gi-hi	2(ci+di)ci-di	F[S].i	ei-fi	F[0].i
		fstp	dword ptr[esi+8*2]		;	ai-bi	gi-hi	2(ci+di)ci-di	F[S].i	ei-fi	F[0].i
		fxch	st(4)					;	F[S].i	gi-hi	2(ci+di)ci-di	ai-bi	ei-fi	F[0].i
		fsub	st(2),	st				;	F[S].i	gi-hi	F[S+2]i	ci-di	ai-bi	ei-fi	F[0].i
		fstp	dword ptr[esi+edi]		;	gi-hi	F[S+2]i	ci-di	ai-bi	ei-fi	F[0].i
		fxch	st(5)					;	F[0].i	F[S+2]i	ci-di	ai-bi	ei-fi	gi-hi
		fstp	dword ptr[esi]			;
		fstp	dword ptr[esi+edi+8*2]	;	ci-di	ai-bi	ei-fi	gi-hi
//			aa = f1[S+1];	bb = f1[S+0];	ee = f1[S+3];	ff = f1[S+2];
//			cc = f1[1];		dd = f1[0];		gg = f1[3];		hh = f1[2];

//			a=aa+cc;		b = bb+dd;		e = ee+gg;		f = ff+hh;
//			c=aa-cc;		d = bb-dd;		g = ee-gg;		h = ff-hh;

//			f1[1].j = a.j-b.j + (e.i-f.i);
//			f1[3].j = a.j-b.j - (e.i-f.i);
//			f1[S+1].j=c.j-d.j + (g.i-h.i);
//			f1[S+3].j=c.j-d.j - (g.i-h.i);
		fld		dword ptr[esi+4]		;
		fld		dword ptr[esi+8+4]		;
		fld		dword ptr[esi+edi+4]	;//	st0		st1		st2		st3		st4		st5		st6		st7
		fld		dword ptr[esi+edi+8+4]	;	aaj		bbj		ccj		ddj		ci-di	ai-bi	ei-fi	gi-hi
		fadd	st(2),	st				;
		fadd	st,		st				;	2aaj	bbj		aj		ddj		ci-di	ai-bi	ei-fi	gi-hi
		fxch	st(1)					;	bbj		2aaj	aj		ddj		ci-di	ai-bi	ei-fi	gi-hi
		fadd	st(3),	st				;
		fadd	st,		st				;	2bbj	2aaj	aj		bj		ci-di	ai-bi	ei-fi	gi-hi
		fxch	st(1)					;	2aaj	2bbj	aj		bj		ci-di	ai-bi	ei-fi	gi-hi
		fsub	st,		st(2)			;	cj		2bbj	aj		bj		ci-di	ai-bi	ei-fi	gi-hi
		fxch	st(3)					;	bj		2bbj	aj		cj		ci-di	ai-bi	ei-fi	gi-hi
		fsub	st(1),	st				;	bj		dj		aj		cj		ci-di	ai-bi	ei-fi	gi-hi
		fsub	st(2),	st				;
		fadd	st,		st				;	2bj		dj		aj-bj	cj		ci-di	ai-bi	ei-fi	gi-hi
		fxch	st(1)					;	dj		2bj		aj-bj	cj		ci-di	ai-bi	ei-fi	gi-hi
		fsub	st(3),	st				;
		fadd	st,		st				;	2dj		2bj		aj-bj	cj-dj	ci-di	ai-bi	ei-fi	gi-hi
		fxch	st(1)					;	2bj		2dj		aj-bj	cj-dj	ci-di	ai-bi	ei-fi	gi-hi
		fadd	st,		st(2)			;	aj+bj	2dj		aj-bj	cj-dj	ci-di	ai-bi	ei-fi	gi-hi
		fxch	st(6)					;	ei-fi	2dj		aj-bj	cj-dj	ci-di	ai-bi	aj+bj	gi-hi
		fadd	st(2),	st				;
		fadd	st,		st				;	2ef		2dj		ab+ef	cj-dj	ci-di	ai-bi	aj+bj	gi-hi
		fxch	st(1)					;	2dj		2ef		ab+ef	cj-dj	ci-di	ai-bi	aj+bj	gi-hi
		fadd	st,		st(3)			;	cj+dj	2ef		ab+ef	cj-dj	ci-di	ai-bi	aj+bj	gi-hi
		fxch	st(7)					;	gi-hi	2ef		ab+ef	cj-dj	ci-di	ai-bi	aj+bj	cj+dj
		fadd	st(3),	st				;
		fadd	st,		st				;	2gh		2ef		ab+ef	cd+gh	ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(1)					;	2ef		2gh		ab+ef	cd+gh	ci-di	ai-bi	aj+bj	cj+dj
		fsubr	st,		st(2)			;	ab-ef	2gh		ab+ef	cd+gh	ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(1)					;	2gh		ab-ef	ab+ef	cd+gh	ci-di	ai-bi	aj+bj	cj+dj
		fsubr	st,		st(3)			;	F[S+3]j	F[3]j	F[1]j	F[S+1]j	ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(2)					;	F[1]j	F[3]j	F[S+3]j	F[S+1]j	ci-di	ai-bi	aj+bj	cj+dj
		fstp	dword ptr[esi+8+4]		;	F[3]j	F[S+3]j	F[S+1]j	ci-di	ai-bi	aj+bj	cj+dj
		fld		dword ptr[esi+8*3+4]	;	ggi		F[3]j	F[S+3]j	F[S+1]j	ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(3)					;	F[S+1]j	F[3]j	F[S+3]j	ggi		ci-di	ai-bi	aj+bj	cj+dj
		fstp	dword ptr[esi+edi+8+4]	;	F[3]j	F[S+3]j	ggi		ci-di	ai-bi	aj+bj	cj+dj
		fld		dword ptr[esi+edi+8*3+4];	eej		F[3]j	F[S+3]j	ggi		ci-di	ai-bi	aj+bj	cj+dj
		fadd	st(3),	st				;	eej		F[3]j	F[S+3]j	ej		ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(1)					;	F[3]j	eej		F[S+3]j	ej		ci-di	ai-bi	aj+bj	cj+dj
		fstp	dword ptr[esi+8*3+4]	;	eej		F[S+3]j	ej		ci-di	ai-bi	aj+bj	cj+dj
		fadd	st,		st				;	2eej	F[S+3]j	ej		ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(1)					;	F[S+3]j	2eej	ej		ci-di	ai-bi	aj+bj	cj+dj
		fstp	dword ptr[esi+edi+8*3+4];	2eej	ej		ci-di	ai-bi	aj+bj	cj+dj
//			aa = f1[S+1];	bb = f1[S+0];	ee = f1[S+3];	ff = f1[S+2];
//			cc = f1[1];		dd = f1[0];		gg = f1[3];		hh = f1[2];
//			a=aa+cc;		b = bb+dd;		e = ee+gg;		f = ff+hh;
//			c=aa-cc;		d = bb-dd;		g = ee-gg;		h = ff-hh;
//			f1[1].i = a.i-b.i - (e.j-f.j);
//			f1[3].i = a.i-b.i + (e.j-f.j);
		fld		dword ptr[esi+8*2+4]	;//	st0		st1		st2		st3		st4		st5		st6		st7
		fld		dword ptr[esi+edi+8*2+4];	ffj		hhj		2eej	ej		ci-di	ai-bi	aj+bj	cj+dj
		fadd	st(1),	st				;	ffj		fj		2eej	ej		ci-di	ai-bi	aj+bj	cj+dj
		fadd	st,		st				;	2ffj	fj		2eej	ej		ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(2)					;	2eej	fj		2ffj	ej		ci-di	ai-bi	aj+bj	cj+dj
		fsub	st,		st(3)			;	gj		fj		2ffj	ej		ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(1)					;	fj		gj		2ffj	ej		ci-di	ai-bi	aj+bj	cj+dj
		fsub	st(2),	st				;	fj		gj		hj		ej		ci-di	ai-bi	aj+bj	cj+dj
		fsub	st(3),	st				;
		fadd	st,		st				;	2fj		gj		hj		ej-fj	ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(2)					;	hj		gj		2fj		ej-fj	ci-di	ai-bi	aj+bj	cj+dj
		fsub	st(1),	st				;
		fadd	st,		st				;	2hj		gj-hj	2fj		ej-fj	ci-di	ai-bi	aj+bj	cj+dj
		fxch	st(5)					;	ai-bi	gj-hj	2fj		ej-fj	ci-di	2hj		aj+bj	cj+dj
		fadd	st,		st(3)			;	ab+ef	gj-hj	2fj		ej-fj	ci-di	2hj		aj+bj	cj+dj
		fxch	st(3)					;	ej-fj	gj-hj	2fj		ab+ef	ci-di	2hj		aj+bj	cj+dj
		fadd	st(2),	st				;
		fadd	st,		st				;	2ef		gj-hj	ej+fj	ab+ef	ci-di	2hj		aj+bj	cj+dj
		fxch	st(1)					;	gj-hj	2ef		ej+fj	ab+ef	ci-di	2hj		aj+bj	cj+dj
		fadd	st(5),	st				;	gj-hj	2ef		ej+fj	ab+ef	ci-di	gj+hj	aj+bj	cj+dj
		fsub	st(4),	st				;
		fadd	st,		st				;	2gh		2ef		ej+fj	ab+ef	cd-gh	gj+hj	aj+bj	cj+dj
		fxch	st(3)					;	ab+ef	2ef		ej+fj	2gh		cd-gh	gj+hj	aj+bj	cj+dj
		fsubr	st(1),	st				;	F[3]i	F[1]i	ej+fj	2gh		cd-gh	gj+hj	aj+bj	cj+dj
		fstp	dword ptr[esi+8*3]		;
		fstp	dword ptr[esi+8]		;	ej+fj	2gh		cd-gh	gj+hj	aj+bj	cj+dj
//			f1[0].j  = a.j+b.j + (e.j+f.j);
//			f1[2].j  = a.j+b.j - (e.j+f.j);
//			f1[S].j  = c.j+d.j + (g.j+h.j);
//			f1[S+2].j= c.j+d.j - (g.j+h.j);
//			f1[S+1].i= c.i-d.i - (g.j-h.j);
//			f1[S+3].i= c.i-d.i + (g.j-h.j);
		fadd	st(4),	st				;//	st0		st1		st2		st3		st4		st5		st6		st7
		fadd	st,		st				;	2EF		2gh		cd-gh	gj+hj	AB+EF	cj+dj
		fxch	st(2)					;	cd-gh	2gh		2EF		gj+hj	AB+EF	cj+dj
		fadd	st(1),	st				;	F[S+1]i	F[S+3]i	2EF		gj+hj	AB+EF	cj+dj
		fstp	dword ptr[esi+edi+8]	;
		fstp	dword ptr[esi+edi+8*3]	;	2EF		gj+hj	AB+EF	cj+dj
		fsubr	st,		st(2)			;	F[2]j	gj+hj	F[0]j	cj+dj
		fld		st(3)					;	cj+dj	F[2]j	gj+hj	F[0]j	cj+dj
		fadd	st,		st(2)			;	F[S]j	F[2]j	gj+hj	F[0]j	cj+dj
		fxch	st(3)					;	gj+hj	F[S]j	cj+dj
		fstp	dword ptr[esi+4]		;
		fstp	dword ptr[esi+8*2+4]	;	gj+hj	F[S]j	cj+dj
		fsubp	st(2),	st				;	F[S]j	F[S+2]j
		fstp	dword ptr[esi+edi+4]	;
		fstp	dword ptr[esi+edi+8*2+4];

		add		esi,	8*4				;
		dec		ecx						;
		jnz		f2t1_x					;
		add		esi,	edi				;
		dec		edx						;
		jnz		f2t1_y					;
		} // ASM1	=====================

#endif
//---------------------------------------------------------
#ifdef C_FFT
	DCOMPLEX *su;
	for(s=4; s<S; s*=2)
	 for(n=S/s/2, ct=16*S/s, tx = (1-S)*8, cv=kv=0; kv<s/2; kv++, cv+=2*ct)
	  for(v = cst[cv/16], jv=0; jv<S; jv+=s) {
		f1 = Fp+(jv+kv)*S;	f2 = f1 + s*S/2;
		for(su=cst, ku=s; ku>0; ku--, f1 += tx/8, f2 += tx/8, su+=ct/16)
		  for(ju=n; ju>0; ju--){
			a = f1[0];	b = f1[s] * su[0];	c = f2[0] * v;	d = f2[s] * su[cv/16];
			f1[0] = a + b + c + d;
			f2[0] = a + b - c - d;
			f1[s] = a - b + c - d;
			f2[s] = a - b - c + d;
			f1 += 2*s;	f2+=2*s;
		}
	}
#else
	for(s=4; s<S; s*=2)
	 for(n=S/s/2, ct=16*S/s, tx = (1-S)*8, cv=kv=0; kv<s/2; kv++, cv+=2*ct)
	  for(v = cst[cv/16], jv=0; jv<S; jv+=s) {
		f1 = Fp+(jv+kv)*S;	f2 = f1 + s*S/2;
_asm{	//===============================
		push	ebp						;
		mov		esi,	[f1]			;
		mov		edi,	[f2]			;
		mov		ebx,	[s]				;
		shl		ebx,	3				;
		mov		ecx,	[cst]			;
		mov		edx,	[cv]			;
		mov		ebp,	[s]				;
f2t1_k:	//=============================================================================================
		mov		eax,	[n]				;
f2t1_j:									;	st0		st1		st2		st3		st4		st5		st6		st7
		fld		dword ptr[esi+ebx]		;	br
		fmul	qword ptr[ecx]			;	br*cu
		fld		dword ptr[esi+ebx+4]	;	bi		br*cu
		fmul	qword ptr[ecx+8]		;	bi*su	br*cu
		fld		dword ptr[esi+ebx]		;	br		bi*su	br*cu
		fmul	qword ptr[ecx+8]		;	br*su	bi*su	br*cu
		fld		dword ptr[esi+ebx+4]	;	bi		br*su	bi*su	br*cu
		fmul	qword ptr[ecx]			;	bi*cu	br*su	bi*su	br*cu
		fxch	st(3)					;	br*cu	br*su	bi*su	bi*cu
		fsubrp	st(2),	st				;	br*su	Br		bi*cu
		faddp	st(2),	st				;	Br		Bi
;----------------------------------------
		fld		dword ptr[edi]			;	cr		Br		Bi
		fmul	qword ptr[v]			;	cr*cv	Br		Bi
		fld		dword ptr[edi+4]		;	ci		cr*cv	Br		Bi
		fmul	qword ptr[v+8]			;	ci*sv	cr*cv	Br		Bi
		fld		dword ptr[edi]			;	cr		ci*sv	cr*cv	Br		Bi
		fmul	qword ptr[v+8]			;	cr*sv	ci*sv	cr*cv	Br		Bi
		fld		dword ptr[edi+4]		;	ci		cr*sv	ci*sv	cr*cv	Br		Bi
		fmul	qword ptr[v]			;	ci*cv	cr*sv	ci*sv	cr*cv	Br		Bi
		fxch	st(3)					;	cr*cv	cr*sv	ci*sv	ci*cv	Br		Bi
		fsubrp	st(2),	st				;	cr*sv	Cr		ci*cv	Br		Bi
		faddp	st(2),	st				;	Cr		Ci		Br		Bi
;----------------------------------------
		fld		dword ptr[edi+ebx]		;
		fmul	qword ptr[ecx+edx]		;
		fld		dword ptr[edi+ebx+4]	;
		fmul	qword ptr[ecx+edx+8]	;
		fld		dword ptr[edi+ebx]		;
		fmul	qword ptr[ecx+edx+8]	;
		fld		dword ptr[edi+ebx+4]	;
		fmul	qword ptr[ecx+edx]		;
		fxch	st(3)					;
		fsubrp	st(2),	st				;
		faddp	st(2),	st				;	Dr		Di		Cr		Ci		Br		Bi
;---------------------------------------;	st0		st1		st2		st3		st4		st5		st6		st7
		fld		st(2)					;	Cr		Dr		Di		Cr		Ci		Br		Bi
		fadd	st,		st(1)			;	Cr+Dr	Dr		Di		Cr		Ci		Br		Bi
		fxch	st(3)					;	Cr		Dr		Di		Cr+Dr	Ci		Br		Bi
		fsubrp	st(1),	st				;	Cr-Dr	Di		Cr+Dr	Ci		Br		Bi
		fld		st(3)					;	Ci		Cr-Dr	Di		Cr+Dr	Ci		Br		Bi
		fsub	st,		st(2)			;	Ci-Di	Cr-Dr	Di		Cr+Dr	Ci		Br		Bi
		fxch	st(4)					;	Ci		Cr-Dr	Di		Cr+Dr	Ci-Di	Br		Bi
		faddp	st(2),	st				;	Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	Br		Bi
;----------------------------------------
		fld		dword ptr[esi]			;	Ar		Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	Br		Bi
		fsub	st,		st(5)			;	Ar-Br	Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	Br		Bi
		fxch	st(5)					;	Br		Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	Ar-Br	Bi
		fadd	dword ptr[esi]			;	Ar+Br	Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	Ar-Br	Bi
		fld		st(5)					;	Ar-Br	Ar+Br	Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	Ar-Br	Bi
		fadd	st,		st(2)			;	BR		Ar+Br	Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	Ar-Br	Bi
		fxch	st(6)					;	Ar-Br	Ar+Br	Cr-Dr	Ci+Di	Cr+Dr	Ci-Di	BR		Bi
		fsubrp	st(2),	st				;	Ar+Br	DR		Ci+Di	Cr+Dr	Ci-Di	BR		Bi
		fld		st						;	Ar+Br	Ar+Br	DR		Ci+Di	Cr+Dr	Ci-Di	BR		Bi
		fadd	st,		st(4)			;	AR		Ar+Br	DR		Ci+Di	Cr+Dr	Ci-Di	BR		Bi
		fxch	st(4)					;	Cr+Dr	Ar+Br	DR		Ci+Di	AR		Ci-Di	BR		Bi
		fsubp	st(1),	st				;	CR		DR		Ci+Di	AR		Ci-Di	BR		Bi
		fld		dword ptr[esi+4]		;	Ai		CR		DR		Ci+Di	AR		Ci-Di	BR		Bi
		fsub	st,	st(7)				;	Ai-Bi	CR		DR		Ci+Di	AR		Ci-Di	BR		Bi
		fxch	st(7)					;	Bi		CR		DR		Ci+Di	AR		Ci-Di	BR		Ai-Bi
		fadd	dword ptr[esi+4]		;	Ai+Bi	CR		DR		Ci+Di	AR		Ci-Di	BR		Ai-Bi
		fxch	st(6)					;	BR		CR		DR		Ci+Di	AR		Ci-Di	Ai+Bi	Ai-Bi
		fstp	dword ptr[esi+ebx]		;
		fstp	dword ptr[edi]			;
		fstp	dword ptr[edi+ebx]		;	Ci+Di	AR		Ci-Di	Ai+Bi	Ai-Bi
		fld		st						;	Ci+Di	Ci+Di	AR		Ci-Di	Ai+Bi	Ai-Bi
		fadd	st,		st(4)			;	AI		Ci+Di	AR		Ci-Di	Ai+Bi	Ai-Bi
		fxch	st(4)					;	Ai+Bi	Ci+Di	AR		Ci-Di	AI		Ai-Bi
		fsubrp	st(1),	st				;	CI		AR		Ci-Di	AI		Ai-Bi
		fld		st(4)					;	Ai-Bi	CI		AR		Ci-Di	AI		Ai-Bi
		fadd	st,		st(3)			;	BI		CI		AR		Ci-Di	AI		Ai-Bi
		fxch	st(3)					;	Ci-Di	CI		AR		BI		AI		Ai-Bi
		fsubp	st(5),	st				;	CI		AR		BI		AI		DI
		fstp	dword ptr[edi+4]		;
		fstp	dword ptr[esi]			;
		fstp	dword ptr[esi+ebx+4]	;
		fstp	dword ptr[esi+4]		;
		fstp	dword ptr[edi+ebx+4]	;//=======================================================
		lea		esi,	[esi+ebx*2]		; f1 += 2*s
		lea		edi,	[edi+ebx*2]		; f2 += 2*s
		dec		eax						; ju--
		jnz		f2t1_j					;
;--------------------------------------------------
		add		ecx,	[ct]			; su += ct
		add		esi,	[tx]			; f1 += 1-S
		add		edi,	[tx]			; f2 += 1-S
		dec		ebp						; ku--
		jnz		f2t1_k					;
		pop		ebp						;
		}// ASM2	===================================
	}
#endif
//---------------------------------------------------------
#ifdef C_FFT
	s=S/2; n=s*S; f1=Fp;
	for(cv=kv=0; kv<s; kv++, cv+=2){
		v = cst[cv];
		for(ku=S; ku>0; ku--, f1++){
			a = f1[0];	c = f1[n] * v;
			f1[0] = a + c;
			f1[n] = a - c;
		}
	}
#else
_asm {
		mov		esi,	[Fp]			;
		mov		edi,	[S]				;
		mov		edx,	edi				;
		imul	edi,	edi				;
		shl		edi,	2				;
		shr		edx,	1				;
		mov		ebx,	[cst]			;
f2t3_y:									;
		mov		ecx,	[S]				;
		fld		qword ptr[ebx]			;	st0		st1		st2		st3		st4		st5		st6		st7
		fld		qword ptr[ebx+8]		;	sin		cos
f2t3_x:
		fld		dword ptr[esi+edi]		;
		fld		dword ptr[esi+edi+4]	;	cj		ci		sin		cos
		fld		st						;	cj		cj		ci		sin		cos
		fmul	st,		st(3)			;	cj*s	cj		ci		sin		cos
		fld		st(2)					;	ci		cj*s	cj		ci		sin		cos
		fmul	st,		st(5)			;	ci*c	cj*s	cj		ci		sin		cos
		fxch	st(3)					;	ci		cj*s	cj		ci*c	sin		cos
		fmul	st,		st(4)			;	ci*s	cj*s	cj		ci*c	sin		cos
		fxch	st(2)					;	cj		cj*s	ci*s	ci*c	sin		cos
		fmul	st,		st(5)			;	cj*c	cj*s	ci*s	ci*c	sin		cos
		fxch	st(1)					;	cj*s	cj*c	ci*s	ci*c	sin		cos
		fsubp	st(3),	st				;	cj*c	ci*s	ti		sin		cos
		fld		dword ptr[esi]			;	ai		cj*c	ci*s	ti		sin		cos
		fld		st						;	ai		ai		cj*c	ci*s	ti		sin		cos
		fsub	st,		st(4)			;	c'i		ai		cj*c	ci*s	ti		sin		cos
		fxch	st(3)					;	ci*s	ai		cj*c	c'i		ti		sin		cos
		faddp	st(2),	st				;	ai		tj		c'i		ti		sin		cos
		faddp	st(3),	st				;	tj		c'i		a'i		sin		cos
		fld		dword ptr[esi+4]		;	aj		tj		c'i		a'i		sin		cos
		fld		st						;	aj		aj		tj		c'i		a'i		sin		cos
		fsub	st,		st(2)			;	c'j		aj		tj		c'i		a'i		sin		cos
		fxch	st(2)					;	tj		aj		c'j		c'i		a'i		sin		cos
		faddp	st(1),	st				;	a'j		c'j		c'i		a'i		sin		cos
		fxch	st(3)					;	a'i		c'j		c'i		a'j		sin		cos
		fstp	dword ptr[esi]			;
		fstp	dword ptr[esi+edi+4]	;
		fstp	dword ptr[esi+edi]		;
		fstp	dword ptr[esi+4]		;
		add		esi,	2*4				;
		dec		ecx						;
		jnz		f2t3_x					;
		fstp	st						;
		fstp	st						;
		add		ebx,	2*2*8			;
		dec		edx						;
		jnz		f2t3_y
		} // ASM3	=====================
#endif
}


//####################################################################################
//####################################################################################
//####################################################################################
//####################################################################################
BOOL	FT2R::IFT(void){
int ocw, n = S/2;
FCOMPLEX * Fp = (FCOMPLEX*)BX.P;
	if( (S <8) || (cst==NULL) || (Fp==NULL) ) return FALSE;
	ocw = set_FPUCW(0x37F);
	encod(Fp,		cst, S);
	fft2F(Fp,		cst, n);
	fft2F(Fp+n*n,	cst, n);
	brev2(Fp,		S,	 n);
	set_FPUCW(ocw);
	return	TRUE;
}
//-------------------------------------------------------------------
BOOL	FT2R::FFT(void){
int ocw, n = S/2;
FCOMPLEX * Fp = (FCOMPLEX*)BX.P;
	if( (S <8) || (cst==NULL) || (Fp==NULL) ) return FALSE;
	ocw = set_FPUCW(0x37F);
	brev2(Fp,		S,	 n);
	fft2T(Fp,		cst, n);
	fft2T(Fp+n*n,	cst, n);
	decod(Fp,		cst, S);
	set_FPUCW(ocw);
	return	TRUE;
}
//####################################################################################
//####################################################################################
//####################################################################################
//MACRO getampf
/*
#define getampf(src) }				_asm\
		fld		dword ptr[src]		_asm\
		fmul	st,		st			_asm\
		fld		dword ptr[src+4]	_asm\
		fmul	st,		st			_asm\
		faddp	st(1),	st			_asm\
		fsqrt						_asm{
*/
#define getampf(src)}\
_asm		fld		dword ptr[src]	\
_asm		fmul	st,		st		\
_asm		fld		dword ptr[src+4]\
_asm		fmul	st,		st		\
_asm		faddp	st(1),	st		\
_asm		fsqrt					\
_asm{


//------------------------------------------------------
//MACRO xamp
#define xamp(Z)}\
_asm		fmul	st,		st(1)	\
_asm		fadd	[dc]			\
_asm		fcom	[flt255]		\
_asm		fstsw	ax				\
_asm		sahf					\
_asm		jbe		aa##Z			\
_asm		fstp	st				\
_asm		mov		eax,	255		\
_asm		jmp		cc##Z			\
_asm	aa##Z:						\
_asm		ftst					\
_asm		fstsw	ax				\
_asm		sahf					\
_asm		ja		bb##Z			\
_asm		fstp	st				\
_asm		xor		eax,	eax		\
_asm		jmp		cc##Z			\
_asm	bb##Z:						\
_asm		fistp	[tmp]			\
_asm		mov		eax,	[tmp]	\
_asm	cc##Z:\
_asm{
//-------------------------------------------------------
//------------------------------------------------------
void	FT2R::GetFAmp1Str8(LPVOID fft_adr, LPBYTE img_adr,int dim,int y,float dc,float kr0,float kr1,float kr2){

int		tmp, cnt;
float	flt255 = 255.f;

_asm{
		mov		ebx,	[y]			;
		mov		ecx,	[dim]		;
		mov		edx,	ecx			;
		sub		edx,	ebx			;
		sub		edx,	ebx			; edx = fs - 2*y
		imul	edx,	ecx			;
		sal		edx,	2			; edx = fofs = (fs-2*y)*fs*4

		imul	ebx,	ecx			; ebx = iofs = y*fs
		mov		esi, 	[fft_adr]	;
		lea		esi,	[esi+4*ebx]	; esi = fptr = y*fs*4
		push	esi					;
//----------------------------------------
		mov		edi,	[img_adr]	;
		mov		eax,	ecx			;
		sar		ecx,	1			;
		add		edi,	ecx			; edi = iptr = fs/2

		xor		ebx,	ebx			;
		test	[y],	-1			;
		jz		ga2					; y == 0;
		or		edx,	edx			;
		jz		ga2					; y == xs/2
		mov		ebx,	eax			;
		add		edi,	eax			; edi = iptr = fs + fs/2
ga2:								;
//----------------------------------------
		fld1						;
		fild	[y]					;
		fmul	st,		st			;
		fld		[kr0]				;	kr0	 y2		1.
		fldz						;	x		 kr0	y2		1.
		fsub	st,		st(3)		;
		fld		st					;
		mov		[cnt],	ecx			;
		mov		ecx,	ebx			;
ga10:								;
		fstp	st					;
		fadd	st,		st(3)		;	x++		kr0		y2		1.
		fld		st					;	x		x		kr0		y2		1.
		fmul	st,		st			;	x2		x		kr0		y2		1.
		fadd	st,		st(3)		;	r2		x		kr0		y2		1.
		fld		st					;	r2		r2		x		kr0		y2		1.
		fsqrt						;	r		r2		x		kr0		y2		1.
		fmul	[kr1]				;	f1		r2		x		kr0		y2		1.
		fxch	st(1)				;	r2		f1		x		kr0		y2		1.
		fmul	[kr2]				;	f2		f1		x		kr0		y2		1.
		faddp	st(1),	st			;	f12		x		kr0		y2		1.
		fadd	st,		st(2)		;	f012	x		kr0		y2		1.
//----------------------------------------
		or		ebx,	ebx			;
		jnz		ga11				;
		fld		dword ptr[esi]		; fft[0][0].re == F[0][0]
		fabs						;
		jmp		ga12				;
ga11:
		getampf	(esi)
ga12:								;
		xamp	(1)
		mov		[edi+ebx],al		; img[k][h]
		neg		ebx					;
		mov		[edi+ebx],al		; img[-k][-h]
		neg		ebx					;
//----------------------------------------
		or		ecx,	ecx			;
		jz		ga14				; if(k==0) goto @@14
		cmp		ebx,	ecx			; if(h==0) goto @@13
		je		ga13				;
		getampf	(esi+edx)			; fft[-k][h]
		xamp	(2)					;
		mov		[edi+ecx],al		; img[-k][h]
		neg		ecx					;
		mov		[edi+ecx],al		; img[k][-h]
		neg		ecx					;
ga13:								;
		dec		ecx					;
ga14:								;
//----------------------------------------
		inc		ebx					;
		add		esi,	8			;
		dec		[cnt]				;
		jnz		ga10				;
//----------------------------------------
		pop		esi					; fft[k][0]
		or		ecx,	ecx			; k == 0?
		jnz		ga24				;
		fld		dword ptr[esi+4]	; fft[0][0].im = F[0][-H]
		fabs						;
		xamp	(3)					;
		jmp		ga25				;
ga24:
		getampf (esi+edx)			;
		xamp	(4)					;
		mov		[edi+ecx],al		; img[ k][-H]
ga25:								;
		neg		ebx					;
		mov		[edi+ebx],al		; img[-k][-H]
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
	} // ASM
}
#undef	getampf
#undef	xamp
//------------------------------------------------------
float	FT2R::GetF_MaxVal(void * fft_adr, int xs){
float amax;
_asm{
		mov		esi, 	[fft_adr]		;
		mov		ecx,	[xs]			;
		imul	ecx,	ecx				;
		dec		ecx						;
		add		esi,	4				; skip DC
		xor		ebx,	ebx				;
		mov		edx,	7FFFFFFFh		;
t5:										;
		mov		eax,	dword ptr[esi]	;
		add		esi,	4				;
		and		eax,	edx				;
		cmp		eax,	ebx				;
		jle		t6						;
		mov		ebx,	eax				;
t6:										;
		dec		ecx						;
		jnz		t5						;
		mov		[amax],	ebx				;
	}//ASM
	return	amax;
}
//------------------------------------------------------
BOOL	FT2R::DisplayFFT(BM8& BM, float amax)
{
	int		xs = S, fontHeight, y;
	float	xs2=xs*.5, dc, kr0, kr1, kr2;
	LPBYTE	lps1, lps2, lps3;

	if(BX.P==NULL) return FALSE;
	if( (BM.P==NULL) || (BM.W != xs) || (BM.H != xs) )	BM.ReAlloc(xs,xs,0,0);

	if(BM.P==NULL) return FALSE;

	if( (lps1 = (LPBYTE) GlobalAlloc(GPTR, 3*xs)) == NULL) return FALSE;
	lps2 = lps1+xs, lps3 = lps2+xs;

	if (amax!=0)		MaxVal = amax;
	else if (fDoMaxVal) MaxVal = GetF_MaxVal(BX.P, xs);

	dc  = (float)(iDC);
	kr0 = 256. / MaxVal * pow(10., (double)(iScale)/50.);
	kr1 = kr0*(float)(iLinear)/5./xs2;
	kr2 = kr0*(float)(iParab)/5./xs2/xs2;

	GetFAmp1Str8(BX.P, lps1, xs, 0, dc, kr0, kr1, kr2);
	memcpy(BM.P + (xs/2)*BM.W, lps1, xs);

	for(fontHeight=1; fontHeight<xs/2; fontHeight++)
	{
		GetFAmp1Str8(BX.P, lps1, xs, fontHeight, dc, kr0, kr1, kr2);
		y = xs/2 - fontHeight;
		memcpy(BM.P + (xs/2 - fontHeight)*BM.W, lps1, xs);
		memcpy(BM.P + (xs/2 + fontHeight)*BM.W, lps3, xs);
	}

	GetFAmp1Str8(BX.P, lps3, xs, xs/2, dc, kr0, kr1, kr2);
	memcpy(BM.P, lps3, xs);

	GlobalFree(lps1);
	return TRUE;
}

//####################################################################################
#define	PSz	4
#define getampf(src)}\
_asm		fld		dword ptr[src]	\
_asm		fmul	st,		st		\
_asm		fld		dword ptr[src+4]\
_asm		fmul	st,		st		\
_asm		faddp	st(1),	st		\
_asm		fsqrt					\
_asm{


//------------------------------------------------------
//MACRO xampf
#define xampf(Z)}\
_asm		fmul	st,		st(1)	\
_asm		fadd	[dc]			\
_asm		fstp	[tmp]			\
_asm		mov		eax,	[tmp]	\
_asm		or		eax,	eax		\
_asm		jns		aa##Z			\
_asm		xor		eax,	eax		\
_asm	aa##Z:\
_asm{
//-------------------------------------------------------
//------------------------------------------------------
void	FT2R::GetFAmp1StrF(LPVOID fft_adr, float* img_adr,int dim,int y,float dc,float kr0,float kr1,float kr2){
int		tmp, cnt;
//float	flt255 = 255.f;

_asm{
		mov		ebx,	[y]			;
		mov		ecx,	[dim]		;
		mov		edx,	ecx			;
		sub		edx,	ebx			;
		sub		edx,	ebx			; edx = fs - 2*y
		imul	edx,	ecx			;
		sal		edx,	2			; edx = fofs = (fs-2*y)*fs*4

		imul	ebx,	ecx			; ebx = iofs = y*fs
		mov		esi, 	[fft_adr]	;
		lea		esi,	[esi+4*ebx]	; esi = fptr = y*fs*4
		push	esi					;
//----------------------------------------
		mov		edi,	[img_adr]	;
		mov		eax,	ecx			;
		sar		ecx,	1			;
		lea		edi,	[edi+ecx*PSz]; edi = iptr = fs/2

		xor		ebx,	ebx			;
		test	[y],	-1			;
		jz		ga2					; y == 0;
		or		edx,	edx			;
		jz		ga2					; y == xs/2
		mov		ebx,	eax			;
		lea		edi,	[edi+eax*PSz]; edi = iptr = fs + fs/2
ga2:								;
//----------------------------------------
		fld1						;
		fild	[y]					;
		fmul	st,		st			;
		fld		[kr0]				;	kr0	 y2		1.
		fldz						;	x		 kr0	y2		1.
		fsub	st,		st(3)		;
		fld		st					;
		mov		[cnt],	ecx			;
		mov		ecx,	ebx			;
ga10:								;
		fstp	st					;
		fadd	st,		st(3)		;	x++		kr0		y2		1.
		fld		st					;	x		x		kr0		y2		1.
		fmul	st,		st			;	x2		x		kr0		y2		1.
		fadd	st,		st(3)		;	r2		x		kr0		y2		1.
		fld		st					;	r2		r2		x		kr0		y2		1.
		fsqrt						;	r		r2		x		kr0		y2		1.
		fmul	[kr1]				;	f1		r2		x		kr0		y2		1.
		fxch	st(1)				;	r2		f1		x		kr0		y2		1.
		fmul	[kr2]				;	f2		f1		x		kr0		y2		1.
		faddp	st(1),	st			;	f12		x		kr0		y2		1.
		fadd	st,		st(2)		;	f012	x		kr0		y2		1.
//----------------------------------------
		or		ebx,	ebx			;
		jnz		ga11				;
		fld		dword ptr[esi]		; fft[0][0].re == F[0][0]
		fabs						;
		jmp		ga12				;
ga11:
		getampf	(esi)
ga12:								;
		xampf	(1)
		mov		[edi+ebx*PSz],eax	; img[k][h]
		neg		ebx					;
		mov		[edi+ebx*PSz],eax	; img[-k][-h]
		neg		ebx					;
//----------------------------------------
		or		ecx,	ecx			;
		jz		ga14				; if(k==0) goto @@14
		cmp		ebx,	ecx			; if(h==0) goto @@13
		je		ga13				;
		getampf	(esi+edx)			; fft[-k][h]
		xampf	(2)					;
		mov		[edi+ecx*PSz],eax	; img[-k][h]
		neg		ecx					;
		mov		[edi+ecx*PSz],eax	; img[k][-h]
		neg		ecx					;
ga13:								;
		dec		ecx					;
ga14:								;
//----------------------------------------
		inc		ebx					;
		add		esi,	8			;
		dec		[cnt]				;
		jnz		ga10				;
//----------------------------------------
		pop		esi					; fft[k][0]
		or		ecx,	ecx			; k == 0?
		jnz		ga24				;
		fld		dword ptr[esi+4]	; fft[0][0].im = F[0][-H]
		fabs						;
		xampf	(3)					;
		jmp		ga25				;
ga24:
		getampf (esi+edx)			;
		xampf	(4)					;
		mov		[edi+ecx*PSz],eax	; img[ k][-H]
ga25:								;
		neg		ebx					;
		mov		[edi+ebx*PSz],eax	; img[-k][-H]
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
	} // ASM
}
#undef	getampf
#undef	xampf
//------------------------------------------------------
//------------------------------------------------------
//------------------------------------------------------
BOOL	FT2R::DisplayFFT(BMF& BM, float amax){
int		xs = S, fontHeight, y;
float	xs2=xs/2., dc, kr0, kr1, kr2;
float	*lps1, *lps2, *lps3;

	if(BX.P==NULL) return FALSE;
	if( (BM.P==NULL) || (BM.W != xs) || (BM.H != xs) )	BM.ReAlloc(xs,xs,0,0);

	if(BM.P==NULL) return FALSE;

	if( (lps1 = (float*)GlobalAlloc(GPTR, 3*xs*PSz)) == NULL) return FALSE;
	lps2 = lps1+xs, lps3 = lps2+xs;

	if(amax!=0)			MaxVal = amax;
	else if (fDoMaxVal)	MaxVal = GetF_MaxVal(BX.P, xs);

	dc  = (float)(iDC);
	kr0 = 256. / MaxVal * pow(10., (double)(iScale)/50.);
	kr1 = kr0*(float)(iLinear)/5./xs2;
	kr2 = kr0*(float)(iParab)/5./xs2/xs2;

	GetFAmp1StrF(BX.P, lps1, xs, 0, dc, kr0, kr1, kr2);
	memcpy(BM.P + (xs/2)*BM.W, lps1, xs*PSz);

	for(fontHeight=1; fontHeight<xs/2; fontHeight++) {
		GetFAmp1StrF(BX.P, lps1, xs, fontHeight, dc, kr0, kr1, kr2);
		y = xs/2 - fontHeight;
		memcpy(BM.P + (xs/2 - fontHeight)*BM.W, lps1, xs*PSz);
		memcpy(BM.P + (xs/2 + fontHeight)*BM.W, lps3, xs*PSz);
	}

	GetFAmp1StrF(BX.P, lps3, xs, xs/2, dc, kr0, kr1, kr2);
	memcpy(BM.P, lps3, xs*PSz);

	GlobalFree(lps1);
	return TRUE;
}
#undef	PSz

//####################################################################################




// Commented by Peter on 30 April 2008
/*


#define ffcvt }\
_asm	cdq					\
_asm	shr		edx,	1	\
_asm	xor		eax,	edx	\
_asm{
//-------------------------------
void	FT2R::GetFloatMinMax(LPVOID src, int cnt, float& _min_, float& _max_){
_asm{
		mov		eax,	[_max_]	;
		mov		eax,	[eax]	;
		ffcvt					;
		mov		ebx,	eax		;
		mov		eax,	[_max_]	;
		mov		eax,	[eax]	;
		ffcvt					;
		mov		edi,	eax		; edi=_min_, ebx=_max_
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
		mov		esi,	[_min_]	;
		mov		[esi],	eax		;
		mov		eax,	ebx		;
		ffcvt					;
		mov		esi,	[_max_]	;
		mov		[esi],	eax		;
	}//ASM
}
#undef	ffcvt
*/

// End of comment by Peter on 30 April 2008

//------------------------------------------------
void	FT2R::GetF_Img8(LPVOID src, LPVOID dst, int dim, double _min_, double gain)
{
	double	flt05 = 0.5f, flt255 = 255.0;
	float _gain = 0.0f;
	_asm {
		mov		esi, 	[src]		;
		mov		edi, 	[dst]		;
		mov		ecx,	[dim]		;
		fld		[flt05]				;
		fld		[flt255]			;
		fld		[_min_]				;
		fld		[gain]				;
t10:
		fld		dword ptr[esi]		; D   sc   _min_  255  0.5
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
//		fistp	[gain]				;
//		mov		eax,	[gain]		;
		fistp	[_gain]				;
		mov		eax,	[_gain]		;
		mov		[edi],	al	;
t20:								;
		inc		edi					;
		dec		ecx					;
		jnz		t10					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
	} // ASM
}

//------------------------------------------------
void	FT2R::GetF_Img16(LPVOID src, LPVOID dst, int dim, double _min_, double gain)
{
	double	flt05 = 0.5, flt255 = 65535.;
	float _gain = 0.0f;
	_asm{
		mov		esi, 	[src]		;
		mov		edi, 	[dst]		;
		mov		ecx,	[dim]		;
		fld		[flt05]				;
		fld		[flt255]			;
		fld		[_min_]				;
		fld		[gain]				;
t10:
		fld		dword ptr[esi]		; D   sc   _min_  65535  0.5
		fsub	st,		st(2)		;
		add		esi,	4			;
		fmul	st,		st(1)		;
//----------------------------------------
		fcom	st(4)				;
		fstsw	ax					;
		sahf						;
		ja		t13					;
		fstp	st					;
		mov		word ptr [edi],	0	;
		jmp		t20					;
t13:								;
		fcom	st(3)				;
		fstsw	ax					;
		sahf						;
		jbe		t14					;
		fstp	st					;
		mov		word ptr [edi], 65535 ;
		jmp		t20					;
t14:								;
//----------------------------------------
//		fistp	[gain]				;
//		mov		eax,	[gain]		;
		fistp	[_gain]				;
		mov		eax,	[_gain]		;
		mov		[edi],	ax			;
t20:								;
		add		edi,	2			;
		dec		ecx					;
		jnz		t10					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
	} // ASM
}
//------------------------------------------------
void	FT2R::GetF_Img32(LPVOID src, LPVOID dst, int dim, double _min_, double gain)
{
	double	flt05 = 0.5, flt255 = (double)(DWORD)0xFFFFFFFF;
	float _gain = 0.0f;
//	float _gain = 0.0f;
	_asm {
		mov		esi, 	[src]		;
		mov		edi, 	[dst]		;
		mov		ecx,	[dim]		;
		fld		[flt05]				;
		fld		[flt255]			;
		fld		[_min_]				;
		fld		[gain]				;
t10:
		fld		dword ptr[esi]		; D   sc   _min_  65535  0.5
		fsub	st,		st(2)		;
		add		esi,	4			;
		fmul	st,		st(1)		;
//----------------------------------------
		fcom	st(4)				;
		fstsw	ax					;
		sahf						;
		ja		t13					;
		fstp	st					;
		mov		dword ptr [edi], 0	;
		jmp		t20					;
t13:								;
		fcom	st(3)				;
		fstsw	ax					;
		sahf						;
		jbe		t14					;
		fstp	st					;
		mov		dword ptr [edi], 0xFFFFFFFF ;
		jmp		t20					;
t14:								;
//----------------------------------------
//		fistp	[gain]				;
//		mov		eax,	[gain]		;
		fistp	qword ptr [_gain]	;
		mov		eax, dword ptr [_gain]		;
		mov		[edi],	eax			;
t20:								;
		add		edi,	4			;
		dec		ecx					;
		jnz		t10					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
		fstp	st					;
	} // ASM
}

//=========================================================================
BOOL	FT2R::DisplayIFT(BM8& BM, float gain)
{
	double	_min_=0., _max_=1.;
	int		y, xs = S;

	if(BX.P==NULL) return FALSE;
	if( (BM.P==NULL) || (BM.W!=xs) || (BM.H!=xs) ) BM.ReAlloc(xs,xs,0,0);
	if( BM.P==NULL)	return FALSE;

	if (iScaling)
	{
		_min_ = _max_ = BX[0];
// 		GetFloatMinMaxD(BX.P, xs*xs, _min_, _max_);
		GetFloatMinMax_t<float>(BX.P, xs*xs, _min_, _max_);
		if ((_max_-_min_) < 1e-7) _max_ = _min_+1.;
		_max_ = 255./(_max_ - _min_);
	}
	if(gain!=0)	_max_ *= gain;
	for(y=0; y<xs; y++)	GetF_Img8 (BX.P+y*xs, BM.P+y*BM.W, xs, _min_, _max_);
	return TRUE;
}

BOOL	FT2R::DisplayIFT(BM8S& BM, float gain)
{
	double	_min_=0., _max_=1.;
	int		y, xs = S;

	if(BX.P==NULL) return FALSE;
	if( (BM.P==NULL) || (BM.W!=xs) || (BM.H!=xs) ) BM.ReAlloc(xs,xs,0,0);
	if( BM.P==NULL)	return FALSE;

	if (iScaling)
	{
		_min_ = _max_ = BX[0];
// 		GetFloatMinMaxD(BX.P, xs*xs, _min_, _max_);
		GetFloatMinMax_t<float>(BX.P, xs*xs, _min_, _max_);
		if ((_max_-_min_) < 1e-7) _max_ = _min_+1.;
		_max_ = 255./(_max_ - _min_);
	}
	if(gain!=0)	_max_ *= gain;
	for(y=0; y<xs; y++)	GetF_Img8 (BX.P+y*xs, BM.P+y*BM.W, xs, _min_, _max_);
	return TRUE;
}

BOOL	FT2R::DisplayIFT(BM16& BM, float gain)
{
	double	_min_=0., _max_=1.;
	int		y, xs = S;

	if( BX.P==NULL) return FALSE;
	if((BM.P==NULL) || (BM.W!=xs) || (BM.H!=xs) ) BM.ReAlloc(xs,xs,0,0);
	if( BM.P==NULL)	return FALSE;

	if (iScaling)
	{
		_min_ = _max_ = BX[0];
//		GetFloatMinMaxD(BX.P, xs*xs, _min_, _max_);
		GetFloatMinMax_t<float>(BX.P, xs*xs, _min_, _max_);
		if ((_max_-_min_) < 1e-7) _max_ = _min_+1.;
		_max_ = 65535./(_max_-_min_);
	}
	if(gain!=0)	_max_ *= gain;
	for(y=0; y<xs; y++)
	{
		GetF_Img16(BX.P+y*xs, BM.P+y*BM.W, xs, _min_, _max_);
	}
	return TRUE;
}

BOOL	FT2R::DisplayIFT(BM16S& BM, float gain)
{
	double	_min_=0., _max_=1.;
	int		y, xs = S;

	if( BX.P==NULL) return FALSE;
	if((BM.P==NULL) || (BM.W!=xs) || (BM.H!=xs) ) BM.ReAlloc(xs,xs,0,0);
	if( BM.P==NULL)	return FALSE;

	if (iScaling)
	{
		_min_ = _max_ = BX[0];
//		GetFloatMinMaxD(BX.P, xs*xs, _min_, _max_);
		GetFloatMinMax_t<float>(BX.P, xs*xs, _min_, _max_);
		if ((_max_-_min_) < 1e-7) _max_ = _min_+1.;
		_max_ = 65535./(_max_-_min_);
	}
	if(gain!=0)	_max_ *= gain;
	for(y=0; y<xs; y++)
	{
		GetF_Img16(BX.P+y*xs, BM.P+y*BM.W, xs, _min_, _max_);
	}
	return TRUE;
}

//=========================================================================
BOOL	FT2R::DisplayIFT(BM32& BM, float gain)
{
	double	_min_ = 0.0, _max_ = 1.0;
	int		y, xs = S;

	if( BX.P==NULL) return FALSE;
	if((BM.P==NULL) || (BM.W!=xs) || (BM.H!=xs) ) BM.ReAlloc(xs,xs,0,0);
	if( BM.P==NULL)	return FALSE;

	if (iScaling)
	{
		_min_ = _max_ = BX[0];
//		GetFloatMinMaxD(BX.P, xs*xs, _min_, _max_);
		GetFloatMinMax_t<float>(BX.P, xs*xs, _min_, _max_);
		if ((_max_ - _min_) < 1e-7) _max_ = _min_ + 1.;
		_max_ = ((double)(DWORD)0xFFFFFFFF)/(_max_ - _min_);
	}
	if(gain!=0)	_max_ *= gain;
	for(y=0; y<xs; y++)
	{
		GetF_Img32(BX.P+y*xs, BM.P+y*BM.W, xs, _min_, _max_);
	}
	return TRUE;
}

BOOL	FT2R::DisplayIFT(BM32S& BM, float gain)
{
	double	_min_ = 0.0, _max_ = 1.0;
	int		y, xs = S;

	if( BX.P==NULL) return FALSE;
	if((BM.P==NULL) || (BM.W!=xs) || (BM.H!=xs) ) BM.ReAlloc(xs,xs,0,0);
	if( BM.P==NULL)	return FALSE;

	if (iScaling)
	{
		_min_ = _max_ = BX[0];
//		GetFloatMinMaxD(BX.P, xs*xs, _min_, _max_);
		GetFloatMinMax_t<float>(BX.P, xs*xs, _min_, _max_);
		if ((_max_ - _min_) < 1e-7) _max_ = _min_ + 1.;
		_max_ = ((double)(DWORD)0xFFFFFFFF)/(_max_ - _min_);
	}
	if(gain!=0)	_max_ *= gain;
	for(y=0; y<xs; y++)
	{
		GetF_Img32(BX.P+y*xs, BM.P+y*BM.W, xs, _min_, _max_);
	}
	return TRUE;
}

//####################################################################################
/*
FCOMPLEX	GetVal(const FT2R& FT, int h, int k){
FCOMPLEX*	fptr = (FCOMPLEX*)FT.BX.P;
int	xs = FT.S;
FCOMPLEX val;
#define w 0
_asm{
		mov		esi, 	[fptr]			;
		mov		eax,	[h]				;
		mov		ebx,	[k]				;
		mov		ecx,	[xs]			;
		mov		edx,	ecx				;
		dec		edx						;
		add		eax,	eax				; h *= 2
		test	eax,	edx				;
		jnz		a20						; (h != 0) && (h != H)
;........................................
		sal		ebx,	1				;
		test	ebx,	edx				;
		jz		a2						;
		xor		eax,	ebx				;
		sar		ebx,	1				;
		test	eax,	ecx				;
		mov		eax,	0				;
		jz		a22						; (h==0 && k>0) || (h==H && k<0)
		jmp		a21						;
;........................................
a2:										; h,k in (0, H)
		and		eax,	ecx				;
		jz		a4						;
		add		esi,	4				;
a4:		and		ebx,	ecx				;
//		imul	ebx,	ecx				; 4*(k*xs/2)
//-----------------------------------------
		imul	ebx,	ecx				; 4*(k*xs/2)
#if(w)
		mov		eax,	dword ptr[val]	;
		mov		dword ptr[esi+ebx*2],eax; Re
#else									;
		mov		eax,dword ptr[esi+ebx*2]; Re
		mov		dword ptr[val], eax		;
		mov		dword ptr[val+4], 0		;
#endif									;
//-----------------------------------
		jmp		a99						;
a20:	//--------------------------------
		or		eax,	eax				;
		jns		a22						; h > 0
		neg		eax						; -h
a21:									;
		neg		ebx						; -k
#if(w)
		xor		dword ptr[val+4],	80000000h	; Im = -Im
#else
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h
		mov		eax,	dword ptr[esi+ebx*4]; Re
		mov		dword ptr[val], eax		;
		mov		eax,	dword ptr[esi+ebx*4+4]; Im
		xor		eax,	80000000h		;
		mov		dword ptr[val+4], eax	;
		jmp		a99						;
#endif									;
a22:	;................................
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h
#if(w)									;
		mov		eax,	dword ptr[val+4];
		mov	dword ptr[esi+ebx*4+4],eax	; Im
		mov		eax,	dword ptr[val]	;
		mov	dword ptr[esi+ebx*4],eax	; Re
#else									;
		mov		eax,	dword ptr[esi+ebx*4]	; Re
		mov		dword ptr[val], eax		;
		mov		eax,	dword ptr[esi+ebx*4+4]	; Im
		mov		dword ptr[val+4], eax	;
#endif									;
a99:	;--------------------------------
	}//ASM
#undef w
	return	val;
}
*/
//####################################################################################
//####################################################################################
FCOMPLEX	FT2R::GetVal(int h, int k){
FCOMPLEX*	fptr = (FCOMPLEX*)BX.P;
int	xs = S;
FCOMPLEX val;
_asm{
		mov		esi, 	[fptr]			;
		mov		eax,	[h]				;
		mov		ebx,	[k]				;
		mov		ecx,	[xs]			;
		mov		edx,	ecx				;
		dec		edx						;
		add		eax,	eax				; h *= 2
		test	eax,	edx				;
		jnz		a20						; (h != 0) && (h != H)
;........................................
		sal		ebx,	1				;
		test	ebx,	edx				;
		jz		a2						;
		xor		eax,	ebx				;
		sar		ebx,	1				;
		test	eax,	ecx				;
		mov		eax,	0				;
		jz		a22						; (h==0 && k>0) || (h==H && k<0)
		jmp		a21						;
;........................................
a2:										; h,k in (0, H)
		and		eax,	ecx				;
		jz		a4						;
		add		esi,	4				;
a4:		and		ebx,	ecx				;
//		imul	ebx,	ecx				; 4*(k*xs/2)
//-----------------------------------------
		imul	ebx,	ecx				; 4*(k*xs/2)
		mov		eax,dword ptr[esi+ebx*2]; Re
		mov		dword ptr[val], eax		;
		mov		dword ptr[val+4], 0		;
		jmp		a99						;
a20:	;--------------------------------
		or		eax,	eax				;
		jns		a22						; h > 0
		neg		eax						; -h
a21:									;
		neg		ebx						; -k
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h
		mov		eax,	dword ptr[esi+ebx*4]; Re
		mov		dword ptr[val], eax		;
		mov		eax,	dword ptr[esi+ebx*4+4]; Im
		xor		eax,	80000000h		;
		mov		dword ptr[val+4], eax	;
		jmp		a99						;
a22:	;................................
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h
		mov		eax,	dword ptr[esi+ebx*4]	; Re
		mov		dword ptr[val], eax		;
		mov		eax,	dword ptr[esi+ebx*4+4]	; Im
		mov		dword ptr[val+4], eax	;
a99:	;--------------------------------
	}//ASM
	return	val;
}
//####################################################################################
FCOMPLEX	FT2R::GetVal(ICOMPLEX hk){
FCOMPLEX*	fptr = (FCOMPLEX*)BX.P;
int	xs = S;
FCOMPLEX val;
_asm{
		mov		esi, 	[fptr]			;
		mov		eax,	dword ptr[hk]	;
		mov		ebx,	dword ptr[hk+4]	;
		mov		ecx,	[xs]			;
		mov		edx,	ecx				;
		dec		edx						;
		add		eax,	eax				; h *= 2
		test	eax,	edx				;
		jnz		a20						; (h != 0) && (h != H)
;........................................
		sal		ebx,	1				;
		test	ebx,	edx				;
		jz		a2						;
		xor		eax,	ebx				;
		sar		ebx,	1				;
		test	eax,	ecx				;
		mov		eax,	0				;
		jz		a22						; (h==0 && k>0) || (h==H && k<0)
		jmp		a21						;
;........................................
a2:										; h,k in (0, H)
		and		eax,	ecx				;
		jz		a4						;
		add		esi,	4				;
a4:		and		ebx,	ecx				;
//		imul	ebx,	ecx				; 4*(k*xs/2)
//-----------------------------------------
		imul	ebx,	ecx				; 4*(k*xs/2)
		mov		eax,dword ptr[esi+ebx*2]; Re
		mov		dword ptr[val], eax		;
		mov		dword ptr[val+4], 0		;
		jmp		a99						;
a20:	;--------------------------------
		or		eax,	eax				;
		jns		a22						; h > 0
		neg		eax						; -h
a21:									;
		neg		ebx						; -k
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h
		mov		eax,	dword ptr[esi+ebx*4]; Re
		mov		dword ptr[val], eax		;
		mov		eax,	dword ptr[esi+ebx*4+4]; Im
		xor		eax,	80000000h		;
		mov		dword ptr[val+4], eax	;
		jmp		a99						;
a22:	;................................
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h
		mov		eax,	dword ptr[esi+ebx*4]	; Re
		mov		dword ptr[val], eax		;
		mov		eax,	dword ptr[esi+ebx*4+4]	; Im
		mov		dword ptr[val+4], eax	;
a99:	;--------------------------------
	}//ASM
	return	val;
}
//####################################################################################
//####################################################################################
void	FT2R::PutVal(int h, int k, FCOMPLEX val){
FCOMPLEX*	fptr = (FCOMPLEX*)BX.P;
int	xs = S;
_asm{
		mov		esi, 	[fptr]			;
		mov		eax,	[h]				;
		mov		ebx,	[k]				;
		mov		ecx,	[xs]			;
		mov		edx,	ecx				;
		dec		edx						;
		add		eax,	eax				; h *= 2
		test	eax,	edx				;
		jnz		a20						; (h != 0) && (h != H)
;........................................
		sal		ebx,	1				;
		test	ebx,	edx				;
		jz		a2						;
		xor		eax,	ebx				;
		sar		ebx,	1				;
		test	eax,	ecx				;
		mov		eax,	0				;
		jz		a22						; (h==0 && k>0) || (h==H && k<0)
		jmp		a21						;
;........................................
a2:										; h,k in (0, H)
		and		eax,	ecx				;
		jz		a4						;
		add		esi,	4				;
a4:		and		ebx,	ecx				;
		imul	ebx,	ecx				; 4*(k*xs/2)

		mov		eax,	dword ptr[val]	;
		mov		dword ptr[esi+ebx*2],eax; Re
		jmp		a99						;
a20:	;--------------------------------
		or		eax,	eax				;
		jns		a22						; h > 0
		neg		eax						; -h
a21:									;
		neg		ebx						; -k
		xor		dword ptr[val+4],	80000000h	; Im = -Im
a22:	;................................
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h

		mov		eax,	dword ptr[val+4]			;
		mov	dword ptr[esi+ebx*4+4],eax	; Im
		mov		eax,	dword ptr[val]			;
		mov	dword ptr[esi+ebx*4],eax	; Re
a99:	;--------------------------------
	}//ASM
}
//####################################################################################
void	FT2R::PutAP(int h, int k, float amp, float pha){
FCOMPLEX val;
	_asm {
		fld     [pha]                   ;
		fsincos                         ; cos, sin
		fmul    [amp]                   ; Re,  sin
		fxch    st(1)                   ; sin, Re
		fmul    [amp]                   ; Im,  Re
		fxch	st(1)					; Re,  Im
		fstp	dword ptr [val]			;
		fstp	dword ptr [val+4]		;
	}
	PutVal(h,k,val);
}
//####################################################################################
void	FT2R::PutVal(ICOMPLEX hk, FCOMPLEX val){
FCOMPLEX*	fptr = (FCOMPLEX*)BX.P;
int	xs = S;
_asm{
		mov		esi, 	[fptr]			;
		mov		eax,	dword ptr[hk]	;
		mov		ebx,	dword ptr[hk+4]	;
		mov		ecx,	[xs]			;
		mov		edx,	ecx				;
		dec		edx						;
		add		eax,	eax				; h *= 2
		test	eax,	edx				;
		jnz		a20						; (h != 0) && (h != H)
;........................................
		sal		ebx,	1				;
		test	ebx,	edx				;
		jz		a2						;
		xor		eax,	ebx				;
		sar		ebx,	1				;
		test	eax,	ecx				;
		mov		eax,	0				;
		jz		a22						; (h==0 && k>0) || (h==H && k<0)
		jmp		a21						;
;........................................
a2:										; h,k in (0, H)
		and		eax,	ecx				;
		jz		a4						;
		add		esi,	4				;
a4:		and		ebx,	ecx				;
		imul	ebx,	ecx				; 4*(k*xs/2)

		mov		eax,	dword ptr[val]	;
		mov		dword ptr[esi+ebx*2],eax; Re
		jmp		a99						;
a20:	;--------------------------------
		or		eax,	eax				;
		jns		a22						; h > 0
		neg		eax						; -h
a21:									;
		neg		ebx						; -k
		xor		dword ptr[val+4],	80000000h	; Im = -Im
a22:	;................................
		and		ebx,	edx				;
		imul	ebx,	ecx				; 2*(k*xs/2)
		add		ebx,	eax				; 2*(k*xs/2) + 2*h

		mov		eax,	dword ptr[val+4]			;
		mov	dword ptr[esi+ebx*4+4],eax	; Im
		mov		eax,	dword ptr[val]			;
		mov	dword ptr[esi+ebx*4],eax	; Re
a99:	;--------------------------------
	}//ASM
}
//####################################################################################
#endif	//__MEAT__
//####################################################################################
//####################################################################################
//####################################################################################
//####################################################################################
//#undef	ICOMPLEX
//#undef	FCOMPLEX
//#undef	DCOMPLEX
#undef	F
#undef	D
//#pragma optimize("",on)
#endif

