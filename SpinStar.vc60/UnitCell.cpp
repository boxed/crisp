// UnitCell.cpp: implementation of the CUnitCell class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "MathKrnl.h"
#include "SpinningStar/UnitCell.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

using namespace std;

namespace UnitCell
{

#define		_AL		0
#define		_BT		1
#define		_GM		2

static int	IIS[] = {0,0,0,0,0, 4,6,0,0,0, 4,5,6,0,0, 2,4,5,6,0,
			 2,4,5,6,0, 2,3,5,6,0, 2,3,4,5,6};

static int IIG[] = {0,0,0,0,0, 0,0,0,0,0, 0,0,0,0,0,
			 1,0,0,0,0, 1,1,0,0,0, 1,1,4,4,0, 1,1,0,0,0};

static double GII[] = {0.0,0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0,0.0, 0.0,0.0,0.0,0.0,0.0,
			   1.0,0.0,0.0,0.0,0.0, 1.0,0.5,0.0,0.0,0.0, 1.0,1.0,1.0,1.0,0.0,
			   1.0,1.0,0.0,0.0,0.0};

static int IVAR[] = {1,2,3,4,5,6, 1,2,3,0,5,0, 1,2,3,0,0,0, 1,1,3,0,0,0,
			  1,1,3,0,0,0, 1,1,1,4,4,4, 1,1,1,0,0,0};

//////////////////////////////////////////////////////////////////////////
//	local functions

// CMatrix A(7, 14) CVector C[7], CMatrix Der[6][6], CVector Esd2[7]
void Metric(CMatrix &A, CVector &C, CMatrix &Der, CVector &Esd2);

class CUnitCell2D : public CUnitCell
{
// constructor/destructor
public:
	CUnitCell2D();
	virtual ~CUnitCell2D()	{ }

	void CalculateVolume(CELL_TYPE nType);		// calculates the volume
	void CalculateParameters(CELL_TYPE nType);	// calculates the cell parameters
	double CalculateH(const HKLMiller &hkl,
		CELL_TYPE nType = REAL);	// calculate |H| = 1 / d for given HKL data

	void SetParameters(const double *pParams, ANGLES nAngles = DEGREES, CELL_TYPE nType = REAL);	// set the parameters
	void SetParameters(const float *pParams, ANGLES nAngles = DEGREES, CELL_TYPE nType = REAL);	// set the parameters

	double GetParameter(UINT nParam, CELL_TYPE nType = REAL) const;
	const double *GetParameters(CELL_TYPE nType = REAL)
	{
		if( REAL == nType ) return a;
		else if( RECIPROCAL == nType ) return ra;
		return a;
	}
	double GetCosine(UINT nParam, CELL_TYPE nType = REAL);
	double GetSine(UINT nParam, CELL_TYPE nType = REAL);
	double GetVolume(CELL_TYPE nType = REAL);

	BOOL RefineParameters(SYMMETRY nSymmetry, CReflVector &vReflexes);

	// coordinate transformations
	void CalcTransformMatrix();

protected:
	// real parameters
	double a[3];		// plane unit cell parameters: a, b, c, Gamma
	double CosA;		// cosine of the Gamma angle
	double SinA;		// sine of the Gamma angle
	double Area;		// volume
	// reciprocal parameters
	double ra[3];
	double rCosA;
	double rSinA;
	double rArea;
	// error metric
//		CVector Esds;
};

class CUnitCell3D : public CUnitCell
{
// constructor/destructor
public:
	CUnitCell3D();
	virtual ~CUnitCell3D()	{ }

	void CalculateVolume(CELL_TYPE nType);		// calculates the volume
	void CalculateParameters(CELL_TYPE nType);	// calculates the cell parameters
	double CalculateH(const HKLMiller &hkl,
		CELL_TYPE nType = REAL);	// calculate |H| = 1 / d for given HKL data

	void SetParameters(const double *pParams, ANGLES nAngles = DEGREES, CELL_TYPE nType = REAL);	// set the parameters
	void SetParameters(const float *pParams, ANGLES nAngles = DEGREES, CELL_TYPE nType = REAL);	// set the parameters

	double GetParameter(UINT nParam, CELL_TYPE nType = REAL) const;
	const double *GetParameters(CELL_TYPE nType = REAL)
	{
		if( REAL == nType ) return a;
		else if( RECIPROCAL == nType ) return ra;
		return a;
	}
	double GetCosine(UINT nParam, CELL_TYPE nType = REAL);
	double GetSine(UINT nParam, CELL_TYPE nType = REAL);
	double GetVolume(CELL_TYPE nType = REAL);

	BOOL RefineParameters(SYMMETRY nSymmetry, CReflVector &vReflexes);

	// coordinate transformations
	void CalcTransformMatrix();
	void GetMatrix(CMatrix3x3 &out) { out = m_mCryst2Cartes; }
	void TransformPoint(const CVector3d &in, CVector3d &out)
	{
		out = m_mCryst2Cartes * in;
	}
	void TransformPoint(const CVector3f &in, CVector3f &out)
	{
		out = m_mCryst2Cartes * in;
	}
	void TransformRPoint(const CVector3d &in, CVector3d &out)
	{
		out = m_mInvCryst2Cartes * in;
	}
	void TransformRPoint(const CVector3f &in, CVector3f &out)
	{
		out = m_mInvCryst2Cartes * in;
	}

protected:
	// real parameters
	double a[6];		// unit cell parameters: a, b, c, Alpha, Beta, Gamma
	double CosA[3];		// cosine of the Alpha, Beta and Gamma angles
	double SinA[3];		// sine of the Alpha, Beta and Gamma angles
	double V;		// volume
	// reciprocal parameters
	double ra[6];
	double rCosA[3];
	double rSinA[3];
	double rV;
	// error metric
	CVector Esds;
};
//////////////////////////////////////////////////////////////////////
// Unit cell construction
//////////////////////////////////////////////////////////////////////

CUnitCell *CUnitCell::CreateUnitCell(CELL_DIMENSION nDim)
{
	CUnitCell *pUnitCell = NULL;
	if( CELL_2D == nDim )
	{
		pUnitCell = new CUnitCell2D;
	}
	else if( CELL_3D == nDim )
	{
		pUnitCell = new CUnitCell3D;
	}
	return pUnitCell;
}

bool CUnitCell::DestroyUnitCell(CUnitCell *pUnitCell)
{
	if( NULL != pUnitCell )
	{
		delete pUnitCell;
		return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUnitCell2D::CUnitCell2D()
//	: Esds(7)
{
	ZeroMemory(a, sizeof(a));
	ZeroMemory(ra, sizeof(ra));
	CosA = SinA = 0.0;
	rCosA = rSinA = 0.0;
	rArea = Area = 0.0;

	m_nCellDimension = CELL_2D;
}

double CUnitCell2D::GetParameter(UINT nParam, CELL_TYPE nType/*= REAL*/) const
{
	if( nParam < 3 )
	{
		if( nType == REAL )
		{
			return a[nParam];
		}
		else if( nType == RECIPROCAL )
		{
			return ra[nParam];
		}
		else
		{
			throw _T("CUnitCell2D::GetParameter() -> wrong unit cell type.");
		}
	}
	else
	{
		throw _T("CUnitCell2D::GetParameter() -> wrong parameter index.");
	}
}

double CUnitCell2D::GetCosine(UINT nParam, CELL_TYPE nType/*= REAL*/)
{
	if( nParam > 0 )
	{
		throw _T("CUnitCell2D::GetCosine() -> wrong angle index.");
	}
	if( nType == REAL )
	{
		return CosA;
	}
	else if( nType == RECIPROCAL )
	{
		return rCosA;
	}
	else
	{
		throw _T("CUnitCell2D::GetParameter() -> wrong unit cell type.");
	}
}

double CUnitCell2D::GetSine(UINT nParam, CELL_TYPE nType)
{
	if( nParam > 0 )
	{
		throw _T("CUnitCell2D::GetSine() -> wrong parameter index.");
	}
	if( nType == REAL )
	{
		return SinA;
	}
	else if( nType == RECIPROCAL )
	{
		return rSinA;
	}
	else
	{
		throw _T("CUnitCell2D::GetParameter() -> wrong unit cell type.");
	}
}

double CUnitCell2D::GetVolume(CELL_TYPE nType)
{
	if( nType == REAL )
	{
		return Area;
	}
	else if( nType == RECIPROCAL )
	{
		return rArea;
	}
	else
	{
		throw _T("CUnitCell2D::GetParameter() -> wrong unit cell type.");
	}
}

void CUnitCell2D::SetParameters(const double *pParams, ANGLES nAngles, CELL_TYPE nType)
{
	double params[3];

	// copy the parameters for own usage
	memcpy(params, pParams, sizeof(double)*3);

	switch(nAngles)
	{
	case DEGREES:
		params[2] = DEG2RAD(params[2]);
		break;

	case RADIANS:
		break;

	default:
		throw _T("CUnitCell2D::SetParameters() -> wrong angles type.");
	}

	switch(nType)
	{
	case REAL:
		{
			// a and b parameters
			a[0] = params[0];
			a[1] = params[1];
			// the angle
			a[2] = params[2];
			FastSinCos(a[2], SinA, CosA);
			if( FastAbs(SinA) < 1e-6 )
				SinA = 0.0;
			if( FastAbs(CosA) < 1e-6 )
				CosA = 0.0;
			CalculateVolume(REAL);
			CalculateParameters(RECIPROCAL);
			break;
		}
	case RECIPROCAL:
		{
			// a and b parameters
			ra[0] = params[0];
			ra[1] = params[1];
			// the angle
			ra[2] = params[2];
			FastSinCos(ra[2], rSinA, rCosA);
			if( FastAbs(rSinA) < 1e-6 )
				rSinA = 0.0;
			if( FastAbs(rCosA) < 1e-6 )
				rCosA = 0.0;
			CalculateVolume(RECIPROCAL);
			CalculateParameters(REAL);
			break;
		}
	default:
		throw _T("CUnitCell2D::SetParameters() -> unknown cell type.");
	}
	// coordinates transformation matrix
	CalcTransformMatrix();
}

void CUnitCell2D::SetParameters(const float *pParams, ANGLES nAngles, CELL_TYPE nType)
{
	double params[3];
	for(int i = 0; i < 3; i++)
	{
		params[i] = pParams[i];
	}
	SetParameters(params, nAngles, nType);
}
// calculate volume using real/reciprocal parameters
void CUnitCell2D::CalculateVolume(CELL_TYPE nType)
{
	switch(nType)
	{
	case REAL:
		{
			Area = a[0] * a[1] * SinA;
			break;
		}
	case RECIPROCAL:
		{
			rArea = ra[0] * ra[1] * rSinA;
			break;
		}
	default:
		throw _T("Unknown cell type.");
	}
}
// calculate real/reciprocal parameters using opposite parameters
void CUnitCell2D::CalculateParameters(CELL_TYPE nType)
{
	switch(nType)
	{
	case REAL: // calculate real from reciprocal
		{
			CalculateVolume(RECIPROCAL);
			a[0] = 1.0 / (ra[0] * rSinA);
			a[1] = 1.0 / (ra[1] * rSinA);
			SinA =  rSinA;
			CosA = -rCosA;
			Area = 1.0 / rArea;
			// the angle
			a[2] = acos(CosA);
			break;
		}
	case RECIPROCAL: // calculate reciprocal from real
		{
			CalculateVolume(REAL);
			ra[0] = 1 / (a[0] * SinA);
			ra[1] = 1 / (a[1] * SinA);
			rSinA =  SinA;
			rCosA = -CosA;
			rArea = 1.0 / Area;
			// the angle
			ra[2] = acos(rCosA);
			break;
		}
	default:
		throw _T("Unknown cell type.");
	}
}
// calculates H = 1 / d using reciprocal and real parameters
double CUnitCell2D::CalculateH(const HKLMiller &hkl, CELL_TYPE nType)
{
	double dH = 0.0;
	HKLindex H = hkl.x, K = hkl.y;

	switch(nType)
	{
	case REAL:
		dH = FastSqrt(SQR(H/a[0]) + SQR(K/a[1]) - 2*H*K*CosA/(a[0]*a[1])) / SinA;
		break;
	case RECIPROCAL:
		dH = FastSqrt(SQR(H*ra[0]) + SQR(K*ra[1]) + 2*H*K*ra[0]*ra[1]*rCosA);
		break;
	default:
		throw _T("Unknown cell type.");
	}

	return dH;
}

void CUnitCell2D::CalcTransformMatrix()
{
	m_mCryst2Cartes.d[0][0] = a[0];
	m_mCryst2Cartes.d[0][1] = a[1] * CosA;
	m_mCryst2Cartes.d[0][2] = 0.0;

	m_mCryst2Cartes.d[1][0] = 0.0;
	m_mCryst2Cartes.d[1][1] = a[1] * SinA;
	m_mCryst2Cartes.d[1][2] = 0.0;

	m_mCryst2Cartes.d[2][0] = 0.0;
	m_mCryst2Cartes.d[2][1] = 0.0;
	m_mCryst2Cartes.d[2][2] = 1.0;

	m_mCryst2Cartes.Invert(m_mInvCryst2Cartes);

	m_mRecCryst2Cartes.d[0][0] = ra[0];
	m_mRecCryst2Cartes.d[0][1] = ra[1] * rCosA;
	m_mRecCryst2Cartes.d[0][2] = 0.0;

	m_mRecCryst2Cartes.d[1][0] = 0.0;
	m_mRecCryst2Cartes.d[1][1] = ra[1] * rSinA;
	m_mRecCryst2Cartes.d[1][2] = 0.0;

	m_mRecCryst2Cartes.d[2][0] = 0.0;
	m_mRecCryst2Cartes.d[2][1] = 0.0;
	m_mRecCryst2Cartes.d[2][2] = 1.0;

	m_mRecCryst2Cartes.Invert(m_mInvRecCryst2Cartes);
}

BOOL CUnitCell2D::RefineParameters(SYMMETRY nSymmetry, CReflVector &vReflexes)
{
	return FALSE;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CUnitCell3D::CUnitCell3D()
	: Esds(7)
{
	ZeroMemory(a, sizeof(a));
	ZeroMemory(CosA, sizeof(CosA));
	ZeroMemory(SinA, sizeof(SinA));
	ZeroMemory(ra, sizeof(ra));
	ZeroMemory(rCosA, sizeof(rCosA));
	ZeroMemory(rSinA, sizeof(rSinA));
	rV = V = 0.0;

	m_nCellDimension = CELL_3D;
}

double CUnitCell3D::GetParameter(UINT nParam, CELL_TYPE nType/*= REAL*/) const
{
	if( nParam < 6 )
	{
		if( nType == REAL )
		{
			return a[nParam];
		}
		else if( nType == RECIPROCAL )
		{
			return ra[nParam];
		}
		else
		{
			throw _T("CUnitCell3D::GetParameter() -> wrong unit cell type.");
		}
	}
	else
	{
		throw _T("CUnitCell3D::GetParameter() -> wrong parameter index.");
	}
}

double CUnitCell3D::GetCosine(UINT nParam, CELL_TYPE nType/*= REAL*/)
{
	if( nParam >= 3 )
	{
		throw _T("CUnitCell3D::GetCosine() -> wrong parameter index.");
	}
	if( nType == REAL )
	{
		return CosA[nParam];
	}
	else if( nType == RECIPROCAL )
	{
		return rCosA[nParam];
	}
	else
	{
		throw _T("CUnitCell3D::GetParameter() -> wrong unit cell type.");
	}
}

double CUnitCell3D::GetSine(UINT nParam, CELL_TYPE nType)
{
	if( nParam >= 3 )
	{
		throw _T("CUnitCell3D::GetSine() -> wrong parameter index.");
	}
	if( nType == REAL )
	{
		return SinA[nParam];
	}
	else if( nType == RECIPROCAL )
	{
		return rSinA[nParam];
	}
	else
	{
		throw _T("CUnitCell3D::GetParameter() -> wrong unit cell type.");
	}
}


double CUnitCell3D::GetVolume(CELL_TYPE nType)
{
	if( nType == REAL )
	{
		return V;
	}
	else if( nType == RECIPROCAL )
	{
		return rV;
	}
	else
	{
		throw _T("CUnitCell3D::GetParameter() -> wrong unit cell type.");
	}
}


void CUnitCell3D::SetParameters(const double *pParams, ANGLES nAngles, CELL_TYPE nType)
{
	int i;
	double params[6];

	// copy the parameters for own usage
	memcpy(params, pParams, sizeof(double)*6);

	switch(nAngles)
	{
	case DEGREES:
		params[ALPHA] = DEG2RAD(params[ALPHA]);
		params[BETA]  = DEG2RAD(params[BETA]);
		params[GAMMA] = DEG2RAD(params[GAMMA]);
		break;

	case RADIANS:
		break;

	default:
		throw _T("CUnitCell3D::SetParameters() -> wrong angles type.");
	}

	switch(nType)
	{
	case REAL:
		{
			for(i = 0; i < 3; i++)
			{
				a[i] = params[i];
			}
			for(i = 3; i < 6; i++)
			{
				a[i] = params[i];
				SinA[i-3] = sin(params[i]);
				CosA[i-3] = cos(params[i]);
				if( fabs(SinA[i-3]) < 1e-6 )
					SinA[i-3] = 0.0;
				if( fabs(CosA[i-3]) < 1e-6 )
					CosA[i-3] = 0.0;
			}
			CalculateVolume(REAL);
			CalculateParameters(RECIPROCAL);
			break;
		}
	case RECIPROCAL:
		{
			for(i = 0; i < 3; i++)
			{
				ra[i] = params[i];
			}
			for(i = 3; i < 6; i++)
			{
				ra[i] = params[i];
				rSinA[i-3] = sin(params[i]);
				rCosA[i-3] = cos(params[i]);
				if( fabs(rSinA[i-3]) < 1e-6 )
					rSinA[i-3] = 0.0;
				if( fabs(rCosA[i-3]) < 1e-6 )
					rCosA[i-3] = 0.0;
			}
			CalculateVolume(RECIPROCAL);
			CalculateParameters(REAL);
			break;
		}
	default:
		throw _T("CUnitCell3D::SetParameters() -> unknown cell type.");
	}
	// calculate transformation matrix
	CalcTransformMatrix();
}

void CUnitCell3D::SetParameters(const float *pParams, ANGLES nAngles, CELL_TYPE nType)
{
	double params[6];
	for(int i = 0; i < 6; i++)
	{
		params[i] = pParams[i];
	}
	SetParameters(params, nAngles, nType);
}
// calculate volume using real/reciprocal parameters
void CUnitCell3D::CalculateVolume(CELL_TYPE nType)
{
	switch(nType)
	{
	case REAL:
		{
			V = a[_A]*a[_B]*a[_C] *
				FastSqrt( 1.0 - SQR(CosA[_AL]) - SQR(CosA[_BT]) - SQR(CosA[_GM]) +
					  2.0 * CosA[_AL] * CosA[_BT] * CosA[_GM] );
			break;
		}
	case RECIPROCAL:
		{
			rV = ra[_A]*ra[_B]*ra[_C] *
				FastSqrt( 1.0 - SQR(rCosA[_AL]) - SQR(rCosA[_BT]) - SQR(rCosA[_GM]) +
					  2.0 * rCosA[_AL] * rCosA[_BT] * rCosA[_GM] );
			break;
		}
	default:
		throw _T("Unknown cell type.");
	}
}
// calculate real/reciprocal parameters using opposite parameters
void CUnitCell3D::CalculateParameters(CELL_TYPE nType)
{
	double *pIParam, *pCParam, *pISin, *pICos, *pCSin, *pCCos, *pIVol, *pCVol;
	switch(nType)
	{
	case REAL:
		{
			CalculateVolume(RECIPROCAL);
			// set initial params to reciprocal
			pIParam = ra;
			pISin = rSinA;
			pICos = rCosA;
			pIVol = &rV;
			// and calculated to real
			pCParam = a;
			pCSin = SinA;
			pCCos = CosA;
			pCVol = &V;
			break;
		}
	case RECIPROCAL:
		{
			CalculateVolume(REAL);
			// set initial params to real
			pIParam = a;
			pISin = SinA;
			pICos = CosA;
			pIVol = &V;
			// and calculated to reciprocal
			pCParam = ra;
			pCSin = rSinA;
			pCCos = rCosA;
			pCVol = &rV;
			break;
		}
	default:
		throw _T("Unknown cell type.");
	}
	int i, p1, p2, prm[] = {0, 1, 2, 0, 1, 2};
	double dIVol = (*pIVol) / (pIParam[_A] * pIParam[_B] * pIParam[_C]);

	// get volume first
	*pCVol = 1.0 / (*pIVol);
	for(i = 0; i < 3; i++)
	{
		// calculate i-th parameter
		p1 = prm[i + 1];
		p2 = prm[i + 2];
		pCParam[i] = pIParam[p1] * pIParam[p2] * pISin[i] / (*pIVol);
		// calculate sine and cosine
		pCSin[i] = dIVol / (pISin[p1] * pISin[p2]);
		pCCos[i] = (pICos[p1]*pICos[p2] - pICos[i]) / (pISin[p1] * pISin[p2]);
		// calculate angle from sine
		pCParam[i+3] = acos(pCCos[i]);
	}
}
// calculates H = 1 / d using reciprocal and real parameters
double CUnitCell3D::CalculateH(const HKLMiller &hkl, CELL_TYPE nType)
{
	double dH = 0, icos;
	HKLindex H = hkl.x, K = hkl.y, L = hkl.z;

	switch(nType)
	{
	case REAL:
		{
			icos = 1.0 / ( 1.0 - SQR(CosA[_AL]) - SQR(CosA[_BT]) - SQR(CosA[_GM]) +
						   2.0 * CosA[_AL] * CosA[_BT] * CosA[_GM] );
			dH = SQR(H*SinA[_AL]/a[_A]) + SQR(K*SinA[_BT]/a[_B]) + SQR(L*SinA[_GM]/a[_C]);
			dH += 2*K*L*(CosA[_BT]*CosA[_GM]-CosA[_AL]) / (a[_B]*a[_C]);
			dH += 2*H*L*(CosA[_AL]*CosA[_GM]-CosA[_BT]) / (a[_A]*a[_C]);
			dH += 2*H*K*(CosA[_AL]*CosA[_BT]-CosA[_GM]) / (a[_A]*a[_B]);
			dH *= icos;
			break;
		}
	case RECIPROCAL:
		{
			dH = SQR(H * ra[_A]) + SQR(K * ra[_B]) + SQR(L * ra[_C]) + 
				2.0 * (	H*K*ra[_A]*ra[_B]*rCosA[_GM] +
						H*L*ra[_A]*ra[_C]*rCosA[_BT] +
						K*L*ra[_B]*ra[_C]*rCosA[_AL] );
			
			break;
		}
	default:
		throw _T("Unknown cell type.");
	}

	return FastSqrt(dH);
}

void CUnitCell3D::CalcTransformMatrix()
{
	m_mCryst2Cartes.LoadIdentity();

	m_mCryst2Cartes.d[0][0] = a[_A];
	m_mCryst2Cartes.d[0][1] = a[_B] * CosA[_C];
	m_mCryst2Cartes.d[0][2] = a[_C] * CosA[_B];

	m_mCryst2Cartes.d[1][1] =  a[_B] * SinA[_C];
	m_mCryst2Cartes.d[1][2] = -a[_C] * SinA[_B] * rCosA[_A];

	m_mCryst2Cartes.d[2][2] = a[_C] * SinA[_B] * rSinA[_A];

	m_mCryst2Cartes.Invert(m_mInvCryst2Cartes);

	m_mRecCryst2Cartes.LoadIdentity();

	m_mRecCryst2Cartes.d[0][0] = ra[_A];
	m_mRecCryst2Cartes.d[0][1] = ra[_B] * rCosA[_C];
	m_mRecCryst2Cartes.d[0][2] = ra[_C] * rCosA[_B];

	m_mRecCryst2Cartes.d[1][1] =  ra[_B] * rSinA[_C];
	m_mRecCryst2Cartes.d[1][2] = -ra[_C] * rSinA[_B] * CosA[_A];

	m_mRecCryst2Cartes.d[2][2] = ra[_C] * rSinA[_B] * SinA[_A];

	m_mRecCryst2Cartes.Invert(m_mInvRecCryst2Cartes);
}

/*
#define		MAX_H	3
#define		MAX_K	3
#define		MAX_L	4

void Run()
{
	FILE *pFile = NULL;
	_HKL hkl;
	CUnitCell3D cell;
	double dH, Params[] = {5.21, 5.21, 7.31, DEG2RAD(90.0), DEG2RAD(90.0), DEG2RAD(120.0)};

	if( (pFile = fopen("TPtest.txt", "wt")) == NULL )
	{
		cout << "Cannot open file for output.\n";
		return;
	}

	cell.SetParameters(Params, CUnitCell3D::REAL);
	cell.CalculateVolume(CUnitCell3D::REAL);
	cell.CalculateParameters(CUnitCell3D::RECIPROCAL);

	double Phi = DEG2RAD(60.0);
	double sPhi = sin(Phi);
	double D, X;

	for(int H = 0; H <= MAX_H; H++)
	{
		for(int K = 0; K <= MAX_K; K++)
		{
			for(int L = 0; L <= MAX_L; L++)
			{
				hkl.H = H;
				hkl.K = K;
				hkl.L = L;
				
				dH = cell.CalculateH(hkl, CUnitCell3D::RECIPROCAL);
//				dH = cell.CalculateH(hkl, CUnitCell3D::REAL);
//				D = sqrt(SQR(L*cell.ra[_C]) + 2*L*cell.ra[_C]*(H*cell.ra[_A]+K*cell.ra[_B])) / sPhi;
				D = (H*cell.ra[_A]*cell.rCosA[_BT] + K*cell.ra[_B]*cell.rCosA[_AL] + L*cell.ra[_C]) / sPhi;

				if( D <= dH )
				{
					X = sqrt(SQR(dH) - SQR(D));
					fprintf(pFile, "%12.6f", dH);
					fprintf(pFile, "%12.6f", D);
					fprintf(pFile, "%12.6f", X);
					// ellipse minor axis
					hkl.L = 0;
					dH = cell.CalculateH(hkl, CUnitCell3D::RECIPROCAL);
					fprintf(pFile, "%12.6f", dH);
					fprintf(pFile, "%6d%6d%6d\n", H, K, L);
				}
			}
		}
	}

	fclose(pFile);
}
*/


BOOL CUnitCell3D::RefineParameters(SYMMETRY nSymmetry, CReflVector &vReflexes)
{
	int				IS[5], IG[5], i, j, k, l, m, n, H, K, L, Nind, Nvar;
	double			BB, R = 0, Q, Qc;
	CMatrix			A(7, 14), CC(3, 6), Der(6, 6);
	CVector			C(7), Esd2(7), G(5), V(7, 0);
	vector<REFLEX>::iterator ri;
	BOOL bFlag = TRUE, bTerm;

	try
	{
		//
		//	Start of refinement
		//	
		n = 5*(nSymmetry - 1);
		for(i = 0; i < 5; i++)
		{
			IS[i] = IIS[n+i];
			IG[i] = IIG[n+i];
			G[i]  = GII[n+i];
		}
		for(i = 0; i < 7; i++)
		{
//			V[i] = 0;
			for(j = 0; j < 14; j++)
			{
				A[i][j] = 0.0;
			}
		}
		//
		//	Build LSQ matrix A
		//
		for(ri = vReflexes.begin(); ri != vReflexes.end(); ++ri)
		{
//			if(!ri->HKList.empty())		// if HKL indices exists
//			{
				Q = 1.0 / (SQR(ri->d));

//				hkli = tpi->HKList.begin();
				H = ri->hkl[_H];
				K = ri->hkl[_K];
				L = ri->hkl[_L];
				C[0] = SQR(H);
				C[1] = SQR(K);
				C[2] = SQR(L);
				C[3] = 2*H*K;
				C[4] = 2*H*L;
				C[5] = 2*K*L;
				C[6] = 0;
				for(i = 0; i < 7; i++)
				{
					V[i] += C[i]*Q;
					for(j = 0; j < 7; j++)
					{
						A[i][j] += C[i]*C[j];
					}
				}
//			}
		}
		for(i = 0; i < 7; i++)
		{
			A[i][i+7] = 1.0;
		}
		V[6]    = 0.0;
		A[6][6] = 1.0;
		//
		//			A + symmentry constraints
		//
		Nvar = 7;
		for(i = 0; i < 5; i++)
		{
			k = IS[i] - 1;
			if(k == -1)
				continue;
			j = IG[i] - 1;
			if(j != -1)
			{
				for(l = 0; l < 7; l++)	// can't pairs with next cycle
				{
					A[l][j] += G[i]*A[l][k];
				}
				for(l = 0; l < 7; l++)
				{
					A[j][l] += G[i]*A[k][l];
				}
				V[j] += G[i]*V[k];
			}
			for(l = 0; l < 7; l++)
			{
				A[l][k] = A[k][l] = 0;
			}
			V[k] = A[k][k+7] = 0;
			Nvar--;
		}
		//
		//			Invert matrix A
		//
		for(n = 1; n <= 7; n++)
		{
			bTerm = FALSE;
			for(k = 1; k <= 5; k++)
			{
				if(IS[k-1] == n)
				{
					bTerm = TRUE;
					break;
				}
			}
			if(bTerm)
				continue;
			if(A[n-1][n-1] == 0)
			{
				::OutputDebugString("** Singular least-squares matrix **\r\n** Insufficient or linear dependent HKL **\r\n**     Please check your input data     **");
				return FALSE;
			}
			BB = 1.0 / A[n-1][n-1];
			for(j = 1; j <= 14; j++)
			{
				A[n-1][j-1] *= BB;
			}
			for(i = 1; i <= 6; i++)
			{
				m = n + i;
				if(m > 7)
					m -= 7;
				bTerm = FALSE;
				for(k = 1; k <= 5; k++)
				{
					if(IS[k-1] == m)
					{
						bTerm = TRUE;
						break;
					}
				}
				if(bTerm)
					continue;
				BB = A[m-1][n-1];
				for(j = 1; j <= 14; j++)
				{
					A[m-1][j-1] -= A[n-1][j-1]*BB;
				}
			}
		}
		//
		//			Get new reciprocal metric tensor C
		//
		for(i = 0; i < 7; i++)
		{
			C[i] = 0;
			for(j = 0; j < 7; j++)
			{
				C[i] += A[i][j+7]*V[j];
			}
		}
		//
		//			Expand inverse matrix
		//
		for(i = 0; i < 5; i++)
		{
			k = IS[i] - 1;
			j = IG[i] - 1;
			if(k != -1)
			{
				if(j != -1)
				{
					C[k] = G[i]*C[j];
					for(l = 0; l < 7; l++)
					{
						A[k][l+7] = G[i]*A[j][l+7];
						A[l][k+7] = G[i]*A[l][j+7];
					}
					A[k][k+7] = SQR(G[i])*A[j][j+7];
				}
			}
		}
		//
		//	Calculate cell from indexed peaks
		//
		// reciprocal parameters
		for(i = 0; i < 3; i++)
		{
			ra[i] = FastSqrt(C[i]);
		}
		// angles
		rCosA[_AL] = C[5]/(ra[_B]*ra[_C]);
		rCosA[_BT] = C[4]/(ra[_A]*ra[_C]);
		rCosA[_GM] = C[3]/(ra[_A]*ra[_B]);
		for(j = 3; j < 6; j++)
		{
			if(FastAbs(rCosA[j]) < 1E-6)
				rCosA[j] = 0;
			rSinA[j-3] = FastSqrt(1 - SQR(rCosA[j]));
		}
		ra[ALPHA]  = acos(rCosA[_AL]);
		ra[BETA]   = acos(rCosA[_BT]);
		ra[GAMMA]  = acos(rCosA[_GM]);
		CalculateVolume(RECIPROCAL);
		CalculateParameters(REAL);
/*
		double sV = FastSqrt(1 - SQR(rCosA[_AL]) - SQR(rCosA[_BT]) -
						   SQR(rCosA[_GM]) +
						   2*rCosA[_AL]*rCosA[_BT]*rCosA[_GM]);
		V  = 1/(a[_A]*a[_B]*a[_C]*sV);
		for(j = 0; j < 3; j++)
			a[j] = Cell.SinA[j]/(Cell.rcpC[j]*Cell.sV);
		Cell._a[3] = (Cell.rcpC[4]*Cell.rcpC[5]-Cell.rcpC[3])/(Cell.SinA[1]*Cell.SinA[2]);
		Cell._a[4] = (Cell.rcpC[3]*Cell.rcpC[5]-Cell.rcpC[4])/(Cell.SinA[0]*Cell.SinA[2]);
		Cell._a[5] = (Cell.rcpC[3]*Cell.rcpC[4]-Cell.rcpC[5])/(Cell.SinA[0]*Cell.SinA[1]);
		for(j = 3; j < 6; j++)
			Cell._a[j] = RAD2DEG(acos(Cell._a[j]));
*/
		for(i = 0; i < 3; i++)
			CC[i][i]= C[i];
		CC[0][1]= C[3];
		CC[1][0]= C[3];
		CC[0][2]= C[4];
		CC[2][0]= C[4];
		CC[1][2]= C[5];
		CC[2][1]= C[5];
		//
		//		Sum w*delta(Q)**2
		//
		Nind = 0;
		for(ri = vReflexes.begin(); ri != vReflexes.end(); ++ri)
		{
//			if(!tpi->HKList.empty())		// if HKL indices exists
//			{
//				for(hkli = tpi->HKList.begin(); hkli != tpi->HKList.end(); ++hkli)
//				{
					H = ri->hkl[_H];
					K = ri->hkl[_K];
					L = ri->hkl[_L];
					Qc = C[0]*SQR(H) + C[1]*SQR(K) + C[2]*SQR(L) + 2.0*(C[3]*H*K + C[4]*H*L + C[5]*K*L);
					R += SQR(1.0 / SQR(ri->d) - Qc);
					Nind++;
//				}
//			}
		}
		Nvar--;
		if(Nind > Nvar)
			R /= (Nind - Nvar);
		//
		//		Multiply inverse LSQ matrix by sum(w*delta**2)/(no-nv)
		//
		for(i = 0; i < 7; i++)
		{
			for(j = 0; j < 7; j++)
			{
				A[i][j] = R*A[i][j+7];
			}
		}
		//
		//		Calculate standard deviations,
		//		variance-covariance matrix and correlation matrix
		//		for the reciprocal cell, then for the direct cell
		//
		Metric(A, C, Der, Esd2);
		for(n = 1; n <= 3; n++)
		{
			for(m = 4; m <= 6; m++)
			{
				CC[n-1][m-1] = 0;
			}
			CC[n-1][n+2] = 1;
		}
		for(n = 1; n <= 3; n++)
		{
			BB = 1/CC[n-1][n-1];
			for(j = 1; j <= 6; j++)
			{
				CC[n-1][j-1] = CC[n-1][j-1]*BB;
			}
			for(i = 1; i <= 2; i++)
			{
				m = n+i;
				if(m > 3) m -= 3;
				BB = CC[m-1][n-1];
				for(j = 1; j <= 6; j++)
				{
					CC[m-1][j-1] -= CC[n-1][j-1]*BB;
				}
			}
		}

		for(i = 1; i <= 3; i++)
		{
			C[i-1] = CC[i-1][i+2];
		}
		C[3] = CC[0][4];
		C[4] = CC[0][5];
		C[5] = CC[1][5];
		for (i = 1; i <= 3; i++)
		{
			for (k = 1; k <= 3; k++)
			{
				Der[i-1][k-1] = -SQR(CC[i-1][k+2]);
			}
			for (j = 1; j <= 2; j++)
			{
				m = j + 1;
				for (n = m; n <= 3; n++)
				{
					k = j+n+1;
					Der[i-1][k-1] = -CC[i-1][j+2]*CC[i-1][n+2];
					Der[k-1][i-1] = 2*Der[i-1][k-1];
				}
			}
		}
		Der[3][3] = -SQR(C[3]) - C[0]*C[1];
		Der[4][4] = -SQR(C[4]) - C[0]*C[2];
		Der[5][5] = -SQR(C[5]) - C[1]*C[2];
		Der[3][4] = -C[3]*C[4] - C[0]*C[5];
		Der[3][5] = -C[3]*C[5] - C[1]*C[4];
		Der[4][5] = -C[4]*C[5] - C[2]*C[3];
		for (i = 4; i <= 6; i++)
		{
			for (k = i; k <= 6; k++)
			{
				Der[k-1][i-1] =  Der[i-1][k-1];
			}
		}
		for (i = 1; i <= 6; i++)
		{
			for (k = i; k <= 6; k++)
			{
				j = k + 6;
				A[i-1][j-1]= 0;
				for (m = 1; m <= 6; m++)
					for (n = 1; n <= 6; n++)
						A[i-1][j-1] += Der[m-1][i-1]*Der[n-1][k-1]*A[m-1][n-1];
			}
		}
		for (i = 1; i <= 6; i++)
		{
			for (k = i; k <= 6; k++)
			{
				A[i-1][k-1]= A[i-1][k+5];
				A[k-1][i-1]= A[i-1][k-1];
			}
		}
		Metric(A, C, Der, Esd2);
		Esds = Esd2;
/*
		for(i = 0; i < 7; i++)
		{
			Esds[i] = Esd2[i];
		}
*/
	}
	catch(char *pMsg)
	{
		printf("CUnitCell3D::RefineParameters() -> %s.\n", pMsg);
		bFlag = FALSE;
	}
	catch(...)
	{
		printf("CUnitCell3D::RefineParameters() -> unknown error.\n");
		bFlag = FALSE;
	}
	return bFlag;
}

void Metric(CMatrix &A, CVector &C, CMatrix &Der, CVector &Esd2)
{
	try
	{
		int		i, j, k, m, n;
		CVector V(6), Esd1(6), D(7);
		CMatrix B(6, 12);
		double	BB, Esd, angl;

		for(i = 0; i < 6; i++)
		{
			for (j = 6; j < 12; j++)
				A[i][j] = B[i][j] = 0;
			A[i][i + 6] = B[i][i + 6] = 1;
		}
		for (i = 0; i < 6; i++)
			Esd1[i] = FastSqrt(A[i][i]);
		for (i = 0; i < 6; i++)
		{
			for (j = 0; j < 6; j++)
			{
				Esd = Esd1[i]*Esd1[j];
				if (Esd > 0)
					A[i][j + 6] = A[i][j] / Esd;
			}
		}
		BB = C[0]*C[1]*C[2] + 2*C[3]*C[4]*C[5] - C[0]*SQR(C[5]) - 
			 C[1]*SQR(C[4]) - C[2]*SQR(C[3]);
		D[6] = FastSqrt(BB);
		V[0] = C[1]*C[2] - SQR(C[5]);
		V[1] = C[0]*C[2] - SQR(C[4]);
		V[2] = C[0]*C[1] - SQR(C[3]);
		V[3] = (C[4]*C[5] - C[2]*C[3])*2;
		V[4] = (C[3]*C[5] - C[1]*C[4])*2;
		V[5] = (C[3]*C[4] - C[0]*C[5])*2;
		BB = 0;
		for (i = 0; i < 6; i++)
			BB += A[i][i]*SQR(V[i]);
		for (i = 0; i < 5; i++)
			for (j = i + 1; j < 6; j++)
				BB += 2*A[i][j]*V[i]*V[j];
		Esd2[6] = FastSqrt(BB)*0.5 / D[6];
		for (i = 0; i < 6; i++)
			for (j = 0; j < 6; j++)
				Der[i][j] = 0;
		for (i = 0; i < 3; i++)
		{
			D[i] = FastSqrt(C[i]);
			Der[i][i] = 0.5 / D[i];
		}
		for (i = 1; i <= 2; i++)
		{
			n = i + 1;
			for (j = n; j <= 3; j++)
			{
				m = i + j + 1;
				Der[m-1][m-1] = 1.0 / (D[i-1]*D[j-1]);
				D[m-1] = C[m-1] * Der[m-1][m-1];
				Der[i-1][m-1] = -0.5*D[m-1] / C[i-1];
				Der[j-1][m-1] = -0.5*D[m-1] / C[j-1];
/*
				D[m-1] = C[m-1] / (D[i-1]*D[j-1]);
				Der[i-1][m-1] = -0.5*D[m-1] / C[i-1];
				Der[j-1][m-1] = -0.5*D[m-1] / C[j-1];
				Der[m-1][m-1] = 1 / (D[i-1]*D[j-1]);
*/
			}
		}
		for (i = 0; i < 6; i++)
		{
			for (k = i; k < 6; k++)
			{
				B[i][k] = 0;
				for (m = 0; m < 6; m++)
				{
					for (n = 0; n < 6; n++)
						B[i][k] += Der[m][i]*Der[n][k]*A[m][n];
				}
				B[k][i] = B[i][k];
			}
		}
		for (i = 0; i < 6; i++)
		{
			for (j = 3; j < 6; j++)
			{
				if(FastAbs(B[i][j]) < 1e-14)
					B[i][j] = B[j][i] = 0;
			}
		}
		for (i = 0; i < 6; i++)
		{
			Esd2[i] = 0;
			if (B[i][i] > 0)
				Esd2[i] = FastSqrt(B[i][i]);
		}
		for (i = 0; i < 6; i++)
		{
			for (j = 0; j < 6; j++)
			{
				Esd = Esd2[i]*Esd2[j];
				if (Esd > 0)
					B[i][j + 6] = B[i][j] / Esd;
			}
		}
		for (i = 3; i < 6; i++)
		{
			angl = acos(D[i]);
			D[i] = RAD2DEG(angl);
			Esd2[i] = RAD2DEG(Esd2[i]) / sin(angl);
		}
		BB = D[5];
		D[5] = D[3];
		D[3] = BB;
		BB = Esd2[5];
		Esd2[5] = Esd2[3];
		Esd2[3] = BB;
		for (i = 0; i < 12; i++)
		{
			BB = B[5][i];
			B[5][i] = B[3][i];
			B[3][i] = BB;
		}
		for (i = 0; i < 6; i++)
		{
			BB = B[i][5];
			B[i][5] = B[i][3];
			B[i][3] = BB;
			BB = B[i][11];
			B[i][11] = B[i][9];
			B[i][9] = BB;
		}
	}
	catch(char *pMsg)
	{
		printf("Metric() -> %s.\n", pMsg);
		throw pMsg;
	}
	catch(...)
	{
		printf("Metric() -> unknown error.\n");
		throw;
	}
}

}; // namespace UnitCell
