/****************************************************************************/
/* Copyright (c) 1998 Calidris **********************************************/
/****************************************************************************/
/* CRISP2.ELD ***************************************************************/
/****************************************************************************/

#include "StdAfx.h"

// #include "SpinStar/typedefs.h"
#include "typedefs.h"

#include "objects.h"
#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
#include "rgconfig.h"

// Added by Peter on 23 November 2004
#include "Crash.h"
#include "HelperFn.h"

#pragma warning(disable:4305)

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

#include "eld.h"
#include "xCBN\shcb.h"

#include "ELD_HOLZ.h"

using namespace std;

extern CRISP scbCrisp;
static DWORD s_dwELD_with_SpinnigStar = IDC_ELD_DEFINE_HOLZ;

static	LPCSTR s_szFlags[3] = {" ","Not estimated", "Misplaced"};

typedef		BOOL (__stdcall *pfnSPINSTAR)(LPVOID);
extern		pfnSPINSTAR		g_pfnSPINSTAR;

//-----------------------------------------------------------
// Integration constants
#define	INT_CUTOFF	0.5		// Cutoff level for refl radius detection

static	WORD	idc_REF[3] = {IDC_ELD_REF1,	IDC_ELD_REF2,	IDC_ELD_REF3};

static	WORD	idc_h  [3] = {IDC_ELD_H1,	IDC_ELD_H2,		IDC_ELD_H3};
static	WORD	idc_k  [3] = {IDC_ELD_K1,	IDC_ELD_K2,		IDC_ELD_K3};
static	WORD	idc_uu [2] = {IDC_ELD_UU1,	IDC_ELD_UU2};
static	WORD	idc_vv [2] = {IDC_ELD_VV1,	IDC_ELD_VV2};
static	WORD	idc_hh [2] = {IDC_ELD_HH1,	IDC_ELD_HH2};
static	WORD	idc_kk [2] = {IDC_ELD_KK1,	IDC_ELD_KK2};
static	WORD	idc_ll [2] = {IDC_ELD_LL1,	IDC_ELD_LL2};

static	int		def_uu [2] = {1,0}, def_vv[2] = {0,1};
static	int		def_hh [2] = {1,0}, def_kk[2] = {0,1}, def_ll[2] = {0,0};
static	float	snh,snk,snx,sny,snn,shx,skx,shh,skk,shk,shy,sky;
//static	BYTE	tmp[65536];
static	DWORD	tmp[65536];
static	float 	ftmp[280];
// commented by Peter, 7 Aug 2009, since kf is not used anywhere
//static	float	kf[36];
static	HPEN	PEN[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, };
static	BOOL	bGdiInitialized = FALSE;
//#define	pt		pELD->m_ppt
static	double	ih[13] ={-0.5,+0.5,+0.5,-0.5,-0.5,-1, 1, 0, 0,-0.1,0.1,   0,  0};
static	double	ik[13] ={-0.5,-0.5,+0.5,+0.5,-0.5, 0, 0,-1, 1,   0,  0,-0.1,0.1};
static	double	ihc[13]={ 0.0,+1.0, 0.0,-1.0,-0.5,-1, 1, 0, 0,-0.1,0.1,   0,  0};
static	double	ikc[13]={-1.0, 0.0,+1.0, 0.0,-0.5, 0, 0,-1, 1,   0,  0,-0.1,0.1};

int GetShape16_(LPELD pEld, float k, int x, int y);
int GetShape16_test(LPELD pEld, float k, int x, int y);
int GetShape8_asm(LPELD pEld, float k, int x, int y);
int GetShape16_asm(LPELD pEld, float k, int x, int y);
int GetShape32_asm(LPELD pEld, float k, int x, int y);

static void CalcSmartCoordinates( EREFL* inReflections, int nrefl, EREFL* outReflection );

//////////////////////////////////////////////////////////////////////////

static void ClipData(int x, int y, int rad, int w, int h,
					 int &nStartX, int &nStartY, int &nEndX, int &nEndY)
{
	nStartX = x - rad;
	if( nStartX < 0 ) nStartX = 0;
	if( nStartX >= w ) nStartX = w - 1;
	nEndX = x + rad;
	if( nEndX < 0 ) nEndX = 0;
	if( nEndX >= w ) nEndX = w - 1;
	nStartY = y - rad;
	if( nStartY < 0 ) nStartY = 0;
	if( nStartY >= h ) nStartY = h - 1;
	nEndY = y + rad;
	if( nEndY < 0 ) nEndY = 0;
	if( nEndY >= h ) nEndY = h - 1;
}

template <typename _Tx>
int GetAXY_t(_Tx *pSrc, const IMAGE &pImg, int x, int y, int rad, double thr, double &axs, double &ays, double &as)
{
	int nPixels = 0;
	int nStartX, nStartY, nEndX, nEndY;

	axs = 0;
	ays = 0;
	as = 0;
	// clip the bounding rectangle
	ClipData(x, y, rad, pImg.w, pImg.h, nStartX, nStartY, nEndX, nEndY);
	pSrc = (_Tx*)(((LPBYTE)pSrc) + pImg.stride * nStartY);
	for(int _y = nStartY; _y <= nEndY; _y++)
	{
		for(int _x = nStartX; _x <= nEndX; _x++)
		{
			double val = Negate(pSrc[_x], pImg.neg);
			if( val >= thr )
			{
				axs += _x * val;
				ays += _y * val;
				as += val;
				nPixels++;
			}
		}
		pSrc = (_Tx*)(((LPBYTE)pSrc) + pImg.stride);
	}
	return nPixels;
}

int GetAXYX(const IMAGE &Img, int x, int y, int rad, double thr, double &axs, double &ays, double &as)
{
	int value = 0;
	switch( Img.npix )
	{
	case PIX_BYTE:		value = GetAXY_t<BYTE>((LPBYTE)		Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	case PIX_CHAR:		value = GetAXY_t<CHAR>((LPSTR)		Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	case PIX_WORD:		value = GetAXY_t<WORD>((LPWORD)		Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	case PIX_SHORT:		value = GetAXY_t<SHORT>((SHORT*)	Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	case PIX_DWORD:		value = GetAXY_t<DWORD>((LPDWORD)	Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	case PIX_LONG:		value = GetAXY_t<LONG>((LPLONG)		Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	case PIX_FLOAT:		value = GetAXY_t<FLOAT>((LPFLOAT)	Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	case PIX_DOUBLE:	value = GetAXY_t<DOUBLE>((LPDOUBLE)	Img.pData, Img, x, y, rad, thr, axs, ays, as); break;
	}
	return value;
}

//////////////////////////////////////////////////////////////////////////
// Added by Peter 18 June 2009
template <typename _Tx>
int GetMeanX_t(_Tx *pSrc, IMAGE *pImg, int x, int y, int rad, double &mean, double &m_min, double &m_max)
{
	int nPixels = 0;
	int nStartX, nStartY, nEndX, nEndY;

	ClipData(x, y, rad, pImg->w, pImg->h, nStartX, nStartY, nEndX, nEndY);
	pSrc = (_Tx*)(((LPBYTE)pSrc) + pImg->stride * nStartY);
	mean = 0.0;
	m_min = m_max = pSrc[0];
	for(int _y = nStartY; _y <= nEndY; _y++)
	{
		for(int _x = nStartX; _x <= nEndX; _x++)
		{
			double value = Negate(pSrc[_x], pImg->neg);
			mean += value;
			m_min = __min(m_min, value);
			m_max = __max(m_max, value);
			nPixels++;
		}
		pSrc = (_Tx*)(((LPBYTE)pSrc) + pImg->stride);
	}
	if( nPixels > 0 )
	{
		mean /= nPixels;
	}
	return nPixels;
}

int GetMeanX(IMAGE *pImg, int x, int y, int rad, double &mean, double &m_min, double &m_max)
{
	int value = 0;
	switch( pImg->npix )
	{
	case PIX_BYTE:	value = GetMeanX_t((LPBYTE)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	case PIX_WORD:	value = GetMeanX_t((LPWORD)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	case PIX_DWORD:	value = GetMeanX_t((LPDWORD)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	case PIX_CHAR:	value = GetMeanX_t((CHAR*)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	case PIX_SHORT:	value = GetMeanX_t((SHORT*)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	case PIX_LONG:	value = GetMeanX_t((LONG*)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	case PIX_FLOAT:	value = GetMeanX_t((FLOAT*)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	case PIX_DOUBLE:value = GetMeanX_t((DOUBLE*)pImg->pData, pImg, x, y, rad, mean, m_min, m_max); break;
	default:
		throw std::exception(_T("GetMeanX -> failed to get max/min/mean values: unsupported pixel format"));
		break;
	}
	return value;
}

//////////////////////////////////////////////////////////////////////////

template <typename _Tx>
class FMapToImg_t
{
public:
	typedef void TReturn;
	void Handle(IMAGE& inImage, FLOAT *pSrc, double _Min, double _Max)
	{
		double dScale = _Max - _Min;
		_Tx* pDst = (_Tx*)inImage.pData;
		if( true == numeric_limits<_Tx>::is_integer )
		{
			for(int y = 0; y < inImage.h; y++)
			{
				for(int x = 0; x < inImage.w; x++)
				{
					double value = pSrc[x] * dScale + _Min;
					value = __max(numeric_limits<_Tx>::min(), value);
					value = __min(numeric_limits<_Tx>::max(), value);
					pDst[x] = Negate(value, inImage.neg);
				}
				pDst = (_Tx*)(((LPBYTE)pDst) + inImage.stride);
				pSrc += inImage.w;
			}
		}
		else
		{
			for(int y = 0; y < inImage.h; y++)
			{
				for(int x = 0; x < inImage.w; x++)
				{
					double value = pSrc[x] * dScale + _Min;
					pDst[x] = Negate(value, inImage.neg);
				}
				pDst = (_Tx*)(((LPBYTE)pDst) + inImage.stride);
				pSrc += inImage.w;
			}
		}
	}
};

void FMapToImgX(FLOAT *pSrc, IMAGE &Img, double _Min, double _Max)
{
	ProcessImage<FMapToImg_t>(Img, pSrc, _Min, _Max);
}

template <typename _Tx>
__forceinline _Tx SCA(LPELD pELD, _Tx a)
{
	switch( pELD->m_image.npix )
	{
	case PIX_BYTE:
	case PIX_CHAR:
		return a;
	case PIX_WORD:
	case PIX_SHORT:
	case PIX_DWORD:
	case PIX_LONG:
	case PIX_FLOAT:
	case PIX_DOUBLE:
		return a * 257;
	}
	return a;
}

//////////////////////////////////////////////////////////////////////////

ProtocolOut::ProtocolOut()
{
	m_hDialog = NULL;
	m_hHeader = NULL;
	m_hProtocolList = NULL;
	m_nType = LATTICE_REFINEMENT;
}

void ProtocolOut::Initialize(HWND hDialog)
{
	if( NULL != hDialog )
	{
		m_hDialog = hDialog;
		m_hHeader = GetDlgItem(hDialog, IDC_ELD_LIST_HEADER);
		m_hProtocolList = GetDlgItem(hDialog, IDC_ELD_LIST);
		AddProtocolString("No data has been extracted yet");
	}
}

void ProtocolOut::SetHeader(LPCSTR pszHeader)
{
	SetWindowText(m_hHeader, pszHeader);
}

void ProtocolOut::AddProtocolString(LPCSTR pszString)
{
	SendMessage(m_hProtocolList, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)pszString);
}

void ProtocolOut::AddProtocolString(LPCWSTR pszString)
{
	SendMessage(m_hProtocolList, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPWSTR)pszString);
}

void ProtocolOut::AddProtocolString(const _tstd::string &_format)
{
	SendMessage(m_hProtocolList, CB_INSERTSTRING, (WPARAM)-1, (LPARAM)(LPSTR)_format.c_str());
}

void ProtocolOut::ResetContent()
{
	SendMessage(m_hProtocolList, CB_RESETCONTENT, 0, 0);
}

int ProtocolOut::GetCount()
{
	return (int)SendMessage(m_hProtocolList, CB_GETCOUNT, 0, 0);
}

void ProtocolOut::SetCurSel(int nCurSel)
{
	SendMessage(m_hProtocolList, CB_SETCURSEL, nCurSel, 0);
}

//////////////////////////////////////////////////////////////////////////

ELD::ELD()
	: m_nPeakWindow(10)
	, m_pPar(0)
	, m_pObj(0)
	, m_mean(0)
	, m_min(0)
	, m_max(0)
	, m_hkmax(0)
	, m_hkmaxe(0) // Maximum hk index for extraction
	, m_bSmartLR(0) // Smart reflection coordinates
	, m_mask(0)
	, m_nMaskW(0)
	, m_nMaskH(0)
	, m_as(0)
	, m_ds(0)
	, m_ws(0)
	, m_wn(0)
	, m_wa(0)
	, m_map(0)
	, m_ppt(0)
	, m_xms(0)
	, m_yms(0)
	, m_rms(50)
	, m_rma(0)
	, m_athr(0)
	, m_athd(0)
	, m_flags(0)
	, m_pCurRefl(0)
	, m_r0(0)
	, m_r1(0)
	, m_sg(0)
	, m_sgx(0)
	, m_kf(0)
	, m_gr1(0)
	, m_wx0(0)
	, m_wy0(0)
	, m_wx1(0)
	, m_wy1(0)
	, m_zy(0)
	, m_MeanSigma(0)
	, m_UserSigma(0)
	, m_ksg(70)
	, m_ksg0(70)
	, m_esdx(0)
	, m_esdy(0)
	, m_bUseFixedSigma(0)
	, m_bProtocol(0)
	, m_last(0)
	, m_bMarks(0)
	, m_bIndex(0)
	, m_bNotListY(0)
	, m_arad(0)
	, m_uh(0)
	, m_uk(0)
	, m_ul(0)
	, m_vh(0)
	, m_vk(0)
	, m_vl(0)
	, m_f2to3(0)
	, m_dvfrom(0)
	, m_dvto(0)
	, m_infrom(0)
	, m_into(0)
	, m_bCentOnly(0)
	, m_cfoi(0)
	, m_meanampl(0)
	, m_n10Histo(0)
	, m_bRefinedLattice(0)
	, m_pZOLZ(0)
	, m_pCurLZ(0)
	, m_bDrawAxes(0)
	, m_handle(0)
	, m_id(0)
	, m_nCurPass(0)
	, m_bUseLaueCircles(0)
	, m_bPaintLaueCirc(0)
	, m_bPaintHOLZaxes(0)
	, m_nCurLaueZone(0)
{
	ZeroMemory(&m_uu, sizeof(m_uu));
	ZeroMemory(&m_vv, sizeof(m_vv));
	ZeroMemory(&m_kk, sizeof(m_kk));
	ZeroMemory(&m_ll, sizeof(m_ll));
}

ELD::~ELD()
{
}

void ELD::GetIndex3D(CVector3d &za, double h, double k, int nLZ, CVector3l &vOut)
{
	CVector3d v1(m_uh, m_uk, m_ul), v2(m_vh, m_vk, m_vl), v3, res;
	double denom = FastAbs(za.x) + FastAbs(za.y) + FastAbs(za.z);
	if( denom > 1e-2 )
	{
		res = h*v1 + k*v2 + (nLZ / denom)*za;
	}
	else
	{
		res = h*v1 + k*v2 + nLZ*za;
	}
	vOut.x = (res.x < 0) ? (int)::ceil(res.x - 0.5) : (int)::floor(res.x + 0.5);
	vOut.y = (res.y < 0) ? (int)::ceil(res.y - 0.5) : (int)::floor(res.y + 0.5);
	vOut.z = (res.z < 0) ? (int)::ceil(res.z - 0.5) : (int)::floor(res.z + 0.5);
}

/****************************************************************************/
void RepaintWindow(HWND hWnd)
{
	InvalidateRect(hWnd, NULL, TRUE);
	RedrawWindow(hWnd, NULL, NULL, RDW_UPDATENOW);
}
// Added by Peter on [14/8/2005]
void RepaintImgWindow(OBJ* lpObj)
{
	OBJ* lpImg = OBJ::GetOBJ(lpObj->ParentWnd);
	if( NULL != lpImg )
	{
		RepaintWindow(lpImg->hWnd);
	}
}
// End of addition [14/8/2005]
/****************************************************************************/
//void XY2Sc(float *x, float *y, OBJ* O, float xc, float yc)
void XY2Sc(float &x, float &y, OBJ* O, float xc, float yc)
{
	// Added by Peter on [11/8/2005]
	double scale = (float)O->scu / (float)O->scd;
	// End of addition [11/8/2005]
//	*x = (*x + xc - O->xo) * (float)O->scu / (float)O->scd + (O->scu/2);
//	*y = (*y + yc - O->yo) * (float)O->scu / (float)O->scd + (O->scu/2);
	x = (x + xc - O->xo) * scale + (O->scu*0.5);
	y = (y + yc - O->yo) * scale + (O->scu*0.5);
}
/****************************************************************************/
template <typename _Tx>
_Tx AbsC(const _Tx &re, const _Tx &im)
{
	return (_Tx)FastSqrt(re*re + im*im);
}
/****************************************************************************/
static double AB_Angle(double ax, double ay, double bx, double by)
{
	return FastAbs(P2R(R2P( FastAtan2(by, bx) - FastAtan2(ay, ax) )));
//	return FastAbs(P2R(R2P( ATAN2(by, bx) - ATAN2(ay, ax) )));
}
/****************************************************************************/
static double SGauss(double s, double x)
{
	double	g, a = 100.0;
//	g = AGauss(s,x);
//	x *= s;
//	s = 1.+x*.5;
//	g = 0.8*g + 0.2 / s / s;

	x *= s;
	g = AGauss(1,x);
//	x = a*x / RMS;
//	g = (g+1./(1.+x*x))*.5;

	return g;
}
/****************************************************************************/
float ELD::CGauss(float s, float x)
{
	x = s * x * m_ksg;
	if( x >= 2047 )
		return 0;
	int x0 = (int)floor(x);
	x -= x0;
	return m_ws[x0]*(1.0 - x) + m_ws[x0 + 1] * x;
}

float ELD::XGauss(float s, float x)
{
	x = s * x * m_ksg0;
	if( x >= 2047)
		return 0;
	int x0 = (int)floor(x);
	x -= x0;
	return m_wn[x0]*(1.0 - x) + m_wn[x0+1]*x;
}
/****************************************************************************/
/*
#define USE_QSORT

#ifdef USE_QSORT
#	define CMP_ARG1	const void*
#	define CMP_ARG2	const void*
#else
#	define CMP_ARG1	const ARITEM&
#	define CMP_ARG2	const EREFL&
#endif

//////////////////////////////////////////////////////////////////////////
// Added by Peter on 17 Mar 2006
static int Others_cmp(CMP_ARG1 x1, CMP_ARG1 x2)
{
#ifdef USE_QSORT
	LPARITEM	R1 = (LPARITEM)x1, R2 = (LPARITEM)x2;
#endif
	if( R1->nOthers < R2->nOthers )  return  1;
	if( R1->nOthers > R2->nOthers )  return -1;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
static int f_cmp(CMP_ARG1 x1, CMP_ARG1 x2)
{
#ifdef USE_QSORT
	LPARITEM	R1 = (LPARITEM)x1, R2 = (LPARITEM)x2;
#endif
	if( R1->l  > R2->l)  return  1;
	if( R1->l  < R2->l)  return -1;
	return 0;
}
//------------------------------------------------------------------
static int f_cmp_n(CMP_ARG1 x1, CMP_ARG1 x2)
{
#ifdef USE_QSORT
	LPARITEM	R1 = (LPARITEM)x1, R2 = (LPARITEM)x2;
#endif
	if(R1->n  > R2->n)  return(-1);
	if(R1->n  < R2->n)  return(1);
	return(0);
}
*/
//------------------------------------------------------------------
static int a0_cmp(const void* x1, const void* x2)
{
	LPEREFL	R1 = (LPEREFL)x1, R2 = (LPEREFL)x2;
	if(R1->a0  > R2->a0)  return( 1);
	if(R1->a0  < R2->a0)  return(-1);
	return(0);
}
//------------------------------------------------------------------
static int hk_cmp(const void* x1, const void* x2)
{
	LPEREFL	R1 = (LPEREFL)x1, R2 = (LPEREFL)x2;
	if(abs(R1->h) > abs(R2->h)) return( 1);
	if(abs(R1->h) < abs(R2->h)) return(-1);
	if(abs(R1->k) > abs(R2->k)) return( 1);
	if(abs(R1->k) < abs(R2->k)) return(-1);
	if(R1->h > R2->h) return( 1);
	if(R1->h < R2->h) return(-1);
	if(R1->k > R2->k) return( 1);
	if(R1->k < R2->k) return(-1);
	return(0);
}
//------------------------------------------------------------------
static int bg_cmp(const void* x1, const void* x2)
{
	LPEREFL	R1 = (LPEREFL)x1, R2 = (LPEREFL)x2;
	if(R1->bg  > R2->bg)  return( 1);
	if(R1->bg  < R2->bg)  return(-1);
	return(0);
}
//------------------------------------------------------------------
static int dval_cmp(const void* x1, const void* x2)
{
	LPEREFL	R1 = (LPEREFL)x1, R2 = (LPEREFL)x2;
	if(R1->dval  < R2->dval)  return( 1);
	if(R1->dval  > R2->dval)  return(-1);
	return hk_cmp(x1,x2);
}
//------------------------------------------------------------------
static void UpdateReflInfo(HWND hDlg, LPELD pEld, int ch)
{
	LaueZone *pLZ;
	int	i;

	// Added by Peter on [8/8/2005]
	pLZ = pEld->m_pCurLZ;
	// end of addition by Peter [8/8/2005]
	for(i = 0; i < 3; i++)
	{
		if( ch )
		{
			SetDlgItemInt( hDlg, idc_h[i], pLZ->h[i], TRUE);
			SetDlgItemInt( hDlg, idc_k[i], pLZ->k[i], TRUE);
		}
	}
	SetDlgItemInt( hDlg, IDC_ELD_PASS, pLZ->nEstMax, FALSE);
	// TODO: Do we need to store these for each LZ?
	SetDlgItemInt( hDlg, IDC_ELD_UU1, pEld->m_uu[0], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_VV1, pEld->m_vv[0], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_UU2, pEld->m_uu[1], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_VV2, pEld->m_vv[1], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_HH1, pEld->m_hh[0], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_KK1, pEld->m_kk[0], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_LL1, pEld->m_ll[0], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_HH2, pEld->m_hh[1], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_KK2, pEld->m_kk[1], TRUE);
	SetDlgItemInt( hDlg, IDC_ELD_LL2, pEld->m_ll[1], TRUE);
}
/****************************************************************************/
static void ScanAll( HWND hDlg, OBJ* O )
{
	static signed char		def_h[3] = {4,0,3};
	static signed char		def_k[3] = {0,4,3};
	BOOL	ff;
	int		i;
	LPELD	pEld = (LPELD) &O->dummy;
	LaueZone *pLZ;

	// Added by Peter on [8/8/2005]
	pLZ = pEld->m_pCurLZ;
	// end of addition by Peter [8/8/2005]
	for(i = 0; i < 3; i++)
	{
		pLZ->h[i] = GetDlgItemInt( hDlg, idc_h[i], &ff, TRUE );
		if( !ff )
			pLZ->h[i] = def_h[i];
		pLZ->k[i] = GetDlgItemInt( hDlg, idc_k[i], &ff, TRUE );
		if( !ff )
			pLZ->k[i] = def_k[i];
	}
	for(i = 0; i < 2; i++)
	{
		pEld->m_uu[i] = GetDlgItemInt( hDlg, idc_uu[i], &ff, TRUE );
		if (!ff) pEld->m_uu[i] = def_uu[i];
		pEld->m_vv[i] = GetDlgItemInt( hDlg, idc_vv[i], &ff, TRUE );
		if (!ff) pEld->m_vv[i] = def_vv[i];
		pEld->m_hh[i] = GetDlgItemInt( hDlg, idc_hh[i], &ff, TRUE );
		if (!ff) pEld->m_hh[i] = def_hh[i];
		pEld->m_kk[i] = GetDlgItemInt( hDlg, idc_kk[i], &ff, TRUE );
		if (!ff) pEld->m_kk[i] = def_kk[i];
		pEld->m_ll[i] = GetDlgItemInt( hDlg, idc_ll[i], &ff, TRUE );
		if (!ff) pEld->m_ll[i] = def_ll[i];
	}
	i = GetDlgItemInt(hDlg, IDC_ELD_PASS, &ff, FALSE);
	if( ff )
	{
		if( i < 0 )  i = 2;
		if( i > 10 ) i = 10;
		pLZ->nEstMax = i;
	}
	pEld->m_nPeakWindow = GetDlgItemInt(hDlg, IDC_ELD_PEAK_SEARCH, &ff, TRUE);
	if( pEld->m_nPeakWindow < 2 )
	{
		pEld->m_nPeakWindow = 2;
	}
	UpdateReflInfo(hDlg, pEld, TRUE);
}
//------------------------------------------------------------------
static void Line_To(HDC hdc, HPEN pen, float x0, float y0, float x1, float y1)
{
	SelectPen(hdc, pen);
	_MoveTo(hdc, round(x0), round(y0));
	LineTo(hdc, round(x1), round(y1));
}
static void HLine_To(HDC hdc, HPEN pen, float x0, float y0, float x1)
{
	Line_To(hdc, pen, x0, y0, x1, y0);
}
static void VLine_To(HDC hdc, HPEN pen, float x0, float y0, float y1)
{
	Line_To(hdc, pen, x0, y0, x0, y1);
}
//------------------------------------------------------------------
static void Poly_Line(HDC hdc, HPEN pen, LPPOINT ppt, int n)
{
	SelectPen( hdc, pen );
	_MoveTo(hdc,ppt[0].x, ppt[0].y); Polyline(hdc, ppt, n);
}
//------------------------------------------------------------------
static void FLine_To(HDC hdc, HPEN pen, LPELD pELD, float x0, float y0, float x1, float y1)
{
	float	kx = pELD->m_wx1-pELD->m_wx0;
	float	ky = pELD->m_zy*(pELD->m_wy1-pELD->m_wy0);
	Line_To(hdc, pen, pELD->m_wx0+x0*kx, pELD->m_wy1-y0*ky, pELD->m_wx0+x1*kx, pELD->m_wy1-y1*ky);
}
static void FHLine_To(HDC hdc, HPEN pen, LPELD pELD, float x0, float y0, float x1)
{
	FLine_To(hdc, pen, pELD, x0, y0, x1, y0);
}
static void FVLine_To(HDC hdc, HPEN pen, LPELD pELD, float x0, float y0, float y1)
{
	FLine_To(hdc, pen, pELD, x0, y0, x0, y1);
}
static void FCircle(HDC hdc, HPEN pen, LPELD pELD, float x0, float y0, float rad)
{
	float	kx = pELD->m_wx1-pELD->m_wx0;
	float	ky = pELD->m_zy*(pELD->m_wy1-pELD->m_wy0);
	int	x = round(kx*x0 + pELD->m_wx0);
	int	y = round(pELD->m_wy1 - ky*y0);
	int	r = round(kx*rad);
	SelectPen( hdc, pen );
	Ellipse( hdc, x-r, y-r, x+r, y+r);
}
//------------------------------------------------------------------
static void FPoly_Line(HDC hdc, HPEN pen, LPELD pELD, LPFLOAT d, int n, float sc, float ofs)
{
	int		i;
	float	kx = (pELD->m_wx1-pELD->m_wx0) / n;
	float	ky = (pELD->m_wy1-pELD->m_wy0) * sc * pELD->m_zy;

	for(i = n; i>=0; i--)
	{
		pELD->m_ppt[n-i].x = round(pELD->m_wx0 - i * kx);
		pELD->m_ppt[n+i].x = round(pELD->m_wx0 + i * kx);
		pELD->m_ppt[n-i].y = pELD->m_ppt[n+i].y = round(pELD->m_wy1 - (d[i] + ofs) * ky);
	}
	Poly_Line(hdc, pen, pELD->m_ppt, n+n+1);
}
//------------------------------------------------------------------
static void xLine( HDC hdc, OBJ* O, float x0, float y0, float x1, float y1, float xc, float yc)
{
	XY2Sc( x0, y0, O , xc, yc);
	_MoveTo( hdc, round(x0), round(y0) );
	XY2Sc( x1, y1, O , xc, yc);
	LineTo( hdc, round(x1), round(y1) );
	LineTo( hdc, round(x1)-1, round(y1) );
}
//------------------------------------------------------------------
void ELD::DrawMark( float x, float y, int pn, int r )
{
	DrawMark(x, y, PEN[pn], r);
}
//------------------------------------------------------------------
void ELD::DrawMark(float x, float y, HPEN pn, int r)
{
	HWND	hPar = m_pObj->ParentWnd;
	OBJ*	pr  = OBJ::GetOBJ(hPar);
	HDC		hdc = GetDC (hPar);
	HPEN	op;
	HBRUSH  ob;
	int		x0, y0, orop;

	orop = SetROP2(hdc, R2_COPYPEN);
	op = SelectPen(hdc, pn);
	if( r > 0 )
	{
		ob = SelectBrush(hdc, GetStockObject(NULL_BRUSH));
		r = round(abs(round(r*.5)) * (float)pr->scu / (float)pr->scd + (pr->scu/2));
		XY2Sc(x, y, pr , 0, 0);
		x0 = round(x);
		y0 = round(y);
		Ellipse( hdc, x0-r, y0-r, x0+r, y0+r);
		SelectBrush(hdc, ob);
	}
	else
	{
		xLine( hdc, pr, x-r, y-r, x+r, y+r, 0, 0);
		xLine( hdc, pr, x-r, y+r, x+r, y-r, 0, 0);
	}
	SetROP2( hdc, orop );
	SelectPen( hdc, op );
	ReleaseDC( hPar, hdc );
}
// End of addition [9/8/2005]

BOOL ELD::IsCalibrated()
{
	//(FastAbs(scale - 1e-10) > 1e-12) &&
	return 0 != (m_pObj->NI.Flags & NI_DIFFRACTION_PATTERN);
}

//------------------------------------------------------------------
// Added by Peter on [14/8/2005]
//void RecalculateRecCell(OBJ* lpObj, LPELD pEld, LaueZone *pLZ, BOOL bResetAllLaueZones)
void ELD::RecalculateRecCell(LaueZone *pLZ, BOOL bResetAllLaueZones)
{
	double x, cf = m_pObj->NI.AspectXY, scale = m_pObj->NI.Scale;

	OBJ* obj = GetBaseObject(m_pObj);

	x = FastAbs(1.0 / (m_h_dir.x*m_k_dir.y*cf - m_h_dir.y*cf*m_k_dir.x));
	if(IsCalibrated())
	{
		x *= scale * 1e10;
	}
	// Commented by Peter on [11 Feb 2006]
	pLZ->gamma = AB_Angle( m_h_dir.x, m_h_dir.y*cf, m_k_dir.x, m_k_dir.y*cf);
//	if( pLZ->gamma >= 0.5 * M_PI )
//	{
//		pLZ->gamma = M_PI - pLZ->gamma;
//		pEld->m_k_dir *= -1;
//		pLZ->k[0] *= -1;
//		pLZ->k[1] *= -1;
//		pLZ->k[2] *= -1;
//	}
	// End of comment by Peter on [11 Feb 2006]
	pLZ->a_length = AbsC(m_h_dir.x, m_h_dir.y*cf);
	pLZ->b_length = AbsC(m_k_dir.x, m_k_dir.y*cf);
	pLZ->ral = x * AbsC(m_k_dir.x, m_k_dir.y*cf);
	pLZ->rbl = x * AbsC(m_h_dir.x, m_h_dir.y*cf);
	if( TRUE == bResetAllLaueZones )
	{
		// now set this basic set to all other Zone Axes
		for(int i = 1; i < ELD_MAX_LAUE_CIRCLES; i++)
		{
			m_vLaueZones[i]->m_nFlags = LaueZone::ZONE_UNINITIALIZED;
			m_vLaueZones[i]->a_length = pLZ->a_length;
			m_vLaueZones[i]->b_length = pLZ->b_length;
			m_vLaueZones[i]->ral = pLZ->ral;
			m_vLaueZones[i]->rbl = pLZ->rbl;
			m_vLaueZones[i]->gamma = pLZ->gamma;
			m_vLaueZones[i]->rgamma = pLZ->rgamma;
			m_vLaueZones[i]->shift = 0;
		}
	}

	m_pObj->HandleWarningMessage(_T("Diffraction pattern not calibrated. Click here to calibrate."), !IsCalibrated(), IDM_V_INFO);
}
// End of addition [14/8/2005]
//------------------------------------------------------------------
static int ReflColor( LPEREFL R, LPELD pELD, BOOL bIntegrate )
{
	int	pn=2;	// Good refl

	if ( !bIntegrate )
	{
		if      ((R->flags &(FLAG_BAD_SHAPE | FLAG_FLAT_CENTER | FLAG_UNDEFINED_CENTER | FLAG_BELOW_BACKGROUND | FLAG_TOO_NOISY | FLAG_MISPLACED)) == 0) pn = 2;
		else if ((R->flags &(FLAG_BAD_SHAPE | FLAG_BELOW_BACKGROUND | FLAG_TOO_NOISY | FLAG_FLAT_CENTER | FLAG_UNDEFINED_CENTER)) != 0 && R->ae<pELD->m_meanampl/3. ) pn = 8;
		else if ((R->flags &(FLAG_BAD_SHAPE | FLAG_BELOW_BACKGROUND | FLAG_TOO_NOISY | FLAG_FLAT_CENTER | FLAG_UNDEFINED_CENTER)) != 0) pn = 1;
		else if ((R->flags & FLAG_MISPLACED )!=0) pn = 5;
		else pn = 7;
	}
	return pn;
}
//------------------------------------------------------------------
// Added by Peter on [9/8/2005]
static void Params2Dlg(HWND hDlg, LPELD pEld)
{
	BOOL bEnable = FALSE;
	LaueZone *pLZ = pEld->m_pCurLZ;

	if( pLZ->m_bHaveCell3D )
	{
		CVector3l vA, vB, vZA;
		// enable 3D cell
		pEld->m_f2to3 = TRUE;
		// reflection A
		pEld->m_uu[0] = 1;
		pEld->m_vv[0] = 0;
		vA.x = pEld->m_hh[0] = pLZ->m_mChangeBase.d[0][0];
		vA.y = pEld->m_kk[0] = pLZ->m_mChangeBase.d[1][0];
		vA.z = pEld->m_ll[0] = pLZ->m_mChangeBase.d[2][0];
		// Reflection B
		pEld->m_uu[1] = 0;
		pEld->m_vv[1] = 1;
		vB.x = pEld->m_hh[1] = pLZ->m_mChangeBase.d[0][1];
		vB.y = pEld->m_kk[1] = pLZ->m_mChangeBase.d[1][1];
		vB.z = pEld->m_ll[1] = pLZ->m_mChangeBase.d[2][1];

		vZA = vA ^ vB;
		long gcd1 = FindGCD2(vZA.x, vZA.y);
		long gcd2 = FindGCD2(vZA.x, vZA.z);
		long gcd3 = FindGCD2(gcd1, gcd2);
		if( abs(gcd3) > 0 )
		{
			vZA /= (double)gcd3;
		}
		sprintf(TMP, "[%d,%d,%d]", vZA.x, vZA.y, vZA.z);
		SetDlgItemText( hDlg, IDC_ELD_ZONEAX, TMP );
		CheckDlgButton( hDlg, IDC_ELD_USE3D, 1 );
		wndChangeDlgSize(hDlg, hDlg,
			IsDlgButtonChecked(hDlg, IDC_ELD_USE3D) ? 0 : IDC_ELD_2D3D,
			IsDlgButtonChecked(hDlg, IDC_ELD_DEFINE_HOLZ) ? 0 : s_dwELD_with_SpinnigStar,
			"ELD", hInst );
		TextToDlgItemW(hDlg, IDC_ELD_3D_A , "%6.3f %c", pLZ->m_adConvCell[0], ANGSTR_CHAR);
		TextToDlgItemW(hDlg, IDC_ELD_3D_B , "%6.3f %c", pLZ->m_adConvCell[1], ANGSTR_CHAR);
		TextToDlgItemW(hDlg, IDC_ELD_3D_C , "%6.3f %c", pLZ->m_adConvCell[2], ANGSTR_CHAR);
		TextToDlgItemW(hDlg, IDC_ELD_3D_AL, "%6.2f %c", pLZ->m_adConvCell[3], DEGREE_CHAR);
		TextToDlgItemW(hDlg, IDC_ELD_3D_BT, "%6.2f %c", pLZ->m_adConvCell[4], DEGREE_CHAR);
		TextToDlgItemW(hDlg, IDC_ELD_3D_GM, "%6.2f %c", pLZ->m_adConvCell[5], DEGREE_CHAR);
		TextToDlgItemW(hDlg, IDC_3D_CELL_TYPE, "%s, %s", pLZ->m_sCellType, pLZ->m_sCentring);
		UpdateReflInfo(hDlg, pEld, TRUE);
	}
	// enable 3D cell if possible
	ShowWindow(GetDlgItem(hDlg, IDC_3D_CELL_TITLE), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_3D_CELL_TYPE ), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_A ), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_B ), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_C ), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_AL), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_BT), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_GM), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_A2 ), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_B2 ), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_C2 ), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_AL2), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_BT2), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	ShowWindow(GetDlgItem(hDlg, IDC_ELD_3D_GM2), pLZ->m_bHaveCell3D ? SW_SHOW : SW_HIDE);
	if( pLZ->c_length > 0 )
	{
		bEnable = TRUE;
	}
	HWND hWnd = GetDlgItem(hDlg, IDC_ELD_CS);
	EnableWindow(hWnd, bEnable);
	ShowWindow(hWnd, (TRUE == bEnable) ? SW_SHOW : SW_HIDE);
	if( pLZ->m_nFlags & LaueZone::ZONE_INDEXED )
	{
		if( FALSE == pEld->IsCalibrated() )				// Default
		{
			TextToDlgItemW(hDlg, IDC_ELD_AS, "a*=%6.3f ", pLZ->a_length);
			TextToDlgItemW(hDlg, IDC_ELD_BS, "b*=%6.3f ", pLZ->b_length);
			TextToDlgItemW(hDlg, IDC_ELD_CS, "c*=%6.3f ", pLZ->c_length);
			TextToDlgItemW(hDlg, IDC_ELD_GS, "%c*=%6.2f%c", GAMMA_CHAR, R2D(pLZ->gamma), DEGREE_CHAR);
//			TextToDlgItemW(hDlg, IDC_ELD_GS, L"%c*=%6.3f\176", 947, R2D(pLZ->gamma));
//			TextToDlgItem(hDlg, IDC_ELD_GS, "\201*=%6.3f ", R2D(pLZ->gamma));
		}
		else
		{
//			PrintWCharText(TMP, "a=%6.3f%c ", pLZ->ral, ANGSTR_CHAR);
			TextToDlgItemW(hDlg, IDC_ELD_AS, "a=%6.3f%c ", pLZ->ral, ANGSTR_CHAR);
			TextToDlgItemW(hDlg, IDC_ELD_BS, "b=%6.3f%c ", pLZ->rbl, ANGSTR_CHAR);
			TextToDlgItemW(hDlg, IDC_ELD_CS, "c=%6.3f%c ", pLZ->rcl, ANGSTR_CHAR);
			TextToDlgItemW(hDlg, IDC_ELD_GS, "%c=%6.2f%c ", GAMMA_CHAR, R2D(M_PI - pLZ->gamma), DEGREE_CHAR);
//			TextToDlgItem(hDlg, IDC_ELD_AS, "a=%6.3fÅ ", pLZ->ral);
//			TextToDlgItem(hDlg, IDC_ELD_BS, "b=%6.3fÅ ", pLZ->rbl);
//			TextToDlgItem(hDlg, IDC_ELD_CS, "c=%6.3fÅ ", pLZ->rcl);
//			TextToDlgItem(hDlg, IDC_ELD_GS, "\201=%6.3f  ", R2D(M_PI - pLZ->gamma));
//			TextToDlgItemW(hDlg, IDC_ELD_AS, L"a=%6.3f%c ", pLZ->ral, 197);
//			TextToDlgItemW(hDlg, IDC_ELD_BS, L"b=%6.3f%c ", pLZ->rbl, 197);
//			TextToDlgItemW(hDlg, IDC_ELD_CS, L"c=%6.3f%c ", pLZ->rcl, 197);
//			TextToDlgItemW(hDlg, IDC_ELD_GS, "%c=%6.3f  ", 176, R2D(M_PI - pLZ->gamma));
		}
	}
	else
	{
		TextToDlgItemW(hDlg, IDC_ELD_AS, "a=");
		TextToDlgItemW(hDlg, IDC_ELD_BS, "b=");
		TextToDlgItemW(hDlg, IDC_ELD_CS, "c=");
		TextToDlgItemW(hDlg, IDC_ELD_GS, "%c=", GAMMA_CHAR);
//		TextToDlgItemW(hDlg, IDC_ELD_GS, L"%c=", 947);
//		TextToDlgItem(hDlg, IDC_ELD_GS, "\201=");
	}
	TextToDlgItemW(hDlg, IDC_ELD_PEAK_SEARCH, "%d", pEld->m_nPeakWindow);
}
//------------------------------------------------------------------
// Completely modified by Peter on 20 Mar 2006
//void RecalculateUserRefls(OBJ* pObj, LaueZone *pLZ, LPARITEM pRefls1/* = NULL*/, int nSize/* = 0*/)
void ELD::RecalculateUserRefls(LaueZone *pLZ, LPARITEM pRefls1/* = NULL*/, int nSize/* = 0*/)
{
	int i, j, h, k, n;
	float x, y, rad;//, as;
// 	DWORD amp;
	double amp, as;

	// Added by Peter, 10 August 2009
	// If we have manual refinement we should not reconcider the 3 user chosen spots, simply exit
	if( false == pLZ->bAuto )
	{
		return;
	}
	// End of addition by Peter, 10 August 2009

	rad = (int)::floor(0.5*__min(pLZ->a_length, pLZ->b_length) + 0.5);

	m_nPeakWindow = rad;

	// look over 3 reflections
	for(i = 0; i < 3; i++)
	{
		h = pLZ->h[i];
		k = pLZ->k[i];
		x = pLZ->GetPeakAbsX(h, k);
		y = pLZ->GetPeakAbsY(h, k);
//		n = PeakCenter(/*pObj, */m_nPeakWindow/*8*/, x, y, m_nPeakWindow/*8.*/, 0.5, TRUE, /*NULL*/amp, as);
		n = PeakCenter2(m_nPeakWindow, x, y, m_nPeakWindow, 0.5, TRUE, amp, as);
		// if the position doesn't correspond to a good reflection
		if( n & FLAG_ERC )
		{
			// added by Peter on 20 Mar 2006
			if( NULL != pRefls1 )
			{
				double dMin, dDist, dx, dy, h_tmp, k_tmp, det;
				int i_h, i_k, h_new, k_new, j_closest;

				// determinant should be good
//				det = pEld->m_h_dir.y*pEld->m_k_dir.x - pEld->m_h_dir.x*pEld->m_k_dir.y;
				det = m_h_dir.y*m_k_dir.x - m_h_dir.x*m_k_dir.y;
				if( FastAbs(det) > 1e-3 )
				{
					h_new = k_new = 0;
					det = 1.0 / det;
					dMin = 1e30;
					j_closest = -1;
					// try to find a closest reflection which belongs to the determined lattice
					for(j = 0; j < nSize; j++)
					{
						dx = /*x - */pRefls1[j].x - m_center.x;
						dy = /*y - */pRefls1[j].y - m_center.y;
						dDist = AbsC(x - pRefls1[j].x, y - pRefls1[j].y);
						// find the indices for the vec
						h_tmp = FastAbs((dy * m_k_dir.x - dx * m_k_dir.y) * det);
						k_tmp = FastAbs((dx * m_h_dir.y - dy * m_h_dir.x) * det);
						if( h_tmp >= 0.0 ) i_h = (int)::floor(h_tmp + 0.5);
						else i_h = (int)::ceil(h_tmp + 0.5);
						if( k_tmp >= 0.0 ) i_k = (int)::floor(k_tmp + 0.5);
						else i_k = (int)::ceil(k_tmp + 0.5);
						if( (FastAbs(i_h - h_tmp) < 0.1) && (FastAbs(i_k - k_tmp) < 0.1) &&
							(dDist < dMin) )
						{
							// try to calculate the cross-product with other 2 reflection
							int prod1, prod2, i1, i2;
							i1 = i + 1;
							i2 = i + 2;
							if( i1 > 2 ) i1 -= 2;
							if( i2 > 2 ) i2 -= 2;
							prod1 = pLZ->h[i1] * i_k - pLZ->k[i1] * i_h;
							prod2 = pLZ->h[i2] * i_k - pLZ->k[i2] * i_h;
							// remember the indices if cross-products are not 0
							if( (0 != prod1) && (0 != prod2) )
							{
								h_new = i_h;
								k_new = i_k;
								dMin = dDist;
								j_closest = j;
							}
						}
					}
					if( (!((h_new == 0) && (k_new == 0))) && (-1 != j_closest) )
					{
						dx = pRefls1[j_closest].x - m_center.x;
						dy = pRefls1[j_closest].y - m_center.y;
						h_tmp = FastAbs((dy * m_k_dir.x - dx * m_k_dir.y) * det);
						k_tmp = FastAbs((dx * m_h_dir.y - dy * m_h_dir.x) * det);
						i_h = (int)::floor(h_tmp + 0.5);
						i_k = (int)::floor(k_tmp + 0.5);
						if( (FastAbs(i_h - h_tmp) < 0.1) && (FastAbs(i_k - k_tmp) < 0.1) )
						{
							x = pLZ->GetPeakAbsX(i_h, i_k);
							y = pLZ->GetPeakAbsY(i_h, i_k);
							n = PeakCenter2(m_nPeakWindow, x, y, m_nPeakWindow, 0.5, TRUE, amp, as);
// 							n = PeakCenter(m_nPeakWindow, x, y, m_nPeakWindow, 0.5, TRUE, amp, as);
// 							n = PeakCenter(8, x, y, 8., 0.5, TRUE, amp, as);
//							n = PeakCenter(pObj, 8, &x, &y, 8., 0.5, TRUE, NULL, as);
							if( 0 == (n & FLAG_ERC) )
							{
								pLZ->h[i] = i_h;
								pLZ->k[i] = i_k;
								pLZ->x[i] = x;
								pLZ->y[i] = y;
							}
						}
					}
					else
					{
						::OutputDebugString("RecalculateUserRefls() -> failed to locate a valid reflection.\n");
					}
				}
			}
			else
			{
				pLZ->x[i] = pLZ->y[i] = 0;
			}
			// commented by Peter on 20 Mar 2006
//			pLZ->x[i] = pLZ->y[i] = 0;
		}
		else
		{
			// succeeded with picking a reflection
			pLZ->x[i] = x;
			pLZ->y[i] = y;
		}
	}
	UpdateReflInfo(m_pObj->hWnd, this, 1);
}
// end of addition by Peter [9/8/2005]
//------------------------------------------------------------------
void DrawELD(OBJ* pObj)
{
	LPELD		pEld = (LPELD) &pObj->dummy;
	LPEREFL		R;
	HWND		hDlg = pObj->hWnd;
	HWND		par = pObj->ParentWnd;
	OBJ*		pr  = OBJ::GetOBJ(par);
	HDC			hDC = GetDC   (par);
	HPEN		hop = SelectPen( hDC, RedPen );
	HBRUSH		hob = SelectBrush(hDC, GetStockObject(NULL_BRUSH));
	int			i, x, y, r;
	float		x0, y0;
	BYTE*		map = pEld->m_map;
	int			f, pn;
	CVector2d	point;
	LaueZone	*pLZ;
	//int			iIntegrate = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_ELD_INTENSITY_TYPE));

	// Added by Peter on [8/8/2005]
	pLZ = pEld->m_pCurLZ;
#ifdef USE_FONT_MANAGER
	int nFntSizeW, nFntSizeH;
	HFONT hFont = FontManager::GetFont(FM_SMALL_FONT, nFntSizeW, nFntSizeH);
#else
	int nFntSizeH = fnts_h;
	HFONT hFont = hfSmall;
#endif
	// end of addition by Peter [8/8/2005]
	for(i = 0; i < 3; i++)
	{
		if( (pLZ->x[i] < 1.0 ) && (pLZ->y[i] < 1.0) )
			continue;
		x0 = pLZ->x[i];
		y0 = pLZ->y[i];
		XY2Sc( x0, y0, pr, 0, 0 );
		x = round(x0);
		y = round(y0);
		switch(i)
		{
			case 0:	SelectPen( hDC, RedPen ); break;
			case 1:	SelectPen( hDC, BluePen );break;
			case 2:	SelectPen( hDC, GreenPen );break;
		}
		_MoveTo( hDC, x-1,  y-5 ); LineTo( hDC, x-1,  y-1 ); LineTo( hDC, x-6,  y-1 );
		_MoveTo( hDC, x+1,  y-5 ); LineTo( hDC, x+1,  y-1 ); LineTo( hDC, x+6,  y-1 );
		_MoveTo( hDC, x-5,  y+1 ); LineTo( hDC, x-1,  y+1 ); LineTo( hDC, x-1,  y+6 );
		_MoveTo( hDC, x+1,  y+5 ); LineTo( hDC, x+1,  y+1 ); LineTo( hDC, x+6,  y+1 );
		if( pEld->m_bIndex )
		{
			SetBkMode( hDC, OPAQUE );
			SetBkColor( hDC, RGB(0,0,0));
			SetTextColor(hDC, RGB(255,255,255));
			SelectFont( hDC, hFont );
			sprintf(TMP,"(%d,%d)", pLZ->h[i], pLZ->k[i]);
			TextOut( hDC, x+2, y - nFntSizeH - 2, TMP, strlen(TMP));
		}
	}
	if( pEld->m_bMarks && (pLZ->m_nFlags & LaueZone::ZONE_INT_EXTRACTED) )//(pEld->m_flags & ESR_DONE) )
	{
		for(R = &*pLZ->m_vRefls.begin(), i = 0; i < pLZ->m_vRefls.size(); R++, i++)
		{
			f = R->flags;
			if( R->i_amp > pEld->m_max - SCA(pEld,30) )
				f &= ~FLAG_FLAT_CENTER;

			pn = ReflColor(R, pEld, ELD::ELD_INTEGRATE_SHAPE_FIT != pEld->m_nIntegrationMode);
			SelectPen(hDC, PEN[pn]);
			EREFL reflection = *R;
			if( pEld->m_bSmartLR )
			{
				CalcSmartCoordinates( &*pLZ->m_vRefls.begin(), pLZ->m_vRefls.size(), &reflection);
			}
			else
			{
				reflection.xc = pEld->m_center.x + R->h*pEld->m_h_dir.x + R->k*pEld->m_k_dir.x + pLZ->shift.x;
				reflection.yc = pEld->m_center.y + R->h*pEld->m_h_dir.y + R->k*pEld->m_k_dir.y + pLZ->shift.y; 
			}
			pEld->DrawMark(reflection.xc, reflection.yc, pn, pEld->m_rma);	// was just x,y
			r = round(abs(round(pEld->m_rma*0.5)) * (float)pr->scu / (float)pr->scd + (pr->scu/2));

			if( 0 != (f & FLAG_USER_DELETED) )
			{
				_MoveTo(hDC, x-r, y-r);
				LineTo(hDC,x+r,y+r);
				_MoveTo(hDC, x-r, y+r);
				LineTo(hDC,x+r,y-r);
			}
		}
	}
	// draw axes
	// red pen
	if( TRUE == pEld->m_bDrawAxes )
	{
		CVector2d dir, dir_90, center, m_h_dir, m_k_dir, point;
		double dRad, a_len;

		m_h_dir  = pEld->m_h_dir;
		m_k_dir  = pEld->m_k_dir;
		center = pEld->m_center;
		m_h_dir.Normalize();
		m_k_dir.Normalize();
		dir = m_h_dir;
		// the direction perpendicular to the axis
		dir_90 = CVector2d(dir.y, -dir.x);
		a_len = pEld->m_pCurLZ->a_length;
		if( (true == pEld->m_vbLaueCircles[0]) && (pEld->m_bUseLaueCircles) )
		{
			dRad = pEld->m_vLaueZones[0]->m_dRadius;
		}
		else
		{
			dRad = __max(pEld->m_image.w, pEld->m_image.h);
		}
		HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
		SelectPen(hDC, hPen);
		for(int j = 0; j < 2; j++)
		{
			point = -dir*dRad + center;
			x0 = point.x;
			y0 = point.y;
			XY2Sc( x0, y0, pr, 0, 0 );
			x = floor(x0 + 0.5);
			y = floor(y0 + 0.5);
			_MoveTo(hDC, x, y);

			point = dir*dRad + center;
			x0 = point.x;
			y0 = point.y;
			XY2Sc( x0, y0, pr, 0, 0 );
			x = floor(x0 + 0.5);
			y = floor(y0 + 0.5);
			LineTo(hDC, x, y);

			int nMarks = (int)::floor(dRad / a_len + 0.5);
			for(int k = 1; k < nMarks; k++)
			{
				point = dir*a_len*k + center;
				x0 = point.x - dir.y * 5;
				y0 = point.y + dir.x * 5;
				XY2Sc( x0, y0, pr, 0, 0 );
				x = floor(x0 + 0.5);
				y = floor(y0 + 0.5);
				_MoveTo(hDC, x, y);

				x0 = point.x + dir.y * 5;
				y0 = point.y - dir.x * 5;
				XY2Sc( x0, y0, pr, 0, 0 );
				x = floor(x0 + 0.5);
				y = floor(y0 + 0.5);
				LineTo(hDC, x, y);
			}

			DeletePen(hPen);
			hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
			SelectPen(hDC, hPen);
			dir = m_k_dir;
			dir_90 = CVector2d(dir.y, -dir.x);
			a_len = pEld->m_pCurLZ->b_length;
		}
		DeletePen(hPen);
	}
	// draw Laue circles
	if( (pEld->m_bUseLaueCircles) && (pEld->m_bPaintLaueCirc) )
	{
		for(i = 0; i < pEld->m_vLaueZones.size(); i++)
		{
			if( false == pEld->m_vbLaueCircles[i] )
			{
				continue;
			}
			HPEN hPen = CreatePen(PS_SOLID, 1, pEld->m_vLaueZones[i]->m_rgbColor);
			double dRad = pEld->m_vLaueZones[i]->m_dRadius;
			dRad = dRad * (float)pr->scu / (float)pr->scd + (pr->scu*.5);
			x0 = pEld->m_center.x;
			y0 = pEld->m_center.y;
			XY2Sc( x0, y0, pr, 0, 0 );
			x = round(x0);
			y = round(y0);
			_MoveTo(hDC, x, y);
			SelectPen(hDC, hPen);
			Ellipse(hDC, x-dRad, y-dRad, x+dRad, y+dRad);
			DeleteObject(hPen);
		}
	}
	DrawELD_HOLZ(pObj, pEld, hDC);
/*
	// draw HOLZ reflections
	HPEN hPen = CreatePen(PS_SOLID, 1, pEld->m_vLaueCircRGB[1]);
	SelectPen(hDC, hPen);
	for(i = 0; i < pEld->m_vZOLZrefls.size(); i++)
	{
		point = pEld->m_vZOLZrefls[i];
		x0 = point.x;
		y0 = point.y;
		XY2Sc( x0, y0, pr, 0, 0 );
		x = round(x0);
		y = round(y0);
		_MoveTo(hDC, x, y);
		Ellipse(hDC, x-2, y-2, x+2, y+2);
	}
*/
	SelectPen(hDC, hop);
	SelectBrush(hDC, hob);
	ReleaseDC( par, hDC );
}
//------------------------------------------------------------------
static void Map_To_IMG(LPBYTE map, LPVOID img, int imgw, int imgh, float n)
{
	int		imgx, x_a, x_b, y_a, y_b, a0, b0;
	float	kx, ky;

	imgx = R4(imgw);
	kx = n * 256. * 64. / (float)imgw;	ky = n * 256. * 64. / (float)imgh;
	x_a =  round(kx);	y_a =  0;	x_b =  0;	y_b =  round(ky);
	a0 = 8192 - (x_a*imgw + y_a*imgh) / 2;
	b0 = 8192 - (x_b*imgw + y_b*imgh) / 2;
	X64_Copy(map, img, imgx, imgh, x_a, y_a, x_b, y_b, a0, b0, 0);
}
/****************************************************************************/
// static void PaintELDDlg(OBJ* O)
void ELD::PaintELDDlg()
{
	//LPELD	pEld = (LPELD) &O->dummy;
	//HWND	hDlg = O->hWnd;
	HWND	hDlg = m_pObj->hWnd;
	HWND	hw = GetDlgItem( hDlg, IDC_ELD_PLOT);
	HDC		hdc= GetDC( hw );
	HPEN	hop;
	HBRUSH  ob = SelectBrush(hdc, GetStockObject(NULL_BRUSH));
	RECT	rc;
	int		i, n, p, sx, sy;
	int		ll;
	float	x, y, ax, ay, a, km = 0.7f, gg;
	HRGN	hr;
	const CVector2d &h_dir = m_h_dir, &k_dir = m_k_dir;

	GetClientRect( hw, &rc );
	InflateRect(&rc, -1, -1);
	hr = CreateRectRgnIndirect(&rc);
	SelectClipRgn(hdc,hr);
	hop = SelectPen( hdc, GrayPen );
	sx = rc.right  - rc.left;
	sy = rc.bottom - rc.top;

// 	if( NULL == O->edp )
// 	{
// 		ll = R4(sx) * sy;
// 		O->edp = (LPBYTE)calloc(1, ll);
// 		if ( ! O->edp )
// 			return;
// 	}
	if( NULL == m_pObj->edp )
	{
		ll = R4(sx) * sy;
		m_pObj->edp = (LPBYTE)calloc(1, ll);
		if ( ! m_pObj->edp )
			return;
	}
	// Modified by Peter on [8/8/2005]
	if( LaueZone::ZONE_INDEXED & m_pCurLZ->m_nFlags )
//	if(m_flags & ELR_DONE)
	// End of modification [8/8/2005]
	{
//		ax = 0.5 * (m_h_dir.x + m_k_dir.x);
//		ay = 0.5 * (m_h_dir.y + m_k_dir.y);
		ax = 0.5 * (m_h_dir.x + m_k_dir.x);
		ay = 0.5 * (m_h_dir.y + m_k_dir.y);
		km = AbsC(ax, ay);
//		if( (a = AbsC(ax - m_h_dir.x, ay - m_h_dir.y)) > km )
		if( (a = AbsC(ax - m_h_dir.x, ay - m_h_dir.y)) > km )
			km = a;
		if( (km /= 31) > 1 )
			km = 1.0;
	}
	if(!(m_flags & MAP_EX))
	{
		m_flags |= MAP_EX;
		if(!IsDlgButtonChecked(hDlg,IDC_ELD_MAP))
			memset(m_map, 72, 4096);
		Map_To_IMG ( m_map, m_pObj->edp, sx, sy, km);
	}
	pntBitmapToScreen ( hw, m_pObj->edp, sx, sy, 0, 0, 1, 1, m_pObj, 0x1100 );
//--------------------------------------------------------------------------
	// Modified by Peter on [8/8/2005]
	if( LaueZone::ZONE_INDEXED & m_pCurLZ->m_nFlags )
//	if(m_flags & ELR_DONE)
	// End of modification [8/8/2005]
	{
		if( IsDlgButtonChecked(hDlg, IDC_ELD_MAP) )
		{
			ax =  sx / 64. / km;
			ay = -sy / 64. / km;
			for(i=0; i<13; i++)
			{
//				m_ppt[i].x = round(ax*(ih[i]*HX+ik[i]*KX) + sx*.5  );
//				m_ppt[i].y = sy - round(ay*(ih[i]*HY+ik[i]*KY) + sy*.5  );
				m_ppt[i].x =	   round(ax*(ih[i]*m_h_dir.x + ik[i]*m_k_dir.x) + sx*.5);
				m_ppt[i].y = sy - round(ay*(ih[i]*m_h_dir.y + ik[i]*m_k_dir.y) + sy*.5);
//				m_ppt[i].y = round(ay*(ih[i]*HY+ik[i]*KY) + sy*.5  );
			}
			Poly_Line(hdc, GrayPen, m_ppt,   5);
			Poly_Line(hdc, RedPen,  m_ppt+5, 2);
			Poly_Line(hdc, BluePen, m_ppt+7, 2);
		}
//--------------------------------------------------------------------------
		m_wx0 = 0.10*sx; m_wy0 = 0.1*sy;
		m_wx1 = 0.96*sx; m_wy1 = 0.9*sy;
		if(IsDlgButtonChecked( hDlg, IDC_ELD_ZX)) n = m_rma; else n = m_rms;
		if(IsDlgButtonChecked( hDlg, IDC_ELD_ZY)) m_zy = 4; else m_zy = 1;
		if(m_flags & SH1_EX)
		{
			for(a=0.1, x=0, i=0; i<=10; i++, x+= 5)
			{
				if(i%5==0) p=3; else p=0;
				FVLine_To(hdc, PEN[p], this, x/n, -a, 1.+a);
				a=0;
			}
			for(y=0, i=0; i<=10; i++, y+= 0.1)
			{
				if(i%5==0) p=3; else p=0;
				if(i%10==0) a = 0.1; else a=0;
				FHLine_To(hdc, PEN[p], this, -a, y, 1.+a);
			}
			x = (float)m_r0/n;
			ax = (float)m_r1/n;
			gg = m_kf; // =1

// 			y  = CGauss(m_sg,m_r0)*gg/m_kf;
// 			ay = CGauss(m_sg, m_r1)*gg/m_kf;
			y  = CGauss(m_sg,m_r0/*, m_ksg, m_ws*/)*gg/m_kf;
			ay = CGauss(m_sg, m_r1/*, m_ksg, m_ws*/)*gg/m_kf;
			FLine_To(hdc, BluePen, this, x,  y, ax, ay);
			FCircle (hdc, BluePen, this, x,  y, 0.05);
			FCircle (hdc, BluePen, this, ax,ay, 0.05);
//----------------------------------------------------------------------
//			FPoly_Line(hdc, magenta_pen,this, m_as, n, 1., -R->bg);
//			FPoly_Line(hdc, GrayPen,   this, m_as, n, 1./(m_as[0]-R->bg),-R->bg);
//			FPoly_Line(hdc, yellow_pen, this, m_as, n, m_kf, m_gr1/m_kf - m_as[m_r1]);
//			FPoly_Line(hdc, RedPen,    this, ftmp,  n, 1., 0.);

			for(i=0; i<=m_rms; i++)
			{
				ftmp[i] = XGauss(m_sg, i/*, m_ksg0, m_wn*/);
// 				ftmp[i] = XGauss(m_sg, i);
			}
			FPoly_Line(hdc, GrayPen,    this, ftmp,  n, gg/m_kf, 0.);

			FPoly_Line(hdc, YellowPen, this, m_as, n, gg, m_gr1/m_kf - m_as[m_r1]);

			for(i=0; i<=m_rms; i++)
			{
// 				ftmp[i] = CGauss(m_sg, i);
				ftmp[i] = CGauss(m_sg, i/*, m_ksg, m_ws*/);
			}
			FPoly_Line(hdc, RedPen,    this, ftmp,  n, gg/m_kf, 0.);

//----------------------------------------------------------------------
		}
	}
	DeleteObject(hr);
	SelectPen( hdc, hop );
	SelectBrush( hdc, ob );
	ReleaseDC(hw,hdc);
}
/****************************************************************************/

template <typename _Tx>
class PeakSearch_t
{
	typedef _Tx type;
	typedef type* pointer;
	typedef const type* const_pointer;

	const_pointer pData;
	int width;
	int height;
	int stride;
	float radius;
	IMAGE& m_image;
// 	type nMin;
// 	type nMax;

	__forceinline const_pointer MakePtr(int x, int y)
	{
		if (x > width)
			return NULL;
		if (y > height)
			return NULL;
		return const_pointer(((LPBYTE)pData) + stride * y + x * sizeof(type));
	}

	__forceinline const_pointer ShiftPtr(const_pointer ptr)
	{
		return const_pointer(((LPBYTE)ptr) + stride);
	}

public:
	PeakSearch_t(IMAGE& inImage, float inRadius)
		: m_image(inImage)
		, pData((_Tx*)inImage.pData)
	{
		width = m_image.w;
		height = m_image.h;
		stride = m_image.stride;

		radius = inRadius;
// 		nMin = *std::min_element(pSrc, pSrc + nSizeX*nSizeY);
// 		nMax = *std::max_element(pSrc, pSrc + nSizeX*nSizeY);
	}

	void ClipBox(int nX, int nY, int &x0, int &y0, int &x1, int &y1)
	{
		x0 = __max(nX - radius, 0);
		y0 = __max(nY - radius, 0);
		x1 = __min(nX + radius, width  - 1);
		y1 = __min(nY + radius, height - 1);
	}

	type GetMaxX(int nX, int nY, int &x_max, int &y_max)
	{
		const_pointer pSrc;
		int x, y, i, x0, y0, x1, y1;
		type nmax;

		ClipBox(nX, nY, x0, y0, x1, y1);
		x_max = x0;
		y_max = y0;
// 		nmax = nMin;
		pSrc = MakePtr(x0, y0);
		if (pSrc == NULL)
			throw std::exception("Failed to run PeakSearch_t::GetMaxY, invalid coordinates");
		nmax = *pSrc;
		for(y = y0; y <= y1; y++)
		{
			for(i = 0, x = x0; x <= x1; x++, i++)
			{
				if( nmax < pSrc[i] )
				{
					nmax = pSrc[i];
					x_max = x;
					y_max = y;
				}
			}
			pSrc = ShiftPtr(pSrc);
		}
		return nmax;
	}

	double GetMasscenter(int nX, int nY, double &x_max, double &y_max, const type &Thresh)
	{
		const_pointer pSrc;
		int x, y, i, x0, y0, x1, y1;
		double sum1 = 0.0, sum2 = 0.0, sum = 0.0;
		type nVal;

		ClipBox(nX, nY, x0, y0, x1, y1);
		x_max = x0;
		y_max = y0;
		pSrc = MakePtr(x0, y0);
		if (pSrc == NULL)
			throw std::exception("Failed to run PeakSearch_t::GetMassCenter, invalid coordinates");
		for(y = y0; y <= y1; y++)
		{
			for(i = 0, x = x0; x <= x1; x++, i++)
			{
				nVal = pSrc[i];
				if( nVal < Thresh ) { continue; }
				sum1 += x * double(nVal);
				sum2 += y * double(nVal);
				sum  += nVal;
			}
			pSrc = ShiftPtr(pSrc);
		}
		x_max = (0.0 != sum) ? sum1 / sum : 0.0;
		y_max = (0.0 != sum) ? sum2 / sum : 0.0;
		return (0.0 == sum) ? 0.0 : sum;
	}

	class Stats
	{
	public:
		Stats()
			: m_mean(0)
			, m_stdev(0)
			, m_max(0)
			, m_min(0)
			, m_max_x(0)
			, m_max_y(0)
		{
		}
		double m_mean;
		double m_stdev;
		type m_max;
		type m_min;
		int m_max_x;
		int m_max_y;
	};

	Stats GetStat(int nX, int nY)
	{
		Stats result;
		int x, y, i, cnt, x0, y0, x1, y1;
		double dSum = 0.0;

		result.m_mean = result.m_stdev = 0.0;
		ClipBox(nX, nY, x0, y0, x1, y1);
		const_pointer pSrc = MakePtr(x0, y0);
		if (pSrc == NULL)
			return result;
		// get mean
		result.m_max = result.m_min = pSrc[0];
		for(y = y0, cnt = 0; y <= y1; y++)
		{
			for(i = 0, x = x0; x <= x1; x++, i++, cnt++)
			{
				dSum += pSrc[i];
				if (pSrc[i] > result.m_max)
				{
					result.m_max = pSrc[i];
					result.m_max_x = x;
					result.m_max_y = y;
				}
				if (pSrc[i] < result.m_min)
				{
					result.m_min = pSrc[i];
				}
			}
			pSrc = ShiftPtr(pSrc);
		}
		if( 0 == cnt )
		{
			return result;
		}
		result.m_mean = dSum / cnt;
		// get stdev
		pSrc = MakePtr(x0, y0);
		// get mean
		for(y = y0, cnt = 0; y <= y1; y++)
		{
			for(i = 0, x = x0; x <= x1; x++, i++, cnt++)
			{
				dSum += SQR(pSrc[i] - result.m_mean);
			}
			pSrc = ShiftPtr(pSrc);
		}
		result.m_stdev = FastSqrt(dSum / cnt);
		return result;
	}
/*
	bool OldPeakCenter(int r, float * xm, float * ym, float cd, float thr, int mode, LPWORD amp)
	{
	LPELD S = (LPELD) &O->dummy;
	int		xc = round(*xm), yc = round(*ym), a=0, x, y;
	float	xm0 = *xm, ym0 = *ym, s0=(r+r+1)*(r+r+1), ax, ay, as, s;
	type	am;
	int		al, flg=0;

#define		MAX		S->max
#define		MIN		S->min
#define		IMG		S->img
#define		IMGW	S->imgw
#define		IMGH	S->imgh
#define		NEG		S->neg
#define		IMGINFO	IMG, IMGW, IMGH, NEG

		if((am = GetMaxX(IMGINFO, xc,yc,r,&x,&y)) < 0 ) return -1;
		if(mode) {xc=x; yc=y;}
		if(GetHisX(IMGINFO, xc,yc,r,tmp) < 0 ) return -1;
		for(al=am; al>=0; al--) if( (a += tmp[al]) > 10) break;
		if(al>SCA(5)) al -= SCA(4);
		if((s=(float)GetAXYX(IMGINFO, xc,yc,r,al,&ax,&ay,&as)) < 0 ) return -1;
		if (amp) *amp=am;
		if(as < 1.) return ECN;
		*xm = ax/as; *ym = ay/as;
		if((s/s0) > thr) flg |= ECB;
		if(AbsC(*xm-xm0, *ym-ym0) > cd) flg |= ECD;
		return flg;
	}*/


	bool PeakReflection(double nX, double nY, double &x_peak, double &y_peak, double &dAmpl, double &sum, double dErr)
	{
		int old_rad;
		double threshold, threshold2;

		// some initial values
		x_peak = nX;
		y_peak = nY;
		dAmpl = 0.0;
		sum = 0.0;
		// get maximum in the box and its coordinates
		Stats stats = GetStat((int)nX, (int)nY);

		// too far from the initial point?
		if( SQR(nX - stats.m_max_x) + SQR(nY - stats.m_max_y) >= SQR(radius) )
		{
			return false;
		}
		// rescale to get the real threshold value
		threshold = std::max(stats.m_mean + stats.m_stdev * dErr, 15.0);
		threshold2 = std::max(1.0, (m_image.m_dMax-m_image.m_dMin)/100);
		//Trace((boost::format("%1% %2% %3%") % m_image.m_dMax % m_image.m_dMin % threshold2).str().c_str());
		if( threshold > stats.m_max)
		{
			if (stats.m_max_x == 252 && stats.m_max_y == 208)
			{
				Trace(Format("PeakReflection rejection 1: %1% %2%, %3% %4%") % nX % nY % stats.m_max_x % stats.m_max_y);
			}

			return false;
		}

		if (stats.m_max-stats.m_min < threshold2)
		{
			if (stats.m_max_x == 252 && stats.m_max_y == 208)
			{
				Trace(Format("PeakReflection rejection 2: %1% %2%, %3% %4%") % nX % nY % stats.m_max_x % stats.m_max_y);
			}

			return false;
		}
		// shrink the integration area twice
		old_rad = radius;
		radius /= 2;
		sum = GetMasscenter((int)stats.m_max_x, (int)stats.m_max_y, x_peak, y_peak, threshold);
		// restore the area
		radius = old_rad;
		// too far from the initial point?
		if( SQR(nX - x_peak) + SQR(nY - y_peak) > SQR(radius) )
		{
			if (stats.m_max_x == 252 && stats.m_max_y == 208)
			{
				Trace(Format("PeakReflection rejection 3: %1% %2%, %3% %4%") % nX % nY % stats.m_max_x % stats.m_max_y);
			}

			return false;
		}
		dAmpl = stats.m_max - stats.m_mean;

		if (stats.m_max_x == 252 && stats.m_max_y == 208)
		{
			Trace(Format("PeakReflection accepted: %1% %2%, %3% %4%") % nX % nY % stats.m_max_x % stats.m_max_y);
		}

		return true;
	}

};

int ELD::PeakCenter2(int r, float &xm, float &ym, float cd, float thr, int mode, double &amp, double &as)
{
	bool res = PeakCenter(r, xm, ym, cd, thr, mode, amp, as);
	// -1 here means 0xFFFFFFF which in turn means that all checks on the return value on this 
	// function, which are bitmask checks, will always succeed since all the bits are set
	return res? 0: -1; 
}

bool ELD::PeakCenter(int r, float &xm, float &ym, float cd, float thr, int mode, double &amp, double &as)
{
	double x_peak, y_peak;//, dAmpl;
	bool res;
	float fRadius = r;//r * 0.5;

	switch( m_image.npix )
	{
	case PIX_BYTE:
		{
			PeakSearch_t<BYTE> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	case PIX_CHAR:
		{
			PeakSearch_t<CHAR> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	case PIX_WORD:
		{
			PeakSearch_t<WORD> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	case PIX_SHORT:
		{
			PeakSearch_t<SHORT> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	case PIX_DWORD:
		{
			PeakSearch_t<DWORD> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	case PIX_LONG:
		{
			PeakSearch_t<LONG> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	case PIX_FLOAT:
		{
			PeakSearch_t<FLOAT> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	case PIX_DOUBLE:
		{
			PeakSearch_t<DOUBLE> srch(m_image, fRadius);
			res = srch.PeakReflection(xm, ym, x_peak, y_peak, amp, as, 3.0);
			break;
		}
	}
	xm = x_peak;
	ym = y_peak;
	return res; 
}
/****************************************************************************/
int ELD::GetReflInfo(EREFL* outReflection, BOOL pass1, BOOL bIntegrate)
{
	LPELD   pEld = this;
	HWND	hDlg = m_pObj->hWnd;
	LPFLOAT	as = pEld->m_as;
	LPFLOAT	ds = pEld->m_ds;
	float	dmax, k0, k1, s0, s1, d, d0, xc0, yc0;
	int		ixl = round(outReflection->xc), iyl = round(outReflection->yc);
	int		i, j, r, rmax, xc, yc, r0, r1;
	double	amean, amin, amplitudeResult, ax, ay, sm;
	WORD	flagResult = 0, wAL;
	LaueZone *pLZ;

	// Added by Peter on [9/8/2005]
	pLZ = pEld->m_pCurLZ;
	// end of addition by Peter [9/8/2005]
	if( pLZ->nEstCount < pLZ->nEstMax )
	{
		pEld->m_bUseFixedSigma = FALSE;
	}
	else
	{
		pEld->m_bUseFixedSigma = TRUE;
		pEld->m_UserSigma = pEld->m_MeanSigma;
	}

	pEld->m_sg = 10;
	pEld->m_kf = 1.;
	pEld->m_gr1 = 0;
	pEld->m_r0 = 0;
	pEld->m_r1 = pEld->m_rms;
	outReflection->flags = outReflection->i_amp = 0;
	outReflection->Amp_bkg = outReflection->a0 = 0;
	outReflection->dEstA = outReflection->ae = 0;

	xc0 = outReflection->xc;
	yc0 = outReflection->yc;

	r = round(0.5*pEld->m_rma);
	s0 = SQR(r+r+1);	// +++ round(0.45*pEld->m_rma);
	if( 0 == GetMeanX(&pEld->m_image, ixl, iyl, r, amean, amin, amplitudeResult) )
		return -1;

	wAL = round((amplitudeResult + 0.1*amean)/1.1);

//	if((amplitudeResult-MIN(pEld) < SCA(pEld,pEld,6)) || (amplitudeResult-amin < SCA(pEld,pEld,5)) || (amplitudeResult-amean < SCA(pEld,pEld,3))) flagResult |= FLAG_TOO_NOISY;
//	if(amplitudeResult-wAL < SCA(pEld,pEld,1))  wAL = (int)amplitudeResult-SCA(pEld,pEld,1);
//	if(amplitudeResult-wAL > SCA(pEld,pEld,10)) wAL = (int)amplitudeResult-SCA(pEld,10);
	if((amplitudeResult - pEld->m_min < 6) || (amplitudeResult-amin < 5) || (amplitudeResult-amean < 3))
		flagResult |= FLAG_TOO_NOISY;
	if(amplitudeResult-wAL < 1)
		wAL = (int)amplitudeResult-1;
	// Commented by Peter on [7/4/2006]
	if(amplitudeResult-wAL > 10)
		wAL = (int)amplitudeResult-10;
	// end of comments by Peter [7/4/2006]

	float sigma = (float)GetAXYX(pEld->m_image, ixl, iyl, r/2, wAL, ax, ay, sm);
	if( sigma < 0 )
	{
		return -1;
	}
	if( sigma > 0.1*s0)
	{
//		if((amplitudeResult-amin)/(MAX(pEld)-MIN(pEld)) < 0.3) flagResult |= FLAG_TOO_NOISY;		// +++
//		else
//		if((s=(float)GetAXYX(pEld,IMGINFO(pEld), ixl, iyl, r, wAL, &ax,&ay,&sm)) < 0 )
		if( (sigma = (float)GetAXYX(pEld->m_image, ixl, iyl, r, wAL, ax, ay, sm)) < 0 )
		{
			return -1;
		}
	}

	if(sm < 1.)
	{
		// very low sum of pixels over the average
		outReflection->xc = xc0;
		outReflection->yc = yc0;
		flagResult |= FLAG_UNDEFINED_CENTER;
	}
	else
	{
		outReflection->xc = ax/sm;
		outReflection->yc = ay/sm;
	}
	// the distance between the lattice point and the peak mass center
	double dDistance = AbsC(outReflection->xc - xc0, outReflection->yc - yc0);
	// Modified by Peter 11 August 2009
	double dMinCell = __min(pLZ->a_length, pLZ->b_length);
	if( dDistance > dMinCell * 0.20 )
		flagResult |= FLAG_UNDEFINED_CENTER;		// 20% error -> undefined center
	if( dDistance > dMinCell * 0.15 )
		flagResult |= FLAG_MISPLACED;				// 15% error -> misplaced
// 	if( dDistance > pEld->m_rma/8. )
// 		flagResult |= FLAG_UNDEFINED_CENTER;
// 	if( dDistance > pEld->m_rma/16.)
// 		flagResult |= FLAG_MISPLACED;
	// End of modification by Peter 11 August 2009

	// the intensity ratio
	d = (float)(amplitudeResult - pEld->m_min) / (float)(pEld->m_max - pEld->m_min);
	if(sigma/s0 > 0.5*(d+0.2))
		flagResult |= FLAG_FLAT_CENTER;
//----------------------------------------------------------------------
//	if( GetShapeX(IMGINFO(pEld), pEld->m_min, 0.125/(pEld->m_max-pEld->m_min),
//			round(outReflection->xc), round(outReflection->yc), pEld->m_mask, pEld->m_xms, pEld->m_yms) < 0 )
//	{
//		return -1;
//	}
	if( FALSE == bIntegrate )
	{
		// TODO: add support for signed data
		switch( m_image.npix )
		{
		case PIX_BYTE:
		case PIX_CHAR:
			i = GetShape8_asm(pEld, 0.125/(pEld->m_max - pEld->m_min), round(outReflection->xc), round(outReflection->yc));
			break;
		case PIX_WORD:
		case PIX_SHORT:
			i = GetShape16_asm(pEld, 0.125/(pEld->m_max - pEld->m_min), round(outReflection->xc), round(outReflection->yc));
			break;
		case PIX_DWORD:
		case PIX_LONG:
			i = GetShape32_asm(pEld, 0.125/(pEld->m_max - pEld->m_min), round(outReflection->xc), round(outReflection->yc));
			break;
		default:
			// unsupported pixel format
			return -1;
			break;
		}
		if( i < 0 )
			return -1;
		/*
		if( (PIX_BYTE == m_image.npix) )
		{
			if( i < 0 )
				return -1;
		}
		else if( (PIX_WORD == pEld->m_image.npix) )
		{
			i = GetShape16_asm(pEld, 0.125/(pEld->m_max - pEld->m_min), round(outReflection->xc), round(outReflection->yc));
			if( i < 0 )
				return -1;
		}
		else if( (PIX_DWORD == pEld->m_image.npix) || (PIX_LONG == pEld->m_image.npix) )
		{
			i = GetShape32_asm(pEld, 0.125/(pEld->m_max - pEld->m_min), round(outReflection->xc), round(outReflection->yc));
			if( i < 0 )
				return -1;
		}
		*/
	}
//	float *pOldAs = as, *pOldDs = ds;
	float *max_ptr = std::max_element(ds+1, ds+pEld->m_rma+2);
	dmax = *max_ptr;
	rmax = max_ptr - ds;
/*	{
		float max_val = *std::max_element(as, as + pEld->m_rms + 1);
		for(i = 0; i < 256; i++)
		{
			as[i] *= 1.0 / max_val;
		}
	}
/*
	{
		float *max_ptr = std::max_element(as + 1, as + pEld->m_rma + 1);
		int nrmax = max_ptr - as;
		for(i = 0; i < nrmax; i++)
		{
			as[i] = as[nrmax];
			ds[i] = 0.0f;
		}
	}
*/
//	as += rmax;
//	ds += rmax;
//	if (rmax==1) return -1;

//	outReflection->a0 = 0.3*as[0] + 0.7*as[1];
	outReflection->a0 = as[0];
	if (outReflection->a0 < 1e-3)
		outReflection->a0 = (amplitudeResult-amean/5.)/256.;
	if (outReflection->a0 < 0.)
		outReflection->a0 = 0;
	i = 0;
	d = -1;
	for(j = 250; j <= 254; j++)
	{
		if( as[j] > d )
		{
			d = as[j];
			i = j;
		}
	}
	as[i] = 0;
	d = -1, d0 = 0;
	for(j = 250; j <= 254; j++)
	{
		d0 += as[j];
		if( as[j] > d )
			d = as[j];
	}
//	outReflection->bg = d0 + d*3.;
	outReflection->bg = 2.0 * d0;
	if( (outReflection->a0 -= outReflection->bg) < 0.01 )
	{
		outReflection->a0 = 0;
		flagResult |= FLAG_BELOW_BACKGROUND;
	}
	outReflection->ae = outReflection->a0;

	//int iIntegrate = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_ELD_INTENSITY_TYPE));
// 	if( 1 == iIntegrate )
	if( ELD_INTEGRATE_COMMON_PROFILE == pEld->m_nIntegrationMode )
	{
		pEld->m_sg = 100;
		pEld->m_kf = 1.;
		pEld->m_gr1 = 0;
		goto eee;
	}
//----------------------------------------------------------------------
	if(dmax < 0.0005)
		flagResult |= FLAG_UNDEFINED_SHAPE_DERIVATIVE;
	for(sigma = d = 0., j=0; j<=pEld->m_rma; j++)
	{
		sigma += (as[j] - as[pEld->m_rma]) * (j+1);
		d += j+1;
	}
	if( sigma/d < 0.0001)
		flagResult |= FLAG_BELOW_BACKGROUND;

	d = 0.001*dmax;
	sigma = outReflection->bg + outReflection->a0/50.;

	for(yc = rmax; yc<pEld->m_rms-5; yc++)
	{
		if((as[yc] < sigma ) || (ds[yc] < d))
			break;
	}
	s0 = pEld->m_athr - pEld->m_athd;
	s1 = pEld->m_athr + pEld->m_athd;
	xc = 0;
	for(r0=xc; r0<= yc; r0++)
	{
		if((as[r0]>= s0) && (as[r0]<= s1))
			break;
	}
	for(r1=yc; r1>= xc; r1--)
	{
		if((as[r1]>= s0) && (as[r1]<= s1))
			break;
	}
	for(j=r0; j<=r1; j++)
	{
		if((as[j] <  s0) && (as[j] >  s1))
			flagResult |= FLAG_BAD_SHAPE;
	}
	if((r1-r0) < 3)
	{
		//Trace(_FMT(_T("foo1 %1% %2%") % r1 % r0);
		goto eee;	// was 3
	}
//----------------------------------------------------------------------

	if( 0 == (flagResult & (FLAG_FLAT_CENTER | FLAG_UNDEFINED_CENTER | FLAG_BAD_SHAPE | FLAG_BELOW_BACKGROUND | FLAG_TOO_NOISY | FLAG_UNDEFINED_SHAPE_DERIVATIVE)) )
	{
		ay = as[pEld->m_r1 = r1];
		ax = as[pEld->m_r0 = r0] - ay;
		for(s0=ax*.5, i=r0+1; i<r1; i++)
			s0 += as[i] - ay;
		if(ax < 0.001)
		{
			//Trace(_T("foo2"));
			goto eee;
		}
		k0 = s0 / ax;
		if(k0 < 0.001)
		{
			//Trace(_T("foo3"));
			goto eee;
		}
		// commented by Peter, 6 Aug 2009, these variables are global, and used in a bad way (same names as member variables of ELD)
// 		g_gptr = pEld->m_ws;
// 		g_sptr = pEld->m_wn;
// 		g_RMS  = pEld->m_rms;
// 		g_ksg = pEld->m_ksg;
// 		g_ksg0 = pEld->m_ksg0;

		if ( pEld->m_bUseFixedSigma )
		{
			sigma = pEld->m_UserSigma;
			ay = CGauss(sigma, r1/*, pEld->m_ksg, pEld->m_ws*/);
			ax = CGauss(sigma, r0/*, pEld->m_ksg, pEld->m_ws*/) - ay;
// 			ay = CGauss(sigma, r1);
// 			ax = CGauss(sigma, r0) - ay;
			for(s1 = ax*.5, j=r0+1; j<r1; j++)
			{
				s1 += CGauss(sigma, j/*, pEld->m_ksg, pEld->m_ws*/) - ay;
// 				s1 += CGauss(sigma, j) - ay;
			}
		}
		else
		{
			for(sigma = 0.1, i=0; i<18; i++)
			{
				ay = CGauss(sigma, r1/*, pEld->m_ksg, pEld->m_ws*/);
				ax = CGauss(sigma, r0/*, pEld->m_ksg, pEld->m_ws*/) - ay;
// 				ay = CGauss(sigma, r1);
// 				ax = CGauss(sigma, r0) - ay;
				for(s1=ax*.5, j=r0+1; j<r1; j++)
				{
					s1 += CGauss(sigma, j/*, pEld->m_ksg, pEld->m_ws*/) - ay;
// 					s1 += CGauss(sigma, j) - ay;
				}
				if (ax<1e-6)
				{
					//Trace(_FMT(_T("foo4 %1% %2% %3%") % ax % sigma % r0);
					goto eee;
				}
				k1 = s1 / ax;
				if(k1 < 0.0001)
				{
					//Trace(_T("foo5"));
					goto eee;
				}
				sigma *= (d = FastSqrt(k1 / k0));
				if( (d=fabs(d - 1.)) < 0.0003)
					break;
// 				if( (sigma*g_ksg*r1>2000) || (sigma<0.01) )
				if( (sigma*m_ksg*r1>2000) || (sigma<0.01) )
				{
					//Trace(_FMT(_T("foo6 %1% %2% %3% %4%") % sigma % ksg % r1 % (sigma*m_ksg*r1));
					goto eee;
				}
			}
		}
		if(s1 < 0.0001)
		{
			//Trace(_T("foo7"));
			goto eee;
		}

		if ( d>0.05 )
		{
			//Trace(_T("foo8"));
			goto eee;
		}
		if ((d=s0/s1)<.005)
		{
			//Trace(_T("foo9"));
			goto eee;
		}
        if  (d       >100.)
		{
			//Trace(_T("fooA"));
			goto eee;
		}
		pEld->m_sg = sigma;
		pEld->m_kf = s1 / s0;
		pEld->m_gr1 = CGauss(sigma, r1/*, pEld->m_ksg, pEld->m_ws*/);
// 		pEld->m_gr1 = CGauss(sigma, r1);
		outReflection->ae = d;

		sigma = outReflection->ae/sigma/sigma;
		if((outReflection->a0 > 0.1) && (outReflection->bg < 0.5) && (r1-r0)>5)
		{
			if (pass1)
			{
				for(i=r0; i<(2*rmax+r1)/3; i++)
				{
					pEld->m_wa[i] += ds[i]/sigma;
					pEld->m_wa[i+256] += 1;
				}
			}
			else
			{
				for(i=r0; i<=r1; i++)
				{
					pEld->m_wa[i] += ds[i]/sigma;
					pEld->m_wa[i+256] += 1;
				}
			}
		}
	}
	else
	{
eee:
		d = (as[0]-outReflection->bg) * INT_CUTOFF;	// Cutoff value
		for(i=0,s0=0.,d0=1;i<__min(pEld->m_xms,pEld->m_yms);i++)
		{
			if (i && as[i]>=as[i-1])
				break; // Zero or negative slope, stop integration
			if (as[i]>d || i==0)
			{
				d0 = i;
			}
			else
			{
				d0 = i-1+(as[i-1]-d)/(as[i-1]-as[i]);
				break;
			}
		}
		outReflection->sr = d0;	// Set this reflection radius
		for(i=0,outReflection->ae=0.,d0=0.;i<pEld->m_arad;i++)
		{
			outReflection->ae += (as[i]-outReflection->bg)*(i+.5);	// Calc refl intensity as I(r)*r
			d0 += i+.5;
		}
		if (outReflection->h == 9 && outReflection->k == 1)
		{
			//Trace(_T("..."));
		}

		outReflection->ae /= d0;
		if (outReflection->ae<0.)
			outReflection->ae=0;
		outReflection->ae *= pEld->m_cfoi;
		flagResult |= FLAG_BAD_SHAPE;
	}
	outReflection->sg = pEld->m_sg;
//----------------------------------------------------------------------
	outReflection->i_amp = (int)amplitudeResult;
	outReflection->flags = flagResult;
	// Added by Peter on 26 Nov 2006
// 	outReflection->Amp_bkg = outReflection->a0*256.0;
// 	outReflection->dEstA = outReflection->ae*256.0;
	outReflection->Amp_bkg = outReflection->a0 * (pEld->m_max - pEld->m_min);
	outReflection->dEstA = outReflection->ae * (pEld->m_max - pEld->m_min);
	// end of Peters addition on 26 Nov 2006
	return outReflection->flags;
}

/****************************************************************************/
BOOL ELD::Calc2to3d(int &a, int &b, int &c)
{
	int		dt;
	int		u1,v1, u2,v2;
	int		uh, uk, ul;
	int		vh, vk, vl;
	int		h1, k1, l1;
	int		h2, k2, l2;
	LPELD pEld = this;

	u1 = pEld->m_uu[0]; v1 = pEld->m_vv[0];
	u2 = pEld->m_uu[1]; v2 = pEld->m_vv[1];
	h1 = pEld->m_hh[0]; k1 = pEld->m_kk[0]; l1 = pEld->m_ll[0];
	h2 = pEld->m_hh[1]; k2 = pEld->m_kk[1]; l2 = pEld->m_ll[1];

	pEld->m_f2to3 = FALSE;

	dt = u1*v2-u2*v1;

	SetDlgItemText( m_pObj->hWnd, IDC_ELD_ZONEAX, "[Invalid]" );

	if (!IsDlgButtonChecked( m_pObj->hWnd, IDC_ELD_USE3D) ) return FALSE;

	if (dt==0) return FALSE;

	uh = v2*h1-v1*h2; if (uh%dt) return FALSE; uh /= dt;
	uk = v2*k1-v1*k2; if (uk%dt) return FALSE; uk /= dt;
	ul = v2*l1-v1*l2; if (ul%dt) return FALSE; ul /= dt;

	vh = u1*h2-u2*h1; if (vh%dt) return FALSE; vh /= dt;
	vk = u1*k2-u2*k1; if (vk%dt) return FALSE; vk /= dt;
	vl = u1*l2-u2*l1; if (vl%dt) return FALSE; vl /= dt;

	if (abs(uk*vl-vk*ul)+abs(vh*ul-uh*vl)+abs(uh*vk-vh*uk) == 0) return FALSE;

	pEld->m_uh = uh; pEld->m_uk = uk; pEld->m_ul = ul;
	pEld->m_vh = vh; pEld->m_vk = vk; pEld->m_vl = vl;

	pEld->m_f2to3 = TRUE;
	a = uk*vl-vk*ul;
	b = vh*ul-uh*vl;
	c = uh*vk-vh*uk;
	if      (a < 0)            { a = -a; b = -b; c = -c; }
	else if (a == 0 && b < 0)  {         b = -b; c = -c; }
	else if (a == 0 && b == 0 && c < 0 ) {       c = -c; }

	long gcd1 = FindGCD2(a, b);
	long gcd2 = FindGCD2(a, c);
	long gcd3 = FindGCD2(gcd1, gcd2);
	if( abs(gcd3) > 0 )
	{
		a /= (double)gcd3;
		b /= (double)gcd3;
		c /= (double)gcd3;
	}
	sprintf(TMP, "[%d,%d,%d]", a, b, c );
	SetDlgItemText( m_pObj->hWnd, IDC_ELD_ZONEAX, TMP );
	return TRUE;
}
/****************************************************************************/
void ELD::FillReflList(int nLaueZone)
{
	//LPELD	pEld = this;
	HWND	hDlg = m_pObj->hWnd;
	LPEREFL R;
	BOOL	bI = IsDlgButtonChecked(hDlg, IDC_ELD_INTENSITIES);
	int		i, f, n, goodReflections=0, misplacedReflections=0, notEstimatedReflections=0;
	int		h, k, l;//, nLaueZone;
	int		ZA_u, ZA_v, ZA_w;
	float	x,y,a0,ae;
	BOOL	bDo3D, bCalb, bIntn, bFool;
	//char	fmtstr[30]="";
	std::string sFormat, sHeader;
	LaueZone *pLZ;

	bDo3D = m_f2to3;
	bCalb = IsCalibrated();
	bFool = m_bProtocol;
	bIntn = IsDlgButtonChecked(hDlg, IDC_ELD_INTENSITIES);

	// Added by Peter on [8/8/2005]
	pLZ = m_vLaueZones[nLaueZone];//m_pCurLZ;
//	nLaueZone = m_nCurLaueZone;
	// get Laue Zone if exists
	if( TRUE == bDo3D )
	{
		if( FALSE == Calc2to3d( ZA_u, ZA_v, ZA_w ) )
		{
			MessageBox(hDlg, "Failed to calculate Zone Axis.\r\n"
				"Indices will be indexed in [0,0,1] Zone Axis.", "ELD ERROR!",
				MB_ICONERROR | MB_OK);
			ZA_u = ZA_v = 0;
			ZA_w = 1;
		}
	}
	// End of addition [8/8/2005]
	if( !bFool )
	{
		m_ProtocolOut.ResetContent();
// 		SendDlgItemMessage(hDlg, IDC_ELD_LIST, CB_RESETCONTENT, 0, 0);
	}
	m_last = m_ProtocolOut.GetCount();
// 	m_last = SendDlgItemMessage(hDlg, IDC_ELD_LIST, CB_GETCOUNT, 0, 0);
//	if (bCalb)	qsort(m_rptr, m_nrefl, sizeof(EREFL), dval_cmp);
//	else		qsort(m_rptr, m_nrefl, sizeof(EREFL), hk_cmp);
	if( bCalb ) qsort(&*pLZ->m_vRefls.begin(), pLZ->m_vRefls.size(), sizeof(EREFL), dval_cmp);
	else		qsort(&*pLZ->m_vRefls.begin(), pLZ->m_vRefls.size(), sizeof(EREFL), hk_cmp);

// 	AddProtocolString(";");
// 	sprintf(TMP,"; File : %s, estimation pass %d", m_pObj->fname, pLZ->nEstCount);
// 	AddProtocolString(TMP);
// 	AddProtocolString(";");
	m_ProtocolOut.AddProtocolString(";");
	m_ProtocolOut.AddProtocolString(_FMT(_T("; File : %s, estimation pass %d"), m_pObj->fname, pLZ->nEstCount));
	m_ProtocolOut.AddProtocolString(";");

	if( bCalb )
	{
		// TODO: correct Angstrem here
		m_ProtocolOut.AddProtocolString(_FMT(_T("; a=%6.3f%c, b=%6.3f%c, gamma=%6.3f"),
			pLZ->ral, ANGSTR_CHAR, pLZ->rbl, ANGSTR_CHAR, R2D(M_PI - pLZ->gamma)));
// 		sprintf(TMP, "; a=%6.3f%c, b=%6.3f%c, gamma=%6.3f",
// 			pLZ->ral, 197, pLZ->rbl, 197, R2D(M_PI - pLZ->gamma));// degree sign, 176);
//			m_ral,m_rbl,R2D(m_gamma));
	}
	else
	{
		m_ProtocolOut.AddProtocolString(_FMT(_T("; a*=%6.3fpix, b*=%6.3fpix, gamma*=%6.3f"),
			pLZ->a_length, pLZ->b_length, R2D(pLZ->gamma)));
//         sprintf(TMP, "; a*=%6.3fpix, b*=%6.3fpix, gamma*=%6.3f",
// 			pLZ->a_length, pLZ->b_length, R2D(pLZ->gamma));// degree sign, 176);
//			m_al,m_bl,R2D(m_gamma));
	}
	//AddProtocolString(TMP);
	m_ProtocolOut.AddProtocolString(";");

// 	if( bCalb )
// 	{
// 		swprintf((LPWSTR)TMP, L"Cell: %6.3f%c %6.3f%c 10 90 90 %6.3f%c",
// 			pLZ->ral, 197, pLZ->rbl, 197, R2D(M_PI - pLZ->gamma), 176);//			m_ral,m_rbl,R2D(m_gamma));
// 	}

	sFormat = "Format:";
	if (bDo3D) { sHeader = "   h   k   l  "; sFormat += " h k l"; }
	else       { sHeader = "   h   k  ";     sFormat += " h k";   }
	sHeader += " d-val  ";
	sFormat += " d";
	if (bIntn) { sHeader += " Iobs    Iest   ";     sFormat += " s a"; }
	else       { sHeader += " Aobs    Aest   ";     sFormat += " s a"; }
	if (bFool) { sHeader += "dX    dY  Sigma Flg"; sFormat += " x x x x"; }
	m_ProtocolOut.AddProtocolString(sFormat);
	//SetWindowText(GetDlgItem(hDlg, IDC_ELD_LIST_HEADER), TMP);
	m_ProtocolOut.SetHeader(sHeader);
	m_ProtocolOut.AddProtocolString(";" + sHeader);
	m_ProtocolOut.AddProtocolString(";------------------------------------------------");

	// TMP[0] = 0;
	// sprintf(fmtstr, "Format:");
	// if (bDo3D) { sprintf(TMP_End, "; h   k   l  ");       sprintf(&fmtstr[strlen(fmtstr)], " h k l"); }
	// else       { sprintf(TMP_End, "; h   k  ");           sprintf(&fmtstr[strlen(fmtstr)], " h k"); }
//	if (bCalb) { sprintf(TMP_End, "d-val  ");             sprintf(&fmtstr[strlen(fmtstr)], " d"); }
	// sprintf(TMP_End, "d-val  ");
	// sprintf(&fmtstr[strlen(fmtstr)], " d");
	// if (bIntn) { sprintf(TMP_End, "Iobs   Iest    ");     sprintf(&fmtstr[strlen(fmtstr)], " s a"); }
	// else       { sprintf(TMP_End, "Aobs   Aest    ");     sprintf(&fmtstr[strlen(fmtstr)], " s a"); }
	// if (bFool) { sprintf(TMP_End, " dX    dY Sigma Flg"); sprintf(&fmtstr[strlen(fmtstr)], " x x x x"); }
	// AddProtocolString(fmtstr);
	// SetWindowText(GetDlgItem(hDlg, IDC_ELD_LIST_HEADER), TMP);
	// AddProtocolString(TMP);
	// AddProtocolString(";------------------------------------------------");

	//int iIntegrate = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_ELD_INTENSITY_TYPE));

	for(R = &*pLZ->m_vRefls.begin(), i = 0; i < pLZ->m_vRefls.size(); R++, i++)
	{
		x = R->xc - R->xc0;
		y = R->yc - R->yc0;
		n = 0;
		f = R->flags;
// 		if( !IsDlgButtonChecked(m_pObj->hWnd, IDC_ELD_INTEGRATE) )
// 		if( 0 == iIntegrate )
		if( ELD_INTEGRATE_SHAPE_FIT == m_nIntegrationMode )
		{
			if      (f&FLAG_BAD_SHAPE)	{ n = 1; notEstimatedReflections++; }
			else if (f&FLAG_MISPLACED) { n = 2; misplacedReflections++; }
			else            { n = 0; goodReflections++; }
		}
		else
		{
			goodReflections++;
		}
// 		if ( bIntn ) {a0 =      R->a0*256.;  ae =      R->ae*256.; }
// 		else	     {a0 = FastSqrt(R->a0*256.); ae = FastSqrt(R->ae*256.); }
// 		if( (0 == iIntegrate) || (1 == iIntegrate) )
		if( (ELD_INTEGRATE_SHAPE_FIT == m_nIntegrationMode) || (ELD_INTEGRATE_COMMON_PROFILE == m_nIntegrationMode) )
		{
			if ( bIntn ) {a0 =      R->Amp_bkg;  ae =      R->dEstA; }
			else	     {a0 = FastSqrt(R->Amp_bkg); ae = FastSqrt(R->dEstA); }
		}
// 		else if( 2 == iIntegrate )
		else if( ELD_INTEGRATE_SIMPLE_SUMMATION == m_nIntegrationMode )
		{
			if ( bIntn ) {a0 =      R->Amp_bkg;  ae =      R->IntegrA; }
			else	     {a0 = FastSqrt(R->Amp_bkg); ae = FastSqrt(R->IntegrA); }
		}
		if( bDo3D )
		{
			h = R->h*m_uh + R->k*m_vh;
			k = R->h*m_uk + R->k*m_vk;
			l = R->h*m_ul + R->k*m_vl;
			// Added by Peter on [8/8/2005]
			h += nLaueZone * ZA_u;
			k += nLaueZone * ZA_v;
			l += nLaueZone * ZA_w;
			// End of addition [8/8/2005]
		}

		sFormat = " ";
		if (((f&FLAG_BAD_SHAPE)!=0 && m_bNotListY) ||
			((f&FLAG_USER_DELETED)!=0) ||
			(m_dvfrom-1e-10 > R->dval) ||
			(m_dvto  +1e-10 < R->dval) ||
			(m_infrom-1e-6 > R->ae)   ||
			(m_into  +1e-6 < R->ae) )
		{
			sFormat = ";";
		}
		if (bDo3D) sFormat += (_FMT(_T("%3d %3d %3d "), h, k, l));
		else       sFormat += (_FMT(_T("%3d %3d "), R->h, R->k));
		sFormat += (_FMT(_T("%7.3f "), R->dval));
        sFormat += (_FMT(_T("%6.2f %7.2f "), a0, ae ));
		if (bFool) sFormat += (_FMT(_T("%5.2f %5.2f %5.3f %s"), x, y, (0.707/R->sg), s_szFlags[n]));
		m_ProtocolOut.AddProtocolString(sFormat);
		// TMP[0] = 0;
		// if (((f&FLAG_BAD_SHAPE)!=0 && m_bNotListY) ||
		// 	((f&FLAG_USER_DELETED)!=0) ||
		// 	(m_dvfrom-1e-10 > R->dval) ||
		// 	(m_dvto  +1e-10 < R->dval) ||
		// 	(m_infrom-1e-6 > R->ae)   ||
		// 	(m_into  +1e-6 < R->ae) )
		// 		sprintf(TMP_End, ";");
		// else
		// 	sprintf(TMP_End, " ");
		// if (bDo3D) sprintf(TMP_End, "%3d %3d %3d ", h, k, l );
		// else       sprintf(TMP_End, "%3d %3d ", R->h, R->k );
//		if (bCalb) sprintf(TMP_End, "%6.3f ", R->dval );
		// sprintf(TMP_End, "%6.3f ", R->dval );
		// sprintf(TMP_End, "%5.2f %7.2f ", a0, ae );
		// if (bFool) sprintf(TMP_End, "%5.2f %5.2f %5.3f %s", x, y, 0.707/R->sg, s_szFlags[n] );
		// AddProtocolString(TMP);
	}
	if( bFool )
		m_ProtocolOut.AddProtocolString(_FMT(_T("; %d Good, %d Misplaced, %d Not estimated"), goodReflections, misplacedReflections, notEstimatedReflections));
	else
		m_ProtocolOut.AddProtocolString(_FMT(_T("; %d reflections estimated"), pLZ->m_vRefls.size()));
	m_ProtocolOut.SetCurSel(m_ProtocolOut.GetCount() - 1);
// 	SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_SETCURSEL,
// 						(int)SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_GETCOUNT, 0, 0)-1, 0 );
	// if( bFool )
	// 	sprintf(TMP,"; %d Good, %d Misplaced, %d Not estimated", goodReflections, misplacedReflections, notEstimatedReflections);
	// else
	// 	sprintf(TMP,"; %d reflections estimated", pLZ->m_vRefls.size());
	// AddProtocolString(TMP);
	// SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_SETCURSEL,
	// 	(int)SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_GETCOUNT, 0, 0)-1, 0 );
}
/****************************************************************************/

// Adjust the coordinates of the given reflection by averaging the offset of the surrounding points and itself. 
// Puts its result in the xc and yc members
static void CalcSmartCoordinates( EREFL* inReflections, int nrefl, EREFL* outReflection )
{
	int h_left = 0, h_right = 0, k_top = 0, k_bottom = 0;
	
//	if (abs(h)==abs(k)) {
	h_left = outReflection->h - 2;
	h_right = outReflection->h + 2;
	k_top = outReflection->k - 2;
	k_bottom = outReflection->k + 2;
/*	} else {
		if (abs(h)>abs(k)) { //vert
			h_left = h_right = h-h/abs(h);
			k_top = k-1;
			k_bottom = k+1;
		} else {			// horiz
			k_top = k_bottom = k-k/abs(k);
			h_left = h-1;
			h_right = h+1;
		}
	}
*/
	EREFL* reflectionIterator = inReflections;
	float	dx = 0, dy = 0;
	int j = 0;
	for(int i = 0; i < nrefl; i++, reflectionIterator++)
	{
		if( 
			reflectionIterator->h >= h_left && 
			reflectionIterator->h <= h_right && 
			reflectionIterator->k >= k_top && 
			reflectionIterator->k <= k_bottom )
		{
			dx += reflectionIterator->xc - reflectionIterator->xl;
			dy += reflectionIterator->yc - reflectionIterator->yl;
			j++;
		}
	}
	if( j )
	{
		dx /= j;
		dy /= j;
	}
	outReflection->xc = outReflection->xl+dx;
	outReflection->yc = outReflection->yl+dy;
}
/*
template <typename _Tx>
void sloped_projection(_Tx *img, int w, int h)
{
	std::vector<double> proj;
	double Sin, Cos, dAng, dist, dy;
	int w2, h2, i, x, y, off, size, pos;
	_Tx *ptr;
	FILE *pOut;

	w2 = w >> 1;
	h2 = h >> 1;
	size = floor(FastSqrt(w*w + h*h) + 0.5);
	off = size / 2;
	proj.resize(size);
	if( NULL == (pOut = fopen("D:\\Max\\ELD.txt", "wt")) )
		return;
	for(i = 0; i < 180; i++)
	{
		ptr = img;
		dAng = M_PI * double(i) / 180;
		Sin = sin(dAng);
		Cos = cos(dAng);
		std::fill_n(proj.begin(), size, 0.0);
//		sprintf(TMP, "D:\\Max\\prof%d.txt", i);
//		if( NULL == (pOut = fopen(TMP, "wt")) )
//			continue;
		for(y = 0; y < h; y++)
		{
			dy = Sin*(y - h2);
			for(x = 0; x < w; x++, ptr++)
			{
				dist = Cos*(x - w2) + dy;
				pos = (int)floor(dist + 0.5) + off;
				if( pos < 0 || pos >= size )
					continue;
				proj[pos] += *ptr;
			}
		}
		for(x = 0; x < size; x++)
		{
			fprintf(pOut, "%.1f\t", proj[x]);
		}
		fprintf(pOut, "\n");
	}
	fclose(pOut);
}
*/

#if 0
static void IntegrateRefls(OBJ* lpObj)
{
	LPELD lpS = (LPELD) &lpObj->dummy;
	LPEREFL	 lpR;
	HWND	hDlg = lpObj->hWnd;
	float rad, mind;
	float xc0, yc0, bkg, area;
	int i, c, size, num;
	int arad = lpS->arad, orop;
	HDC hDC;
	HPEN hp, op;
	HBRUSH hb, ob;
	HBITMAP hBMP, hOldBMP;
	LPVOID bits = NULL;
	BITMAPINFO	*bb;
	LaueZone *pLZ;

	// Added by Peter on [10/8/2005]
	pLZ = lpS->m_pCurLZ;
	// End of addition [10/8/2005]
	mind = __min(pLZ->a_length, pLZ->b_length);
//	mind = min(lpS->a_length, lpS->b_length);
	rad = 0.5 * mind;

	// TODO: move somewhere else
	// create a mask
	{
		bb = (LPBITMAPINFO)calloc(1, sizeof(BITMAPINFO)+sizeof(RGBQUAD)*256);
		LPBYTE	bipal;

		arad = rad;
		size = (arad + arad + 1);

		bb->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bb->bmiHeader.biPlanes = 1;
		bb->bmiHeader.biBitCount = 8;
		bb->bmiHeader.biCompression = BI_RGB;
		bb->bmiHeader.biClrUsed  = COLORS;
		bb->bmiHeader.biClrImportant = COLORS;
		bb->bmiHeader.biSizeImage = R4(size) * size;
		bb->bmiHeader.biWidth  = size;
		bb->bmiHeader.biHeight = size;
		bipal = (LPBYTE)(bb->bmiColors);
		for(c = 0; c < 256; c++)
		{
			*bipal++ = c;
			*bipal++ = c;
			*bipal++ = c;
			*bipal++ = 0;
		}

		// create the mask for the reflection
		if( NULL == (hDC = CreateCompatibleDC(NULL)) )
			goto errexit;

		if( NULL == (hBMP = CreateDIBSection(hDC, bb, DIB_RGB_COLORS, &bits, NULL, 0)) )
			goto errexit;

		hOldBMP = SelectBitmap( hDC, hBMP );

		hp = CreatePen(PS_SOLID, 1, RGB(255,255,255));
		hb = CreateSolidBrush(RGB(200,200,200));

		orop = SetROP2( hDC, R2_COPYPEN );
		ob = SelectBrush( hDC, hb );
		op = SelectPen( hDC, hp );

		memset(bits, 0, R4(size)*size);

		Ellipse( hDC, 0, 0, size, size );

		SetROP2( hDC, orop );
		SelectObject( hDC, op );
		SelectObject( hDC, ob );
		DeleteObject( hb );
		DeleteObject( hp );
	}
	// added by Peter on 09 Feb 2004
	// output to the window
////////////////////
	typedef std::vector<int> Histo;
	typedef std::vector<Histo> HistoArr;
	typedef std::vector<Histo>::iterator HistoArrIt;
	try
	{
		int		nLaueZone;
		double	Ampl;
		nLaueZone = lpS->nCurLaueZone;
		int nPlotL = SQR(lpS->rma)*M_PI;
		std::vector<DWORD> vPlot(nLevels * nPlotL, 0);
/*
//-->
		int		h, k, l;
		int		ZA_u, ZA_v, ZA_w;
		BOOL	bDo3D = lpS->f2to3;
		BOOL	bCalb = lpS->IsCalibrated();
		BOOL	bIntn = IsDlgButtonChecked(hDlg, IDC_ELD_INTENSITIES);

		if( TRUE == bDo3D )
		{
			if( FALSE == Calc2to3d( lpObj, ZA_u, ZA_v, ZA_w ) )
			{
				MessageBox(hDlg, "Failed to calculate Zone Axis.\r\n"
					"Indices will be indexed in [0,0,1] Zone Axis.", "ELD ERROR!",
					MB_ICONERROR | MB_OK);
				ZA_u = ZA_v = 0;
				ZA_w = 1;
			}
		}
		if( bCalb ) qsort(pLZ->m_vRefls.begin(), pLZ->m_vRefls.size(), sizeof(EREFL), dval_cmp);
		else		qsort(pLZ->m_vRefls.begin(), pLZ->m_vRefls.size(), sizeof(EREFL), hk_cmp);

		sprintf(TMP, ";");
		AddProtocolString(TMP);
		sprintf(TMP, ";File %s, integration radius: %f", lpObj->fname, (float)arad);
		AddProtocolString(TMP);
		sprintf(TMP, ";");
		AddProtocolString(TMP);
		if( bCalb )
		{
			sprintf(TMP, "; a=%6.3fÅ, b=%6.3fÅ, gamma=%6.3f",
				lpS->ral, lpS->rbl, R2D(lpS->gamma));
		}
		else
		{
			sprintf(TMP, "; a*=%6.3fpix, b*=%6.3fpix, gamma*=%6.3f",
				lpS->a_length, lpS->b_length, R2D(lpS->gamma));
		}
		AddProtocolString(TMP);
		AddProtocolString(";");
//		sprintf(TMP, "; a=%.3fA, b=%.3fA, gamma=%.3f", lpS->a_length, lpS->b_length, lpS->gamma*180/3.1415926);
//		AddProtocolString(TMP);
		if( bCalb )
		{
			sprintf(TMP, "Cell: %6.3fÅ %6.3fÅ 10 90 90 %6.3f",
			 lpS->ral, lpS->rbl, R2D(lpS->gamma));
		}
		TMP[0] = 0;
		char fmtstr[30] = "";
		sprintf(fmtstr, "Format:");
		if (bDo3D) { sprintf(TMP_End, "; h   k   l  ");       sprintf(&fmtstr[strlen(fmtstr)], " h k l"); }
		else       { sprintf(TMP_End, "; h   k  ");           sprintf(&fmtstr[strlen(fmtstr)], " h k"); }
		if (bCalb) { sprintf(TMP_End, "d-val  ");             sprintf(&fmtstr[strlen(fmtstr)], " d"); }
		if (bIntn) { sprintf(TMP_End, "Iobs   Iest    ");     sprintf(&fmtstr[strlen(fmtstr)], " s a"); }
		else       { sprintf(TMP_End, "Aobs   Aest    ");     sprintf(&fmtstr[strlen(fmtstr)], " s a"); }
		if( lpS->bProtocol )
		{
			sprintf(TMP_End, " dX    dY Sigma Flg");
			sprintf(&fmtstr[strlen(fmtstr)], " x x x x");
		}
		AddProtocolString(fmtstr);
		AddProtocolString(TMP);
		AddProtocolString(";------------------------------------------------");
*/
/*
		sprintf(TMP, ";");
		AddProtocolString(TMP);
		sprintf(TMP, "Format: h k d s a");
		AddProtocolString(TMP);
		sprintf(TMP, "; h k d-val Iobs Iest");
		AddProtocolString(TMP);
		sprintf(TMP, ";----------------------------------------");
		AddProtocolString(TMP);
*/
		// some statistics
		lpS->infrom = 1e20;
		lpS->into = -1e20;
		lpS->bCentOnly = FALSE;
		lpS->meanampl = 0;
		lpS->dvfrom = 1e20;
		lpS->dvto   = -1e20;
#ifdef _DEBUG
		FILE *pOutF = fopen("d:\\working\\eld\\intern_cal.txt", "wt");
#endif
		Histo histo;
		bool bFlag;
		for(i = 0, lpR = pLZ->m_vRefls.begin(); i < pLZ->m_vRefls.size(); i++, lpR++)
		{
			// reflection center
			xc0 = lpR->xc0;
			yc0 = lpR->yc0;
			Ampl = 0.0;
			// sum all points on that circle - it will be a background
			if( 1 == lpS->pix )
			{
				bkg = sum_along_circle<BYTE>((BYTE*)lpS->img, lpS->m_image.w, lpS->m_image.h, xc0, yc0, rad);
				area = sum_inside_circle<BYTE>((BYTE*)lpS->img, lpS->m_image.w, lpS->m_image.h, xc0, yc0, arad,
					(LPBYTE)bits, R4(size), size, num);
				if( num > 0 )
				{
					bFlag = ProcessSingleRefl<BYTE>((BYTE*)lpS->img, lpS->m_image.w, lpS->m_image.h, xc0, yc0, arad,
						(LPBYTE)bits, R4(size), size, bkg, histo, Ampl);
				}
			}
			else if( 2 == lpS->pix )
			{
				bkg = sum_along_circle<WORD>((WORD*)lpS->img, lpS->m_image.w, lpS->m_image.h, xc0, yc0, rad);
				area = sum_inside_circle<WORD>((WORD*)lpS->img, lpS->m_image.w, lpS->m_image.h, xc0, yc0, arad,
					(LPBYTE)bits, R4(size), size, num);
				if( num > 0 )
				{
					bFlag = ProcessSingleRefl<WORD>((WORD*)lpS->img, lpS->m_image.w, lpS->m_image.h, xc0, yc0, arad,
						(LPBYTE)bits, R4(size), size, bkg, histo, Ampl);
				}
			}
#ifdef _DEBUG
			if( (NULL != pOutF) && (true == bFlag) )
			{
				for(int j = 0; j < histo.size(); j++)
				{
					fprintf(pOutF, "%d\t", histo[j]);
					vPlot[histo[j]*nLevels + j]++;
				}
				fprintf(pOutF, "%d\t%d\t%d\n", lpR->h, lpR->k, (int)area);
			}
#endif
/*
//-->
			if( bDo3D )
			{
				h = lpR->h*lpS->uh + lpR->k*lpS->vh;
				k = lpR->h*lpS->uk + lpR->k*lpS->vk;
				l = lpR->h*lpS->ul + lpR->k*lpS->vl;
				// Added by Peter on [8/8/2005]
				h += nLaueZone * ZA_u;
				k += nLaueZone * ZA_v;
				l += nLaueZone * ZA_w;
				// End of addition [8/8/2005]
			}
*/
			lpR->ae = lpR->a0 = Ampl = (area - num*bkg < 0.0) ? 0.0 : area/(float)num-bkg;
			if( lpS->dvfrom > lpR->dval )
				lpS->dvfrom = lpR->dval;
			if( lpS->dvto   < lpR->dval )
				lpS->dvto   = lpR->dval;
			if( lpS->infrom > lpR->a0 )
				lpS->infrom = lpR->a0;
			if( lpS->into   < lpR->a0 )
				lpS->into   = lpR->a0;
/*
//-->
			TMP[0] = 0;
			float fX = lpR->xc - lpR->xc0;
			float fY = lpR->yc - lpR->yc0;
			if( (lpS->bNotListY) ||
				(lpS->dvfrom-1e-10 > lpR->dval) ||
				(lpS->dvto  +1e-10 < lpR->dval) ||
				(lpS->infrom-1e-10 > lpR->ae)   ||
				(lpS->into  +1e-10 < lpR->ae) )
					sprintf(TMP_End, ";");
			else    sprintf(TMP_End, " ");
			if (bDo3D) sprintf(TMP_End, "%3d %3d %3d ", h, k, l );
			else       sprintf(TMP_End, "%3d %3d ", lpR->h, lpR->k );
			if (bCalb) sprintf(TMP_End, "%6.3f ", lpR->dval );
					   sprintf(TMP_End, "%5.1f %7.2f ", Ampl, Ampl );
			if( lpS->bProtocol )
				sprintf(TMP_End, "%5.2f %5.2f", fX, fY);
//			sprintf(TMP, "%4d %4d %10.3f %10.3f %10.3f",
//				lpR->h, lpR->k, lpR->dval, Ampl, Ampl);
			AddProtocolString(TMP);
*/
			// 256 is some scale by unknown & strange reason
			if( lpS->infrom > Ampl/256.0)	lpS->infrom = Ampl / 256.0;
			if( lpS->into   < Ampl/256.0)	lpS->into   = Ampl / 256.0;
			lpS->meanampl +=  Ampl;
		}
		FillReflList(lpObj, lpS->nCurLaueZone);
#ifdef _DEBUG
		if( NULL != pOutF )
		{
			fprintf(pOutF, "---\n");
			for(int i = 0; i < nPlotL; i++)
			{
				for(int j = 0; j < nLevels; j++)
				{
					fprintf(pOutF, "%d\t", vPlot[i*nLevels + j]);
				}
				fprintf(pOutF, "\n");
			}
			fclose(pOutF);
		}
#endif
/*
//-->
		sprintf(TMP,"; %d reflections estimated", pLZ->m_vRefls.size());
		AddProtocolString(TMP);
		if( false == pLZ->m_vRefls.empty() )
			lpS->meanampl /= pLZ->m_vRefls.size();
		// set the text
		SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_SETCURSEL,
			(int)SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_GETCOUNT, 0, 0)-1, 0 );
*/
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("EXCEPTION -> Internal calibration calculations failed on %n-th reflection: %d\n"), i, e.what()));
	}
errexit:
	// clean up
	SelectBitmap( hDC, hOldBMP );
	DeleteObject( hBMP );
	DeleteDC( hDC );
	free(bb);
}
#endif

/****************************************************************************/

int ELD::ExtractRefls(float rmin, float rmax, int nLaueZone)
{
	EldReflVecIt rit;
	LPELD	pEld = this;
	float	s, x, y, d, sgmin=100, sgmax=0, sgmean = 0, sa0, sae;
	int		i, h, k, f, pn, hkm = pEld->m_hkmaxe, r;
	int		nmin = 0, nmax = 0, nmean = 0, noldrefl = 0;
	float	cf = m_pObj->NI.AspectXY;
	BOOL	bCentered = IsDlgButtonChecked( m_pObj->hWnd, IDC_ELD_CENTERED );
	static	float sigma = 1.0;
	int nOldRefls = 0;//, nLaueZone;
	EREFL refl;
	double dMinR = 0, dMaxR = 1e33, dMinR2, dMaxR2, dDist2;
	BOOL bCheckR;
	LaueZone *pLZ;
	CVector2d	point;

	pLZ = pEld->m_vLaueZones[nLaueZone];
	bCheckR = FALSE;
	if( TRUE == pEld->m_bUseLaueCircles )
	{
		if( true == pEld->m_vbLaueCircles[nLaueZone] )
		{
			dMaxR = pEld->m_vLaueZones[nLaueZone]->m_dRadius;
			if( nLaueZone > 0 )
			{
				if( true == pEld->m_vbLaueCircles[nLaueZone-1] )
				{
					dMinR = pEld->m_vLaueZones[nLaueZone-1]->m_dRadius;
				}
			}
			bCheckR = TRUE;
		}
	}

	pEld->m_bUseFixedSigma = FALSE;
	pLZ->nEstCount++;
	sprintf(TMP, "Pass %d", pLZ->nEstCount );
	SetDlgItemText(m_pObj->hWnd, IDC_ELD_PASSINFO, TMP);
	// First pass
	if( 1 == pLZ->nEstCount )
	{
		pEld->m_cfoi = FastSqrt(1 + INT_CUTOFF);
		for(d=0, i=0; i<2048; i++, d+=0.02)
		{
			if(SGauss(sigma,d) < 1e-4)
				break;
		}
		pEld->m_ksg = pEld->m_ksg0 = 50.*2048./(float)i;
		x = 1./pEld->m_ksg;
		for(d=0, i=0; i<2048; i++, d+=x)
			pEld->m_wn[i] = SGauss(sigma,d);
	}
	memcpy(pEld->m_ws, pEld->m_wn, 2048*4);
	pEld->m_ksg = pEld->m_ksg0;
	memset(pEld->m_wa,0,2048*4);

	pLZ->m_vRefls.clear();
	pEld->m_athr = 0.4;	pEld->m_athd = 0.38;	// Original

	if( 0 == hkm )
	{
		hkm = (int)__max(m_image.w / __min(pLZ->a_length,pLZ->b_length), m_image.h / __min(pLZ->a_length,pLZ->b_length)) + 1;
	}
	// Try to estimate the intensities
	try
	{
		dMinR2 = SQR(dMinR);
		dMaxR2 = SQR(dMaxR);
		for(r=1;r<=hkm;r++)
		for(h=-r; h<=r; h++)
		for(k=-r; k<=r; k++)
		{
			if((h|k) && (abs(h)==r || abs(k)==r))
			{
				if( bCentered && ((h + k) & 1) )
					continue;
				// This includes the Laue zone shift
				point = pLZ->GetPeakRelXY(h, k);
				if( (d = AbsC(x = point.x, y = point.y)) < rmin )
					continue;
				// check the radius
				if( TRUE == bCheckR )
				{
					dDist2 = point.Sqr();//SQR(x) + SQR(y);
					if( (dDist2 < dMinR2) || (dDist2 > dMaxR2) )
					{
						continue;
					}
				}
				if( d > rmax )
					continue;
				refl.h = h;//R->h = h;
				refl.k = k;//R->k = k;
				refl.xc = refl.xl = (x += m_center.x);//R->xc = R->xl = (x += X0);
				refl.yc = refl.yl = (y += m_center.y);//R->yc = R->yl = (y += Y0);
				if( (refl.xl < 0) || (refl.yl < 0) ||
					(refl.xl > m_image.w) || (refl.yl > m_image.h) )
					continue;
				pEld->m_hkmaxe = __max(pEld->m_hkmaxe, __max(abs(h),abs(k)));
				if( pEld->m_bSmartLR )
				{
					CalcSmartCoordinates( &*pLZ->m_vRefls.begin(), pLZ->m_vRefls.size(), &refl );
				}
				else
				{
					refl.xc = m_center.x + h*m_h_dir.x + k*m_k_dir.x + pLZ->shift.x;
					refl.yc = m_center.y + h*m_h_dir.y + k*m_k_dir.y + pLZ->shift.y;
				}
				refl.xc0 = refl.xc;//R->xc0 = R->xc;
				refl.yc0 = refl.yc;//R->yc0 = R->yc;
				if( -1 == (f = GetReflInfo(&refl, (1 == pLZ->nEstCount), ELD_INTEGRATE_SHAPE_FIT != m_nIntegrationMode)) )
					continue;
				if( refl.i_amp > pEld->m_max - SCA(pEld,30) )
					f &= ~FLAG_FLAT_CENTER;

				pn = ReflColor( &refl, pEld, ELD_INTEGRATE_SHAPE_FIT != m_nIntegrationMode );

				if((f & (FLAG_BAD_SHAPE | FLAG_FLAT_CENTER | FLAG_UNDEFINED_CENTER | FLAG_BELOW_BACKGROUND | FLAG_TOO_NOISY)) == 0)
				{
					s = refl.sg;
					d = refl.ae;//s = R->sg;	d = R->ae;
					if( (d > 0.2) && (d < 0.6) )
					{
						sgmean += s;
						nmean++;
					}
				}
				//DrawMark(m_center.x + h*m_h_dir.x + k*m_k_dir.x + pLZ->shift.x,	m_center.y + h*m_h_dir.y + k*m_k_dir.y + pLZ->shift.y, pn, pEld->m_rma);	// was just x,y
				//DrawMark(refl.xc,	refl.yc, pn, pEld->m_rma);	// was just x,y
				refl.dval = m_pObj->NI.Scale * 1e10 / AbsC(point.x, point.y*cf);	// d-value
				pLZ->m_vRefls.push_back(refl);
				TextToDlgItemW(m_pObj->hWnd, IDC_ELD_NR, "NR=%4d", pLZ->m_vRefls.size());
			}
		}
	}
	catch(std::exception& e)
	{
		MessageBox(NULL, GetUserErrorAndTrace(_FMT(_T("ExtractRefls() -> %s"), e.what())).c_str(), _T("ERROR"), MB_OK);
	}

	if( ELD_INTEGRATE_SHAPE_FIT != m_nIntegrationMode )
	{
		size_t nRefls = pLZ->m_vRefls.size();
		if( (ELD_INTEGRATE_SHAPE_FIT == m_nIntegrationMode) || (0 == nRefls) )
		{
			return 0;
		}
		// If integration mode is ON -> calculate the average reflection radius
		for(rit = pLZ->m_vRefls.begin(), i = 0, d = 0.0; rit != pLZ->m_vRefls.end(); i++, ++rit)
		{
			d += rit->sr;
		}
		d /= (float) nRefls;
		pEld->m_arad = round(d);

		TextToDlgItemW(m_pObj->hWnd, IDC_ELD_NR, "aR=%3d", pEld->m_arad);

		pEld->m_dvfrom = 1e20;
		pEld->m_dvto = -1e20;
		pEld->m_bCentOnly = FALSE;
		for(rit = pLZ->m_vRefls.begin(), i = 0; rit != pLZ->m_vRefls.end(); ++rit, i++)
		{
			if (pEld->m_dvfrom > rit->dval) pEld->m_dvfrom = rit->dval;
			if (pEld->m_dvto   < rit->dval) pEld->m_dvto   = rit->dval;
		}
		// integrate them finally
		if( ELD_INTEGRATE_COMMON_PROFILE == pEld->m_nIntegrationMode )
		{
			InternalCalibration();
		}
		else if( ELD_INTEGRATE_SIMPLE_SUMMATION == m_nIntegrationMode )
		{
			IntensitySummation();
		}

		pEld->m_flags &= ~(SH1_EX | MAP_EX);
		pLZ->m_nFlags |= LaueZone::ZONE_INT_EXTRACTED;

		goto SuccExit;
	} // END INTEGRATE

	i = 0;
//	if(nmin)  {sgmin  /= nmin;  i++;}
	if(nmean) {sgmean /= nmean; i++;}
//	if(nmax)  {sgmax  /= nmax;  i++;}

	if(i) s = (sgmean)/i; else s = 0.1179;
	if(!nmean) sgmax = sgmin = sgmean = s;
	if(!nmax)  sgmax = sgmean;
	if(!nmin)  sgmin = sgmean;
	pEld->m_MeanSigma = sgmax;		// /0.707

	TextToDlgItemW(m_pObj->hWnd, IDC_ELD_NR, "OK=%4d", nmean);

	for(i=0; i<256; i++)
	{
		if(pEld->m_wa[i+256] > 0.1)
			pEld->m_wa[i] /= pEld->m_wa[i+256];
	}
	for(d=1e-8,i=0; i<256; i++) if(pEld->m_wa[i] > d) d = pEld->m_wa[i];
	for(i=0; i<256; i++) pEld->m_wa[i] /= d;

	memset(pEld->m_wn,0,2048*4);

	for(i=0; i<127; i++)
	{
		x=pEld->m_wa[i]; y = pEld->m_wa[i+1];
		for(d=0, h=0; h<16; h++, d+=0.0625)	pEld->m_wn[i*16+h] = x*(1.-d) + y*d;
	}
	for(d=0, i=2047; i>=0; i--)	pEld->m_wn[i] =(d += pEld->m_wn[i]);
	for(d=1e-8,i=0; i<2048; i++) if(pEld->m_wn[i] > d) d = pEld->m_wn[i];
	for(i=0; i<2048; i++) pEld->m_wn[i] /= d;
	for(i=1; i<2048; i++) if(pEld->m_wn[i]<0.3) break;
	for(h=1; h<2048; h++) if(pEld->m_ws[h]<0.3) break;
	pEld->m_ksg0 = pEld->m_ksg * (float)i / (float)h;

	{	// calculate curve fitting/integration ratio
		float d, d0, dd, s0;
		for(i = 0; i <= pEld->m_rms; i++)
		{
			ftmp[i] = CGauss(pEld->m_MeanSigma, i/*, pEld->m_ksg, pEld->m_ws*/);
		}
		d = INT_CUTOFF;	// Cutoff value
		for(i = 0, s0 = 0.0, d0 = 1; i < pEld->m_rms; i++)
		{
			if (i && ftmp[i]>=ftmp[i-1]) break; // Zero or negative slope, stop integration
			if (ftmp[i]>d || i==0)
			{
				d0 = i;
			}
			else
			{
				d0 = i-1+(ftmp[i-1]-d)/(ftmp[i-1]-ftmp[i]);
				break;
			}
		}
		dd = d0;
		for(i=0,d=0.,d0=0.;i<dd;i++)
		{
			d += (ftmp[i])*(i+.5);	// Calculate reflection intensity as I(r)*r
			d0 += i+.5;
		}
		d /= d0;
		if (d>1e-8) pEld->m_cfoi = 1./d;
	}

	sa0 = 0.0;
	sae = 0.0;
	nmean = 0;
	for(rit = pLZ->m_vRefls.begin()/*, i = 0*/; rit != pLZ->m_vRefls.end(); /*i++, */++rit)
	{
		if( (fabs(rit->a0+rit->bg) < 0.5) && (rit->a0 > 0.15) )
		{
			sa0 += rit->a0;
			sae += rit->ae;
			nmean++;
		}
	}
	// more than 5 good reflections
	if( nmean > 5)
	{
		s = sa0/sae*1.1;
		for(rit = pLZ->m_vRefls.begin()/*, i = 0*/; rit != pLZ->m_vRefls.end();/* i++, */++rit)
		{
			if( 0 == (rit->flags & FLAG_BAD_SHAPE) )
			{
				rit->ae *= s;
			}
		}

	}
	else
	{ // less than 5 good reflections
//		for(R=pEld->m_rptr, i=0; i<pEld->m_nrefl; i++, R++)
//			if ((R->f_amp & FLAG_BAD_SHAPE) == 0) {s = pEld->m_MeanSigma/R->sg;	R->ae *= s*s;}
	}

	pEld->m_flags &= ~(SH1_EX | MAP_EX);
//	pEld->m_flags |= ESR_DONE;
	pLZ->m_nFlags |= LaueZone::ZONE_INT_EXTRACTED;

	pEld->m_dvfrom = 1e20;
	pEld->m_dvto = -1e20;
	pEld->m_infrom = 1e20;
	pEld->m_into = -1e20;
	pEld->m_bCentOnly = FALSE;
	pEld->m_meanampl = 0;
	for(rit = pLZ->m_vRefls.begin(); rit != pLZ->m_vRefls.end(); ++rit)
	{
		if (pEld->m_dvfrom>rit->dval) pEld->m_dvfrom = rit->dval;
		if (pEld->m_dvto  <rit->dval) pEld->m_dvto   = rit->dval;
		if (pEld->m_infrom>rit->ae)   pEld->m_infrom = rit->ae;
		if (pEld->m_into  <rit->ae)   pEld->m_into   = rit->ae;
		pEld->m_meanampl +=  rit->ae;
	}
	if( false == pLZ->m_vRefls.empty() )
		pEld->m_meanampl /= pLZ->m_vRefls.size();
//	if( pEld->m_nrefl )
//		pEld->m_meanampl /= pEld->m_nrefl;

SuccExit:
	FillReflList(nLaueZone);
//SuccExit:
//	if( OR )
//		free(OR);
//	return pEld->m_nrefl;
	InvalidateRect(m_pObj->ParentWnd, NULL, TRUE);
	return pLZ->m_vRefls.size();
}
/****************************************************************************/
static HANDLE CreateHKAList( OBJ* O )
{
	LPELD	pELD = (LPELD) &O->dummy;
	EldReflVecCit cit;
	BOOL	bI = IsDlgButtonChecked( O->hWnd, IDC_ELD_INTENSITIES);

	LPSTR	pbuf;
	static	char fmt[]="%3d %3d %5.1f %5.1f\n";
	size_t	nRefls = pELD->m_pCurLZ->m_vRefls.size();

	sprintf( TMP, fmt, -11, -11, 1234., 1234.);
	if( NULL == (pbuf = (LPSTR)calloc( 1, nRefls*(strlen(TMP)+1)+200 )) )
		return NULL;

	qsort(&*pELD->m_pCurLZ->m_vRefls.begin(), pELD->m_pCurLZ->m_vRefls.size(), sizeof(EREFL), hk_cmp);
	pbuf += sprintf(pbuf, "; File : %s, estimation pass %d\n", O->fname, pELD->m_pCurLZ->nEstCount);
	if( bI )
	{
		pbuf += sprintf(pbuf, "ELDInt 1\n");
		pbuf += sprintf(pbuf, "; h    k   Iobs   Iest  \n");
	}
	else
	{
		pbuf += sprintf(pbuf, "ELDAmp 1\n");
		pbuf += sprintf(pbuf, "; h    k   Aobs   Aest  \n");
	}
	std::fill_n(pbuf, 40, '-');
	pbuf += 40;
	pbuf += sprintf(pbuf, "\n");
	for(cit = pELD->m_pCurLZ->m_vRefls.begin(); cit != pELD->m_pCurLZ->m_vRefls.end(); ++cit)
	{
		if( (ELD::ELD_INTEGRATE_SHAPE_FIT == pELD->m_nIntegrationMode) ||
			(ELD::ELD_INTEGRATE_COMMON_PROFILE == pELD->m_nIntegrationMode) )
		{
			if( bI )
				pbuf += sprintf(pbuf, fmt, cit->h, cit->k, cit->Amp_bkg, cit->dEstA);
			else
				pbuf += sprintf(pbuf, fmt, cit->h, cit->k, FastSqrt(cit->Amp_bkg), FastSqrt(cit->dEstA));
		}
		else if( ELD::ELD_INTEGRATE_SIMPLE_SUMMATION == pELD->m_nIntegrationMode )
		{
			if( bI )
				pbuf += sprintf(pbuf, fmt, cit->h, cit->k, cit->Amp_bkg, cit->IntegrA);
			else
				pbuf += sprintf(pbuf, fmt, cit->h, cit->k, FastSqrt(cit->Amp_bkg), FastSqrt(cit->IntegrA));
		}
	}
	*(pbuf++) = 0;
	return pbuf;
}
/****************************************************************************/
// Added by Peter on [8/8/2005]
static bool GetFormatString(LPCTSTR pszStr, int &h_pos, int &k_pos, int &l_pos,
							int &d_pos, int &A_pos, int &S_pos)
{
	LPCTSTR ptr = pszStr;
	int nPos = 0;

	if( 0 != strncmp(ptr, "Format", strlen("Format")) )
	{
		return false;
	}
	ptr += strlen("Format");
	while( *ptr )
	{
		if( isalpha(*ptr) )
		{
			switch( *ptr )
			{
			case 'h':
				h_pos = nPos;
				break;
			case 'k':
				k_pos = nPos;
				break;
			case 'l':
				l_pos = nPos;
				break;
			case 'd':
				d_pos = nPos;
				break;
			case 's':
				S_pos = nPos;
				break;
			case 'a':
				A_pos = nPos;
				break;
			default:
				break;
			}
			nPos++;
		}
		ptr++;
	}
	return true;
}
// End of addition [8/8/2005]
/****************************************************************************/
// Revised by Peter on 2004 Jan 15
void ELD::ELDSaveHKA( LPCSTR fname )
{
	HWND	hDlg = m_pObj->hWnd;
	TCHAR foo[1000];
	GetWindowText(m_pObj->hWnd, foo, 1000);
	FILE	*out;
	if( NULL == (out = fopen( fname, "wt")) )
		return;

	int count = SendDlgItemMessage(hDlg, IDC_ELD_LIST, CB_GETCOUNT, 0, 0);
	if (count == CB_ERR)
	{
		// throw...
	}
	OutputDebugString(fname);
	TCHAR buffer[1024];
	for (int i = 0; i != count; i++)
	{
		if (SendDlgItemMessage(hDlg, IDC_ELD_LIST, CB_GETLBTEXT, (WPARAM)i, (LPARAM)buffer) == CB_ERR)
		{
			// throw...
		}
		fprintf(out, "%s\n", buffer);
	}

	fclose(out);
	return;
}
/****************************************************************************/
void ELD::Synth()
{
	HWND	hDlg = m_pObj->hWnd;
	EldReflVecCit cit;
	int		h, k, x, y, xp, yp, xm, ym, i, hmax=0, kmax=0;
	float	x0, y0, d, a, am, /*rm, */sg = 0.16;
	int		ll;
	std::vector<float> gauss(400, 0.0f);
	HCURSOR	hoc = SetCursor(hcWait);
//	LaueZone *pLZ;
	CVector2d point;

	if( !(LaueZone::ZONE_INDEXED & m_pCurLZ->m_nFlags) )
		return;

	if( m_pObj->ebp )
		free(m_pObj->ebp);

	ll = m_image.w * m_image.h * 4;
	m_pObj->ebp = (LPBYTE)calloc(2, ll);
	if( NULL == m_pObj->ebp )
		return;

	for(i = 0; i < 400; i++)
	{
		gauss[i] = CGauss(m_MeanSigma, i * 0.2F/*, m_ksg, m_ws*/);
	}

	GetAsyncKeyState(VK_ESCAPE); // reset the state of GetAsyncKeyState

	for(cit = m_pCurLZ->m_vRefls.begin(), i = -1; cit != m_pCurLZ->m_vRefls.end(); i++, ++cit)
	{
		if( i < 0 )
		{
			h = 0;
			k = 0;
			am = 10.;
		}
		else
		{
			h = cit->h;
			k = cit->k;
			am = cit->ae;
		}
//		point = pELD->m_GetPeakAbsXY(h, k);
		x0 = m_pCurLZ->GetPeakAbsX(h, k);//point.x;//h*HX + k*KX + X0;
		y0 = m_pCurLZ->GetPeakAbsY(h, k);//;//h*HY + k*KY + Y0;
		// Added by Peter on [10/8/2005]
		// 000 spot
		if( i < 0 )
		{
			x0 -= m_pCurLZ->shift.x;
			y0 -= m_pCurLZ->shift.y;
		}
		// End of addition [10/8/2005]
		xp = round(x0 + m_rma);
		yp = round(y0 + m_rma);
		xm = round(x0 - m_rma);
		ym = round(y0 - m_rma);
		for(x = xm; x <= xp; x++)
		{
			for(y = ym; y <= yp; y++)
			{
				if( (d = AbsC(x-x0, y-y0)) > m_rma )
					continue;
				a = am * gauss[(int)(d*10.)];
				AddFPix8(m_pObj->ebp, m_image.w, m_image.h, x, y, a);
			}
		}
		hmax = __max(h, hmax);
		kmax = __max(k, kmax);
		if( 31 == (i & 31) )
		{
//			sprintf( TMP, "%2d,%2d", hmax, kmax );
//			SetDlgItemText( hDlg, IDC_ELD_NR, TMP );
			TextToDlgItemW(hDlg, IDC_ELD_NR, "%2d,%2d", hmax, kmax);
			if( GetAsyncKeyState(VK_ESCAPE) )
				break;
		}
	}
	// convert the FLOAT-type m_image into the current pixel format
	FMapToImgX((FLOAT*)m_pObj->ebp, m_image, m_min, m_max);

	InvalidateRect( m_pObj->ParentWnd, NULL, FALSE );
	SetCursor(hoc);
}
/****************************************************************************/
void ELD::UpdateFirstRefls(LPARAM lParam, int h, int k, WPARAM wParam)
{
	HWND	hDlg = m_pObj->hWnd;//O->hWnd;
	OBJ*	I = m_pPar;	// Image Object
	double	dScale = (double)I->scd / (double)I->scu;
	float	xx, x = (LOINT(lParam)) * dScale + I->xo;
	float	yy, y = (HIINT(lParam)) * dScale + I->yo;
	float	xm = x, ym = y, sc;//, as;
	int		i, j, f, x0, y0, nLaueZone;
	LPEREFL R = m_pCurRefl;
	BOOL	bFlg = (h | k);
	double	dMinR, dMaxR, dDist2, dH, dK;
	int		ZA_u, ZA_v, ZA_w, nLZ, nLindex;
	CVector3l vec;
	double ampl, as;

	ScanAll(hDlg, m_pObj);
	nLaueZone = m_nCurLaueZone;
	nLindex = 0;
	xx = x;
	yy = y;

	if( bFlg )
	{
		x = xm = m_pCurLZ->GetPeakAbsX(h, k);
		y = ym = m_pCurLZ->GetPeakAbsY(h, k);
	}

	if( TRUE == m_bUseLaueCircles )
	{
		dMinR = 0;
		dMaxR = 1e22;
		for(nLZ = 0; nLZ < ELD_MAX_LAUE_CIRCLES; nLZ++)
		{
			if( true == m_vbLaueCircles[nLZ] )
			{
				dMaxR = m_vLaueZones[nLZ]->m_dRadius;
			}
			if( nLZ > 0 )
			{
				if( true == m_vbLaueCircles[nLZ-1] )
				{
					dMinR = m_vLaueZones[nLZ-1]->m_dRadius;
				}
			}
			dDist2 = SQR(x - m_center.x) + SQR(y - m_center.y);
			if( (dDist2 >= SQR(dMinR)) && (dDist2 <= SQR(dMaxR)) )
			{
				nLaueZone = nLindex = nLZ;
				m_pCurLZ = m_vLaueZones[nLZ];
				break;
			}
		}
	}
	if( dScale > 1.0 )
	{
		dScale = 1.0;
	}

	int n = m_nPeakWindow / 2;

	f = PeakCenter2(n, x, y, n*dScale, 0.5, TRUE, ampl, as);
	if(f & FLAG_FLAT_CENTER)
	{
		// try a bigger search window
		x = xx;
		y = yy;
		f = PeakCenter2(1.6 * n, x, y, 1.6 * n * dScale, 0.5, TRUE, ampl, as);
	}
	if(f & FLAG_FLAT_CENTER)
	{
		// try an even bigger search window
		n*=2;
		x = xx;
		y = yy;
		f = PeakCenter2(1.6 * n, x, y, 1.6 * n * dScale, 0.5, TRUE, ampl, as);
	}
	f &= FLAG_ERC;

	if( f && (0 == (LaueZone::ZONE_INDEXED & m_pCurLZ->m_nFlags)) )
	{
		MessageBeep(MB_ICONQUESTION);
		return;
	}
	// Modified by Peter on [8/8/2005]
	if( !bFlg && (0 == (LaueZone::ZONE_INDEXED & m_pCurLZ->m_nFlags)) )
//	if( !bFlg && (0 == (pELD->m_flags & ELR_DONE)) )
	// End of modification [8/8/2005]
	{
		for(i = 0; i < 3; i++)
		{
//			if( (FastAbs(x - pLZ->x[i]) < 0.1) && (fabs(y - pLZ->y[i]) < 0.1) )
			if( SQR(x - m_pCurLZ->x[i]) + SQR(y - m_pCurLZ->y[i]) < 1.0 )
				return; // Same as before
		}
	}
	for(i = 0; i < 3; i++)
	{
		if( IsDlgButtonChecked(hDlg, idc_REF[i]) )
			break;
	}
	if( i < 3 )
	{
		m_pCurLZ->x[i] = x;
		m_pCurLZ->y[i] = y;
		CheckDlgButton(hDlg,idc_REF[i], 0);
		if( !bFlg && i < 2 )
			CheckDlgButton(hDlg, idc_REF[(i+1)], 1);
		InvalidateRect( /*O->ParentWnd*/m_pPar->hWnd, NULL, FALSE );
	}

	f = 0;
	/*pELD*/m_flags &= ~(SH1_EX);

	// Modified by Peter on [8/8/2005]
	if( (i >= 3) && (LaueZone::ZONE_INDEXED & m_pCurLZ->m_nFlags) )
//	if( (i >= 3) && (pELD->m_flags & ELR_DONE) )
	// End of modification [8/8/2005]
	{
//		x = xm - X0;
//		y = ym - Y0;
		double denom = 1.0 / (m_k_dir.y*m_h_dir.x - m_k_dir.x*m_h_dir.y);
		x = xm - m_center.x;
		y = ym - m_center.y;
		dH = (x*m_k_dir.y - y*m_k_dir.x) * denom;
		dK = (y*m_h_dir.x - x*m_h_dir.y) * denom;
		x -= m_pCurLZ->shift.x;
		y -= m_pCurLZ->shift.y;
		R->h = h = round( (x*m_k_dir.y - y*m_k_dir.x) * denom );
		R->k = k = round( (y*m_h_dir.x - x*m_h_dir.y) * denom );
//		R->h = h = round( (x*KY - y*KX) / (KY*HX - KX*HY) );
//		R->k = k = round( (y*HX - x*HY) / (KY*HX - KX*HY) );
		x = round(R->xl = m_pCurLZ->GetPeakAbsX(h, k));
		y = round(R->yl = m_pCurLZ->GetPeakAbsY(h, k));
//		x = round(R->xl = (h*HX + k*KX + X0));
//		y = round(R->yl = (h*HY + k*KY + Y0));
		R->xc = R->xl;
		R->yc = R->yl;
		if( m_bSmartLR )
			CalcSmartCoordinates( &*m_pCurLZ->m_vRefls.begin(), m_pCurLZ->m_vRefls.size(), R );
		f = GetReflInfo(R, FALSE, FALSE) & FLAG_BAD_SHAPE;
		if( !f )
		{
			if( IsDlgButtonChecked(hDlg, IDC_ELD_SHAPE) )
				m_flags |= SH1_EX;
			sc = m_MeanSigma/R->sg;
			R->ae *= sc*sc;
		}
		else
			sc = 1.0;
		f++;
	}

	if( IsDlgButtonChecked(hDlg, IDC_ELD_MAP) )
	{
		double	a, amin, amax;
		x0 = round(R->xc);
		y0 = round(R->yc);
		for(amin = 0xFFFF, amax = 0, i=-32; i<32; i++)
		{
			for(j=-32; j<32; j++)
			{
				a = m_image.GetPixel(j + x0, i + y0);
				if( a < amin)
				{
// 					Trace(_FMT(_T("new amin: %d"), a));
					amin = a;
				}
				if((a > amax) && ( abs(i) < 16 ) && (abs(j) < 16) )
				{
// 					Trace(_FMT(_T("new amax: %d"), a));
					amax = a;
				}
			}
		}
		m_flags &= ~MAP_EX;
		sc = 255. / (amax-amin+0.1);

		BYTE* mapIterator = m_map;
		for(i=-32; i<32; i++)
		{
			for(j=-32; j<32; j++)
			{
				a = m_image.GetPixel(j + x0, i + y0);
				if((a = round((a - amin) * sc)) > 255) a = 255;
				*(mapIterator++) = a;
			}
		}
	}
	PaintELDDlg();

	std::string sString;

	if( m_f2to3 )
	{
		Calc2to3d(ZA_u, ZA_v, ZA_w);
		GetIndex3D(CVector3d(ZA_u, ZA_v, ZA_w), dH, dK, nLindex, vec);
//		CVector3d v1(pELD->m_uh, pELD->m_uk, pELD->m_ul), v2(pELD->m_vh, pELD->m_vk, pELD->m_vl), v3;
//		v3 = v1 ^ v2;
//		nLZ = pELD->m_nCurLaueZone;
		sString = _FMT(_T("(%d,%d,%d)"), vec.x, vec.y, vec.z);
		//sprintf(TMP, "(%d,%d,%d)", vec.x, vec.y, vec.z);
//			h*pELD->m_uh + k*pELD->m_vh, h*pELD->m_uk + k*pELD->m_vk, h*pELD->m_ul + k*pELD->m_vl);
//			h*pELD->m_uh + k*pELD->m_vh/* + nLindex*ZA_u*/,//nLZ*ZA_u,
//			h*pELD->m_uk + k*pELD->m_vk/* + nLindex*ZA_v*/,//nLZ*ZA_v,
//			h*pELD->m_ul + k*pELD->m_vl + nLindex/**ZA_w*/);//nLZ*ZA_w);
	}
	else
	{
		sString = _FMT(_T("(%d,%d)"), h, k);
	}

	if( (0 != (h|k)) && (IsCalibrated()) )
	{
		double dScale = m_pObj->NI.Scale * 1e10;
//		sprintf(TMP_End, " %7.4fÅ", O->NI.Scale * 1e10 / AbsC(HX*h+KX*k, (HY*h+KY*k)*O->NI.AspectXY));
		if( m_pCurLZ->m_bHaveCell3D )
		{
			GetIndex3D(CVector3d(ZA_u, ZA_v, ZA_w), dH, dK, nLindex, vec);
			int _Hi = vec.x;//h*pELD->m_uh + k*pELD->m_vh;// + nLindex*ZA_u;
			int _Ki = vec.y;//h*pELD->m_uk + k*pELD->m_vk;// + nLindex*ZA_v;
			int _Li = vec.z;//h*pELD->m_ul + k*pELD->m_vl + nLindex/**ZA_w*/;
			double *pC = m_pCurLZ->m_adConvCell;
			double a  = pC[0];
			double b  = pC[1];
			double c  = pC[2];
			double ca = FastCos(DEG2RAD(pC[3]));
			double cb = FastCos(DEG2RAD(pC[4]));
			double cg = FastCos(DEG2RAD(pC[5]));
			double sa = FastSin(DEG2RAD(pC[3]));
			double sb = FastSin(DEG2RAD(pC[4]));
			double sg = FastSin(DEG2RAD(pC[5]));
			double dVal = (1 - SQR(ca) - SQR(cb) - SQR(cg) + 2.0*ca*cb*cg);
			dVal /= SQR(_Hi*sa/a) + SQR(_Ki*sb/b) + SQR(_Li*sg/c) +
				2.0*(_Ki*_Li*(cb*cg - ca)/(b*c) + _Li*_Hi*(ca*cg - cb)/(a*c) +
				_Hi*_Ki*(ca*cb - cg)/(a*b));
			dVal = FastSqrt(dVal);
//			sprintf(TMP_End, " %7.4fÅ", dVal);
// 			sprintf(TMP_End, " %7.4f%c", dVal, ANGSTR_CHAR);
			sString += _FMT(_T(" %7.4f%c"), dVal, ANGSTR_CHAR);
		}
		else
		{
			sString += _FMT(_T(" %7.4f%c"), (dScale / AbsC(m_pCurLZ->GetPeakRelX(h, k),
				m_pCurLZ->GetPeakRelY(h, k)*m_pObj->NI.AspectXY)), ANGSTR_CHAR);
// 			sprintf(TMP_End, " %7.4f%c", dScale / AbsC(m_pCurLZ->GetPeakRelX(h, k),
// 				m_pCurLZ->GetPeakRelY(h, k)*m_pObj->NI.AspectXY), ANGSTR_CHAR);
// 			sprintf(TMP_End, " %7.4fÅ",
// 				dScale / AbsC(pLZ->GetPeakRelX(h, k), pLZ->GetPeakRelY(h, k)*O->NI.AspectXY));
		}
	}
	SetDlgItemText(hDlg, IDC_ELD_INFO, sString.c_str());
	if( f )
	{
		if( m_f2to3 )
		{
			GetIndex3D(CVector3d(ZA_u, ZA_v, ZA_w), dH, dK, nLindex, vec);
			sString = _FMT(_T(" %3d %3d %3d"), vec.x, vec.y, vec.z);
// 			sprintf(TMP, " %3d %3d %3d", vec.x, vec.y, vec.z);
//				h*pELD->m_uh + k*pELD->m_vh/* + nLZ*ZA_u*/,
//				h*pELD->m_uk + k*pELD->m_vk/* + nLZ*ZA_v*/,
//				h*pELD->m_ul + k*pELD->m_vl + nLZ/**ZA_w*/);
		}
		else
		{
// 			sprintf(TMP, " %3d %3d", h, k);
			sString = _FMT(_T(" %3d %3d"), h, k);
		}
		i = (int)SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_FINDSTRING, m_last, (LPARAM)(LPSTR)sString.c_str());
		if( i == CB_ERR )
		{
			if( m_f2to3 )
			{
				sString = _FMT(_T(" %3d %3d %3d"), vec.x, vec.y, vec.z);
// 				sprintf(TMP, " %3d %3d %3d", vec.x, vec.y, vec.z);
//				sprintf(TMP, ";%3d %3d %3d",
//					h*pELD->m_uh + k*pELD->m_vh/* + nLZ*ZA_u*/,
//					h*pELD->m_uk + k*pELD->m_vk/* + nLZ*ZA_v*/,
//					h*pELD->m_ul + k*pELD->m_vl + nLZ/**ZA_w*/);
			}
			else
			{
// 				sprintf(TMP, ";%3d %3d", h, k);
				sString = _FMT(_T(";%3d %3d"), h, k);
			}
			i = (int)SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_FINDSTRING, m_last, (LPARAM)(LPSTR)sString.c_str());
		}
		SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_SETCURSEL, i, 0);

		{
			EldReflVecIt rit;
			for(rit = m_pCurLZ->m_vRefls.begin(); rit != m_pCurLZ->m_vRefls.end(); ++rit)
			{
				if (h == rit->h && k == rit->k)
				{
/*
					sprintf(TMP,"%s%s%s%s%s%s",
						rit->flags & FLAG_BAD_SHAPE ? "FLAG_BAD_SHAPE ":"",
						rit->flags & FLAG_UNDEFINED_CENTER ? "FLAG_UNDEFINED_CENTER ":"",
						rit->flags & FLAG_MISPLACED ? "FLAG_MISPLACED ":"",
						rit->flags & FLAG_FLAT_CENTER ? "FLAG_FLAT_CENTER ":"",
						rit->flags & FLAG_BELOW_BACKGROUND ? "FLAG_BELOW_BACKGROUND ":"",
						rit->flags & FLAG_TOO_NOISY ? "FLAG_TOO_NOISY ":"");
*/
						if( 0 != (wParam & MK_RBUTTON) )
						{
							if (rit->flags & FLAG_USER_DELETED)
								rit->flags &= ~FLAG_USER_DELETED;
							else
								rit->flags |=  FLAG_USER_DELETED;
							InvalidateRect( m_pObj->ParentWnd, NULL, FALSE );
						}
						break;
				}
			}
		}
	}
	if( m_flags & SH1_EX )
	{
		SetDlgItemText( hDlg, IDC_ELD_NR , _FMT(_T("s=%5.2f"), (0.707/m_sg)).c_str() );
// 		sprintf( TMP, "s=%5.2f", 0.707/m_sg);
// 		SetDlgItemText( hDlg, IDC_ELD_NR , TMP );
	}
}
/****************************************************************************/
static float Det( float a1, float a2, float a3,
				  float b1, float b2, float b3,
				  float c1, float c2, float c3 )
{
	return ( a1*(b2*c3-b3*c2) + a2*(b3*c1-b1*c3) + a3*(b1*c2-b2*c1) );
}
//-------------------------------------------------------------------------
static void XLat0(void)
{
	snn=snh=snk=snx=sny=shx=skx=shh=skk=shk=shy=sky = 0;
}
//-------------------------------------------------------------------------
static void XLat1(float h, float k, float x, float y)
{
	snh += h;	snk += k;	snx += x;	sny += y;
	shx += h * x;	skx += k * x;	shh += h * h;
	skk += k * k;	shk += h * k;	shy += h * y;
	sky += k * y;	snn += 1.;
}
//-------------------------------------------------------------------------
static int XLat2(LPELD pELD)
{
	double	d, inv_d;
	d = Det( shh,shk,snh, shk,skk,snk, snh,snk,snn );
	if( fabs(d) < 1e-6 )
		return FALSE;
	// Added by Peter on [7/8/2005]
	inv_d = 1.0 / d;
	// End of addition [7/8/2005]
	pELD->m_h_dir.x  = Det( shx,shk,snh, skx,skk,snk, snx,snk,snn ) * inv_d;
	pELD->m_k_dir.x  = Det( shh,shx,snh, shk,skx,snk, snh,snx,snn ) * inv_d;
	pELD->m_center.x = Det( shh,shk,shx, shk,skk,skx, snh,snk,snx ) * inv_d;
	pELD->m_h_dir.y  = Det( shy,shk,snh, sky,skk,snk, sny,snk,snn ) * inv_d;
	pELD->m_k_dir.y  = Det( shh,shy,snh, shk,sky,snk, snh,sny,snn ) * inv_d;
	pELD->m_center.y = Det( shh,shk,shy, shk,skk,sky, snh,snk,sny ) * inv_d;

//	pELD->m_al = AbsC(HX, HY);	pELD->m_bl = AbsC(KX, KY);
//	pELD->m_gamma = AB_Angle( HX, HY, KX, KY);
//	return ((pELD->m_al > 3) && (pELD->m_bl > 3));
//	return ( (AbsC(HX, HY) > 3.) && (AbsC(KX, KY) > 3.) );
	return ( (AbsC(pELD->m_h_dir.x, pELD->m_h_dir.y) > 3.) && (AbsC(pELD->m_k_dir.x, pELD->m_k_dir.y) > 3.) );
}
//-------------------------------------------------------------------------
static int XRef(OBJ* O,
				float rmin, float rmax,
				float hkmax, int rad, float cd, int mode, BOOL bDraw)
{
	LPELD	pELD = (LPELD) &O->dummy;
	float	h, k, x, y, xf, yf, d;
	float	hx = pELD->m_h_dir.x, hy = pELD->m_h_dir.y, kx = pELD->m_k_dir.x, ky = pELD->m_k_dir.y, x0 = pELD->m_center.x, y0 = pELD->m_center.y;
	BOOL	bCentered = IsDlgButtonChecked( O->hWnd, IDC_ELD_CENTERED );
	float	esdx, esdy, dx, dy;
	BOOL	bCheckR;
	double	dMinR, dMaxR, dDist2;
	double ampl, as;
	XLat1(1, 0, hx, hy);
	XLat1(0, 1, kx, ky);

	esdx = esdy = 0.0;

	bCheckR = FALSE;
	dMinR = 0.0;
	dMaxR = 1e+33;
	if( TRUE == pELD->m_bUseLaueCircles )
	{
		int nLaueZone = pELD->m_nCurLaueZone;
		if( true == pELD->m_vbLaueCircles[nLaueZone] )
		{
			dMaxR = pELD->m_vLaueZones[nLaueZone]->m_dRadius;
			if( nLaueZone > 0 )
			{
				if( true == pELD->m_vbLaueCircles[nLaueZone-1] )
				{
					dMinR = pELD->m_vLaueZones[nLaueZone-1]->m_dRadius;
				}
			}
			bCheckR = TRUE;
		}
	}

	hkmax = round(hkmax);
	pELD->m_pCurLZ->nLRefCnt = 0;
	for(h = 0; h <= hkmax; h += 1.0)
	{
		for(k = -hkmax; k <= hkmax; k += 1.0)
		{
			if( bCentered && ((round(h) + round(k)) & 1) )
				continue;
			if( (h < 0.1) && (k < 0.1) )
				continue;
			x = h*hx + k*kx;
			y = h*hy + k*ky;
			// Added by Peter on [7/8/2005]
			// check radii
			dDist2 = SQR(x) + SQR(y);
			if( TRUE == bCheckR )
			{
				if( (dDist2 < SQR(dMinR)) || (dDist2 > SQR(dMaxR)) )
				{
					continue;
				}
			}
			// End of addition [7/8/2005]
			if( (d = AbsC(x,y)) < rmin)
				continue;
			if( d > rmax )
				continue;
			xf = x0 - x;
			yf = y0 - y;
			x += x0;
			y += y0;
			dx = x;
			dy = y;
			if(!pELD->PeakCenter(rad, x, y, cd, 0.5, mode, ampl, as))
				goto fried;
			esdx += SQR(x-dx);
			esdy += SQR(y-dy);
			pELD->m_pCurLZ->nLRefCnt++;
			XLat1(h, k, x, y);
			if( bDraw )
				pELD->DrawMark(x, y, 1, -3);
fried:
			dx = xf;
			dy = yf;
			if(!pELD->PeakCenter(rad, xf, yf, cd, 0.5, mode, ampl, as))
				continue;
			esdx += SQR(xf-dx);
			esdy += SQR(yf-dy);
			pELD->m_pCurLZ->nLRefCnt++;
			XLat1(-h, -k, xf, yf);
			if( bDraw )
				pELD->DrawMark(xf, yf, 1, -3);
		}
	}
	pELD->m_esdx = pELD->m_esdy = 0.0;
	if( pELD->m_pCurLZ->nLRefCnt > 1 )
	{
		pELD->m_esdx = FastSqrt(esdx / (float)(pELD->m_pCurLZ->nLRefCnt));
		pELD->m_esdy = FastSqrt(esdy / (float)(pELD->m_pCurLZ->nLRefCnt));
	}
	return XLat2(pELD);
}
/****************************************************************************/
// Modified by Peter [19/8/2005]
#if 1
static void CreateMask2( OBJ* O )
{
	LPELD	pEld = (LPELD) &O->dummy;
	HDC		hdc;
	int		r = 2, i = 0, j, c = 250, orop;
	int		xms, yms, x, y;
	HPEN	hp = NULL, op = NULL;
	HBRUSH	hb = NULL, ob = NULL;
	float	l0, l1;
	BOOL	bCentered = IsDlgButtonChecked( O->hWnd, IDC_ELD_CENTERED);
	HBITMAP	hbmp = NULL, hobmp = NULL;
	LPVOID	bits = NULL;
	int		nMaskW, nMaskH, nMaskSize;
	CVector2d m_h_dir(pEld->m_h_dir), m_k_dir(pEld->m_k_dir);
	LPBYTE	ptr;

	r = round((AbsC(m_h_dir.x, m_h_dir.y) + AbsC(m_k_dir.x, m_k_dir.y))/20.);
	if( r < 2 )
		r = 2;	// !!! was (pEld->m_al + pEld->m_bl)

	l0 = AbsC(ih[0]*m_h_dir.x + ik[0]*m_k_dir.x, ih[0]*m_h_dir.y + ik[0]*m_k_dir.y);
	l1 = AbsC(ih[1]*m_h_dir.x + ik[1]*m_k_dir.x, ih[1]*m_h_dir.y + ik[1]*m_k_dir.y);
	if( l0 > l1 )
		j = 0;
	else
		j = 1;

	double nMinX = 1e33, nMaxX = -1e33, nMinY = 1e33, nMaxY = -1e33;
	for(i = 0; i < 4; i++)
	{
		if( (i%2) == j )
			l0 = 1.;
		else
			l0 = 0.5;
		// TODO: check this l0 = 1.0
		l0 = 1.0;
		if( TRUE == bCentered )
		{
			pEld->m_ppt[i].x = round(l0*(ihc[i]*m_h_dir.x + ikc[i]*m_k_dir.x));
			pEld->m_ppt[i].y =-round(l0*(ihc[i]*m_h_dir.y + ikc[i]*m_k_dir.y));
		}
		else
		{
			pEld->m_ppt[i].x = round(l0*(ih[i]*m_h_dir.x + ik[i]*m_k_dir.x));
			pEld->m_ppt[i].y =-round(l0*(ih[i]*m_h_dir.y + ik[i]*m_k_dir.y));
		}
		if( nMinX > pEld->m_ppt[i].x )
			nMinX = pEld->m_ppt[i].x;
		if( nMaxX < pEld->m_ppt[i].x )
			nMaxX = pEld->m_ppt[i].x;
		if( nMinY > pEld->m_ppt[i].y )
			nMinY = pEld->m_ppt[i].y;
		if( nMaxY < pEld->m_ppt[i].y )
			nMaxY = pEld->m_ppt[i].y;
	}
	nMinX -= 2*r;
	nMaxX += 2*r;
	nMinY -= 2*r;
	nMaxY += 2*r;
	nMaskW = (int)floor(nMaxX - nMinX + 0.5) + 1;
	nMaskH = (int)floor(nMaxY - nMinY + 0.5) + 1;
	pEld->m_nMaskW = nMaskW;
	pEld->m_nMaskH = nMaskH;
	nMaskSize = R4(nMaskW) * nMaskH;
	for(i = 0; i < 4; i++)
	{
		pEld->m_ppt[i].x -= nMinX;
		pEld->m_ppt[i].y -= nMinY;
	}
	// get the size of the mask required
	pEld->m_vMask.resize(nMaskSize + (256 + 256 + 2048*4)*sizeof(float), 0);
	pEld->m_as	= (LPFLOAT)	(&*pEld->m_vMask.begin() + nMaskSize);
	pEld->m_ds	= 			 pEld->m_as   +   256;
	pEld->m_ws	= 			 pEld->m_ds   +   256;
	pEld->m_wn	= 			 pEld->m_ws   +  2048;
	pEld->m_wa	= 			 pEld->m_wn   +  2048;
	// Create HDC and required bitmap
	if( NULL == (hdc = CreateCompatibleDC(NULL)) )
		return;//goto errexit;
	std::vector<BYTE> vBmh(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256);
	LPBITMAPINFOHEADER pBmh = (LPBITMAPINFOHEADER)&*vBmh.begin();
	LPBYTE	bipal = (LPBYTE)pBmh + sizeof(BITMAPINFOHEADER);
	pBmh->biSize = sizeof(BITMAPINFOHEADER);
	pBmh->biPlanes = 1;
	pBmh->biBitCount = 8;
	pBmh->biCompression = BI_RGB;
	pBmh->biClrUsed = pBmh->biClrImportant = COLORS;
	pBmh->biSizeImage = nMaskSize;
// 	pBmh->biWidth = nMaskW;
	pBmh->biWidth = R4(nMaskW);		// changed by Peter 11 August 2009
	pBmh->biHeight = nMaskH;
	for(i = 0; i < 256; i++)
	{
		*bipal++ = i;
		*bipal++ = i;
		*bipal++ = i;
		*bipal++ = 0;
	}
	if( NULL == (hbmp = CreateDIBSection(hdc, (LPBITMAPINFO)pBmh, DIB_RGB_COLORS, &bits, NULL, 0)) )
		return;//goto errexit;
	hobmp = SelectBitmap( hdc, hbmp );

	memset(bits, 255, nMaskSize);

	orop = SetROP2( hdc, R2_COPYPEN );
	hp = CreatePen(PS_SOLID, r, RGB(c,c,c));
	hb = CreateSolidBrush(RGB(1,1,1));
	ob = SelectBrush( hdc, hb );
	op = SelectPen( hdc, hp );

	Polygon(hdc, pEld->m_ppt, 4);
//#ifdef _DEBUG
#if 0
	memcpy(pEld->m_vMask.begin(), bits, nMaskSize);
	{
		// added by Peter to debug the mask
		FILE *pFile = fopen("d:\\mask_poly2.txt", "wt");
		LPBYTE ptr = pEld->m_vMask.begin();
		if(NULL != pFile)
		{
			for(i=0; i<nMaskH; i++)
			{
				for(j=0; j<R4(nMaskW); j++)
				{
					fprintf(pFile, "%1X", ptr[i*R4(nMaskW)+j] & 0xF);
				}
				fprintf(pFile, "\n");
			}
			fclose(pFile);
		}
	}
#endif
	for(i = 0; i < 4; i++)
	{
		c++;
		SelectBrush( hdc, ob );
		SelectPen( hdc, op );
		DeleteObject( hb );
		DeleteObject( hp );
		hp = CreatePen(PS_SOLID, r, RGB(c,c,c));
		hb = CreateSolidBrush(RGB(c,c,c));
		SelectBrush( hdc, hb );
		SelectPen( hdc, hp );
		Ellipse( hdc, pEld->m_ppt[i].x-r, pEld->m_ppt[i].y-r, pEld->m_ppt[i].x+r, pEld->m_ppt[i].y+r);
	}

	memcpy(&*pEld->m_vMask.begin(), bits, nMaskSize );
	// added by Peter on 05 Feb 2006
	if( pEld->m_rma < 0 )
	{
		pEld->m_rma = 0;
	}
	// end of addition by Peter on 05 Feb 2006
	ptr = &*pEld->m_vMask.begin();
	pEld->m_rms = 0;
	pEld->m_rma = 128;
	for(y = j = 0; y < nMaskH; y++)
	{
		for(x = 0; x < nMaskW; x++)
		{
//			c = 2*round(AbsC<double>((i&127)-64, (i>>7)-64) );
			c = 2*round(AbsC<double>(x+nMinX, y+nMinY) );
			if( 1 == ptr[j+x] )
			{
				if( c > pEld->m_rms )
					pEld->m_rms = c;
				// Modified by Peter on 3 Feb 2006
				if( c <= 249 )
				{
					ptr[j+x] = c;
				}
				else
				{
					ptr[j+x] = 255;
				}
//				ptr[j+x] = c;
			}
			if( ptr[j+x] > 249 )
			{
				if( c < pEld->m_rma )
					pEld->m_rma = c;
			}
		}
		j += R4(nMaskW);
	}
	// ADDED by Peter on 19 June 2006
	pEld->m_rma = /*0.5 * */__min(pEld->m_pCurLZ->a_length, pEld->m_pCurLZ->b_length);
	// End of addition

	pEld->m_rms += 3;
	// Added by Peter on 3 Feb 2006
//	if( pEld->m_rms > 249 )
//		pEld->m_rms = 249;
	// end of addition by Peter on 3 Feb 2006
	pEld->m_rma--;
	xms = yms = 0;
	int nOff = 0;
	ptr = &*pEld->m_vMask.begin();
	for(i = 0; i < nMaskH; i++)
	{
		for(j = 0; j < nMaskW; j++)
		{
			if( 255 != ptr[nOff+j/*i*128+j*/] )
			{
				if( (c = abs(j + nMinX/*-64*/)) > xms )
					xms = c;
				if( (c = abs(i + nMinY/*-64*/)) > yms )
					yms = c;
			}
		}
		nOff += R4(nMaskW);
	}
	if( xms > abs(nMinX)/*63*/ )
		xms = abs(nMinX)/*63*/;
	if( yms > abs(nMinY)/*63*/ )
		yms = abs(nMinY)/*63*/;
	pEld->m_xms = xms;
	pEld->m_yms = yms;
	c = 0;
//	pntBitmapToScreen ( GetDlgItem(O->hWnd, IDC_ELD_PLOT ), pEld->m_mask, 128, 128, 0, 0, 1, 1, O, 0x1100 );

	nOff = 0;
	ptr = &*pEld->m_vMask.begin();
	for(i = 0; i < nMaskH; i++)
	{
		for(j = 0; j < nMaskW; j++)
		{
			if( (abs(i + nMinY/*-64*/) <= yms) && (abs(j + nMinX/*-64*/) <= xms) )
				ptr[c++] = ptr[nOff+j/*i*128+j*/];
		}
		nOff += R4(nMaskW);
	}
#ifdef _DEBUG
//#if 0
	{
		// added by Peter to debug the mask
		FILE *pFile = fopen("d:\\mask_after2.txt", "wt");
		LPBYTE ptr = &*pEld->m_vMask.begin();
		if(NULL != pFile)
		{
			for(i=0; i<yms*2+1; i++)
			{
				for(j=0; j<xms*2+1; j++)
				{
					fprintf(pFile/*TMP+j*/, "%1X", ptr[i*(xms*2+1)+j] & 0xF);
				}
				fprintf(pFile, "\n");
			}
			fclose(pFile);
		}
	}
#endif

	SelectBitmap( hdc, hobmp );
	DeleteObject( hbmp );
	SetROP2( hdc, orop );
	DeleteDC( hdc );
	DeleteObject( hb );
	DeleteObject( hp );
// 	g_RMS = pEld->m_rms;
	return;
//errexit:
//	MessageBeep(0xFFFF);
}
#endif

static void CreateMask( OBJ* O )
{
	LPELD	pELD = (LPELD) &O->dummy;
	HDC		hdc;
	int		r = 2, i = 0, j, c = 250, orop;
	int		xms, yms;
	HPEN	hp = NULL, op = NULL;
	HBRUSH	hb = NULL, ob = NULL;
	float	l0, l1;
	BOOL	bCentered = IsDlgButtonChecked( O->hWnd, IDC_ELD_CENTERED);
	HBITMAP	hbmp = NULL, hobmp = NULL;
	LPVOID	bits = NULL;

	memset(pELD->m_mask, 255, 16384);
	r = round((AbsC(pELD->m_h_dir.x, pELD->m_h_dir.y) + AbsC(pELD->m_k_dir.x, pELD->m_k_dir.y))/20.);
	if( r < 2 )
		r = 2;	// !!! was (pELD->m_al + pELD->m_bl)
	if( NULL == (hdc = CreateCompatibleDC(NULL)) )
		goto errexit;
	if( NULL == (hbmp = CreateDIBSection(hdc, (LPBITMAPINFO)O->bp, DIB_RGB_COLORS, &bits, NULL, 0)) )
		goto errexit;
	hobmp = SelectBitmap( hdc, hbmp );

	memset(bits, 255, 16384);

	orop = SetROP2( hdc, R2_COPYPEN );
	hp = CreatePen(PS_SOLID, r, RGB(c,c,c));
	hb = CreateSolidBrush(RGB(1,1,1));
	ob = SelectBrush( hdc, hb );
	op = SelectPen( hdc, hp );

	l0 = AbsC(ih[0]*pELD->m_h_dir.x + ik[0]*pELD->m_k_dir.x, ih[0]*pELD->m_h_dir.y + ik[0]*pELD->m_k_dir.y);
	l1 = AbsC(ih[1]*pELD->m_h_dir.x + ik[1]*pELD->m_k_dir.x, ih[1]*pELD->m_h_dir.y + ik[1]*pELD->m_k_dir.y);
	if( l0 > l1 )
		j = 0;
	else
		j = 1;

	for(i = 0; i < 4; i++)
	{
		if( (i%2) == j )
			l0=1.;
		else
			l0=0.5;
		l0 = 1.0;
		if( TRUE == bCentered )
		{
			pELD->m_ppt[i].x = round(l0*(ihc[i]*pELD->m_h_dir.x + ikc[i]*pELD->m_k_dir.x)) + 64;
			pELD->m_ppt[i].y =-round(l0*(ihc[i]*pELD->m_h_dir.y + ikc[i]*pELD->m_k_dir.y)) + 64;
		}
		else
		{
			pELD->m_ppt[i].x = round(l0*(ih[i]*pELD->m_h_dir.x + ik[i]*pELD->m_k_dir.x)) + 64;
			pELD->m_ppt[i].y =-round(l0*(ih[i]*pELD->m_h_dir.y + ik[i]*pELD->m_k_dir.y)) + 64;
		}
		if( pELD->m_ppt[i].x < 2   ) pELD->m_ppt[i].x = 2;
		if( pELD->m_ppt[i].y < 2   ) pELD->m_ppt[i].y = 2;
		if( pELD->m_ppt[i].x > 126 ) pELD->m_ppt[i].x = 126;
		if( pELD->m_ppt[i].y > 126 ) pELD->m_ppt[i].y = 126;
	}
	Polygon( hdc, pELD->m_ppt, 4);

	for(i = 0; i < 4; i++)
	{
		SelectBrush( hdc, ob );	SelectPen( hdc, op );
		DeleteObject( hb );	DeleteObject( hp ); c++;
		hp = CreatePen(PS_SOLID, r, RGB(c,c,c));
		hb = CreateSolidBrush(RGB(c,c,c));
		SelectBrush( hdc, hb );	SelectPen( hdc, hp );
		Ellipse( hdc, pELD->m_ppt[i].x-r, pELD->m_ppt[i].y-r, pELD->m_ppt[i].x+r, pELD->m_ppt[i].y+r);
	}

	memcpy( pELD->m_mask, bits, 16384 );
	for(pELD->m_rms = 0, pELD->m_rma = 128, i=0; i<16384; i++)
	{
		c = 2*round(AbsC<double>((i&127)-64, (i>>7)-64) );
		if(pELD->m_mask[i] == 1)
		{
			if(c > pELD->m_rms)
				pELD->m_rms = c;
			pELD->m_mask[i] = c;
		}
		if(pELD->m_mask[i] >249)
		{
			if(c < pELD->m_rma)
				pELD->m_rma = c;
		}
	}

	pELD->m_rms += 3;
	pELD->m_rma--;
// Added by Peter on [18/8/2005]
//	pELD->m_rma = 0.5 * __min(pELD->m_pCurLZ->a_length, pELD->m_pCurLZ->b_length);
// end of addition by Peter [18/8/2005]
	for(xms=yms=0, i=0; i<128; i++)
	{
		for(j=0; j<128; j++)
		{
			if(pELD->m_mask[i*128+j] != 255)
			{
				if((c = abs(j-64)) > xms)
					xms=c;
				if((c = abs(i-64)) > yms)
					yms=c;
			}
		}
	}
	if (xms>63) xms=63;
	if (yms>63) yms=63;
	pELD->m_xms = xms;
	pELD->m_yms = yms;
	c=0;

//	pntBitmapToScreen ( GetDlgItem(O->hWnd, IDC_ELD_PLOT ), pELD->m_mask, 128, 128, 0, 0, 1, 1, O, 0x1100 );

	for(i=0; i<128; i++)
	{
		for(j=0; j<128; j++)
		{
			if( (abs(i-64)<=yms) && (abs(j-64)<=xms) )
			{
				pELD->m_mask[c++] = pELD->m_mask[i*128+j];
			}
		}
	}

#ifdef _DEBUG
//#if 0
	// added by Peter to debug the mask
	{
		FILE *pFile = fopen("d:\\mask_after.txt", "wt");
		LPBYTE ptr = pELD->m_mask;
		// added by Peter to debug the mask
		if(NULL != pFile)
		{
			for(i=0; i<yms*2+1; i++)
			{
				for(j=0; j<xms*2+1; j++)
				{
					fprintf(pFile, "%1X", ptr[i*(xms*2+1)+j] & 0xF);
				}
				fprintf(pFile, "\n");
			}
			fclose(pFile);
		}
	}
#endif

	SelectBitmap( hdc, hobmp );
	DeleteObject( hbmp );
	SetROP2( hdc, orop );
	DeleteDC( hdc );
	DeleteObject( hb );	DeleteObject( hp );
// 	g_RMS = pELD->m_rms;
	return;
errexit:
	MessageBeep(0xFFFF);
}
/****************************************************************************/
static BOOL DoAutoLR(HWND hDlg, OBJ* pObj)
{
	EldAritemVec vTmpPeaks;
	LPELD	pEld = (LPELD)&pObj->dummy;
	int		i/*, j, l, a, pp, pp2*/;
//	float	h,/*k,*/x,y/*,xx,yy,d*/;
	float	/*ax,ay,as,s,x0,y0,*/r1,r0,r10/*,n0,n1*/;
	double	x0, y0;
	int		xc,yc,r,ds, nClosestDist;
	int		hi[4] = {0,1,1,-1}, ki[4] = {1,0,1,1};
//	WORD	amp, max;
	double	dLimitRad = 1e33;
	EldAritemVec vRefls, vTemp;
//	LPARITEM pRefls1;
//	ARITEM	ait;
	LaueZone *pLZ;
	BOOL bCheckR = FALSE;
	const int N_SIZE = /*1024*//*256*/512;

//--------------------------------------------------------------------------
	try
	{
		// if the user has Zero Order Laue Zone circle - set the limiting radius
		if( (TRUE == pEld->m_bUseLaueCircles) && (0 == pEld->m_nCurLaueZone) )
		{
			if( true == pEld->m_vbLaueCircles[0] )
			{
				dLimitRad = pEld->m_vLaueZones[0]->m_dRadius;
				bCheckR = TRUE;
			}
		}
		// Added by Peter on [8/8/2005]
		// Should be ZOLZ in fact
		pLZ = pEld->m_pCurLZ;
		// end of addition by Peter [8/8/2005]
		xc = pEld->m_image.w >> 1;
		yc = pEld->m_image.h >> 1;
		r = __min(xc, yc);
		r10 = r>>1;
		//--------------------------------------------------------------------------
// 		if( FALSE == ELD_FindCenter(pEld, x0, y0) )
		if( FALSE == pEld->ELD_FindCenter(x0, y0) )
		{
			return FALSE;
		}
		// set the center and mass center as same
		pEld->m_mass_center.x = pEld->m_center.x = x0;
		pEld->m_mass_center.y = pEld->m_center.y = y0;
		r0 = 2*r;
		r1 = 4*r;
		r = 0;
		// INITIAL axes directions
		pEld->m_h_dir.x = pEld->m_k_dir.y = 10;
		pEld->m_h_dir.y = pEld->m_k_dir.x = 0;
		// The size of the peak-hunt window is 10 pixels
		ds = pEld->m_nPeakWindow;
		// the maximum allowed distance (if less - the reflection is same)
		nClosestDist = ds >> 1;
		if( r1 < r10 )
			r1 = r10;
		// PEAK HUNTING
		Trace(_T("peak search..."));
		// initialize the list of reflections
		vTemp.resize(N_SIZE);
//		if( (r = ELD_DoPeakSearch2(O, vTemp, ds, dLimitRad, bCheckR)) < 2 )
// 		if( (r = ELD_DoPeakSearch(pObj, vTemp, ds, dLimitRad, bCheckR)) < 2 )
		if( (r = pEld->ELD_DoPeakSearch(vTemp, ds, dLimitRad, bCheckR)) < 2 )
		{
			return FALSE;
		}
		// mark each peak on the DP
		for(i = 0; i < r; i++)
		{
			pEld->DrawMark(vTemp[i].x, vTemp[i].y, pLZ->hPen, -3);
//			sprintf(TMP, "%7.2f\t%7.2f\n", vTemp[i].x, vTemp[i].y);
//			OutputDebugString(TMP);
		}
		Trace(_FMT(_T("found %d peaks\n"), r));

		vTmpPeaks.resize(r);
		memcpy(&*vTmpPeaks.begin(), &*vTemp.begin(), r*sizeof(ARITEM));
		Trace(_T("analyzing peaks...\n"));
// 		if( FALSE == ELD_AnalyzePeaks(pObj, pEld, vTmpPeaks, dLimitRad, bCheckR) )
		if( FALSE == pEld->ELD_AnalyzePeaks(vTmpPeaks, dLimitRad, bCheckR) )
		{
			return FALSE;
		}

		Trace(_T("4 passes of the refinement...\n"));
		pLZ->center = pEld->m_center;
		vTmpPeaks.clear();
		for(int iPass = 0; iPass < 4; iPass++)
		{
			double dErrPrec = 0.5;
			if( iPass >= 3 )
			{
				dErrPrec = 0.25;
			}
			Trace(_FMT(_T("\tpass %d...\n"), (iPass+1)));
			// on the last step fill the successfully detected peaks which lie close
			// to the lattice points. This will be used during the search of nearest 3 reflections.
			if( 3 == iPass )
			{
				if( FALSE == pEld->ELD_RefineLattice(iPass, dErrPrec, dLimitRad, bCheckR, &vTmpPeaks) )
				{
					break;
				}
			}
			else
			{
				if( FALSE == pEld->ELD_RefineLattice(iPass, dErrPrec, dLimitRad, bCheckR) )
				{
					break;
				}
			}
		}

		//////////////////////////////////////////////////////////////////////////
		Trace(_T("updating 3 reflections...\n"));

		pEld->RecalculateUserRefls(pLZ, &*vTmpPeaks.begin(), vTmpPeaks.size());

		// Finally refined successfully
		pEld->m_bRefinedLattice = TRUE;
		pLZ->m_nFlags = LaueZone::ZONE_AUTO_INDEXED;
		EnableWindow(GetDlgItem(hDlg, IDC_ELD_DEFINE_HOLZ), pEld->m_bRefinedLattice);
		return TRUE;
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("DoAutoLR() -> %s\n"), e.what()));
		return FALSE;
	}
}

/****************************************************************************/
// void protocolLatRef( OBJ* O )
void ELD::protocolLatRef()//ProtocolOut &output)
{
	float	h, k;//, as;
	HWND	hDlg = m_pObj->hWnd;
	int		nRad = 8;
	float	dval, x, y;
	BOOL	bCentered = IsDlgButtonChecked( hDlg, IDC_ELD_CENTERED );
	CVector2d pt2, pt2m;
// 	DWORD	ampl;
	double ampl, as;
	double	dScale, dAspect;
	PROTOCOL_REFL refl;
	static LPCSTR pszHeaderCalibr = "  h   k  d-val   Xest    Yest    Xobs    Yobs    dX    dY   ";
	static LPCSTR pszHeader = "  h   k   Xest    Yest    Xobs    Yobs    dX    dY   ";
	std::string sString;

// 	last = SendDlgItemMessage(hDlg, IDC_ELD_LIST, CB_GETCOUNT, 0, 0);
//	last = m_ProtocolOut.GetCount();
	if( !m_bProtocol )
		return;
	if( TRUE == IsCalibrated() )
	{
// 		output << "  h   k  d-val    Xest    Yest    Xobs    Yobs    dX    dY   ";
// 		output << "------------------------------------------------------------";
		m_ProtocolOut.AddProtocolString(pszHeaderCalibr);
		m_ProtocolOut.SetHeader(pszHeaderCalibr);
		m_ProtocolOut.AddProtocolString("------------------------------------------------------------");
	}
	else
	{
// 		output << "  h   k    Xest    Yest    Xobs    Yobs    dX    dY   ";
// 		output << "------------------------------------------------------";
		m_ProtocolOut.AddProtocolString(pszHeader);
		m_ProtocolOut.SetHeader(pszHeader);
		m_ProtocolOut.AddProtocolString("------------------------------------------------------");
	}
	// clear the reflections list
	if( m_nCurPass < 0 ) m_nCurPass = 0;
	if( m_nCurPass >= ELD_MAX_PASSES_LATREF ) m_nCurPass = 0;
	m_aPasses[m_nCurPass].vRefls.clear();
	// some scale factors
	dScale = m_pObj->NI.Scale * 1e10;
	dAspect = m_pObj->NI.AspectXY;
	// print all detected peaks
	for(h = -m_hkmax; h <= m_hkmax; h += 1.0)
	{
		for(k = -m_hkmax; k <= m_hkmax; k += 1.0)
		{
			if( (TRUE == bCentered) && ((round(h) + round(k)) & 1) )
				continue;
			if( (FastAbs(h) < 0.1) && (FastAbs(k) < 0.1) )
				continue;
			pt2 = pt2m = m_pCurLZ->GetPeakAbsXY(h, k);
//			x = xm = h*pELD->m_h_dir.x + k*pELD->m_k_dir.x + pELD->m_center.x;
//			y = ym = h*pELD->m_h_dir.y + k*pELD->m_k_dir.y + pELD->m_center.y;
			if( (pt2.x < nRad) || (pt2.y < nRad) ||
				(pt2.x >= m_image.w - nRad) || (pt2.y >= m_image.h - nRad) )
			{
				continue;
			}
			// Added by Peter on 01 Nov 2005
			x = pt2.x;
			y = pt2.y;
			// End of addition by Peter on 01 Nov 2005
			if(!PeakCenter(nRad, x, y, nRad, 0.5, TRUE, ampl, as))
				continue;
			pt2.x = x;
			pt2.y = y;
			// print H and K
			sString = _FMT(_T("%3d %3d "), (int)h, (int)k);
			//sprintf(TMP, "%3d %3d ", (int)h, (int)k);
			refl.h = (int)h;
			refl.k = (int)k;
			refl.dval = 0.0;
			refl.pt2 = pt2;
			refl.pt2m = pt2m;
			if( TRUE == IsCalibrated() )
			{
				dval = /*O->NI.Scale * 1e10*/dScale /
					AbsC(m_pCurLZ->GetPeakRelX(h, k), m_pCurLZ->GetPeakRelY(h, k)*dAspect/*O->NI.AspectXY*/);
//					AbsC(pELD->m_h_dir.x*h+pELD->m_k_dir.x*k, (pELD->m_h_dir.y*h+pELD->m_k_dir.y*k)*O->NI.AspectXY);
				refl.dval = dval;
				sString += _FMT(_T("%6.2f "), dval);
				// sprintf(TMP_End, "%6.2f ", dval);
			}
			sString += _FMT(_T("%7.2f %7.2f %7.2f %7.2f %5.2f %5.2f"),
					pt2m.x, pt2m.y, pt2.x, pt2.y, (pt2.x - pt2m.x), (pt2.y - pt2m.y));
//			output << TMP;
			m_aPasses[m_nCurPass].vRefls.push_back(refl);
			m_ProtocolOut.AddProtocolString(sString);
			// AddProtocolString(TMP);
		}
	}
}
/****************************************************************************/
void ELD::protocolLatAxies(int nrefl, int pass)//, ProtocolOut &output)
{
//	LPELD	pELD = (LPELD) &O->dummy;
//	HWND	hDlg = m_pObj->hWnd;

	if( !m_bProtocol )
		return;
	if( 0 == pass )
	{
		m_ProtocolOut.AddProtocolString(_FMT(_T("; File: %s, Lattice refinement"), m_pObj->fname));
		m_ProtocolOut.AddProtocolString(";--------------------------------------------");
// 		sprintf(TMP,"; File: %s, Lattice refinement", m_pObj->fname);
// 		m_ProtocolOut.AddProtocolString(TMP);
// 		m_ProtocolOut.AddProtocolString(";--------------------------------------------");
	}

//	sprintf(TMP, ";Pass%1d 0(%.2f,%.2f) A(%.2f,%.2f) B(%.2f,%.2f) Nr=%d",
//						pass, X0, Y0, HX, HY, KX, KY, nrefl);
	// modified by Peter on 01 Nov 2005 - sign of Y-component is changed
	std::string sString = (_FMT(_T(";Pass%1d 0(%.2f,%.2f) A(%.2f,%.2f) B(%.2f,%.2f) Nr=%d")
		, pass
		, m_center.x , m_center.y		// center
		, m_h_dir.x , (-m_h_dir.y)		// h-axis
		// Changed sign to "+" of "pELD->m_k_dir.y" by Peter on 02 Nov 2005, so Trice can read correctly
//		pELD->m_k_dir.x,  pELD->m_k_dir.y,		// k-axis
		, m_k_dir.x , (-m_k_dir.y)		// k-axis
		, nrefl));
	m_ProtocolOut.AddProtocolString(sString);
	if( m_nCurLaueZone > 0 )
	{
		// the shift
		sString += (_FMT(_T("pELD(%.2f,%.2f)"), m_pCurLZ->shift.x, m_pCurLZ->shift.y));
		m_ProtocolOut.AddProtocolString(sString);
	}
	// ESDs
	m_ProtocolOut.AddProtocolString(_FMT(_T(";  ESD(%.2f,%.2f)"), m_esdx, m_esdy));
	m_ProtocolOut.SetCurSel(m_ProtocolOut.GetCount() - 2);
}

/****************************************************************************/
BOOL ELD::DoLatRef(int autoref)
{
	float	hkmax = 0, rmin = 1e20, rmax = 0;
	int		nLaueZone;
	LPBYTE	map = m_map;
	float	radii;
	float	cf = m_pObj->NI.AspectXY;
	BOOL	bLROK, bCheckR;
	double	dMinR, dMaxR;//, dDist2;

	HWND	hDlg = m_pObj->hWnd;
	nLaueZone = m_nCurLaueZone;
	m_pCurLZ->nEstCount = 0;		// No Int. estimations
	m_hkmaxe = 0;		// Maximum hk for extraction
	m_flags = 0;

	// clear all reflections in the current Laue Zone
	m_pCurLZ->m_vRefls.clear();
	bCheckR = FALSE;
	dMinR = 0.0;
	dMaxR = 1e+33;

	if( autoref )
	{
		// if we are trying to index HOLZ automatically (only for zones other than 0).
		if( TRUE == m_bUseLaueCircles )
		{
			// check if the Current Laue Zone was defined properly
			if( true == m_vbLaueCircles[nLaueZone] )
			{
				if( nLaueZone > 0 )
				{
					BOOL bRes = DoAutoHOLZ();
					Params2Dlg(hDlg, this);
					DrawELD(m_pObj);
					return bRes;
				}
				else
				{
					bCheckR = TRUE;
					dMaxR = m_vLaueZones[nLaueZone]->m_dRadius;
				}
			}
			else
			{
				MessageBox(NULL, "The Laue Zone has not been initialized yet.\r\n"
					"Every Laue Zone requires 2 limiting circles.",
					"ELD ERORR", MB_OK);
				return FALSE;
			}
		}
		if( FALSE == DoAutoLR(hDlg, m_pObj) )
		{
			DrawELD(m_pObj);
			return FALSE;
		}
		else
		{
			goto Continue_REF;
		}
	}
	else if( (TRUE == m_bUseLaueCircles) && (nLaueZone > 0) )
	{
		if( true == m_vbLaueCircles[nLaueZone] )
		{
			// TODO: check if reflections were picked
			BOOL bRes = DoHOLZref(nLaueZone);
			Params2Dlg(hDlg, this);
			RepaintImgWindow(m_pObj);
			DrawELD(m_pObj);
			return bRes;
		}
		else
		{
			MessageBox(m_pObj->hWnd, "You didn't define the Laue Zone!", "ELD ERROR", MB_OK);
			return FALSE;
		}
	}
	// initialize some parameters
	XLat0();
	if( FALSE == autoref )
	{
		ELD_CalculateUserLattice();
	}
	else // if we have (TRUE == autoref)
	{
		// Square 9x9
		double a = AbsC(m_h_dir.x, m_h_dir.y);
		double b = AbsC(m_k_dir.x, m_k_dir.y);
		rmin  = __min(a, b)*0.5;
		rmax  = __max(a, b)*3.0;
		radii = __min(a, b)*0.5;
		hkmax = 1.;
	}
	// moved here from the place down by Peter on 11 Sep 2001
	if( autoref && R2D(AB_Angle( m_h_dir.x, m_h_dir.y*cf, m_k_dir.x, m_k_dir.y*cf)) < 90.0 )
	{
		std::swap(m_h_dir, m_k_dir);
	}

	bLROK = TRUE;

	if( !bLROK )
	{
		if( autoref )
			return FALSE;
		MessageBox( MainhWnd, "Lattice not refined, will use user defined",
			"ELD ERROR", MB_OK | MB_ICONINFORMATION );
	}

Continue_REF:
	CreateMask2(m_pObj);

	if( 1 == autoref )
	{
		m_pCurLZ->m_nFlags = LaueZone::ZONE_AUTO_INDEXED;
	}
	else
	{
		m_pCurLZ->m_nFlags = LaueZone::ZONE_MANUALLY_INDEXED;
	}

	RecalculateUserRefls(m_pCurLZ);

	XRef(m_pObj, 0, 1e6, hkmax, 5, 3., TRUE, FALSE);

	Params2Dlg(hDlg, this);
	DrawELD(m_pObj);		// Update 3 refls

	// Finally refined successfully
	if( FALSE == m_bRefinedLattice )
	{
		m_bRefinedLattice = TRUE;
		EnableWindow(GetDlgItem(hDlg, IDC_ELD_DEFINE_HOLZ), m_bRefinedLattice);
	}
	// end of addition

	protocolLatRef();

	return TRUE;
}
/*-------------------------------------------------------------------------*/
static LRESULT ChangeItemColors( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam )
{
	HDC		hdc = (HDC)wParam;
	HWND	hwnd = (HWND)lParam;
	int		idc = GetDlgCtrlID(hwnd);
	int		i;
	LRESULT	ret;
	static	BOOL brec=FALSE;
//	HBRUSH	hBrush;
//	RECT	rc;

	if (brec) return FALSE;
	brec = TRUE;
	ret = DefDlgProc(hDlg, msg, wParam, lParam);

	for(i = 0; i < 3; i++)
	{
		if (idc == idc_REF[i])	break;
		if (idc == idc_h[i])	break;
		if (idc == idc_k[i])	break;
	}
	switch(idc)
	{
		case IDC_ELD_AS: i = 0; break;
		case IDC_ELD_BS: i = 1; break;
		case IDC_ELD_CS: i = 2; break;
//		case IDC_ELD_YELLOW_TYPE: i = 4; hBrush = ::CreateSolidBrush(RGB(255, 255, 0)); break;
//		case IDC_ELD_BLUE_TYPE: i = 4; hBrush = ::CreateSolidBrush(RGB(0, 255, 255)); break;
//		case IDC_ELD_VIOLET_TYPE: i = 4; hBrush = ::CreateSolidBrush(RGB(255, 0, 255)); break;
//		case IDC_ELD_RED_TYPE: i = 4; hBrush = ::CreateSolidBrush(RGB(255, 0, 0)); break;
	}
	switch( i )
	{
		case 0:	SelectPen(hdc, RedPen);   SetTextColor(hdc, RGB(255,0,0)); break;
		case 1:	SelectPen(hdc, BluePen);  SetTextColor(hdc, RGB(0,0,255)); break;
		case 2:	SelectPen(hdc, GreenPen); SetTextColor(hdc, RGB(0,160,0)); break;
/*
		case 4:
			{
				if(hBrush && hwnd && hdc)
				{
					GetClientRect(hwnd, &rc);
					FillRect(hdc, &rc, hBrush);
					::DeleteObject(hBrush);
				}
				break;
			}
*/
	}

	brec = FALSE;
	return ret;
}
/*-------------------------------------------------------------------------*/
void EnableRefine(OBJ* O, LaueZone *pLZ)
{
	int		i;
	BOOL	f = TRUE;

//	if( TRUE == pLZ->bAuto )
//	{
//		EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_REFINE), TRUE);
//		EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_DOALL), TRUE);
//		EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_CENTERED), FALSE);
//		for(i = 0; i < 3; i++)
//		{
//			CheckDlgButton(O->hWnd, idc_REF[i], FALSE);
//			EnableWindow(GetDlgItem(O->hWnd, idc_REF[i]), FALSE);
//			EnableWindow(GetDlgItem(O->hWnd, idc_h[i]), FALSE);
//			EnableWindow(GetDlgItem(O->hWnd, idc_k[i]), FALSE);
//		}
//	}
//	else
	if( FALSE == pLZ->bAuto )
	{
		f = ( pLZ->x[0]*pLZ->x[1]*pLZ->x[2]*pLZ->y[0]*pLZ->y[1]*pLZ->y[2] > 1.0 );
//		EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_REFINE), f);
//		EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_DOALL), f);
//		EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_CENTERED), TRUE);
/*
		for(i = 0; i < 3; i++)
		{
			EnableWindow(GetDlgItem(O->hWnd, idc_REF[i]), TRUE);
			EnableWindow(GetDlgItem(O->hWnd, idc_h[i]), TRUE);
			EnableWindow(GetDlgItem(O->hWnd, idc_k[i]), TRUE);
		}
*/
	}
	EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_REFINE), f);
	EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_DOALL), f);
	EnableWindow(GetDlgItem(O->hWnd, IDC_ELD_CENTERED), !pLZ->bAuto);
	for(i = 0; i < 3; i++)
	{
		if( TRUE == pLZ->bAuto )
		{
			CheckDlgButton(O->hWnd, idc_REF[i], FALSE);
		}
		EnableWindow(GetDlgItem(O->hWnd, idc_REF[i]), !pLZ->bAuto);
		EnableWindow(GetDlgItem(O->hWnd, idc_h[i]), !pLZ->bAuto);
		EnableWindow(GetDlgItem(O->hWnd, idc_k[i]), !pLZ->bAuto);
	}
}
/*-------------------------------------------------------------------------*/
BOOL CALLBACK ELDSaveDlgProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ* obj = (OBJ*)GetWindowLong( hDlg, DWL_USER );
	//LPELD pELD = (LPELD)GetWindowLong( hDlg, DWL_USER );
	ELD* pELD = (ELD*)&GetBaseObject(obj)->dummy;

	switch( message )
	{
	case WM_INITDIALOG:
		obj = (OBJ*)lParam;
		pELD = (ELD*)&OBJ::GetOBJ(obj->ParentWnd)->dummy;
		obj->hWnd = hDlg;
		if (!pELD->m_bProtocol)
		{
			obj->AddWarningMessage(_T("To use the HKE file in Trice 'protocol' must be enabled"));
		}
		SetWindowLong( hDlg, DWL_USER, lParam );
		SetDlgItemDouble( hDlg, IDC_ELS_DVFROM, pELD->m_dvfrom, 0 );
		SetDlgItemDouble( hDlg, IDC_ELS_DVTO, pELD->m_dvto, 0 );
		SetDlgItemDouble( hDlg, IDC_ELS_INFROM, pELD->m_infrom*256., 0 );
		SetDlgItemDouble( hDlg, IDC_ELS_INTO, pELD->m_into*256., 0 );
		CheckDlgButton( hDlg, IDC_ELS_CENTONLY, pELD->m_bCentOnly );
		regReadWindowPos (hDlg, "ELD save");
		return TRUE;

	case WM_SIZE:
	case WM_MOVE:
		obj->UpdateInfoWindowsPos();
		return FALSE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemDouble( hDlg, IDC_ELS_DVFROM, &pELD->m_dvfrom, 0 );
			GetDlgItemDouble( hDlg, IDC_ELS_DVTO, &pELD->m_dvto, 0 );
			if (GetDlgItemDouble( hDlg, IDC_ELS_INFROM, &pELD->m_infrom, 0 ))  pELD->m_infrom /= 256.;
			if (GetDlgItemDouble( hDlg, IDC_ELS_INTO, &pELD->m_into, 0 ))      pELD->m_into   /= 256.;
			pELD->m_bCentOnly = IsDlgButtonChecked( hDlg, IDC_ELS_CENTONLY );
			regWriteWindowPos(hDlg, "ELD save", FALSE);
			EndDialog( hDlg, TRUE );
			return TRUE;

		case IDCANCEL:
			EndDialog( hDlg, FALSE );
			return TRUE;
		}
	}
	return FALSE;
}
/****************************************************************************/
// Added by Peter on [3/8/2005]
static BOOL CloseELDDialog( HWND hDlg, OBJ* lpObj, LPELD pEld )
{
	pEld->Deinitialize();
//	LaueZoneVecIt it;

//	if( false == pEld->m_vLaueZones.empty() )
//	{
//		for(it = pEld->m_vLaueZones.begin(); it != pEld->m_vLaueZones.end(); ++it)
//		{
//			(*it)->Deinit();
////			// DO NOT delete ZOLZ - it is not dynamically allocated
////			if( it == pEld->m_vLaueZones.begin() )
////			{
////				continue;
////			}
//			delete *it;
//		}
//		pEld->m_vLaueZones.clear();
//	}
	return TRUE;
}

void ELD::MakeHisto()
{
	typedef LONG HisType;
	std::vector<HisType> histo;
	size_t nMax;
	int i, l, k;
	double 	an = 0.0, as = 0.0, an0;
	HisType *his;

	m_image.CalculateHisto(histo);
	an0 = round(0.005*(double)m_image.w * (double)m_image.h);
	if( an0 < 1 )
		an0 = 1;
	bool bScale = false;
	switch( m_image.npix )
	{
	case PIX_BYTE: nMax = 255; break;
	case PIX_CHAR: nMax = 255; bScale = true; break;
	case PIX_WORD: nMax = 65535; break;
	case PIX_DWORD:
	case PIX_SHORT:
	case PIX_LONG:
	case PIX_FLOAT:
	case PIX_DOUBLE: nMax = 65535; bScale = true; break;
	}
	m_min = 0;
	m_max = nMax;
	his = &*histo.begin();

	double dScale = 1.0;
	if( true == bScale )
	{
		dScale = (m_image.m_dMax - m_image.m_dMin) / double(nMax);
	}
	int nImgSize = m_image.w * m_image.h;
	for(i = nMax; i >= 0; i--)
	{
		if( his[i] >= 10 )
		{
			break;
		}
	}
	m_n10Histo = i;
// 	for(i = 0; i <= nMax; i++)
// 	{
// 		if( an < an0 )
// 			m_min = i;
// 		as += (float)i*(float)his[i];
// 		an += (float)his[i];
// 	}
	for(i = 0; i <= nMax; i++)
	{
		LONG value = his[i];
		if( an < an0 )
		{
			m_min = i;
		}
		as += value * ( (true == bScale) ? (i * dScale + m_image.m_dMin) : i );
		an += value;
	}
	m_mean = round(as/an);
	an = 0;
	for(i = nMax; i > 0; i--)
	{
		if(an < an0)
			m_max = i;
		an += (float)his[i];
	}
	k = (m_min + m_max)*0.5;
	for(an = 0, l = m_min; l<k; l++)
		an += (float)his[l];
	for(as = 0, l = k; l < m_max; l++)
		as += (float)his[l];
	m_image.SetNegative(as > an);
//	if( as < an )
//		m_image.neg = 0;
//	else
	if( as > an )
	{
		// TODO: nMax for DWORD & FLOAT
//		m_image.neg = nMax;//0xFF;
		switch( m_image.npix )
		{
		case PIX_BYTE:
		case PIX_CHAR:
			{
				BYTE tmp = ((BYTE)m_min) ^ 0xFF;
				m_min = ((BYTE)m_max) ^ 0xFF;
				m_max = tmp;
				break;
			}
		case PIX_WORD:
		case PIX_SHORT:
			{
				WORD tmp = (WORD)m_min ^ 0xFFFF;
				m_min = (WORD)m_max ^ 0xFFFF;
				m_max = tmp;
				break;
			}
		case PIX_DWORD:
		case PIX_LONG:
			{
				DWORD tmp = (DWORD)m_min ^ 0xFFFFFFFF;
				m_min = (DWORD)m_max ^ 0xFFFFFFFF;
				m_max = tmp;
				break;
			}
		case PIX_FLOAT:
			{
//				float tmp = -m_min;// ^ 0xFF;
//				m_min = -m_max;// ^ 0xFF;
//				m_max = tmp;
				break;
			}
		case PIX_DOUBLE:
			{
//				float tmp = -m_min;// ^ 0xFF;
//				m_min = -m_max;// ^ 0xFF;
//				m_max = tmp;
				break;
			}
		}
//		i = m_min ^ nMax;//0xFF;
//		m_min = m_max ^ nMax;//0xFF;
//		m_max = i;
	}
// 	if( true == bScale )
// 	{
// 		m_min = m_min * dScale + m_image.m_dMin;
// 		m_max = m_max * dScale + m_image.m_dMin;
// 	}
	m_min = m_image.m_dMin;
	m_max = m_image.m_dMax;
}
/*
void MakeHisto(OBJ* lpPar, LPELD pEld, int nSize)
{
std::vector<LONG> histo(nSize);
size_t nMax = nSize - 1;
int i, l, k;
float 	an=0, as=0, an0;
LONG *his = histo.begin();

  pEld->m_min = 0;
  pEld->m_max = nMax;
  if( nMax > 256 )
  {
		im16GetHisto(his, (LPWORD)IMG(pEld), IMGW(pEld), 8, 8,
		IMGW(pEld)-16, IMGH(pEld)-16, FALSE);
		an0 = round(0.005*(float)IMGW(pEld) * (float)IMGH(pEld));
		if( an0 < 1 )
		an0 = 1;
		}
		else
		{
		im8GetHisto(his, (LPBYTE)IMG(pEld), IMGW(pEld), 8, 8,
		IMGW(pEld)-16, IMGH(pEld)-16, FALSE);
		an0 = round(0.0005*(float)IMGW(pEld) * (float)IMGH(pEld));
		}
		for(i = 0; i <= nMax; i++)
		{
		if( an < an0 )
		pEld->m_min = i;
		as += (float)i*(float)his[i];
		an += (float)his[i];
		}
		pEld->m_mean = round(as/an);
		an = 0;
		for(i = nMax; i > 0; i--)
		{
		if(an < an0)
		pEld->m_max = i;
		an += (float)his[i];
		}
		k = (pEld->m_min + pEld->m_max)*0.5;
		for(an = 0, l = pEld->m_min; l<k; l++)
		an += (float)his[l];
		for(as = 0, l = k; l < pEld->m_max; l++)
		as += (float)his[l];
		if( as < an )
		pEld->m_neg = 0;
		else
		{
		pEld->m_neg = nMax;//0xFF;
		i = pEld->m_min ^ nMax;//0xFF;
		pEld->m_min = pEld->m_max ^ nMax;//0xFF;
		pEld->m_max = i;
		}
		}
*/

class Destroyer
{
	Destroyer()
	{
	}
	Destroyer(const Destroyer&)
	{
	}
	Destroyer &operator()(const Destroyer&)
	{
	}
public:
	~Destroyer()
	{
		std::vector<HGDIOBJ>::iterator it;
		for(it = vObjects.begin(); it != vObjects.end(); ++it)
		{
			::DeleteObject(*it);
		}
		vObjects.clear();
	}
	static Destroyer &instance()
	{
		static Destroyer _Destroyer;
		return _Destroyer;
	}
	void RegisterObj(HGDIOBJ hObj)
	{
		if( NULL != hObj )
		{
			vObjects.push_back(hObj);
		}
	}

protected:
	std::vector<HGDIOBJ> vObjects;
};

// End of addition [3/8/2005]
/****************************************************************************/
BOOL InitELDDialog( HWND hDlg, OBJ* O, LPELD pEld )
{
	OBJ*	pr  = OBJ::GetOBJ(O->ParentWnd);
	WORD	i;
//	int		l;
	LPBITMAPINFOHEADER bb;
	LPBYTE	bipal;

/*
	wchar_t	temp[256], *ptr;
	ptr = temp;
	for(i = 945; i < 1056; i++)
	{
		ptr += swprintf(ptr, L"%c", i);
	}
	MessageBoxW(NULL, temp, L"UNICODE", MB_OK);
*/

//----------------------------------------------------------------------
	// Added by Peter on [3/8/2005]
	// check if we have permission for the Spinning Star in the LIC file
	if( 0 != (scbCrisp.flags & CRISP_ELD_SPINSTAR) )
	{
		s_dwELD_with_SpinnigStar = IDC_ELD_LAUE_ZONES;
	}
	else
	{
		s_dwELD_with_SpinnigStar = IDC_ELD_DEFINE_HOLZ;
	}
	// add maximum number of LAUE ZONES
	HWND hCombo = GetDlgItem(hDlg, IDC_ELD_LAUE_ZONE_NR);
	for(i = 0; i < ELD_MAX_LAUE_CIRCLES; i++)
	{
		sprintf(TMP, "%d", i);
		ComboBox_AddString(hCombo, TMP);
	}
	ComboBox_SetCurSel(hCombo, 0);
	// add all Intensity types
	hCombo = GetDlgItem(hDlg, IDC_ELD_INTENSITY_TYPE);
	ComboBox_AddString(hCombo, "Shape fitting");
	ComboBox_AddString(hCombo, "Shape integration");
	ComboBox_AddString(hCombo, "Pixel summation");
	ComboBox_SetCurSel(hCombo, 0);
	// create space for the Laue circles data
//	pEld->m_vbLaueCircles.resize(ELD_MAX_LAUE_CIRCLES);
	// set all colors and as un-initialized
//	fill_n(pEld->m_vbLaueCircles.begin(), pEld->m_vbLaueCircles.size(), false);
	// Current LAUE ZONE
//	pEld->m_nCurLaueZone = 0;
	// We always have the ZOLZ
//	pEld->m_pZOLZ = new LaueZone(0, pEld);
//	pEld->m_vLaueZones.push_back(pEld->m_pZOLZ);
	// call constructor explicitly
//	pEld->m_pCurLZ = pEld->m_pZOLZ;
//	for(i = 1; i < ELD_MAX_LAUE_CIRCLES; i++)
//	{
//		pEld->m_vLaueZones.push_back(new LaueZone(i, pEld));
//	}
//	for(i = 0; i < ELD_MAX_LAUE_CIRCLES; i++)
//	{
//		pEld->m_vLaueZones[i]->m_pszImgName = O->title;
//	}
	TextToDlgItemW(hDlg, IDC_ELD_LZ_NUM, "LZ = %d", pEld->m_nCurLaueZone);
	// The lattice has not been refined YET
//	pEld->m_bRefinedLattice = FALSE;
	// End of addition [3/8/2005]
	// ADDED BY PETER ON 23 JULY 2005
	// FOR HOLZ DEFINITION
//	pEld->m_bPaintLaueCirc = TRUE;
//	pEld->m_bPaintHOLZaxes = TRUE;
//	pEld->m_bDrawAxes = TRUE;
//	pEld->m_bMarks = FALSE;
	CheckDlgButton(hDlg, IDC_ELD_SHOW_LAUE_CIRCLES, 1);
	CheckDlgButton(hDlg, IDC_ELD_SHOW_HOLZ_AXES, 1);
	CheckDlgButton(hDlg, IDC_ELD_DRAWAXES, 1);
	// Since the lattice has not been refined yet - disable the Check box for Laue Zones
	EnableWindow(GetDlgItem(hDlg, IDC_ELD_DEFINE_HOLZ), FALSE);
	Params2Dlg(hDlg, pEld);
	if( NULL == g_pfnSPINSTAR )
	{
		HWND hWnd = GetDlgItem(hDlg, IDC_ELD_CALL_SPIN_STAR);
		ShowWindow(hWnd, SW_HIDE);
		EnableWindow(hWnd, FALSE);
	}
	// END OF ADDITION BY PETER ON 23 JULY 2005
//----------------------------------------------------------------------
//	if( sizeof(ELD) > sizeof(O->dummy) )
//		return FALSE; // Very important
//----------------------------------------------------------------------
//	IMG(pEld)  = O->dp;
//	IMGW(pEld)	= R4(O->x);
//	IMGH(pEld)	= O->y;
//----------------------------------------------------------------------
//	l  = sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256;
//	l += 16384;					// mask
//	l +=   256*sizeof(float);	// as
//	l +=   256*sizeof(float);	// ds
//	l +=  2048*sizeof(float);	// ws
//	l +=  2048*sizeof(float);	// wn
//	l +=  2048*sizeof(float);	// wa
//	l +=  4096*sizeof(POINT);	// ppt
//	l +=  4096*sizeof(BYTE);	// map
//	l +=     1*sizeof(EREFL);	// R

//	O->bp = (LPBYTE)calloc( 1, l + 16384 + 4*(256*2+2048*3+4096) + 4096 + sizeof(EREFL));
//	if ( NULL == (O->bp = (LPBYTE)calloc( 1, l*8 )) )
//		return FALSE;
//---------------------------------------------
//	pEld->m_mask = O->bp + l;                     //
//	pEld->m_as	= (LPFLOAT)	(pEld->m_mask + 16384);   //
//	pEld->m_ds	= 			 pEld->m_as   +   256;    //
//	pEld->m_ws	= 			 pEld->m_ds   +   256;
//	pEld->m_wn	= 			 pEld->m_ws   +  2048;
//	pEld->m_wa	= 			 pEld->m_wn   +  2048;
//	pEld->m_ppt	= (LPPOINT)	(pEld->m_wa   +  2048);
//	pEld->m_map	= (LPBYTE)	(pEld->m_ppt	 +	4096);
//	pEld->m_pCurRefl	= (LPEREFL)	(pEld->m_map	 +  4096);
//---------------------------------------------

	O->npix = PIX_BYTE;			// used in the map painting
	O->imin= 0;
	O->imax= 255;

//	pEld->m_arad = 1;		// Average refl radius
//	pEld->m_bMarks = TRUE;	// Draw marks
//	pEld->m_meanampl = 1;	// Mean amplitude value

	bb = (LPBITMAPINFOHEADER) O->bp;
	bipal = O->bp+sizeof(BITMAPINFOHEADER);
	bb->biSize = sizeof(BITMAPINFOHEADER);
	bb->biPlanes = 1;
	bb->biBitCount = 8;
	bb->biCompression = BI_RGB;
	bb->biClrUsed = bb->biClrImportant = COLORS;
	bb->biSizeImage = 128*128;
	bb->biWidth = 128;
	bb->biHeight = 128;
	for(i = 0; i < 256; i++)
	{
		*bipal++ = i;
		*bipal++ = i;
		*bipal++ = i;
		*bipal++ = 0;
	}
//----------------------------------------------------------------------
	// Modified by Peter on [3/8/2005]
//	O->dp = (LPBYTE)calloc( 1, RFLMEM );
//	if ( ! O->dp )
//		return FALSE;
//	pEld->m_rptr = (LPEREFL)O->dp;

	O->dp = NULL;

	// End of modification by Peter on [3/8/2005]
//----------------------------------------------------------------------
//	// Modified by Peter on [3/8/2005]
//	pEld->m_pix = pr->pix;
//	if( 1 == pr->pix )
//	{
//		MakeHisto(pr, pEld, 256);
//	}
//	else if(2 == pr->pix )
//	{
//		MakeHisto(pr, pEld, 65536);
//	}
//	// End of modification [3/8/2005]
//----------------------------------------------------------------------
	if( FALSE == bGdiInitialized )
	{
		PEN[0] = (HPEN)GetStockObject(BLACK_PEN);
		PEN[1] = RedPen;
		PEN[2] = YellowPen;
		PEN[3] = GrayPen;
		PEN[4] = GreenPen;
		PEN[5] = MagentaPen;
		PEN[6] = BluePen;
		PEN[7] = (HPEN)GetStockObject(WHITE_PEN);
		if( NULL == PEN[8] )
		{
			PEN[8] = CreatePen(PS_SOLID, 1, RGB(0,255,255));
		}
		for(int i = 0; i < 8; i++)
		{
			Destroyer::instance().RegisterObj(PEN[i]);
		}
		bGdiInitialized = TRUE;
	}
//	pEld->m_flags = 0;
//	memset(pEld->m_map, 120, 4096);
	// Commented by Peter on [8/8/2005]
	// moved to LaueZone constructor
/*
	for(l = 0; l < ELD_MAX_LAUE_CIRCLES; l++)
	{
		LaueZone *pZone = pEld->m_vLaueZones[l];
		for(i = 0; i < 3; i++)
		{
			pZone->h[i] = def_h[i];
			pZone->k[i] = def_k[i];
			pZone->x[i] = pZone->y[i] = 0;
		}
	}
*/
	// end of commenting by Peter [8/8/2005]
//	for(i = 0; i < 2; i++)
//	{
//		pEld->m_uu[i] = def_uu[i]; pEld->m_vv[i] = def_vv[i];
//		pEld->m_hh[i] = def_hh[i]; pEld->m_kk[i] = def_kk[i]; pEld->m_ll[i] = def_ll[i];
//	}

//	pEld->m_bAuto = TRUE;

	UpdateReflInfo( hDlg, pEld, 1);
	CheckDlgButton( hDlg, IDC_ELD_REF1, 1);
	CheckDlgButton( hDlg, IDC_ELD_MAP,  0);
	CheckDlgButton( hDlg, IDC_ELD_SHAPE,1);
	CheckDlgButton( hDlg, IDC_ELD_ZX,   1);
	CheckDlgButton( hDlg, IDC_ELD_ZY,   0);
	CheckDlgButton( hDlg, IDC_ELD_INTENSITIES, 1);
	CheckDlgButton( hDlg, IDC_ELD_AUTO, pEld->m_pCurLZ->bAuto);
	ComboBox_SetCurSel(GetDlgItem(hDlg, IDC_ELD_INTENSITY_TYPE), 0);
//	CheckDlgButton( hDlg, IDC_ELD_SHAPEFIT, 1);
	CheckDlgButton( hDlg, IDC_ELD_DRAWMARKS, pEld->m_bMarks);

#ifdef USE_FONT_MANAGER
	HFONT hFont = FontManager::GetFont(FM_NORMAL_FONT);
	HFONT hFontGreek = FontManager::GetFont(FM_GREEK_FONT);
	SendDlgItemMessage(hDlg, IDC_ELD_LIST_HEADER, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_NORMAL_FONT_CW), 0);
	SendDlgItemMessage(hDlg, IDC_ELD_LIST, WM_SETFONT, (WPARAM)FontManager::GetFont(FM_NORMAL_FONT_CW), 0);
#else
	HFONT hFont = hfNormal;
	HFONT hFontGreek = hfGreek;
	SendDlgItemMessage(hDlg, IDC_ELD_LIST_HEADER, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_LIST, WM_SETFONT, (WPARAM)hFont, 0);
#endif

	SendDlgItemMessage(hDlg, IDC_ELD_AS, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_BS, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_CS, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_GS, WM_SETFONT, (WPARAM)hFontGreek, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_NR, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_LZ_NUM, WM_SETFONT, (WPARAM)hFont, 0);

	SendDlgItemMessage(hDlg, IDC_ELD_INFO, WM_SETFONT, (WPARAM)hFont, 0);

	SendDlgItemMessage(hDlg, IDC_ELD_3D_A, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_3D_B, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_3D_C, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_3D_AL, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_3D_BT, WM_SETFONT, (WPARAM)hFont, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_3D_GM, WM_SETFONT, (WPARAM)hFont, 0);

	SendDlgItemMessage(hDlg, IDC_ELD_3D_AL2, WM_SETFONT, (WPARAM)hFontGreek, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_3D_BT2, WM_SETFONT, (WPARAM)hFontGreek, 0);
	SendDlgItemMessage(hDlg, IDC_ELD_3D_GM2, WM_SETFONT, (WPARAM)hFontGreek, 0);

	TextToDlgItemW(hDlg, IDC_ELD_3D_AL2, "%c= ", ALPHA_CHAR);
	TextToDlgItemW(hDlg, IDC_ELD_3D_BT2, "%c= ", BETA_CHAR);
	TextToDlgItemW(hDlg, IDC_ELD_3D_GM2, "%c= ", GAMMA_CHAR);

	return	TRUE;
}
/*-------------------------------------------------------------------------*/
void UpdateSpinControl( HWND hDlg, LPELD pELD, WPARAM wParam, LPARAM lParam )
{
	static BOOL rec = FALSE;
	int		i, d, f;
	BOOL	bUpdate = FALSE;

	if( rec )
		return;
	rec = TRUE;

	if (HIWORD(wParam) == EN_KILLFOCUS ||
		HIWORD(wParam) == EN_CHANGE )
	{
		d = GetDlgItemInt( hDlg, LOWORD(wParam), &f, TRUE);
		for (i=0;i<2;i++)
		{
			if (idc_uu[i] == LOWORD(wParam)) pELD->m_uu[i] = d;
			if (idc_vv[i] == LOWORD(wParam)) pELD->m_vv[i] = d;
			if (idc_hh[i] == LOWORD(wParam)) pELD->m_hh[i] = d;
			if (idc_kk[i] == LOWORD(wParam)) pELD->m_kk[i] = d;
			if (idc_ll[i] == LOWORD(wParam)) pELD->m_ll[i] = d;
		}
		if (HIWORD(wParam) == EN_KILLFOCUS) bUpdate = TRUE;
	}
	if (HIWORD(wParam) == SE_MOVE)
	{
		if (LOWORD(lParam) == JC_UP)   d=  1;
		if (LOWORD(lParam) == JC_DOWN) d= -1;
		for (i=0;i<2;i++)
		{
			if (idc_uu[i] == LOWORD(wParam)) pELD->m_uu[i] += d;
			if (idc_vv[i] == LOWORD(wParam)) pELD->m_vv[i] += d;
			if (idc_hh[i] == LOWORD(wParam)) pELD->m_hh[i] += d;
			if (idc_kk[i] == LOWORD(wParam)) pELD->m_kk[i] += d;
			if (idc_ll[i] == LOWORD(wParam)) pELD->m_ll[i] += d;
		}
		bUpdate = TRUE;
	}
	if (bUpdate)
	{
		SetDlgItemInt( hDlg, IDC_ELD_UU1, pELD->m_uu[0], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_VV1, pELD->m_vv[0], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_UU2, pELD->m_uu[1], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_VV2, pELD->m_vv[1], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_HH1, pELD->m_hh[0], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_KK1, pELD->m_kk[0], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_LL1, pELD->m_ll[0], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_HH2, pELD->m_hh[1], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_KK2, pELD->m_kk[1], TRUE);
		SetDlgItemInt( hDlg, IDC_ELD_LL2, pELD->m_ll[1], TRUE);
	}
	rec = FALSE;
}
/*-------------------------------------------------------------------------*/
// ADDED BY PETER ON 23 JULY 2005
// FOR HOLZ DEFINITION
static void UpdateLaueCircle(OBJ* lpObj, LPELD pEld, LPARAM lParam, WPARAM wParam)
{
	OBJ*	I = OBJ::GetOBJ( lpObj->ParentWnd );	// Image Object
	double	x = (LOINT(lParam)) * I->scd / I->scu + I->xo;
	double	y = (HIINT(lParam)) * I->scd / I->scu + I->yo;
	double  dRad = FastSqrt(SQR(x - pEld->m_center.x) + SQR(y - pEld->m_center.y));
	double  dRad1, dRad2;
	int iCircle = pEld->m_nCurLaueZone;

	dRad1 = 0;
	dRad2 = 1e33;
	if( (iCircle > 0) && (true == pEld->m_vbLaueCircles[iCircle-1]) )
	{
		dRad1 = pEld->m_vLaueZones[iCircle-1]->m_dRadius;
	}
	if( (iCircle < ELD_MAX_LAUE_CIRCLES-1) &&
		(true == pEld->m_vbLaueCircles[iCircle+1]) )
	{
		dRad2 = pEld->m_vLaueZones[iCircle+1]->m_dRadius;
	}
	if( (dRad > dRad1) && (dRad < dRad2) )
	{
		pEld->m_pCurLZ->m_dRadius = dRad;
		pEld->m_vbLaueCircles[pEld->m_nCurLaueZone] = true;
		RepaintWindow(I->hWnd);
	}
	else
	{
		MessageBeep(0xFFFF);
	}
}

void ELD::RunSpinningStar()
{
	LaueZone *apLZ[2] = {NULL, NULL}, *pLZ;
	int i;

	// add ZOLZ & FOLZ
	for(i = 0; i < 2; i++)
	{
		pLZ = m_vLaueZones[i];
		if( (true == m_vbLaueCircles[i]) &&
			(pLZ->m_nFlags & LaueZone::ZONE_INDEXED) )
		{
			// If the flag bit for estimated intensities is not set
			// then try to estimate the intensities
			if( 0 == (pLZ->m_nFlags & LaueZone::ZONE_INT_EXTRACTED) )
			{
				ExtractRefls(0, 1e6, i);
				pLZ->m_nFlags |= LaueZone::ZONE_INT_EXTRACTED;
			}
			apLZ[i] = pLZ;
		}
	}
	// final check before the call
	if( (NULL == apLZ[0]) && (NULL == apLZ[1]) )
	{
		pLZ = m_vLaueZones[0];
		if( pLZ->m_nFlags & LaueZone::ZONE_INDEXED )
		{
			// If the flag bit for estimated intensities is not set
			// then try to estimate the intensities
			if( 0 == (pLZ->m_nFlags & LaueZone::ZONE_INT_EXTRACTED) )
			{
				ExtractRefls(0, 1e6, 0);
				pLZ->m_nFlags |= LaueZone::ZONE_INT_EXTRACTED;
			}
			apLZ[0] = pLZ;
		}
		else
		{
			MessageBox(NULL, "The ZOLZ must be indexed first.",
				"ELD ERROR", MB_ICONERROR | MB_OK);
		}
	}
	if( (NULL != g_pfnSPINSTAR) && (NULL != apLZ[0]) )
	{
		LaueZone *pLZ1 = apLZ[0];
		int autoOffset = (BYTE*)&pLZ1->bAuto-(BYTE*)pLZ1;
		int laueSize = sizeof(LaueZone);
		int vecSize = sizeof(EldReflVec);
		Trace(Format("offset: %1%, size: %2%, size2: %3%") % autoOffset % laueSize % vecSize);
		g_pfnSPINSTAR(apLZ);
	}
/*/
	SECURITY_ATTRIBUTES attrib;
	attrib.nLength = sizeof(attrib);
	attrib.lpSecurityDescriptor = NULL;
	attrib.bInheritHandle = FALSE;

	if( NULL != _handle )
	{
		MessageBox(NULL, "Cannot start new Symmetry Analyzer. Stop previous.",
			"ELD warning", MB_OK);
	}
	else
	{
		_handle = ::CreateThread(&attrib, 0, ELD_CallSpinStar,
			(LPVOID)this, CREATE_SUSPENDED, &_id);
		::ResumeThread(_handle);
	}
//*/
}
// End of addition [12/8/2005]
BOOL ELD::Initialize(OBJ* pObj)
{
	int i, l;
	if( NULL == (m_pObj = pObj) )
	{
		::OutputDebugString("ELD::Initialize() -> Trying to initialize ELD with NULL object.\n");
		return FALSE;
	}
	if( sizeof(ELD) > sizeof(pObj->dummy) )
	{
		return FALSE; // Very important
	}
	// parent object
	if( NULL == (m_pPar = OBJ::GetOBJ(pObj->ParentWnd)) )
	{
		::OutputDebugString("ELD::Initialize() -> Parent object is NULL.\n");
		return FALSE;
	}
	m_nPeakWindow = 10;
	// create space for the Laue circles data
//	vLaueCircles.resize(ELD_MAX_LAUE_CIRCLES);
//	vLaueCircRGB.resize(ELD_MAX_LAUE_CIRCLES);
	m_vbLaueCircles.resize(ELD_MAX_LAUE_CIRCLES);
	// set all colors and as un-initialized
//	vLaueCircRGB[0] = RGB(255, 0, 0);
//	vLaueCircRGB[1] = RGB(0, 255, 0);
	fill_n(m_vbLaueCircles.begin(), m_vbLaueCircles.size(), false);
	// Current LAUE ZONE
	m_nCurLaueZone = 0;
	// We always have the ZOLZ
	m_pZOLZ = new LaueZone(0, this);
	//	pEld->m_pZOLZ->Init(0, pEld);
	m_vLaueZones.push_back(m_pZOLZ);
	// call constructor explicitly
	m_pCurLZ = m_pZOLZ;
	for(i = 1; i < ELD_MAX_LAUE_CIRCLES; i++)
	{
		m_vLaueZones.push_back(new LaueZone(i, this));
	}
	for(i = 0; i < ELD_MAX_LAUE_CIRCLES; i++)
	{
		m_vLaueZones[i]->m_pszImgName = pObj->title;
	}
	// The lattice has not been refined YET
	m_bRefinedLattice = FALSE;
	m_bPaintLaueCirc = TRUE;
	m_bPaintHOLZaxes = TRUE;
	m_bDrawAxes = TRUE;
	m_bMarks = FALSE;
	// m_image initialization
	m_image.pData = m_pPar->dp;
// 	m_image.w	= R4(/*pObj*/m_pPar->x);
	m_image.w	= m_pPar->x;
	m_image.h	= m_pPar->y;
	m_image.npix = m_pPar->npix;
	m_image.stride = m_pPar->stride;//R4(m_image.w * IMAGE::PixFmt2Bpp(m_image.npix));

	m_image.MedianFilter();

//	pEld->m_EstMax = 4;
	m_arad = 1;		// Average refl radius
	m_bMarks = TRUE;	// Draw marks
	m_meanampl = 1;	// Mean amplitude value

	MakeHisto();

	//----------------------------------------------------------------------
	l  = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256;
	l += 16384;					// mask
	l +=   256*sizeof(float);	// as
	l +=   256*sizeof(float);	// ds
	l +=  2048*sizeof(float);	// ws
	l +=  2048*sizeof(float);	// wn
	l +=  2048*sizeof(float);	// wa
	l +=  4096*sizeof(POINT);	// ppt
	l +=  4096*sizeof(BYTE);	// map
	l +=     1*sizeof(EREFL);	// R

	if ( NULL == (pObj->bp = (LPBYTE)calloc( 1, l*8 )) )
	{
		return FALSE;
	}

	m_mask = pObj->bp + l;                     //
	m_as	= (LPFLOAT)	(m_mask + 16384);   //
	m_ds	= 			 m_as   +   256;    //
	m_ws	= 			 m_ds   +   256;
	m_wn	= 			 m_ws   +  2048;
	m_wa	= 			 m_wn   +  2048;
	m_ppt	= (LPPOINT)	(m_wa   +  2048);
	m_map	= (BYTE*)	(m_ppt  +	4096);
	m_pCurRefl	= (LPEREFL)	(m_map	 +  4096);

	m_flags = 0;
	memset(m_map, 120, 4096);

	for(i = 0; i < 2; i++)
	{
		m_uu[i] = def_uu[i]; m_vv[i] = def_vv[i];
		m_hh[i] = def_hh[i]; m_kk[i] = def_kk[i]; m_ll[i] = def_ll[i];
	}

	m_handle = NULL;			// for SpinStar dialog call
	m_id = 0;				// .... same

	return TRUE;
}

void ELD::Deinitialize()
{
	LaueZoneVecIt it;

	m_vbLaueCircles.clear();
	if( false == m_vLaueZones.empty() )
	{
		for(it = m_vLaueZones.begin(); it != m_vLaueZones.end(); ++it)
		{
			(*it)->Deinit();
			delete *it;
		}
		m_vLaueZones.clear();
	}
}

/*-------------------------------------------------------------------------*/
LRESULT CALLBACK ELDWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
//	try
	try
	{
	OBJ*		pr;
	OBJ*		pObj = OBJ::GetOBJ(hDlg);
	OBJ*		pImg;					// m_image window object
	LPELD		pEld = (LPELD) &pObj->dummy;
	HCURSOR		hOldCur;
	HWND		hCombo;
	int			a, b, c;

	if( pObj )
	{
		pImg = OBJ::GetOBJ(pObj->ParentWnd);
	}
	switch (message)
	{
		case WM_MOVE:
		case WM_SIZE:
			pObj->UpdateInfoWindowsPos();
			break;

		case WM_INITDIALOG:
			// ADDED BY PETER ON 29 JUNE 2004
			// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
			// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
			pObj = (OBJ*)lParam;
			pEld = (LPELD) &pObj->dummy;
			InitializeObjectDialog(hDlg, pObj);

			pEld->Initialize(pObj);

			InitELDDialog( hDlg, pObj, pEld );
			EnableWindow(GetDlgItem(hDlg, IDC_ELD_EXTRACT), FALSE);
			EnableRefine( pObj, pEld->m_pCurLZ );
			// CHANGED BY PETER ON 23 JULY 2005
			// FOR HOLZ DEFINITION
//			wndChangeDlgSize(hDlg, hDlg,
//				IsDlgButtonChecked(hDlg, IDC_ELD_USE3D) ? 0 : IDC_ELD_2D3D,
//				0, "ELD", hInst );
			wndChangeDlgSize(hDlg, hDlg,
				IsDlgButtonChecked(hDlg, IDC_ELD_USE3D) ? 0 : IDC_ELD_2D3D,
				IsDlgButtonChecked(hDlg, IDC_ELD_DEFINE_HOLZ) ? 0 : s_dwELD_with_SpinnigStar,
				"ELD", hInst );
			// END OF ADDITION 23 JULY 2005

			pEld->m_ProtocolOut.Initialize(hDlg);

			return TRUE;

		case WM_NCDESTROY:
			// Added by Peter on [3/8/2005]
			CloseELDDialog(hDlg, pObj, pEld);
			if (pObj->bp)  free( pObj->bp ); pObj->bp = NULL;
			if (pObj->dp)  free( pObj->dp ); pObj->dp = NULL;
			if (pObj->ebp) free( pObj->ebp); pObj->ebp = NULL;
			if (pObj->edp) free( pObj->edp); pObj->edp = NULL;
			if (pObj->ParentWnd) InvalidateRect( pObj->ParentWnd, NULL, FALSE );
			pEld->Deinitialize();
			objFreeObj( hDlg );
			// End of addition [3/8/2005]
			break;
		// Added by Peter on [3/8/2005]
		case WM_CLOSE:
//			CloseELDDialog(hDlg, pObj, pEld);
			break;
		// End of addition [3/8/2005]

		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORSCROLLBAR:
			return ChangeItemColors( hDlg, message, wParam, lParam );

		case WM_PAINT:
// 			PaintELDDlg(pObj);
			pEld->PaintELDDlg();
			break;

		case FM_ParentInspect:
			if( (wParam & MK_CONTROL) && (pEld->m_bUseLaueCircles) )//GetAsyncKeyState(VK_CONTROL) )
			{
				UpdateLaueCircle(pObj, pEld, lParam, wParam);
			}
			else
			{
				if( (NULL != pImg) && (WM_LBUTTONDOWN == pImg->nMouseMessage) )
				{
					pEld->UpdateFirstRefls(/*pObj, */lParam, 0, 0, wParam);
					EnableRefine( pObj, pEld->m_pCurLZ );
				}
			}
			return TRUE;

		case FM_UpdateNI:
			pObj->NI = OBJ::GetOBJ(pObj->ParentWnd)->NI;
			objInformChildren( hDlg, FM_UpdateNI, wParam, lParam);
			break;

		case FM_ParentPaint:
			DrawELD( pObj );
			break;

		case FM_Update:
			pr   = OBJ::GetOBJ(pObj->ParentWnd);
			pEld->m_image.pData  = pr->dp;
			pEld->m_image.w = R4(pr->x);
			pEld->m_image.h = pr->y;
			break;

//		case WM_DRAWITEM:	DrawUD( hDlg, (LPDRAWITEMSTRUCT) lParam); break;

		case WM_MOUSEACTIVATE:
		case WM_ACTIVATE: if (wParam==WA_INACTIVE) break;
		case WM_QUERYOPEN:
			ActivateObj(hDlg);
			break;

		case WM_PARENTNOTIFY:
			TMP[0] = 0;
			break;

		// ADDED by Peter on 3 Aug 2006 in order to process INVERT IMAGE menu command
		case WM_SYSCOMMAND:
			if( IDM_OM_INVERT == wParam )
			{
				pEld->MakeHisto();
			}
			break;

		case WM_COMMAND:
			switch( HIWORD(wParam) )
			{
			case CBN_SELCHANGE:
				if( IDC_ELD_LAUE_ZONE_NR != (int)LOWORD(wParam) )
				{
					break;
				}
				// scan changes first
				ScanAll(hDlg, pObj);
				// Set new information
//Change_LZ_nr:
//				hCombo = GetDlgItem(hDlg, IDC_ELD_LAUE_ZONE_NR);
				hCombo = (HWND)lParam;       // handle to combo box
				pEld->m_nCurLaueZone = ComboBox_GetCurSel(hCombo);
				pEld->m_pCurLZ = pEld->m_vLaueZones[pEld->m_nCurLaueZone];
				EnableWindow(GetDlgItem(hDlg, IDC_ELD_EXTRACT),
					pEld->m_pCurLZ->m_nFlags & LaueZone::ZONE_INDEXED);
				TextToDlgItemW(hDlg, IDC_ELD_LZ_NUM, "LZ = %d", pEld->m_nCurLaueZone);
				UpdateReflInfo(hDlg, pEld, TRUE);
				Params2Dlg(hDlg, pEld);
				CheckDlgButton( hDlg, IDC_ELD_AUTO, pEld->m_pCurLZ->bAuto);
				EnableRefine(pObj, pEld->m_pCurLZ);
				// Finally repaint the EDP window
				RepaintWindow(pImg->hWnd);
				DrawELD(pObj);
				break;
			// END OF ADDITION BY PETER ON 23 JULY 2005
				break;
			default:
				break;
			}
			switch( LOWORD(wParam) )
			{
			case IDC_ELD_AUTO:
//				pEld->m_bAuto = IsDlgButtonChecked(hDlg, IDC_ELD_AUTO);
				pEld->m_pCurLZ->bAuto = IsDlgButtonChecked(hDlg, IDC_ELD_AUTO);
				EnableRefine(pObj, pEld->m_pCurLZ);
				return TRUE;

			case IDC_ELD_SMARTLR:
		 		pEld->m_bSmartLR = IsDlgButtonChecked(hDlg, IDC_ELD_SMARTLR);
		 		return TRUE;

			case IDC_ELD_DOALL:
			case IDC_ELD_REFINE:
				SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_RESETCONTENT, 0, 0);

				pEld->m_pCurLZ->bAuto = IsDlgButtonChecked(hDlg, IDC_ELD_AUTO);
				hOldCur = SetCursor(hcWait);
		 		ScanAll( hDlg, pObj );
		 		pEld->m_bMarks = FALSE;
			 	InvalidateRect( pObj->ParentWnd, NULL, FALSE);
		 		SendMessage( pObj->ParentWnd, WM_PAINT, 0, 0);
		 		pEld->m_bMarks = IsDlgButtonChecked(hDlg, IDC_ELD_DRAWMARKS);
				if (!pEld->IsCalibrated())
				{
					//MessageBox(NULL, _T("Warning: pattern not calibrated"), _T("Warning"), MB_OK);
					//RECT r;
					//GetWindowRect(hDlg, &r);
					//r.bottom += 20;
					//MoveWindow(hDlg, r.left, r.top, r.right-r.left, r.bottom-r.top, TRUE);
				}
// 				if( !DoLatRef(pObj, pEld->m_pCurLZ->bAuto) )
		 		if( !pEld->DoLatRef(pEld->m_pCurLZ->bAuto) )
				{
					SetCursor(hOldCur);
		 			myError(IDS_ERRREFINELATTICE);
		 			return TRUE;
		 		}
				SetCursor(hOldCur);
				// Modified by Peter on [8/8/2005]
//				EnableWindow(GetDlgItem(hDlg, IDC_ELD_EXTRACT), (pEld->m_flags & ELR_DONE));
				EnableWindow(GetDlgItem(hDlg, IDC_ELD_EXTRACT),
					LaueZone::ZONE_INDEXED & pEld->m_pCurLZ->m_nFlags);
				// End of modification [8/8/2005]
		 		if( wParam != IDC_ELD_DOALL )
					goto xxx1;

			case IDC_ELD_EXTRACT:
				{
					// Added by Peter on 09 Feb 2004
					int iIntegrate = ComboBox_GetCurSel(GetDlgItem(pObj->hWnd, IDC_ELD_INTENSITY_TYPE));
					switch( iIntegrate )
					{
					case 0: pEld->m_nIntegrationMode = ELD::ELD_INTEGRATE_SHAPE_FIT; break;
					case 1: pEld->m_nIntegrationMode = ELD::ELD_INTEGRATE_COMMON_PROFILE; break;
					case 2: pEld->m_nIntegrationMode = ELD::ELD_INTEGRATE_SIMPLE_SUMMATION; break;
					}
					// END of addition
					pEld->m_pCurLZ->nEstCount = 0;
					while((pEld->m_pCurLZ->nEstCount < pEld->m_pCurLZ->nEstMax))
					{
						hOldCur = SetCursor(hcWait);
						// Modified by Peter on [8/8/2005]
						if( !(LaueZone::ZONE_INDEXED & pEld->m_pCurLZ->m_nFlags) )
//						if( !(pEld->m_flags & ELR_DONE) )
						// End of modification [8/8/2005]
							return TRUE;
			 			pEld->m_bMarks = FALSE;
				 		InvalidateRect( pObj->ParentWnd, NULL, FALSE);
			 			SendMessage( pObj->ParentWnd, WM_PAINT, 0, 0);
			 			pEld->m_bMarks = IsDlgButtonChecked(hDlg, IDC_ELD_DRAWMARKS);
		 				pEld->ExtractRefls(0, 1e6, pEld->m_nCurLaueZone);
			 			SetCursor(hOldCur);
						if( 0 != iIntegrate )
							break;
			 		}
					goto xxx1;
				}

			case IDC_ELD_MAP:
				pEld->m_flags &= ~MAP_EX;
				goto xxx1;

			case IDC_ELD_SHAPE:
				pEld->m_flags &= ~SH1_EX;
			xxx1:
			{
				int h=0, k=0;
				int	i = (int)SendMessage( GetDlgItem(hDlg, IDC_ELD_LIST), CB_GETCURSEL, 0, 0);
				if ( (i != CB_ERR ) && (pEld->m_pCurLZ->m_nFlags & LaueZone::ZONE_INT_EXTRACTED)//(pEld->m_flags & ESR_DONE)
					 && (CB_ERR != SendMessage( GetDlgItem(hDlg, IDC_ELD_LIST), CB_GETLBTEXT, i, (LPARAM)(LPSTR)TMP))
					 && (sscanf( TMP, "%3d %3d", &h, &k) == 2))
				{
//					UpdateFirstRefls(pObj, 0, h, k, 0 );
					pEld->UpdateFirstRefls(0, h, k, 0 );
				}
			}
				return TRUE;

			case IDC_ELD_SYNTH:
//				if( pEld->m_flags & ESR_DONE )
				if( pEld->m_pCurLZ->m_nFlags & LaueZone::ZONE_INT_EXTRACTED )
					pEld->Synth( /*pObj*/ );
				else MessageBeep(0xFFFF);
				break;

			case IDC_ELD_PROTON:
				pEld->m_bProtocol = IsDlgButtonChecked(hDlg, IDC_ELD_PROTON);
				return TRUE;

			case IDC_ELD_NOTLISTY:
				pEld->m_bNotListY = IsDlgButtonChecked(hDlg, IDC_ELD_NOTLISTY);
				return TRUE;

			case IDC_ELD_ZX:
			case IDC_ELD_ZY:
				pEld->PaintELDDlg();
				return TRUE;

			case IDC_ELD_LIST:
				if( CBN_SELCHANGE == HIWORD(lParam) ) goto xxx1;
				return TRUE;

			case IDC_ELD_DRAWMARKS:
		 		pEld->m_bMarks = IsDlgButtonChecked(hDlg, IDC_ELD_DRAWMARKS);
				InvalidateRect(pObj->ParentWnd, NULL, FALSE);
				return TRUE;

			case IDC_ELD_DRAWINDEX:
		 		pEld->m_bIndex = IsDlgButtonChecked(hDlg, IDC_ELD_DRAWINDEX);
			 	InvalidateRect(pObj->ParentWnd, NULL, FALSE);
			 	return TRUE;

			case IDC_ELD_DRAWAXES:
				pEld->m_bDrawAxes = IsDlgButtonChecked(hDlg, IDC_ELD_DRAWAXES);
				InvalidateRect(pObj->ParentWnd, NULL, FALSE);
				return TRUE;

			case IDM_F_SAVE:
			case IDC_ELD_SAVE:
//				if( pEld->m_flags & ESR_DONE )
				if( pEld->m_pCurLZ->m_nFlags & LaueZone::ZONE_INT_EXTRACTED )
				{
					OBJ obj;
					obj.ParentWnd = hDlg;
					if ( ! DialogBoxParam( hInst, "ELDSAVE", hDlg, ELDSaveDlgProc, (LPARAM)&obj ) )
						return TRUE;
					// Added by Peter on 09 Feb 2004
					int iIntegrate = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_ELD_INTENSITY_TYPE));
//					BOOL	bIntegrate = IsDlgButtonChecked( pObj->hWnd, IDC_ELD_INTEGRATE );
					// END add
					if( 0 == iIntegrate/*!bIntegrate*/ )
					{
						pEld->FillReflList(pEld->m_nCurLaueZone);
					}
					_splitpath( pObj->fname, NULL, NULL, TMP, NULL );	// take file name only
					strcat(TMP, _T(".hke"));
					if ( ! fioGetFileNameDialog( MainhWnd, "Save ELD HK list", &ofNames[OFN_HKELIST], TMP, TRUE))
						return TRUE;
					pEld->ELDSaveHKA( fioFileName() );
// 					ELDSaveHKA( pObj, pEld, fioFileName() );
				}
				else
				{
					MessageBox(NULL, _T("Uncalibrated data cannot be saved"), _T("Error"), MB_OK);
				}
				return TRUE;

			case IDC_ELD_CLRLIST:
				SendDlgItemMessage( hDlg, IDC_ELD_LIST, CB_RESETCONTENT, 0, 0);
				return TRUE;

			case IDC_ELD_INTENSITIES:
			case IDC_ELD_AMPLITUDES:
//				if( pEld->m_flags & ESR_DONE )
				if( pEld->m_pCurLZ->m_nFlags & LaueZone::ZONE_INT_EXTRACTED )
				{
					pEld->FillReflList(pEld->m_nCurLaueZone);
				}
				return TRUE;

			case IDM_E_COPY:	// Copy to clipboard
				if( OpenClipboard( hDlg ) )
				{
					EmptyClipboard();
					SetClipboardData( CF_TEXT, CreateHKAList(pObj) );
					CloseClipboard();
				}
				return TRUE;

			case IDC_ELD_REF1:
			case IDC_ELD_REF2:
			case IDC_ELD_REF3:
				// Added by Peter on [8/8/2005]
//				pEld->m_flags &= ~ELR_DONE;
				pEld->m_pCurLZ->m_nFlags &= ~LaueZone::ZONE_INDEXED;
				// End of addition [8/8/2005]
				EnableWindow(GetDlgItem(hDlg, IDC_ELD_EXTRACT), FALSE);
			 	InvalidateRect(pObj->ParentWnd, NULL, FALSE);
			 	return TRUE;

			case IDC_ELD_INTENSITY_TYPE:
// 			case IDC_ELD_INTEGRATE:
// 			case IDC_ELD_SHAPEFIT:
				pEld->m_pCurLZ->nEstCount = 0;
				pEld->m_flags &= ~(LaueZone::ZONE_INT_EXTRACTED);
//				pEld->m_flags &= ~ESR_DONE;
				return TRUE;

			case IDC_ELD_USE3D:
			// CHANGED BY PETER ON 23 JULY 2005
			// FOR HOLZ DEFINITION
//				wndChangeDlgSize( hDlg, hDlg,
//					IsDlgButtonChecked(hDlg, IDC_ELD_USE3D) ? 0:IDC_ELD_2D3D, 0, "ELD", hInst);
				wndChangeDlgSize(hDlg, hDlg,
					IsDlgButtonChecked(hDlg, IDC_ELD_USE3D) ? 0 : IDC_ELD_2D3D,
					IsDlgButtonChecked(hDlg, IDC_ELD_DEFINE_HOLZ) ? 0 : s_dwELD_with_SpinnigStar,
					"ELD", hInst );
				pEld->Calc2to3d( a, b, c );
			// END OF CHANGES BY PETER ON 23 JULY 2005
				break;

			// ADDED BY PETER ON 23 JULY 2005
			// FOR HOLZ DEFINITION
			case IDC_ELD_DEFINE_HOLZ:
				// check if we have permission for the Spinning Star in the LIC file
				wndChangeDlgSize(hDlg, hDlg,
					IsDlgButtonChecked(hDlg, IDC_ELD_USE3D) ? 0 : IDC_ELD_2D3D,
					IsDlgButtonChecked(hDlg, IDC_ELD_DEFINE_HOLZ) ? 0 : s_dwELD_with_SpinnigStar,
					"ELD", hInst );
				pEld->m_bUseLaueCircles = IsDlgButtonChecked(hDlg, IDC_ELD_DEFINE_HOLZ) ? TRUE : FALSE;
				// do not use them anymore
				if( FALSE == pEld->m_bUseLaueCircles )
				{
					hCombo = GetDlgItem(hDlg, IDC_ELD_LAUE_ZONE_NR);
					ComboBox_SetCurSel(hCombo, 0);
					goto Change_LZ_nr;
				}
				RepaintWindow(pImg->hWnd);
				DrawELD(pObj);
				break;

			case IDC_ELD_SHOW_LAUE_CIRCLES:
				pEld->m_bPaintLaueCirc = IsDlgButtonChecked(hDlg, IDC_ELD_SHOW_LAUE_CIRCLES) ? TRUE : FALSE;
				RepaintWindow(pImg->hWnd);
				DrawELD(pObj);
				break;

			case IDC_ELD_SHOW_HOLZ_AXES:
				pEld->m_bPaintHOLZaxes = IsDlgButtonChecked(hDlg, IDC_ELD_SHOW_HOLZ_AXES) ? TRUE : FALSE;
				RepaintWindow(pImg->hWnd);
				DrawELD(pObj);
				break;

			case CBN_SELCHANGE:
//			case IDC_ELD_LAUE_ZONE_NR:
				if( IDC_ELD_LAUE_ZONE_NR != (int)LOWORD(wParam) )
				{
					break;
				}
				// scan changes first
				ScanAll(hDlg, pObj);
				// Set new information
Change_LZ_nr:
//				hCombo = GetDlgItem(hDlg, IDC_ELD_LAUE_ZONE_NR);
				hCombo = (HWND)lParam;       // handle to combo box
				pEld->m_nCurLaueZone = ComboBox_GetCurSel(hCombo);
				pEld->m_pCurLZ = pEld->m_vLaueZones[pEld->m_nCurLaueZone];
				EnableWindow(GetDlgItem(hDlg, IDC_ELD_EXTRACT),
					pEld->m_pCurLZ->m_nFlags & LaueZone::ZONE_INDEXED);
				TextToDlgItemW(hDlg, IDC_ELD_LZ_NUM, "LZ = %d", pEld->m_nCurLaueZone);
				UpdateReflInfo(hDlg, pEld, TRUE);
				Params2Dlg(hDlg, pEld);
				CheckDlgButton( hDlg, IDC_ELD_AUTO, pEld->m_pCurLZ->bAuto);
				EnableRefine(pObj, pEld->m_pCurLZ);
				// Finally repaint the EDP window
				RepaintWindow(pImg->hWnd);
				DrawELD(pObj);
				break;

			case IDC_ELD_CALL_SPIN_STAR:
				pEld->RunSpinningStar(/*pObj, pEld*/);
				break;
			// END OF ADDITION BY PETER ON 23 JULY 2005

			case IDC_ELD_UU1:	case IDC_ELD_UU2:
			case IDC_ELD_VV1:	case IDC_ELD_VV2:
			case IDC_ELD_HH1:	case IDC_ELD_HH2:
			case IDC_ELD_KK1:	case IDC_ELD_KK2:
			case IDC_ELD_LL1:	case IDC_ELD_LL2:
				UpdateSpinControl( hDlg, pEld, wParam, lParam );
				pEld->Calc2to3d( a, b, c );
				break;

			case IDCANCEL:
				DestroyWindow( hDlg );
				return TRUE;
			}
		break;
	}
	}
	catch( std::exception& e)
	{
		Trace(_FMT(_T("EldWndProc() -> %s"), e.what()));
		MessageBox(NULL, e.what(), "Error", MB_OK);
//		DestroyWindow( hDlg );
	}
	return FALSE;
}

int GetShape16_test(LPELD pEld, float k, int x, int y)
{
	LPBYTE mask = (LPBYTE)&*pEld->m_vMask.begin();
	LPWORD img = (LPWORD)pEld->m_image.pData;
	int imw, imh, aneg, xms, yms;
	long nMin;
	DWORD iii, cntw, nRet, nMaskSize = R4(pEld->m_nMaskW)*pEld->m_nMaskH;

	iii = (imw = pEld->m_image.w);
	imh = pEld->m_image.h;
	aneg = pEld->m_image.neg;
	nMin = pEld->m_min;
	xms = pEld->m_xms;
	yms = pEld->m_yms;
	LPWORD img_end = img + imh*imw;

	int dx, dy, off, nOffset, nCount;
	LPBYTE curmask = mask;
	LPWORD pixels;
	vector<DWORD> vCounter(pEld->m_rms+32);
	vector<float> vSums(pEld->m_rms+32);
	LPDWORD counter = &*vCounter.begin();
	LPFLOAT ptr = (LPFLOAT)&*vSums.begin();
	WORD val;
	DWORD msk;

	nRet = 128;
	if( (x - xms < 0) || (x + xms >= imw) || (y - yms < 0) || (y + yms >= imh) )
		return -1;
	iii = imw;
	nCount = pEld->m_nMaskW*pEld->m_nMaskH;
	nOffset = 0;
	pixels = img + (x - xms) + imw * (y - yms);
	try
	{
		cntw = 2*xms + 1;
		while( nCount > 0 )
		{
			while( 1 )
			{
				nCount--;
				if( (0xFF != mask[nOffset]) || (nCount <= 0) )
					break;
				nOffset++;
			}
			if( nCount <= 0 )
				break;
			dx = nOffset % cntw;
			dy = nOffset / cntw;
			off = dy * imw + dx;
			val = (pixels[off] ^ aneg) - nMin;
			msk = mask[nOffset];
			ptr[msk] += val;
			counter[msk]++;
			nOffset++;
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("GetShape16_test() -> %s"), e.what()));
	}
	int nCnt = pEld->m_rms + 6;
	nCount = 0;
	while( 1 )
	{
		for(; nCount < nCnt; nCount++)
		{
			if( 0 != counter[nCount] )
			{
				break;
			}
		}
		if( nCount >= nCnt )
		{
			break;
		}
		ptr[nCount] *= k / counter[nCount];
		nCount++;
	}
	return nRet;
}

int GetShape32_asm(LPELD pEld, float k, int x, int y)
{
	LPBYTE mask = (LPBYTE)&*pEld->m_vMask.begin();
	LPWORD img = (LPWORD)pEld->m_image.pData;
	int imw, imh, aneg, xms, yms;
	long nMin;
	DWORD iii, cntw, nRet, nMaskSize = R4(pEld->m_nMaskW)*pEld->m_nMaskH;

	iii = pEld->m_image.stride;
	imw = pEld->m_image.w;
	imh = pEld->m_image.h;
	aneg = pEld->m_image.neg;
	nMin = pEld->m_min;
	xms = pEld->m_xms;
	yms = pEld->m_yms;

	__asm
	{
	mov		edx,	[img]			; // EDX = img
	mov		eax,	-1				; // EAX = -1
;//.........................................
	mov		ecx,	[x]				; // ECX = x
	mov		esi,	[xms]			; // ESI = xms
	sub		ecx,	esi				; // ECX = x - xms
	js		Label_99				; // x - xms < 0 (ECX < 0)???
;//	add		edx,	ecx				;
	lea		edx,	[edx+ecx*4]		; // EDX = img + 4*(x - xms)

	lea		ecx,	[ecx+2*esi+4]	; // ECX = x + xms + 4
	mov		ebx,	[imw]			; // EBX = imw
;//	mov		[iii],	ebx				; // iii = imw
	cmp		ecx,	ebx				;
	jnc		Label_99				; // x + xms + 4 > imw (ECX > imw)???
;//........................................
	mov		ecx,	[y]				; // ECX = y
	mov		edi,	[yms]			; // EDI = yms
	sub		ecx,	edi				; // ECX = y - yms
	js		Label_99				; // y - yms < 0 ???
	mov		ebx,	[iii]			; // Peter, 02 Sept 2009
	imul	ebx,	ecx				; // EBX = imw*(y - yms)
;//	add		edx,	ebx				;
;// lea		edx,	[edx+ebx*4]		; // EDX = img + 4*(x - xms) + 4*imw(y - yms)
	lea		edx,	[edx+ebx]		; // EDX = img + 4*(x - xms) + 4*imw(y - yms)

	lea		ecx,	[ecx+2*edi+2]	; // ECX = y + yms + 2
	cmp		ecx,	[imh]			;
	jnc		Label_99				; // y + yms + 2 > imgh ???
;//............................................
	lea		edi,	[edi*2+1]		; // EDI = yms*2 + 1 <--- mask_h???
	lea		esi,	[esi*2+1]		;
	mov		[cntw],	esi				; 2*xms+1
;//	mov		esi,	[cntw]			; // ESI = xms*2 + 1 <--- mask_w (DWORD aligned)
	imul	esi,	edi				; // ESI = (xms*2 + 1) * (yms*2 + 1)
;//.........................................
	mov		ecx,	512				; // ECX = 512
	xor		eax,	eax				; // EAX = 0
	mov		edi,	[mask]			; // EDI = mask
	push	edi						;
	add		edi,	[nMaskSize]		; // EDI = mask + nMaskSize
	push	edi						;
rep	stos	dword ptr [edi]			; // Fill EDI[0...ECX=512] aray of float numbers by 0-s
	mov		ecx,	esi				; // ECX = (xms*2 + 1) * (yms*2 + 1)
	pop		esi						; // ESI = mask + nMaskSize
	pop		edi						; // EDI = mask
	xor		ebx,	ebx				; // EBX = 0
;//........................................
Label_10:
	mov		eax,	255		; // EAX = 255
	repz	scasb			; // EDI(CurMask) = the array (at EDI) element which is not 0xFF(byte)

	mov		eax,	[mask]	; // EAX = mask
	sub		eax,	edi		;
	not		eax				; // EAX = CurMask - mask
;//	div		byte ptr [cntw]	;		// commented by Peter on 19 Aug 2005 - cntw can be WORD
	push	edx				;		// added by Peter on 19 Aug 2005 - saves EDX
	xor		edx, edx		;
	div		dword ptr [cntw];		// added by Peter on 19 Aug 2005 - cntw can be WORD
;//	xor		ebx, ebx		;
;//	xchg	bl,	ah			; ax = dy, bx = dx
	mov		ebx, edx		; // EAX = dy, EBX = dx
	pop		edx				;		// added by Peter on 19 Aug 2005 - restores EDX
	imul	eax,	[iii]	; // EAX = imw * dy
	add		eax,	ebx		; // EAX = imw * dy + dx

;//	movzx	eax, word ptr [edx+eax*2]; // EAX = 2*(imw*dy + dx) + [img + 2*(x - xms) + 2*imw(y - yms)]
	mov		eax, dword ptr [edx+eax]; // EAX = 2*(imw*dy + dx) + [img + 2*(x - xms) + 2*imw(y - yms)]

	xor		eax,	[aneg]			;
	sub		eax,	[nMin]			;
	movzx	ebx,	byte ptr [edi-1]; // EBX = *CurMask
	sal		ebx,	2				; // (*CurMask) << 2
	add		[esi+ebx], eax			; // [mask + nMaskSize] + ((*CurMask) << 2) <-- add EAX (current pixel)
	inc		word ptr [esi+ebx+1024]	; // [mask + nMaskSize] + ((*CurMask) << 2) + 1024 <-- INCREMENT
	or		ecx,	ecx				;
	jnz		short	Label_10		; // ECX == 0???
;--------------------------------------------------------
	mov		ecx,	256				; // ECX = 256
	lea		edi,	[esi+1024]		; // EDI = [mask + nMaskSize] + 1024
	xor		eax,	eax				; // EAX = 0
	fld		[k]						; // st0 = k
Label_20:
	repz	scasd					; // EDI = scan the EDI array for non-zero element up to max ECX==256 times
	jecxz	Label_28				; // quit if no element was found
	fild	dword ptr [edi-4]		; // read INTEGER (number of pixels)
	fst		dword ptr [edi-4]		; // put as FLOAT
	fidivr	dword ptr [edi-4-1024]; // divide [sum of pixels] / st0(num of pixels)
	fmul	st,	st(1)			;
	fstp	dword ptr [edi-4-1024];
	jmp		short	Label_20		;
Label_28:
	fstp	st					; // remove st
;//---------------------------------------; st0 st1 st2 st3 st4 st5 st6 st7
;//	push	es					;
;//	pop	ds					; st0 st1 st2 st3 st4 st5 st6 st7
;//	mov	eax,	[si]		; Amp
;//	mov	[si+4*249],eax		;
	mov		edi, esi		; // EDI = mask + nMaskSize
	mov		eax, 1			; // EAX = 1
	fldz					; // a0
	mov		ecx, 248		; // ECX = 248
Label_30:					;
	test	dword ptr [esi+1024],-1	; // [mask + nMaskSize + 1024] & 0xFFFFFFFF
	jz		short	Label_38		; st0 st1 st2 st3 st4 st5 st6 st7
	fld		dword ptr [esi]			; x1   a0
	fsub	dword ptr [esi+8]		;x2-x1 a0
	fxch	st(1)					; a0   a1
	fadd	st,	st(1)				;a0+a1 a1
	fstp	dword ptr [esi+1024]	;
	fld		st						;
	fadd	st,	st					; 2a1  a1
	fstp	dword ptr [esi+4+1024]	;
	add		esi,	8				;
	add		eax,	2				;
	loop	Label_30				;
Label_38:							;
	fstp	dword ptr [esi+1024]; // [mask + nMaskSize + 1024] <-- st0
	mov		dword ptr [edi+1024],0	; // [mask + nMaskSize + 1024] <-- 0
	fldz						; // st0 = 0
Label_40:
	fadd	dword ptr [esi+1024]; // st0 += [mask + nMaskSize + 1024]
	fst		dword ptr [esi]			; // [mask + nMaskSize] <-- st0
	fadd	dword ptr [esi+1024]; // st0 += [mask + nMaskSize + 1024]
	sub		esi,	4				; // ESI -= 4 (mask + nMaskSize =- 4)
	cmp		esi,	edi				; // ESI >= EDI???
	jge		short	Label_40		; //
	fstp	dword ptr[esi]		;
;--------------------------------------------------------
Label_99:
	mov		[nRet], eax
	}
	return nRet;
}

int GetShape16_asm(LPELD pEld, float k, int x, int y)
{
	LPBYTE mask = (LPBYTE)&*pEld->m_vMask.begin();
	LPWORD img = (LPWORD)pEld->m_image.pData;
	int imw, imh, aneg, xms, yms;
	long nMin;
	DWORD iii, cntw, nRet, nMaskSize = R4(pEld->m_nMaskW)*pEld->m_nMaskH;

	iii = pEld->m_image.stride;
	imw = pEld->m_image.w;
	imh = pEld->m_image.h;
	aneg = pEld->m_image.neg;
	nMin = floor(pEld->m_min + 0.5);
	xms = pEld->m_xms;
	yms = pEld->m_yms;

	__asm
	{
	mov		edx,	[img]		; // EDX = img
	mov		eax,	-1			; // EAX = -1
;//.........................................
	mov		ecx,	[x]			; // ECX = x
	mov		esi,	[xms]		; // ESI = xms
	sub		ecx,	esi			; // ECX = x - xms
	js		Label_99		; // x - xms < 0 (ECX < 0)???
;//	add		edx,	ecx			;
	lea		edx,	[edx+ecx*2]	; // EDX = img + 2*(x - xms)

	lea		ecx,	[ecx+2*esi+4]	; // ECX = x + xms + 4
	mov		ebx,	[imw]			; // EBX = imw
;//	mov		[iii],	ebx				; // iii = imw
	cmp		ecx,	ebx				;
	jnc		Label_99				; // x + xms + 4 > imw (ECX > imw)???
;//........................................
	mov		ecx,	[y]				; // ECX = y
	mov		edi,	[yms]			; // EDI = yms
	sub		ecx,	edi				; // ECX = y - yms
	js		Label_99				; // y - yms < 0 ???
	mov		ebx,	[iii]			; // Added by Peter 25 Aug 2009
	imul	ebx,	ecx				; // EBX = imw*(y - yms)
;//	add		edx,	ebx				;
;//	lea		edx,	[edx+ebx*2]		; // EDX = img + 2*(x - xms) + 2*imw(y - yms)
	lea		edx,	[edx+ebx]		; // EDX = img + 2*(x - xms) + 2*imw(y - yms)

	lea		ecx,	[ecx+2*edi+2]	; // ECX = y + yms + 2
	cmp		ecx,	[imh]			;
	jnc		Label_99				; // y + yms + 2 > imgh ???
;//............................................
	lea		edi,	[edi*2+1]		; // EDI = yms*2 + 1 <--- mask_h???
	lea		esi,	[esi*2+1]		;
	mov		[cntw],	esi				; // 2*xms+1
;//	mov		esi,	[cntw]			; // ESI = xms*2 + 1 <--- mask_w (DWORD aligned)
	imul	esi,	edi				; // ESI = (xms*2 + 1) * (yms*2 + 1)
;//.........................................
	mov		ecx,	512				; // ECX = 512
	xor		eax,	eax				; // EAX = 0
	mov		edi,	[mask]			; // EDI = mask
	push	edi						;
	add		edi,	[nMaskSize]		; // EDI = mask + nMaskSize
	push	edi						;
rep	stos	dword ptr [edi]			; // Fill EDI[0...ECX=512] aray of float numbers by 0-s
	mov		ecx,	esi				; // ECX = (xms*2 + 1) * (yms*2 + 1)
	pop		esi						; // ESI = mask + nMaskSize
	pop		edi						; // EDI = mask
	xor		ebx,	ebx				; // EBX = 0
;//........................................
Label_10:
	mov		eax,	255		; // EAX = 255
	repz	scasb			; // EDI(CurMask) = the array (at EDI) element which is not 0xFF(byte)

	mov		eax,	[mask]	; // EAX = mask
	sub		eax,	edi		;
	not		eax				; // EAX = CurMask - mask
;//	div		byte ptr [cntw]	;		// commented by Peter on 19 Aug 2005 - cntw can be WORD
	push	edx				;		// added by Peter on 19 Aug 2005 - saves EDX
	xor		edx, edx		;
	div		dword ptr [cntw];		// added by Peter on 19 Aug 2005 - cntw can be WORD
;//	xor		ebx, ebx		;
;//	xchg	bl,	ah			; ax = dy, bx = dx
	mov		ebx, edx		; // EAX = dy, EBX = dx
	pop		edx				;		// added by Peter on 19 Aug 2005 - restores EDX
	imul	eax,	[iii]	; // EAX = imw * dy
	add		eax,	ebx		; // EAX = imw * dy + dx

	add		eax,	ebx		; // EAX = imw * dy + dx

// 	movzx	eax, word ptr [edx+eax*2]; // EAX = 2*(imw*dy + dx) + [img + 2*(x - xms) + 2*imw(y - yms)]
	movzx	eax, word ptr [edx+eax]; // EAX = 2*(imw*dy + dx) + [img + 2*(x - xms) + 2*imw(y - yms)]

	xor		eax,	[aneg]		;
	sub		eax,	[nMin]		;
	movzx	ebx,	byte ptr [edi-1]; // EBX = *CurMask
	sal		ebx,	2			; // (*CurMask) << 2
	add		[esi+ebx], eax		; // [mask + nMaskSize] + ((*CurMask) << 2) <-- add EAX (curretn pixel)
	inc		word ptr [esi+ebx+1024]	; // [mask + nMaskSize] + ((*CurMask) << 2) + 1024 <-- INCREMENT
	or		ecx,	ecx			;
	jnz		short	Label_10	; // ECX == 0???
;--------------------------------------------------------
	mov		ecx,	256			; // ECX = 256
	lea		edi,	[esi+1024]	; // EDI = [mask + nMaskSize] + 1024
	xor		eax,	eax			; // EAX = 0
	fld		[k]					; // st0 = k
Label_20:
repz	scasd					; // EDI = scan the EDI array for non-zero element up to max ECX==256 times
	jecxz	Label_28			; // quit if no element was found
	fild	dword ptr [edi-4]	; // read INTEGER (number of pixels)
	fst		dword ptr [edi-4]		; // put as FLOAT
	fidivr	dword ptr [edi-4-1024]; // divide [sum of pixels] / st0(num of pixels)
	fmul	st,	st(1)			;
	fstp	dword ptr [edi-4-1024];
	jmp		short	Label_20		;
Label_28:
	fstp	st					; // remove st
;//---------------------------------------; st0 st1 st2 st3 st4 st5 st6 st7
;//	push	es					;
;//	pop	ds					; st0 st1 st2 st3 st4 st5 st6 st7
;//	mov	eax,	[si]		; Amp
;//	mov	[si+4*249],eax		;
	mov		edi, esi		; // EDI = mask + nMaskSize
	mov		eax, 1			; // EAX = 1
	fldz					; // a0
	mov		ecx, 248		; // ECX = 248
Label_30:					;
	test	dword ptr [esi+1024],-1	; // [mask + nMaskSize + 1024] & 0xFFFFFFFF
	jz		short	Label_38		; st0 st1 st2 st3 st4 st5 st6 st7
	fld		dword ptr [esi]			; x1   a0
	fsub	dword ptr [esi+8]		;x2-x1 a0
	fxch	st(1)					; a0   a1
	fadd	st,	st(1)				;a0+a1 a1
	fstp	dword ptr [esi+1024]	;
	fld		st						;
	fadd	st,	st					; 2a1  a1
	fstp	dword ptr [esi+4+1024]	;
	add		esi,	8				;
	add		eax,	2				;
	loop	Label_30				;
Label_38:							;
	fstp	dword ptr [esi+1024]; // [mask + nMaskSize + 1024] <-- st0
	mov		dword ptr [edi+1024],0	; // [mask + nMaskSize + 1024] <-- 0
	fldz						; // st0 = 0
Label_40:
	fadd	dword ptr [esi+1024]; // st0 += [mask + nMaskSize + 1024]
	fst		dword ptr [esi]			; // [mask + nMaskSize] <-- st0
	fadd	dword ptr [esi+1024]; // st0 += [mask + nMaskSize + 1024]
	sub		esi,	4				; // ESI -= 4 (mask + nMaskSize =- 4)
	cmp		esi,	edi				; // ESI >= EDI???
	jge		short	Label_40		; //
	fstp	dword ptr[esi]		;
;--------------------------------------------------------
Label_99:
	mov		[nRet], eax
	}
	return nRet;
}

int GetShape8_asm(LPELD pEld, float k, int x, int y)
{
	LPBYTE mask = (LPBYTE)&*pEld->m_vMask.begin();
	LPBYTE img = (LPBYTE)pEld->m_image.pData;
	int imw, imh, aneg, xms, yms;
	long nMin;
	DWORD iii, cntw, nRet, nMaskSize = R4(pEld->m_nMaskW)*pEld->m_nMaskH;

	iii = pEld->m_image.stride;
	imw = pEld->m_image.w;
	imh = pEld->m_image.h;
	aneg = pEld->m_image.neg;
	nMin = floor(pEld->m_min + 0.5);
	xms = pEld->m_xms;
	yms = pEld->m_yms;

	__asm
	{
	mov		edx,	[img]			;
	mov		eax,	-1				;
;//.........................................
	mov		ecx,	[x]				;
	mov		esi,	[xms]			;
	sub		ecx,	esi				; x0 - xms
	js		Label_99				; x0-xms < 0
;//	add		edx,	ecx				;
	lea		edx,	[edx+ecx*1]		;

	lea		ecx,	[ecx+2*esi+4]	; x0 + xms
	mov		ebx,	[imw]			;
;//	mov		[iii],	ebx				;
	cmp		ecx,	ebx				;
	jnc		Label_99				; x+xms > imgw
;//........................................
	mov		ecx,	[y]				;
	mov		edi,	[yms]			;
	sub		ecx,	edi				;
	js		Label_99				; y0-yms < 0
	mov		ebx,	[iii]			; // Added by Peter 25 Aug 2009
	imul	ebx,	ecx				;
;//	add		edx,	ebx				;
	lea		edx,	[edx+ebx*1]		;
	lea		ecx,	[ecx+2*edi+2]	;
	cmp		ecx,	[imh]			;
	jnc		Label_99				; y0+yms > imgh
;//............................................
	lea		edi,	[edi*2+1]		;
	lea		esi,	[esi*2+1]		;
	mov		[cntw],	esi				; 2*xms+1
	imul	esi,	edi				;
;//.........................................
	mov		ecx,	512				;
	xor		eax,	eax				;
	mov		edi,	[mask]			;
	push	edi						;
	add		edi,	[nMaskSize]		; // 128*128
	push	edi						;
	rep	stos	dword ptr [edi]		;
	mov		ecx,	esi				;
	pop		esi						;
	pop		edi						;
	xor		ebx,	ebx				;
;//........................................
Label_10:
	mov		eax,	255		;
	repz	scasb				;
	mov		eax,	[mask]		;
	sub		eax,	edi		;
	not		eax			;
;//	div		byte ptr [cntw]	;		// commented by Peter on 02 Feb 2006 - cntw can be WORD
	push	edx				;		// added by Peter on 02 Feb 2006 - saves EDX
	xor		edx, edx		;
	div		dword ptr [cntw];		// added by Peter on 02 Feb 2006 - cntw can be WORD
;//	xor		ebx,	ebx		;
;//	xchg	bl,	ah		; ax = dy, bx = dx
	mov		ebx, edx		;		// EAX = dy, EBX = dx
	pop		edx				;		// added by Peter on 02 Feb 2006 - restores EDX
	imul	eax,	[iii]		;
	add		eax,	ebx		;

	movzx	eax,	byte ptr [edx+eax*1];

	xor		eax,	[aneg]		;
	sub		eax,	[nMin]		;
	movzx	ebx,	byte ptr [edi-1];
	sal		ebx,	2		;
	add		[esi+ebx], eax		;
	inc		word ptr [esi+ebx+1024]	;
	or		ecx,	ecx		;
	jnz		short	Label_10		;
;--------------------------------------------------------
	mov		ecx,	256		;
	lea		edi,	[esi+1024]	;
	xor		eax,	eax		;
	fld		[k]			;
Label_20:
repz	scasd
	jecxz	Label_28			;
	fild	dword ptr [edi-4]	;
	fst		dword ptr [edi-4]	;
	fidivr	dword ptr [edi-4-1024];
	fmul	st,	st(1)		;
	fstp	dword ptr [edi-4-1024];
	jmp		short	Label_20		;
Label_28:
	fstp	st			;
;//---------------------------------------; st0 st1 st2 st3 st4 st5 st6 st7
;//	push	es			;
;//	pop		ds			; st0 st1 st2 st3 st4 st5 st6 st7
;//	mov	eax,	[si]		; Amp
;//	mov	[si+4*249],eax		;
	mov	edi,	esi		;
	mov	eax,	1		;
	fldz				; a0
	mov	ecx,	248		;
Label_30:					;
	test	dword ptr [esi+1024],-1	;
	jz	short	Label_38		; st0 st1 st2 st3 st4 st5 st6 st7
	fld	dword ptr [esi]		;
	fsub	dword ptr [esi+8]	; a1  a0
	fxch	st(1)			; a0  a1
	fadd	st,	st(1)		;a0+a1 a1
	fstp	dword ptr [esi+1024]		;
	fld	st			;
	fadd	st,	st		;2a1  a1
	fstp	dword ptr [esi+4+1024]	;
	add	esi,	8		;
	add	eax,	2		;
	loop	Label_30			;
Label_38:					;
	fstp	dword ptr [esi+1024]	;
	mov	dword ptr [edi+1024],0	;
	fldz
Label_40:
	fadd	dword ptr [esi+1024]	;
	fst	dword ptr [esi]		;
	fadd	dword ptr [esi+1024]	;
	sub	esi,	4		;
	cmp	esi,	edi		;
	jne	short	Label_40		;
	fstp	dword ptr[esi]		;
;//--------------------------------------------------------
Label_99:
	mov		[nRet], eax
	}
	return nRet;
}
