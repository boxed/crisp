/****************************************************************************/
/* Copyright (c) 2006 Calidris **********************************************/
/* Written by Peter Oleynikov  **********************************************/
/****************************************************************************/
/* CRISP2.ELD ***************************************************************/
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
#include "rgconfig.h"

#include "Crash.h"

#pragma warning(disable:4305)

#ifdef _DEBUG
#pragma optimize ("",off)
#endif

#include "eld.h"
#include "xCBN\shcb.h"

#include "ELD_HOLZ.h"

typedef struct PEAKSEARCHtag
{
	double	piPos;		// position of the peak along the profile
	int		piStart;	// index in the data array of start of the peak along the profile
	int		piEnd;		// index in the data array of end of the peak along the profile
	double	piWghtPos;	// Mass center

} PEAKSEARCH, *LPPEAKSEARCH;

//void LookForReflections(OBJ* lpObj, LPELD lpEld);
typedef struct FFT_SHIFT_PARAMS_
{
	CVector2d vShift;
	double dFshift;
	double dMaxAmpl;
	double dPhase;
	int nFtSize;

} FFT_SHIFT_PARAMS;

void CalculateFFT(LPELD lpEld, const CVector2d &dir, double param,
				  FFT_SHIFT_PARAMS &fftprm);//CVector2d &dOffset, double &dArg, double &dMaxAmp);

//////////////////////////////////////////////////////////////////////////
// Class added [8/3/2005]

static signed char def_h[3] = {4, 0, 3};
static signed char def_k[3] = {0, 4, 3};

static COLORREF s_LaueZoneColors[] =
{
	RGB(255, 0, 0),
	RGB(0, 255, 0),
	RGB(0, 0, 255),
	RGB(255, 255, 0),
	RGB(255, 0, 255),
	RGB(0, 255, 255),
};

LaueZone::LaueZone(int nZone, LPELD lpEld)
	: h_dir(lpEld->m_h_dir)
	, k_dir(lpEld->m_k_dir)
	, center(lpEld->m_center)

{
	Init(nZone, lpEld);
}

LaueZone::~LaueZone()
{
	Deinit();
}

void LaueZone::Init(int nZone, LPELD lpEld)
{
	int i;
	m_nZoneNr = nZone;
	m_nFlags = ZONE_UNINITIALIZED;
	m_dRadius = 0;
	if( nZone < sizeof(s_LaueZoneColors)/sizeof(COLORREF) )
	{
		m_rgbColor = s_LaueZoneColors[nZone];
	}
	else
	{
		m_rgbColor = RGB(255, 0, 0);
	}
	for(i = 0; i < 3; i++)
	{
		h[i] = def_h[i];
		k[i] = def_k[i];
		x[i] = y[i] = 0;
	}
	hPen = CreatePen(PS_SOLID, 1, m_rgbColor);
	nEstCount = 0;		// Number of estimation passes
	nEstMax   = 4;		// Maximum number of estimations

	// lattice
	h_dir = lpEld->m_h_dir;
	k_dir = lpEld->m_k_dir;
	center = lpEld->m_center;
	shift = 0;
	a_length = b_length = c_length = -1.0;
	ral = rbl = rbl = -1.0;		// Lengths are in Angstroms
	rgamma = gamma = M_PI * 0.5;
	nLRefCnt = 0;

	m_dAvgRad = 0.0;

	m_mChangeBase.LoadIdentity();
	for(i = 0; i < 6; i++)
	{
		m_adCell[i] = m_adCellR[i] = m_adRedCell[i] = 0.0;
	}
	m_bHaveCell3D = false;

	m_sCellType[0] = 0;
	m_sCentring[0] = 0;

	bAuto = TRUE;
}

void LaueZone::Deinit()
{
	m_vRefls.clear();
	if( NULL != hPen )
	{
		DeletePen(hPen);
	}
}

//////////////////////////////////////////////////////////////////////////

void DrawELD_HOLZ(OBJ* lpObj, LPELD lpEld, HDC hDC)
{
	HWND		par = lpObj->ParentWnd;
	OBJ*		pr  = OBJ::GetOBJ(par);
	int			x, y, i, j;
	float		x0, y0;
	CVector2d	point, dir_h, dir_k, center;
	HPEN		hPen, nOldPen = 0;
	double		dRad = 0;

	// Draw axes
	if( (lpEld->m_bUseLaueCircles) && (lpEld->m_bPaintLaueCirc) && (lpEld->m_bPaintHOLZaxes) )
	{
		// find a first valid Laue Zone circle (except ZOLZ)
		for(i = 0; i < ELD_MAX_LAUE_CIRCLES; i++)
		{
			if( true == lpEld->m_vbLaueCircles[i] )
			{
				dRad = lpEld->m_vLaueZones[i]->m_dRadius;
				break;
			}
		}
		// draw only if found
		if( i < ELD_MAX_LAUE_CIRCLES )
		{
			dir_h  = lpEld->m_h_dir;
			dir_k  = lpEld->m_k_dir;
			center = lpEld->m_center;
			dir_h.Normalize();
			dir_k.Normalize();
			CVector2d &dir = dir_h;

			hPen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
			SelectPen(hDC, hPen);
			for(j = 0; j < 2; j++)
			{
				point = dir*dRad + center;
				x0 = point.x;
				y0 = point.y;
				XY2Sc( x0, y0, pr, 0, 0 );
				x = floor(x0 + 0.5);
				y = floor(y0 + 0.5);
				_MoveTo(hDC, x0, y0);

				point = dir*10000 + center;
				x0 = point.x;
				y0 = point.y;
				XY2Sc( x0, y0, pr, 0, 0 );
				x = floor(x0 + 0.5);
				y = floor(y0 + 0.5);
				LineTo(hDC, x0, y0);

				point = -dir*dRad + center;
				x0 = point.x;
				y0 = point.y;
				XY2Sc( x0, y0, pr, 0, 0 );
				x = floor(x0 + 0.5);
				y = floor(y0 + 0.5);
				_MoveTo(hDC, x0, y0);

				point = -dir*10000 + center;
				x0 = point.x;
				y0 = point.y;
				XY2Sc( x0, y0, pr, 0, 0 );
				x = floor(x0 + 0.5);
				y = floor(y0 + 0.5);
				LineTo(hDC, x0, y0);

				DeletePen(hPen);
				hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
				SelectPen(hDC, hPen);
				dir = dir_k;
			}
			DeletePen(hPen);
		}
	}
	if( 0 != nOldPen )
		SelectPen(hDC, nOldPen);
}
/*
class HOLZrefFn1D : public NonlinearFit1Function
{
public:
	virtual void Precalculate(double *pdParams, bool *pbParams, int nMA) {}
	virtual double Calculate(double x, double *a, double *dyda, int na)
	{
		double res = 0, a_2, a_3, a_4, a_5;
		int n = (int)x;
		int h = m_pHK->operator[](n).x;
		int k = m_pHK->operator[](n).y;
		FastSinCos(a[2], a_3, a_2);
		FastSinCos(a[3], a_5, a_4);

		res = FastSqrt(SQR(h*a[0]*a_2 + k*a[1]*a_4 + a[6] + a[4]) +
				SQR(h*a[0]*a_3 + k*a[1]*a_5 + a[7] + a[5]));
		return res;
	}
	virtual int GetNumParams(int nNumProfiles) { return 8; }
	virtual LPCTSTR GetName() { return "HOLZ refinement"; }
	virtual UINT GetType() { return TYPE_1D; }

	Pt2Vector *m_pHK;

};
*/
//bool DoAutoHOLZrefinement(Pt2Vector &vPtObs, Pt2Vector &vPtCalc, LPELD lpEld,
//						  VectorD &vParamsOut)
bool ELD::DoAutoHOLZrefinement(Pt2Vector &vPtObs, Pt2Vector &vPtCalc, VectorD &vParamsOut)
{
//	CNonlinearFit nlf;
//	bool bParams[10];
//	VectorD vX, vY;
//	HOLZrefFn1D  fnHOLZrefFn;
//	CVector2d h_dir(lpEld->m_h_dir), k_dir(lpEld->m_k_dir);
//	LaueZone *pLZ = lpEld->m_pCurLZ;
	bool bRes = false;

	if( vPtCalc.size() < 8 )
	{
		return false;
	}

	if (vPtObs.size() < vPtCalc.size())
	{
		throw std::exception("DoAutoHOLZrefinement failed: more calculated than observed reflections");
	}

	vParamsOut.resize(8);
	//////////////////////////////////////////////////////////////////////////
	{
		double h, k, x, y, Sh, Sk, Shk, Sh2, Sk2, Sx, Sy, Sxh, Sxk, Syh, Syk;
		int N = vPtCalc.size();
		Sh = Sk = Shk = Sh2 = Sk2 = Sx = Sy = Sxh = Sxk = Syh = Syk = 0.0;
		for(int i = 0; i < N; i++)
		{
			h = vPtCalc[i].x;
			k = vPtCalc[i].y;
			x = vPtObs[i].x;
			y = vPtObs[i].y;
			Sh  += h;
			Sk  += k;
			Shk += h*k;
			Sh2 += h*h;
			Sk2 += k*k;
			Sx  += x;
			Sy  += y;
			Sxh += x*h;
			Sxk += x*k;
			Syh += y*h;
			Syk += y*k;
		}
		CMatrix A(6,6);
		CVector B(6), X;
		for(int j = 0; j < 6; j++)
		{
			for(int k = 0; k < 6; k++)
			{
				A[j][k] = 0.0;
			}
		}
		A[0][0] = A[1][1] = N;
		A[2][2] = A[3][3] = Sh2;
		A[4][4] = A[5][5] = Sk2;
		A[2][0] = A[0][2] = A[3][1] = A[1][3] = Sh;
		A[4][0] = A[0][4] = A[5][1] = A[1][5] = Sk;
		A[4][2] = A[2][4] = A[5][3] = A[3][5] = Shk;
		B[0] = Sx;
		B[1] = Sy;
		B[2] = Sxh;
		B[3] = Syh;
		B[4] = Sxk;
		B[5] = Syk;
		if( TRUE == Solver(A, B, X) )
		{
			vParamsOut[0] = FastSqrt(SQR(X[2]) + SQR(X[3]));
			vParamsOut[1] = FastSqrt(SQR(X[4]) + SQR(X[5]));
			vParamsOut[2] = FastAtan2(X[2], X[3]);
			vParamsOut[3] = FastAtan2(X[4], X[5]);
			vParamsOut[4] = m_center.x;
			vParamsOut[5] = m_center.y;
			vParamsOut[6] = -m_center.x + X[0];
			vParamsOut[7] = -m_center.y + X[1];
			bRes = true;
		}
	}
	return bRes;
	//////////////////////////////////////////////////////////////////////////
/*
	fnHOLZrefFn.m_pHK = &vPtCalc;

	vX.resize(vPtObs.size());
	vY.resize(vPtObs.size());

	for(int i = 0; i < vX.size(); i++)
	{
		vX[i] = i;
		vY[i] = FastSqrt(SQR(vPtObs[i].x) + SQR(vPtObs[i].y));
	}
	for(i = 0; i < 10; i++)
	{
		bParams[i] = false;
	}
//	bParams[0] = bParams[1] = bParams[2] = bParams[4] = true;
	bParams[0] = bParams[1] = bParams[2] = bParams[3] = bParams[6] = bParams[7] = true;
	bParams[4] = bParams[5] = false;
	h_dir.Normalize();
	k_dir.Normalize();
	vParamsOut[0] = pLZ->a_length;//lpEld->m_al;
	vParamsOut[1] = pLZ->b_length;//lpEld->m_bl;
	vParamsOut[2] = FastAtan2(h_dir.x, h_dir.y);
	vParamsOut[3] = FastAtan2(k_dir.x, k_dir.y);
	vParamsOut[4] = lpEld->m_center.x;//lpEld->m_x0;
	vParamsOut[5] = lpEld->m_center.y;//lpEld->m_y0;
	vParamsOut[6] = pLZ->shift.x;//pLZ->dShX;
	vParamsOut[7] = pLZ->shift.y;//pLZ->dShY;

	bRes = nlf.Fit(vY.begin(), vX.begin(), NULL, vX.size(),
		vParamsOut.begin(), bParams, vParamsOut.size(), &fnHOLZrefFn);

	return bRes;
*/
}

//void SetELDparamsFromHOLZRef(OBJ* lpObj, LPELD lpEld, std::vector<double> &vParams)
void ELD::SetELDparamsFromHOLZRef(VectorD &vParams)
{
//	LaueZone *pLZ = lpEld->m_pCurLZ;
	double dSin, dCos;

	// TODO: make checks if parameters are not too far away
	m_pCurLZ->a_length = vParams[0];
	m_pCurLZ->b_length = vParams[1];
	FastSinCos(vParams[2], dSin, dCos);
//	lpEld->m_h_dir.x = pCurLZ->a_length * dCos;
//	lpEld->m_h_dir.y = pCurLZ->a_length * dSin;
	FastSinCos(vParams[3], dSin, dCos);
//	lpEld->m_k_dir.x = pCurLZ->b_length * dCos;
//	lpEld->m_k_dir.y = pCurLZ->b_length * dSin;
	m_center.x = vParams[4];
	m_center.y = vParams[5];
	m_pCurLZ->shift.x = vParams[6];
	m_pCurLZ->shift.y = vParams[7];

	dSin = FastSin(m_pCurLZ->gamma);
	m_pCurLZ->ral = 1.0 / (m_pCurLZ->a_length * dSin);
	m_pCurLZ->rbl = 1.0 / (m_pCurLZ->b_length * dSin);
	if( IsCalibrated() )
	{
		m_pCurLZ->ral *= m_pObj->NI.Scale * 1e10;
		m_pCurLZ->rbl *= m_pObj->NI.Scale * 1e10;
	}
}

BOOL AskUserForAxesChange(OBJ* lpObj, LPELD lpEld,
						  double a_new, double b_new, double gamma_new,
						  const CVector2d &dir_h_new, const CVector2d &dir_k_new)
{
	LaueZone *pLZ, *pZOLZ;
	wchar_t str[1024];

	pZOLZ = lpEld->m_pZOLZ;
	pLZ = lpEld->m_pCurLZ;
	if(! lpEld->IsCalibrated() )
	{
		swprintf(str, L"The lattice seems to be centered.\r\n"
			L"Would you like to change the axes and unit cell parameters?\r\n"
			L"Old   cell a* = %7.3fpix, b* = %7.3fpix, gm = %7.2f%c\r\n"
			L"New cell a* = %7.3fpix, b* = %7.3fpix, gm = %7.2f%c",
			pLZ->a_length, pLZ->b_length, R2D(pLZ->gamma), 176, a_new, b_new, R2D(gamma_new), 176);
	}
	else
	{
		double scale = lpObj->NI.Scale * 1e10;
		swprintf(str, L"The lattice seems to be centered.\r\n"
			L"Would you like to change the axes and unit cell parameters?\r\n"
			L"Old   cell a = %7.3f%c, b = %7.3f%c, gm = %7.2f%c\r\n"
			L"New cell a = %7.3f%c, b = %7.3f%c, gm = %7.2f%c",
			pLZ->ral, 197, pLZ->rbl, 197, R2D(M_PI - pLZ->gamma), 176,
			scale / (a_new*FastSin(gamma_new)), 197,
			scale / (b_new*FastSin(gamma_new)), 197, R2D(M_PI - gamma_new), 176);
	}
	if( IDNO == MessageBoxW(NULL, str, L"-= Change Axes =-", MB_ICONQUESTION | MB_YESNO) )
	{
		return FALSE;
	}
	// set the basic axes
	lpEld->m_h_dir = dir_h_new * a_new;
	lpEld->m_k_dir = dir_k_new * b_new;
	// Uninitialize all Laue Zones first
	for(int i = 0; i < ELD_MAX_LAUE_CIRCLES; i++)
	{
		pLZ = lpEld->m_vLaueZones[i];
		pLZ->a_length = a_new;
		pLZ->b_length = b_new;
		pLZ->gamma = gamma_new;
		pLZ->m_vRefls.clear();
	}
	// recalculate the basic 3 reflections if they are not user defined
	if( FALSE == pZOLZ->bAuto )
	{
		// disable the 3 reflections
		for(int i = 0; i < 3; i++)
		{
			pZOLZ->x[i] = pZOLZ->y[i] = 0;
			pZOLZ->h[i] = def_h[i];
			pZOLZ->k[i] = def_k[i];
		}
	}
	// TODO: make Manual indexing and find 3 suitable reflections
	lpEld->RecalculateUserRefls(pZOLZ);
	// change the average reflection radius
	lpEld->m_arad = 0.5*__min(a_new, b_new);
	lpEld->m_rma = 0.5 * FastSqrt(SQR(a_new) + SQR(b_new));
	// TODO: if intensities were estimated do that again!!!
	// we must refine the ZOLZ lattice again
	lpEld->DoHOLZref(0);
	lpEld->RecalculateRecCell(pZOLZ, TRUE);
	return TRUE;
}

BOOL InformUserAboutAxesChange(OBJ* lpObj, LPELD lpEld,
							   const CVector2d &dir_h_new, const CVector2d &dir_k_new)
{
	LaueZone *pLZ, *pZOLZ;
	wchar_t str[1024];
	double a_new, b_new, gamma_new;

	a_new = dir_h_new.Length();
	b_new = dir_k_new.Length();
	gamma_new = FastAcos((dir_h_new & dir_k_new) / (dir_h_new.Length()*dir_k_new.Length()));
	pZOLZ = lpEld->m_pZOLZ;
	pLZ = lpEld->m_pCurLZ;
	if(!lpEld->IsCalibrated())
	{
		swprintf(str, L"The lattice seems to be centerd.\r\n"
			L"The axes and unit cell parameters will be changed.\r\n"
			L"Old   cell a* = %7.3fpix, b* = %7.3fpix, gm = %7.2f%c\r\n"
			L"New cell a* = %7.3fpix, b* = %7.3fpix, gm = %7.2f%c",
			pLZ->a_length, pLZ->b_length, R2D(pLZ->gamma), 176, a_new, b_new, R2D(gamma_new), 176);
	}
	else
	{
		double scale = lpObj->NI.Scale * 1e10;
		swprintf(str, L"The lattice seems to be centerd.\r\n"
			L"The axes and unit cell parameters will be changed.\r\n"
			L"Old   cell a = %7.3f%c, b = %7.3f%c, gm = %7.2f%c\r\n"
			L"New cell a = %7.3f%c, b = %7.3f%c, gm = %7.2f%c",
			pLZ->ral, 197, pLZ->rbl, 197, R2D(M_PI - pLZ->gamma), 176,
			scale / (a_new*FastSin(gamma_new)), 197,
			scale / (b_new*FastSin(gamma_new)), 197, R2D(M_PI - gamma_new), 176);
	}
	MessageBoxW(NULL, str, L"-= Change Axes =-", MB_OK);
	// set the basic axes
	lpEld->m_h_dir = dir_h_new;
	lpEld->m_k_dir = dir_k_new;
	// Uninitialize all Laue Zones first
	for(int i = 0; i < ELD_MAX_LAUE_CIRCLES; i++)
	{
		pLZ = lpEld->m_vLaueZones[i];
		pLZ->a_length = a_new;
		pLZ->b_length = b_new;
		pLZ->gamma = gamma_new;
		pLZ->m_vRefls.clear();
	}
	// recalculate the basic 3 reflections if they are not user defined
	if( FALSE == pZOLZ->bAuto )
	{
		// disable the 3 reflections
		for(int i = 0; i < 3; i++)
		{
			pZOLZ->x[i] = pZOLZ->y[i] = 0;
			pZOLZ->h[i] = def_h[i];
			pZOLZ->k[i] = def_k[i];
		}
	}
	// TODO: make Manual indexing and find 3 suitable reflections
	lpEld->RecalculateUserRefls(pZOLZ);
	// change the average reflection radius
	lpEld->m_arad = (int)::floor(0.5*__min(a_new, b_new) + 0.5);
	lpEld->m_rma  = (int)::floor(0.5 * FastSqrt(SQR(a_new) + SQR(b_new)) + 0.5);
	// TODO: if intensities were estimated do that again!!!
	// we must refine the ZOLZ lattice again
	lpEld->DoHOLZref(0);
//	lpEld->RecalculateUserRefls(pZOLZ, (true == pZOLZ->m_vRefls.empty()) ? NULL : pZOLZ->m_vRefls.begin());
	lpEld->RecalculateRecCell(pZOLZ, TRUE);
	lpEld->m_hkmaxe *= pLZ->m_mChangeBase.Determinant();
	return TRUE;
}

// calculate the correct shifts for orthogonal system of coordinates
// this will be the coordinates of intersection point of 2 lines -> vShiftX and vShiftY
void CalculateShift(const FFT_SHIFT_PARAMS &fftprmX,
					const FFT_SHIFT_PARAMS &fftprmY,
					CVector2d &shift)
{
	const CVector2d &vx = fftprmX.vShift, &vy = fftprmY.vShift;
	double denom = vx.x*vy.y - vx.y*vy.x, dFx = fftprmX.dFshift, dFy = fftprmY.dFshift;
	shift = 0;
	if( FastAbs(denom) > 1e-6 )
	{
		denom = 1.0 / denom;
		shift.x = (dFx*vy.y - dFy*vx.y) * denom;
		shift.y = (dFy*vx.x - dFx*vy.x) * denom;
	}
	CVector2d s1, s2, sh;
	s1.x = 0.5 * fftprmX.dPhase * fftprmX.nFtSize / (M_PI * fftprmX.vShift.x);
	s1.y = 0.5 * fftprmX.dPhase * fftprmX.nFtSize / (M_PI * fftprmX.vShift.y);

	s2.x = 0.5 * fftprmY.dPhase * fftprmY.nFtSize / (M_PI * fftprmY.vShift.x);
	s2.y = 0.5 * fftprmY.dPhase * fftprmY.nFtSize / (M_PI * fftprmY.vShift.y);

//	shift = s1 + s2;
}

BOOL IsPossibleCellDoubling(OBJ* lpObj, LPELD lpEld, const CVector2d &shift,
							double &a_new, double &b_new, double &gamma_new,
							CVector2d &dir_h_new, CVector2d &dir_k_new)
{
	double a_prm, b_prm, gamma, leng, param, angle, dSin, dCos;
	CVector2d dir_h_n, dir_k_n, dir, dir2;
	LaueZone *pLZ = NULL;
	BOOL bRes = FALSE;

	pLZ = lpEld->m_pCurLZ;
	a_prm = pLZ->a_length;
	b_prm = pLZ->b_length;
	gamma = pLZ->gamma;
	dir_h_n = Normalize(lpEld->m_h_dir);
	dir_k_n = Normalize(lpEld->m_k_dir);

	// the shift is too small
	if( shift.Length() < 0.1*__min(a_prm, b_prm) )
	{
//		return FALSE;
	}

	// Check cell doubling for the orthorhombic and hexagonal cases
	// a is very close to b
	if( FastAbs(1.0 - a_prm / b_prm) < 0.1 )
	{
		// gamma is close to 60/120
		if( (FastAbs(R2D(gamma) - 60) < 5) || (FastAbs(R2D(gamma) - 120) < 5) )
		{
			// if we have a strange shift 1/2 and 1/3, or 1/2 and 2/3
			dir = shift;
			dir2 = Normalize(0.5 * (dir_h_n + dir_k_n));
			leng = !dir;
			if( leng > 1e-4 )
			{
				dir.Normalize();
				param = 2.0 / 3.0 * FastSqrt(SQR(a_prm) - SQR(0.5*b_prm));
				angle = FastAcos(dir & dir2);
				if( (FastAbs(1.0 - param / leng) < 0.075) &&
					((FastAbs(1.0 - angle/M_PI) < 0.03) || (FastAbs(angle)) < 0.03) ||
					 (FastAbs(1.0 - angle/M_PI_2) < 0.03) )
				{
					a_new = b_new = param;
					gamma_new = M_PI / 3.0;
					dir_h_new = dir2;
					angle = FastAcos(dir_h_n & dir);
					FastSinCos(gamma_new, dSin, dCos);
					dir_k_new.x = dir_h_new.x*dCos - dir_h_new.y*dSin;
					dir_k_new.y = dir_h_new.x*dSin + dir_h_new.y*dCos;
					dir_k_new.Normalize();
					bRes = TRUE;
				}
			}
		}
		else if( (FastAbs(R2D(gamma) - 60) > 5) && (FastAbs(R2D(gamma) - 120) > 5) &&
			(FastAbs(R2D(gamma) - 90) > 5) )
		{
			a_new = a_prm * FastCos(0.5 * gamma);
			b_new = b_prm * FastSin(0.5 * gamma);
			gamma_new = 0.5 * M_PI;
			dir_h_new = Normalize(0.5 * (dir_h_n + dir_k_n));
			dir_k_new = Normalize(CVector2d(-dir_h_new.y, dir_h_new.x));
//			dSin = FastSin(gamma_new);
			bRes = TRUE;
		}
	}
	else
	{
		double temp1 = 2.0 * a_prm * FastCos(gamma);
		double temp2 = 2.0 * b_prm * FastCos(gamma);
		if( FastAbs(1.0 - temp1 / b_prm) < 0.1 )
		{
			a_new = a_prm * FastSin(gamma);
			b_new = 0.5 * b_prm;
			gamma_new = 0.5 * M_PI;
			dir_k_new = dir_k_n;
			dir_h_new = CVector2d(-dir_k_new.y, dir_k_new.x);
			bRes = TRUE;
		}
		else if( FastAbs(1.0 - temp2 / a_prm) < 0.1 )
		{
			a_new = 0.5 * a_prm;
			b_new = b_prm * FastSin(gamma);
			gamma_new = 0.5 * M_PI;
			dir_h_new = dir_h_n;
			dir_k_new = CVector2d(-dir_h_new.y, dir_h_new.x);
			bRes = TRUE;
		}
	}
	return bRes;
}

BOOL CheckCellDoubling(OBJ* lpObj, LPELD lpEld,
					   const FFT_SHIFT_PARAMS &fftprmX,
					   const FFT_SHIFT_PARAMS &fftprmY)
{
	FFT_SHIFT_PARAMS fftprm_x, fftprm_y;
	double a_prm, b_prm, gamma, a_new, b_new, gamma_new, dSin;
	CVector2d dir_h_new, dir_k_new, shift;
	LaueZone *pLZ;
	BOOL bRes = FALSE;

	// TODO: disabled for now!
	return FALSE;

//	dir_h_n = Normalize(lpEld->m_h_dir);
//	dir_k_n = Normalize(lpEld->m_k_dir);
	pLZ = lpEld->m_pCurLZ;
	a_prm = pLZ->a_length;
	b_prm = pLZ->b_length;
	gamma = pLZ->gamma;
	// calculate the shift
	CalculateShift(fftprmX, fftprmY, shift);
	bRes = IsPossibleCellDoubling(lpObj, lpEld, shift,
		a_new, b_new, gamma_new, dir_h_new, dir_k_new);
/*
	// Check cell doubling for the orthorhombic and hexagonal cases
	// a is very close to b
	if( FastAbs(1.0 - a_prm / b_prm) < 0.1 )
	{
		// gamma is close to 60/120
		if( (FastAbs(R2D(gamma) - 60) < 5) || (FastAbs(R2D(gamma) - 120) < 5) )
		{
			// if we have a strange shift 1/2 and 1/3, or 1/2 and 2/3
			dir = CVector2d(dShiftX, dShiftY);
			dir2 = Normalize(0.5 * (dir_h_n + dir_k_n));
			leng = !dir;
			dir.Normalize();
			param = 2.0 / 3.0 * FastSqrt(SQR(a_prm) - SQR(0.5*b_prm));
			angle = FastAcos(dir & dir2);
			if( (FastAbs(1.0 - param / leng) < 0.05) &&
				((FastAbs(1.0 - angle/M_PI) < 0.03) || (FastAbs(angle)) < 0.03) )
			{
				a_new = b_new = param;
				gamma_new = M_PI / 3.0;
				dir_h_new = dir2;
				angle = FastAcos(dir_h_n & dir);
				FastSinCos(gamma_new, dSin, dCos);
				dir_k_new.x = dir_h_new.x*dCos - dir_h_new.y*dSin;
				dir_k_new.y = dir_h_new.x*dSin + dir_h_new.y*dCos;
				dir_k_new.Normalize();
				bRes = TRUE;
			}
		}
		else if( (FastAbs(R2D(gamma) - 60) > 5) && (FastAbs(R2D(gamma) - 120) > 5) )
		{
			a_new = a_prm * FastCos(0.5 * gamma);
			b_new = b_prm * FastSin(0.5 * gamma);
			gamma_new = 0.5 * M_PI;
			dir_h_new = Normalize(0.5 * (dir_h_n + dir_k_n));
			dir_k_new = Normalize(CVector2d(-dir_h_new.y, dir_h_new.x));
			dSin = FastSin(gamma_new);
			bRes = TRUE;
		}
	}
*/
	dSin = FastSin(gamma_new);
	if( TRUE == bRes )
	{
		CalculateFFT(lpEld, dir_k_new, a_new*dSin, fftprm_x);
		CalculateFFT(lpEld, dir_h_new, b_new*dSin, fftprm_y);
		CalculateShift(fftprm_x, fftprm_y, pLZ->shift);
//		if( (fftprm_x.dMaxAmpl > fftprmX.dMaxAmpl) &&
//			(fftprm_y.dMaxAmpl > fftprmY.dMaxAmpl) )
//		leng = 0.01 * FastSqrt(SQR(a_prm) + SQR(b_prm));
//		if( (FastAbs(dShiftX) < leng) && (FastAbs(dShiftY) < leng) )
//		{
//		pLZ->shift = CVector2d(dShiftX, dShiftY);
		bRes = AskUserForAxesChange(lpObj, lpEld, a_new, b_new, gamma_new,
			dir_h_new, dir_k_new);
//		}
	}
	return bRes;
}

BOOL CalculateShifts(LPELD lpEld, double &axis_a, double &axis_b, double &gamma,
					 double dRad1, double dRad2,
					 FFT_SHIFT_PARAMS &fftprmX, FFT_SHIFT_PARAMS &fftprmY)
{
	double dSin, dCos;//, dShiftX, dShiftY;
	LaueZone *pLZ = lpEld->m_pCurLZ;
	FFT_SHIFT_PARAMS fftprmX2, fftprmY2;

	// axes lengths and angle, copy from ZOLZ if missing
	if( (pLZ->a_length < 0) || (pLZ->b_length < 0) )
	{
		pLZ->a_length = lpEld->m_pZOLZ->a_length;
		pLZ->b_length = lpEld->m_pZOLZ->b_length;
		pLZ->gamma = lpEld->m_pZOLZ->gamma;
	}
	axis_a = pLZ->a_length;
	axis_b = pLZ->b_length;
	gamma = pLZ->gamma;
	FastSinCos(gamma, dSin, dCos);

	// axes direction vectors and center
	CVector2d center = lpEld->m_center;
	CVector2d dir_h_n(lpEld->m_h_dir), dir_h(lpEld->m_h_dir);
	CVector2d dir_k_n(lpEld->m_k_dir), dir_k(lpEld->m_k_dir);
	dir_h_n.Normalize();
	dir_k_n.Normalize();
//	CVector2d rec_h = CVector2d(-dir_k_n.y, dir_k_n.x);
//	CVector2d rec_k = CVector2d(-dir_h_n.y, dir_h_n.x);
	if( (dRad1 <= axis_b) || (dRad1 <= axis_b) )
	{
		MessageBox(NULL, "The first limiting circle is too close to the 000-reflection.",
			"ELD ERROR", MB_OK);
		return FALSE;
	}
	// 0 shift right now
	pLZ->shift = 0;
//	pLZ->shift.y = 0;
	// ZOLZ
/*
	int nOldLaueZone = lpEld->m_nCurLaueZone;
	lpEld->m_nCurLaueZone = 0;
	FFT_SHIFT_PARAMS fftprmX_ZOLZ, fftprmY_ZOLZ;
	CalculateFFT(lpEld, dir_k_n, axis_a*dSin, fftprmX_ZOLZ);
	CalculateFFT(lpEld, dir_h_n, axis_b*dSin, fftprmY_ZOLZ);
	lpEld->m_nCurLaueZone = nOldLaueZone;
*/
	// HOLZ
	CalculateFFT(lpEld, dir_k_n, axis_a*dSin, fftprmX);
	CalculateFFT(lpEld, dir_h_n, axis_b*dSin, fftprmY);
	// doubled real cell (half in reciprocal)
	CalculateFFT(lpEld, dir_k_n, 0.5*axis_a*dSin, fftprmX2);
	CalculateFFT(lpEld, dir_h_n, 0.5*axis_b*dSin, fftprmY2);
	if( fftprmX2.dMaxAmpl > 2.0*fftprmX.dMaxAmpl )
	{
		fftprmX = fftprmX2;
	}
	if( fftprmY2.dMaxAmpl > 2.0*fftprmY.dMaxAmpl )
	{
		fftprmY = fftprmY2;
	}
	axis_a = fftprmX.nFtSize / (!fftprmX.vShift) / dSin;
	axis_b = fftprmY.nFtSize / (!fftprmY.vShift) / dSin;
	// calculate the correct shifts for orthogonal system of coordinates
	// this will be the coordinates of intersection point of
	// 2 lines -> vShiftX and vShiftY
//	CalculateShift(fftprmX_ZOLZ, fftprmY_ZOLZ, center);//dShiftX, dShiftY);
	CalculateShift(fftprmX, fftprmY, pLZ->shift);//dShiftX, dShiftY);
	pLZ->shift *= -1;
//	pLZ->shift.x = 0;//7.126;
//	pLZ->shift.y = 0;//-12.878;
	pLZ->a_length = axis_a;
	pLZ->b_length = axis_b;
	center = Normalize(fftprmY.vShift);
	pLZ->h_dir = axis_a*CVector2d(center.y, -center.x);
	center = Normalize(fftprmX.vShift);
	pLZ->k_dir = axis_b*CVector2d(center.y, -center.x);
//	pLZ->shift += center;

	return TRUE;
}

BOOL ELD::DoAutoHOLZ()
{
	FFT_SHIFT_PARAMS fftprm_x, fftprm_y;
	int a;//, nLaueZone = nCurLaueZone;
	double dSum, dRad1, dRad2, axis_a, axis_b, gamma, X, Y, dist;
	float x, y, rad;
	CVector2d vShiftX, vShiftY, shift, /*center, */point;
	Pt2Vector vPtObs, vPtCalc;
	std::vector<double> vParams;
//	LaueZone *pLZ;
// 	DWORD ampl;
	double ampl;
	BOOL bChangeAxes;
	CVector2d new_a, new_b;

	bChangeAxes = FALSE;
	// Check if the corresponding limiting Laue circles has been defined
	// nLaueZone is > than 0!!!
	if( (false == m_vbLaueCircles[m_nCurLaueZone]) ||
		(false == m_vbLaueCircles[m_nCurLaueZone-1]) )
	{
		MessageBox(NULL, "The Laue Zone has not been initialized yet.\r\n"
			"Every Laue Zone requires 2 limiting circles.",
			"ELD ERORR", MB_OK);
		return FALSE;
	}
//	if( false == vbLaueCircles[nLaueZone-1] )
//	{
//		MessageBox(NULL, "The Laue Zone has not been initialized yet.\r\n"
//			"Every Laue Zone requires at least 1 limiting circle.",
//			"ELD ERORR", MB_OK);
//		return FALSE;
//	}
//	pLZ = pCurLZ;
	// limiting circles
	dRad1 = m_vLaueZones[m_nCurLaueZone-1]->m_dRadius;
	dRad2 = m_vLaueZones[m_nCurLaueZone]->m_dRadius;
//	if( false == vbLaueCircles[nLaueZone] )
//		dRad2 = 10000;
	CalculateShifts(this, axis_a, axis_b, gamma, dRad1, dRad2, fftprm_x, fftprm_y);
//	center = center;
	shift = m_pCurLZ->shift;
	// COMMENTED FOR NOW
//	if( TRUE == CheckCellDoubling(lpObj, lpEld, fftprm_x, fftprm_y) )
//	{
//		CalculateShifts(lpEld, axis_a, axis_b, gamma, dRad1, dRad2, fftprm_x, fftprm_y);
//		center = center;
//		shift = pLZ->shift;
//	}
//	// get it back
//	center = center;
//	pLZ->shift = shift;

//	LookForReflections(lpObj, lpEld);
//	CalculateProfiles(lpObj, lpEld, dShiftX, dShiftY);
	// try to locate closest reflections to the 1st axis (h-axis)
	// try |b*|, then try 1/2*|b*|
	int nStartH  = floor(dRad1 / axis_a + 0.5);
	int nFinishH = floor(dRad2 / axis_a + 0.5);
	int nStartK  = floor(dRad1 / axis_b + 0.5);
	int nFinishK = floor(dRad2 / axis_b + 0.5);
	if( (nStartH == nFinishH) || (nStartK == nFinishK) )
	{
		MessageBox(NULL, "Two circles defining the Laue Zone are too close to each other.",
			"ELD ERROR", MB_OK);
		return FALSE;
	}
	rad = 0.5*__min(axis_a, axis_b);
	// 2-passes
	for(int i = 0; i < 2; i++)
	{
		vPtCalc.clear();
		vPtObs.clear();
		// reset the counter
		m_pCurLZ->nLRefCnt = 0;
		dSum = 0;
		for(Y = -nFinishK-1; Y <= nFinishK+1; Y += 1.0)
		{
			for(X = -nFinishH-1; X <= nFinishH+1; X += 1.0)
			{
				// current point in the coordinate system of EDP
				point = m_pCurLZ->GetPeakRelXY(X, Y);
				dist = point.Sqr();
				// check limiting radii
				if( (dist < SQR(dRad1)) || (dist > SQR(dRad2)) )
				{
					continue;
				}
				point += m_center;
				x = point.x;
				y = point.y;
				double as = 0;
				a = PeakCenter2(rad, x, y, rad, 0.5, TRUE, ampl, as);
				if( a || (SQR(x-point.x) + SQR(y-point.y) > SQR(0.5*rad)) )
					continue;
				DrawMark(x, y, m_pCurLZ->hPen, -3);
				vPtCalc.push_back(CVector2d(X, Y));
				vPtObs.push_back(CVector2d(x, y));
				dSum += dist;
				m_hkmaxe = __max(m_hkmaxe, __max(abs(X), abs(Y)));
			}
		}
		m_pCurLZ->nLRefCnt = vPtObs.size();
		// do the refinement
		if( true == DoAutoHOLZrefinement(vPtObs, vPtCalc, vParams) )
		{
			SetELDparamsFromHOLZRef(vParams);
		}
// 		protocolLatAxies(m_pObj, pCurLZ->nLRefCnt, i + 1);
		protocolLatAxies(m_pCurLZ->nLRefCnt, i + 1);
		{
//			CCircleFit cfit;
//			VectorD vX(vPtObs.size()), vY(vPtObs.size());
//			for(i = 0; i < vPtObs.size(); i++)
//			{
//				vX[i] = vPtObs[i].x;
//				vY[i] = vPtObs[i].y;
//			}
//			cfit.Solve()
			dSum = FastSqrt(dSum / vPtObs.size());
			m_pCurLZ->m_bHaveCell3D = false;
			m_pCurLZ->m_mChangeBase.LoadIdentity();
			// try to estimate c*
			if( (dSum > 0) && (0 != (m_pObj->NI.Flags & NI_DIFFRACTION_PATTERN)) )
			{
				// remember the radius
				m_pCurLZ->m_dAvgRad = dSum;
				// the Accelerating voltage
				double Ea = m_pObj->NI.Vacc;
				// The wavelength
				double dLamda = 12.27 / FastSqrt(Ea*(1 + 0.978e-6*Ea));
				// c-real (in Ångströms) and c*-reciprocal (in pixels)
				m_pCurLZ->rcl = 2.0 * SQR(m_pObj->NI.Scale * 1e10 / dSum) / dLamda;
				m_pCurLZ->c_length = m_pObj->NI.Scale * 1e10 / m_pCurLZ->rcl;
				// conventional cell succeeded
				if( TRUE == CalculateCell3D(m_pCurLZ) )
				{
					bChangeAxes = AnalyzeConvCell(m_pCurLZ, new_a, new_b);
				}
			}
		}
	}
	// finally we must change the axes!
	if( TRUE == bChangeAxes )
	{
		InformUserAboutAxesChange(m_pObj, this, new_a, new_b);
		m_pCurLZ->m_bHaveCell3D = false;
		m_pCurLZ->m_mChangeBase.LoadIdentity();
		// TODO: do not recalculate the cell? just change indices on two basic vectors!
		CalculateCell3D(m_pCurLZ);
	}
	m_pCurLZ->m_nFlags = LaueZone::ZONE_AUTO_INDEXED;
	protocolLatRef();
// 	protocolLatRef(m_pObj);

	// success
	return TRUE;
}

BOOL ELD::AnalyzeConvCell(LaueZone *pLZ, CVector2d &new_a, CVector2d &new_b)
{
	// now we have a conventional cell and the transformation matrix
	// recalculate new axes and compare them to old
	// if they differ, inform the user about the change of basis
	CMatrix3x3 invMtx;
	CVector3d hkl1, hkl2, hkl3;
	double a_len, b_len, gamma_new, h_len, k_len, gamma_old, derr;
	static const CVector3d v100(1, 0, 0), v010(0, 1, 0), v001(0, 0, 1);
	// we need an inverse of the matrix here
	pLZ->m_mChangeBase.Invert(invMtx);
	hkl1 = invMtx * v100;
	hkl2 = invMtx * v010;
	hkl3 = invMtx * v001;
	// per-element multiplication to get the direction
	new_a = m_h_dir * hkl1.x + m_k_dir * hkl1.y;
	new_b = m_h_dir * hkl2.x + m_k_dir * hkl2.y;
	// do not change if the axes are equal and the angle between them is the same
	h_len = pLZ->a_length;
	k_len = pLZ->b_length;
	derr = 0.1*__min(h_len, k_len);
	a_len = !new_a;
	b_len = !new_b;
	if( a_len < derr )
	{
		new_a = m_h_dir * hkl3.x + m_k_dir * hkl3.y;
		if( (a_len = !new_a) < derr )
		{
			return FALSE;
		}
	}
	if( b_len < derr )
	{
		new_b = m_h_dir * hkl3.x + m_k_dir * hkl3.y;
		if( (b_len = !new_b) < derr )
		{
			return FALSE;
		}
	}
	gamma_new = FastAcos((new_a & new_b) / (a_len * b_len));
	gamma_old = pLZ->gamma;
	if( (FastAbs(a_len - h_len) < 0.05*__min(a_len, h_len)) &&
		(FastAbs(b_len - k_len) < 0.05*__min(b_len, k_len)) &&
		(FastAbs(gamma_old - gamma_new) < DEG2RAD(1.0)) )
	{
		return FALSE;
	}
//	if( (FastAbs((new_a & h_dir) / ((!new_a) * (!h_dir))) < DEG2RAD(1.0)) ||
//		(FastAbs((new_b & k_dir) / ((!new_b) * (!k_dir))) < DEG2RAD(1.0)) ||
//	{
//		return TRUE;
//	}
	return TRUE;
}

//BOOL ELD::CalculateCell3D(OBJ* pObj, LPELD lpEld, LaueZone *pLZ)
BOOL ELD::CalculateCell3D(LaueZone *pLZ)
{
	BOOL bRes = FALSE;
	CVector3d vA, vB, vC;
	double _alpha, _beta, _gamma, _alphar, _betar, _gammar, _calphar, _cbetar, _cgammar;
	double _scale, _a, _b, _c, _ar, _br, _cr, _Vr, *ptr;
	CMatrix3x3 conv, mtx;

	_scale = m_pObj->NI.Scale * 1e10;
	vA = CVector3d(pLZ->h_dir.x, pLZ->h_dir.y, 0);
	vB = CVector3d(pLZ->k_dir.x, pLZ->k_dir.y, 0);
	vC = CVector3d(pLZ->shift.x, pLZ->shift.y, pLZ->c_length);
	pLZ->m_adCellR[0] = _ar = !vA;
	pLZ->m_adCellR[1] = _br = !vB;
	pLZ->m_adCellR[2] = _cr = !vC;
	_calphar = (vB & vC) / (_br * _cr);
	_cbetar  = (vA & vC) / (_ar * _cr);
	_cgammar = (vA & vB) / (_ar * _br);
	// the volume
	_Vr = _ar*_br*_cr * FastSqrt(1.0 - SQR(_calphar) - SQR(_cbetar) - SQR(_cgammar) +
		2.0*_calphar*_cbetar*_cgammar);
	pLZ->m_adCellR[3] = _alphar = FastAcos(_calphar);
	pLZ->m_adCellR[4] = _betar  = FastAcos(_cbetar);
	pLZ->m_adCellR[5] = _gammar = FastAcos(_cgammar);
	// real-space unit cell parameters
	pLZ->m_adCell[0] = _a = _br*_cr*FastSin(_alphar)*_scale / _Vr;
	pLZ->m_adCell[1] = _b = _ar*_cr*FastSin(_betar )*_scale / _Vr;
	pLZ->m_adCell[2] = _c = _ar*_br*FastSin(_gammar)*_scale / _Vr;
	pLZ->m_adCell[3] = _alpha = FastAcos((_cbetar*_cgammar - _calphar) / (FastSin(_betar)*FastSin(_gammar)));
	pLZ->m_adCell[4] = _beta  = FastAcos((_calphar*_cgammar - _cbetar) / (FastSin(_alphar)*FastSin(_gammar)));
	pLZ->m_adCell[5] = _gamma = FastAcos((_calphar*_cbetar - _cgammar) / (FastSin(_alphar)*FastSin(_betar)));
	sprintf(TMP, "ELD cell:\n\ta=%.4f, b=%.4f, c=%.4f, al=%.4f, bt=%.4f, gm=%.4f\n",
		_a, _b, _c, RAD2DEG(_alpha), RAD2DEG(_beta), RAD2DEG(_gamma));
	OutputDebugString(TMP);
	// reduce the cell
	pLZ->m_adCell[3] = RAD2DEG(pLZ->m_adCell[3]);
	pLZ->m_adCell[4] = RAD2DEG(pLZ->m_adCell[4]);
	pLZ->m_adCell[5] = RAD2DEG(pLZ->m_adCell[5]);
	Buerger_ReduceCell(pLZ->m_adCell, pLZ->m_adRedCell, pLZ->m_mChangeBase);
	sprintf(TMP, "reduced cell:\n\ta=%.4f, b=%.4f, c=%.4f, al=%.4f, bt=%.4f, gm=%.4f\n",
		pLZ->m_adRedCell[0], pLZ->m_adRedCell[1], pLZ->m_adRedCell[2],
		RAD2DEG(pLZ->m_adRedCell[3]), RAD2DEG(pLZ->m_adRedCell[4]), RAD2DEG(pLZ->m_adRedCell[5]));
	OutputDebugString(TMP);
	ptr = &pLZ->m_mChangeBase.d[0][0];
	sprintf(TMP, "change of base matrix:\n\t%2d\t%2d\t%2d\n\t%2d\t%2d\t%2d\n\t%2d\t%2d\t%2d\n",
		(int)ptr[0], (int)ptr[1], (int)ptr[2],
		(int)ptr[3], (int)ptr[4], (int)ptr[5],
		(int)ptr[6], (int)ptr[7], (int)ptr[8]);
	OutputDebugString(TMP);
	pLZ->m_bHaveCell3D = true;

	ConventionalCell(pLZ->m_adRedCell, conv, pLZ->m_sCellType, pLZ->m_sCentring);
	ptr = &conv.d[0][0];
	sprintf(TMP, "reduced -> conventional matrix:\n\t%2d\t%2d\t%2d\n\t%2d\t%2d\t%2d\n\t%2d\t%2d\t%2d\n",
		(int)ptr[0], (int)ptr[1], (int)ptr[2],
		(int)ptr[3], (int)ptr[4], (int)ptr[5],
		(int)ptr[6], (int)ptr[7], (int)ptr[8]);
	OutputDebugString(TMP);
	pLZ->m_mChangeBase = conv * pLZ->m_mChangeBase;
	ptr = &pLZ->m_mChangeBase.d[0][0];
	sprintf(TMP, "final matrix:\n\t%2d\t%2d\t%2d\n\t%2d\t%2d\t%2d\n\t%2d\t%2d\t%2d\n",
		(int)ptr[0], (int)ptr[1], (int)ptr[2],
		(int)ptr[3], (int)ptr[4], (int)ptr[5],
		(int)ptr[6], (int)ptr[7], (int)ptr[8]);
	OutputDebugString(TMP);

	// recalculate reciprocal vectors into real-space vectors
	pLZ->m_vA = (vB ^ vC) * _scale / _Vr;
	pLZ->m_vB = (vC ^ vA) * _scale / _Vr;
	pLZ->m_vC = (vA ^ vB) * _scale / _Vr;
	mtx.Row(0, pLZ->m_vA);
	mtx.Row(1, pLZ->m_vB);
	mtx.Row(2, pLZ->m_vC);
	mtx = pLZ->m_mChangeBase * mtx;
	pLZ->m_vA = mtx.Row(0);
	pLZ->m_vB = mtx.Row(1);
	pLZ->m_vC = mtx.Row(2);

	pLZ->m_adConvCell[0] = !pLZ->m_vA;
	pLZ->m_adConvCell[1] = !pLZ->m_vB;
	pLZ->m_adConvCell[2] = !pLZ->m_vC;
	pLZ->m_adConvCell[3] = RAD2DEG(FastAcos((pLZ->m_vB & pLZ->m_vC) / ((!pLZ->m_vB)*(!pLZ->m_vC))));
	pLZ->m_adConvCell[4] = RAD2DEG(FastAcos((pLZ->m_vA & pLZ->m_vC) / ((!pLZ->m_vA)*(!pLZ->m_vC))));
	pLZ->m_adConvCell[5] = RAD2DEG(FastAcos((pLZ->m_vA & pLZ->m_vB) / ((!pLZ->m_vA)*(!pLZ->m_vB))));
	bRes = TRUE;

	// change of base matrix
	return bRes;
}

//BOOL DoHOLZref(OBJ* lpObj, LPELD lpEld, int nLaueZone)
BOOL ELD::DoHOLZref(int nLaueZone)
{
	int a, /*nLaueZone, */nMinH, nMaxH, nMinK, nMaxK;
	double xc, yc, dMinR, dMaxR, rad, x0, y0, dShiftX, dShiftY;
	float x, y;
	BOOL bCheckR = FALSE;
// 	DWORD amp;
	LaueZone *pLZ;
	Pt2Vector vPtObs, vPtCalc;
	std::vector<double> vParams;
	double amp;

	pLZ = m_vLaueZones[nLaueZone];
	dMinR = 0;
	dMaxR = 1e33;
	if( true == m_vbLaueCircles[nLaueZone] )
	{
		dMaxR = pLZ->m_dRadius;
		bCheckR = TRUE;
	}
	if( (nLaueZone > 0) && (true == m_vbLaueCircles[nLaueZone-1]) )
	{
		dMinR = m_vLaueZones[nLaueZone-1]->m_dRadius;
		bCheckR = TRUE;
	}
	// get corresponding MAX grid points for h- & k-axes directions
	if( FALSE == bCheckR )
	{
		dMinR = 0;
		dMaxR = FastSqrt(SQR(m_image.w) + SQR(m_image.h));
	}
	// nMinH is starting index (POSITIVE!!!)
	nMinH = floor(2*dMinR / pLZ->a_length + 0.5);//lpEld->m_al;
	nMaxH = floor(2*dMaxR / pLZ->a_length + 0.5);//lpEld->m_al;
	nMinK = floor(2*dMinR / pLZ->b_length + 0.5);//lpEld->m_bl;
	nMaxK = floor(2*dMaxR / pLZ->b_length + 0.5);//lpEld->m_bl;
	rad = (int)::floor(0.5*__min(pLZ->a_length, pLZ->b_length) + 0.5);//(lpEld->m_al, lpEld->m_bl);
	// use specified 1st reflection (ONLY!!!) as starting point
	double dH0, dK0, dRemH0, dRemK0;
	int nH0, nK0;
//	double denom = lpEld->m_h_dir.x*lpEld->m_k_dir.y - lpEld->m_k_dir.x*lpEld->m_h_dir.y;
	double denom = m_h_dir.x*m_k_dir.y - m_k_dir.x*m_h_dir.y;
	if( FastAbs(denom) < 1e-6 )
	{
		// cannot do much - base axes are wrong
		return FALSE;
	}
	xc = pLZ->x[0];
	yc = pLZ->y[0];
	x = xc - m_center.x;
	y = yc - m_center.y;
//	dH0 = ( x*lpEld->m_k_dir.y - y*lpEld->m_k_dir.x) / denom;
//	dK0 = (-x*lpEld->m_h_dir.y + y*lpEld->m_h_dir.x) / denom;
	dH0 = ( x*m_k_dir.y - y*m_k_dir.x) / denom;
	dK0 = (-x*m_h_dir.y + y*m_h_dir.x) / denom;
	// TODO: if 1/2 of translation?
	nH0 = (dH0 < 0 ? -1.0 : 1.0) * (int)floor(FastAbs(dH0) + 0.5);
	nK0 = (dK0 < 0 ? -1.0 : 1.0) * (int)floor(FastAbs(dK0) + 0.5);
	// remaining
	dRemH0 = dH0 - nH0;
	dRemK0 = dK0 - nK0;
	// reset the counter
//	lpEld->m_pCurLZ->nLRefCnt = 0; // <--- ??? Why pCurLZ???
	pLZ->nLRefCnt = 0;
	for(int k = -nMaxK; k <= nMaxK; k++)
	{
		for(int h = -nMaxH; h <= nMaxH; h++)
		{
//			x = h*lpEld->m_h_dir.x + k*lpEld->m_k_dir.x;
//			y = h*lpEld->m_h_dir.y + k*lpEld->m_k_dir.y;
			x = h*m_h_dir.x + k*m_k_dir.x;
			y = h*m_h_dir.y + k*m_k_dir.y;
			if( TRUE == bCheckR )
			{
				x0 = x + xc - m_center.x;
				y0 = y + yc - m_center.y;
				double dDist2 = SQR(x0) + SQR(y0);
				if( (dDist2 < SQR(dMinR)) || (dDist2 > SQR(dMaxR)) )
				{
					continue;
				}
			}
			x0 = (x += xc);
			y0 = (y += yc);
			double as = 0;
			a = PeakCenter2(rad, x, y, rad, 0.5, TRUE, amp, as);
			if( a || (SQR(x - x0) + SQR(y - y0) > SQR(0.5*rad)) )
				continue;
			DrawMark(x, y, pLZ->hPen, -3);
			vPtCalc.push_back(CVector2d(h + nH0, k + nK0));
			vPtObs.push_back(CVector2d(x, y));
		}
	}
	pLZ->nLRefCnt = vPtObs.size();
	// do the refinement
	dShiftX = dShiftY = 0;
	if( vPtCalc.size() > 8 )
	{
		if( true == DoAutoHOLZrefinement(vPtObs, vPtCalc, vParams) )
		{
			SetELDparamsFromHOLZRef(vParams);
		}
	}
	pLZ->m_nFlags = LaueZone::ZONE_MANUALLY_INDEXED;
	// success
	return TRUE;
}

void CalculateFFT(LPELD lpEld, const CVector2d &dir, double param,
				  FFT_SHIFT_PARAMS &fftprm)
{
	int nLaueZone, nStartR, nEndR, nStartX, nEndX, nCurX, nCurY, nCurY2, nOff, nStride;
	int nFTsize, nWidth, nHeight;
	PIXEL_FORMAT nFmt;
	int nBpp;
	double dRad1, dRad2;
	CVector2d center;
	LPVOID lpData;

	// Laue Zone number
	nLaueZone = lpEld->m_nCurLaueZone;
	dRad1 = 0;
	// limiting circles
	if( nLaueZone > 0 )
	{
		dRad1 = lpEld->m_vLaueZones[nLaueZone - 1]->m_dRadius;
	}
	dRad2 = lpEld->m_vLaueZones[nLaueZone]->m_dRadius;
	// axes direction vectors and center
	center = lpEld->m_center;
	nStartR = (int)floor(dRad1);
	nEndR   = (int)floor(dRad2 + 0.5);
	// Image width & height
	nWidth = lpEld->m_image.w;
	nHeight = lpEld->m_image.h;
	nStride = nWidth;
	lpData = lpEld->m_image.pData;
	nFmt = lpEld->m_image.npix;
	nBpp = IMAGE::PixFmt2Bpp(nFmt);
	nStride *= nBpp;
//	if( PIX_WORD == nPix ) nStride <<= 1;
//	else if( PIX_DWORD == nPix ) nStride <<= 2;
//	else if( PIX_FLOAT == nPix ) nStride <<= 2;
	nStride = R4(nStride);
	nFTsize = 2*nEndR + 1/*1024*/;
	// FFT tests
	{
		const int N = 1, M = 2*N+1;
		int h, k, /*pix, */nX, nY, nStartR2;
		double pix, array_Re[M*M], array_Im[M*M];
		double dSinT0, dCosT0, dCurSin1, dCurCos1, dCurSin2, dCurCos2;
		double tmp, dMul, dSinDH, dCosDH, dSinDK, dCosDK, tx, ty;
		LPBYTE lpCurPtr;

		std::fill_n(array_Re, SQR(M), 0);
		std::fill_n(array_Im, SQR(M), 0);
		dMul = -M_2PI/nFTsize;
		tx = -dir.y*nFTsize/param - N;
		ty =  dir.x*nFTsize/param - N;
//		tx = -dir.y*nFTsize/param;
//		ty =  dir.x*nFTsize/param;

		nStartR2 = SQR(nStartR);
		nY = -nEndR + center.y;
		for(nCurY = -nEndR; nCurY <= nEndR; nCurY++, nY++)
		{
			nStartX = -(nEndX = (int)floor(FastSqrt(SQR(nEndR) - SQR(nCurY))));
			if( (nY < 0) || (nY >= nHeight/*lpEld->m_imgh*/) )
			{
				continue;
			}
			nCurY2 = SQR(nCurY);
			// set the BYTE data pointer
			nX = nStartX + center.x;
			lpCurPtr = ((LPBYTE)lpData) + nStride*nY + nX*nBpp;
			for(nCurX = nStartX; nCurX <= nEndX; nCurX++, nX++, lpCurPtr+=nBpp)
			{
				if( (nX < 0) || (nX >= nWidth) || (SQR(nCurX) + nCurY2 < nStartR2) )
				{
					continue;
				}
				switch( nFmt )
				{
				case PIX_BYTE: pix = *lpCurPtr; break;
				case PIX_CHAR: pix = *((LPSTR)lpCurPtr); break;
				case PIX_WORD: pix = *((LPWORD)lpCurPtr); break;
				case PIX_SHORT: pix = *((SHORT*)lpCurPtr); break;
				case PIX_DWORD: pix = *((LPDWORD)lpCurPtr); break;
				case PIX_LONG: pix = *((LPLONG)lpCurPtr); break;
				case PIX_FLOAT: pix = *((float*)lpCurPtr); break;
				case PIX_DOUBLE: pix = *((double*)lpCurPtr); break;
				}
				// initial values
//				FastSinCos(dMul*(nCurX*(tx-N) + nCurY*(ty-N)), dSinT0, dCosT0);
				FastSinCos(dMul*(nCurX*tx + nCurY*ty), dSinT0, dCosT0);
				FastSinCos(dMul*nCurX, dSinDH, dCosDH);
				FastSinCos(dMul*nCurY, dSinDK, dCosDK);
				dCurCos1 = dCosT0;
				dCurSin1 = dCurSin2 = dSinT0;
				for(k = 0, nOff = 0; k < M; k++)
				{
					dCurCos2 = dCurCos1;
					dCurSin2 = dCurSin1;
					for(h = 0; h < M; h++, nOff++)
					{
						array_Re[nOff] += pix * dCurCos2;
						array_Im[nOff] += pix * dCurSin2;
						tmp = dCurSin2*dCosDH + dCurCos2*dSinDH;
						dCurCos2 = dCurCos2*dCosDH - dCurSin2*dSinDH;
						dCurSin2 = tmp;
					}
					tmp = dCurSin1*dCosDK + dCurCos1*dSinDK;
					dCurCos1 = dCurCos1*dCosDK - dCurSin1*dSinDK;
					dCurSin1 = tmp;
				}
			}
		}
/*
		// UNOPTIMIZED BASIC ALGORITHM
		for(nCurY = -nEndR; nCurY <= nEndR; nCurY++)
		{
			nStartX = -(nEndX = (int)floor(FastSqrt(SQR(nEndR) - SQR(nCurY))));
			nY = nCurY + center.y;
			if( nY < 0 || nY >= lpEld->m_imgh )
			{
				continue;
			}
			for(nCurX = nStartX; nCurX <= nEndX; nCurX++)
			{
				nX = nCurX + center.x;
				if( (nX < 0) || (nX >= lpEld->m_imgw) || (SQR(nCurX)+SQR(nCurY) < SQR(nStartR)) )
				{
					continue;
				}
				if( 1 == lpEld->m_pix )
				{
					pix = *(((LPBYTE)lpData) + nStride*nY + nX);
				}
				else if( 2 == lpEld->m_pix )
				{
					pix = *(((LPBYTE)lpData) + nStride*nY + (nX<<1));
				}
				for(nOff = 0, k = -2; k <= 2; k++)
				{
					for(h = -2; h <= 2; h++, nOff++)
					{
						dArg = dMul*(nCurX*(h+tx) + nCurY*(k+ty));
						array_Re[nOff] += pix * cos(dArg);
						array_Im[nOff] += pix * sin(dArg);
					}
				}
			}
		}
*/
		// look for the maximum
		double dPha;
		int nMaxI = 0, nMaxH = 0, nMaxK = 0;
		fftprm.dMaxAmpl = 0;
		for(nOff = k = 0; k < M; k++)
		{
			for(h = 0; h < M; h++, nOff++)
			{
				// calculate the phase in RADIANS
				dPha = FastAtan2(array_Re[nOff], array_Im[nOff]);
				// calculate the Amplitude
				array_Re[nOff] = FastSqrt(SQR(array_Re[nOff])+SQR(array_Im[nOff]));
				// save the phase
				array_Im[nOff] = dPha;
				// remember the maximum
				if( array_Re[nOff] > fftprm.dMaxAmpl )
				{
					fftprm.dMaxAmpl = array_Re[nOff];
					fftprm.dPhase = dPha;
					nMaxI = nOff;
					nMaxH = h;// - N; // TODO: check this carefully!!!
					nMaxK = k;// - N; // TODO: check this carefully!!!
				}
			}
		}
#ifdef _DEBUG
		::OutputDebugString("--------- FFT results ---------\n");
		::OutputDebugString("--------- Amplitudes ---------\n");
		::OutputDebugString("\t");
		for(k = 0; k < M; k++)
		{
			sprintf(TMP, "%g\t", tx/* - N*/ + k);
			::OutputDebugString(TMP);
		}
		::OutputDebugString("\n");
		for(k = 0; k < M; k++)
		{
			sprintf(TMP, "%g\t", ty/* - N*/ + k);
			::OutputDebugString(TMP);
			for(int kl = 0; kl < M; kl++)
			{
				sprintf(TMP, "%g\t", array_Re[k*M+kl]);
				::OutputDebugString(TMP);
			}
			::OutputDebugString("\n");
		}
//		for(k = 0; k < SQR(M); k++)
//		{
//			sprintf(TMP, "%g", array_Re[k]);
//			::OutputDebugString(TMP);
//			::OutputDebugString((!((k+1)%M) && k>0) ? "\n" : "\t");
//		}
		::OutputDebugString("--------- Phases ---------\n");
		for(k = 0; k < SQR(M); k++)
		{
			sprintf(TMP, "%g", R2D(array_Im[k]));
			::OutputDebugString(TMP);
			::OutputDebugString((!((k+1)%M) && k>0) ? "\n" : "\t");
		}
		::OutputDebugString("---------\n");
#endif // _DEBUG
//		vShift = CVector2d(nMaxH+tx-N, nMaxK+ty-N);
		fftprm.vShift = CVector2d(nMaxH + tx, nMaxK + ty);
		fftprm.dFshift = nFTsize*array_Im[nMaxI] / M_2PI;
		fftprm.nFtSize = nFTsize;
		sprintf(TMP, "SHIFT: (%g,%g), PHA = %g\n",
			fftprm.vShift.x, fftprm.vShift.y, fftprm.dFshift);
		::OutputDebugString(TMP);
	}
}
/*
template <typename _Tx>
double sum_along_circle(_Tx *img, int w, int h, float xc0, float yc0, float rad)
{
	double clen, step, angle, x, y;
	double sum = 0.0;
	int nlen, i, nx, ny;
	_Tx *ptr;

	// circle length around the reflection
	clen = 2 * M_PI * rad;
	nlen = floor(clen + 0.5f);
	// angle step
	step = 2 * M_PI / float(nlen);

	// sum all pixels
	for(angle = 0.0, i = 0; i < nlen; i++, angle += step)
	{
		x = xc0 + rad * cos(angle);
		y = yc0 + rad * sin(angle);
		nx = floor(x + 0.5);
		ny = floor(y + 0.5);
		ptr = img + nx + ny * w;
		sum += *ptr;
	}

	sum /= double(nlen);

	return sum;
}

template <typename _Tx>
double sum_inside_circle(_Tx *img, int w, int h, float xc0, float yc0, int arad,
						 LPBYTE lpMask, int mw, int mh, int &num)
{
	double sum = 0.0;
	int i, nx, ny, cnt;
	_Tx *ptr;

	nx = floor(xc0 + 0.5)-arad;
	ny = floor(yc0 + 0.5)-arad;
	if( nx < 0 )
		nx = 0;
	if( ny < 0 )
		ny = 0;
	ptr = img + w * ny + nx;

	num = 0;
	// sum all pixels
	for(i = 0; i < mh; i++)
	{
		if( ny+i >= h )
			break;
		for(cnt = 0; cnt < mw; cnt++)
		{
			if( nx+cnt >= w )
				break;
			if( lpMask[cnt] != 0 )
			{
				num++;
				sum += ptr[cnt];
			}
		}
		lpMask += mw;
		ptr += w;
	}

//	sum /= double(num);

	return sum;
}

LPVOID CreateMask(int rad, int &size)
{
	int arad, c, orop;
	LPVOID bits = NULL;
	HBITMAP hBMP, hOldBMP;
	HDC hDC;
	HPEN hp, op;
	HBRUSH hb, ob;

	// create a mask
	{
		LPBITMAPINFO bb = (LPBITMAPINFO)calloc(1, sizeof(BITMAPINFO)+sizeof(RGBQUAD)*256);
		LPBYTE bipal;

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
		if( NULL == (hDC = ::CreateCompatibleDC(NULL)) )
			goto errexit;

		if( NULL == (hBMP = ::CreateDIBSection(hDC, bb, DIB_RGB_COLORS, &bits, NULL, 0)) )
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
	return bits;
errexit:
	return NULL;
}
*/
/*
typedef struct PEAK_GROUP_
{
	~PEAK_GROUP_() { vPeaks.clear(); }
	double dDist;
	Pt3Vector vPeaks;

} PEAK_GROUP;

typedef std::vector<PEAK_GROUP>					GroupVec;
typedef std::vector<PEAK_GROUP>::iterator		GroupVecIt;
typedef std::vector<PEAK_GROUP>::const_iterator	GroupVecCit;

struct CalcDist : public std::unary_function<CVector3, void>
{
	CalcDist(const CVector2d &dir) : vDir(dir) {}
	void operator()(CVector3 &v)
	{
		v.z = CVector2d(v.x, v.y) & vDir;
	}
	CVector2d vDir;
};

struct Sort_Desc : public std::binary_function<CVector3, CVector3, bool>
{
	bool operator()(const CVector3 &v1, const CVector3 &v2) const
	{
		return v1.z < v2.z;
	}
};

/*
// true if we have 1/2 rows of reflections, false otherwise
bool AnalyzePeaks(OBJ* lpObj, const CVector2d &center, double param, double gamma,
				  const CVector2d &dir, Pt3Vector &vPeaks, Pt3Vector &vPeaksOut,
				  double &dOffset)
{
	GroupVec vGroups, vGoodGroups;
	GroupVecCit git;
	Pt3VectorCit pit, cit;
	PEAK_GROUP group;
	const PEAK_GROUP *pClosestGroup, *pBestClosestGroup;
	double dCurDist, dLimit, dClosestDist, dDiv, dCurChi2, dChi2;
	int nPeaksHalf, nPeaksFull, nCurPeaksHalf, nCurPeaksFull, nPeaks;
	double da;

	if( true == vPeaks.empty() )
	{
		return false;
	}
	dLimit = param * sin(gamma);
	// calculate distance from each peak to the axis (using normal vector to dir-vector)
	std::for_each(vPeaks.begin(), vPeaks.end(), CalcDist(CVector2d(-dir.y, dir.x)));
	// sort in descending order
	std::sort(vPeaks.begin(), vPeaks.end(), Sort_Desc());
	// initialize
	vGroups.clear();
	nPeaks = 1;
	dCurDist = vPeaks.front().z;
	group.vPeaks.push_back(vPeaks.front());
	// search groups of reflections
	da = 0;
	nPeaksFull = 0;
	for(pit = vPeaks.begin()+1; pit != vPeaks.end(); ++pit)
	{
		dDiv = pit->z - floor(pit->z/dLimit + 0.5)*dLimit;
		if( fabs(dDiv) < 0.3*dLimit )
		{
			da += dDiv;
			nPeaksFull++;
		}
		// new group?
		if( fabs(pit->x+center.x - 96) < 2 && fabs(pit->y+center.y - 171) < 2 )
		{
			pit = pit;
		}
		if( fabs(pit->z - dCurDist / nPeaks) < dLimit * 0.1 )
		{
			// same group - add reflection
			group.vPeaks.push_back(*pit);
			dCurDist += pit->z;
			nPeaks++;
//			sprintf(TMP, "%g\t%g\n", pit->x, pit->y);
//			::OutputDebugString(TMP);
		}
		else
		{
			// recalculate the mean distance
			group.dDist = 0;
			for(cit = group.vPeaks.begin(); cit != group.vPeaks.end(); ++cit)
			{
				group.dDist += cit->z;
			}
			group.dDist /= group.vPeaks.size();
			// add the group
			vGroups.push_back(group);
			// re-initialize the current distance
			dCurDist = pit->z;
			// clear reflections
			group.vPeaks.clear();
			group.vPeaks.push_back(*pit);
			nPeaks = 1;
		}
	}
	dOffset = da / nPeaksFull;
	// do we have any groups left?
	if( true == vGroups.empty() )
	{
		return false;
	}
	// find the closest to the axis GROUP of reflections and count other groups
	// which are exactly on the distances of param*sin(gamma) or 1/2*param*sin(gamma)
	dClosestDist = 0;
	nPeaksHalf = nPeaksFull = 0;
	pBestClosestGroup = NULL;
	dChi2 = 1e+33;
	while( true )
	{
		// look for a closest group but not closer than the previously found
		pClosestGroup = NULL;
		dCurDist = fabs(vGroups.front().dDist);
		for(git = vGroups.begin(); git != vGroups.end(); ++git)
		{
#ifdef _DEBUG
			if( fabs(fabs(git->dDist) - 325) < 3 )
			{
				git = git;
				for(cit = git->vPeaks.begin(); cit != git->vPeaks.end(); ++cit)
				{
//					sprintf(TMP, "%g\t%g\n", cit->x+center.x, cit->y+center.y);
//					::OutputDebugString(TMP);
				}
			}
#endif
			if( (fabs(git->dDist) < fabs(dCurDist)) &&
				(fabs(git->dDist) > fabs(dClosestDist)) )
			{
				pClosestGroup = git;
				dCurDist = git->dDist;
			}
		}
		// found nothing
		if( NULL == pClosestGroup )
		{
			break;
		}
		// we went out too far
		if( dCurDist > dLimit*1.1 )
		{
			break;
		}
		// remember the current closest distance
		dClosestDist = dCurDist;
		// look for the groups
		nCurPeaksHalf = nCurPeaksFull = 0;
		dCurChi2 = 0;
		for(git = vGroups.begin(); git != vGroups.end(); ++git)
		{
#ifdef _DEBUG
			if( fabs(fabs(git->dDist) - 325) < 3 )
			{
				git = git;
			}
#endif
			// is this number close to integer or integer+1/2?
			dDiv = fabs((git->dDist - dClosestDist) / dLimit);
			dCurDist = fabs(dDiv - (int)floor(dDiv+0.5));
			if( dCurDist < 0.1 )
			{
				nCurPeaksFull += git->vPeaks.size();
				dCurChi2 += SQR(dCurDist) / git->vPeaks.size();
			}
			else if( fabs(2.0*dDiv - (int)floor(2.0*dDiv+0.5)) < 0.1 )
			{
				nCurPeaksHalf += git->vPeaks.size();
			}
		}
		// look if the current closest group setting is better than the best
		if( NULL == pBestClosestGroup )
		{
			pBestClosestGroup = pClosestGroup;
			nPeaksFull = nCurPeaksFull;
			nPeaksHalf = nCurPeaksHalf;
			dChi2 = dCurChi2;
			sprintf(TMP, "best dist=%g, peaks %d\n", pBestClosestGroup->dDist, nCurPeaksFull);
			::OutputDebugString(TMP);
		}
		else if( nCurPeaksFull > nPeaksFull )
		{
			pBestClosestGroup = pClosestGroup;
			nPeaksFull = nCurPeaksFull;
			nPeaksHalf = nCurPeaksHalf;
			dChi2 = dCurChi2;
			sprintf(TMP, "best dist=%g, peaks %d\n", pBestClosestGroup->dDist, nCurPeaksFull);
			::OutputDebugString(TMP);
		}
	}
	dOffset = pBestClosestGroup->dDist;
	if( fabs(dOffset) > 0.5*dLimit )
	{
		if( dOffset < 0.0 )
		{
			dOffset += dLimit;
		}
		else if( dOffset > 0.0 )
		{
			dOffset -= dLimit;
		}
	}
	for(git = vGroups.begin(); git != vGroups.end(); ++git)
	{
		dDiv = fabs((git->dDist - dOffset) / dLimit);
		dCurDist = fabs(dDiv - (int)floor(dDiv+0.5));
		if( dCurDist < 0.1 )
		{
			for(cit = git->vPeaks.begin(); cit != git->vPeaks.end(); ++cit)
			{
				Mark(lpObj, center.x + cit->x, center.y + cit->y, 1, -3);
//				sprintf(TMP, "%g\t%g\n", cit->x, cit->y);
//				::OutputDebugString(TMP);
			}
		}
	}
	::OutputDebugString("-------------");
	for(cit = vPeaks.begin(); cit != vPeaks.end(); ++cit)
	{
		double y = cit->z / dLimit;
		if( y < 0 )
		{
			y = cit->z - floor(y - 0.5)*dLimit;
		}
		else if( y > 0 )
		{
			y = cit->z - floor(y + 0.5)*dLimit;
		}
		if( y < -dLimit )
		{
			y += dLimit;
		}
		if( y > dLimit )
		{
			y -= dLimit;
		}
		sprintf(TMP, "%g\n", y);
		::OutputDebugString(TMP);
	}
	::OutputDebugString("-------------");
	if( ((nPeaksHalf < nPeaksFull) && (nPeaksHalf >= 0.25*nPeaksFull)) ||
		((nPeaksFull < nPeaksHalf) && (nPeaksFull >= 0.25*nPeaksHalf)) )
	{
		return true;
	}
	return false;
//-----
#if 0
	// now collect all groups which have distances param*sin(gamma) AND separately
	// all groups which have distances 0.5*param*sin(gamma) between each other
	nPeaksHalf = nPeaksFull = 0;
	for(GroupVecCit git = vGroups.begin(); git != vGroups.end(); ++git)
	{
		for(GroupVecCit git2 = vGroups.begin(); git2 != vGroups.end(); ++git2)
		{
			if( git2 == git )
			{
				continue;
			}
			dCurDist = fabs(git2->dDist - git->dDist);
			if( fabs(dCurDist - dLimit) < dLimit*0.15 )
			{
				if( fabs(git->dDist) < fabs(pClosestGroup->dDist) )
				{
					pClosestGroup = git;
				}
				nPeaksFull += git->vPeaks.size();
//				sprintf(TMP, "dist = %g\n", dCurDist);
//				::OutputDebugString(TMP);
				for(Pt3VectorCit cit = git->vPeaks.begin(); cit != git->vPeaks.end(); ++cit)
				{
					Mark(lpObj, center.x + cit->x, center.y + cit->y, 1, -3);
					sprintf(TMP, "%g\t%g\n", cit->x, cit->y);
					::OutputDebugString(TMP);
				}
				break;
			}
			else if( fabs(dCurDist - 0.5*dLimit) < dLimit*0.15 )
			{
				if( fabs(git->dDist) < fabs(pClosestGroup->dDist) )
				{
					pClosestGroup = git;
				}
				nPeaksHalf += git->vPeaks.size();
				break;
			}
		}
	}
//-------------
#endif
}
*/
/*
void LookForReflections(OBJ* lpObj, LPELD lpEld)
{
	int nLaueZone;
	double dRad1, dRad2;//, dOffsetX, dOffsetY;
	CVector2d center, dir_h, dir_k;
	Pt3Vector vPeaks, vNewPeaks;
	Pt3VectorCit cit;

	nLaueZone = lpEld->m_nCurLaueZone;
	// limiting circles
	dRad1 = lpEld->m_vLaueZones[nLaueZone-1]->m_dRadius;
	dRad2 = lpEld->m_vLaueZones[nLaueZone]->m_dRadius;
	// axes direction vectors and center
	center = CVector2d(lpEld->m_x0, lpEld->m_y0);
	dir_h = CVector2d(lpEld->m_hx, lpEld->m_hy);
	dir_k = CVector2d(lpEld->m_kx, lpEld->m_ky);
	dir_h.Normalize();
	dir_k.Normalize();
	float rad = 0.5*min(lpEld->m_al, lpEld->m_bl);
	float x, y, xx, yy, d2, dx, dy;
	WORD amp;
	int a;//, size, num;
//	LPBYTE bits = (LPBYTE)CreateMask(rad, size);
	FILE *pFile = fopen("d:\\eld_proj_pts.txt", "wt");
	for(y = -dRad2; y <= dRad2; y += rad)
	{
		for(x = -dRad2; x <= dRad2; x += rad)
		{
			d2 = SQR(x)+SQR(y);
			xx = x + center.x;
			yy = y + center.y;
			// check the limiting radius
			if( (d2 > SQR(dRad2)) || (d2 < SQR(dRad1)) ||
				(xx < rad) || (xx > lpEld->m_imgw - rad) ||
				(yy < rad) || (yy > lpEld->m_imgh - rad) )
			{
				continue;
			}
			a = PeakCenter(lpObj, rad, &xx, &yy, rad, 0.3, TRUE, &amp);
			if( a )
			{
				continue;
			}
			// look if we already have one
			dx = xx - center.x;
			dy = yy - center.y;
			for(cit = vPeaks.begin(); cit != vPeaks.end(); ++cit)
			{
				if( SQR(cit->x - dx) + SQR(cit->y - dy) < 3.0 )
				{
					break;
				}
			}
			// didn't find any
			if( cit == vPeaks.end() )
			{
				vPeaks.push_back(CVector3(dx, dy, 0));
			}
//			Mark(lpObj, xx, yy, 1, -3);
//			if( pFile )
//			{
//				fprintf(pFile, "%g\t%g\n", CVector2d(xx-center.x, yy-center.y)&dir_k,
//					CVector2d(xx-center.x, yy-center.y)&dir_h);
//			}
#ifdef 0
			if( (d2 > SQR(dRad2)) || (d2 < SQR(dRad1)) ||
				(xx < rad) || (xx > lpEld->m_imgw - rad) ||
				(yy < rad) || (yy > lpEld->m_imgh - rad) )
			{
				continue;
			}
			if( 1 == lpEld->m_pix )
			{
				double bkg = sum_along_circle<BYTE>((LPBYTE)lpEld->m_img,
					lpEld->m_imgw, lpEld->m_imgh, xx, yy, rad);
				double area = sum_inside_circle<BYTE>((LPBYTE)lpEld->m_img,
					lpEld->m_imgw, lpEld->m_imgh, xx, yy, rad,
					(LPBYTE)bits, R4(size), size, num);
				if( num > 0 )
				{
					if( area/num > 1.1*bkg )
					{
						Mark(lpObj, xx, yy, 1, -3);
					}
				}
			}
#endif // 0
		}
	}
	if( pFile )
	{
		fclose(pFile);
	}
	// analyze peak distances to H-axis
//	AnalyzePeaks(lpObj, center, lpEld->m_bl, lpEld->m_gamma, dir_h, vPeaks, vNewPeaks, dOffsetY);
	// analyze peak distances to K-axis
//	AnalyzePeaks(lpObj, center, lpEld->m_al, lpEld->m_gamma, dir_k, vPeaks, vNewPeaks, dOffsetX);
//	::OutputDebugString("FINAL PEAKS:\n");
//	for(cit = vPeaks.begin(); cit != vPeaks.end(); ++cit)
//	{
//		dx = cit->x - dOffsetX;
//		dy = cit->y - dOffsetY;
//		if( SQR(cit->x - dx) + SQR(cit->y - dy) < 3.0 )
//		{
//			sprintf(TMP, "%g\t%g\n", cit->x, cit->y);
//			::OutputDebugString(TMP);
//		}
//	}
}
*/
/*
void CalculateProfileXY(OBJ* lpObj, LPELD lpEld,
						VectorF &vProfileX, VectorF &vProfileY, int &nCentre)
{
	int nLaueZone, nStartR, nEndR, nStartX, nEndX, nCurX, nCurY, nOff, nStride;
	double dRad1, dRad2, sum_x, sum_y;
	CVector2d dir_h, dir_k, center, point;
	LPVOID lpData;

	nLaueZone = lpEld->m_nCurLaueZone;
	// limiting circles
	dRad1 = lpEld->m_vLaueCircles[nLaueZone-1];
	dRad2 = lpEld->m_vLaueCircles[nLaueZone];
	// axes direction vectors and center
	center = CVector2d(lpEld->m_x0, lpEld->m_y0);
	dir_h = CVector2d(lpEld->m_hx, lpEld->m_hy);
	dir_k = CVector2d(lpEld->m_kx, lpEld->m_ky);
	dir_h.Normalize();
	dir_k.Normalize();
	nStartR = (int)floor(dRad1);
	nEndR   = (int)floor(dRad2 + 0.5);
	nStride = lpEld->m_imgw;
	lpData = lpEld->m_img;
	if( 2 == lpEld->m_pix )
	{
		nStride *= 2;
	}
	vProfileX.resize(nEndR*2 + 1, 0);
	vProfileY.resize(nEndR*2 + 1, 0);
	for(nCurY = -nEndR, nOff = 0; nCurY <= nEndR; nCurY++, nOff++)
	{
		nStartX = 0;
		if( abs(nCurY) < nStartR )
		{
			nStartX = (int)floor(sqrt(SQR(nStartR) - SQR(nCurY)));
		}
		nEndX = (int)floor(sqrt(SQR(nEndR) - SQR(nCurY)));
		sum_x = sum_y = 0;
		for(nCurX = nStartX; nCurX <= nEndX; nCurX++)
		{
			// x-direction
			point = center + (dir_h*nCurX + dir_k*nCurY);
			if( (point.x < 0) || (point.x >= lpEld->m_imgw) ||
				(point.y < 0) || (point.y >= lpEld->m_imgh) )
			{
				continue;
			}
			if( 1 == lpEld->m_pix )
			{
				sum_x += Bilinear<BYTE>((LPBYTE)lpData, nStride, point.x, point.y);
			}
			else if( 2 == lpEld->m_pix )
			{
				sum_x += Bilinear<WORD>((LPWORD)lpData, nStride, point.x, point.y);
			}
			// y-direction
			point = center + (dir_h*nCurY + dir_k*nCurX);
			if( (point.x < 0) || (point.x >= lpEld->m_imgw) ||
				(point.y < 0) || (point.y >= lpEld->m_imgh) )
			{
				continue;
			}
			if( 1 == lpEld->m_pix )
			{
				sum_y += Bilinear<BYTE>((LPBYTE)lpData, nStride, point.x, point.y);
			}
			else if( 2 == lpEld->m_pix )
			{
				sum_y += Bilinear<WORD>((LPWORD)lpData, nStride, point.x, point.y);
			}
		}
		vProfileX[nOff] = sum_x;
		vProfileY[nOff] = sum_y;
	}
#ifdef _DEBUG
	FILE *pFile;
	if( NULL != (pFile = fopen("d:\\eld_profile_x.txt", "wt")) )
	{
		for(nCurX = 0; nCurX < vProfileX.size(); nCurX++)
		{
			fprintf(pFile, "%d\t%g\n", nCurX, vProfileX[nCurX]);
		}
		fclose(pFile);
	}
	if( NULL != (pFile = fopen("d:\\eld_profile_y.txt", "wt")) )
	{
		for(nCurX = 0; nCurX < vProfileY.size(); nCurX++)
		{
			fprintf(pFile, "%d\t%g\n", nCurX, vProfileY[nCurX]);
		}
		fclose(pFile);
	}
#endif
	nCentre = nEndR;//+1;
}
/*
static int ScanInterval(VectorFIt begin, VectorFIt end, int m, int beta, bool bReverse)
{
	int p, n;
	double bkg, mul, val, prev_val, sum_I;
	VectorFIt cur;

	n = end - begin;
	prev_val = 10e+30;
	double inv_beta = 1.0 / (double) beta;
	cur = begin + m;
	sum_I = *cur;
	mul = 2.0 * inv_beta;
	if( !bReverse )
		p = m + 1;
	else
		p = m - 1;
	while( 1 )
	{
		if( !bReverse )
		{
			if( p > n - beta )
				break;
			cur++;
			sum_I += *cur;
			bkg = std::accumulate(cur + 1, cur + beta + 1, 0.0);
			p++;
		}
		else
		{
			if( p < beta )
				break;
			cur--;
			sum_I += *cur;
			bkg = std::accumulate(cur - beta, cur, 0.0);
			p--;
		}
		val = (sum_I + SQR(mul)*bkg) / (sum_I - mul*bkg);
		if( val > prev_val )
		{
			break;
		}
		prev_val = val;
		mul += inv_beta;
	}
	return (bReverse) ? p+1 : p-1;
}

bool DoPeakSearch(VectorFIt begin, VectorFIt end, std::vector<PEAKSEARCH> &vPeaks, int iStart)
{
	double sx, sa;
	int i, m, startP, endP;
	PEAKSEARCH	pi;
	VectorFIt max_el;

	max_el = std::max_element(begin, end);
	m = max_el - begin;	// the point of the maximum
	if( (0 == m) || (max_el == end-1) )
		return false;

	// scan left interval
	startP = ScanInterval(begin, end, m, 3, TRUE);
	// scan right interval
	endP = ScanInterval(begin, end, m, 3, FALSE);
	pi.piStart = startP+iStart;
	pi.piPos = m+iStart;
	pi.piEnd = endP+iStart;
	sx = sa = 0;
	for(i = pi.piStart; i <= pi.piEnd; i++)
	{
		sx += begin[i];
		sa += i*begin[i];
	}
	pi.piWghtPos = sa / sx;
	sprintf(TMP, "peak: start:%d\tend:%d\tpos:%d\n", pi.piStart, pi.piEnd, (int)pi.piPos);
	::OutputDebugString(TMP);
//	TRACE3("peak: start:%d\tend:%d\tpos:%d\n", pi.piStart, pi.piEnd, (int)pi.piPos);
	vPeaks.push_back(pi);
	DoPeakSearch(begin, begin+startP, vPeaks, iStart);
	DoPeakSearch(begin+endP, end, vPeaks, endP+iStart);

	return true;
}

void CalculateProfiles(OBJ* lpObj, LPELD lpEld, double &dShiftX, double &dShiftY)
{
	VectorF vProfileX, vProfileY;
	int nCentre, nFirstX, nLastX;

	dShiftX = dShiftY = 0.0;
	CalculateProfileXY(lpObj, lpEld, vProfileX, vProfileY, nCentre);

	if( lpEld->m_bl > lpEld->m_al )
	{
		nFirstX = (int)floor(nCentre - lpEld->m_bl);//0.5);
		nLastX  = (int)floor(nCentre + lpEld->m_bl+0.5);//0.5 + 0.5);
		// analyze vProfileX
		if( nFirstX < 0 )
		{
			nFirstX = 0;
		}
		if( nLastX >= vProfileX.size() )
		{
			nLastX = vProfileX.size() - 1;
		}
		if( nFirstX >= nLastX )
		{
			::OutputDebugString("ELD_HOLZ -> DoPeakSearch() -> too small profile.\n");
			return;
		}

		VectorFIt begin = vProfileX.begin() + nFirstX;
		VectorFIt end   = vProfileX.begin() + nLastX;
		std::vector<PEAKSEARCH> vPeaks;
		std::vector<PEAKSEARCH>::iterator pit;
		if( (false == DoPeakSearch(begin, end, vPeaks, 0)) ||
			(true == vPeaks.empty()) )
		{
			return;
		}
		float min_dist = lpEld->m_bl;
		float fPeakPos = vPeaks.front().piWghtPos + nFirstX;
		// look the peak closest to the center
		for(pit = vPeaks.begin(); pit != vPeaks.end(); ++pit)
		{
			if( fabs(pit->piWghtPos + nFirstX - nCentre) < min_dist )
			{
				fPeakPos = pit->piWghtPos + nFirstX;
				min_dist = fabs(fPeakPos - nCentre);
			}
		}
		dShiftY = min_dist;
		sprintf(TMP, "origin shift for reflections along Y %g\n", dShiftY);
		::OutputDebugString(TMP);
	}
	else
	{
		// analyze vProfileY
	}
}
// If 1/2 Shift then the offset cannot be applied!!!
*/
