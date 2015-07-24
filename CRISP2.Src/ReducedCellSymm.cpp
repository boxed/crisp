/****************************************************************************/
/* Copyright (c) 2006 Calidris **********************************************/
/* Written by Peter Oleynikov  **********************************************/
/****************************************************************************/
/* CRISP2.ELD ***************************************************************/
/****************************************************************************/

#include "StdAfx.h"

// #include "MathKrnl/MathKrnl.h"
// #include "MathKrnl/MarqMin.h"

#ifdef _DEBUG
#pragma optimize ("",off)
#endif

/*
typedef enum
{
	AA_T = 0,
	BB_T,
	CC_T,

} CONV_MUL_TYPE;


double GetMultipliers(CONV_MUL_TYPE nType, double *c)
{
	static const int A = 0, B = 1, C = 2;
	double val = 0.0;
	switch( nType )
	{
	case AA_T: val = c[A]*
		break;
	default:
	}
}
*/

// returns +1 if the cell is positive reduced
// returns -1 if the cell is negative reduced
int ReducedFormType(double *padCell)
{
	return +1;
}

class _Mult
{
public:
	_Mult(const double &mul = 0.0, const double &err = 0.0) { _mul = mul; _err = err; }
	double operator = (double val)
	{
		_err = FastAbs(0.02 * (_mul = val));
		return _mul;
	}
	bool operator == (double val) const
	{
		return (FastAbs(_mul - val) <= _err) ? true : false;
	}
	bool operator == (const _Mult &mul) const
	{
		return mul == _mul;
	}
	double operator()() { return _mul; }
	double _mul;
	double _err;
};

/*
inline bool IsEqual(double d1, double d2, double err)
{
	return (FastAbs(d1 - d2) <= err) ? true : false;
}
*/

BOOL ConventionalCell(double *padCell, CMatrix3x3 &conv, char *pszCellType, char *pszCentring)
{
	BOOL bRes = FALSE;
	conv.LoadIdentity();
	double _a, _b, _c, _a2, _b2, _c2, _bc, _ac, _ab, _al, _bt, _gm;
	char tmp[256];
	// I-centered matrix
	static const double mtx_1[9] = { 0, 0, -1,   0, 1, 0,   1, 0, 1};
	// C-centered matrix
	static const double mtx_2[9] = {-1, 0, -1,   0, 1, 0,   1, 0, 0};
	typedef enum
	{
		BR_CUB,
		BR_RHOM,
		BR_TETR,
		BR_HEXA,
		BR_ORTH,
		BR_MONO,
		BR_TRIC,

	} BRAVAIS_LATT;
	
	typedef enum
	{
		P_PRIM,
		C_CENTR,
		I_CENTR,
		F_CENTR,
		R_CENTR,

	} CENTRING;
	typedef struct
	{
		int nRedFormN;			// reduced form number
		double _aa;
		double _bb;
		double _cc;
		double _bc;
		double _ac;
		double _ab;
		BRAVAIS_LATT nLatt;
		CENTRING nCentr;
		int nRedType;			// +1 if positevly reduced, -1 if negatively reduced
		int mtx[9];
		double comp1;			// in case of special matrix we must check that comp1 < comp2
		double comp2;
		const double *mtx_extra;	// some special cases need to premultiply
		
	} RED2CONV_t;
	
	static const char *asBravais[] =
	{
		"Cubic",
		"Rhombohedral",
		"Tetragonal",
		"Hexagonal",
		"Orthorhombic",
		"Monoclinic",
		"Triclinic",
	};
	static const char *asCentr[] = { "P",	"C",	"I",	"F",	"hR", };

	_a2 = SQR(_a = padCell[0]);
	_b2 = SQR(_b = padCell[1]);
	_c2 = SQR(_c = padCell[2]);
	_al = padCell[3];
	_bt = padCell[4];
	_gm = padCell[5];

	_bc = _b*_c*FastCos(_al);
	_ac = _a*_c*FastCos(_bt);
	_ab = _a*_b*FastCos(_gm);

	const double dDelta = 0.1;
	double dErr = 0.02;
	::OutputDebugString("   ------------\n");
	::OutputDebugString("Finding the conventional cell from the reduced cell:\n");

	const int N_CYCLES = 5;
	int pos = 0;
	int iStep, iSol, iBest, aiBest[N_CYCLES], aiCentr[N_CYCLES], aiLatt[N_CYCLES];
	CMatrix3x3 amConv[N_CYCLES];

	for(iStep = 0; iStep < N_CYCLES; iStep++)
	{
		aiBest[iStep] = -1;
		amConv[iStep].LoadIdentity();
		aiLatt[iStep] = -1;
		aiCentr[iStep] = -1;
	}

	pszCentring[0] = pszCellType[0] = 0;
	for(iStep = 0; iStep < N_CYCLES; iStep++)
	{
		_Mult s_AA(_a2, dDelta*_a2), s_BB(_b2, dDelta*_b2), s_CC(_c2, dDelta*_c2);
		_Mult _BC(_bc, dErr*_b*_c), _AC(_ac, dErr*_a*_c), _AB(_ab, dErr*_a*_b);
		RED2CONV_t cases[42] =
		{
		//  No.     _aa      _bb      _cc     _bc    _ac    _ab      Brav.       Centr.        
			{1 ,	_a2,	_a2,	_a2,	0.5*_a2,	0.5*_a2,	0.5*_a2,	BR_CUB,	F_CENTR,	+1,	{1,	-1,	1,	1,	1,	-1,	-1,	1,	1},	0,	0,	NULL},	//	1
			{2 ,	_a2,	_a2,	_a2,	_bc,	_bc,	_bc,	BR_RHOM,	R_CENTR,	+1,	{1,	-1,	0,	-1,	0,	1,	-1,	-1,	-1},	0,	0,	NULL},	//	2
			{3 ,	_a2,	_a2,	_a2,	0,	0,	0,	BR_CUB,	P_PRIM,	-1,	{1,	0,	0,	0,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	3
			{4 ,	_a2,	_a2,	_a2,	-FastAbs(_bc),	-FastAbs(_bc),	-FastAbs(_bc),	BR_RHOM,	R_CENTR,	-1,	{1,	-1,	0,	-1,	0,	1,	-1,	-1,	-1},	0,	0,	NULL},	//	4
			{5 ,	_a2,	_a2,	_a2,	-_a2/3.0,	-_a2/3.0,	-_a2/3.0,	BR_CUB,	I_CENTR,	-1,	{1,	0,	1,	1,	1,	0,	0,	1,	1},	0,	0,	NULL},	//	5
			{6 ,	_a2,	_a2,	_a2,	0.5*(-_a2+FastAbs(_ab)),	0.5*(-_a2+FastAbs(_ab)),	-FastAbs(_ab),	BR_TETR,	I_CENTR,	-1,	{0,	1,	1,	1,	0,	1,	1,	1,	0},	0,	0,	NULL},	//	6
			{7 ,	_a2,	_a2,	_a2,	-FastAbs(_bc),	0.5*(-_a2+FastAbs(_bc)),	0.5*(-_a2+FastAbs(_bc)),	BR_TETR,	I_CENTR,	-1,	{1,	0,	1,	1,	1,	0,	0,	1,	1},	0,	0,	NULL},	//	7
			{8 ,	_a2,	_a2,	_a2,	-FastAbs(_bc),	-FastAbs(_ac),	-(FastAbs(_a2)-FastAbs(_bc)-FastAbs(_ac)),	BR_ORTH,	I_CENTR,	-1,	{-1,	-1,	0,	-1,	0,	-1,	0,	-1,	-1},	0,	0,	NULL},	//	8
			{9 ,	_a2,	_a2,	_c2,	0.5*_a2,	0.5*_a2,	0.5*_a2,	BR_RHOM,	R_CENTR,	+1,	{1,	0,	0,	-1,	1,	0,	-1,	-1,	3},	0,	0,	NULL},	//	9
			{10,	_a2,	_a2,	_c2,	_bc,	_bc,	_ab,	BR_MONO,	C_CENTR,	+1,	{1,	1,	0,	1,	-1,	0,	0,	0,	-1},	_c2,	4*FastAbs(_bc),	mtx_1},	//	10
			{11,	_a2,	_a2,	_c2,	0,	0,	0,	BR_TETR,	P_PRIM,	-1,	{1,	0,	0,	0,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	11
			{12,	_a2,	_a2,	_c2,	0,	0,	-0.5*_a2,	BR_HEXA,	P_PRIM,	-1,	{1,	0,	0,	0,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	12
			{13,	_a2,	_a2,	_c2,	0,	0,	-FastAbs(_ab),	BR_ORTH,	C_CENTR,	-1,	{1,	1,	0,	-1,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	13
			{14,	_a2,	_a2,	_c2,	-FastAbs(_bc),	-FastAbs(_bc),	-FastAbs(_ab),	BR_MONO,	C_CENTR,	-1,	{1,	1,	0,	-1,	1,	0,	0,	0,	1},	_c2,	4*FastAbs(_bc),	mtx_1},	//	14
			{15,	_a2,	_a2,	_c2,	-0.5*_a2,	-0.5*_a2,	0,	BR_TETR,	I_CENTR,	-1,	{1,	0,	0,	0,	1,	0,	1,	1,	2},	0,	0,	NULL},	//	15
			{16,	_a2,	_a2,	_c2,	-FastAbs(_bc),	-FastAbs(_bc),	-(_a2-2*FastAbs(_bc)),	BR_ORTH,	F_CENTR,	-1,	{-1,	-1,	0,	1,	-1,	0,	1,	1,	2},	0,	0,	NULL},	//	16
			{17,	_a2,	_a2,	_c2,	-FastAbs(_bc),	-FastAbs(_ac),	-(_a2-FastAbs(_bc)-FastAbs(_ac)),	BR_MONO,	I_CENTR,	-1,	{-1,	0,	-1,	-1,	-1,	0,	0,	1,	1},	3*_a2,	_c2+2*FastAbs(_ac),	mtx_2},	//	17
			{18,	_a2,	_b2,	_b2,	0.25*_a2,	0.5*_a2,	0.5*_a2,	BR_TETR,	I_CENTR,	+1,	{0,	-1,	1,	1,	-1,	-1,	1,	0,	0},	0,	0,	NULL},	//	18
			{19,	_a2,	_b2,	_b2,	_bc,	0.5*_a2,	0.5*_a2,	BR_ORTH,	I_CENTR,	+1,	{-1,	0,	0,	0,	-1,	1,	-1,	1,	1},	0,	0,	NULL},	//	19
			{20,	_a2,	_b2,	_b2,	_bc,	_ac,	_ac,	BR_MONO,	C_CENTR,	+1,	{0,	1,	1,	0,	1,	-1,	-1,	0,	0},	_a2,	4*FastAbs(_ac),	mtx_1},	//	20
			{21,	_a2,	_b2,	_b2,	0,	0,	0,	BR_TETR,	P_PRIM,	-1,	{0,	1,	0,	0,	0,	1,	1,	0,	0},	0,	0,	NULL},	//	21
			{22,	_a2,	_b2,	_b2,	-0.5*_b2,	0,	0,	BR_HEXA,	P_PRIM,	-1,	{0,	1,	0,	0,	0,	1,	1,	0,	0},	0,	0,	NULL},	//	22
			{23,	_a2,	_b2,	_b2,	-FastAbs(_bc),	0,	0,	BR_ORTH,	C_CENTR,	-1,	{0,	1,	1,	0,	-1,	1,	1,	0,	0},	0,	0,	NULL},	//	23
			{24,	_a2,	_b2,	_b2,	-0.5*(_b2-_a2/3.0),	-_a2/3.0,	-_a2/3.0,	BR_RHOM,	R_CENTR,	-1,	{1,	2,	1,	0,	-1,	1,	1,	0,	0},	0,	0,	NULL},	//	24
			{25,	_a2,	_b2,	_b2,	-FastAbs(_bc),	-FastAbs(_ac),	-FastAbs(_ac),	BR_MONO,	C_CENTR,	-1,	{0,	1,	1,	0,	-1,	1,	1,	0,	0},	_a2,	4*FastAbs(_ac),	mtx_1},	//	25
			{26,	_a2,	_b2,	_c2,	0.25*_a2,	0.5*_a2,	0.5*_a2,	BR_ORTH,	F_CENTR,	+1,	{1,	0,	0,	-1,	2,	0,	-1,	0,	2},	0,	0,	NULL},	//	26
			{27,	_a2,	_b2,	_c2,	_bc,	0.5*_a2,	0.5*_a2,	BR_MONO,	I_CENTR,	+1,	{0,	-1,	1,	-1,	0,	0,	1,	-1,	-1},	3*_b2,	_c2+2*FastAbs(_bc),	mtx_2},	//	27
			{28,	_a2,	_b2,	_c2,	0.5*_ab,	0.5*_a2,	_ab,	BR_MONO,	C_CENTR,	+1,	{-1,	0,	0,	-1,	0,	2,	0,	1,	0},	0,	0,	NULL},	//	28
			{29,	_a2,	_b2,	_c2,	0.5*_ac,	_ac,	0.5*_a2,	BR_MONO,	C_CENTR,	+1,	{1,	0,	0,	1,	-2,	0,	0,	0,	-1},	0,	0,	NULL},	//	29
			{30,	_a2,	_b2,	_c2,	0.5*_b2,	0.5*_ab,	_ab,	BR_MONO,	C_CENTR,	+1,	{0,	1,	0,	0,	1,	-2,	-1,	0,	0},	0,	0,	NULL},	//	30
//			{31,	_a2,	_b2,	_c2,	_bc,	_ac,	_ab,	BR_TRIC,	P_PRIM,	+1,	{1,	0,	0,	0,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	31
			{32,	_a2,	_b2,	_c2,	0,	0,	0,	BR_ORTH,	P_PRIM,	-1,	{1,	0,	0,	0,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	32
			{33,	_a2,	_b2,	_c2,	0,	-FastAbs(_ac),	0,	BR_MONO,	P_PRIM,	-1,	{1,	0,	0,	0,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	33
			{34,	_a2,	_b2,	_c2,	0,	0,	-FastAbs(_ab),	BR_MONO,	P_PRIM,	-1,	{-1,	0,	0,	0,	0,	-1,	0,	-1,	0},	0,	0,	NULL},	//	34
			{35,	_a2,	_b2,	_c2,	-FastAbs(_bc),	0,	0,	BR_MONO,	P_PRIM,	-1,	{0,	-1,	0,	-1,	0,	0,	0,	0,	-1},	0,	0,	NULL},	//	35
			{36,	_a2,	_b2,	_c2,	0,	-0.5*_a2,	0,	BR_ORTH,	C_CENTR,	-1,	{1,	0,	0,	-1,	0,	-2,	0,	1,	0},	0,	0,	NULL},	//	36
			{37,	_a2,	_b2,	_c2,	-FastAbs(_bc),	-0.5*_a2,	0,	BR_MONO,	C_CENTR,	-1,	{1,	0,	2,	1,	0,	0,	0,	1,	0},	_b2,	4*FastAbs(_bc),	mtx_1},	//	37
			{38,	_a2,	_b2,	_c2,	0,	0,	-0.5*_a2,	BR_ORTH,	C_CENTR,	-1,	{-1,	0,	0,	1,	2,	0,	0,	0,	-1},	0,	0,	NULL},	//	38
			{39,	_a2,	_b2,	_c2,	-FastAbs(_bc),	0,	-0.5*_a2,	BR_MONO,	C_CENTR,	-1,	{-1,	-2,	0,	-1,	0,	0,	0,	0,	-1},	_c2,	4*FastAbs(_bc),	mtx_1},	//	39
			{40,	_a2,	_b2,	_c2,	-0.5*_b2,	0,	0,	BR_ORTH,	C_CENTR,	-1,	{0,	-1,	0,	0,	1,	2,	-1,	0,	0},	0,	0,	NULL},	//	40
			{41,	_a2,	_b2,	_c2,	-0.5*_b2,	-FastAbs(_ac),	0,	BR_MONO,	C_CENTR,	-1,	{0,	-1,	-2,	0,	-1,	0,	-1,	0,	0},	_a2,	4*FastAbs(_ac),	mtx_1},	//	41
			{42,	_a2,	_b2,	_c2,	-0.5*_b2,	-0.5*_a2,	0,	BR_ORTH,	I_CENTR,	-1,	{-1,	0,	0,	0,	-1,	0,	1,	1,	2},	0,	0,	NULL},	//	42
			{43,	_a2,	_b2,	_c2,	-0.5*(_b2-FastAbs(_ab)),	-0.5*(_a2-FastAbs(_ab)),	-FastAbs(_ab),	BR_MONO,	I_CENTR,	-1,	{-1,	0,	0,	-1,	-1,	-2,	0,	-1,	0},	0,	0,	NULL},	//	43
//			{44,	_a2,	_b2,	_c2,	-FastAbs(_bc),	-FastAbs(_ac),	-FastAbs(_ab),	BR_TRIC,	P_PRIM,	-1,	{1,	0,	0,	0,	1,	0,	0,	0,	1},	0,	0,	NULL},	//	44
		};
		iBest = -1;
		RED2CONV_t *pCase = cases;
		for(iSol = 0; iSol < sizeof(cases) / sizeof(RED2CONV_t); iSol++, pCase++)
		{
			if( (s_AA == pCase->_aa) &&
				(s_BB == pCase->_bb) &&
				(s_CC == pCase->_cc) &&
				(_BC == pCase->_bc) &&
				(_AC == pCase->_ac) &&
				(_AB == pCase->_ab) )
			{
				if( (-1 != iBest) && (cases[iBest].nLatt < pCase->nLatt) )
				{
					continue;
				}
				iBest = iSol;
				sprintf(tmp, "\t\tcase %2d, %s %s\n", pCase->nRedFormN,
					asBravais[pCase->nLatt], asCentr[pCase->nCentr]);
				::OutputDebugString(tmp);
				aiCentr[iStep] = pCase->nCentr;
				aiLatt[iStep] = pCase->nLatt;
//				strncpy(pszCentring, asCentr[pCase->nCentr], 31);
//				strncpy(pszCellType, asBravais[pCase->nLatt], 31);
				pos = 0;
				// copy the matrix
				for(int j = 0; j < 3; j++)
				{
					for(int k = 0; k < 3; k++, pos++)
					{
						amConv[iStep].d[j][k] = pCase->mtx[pos];
					}
				}
				// do we have a premultiplication matrix?
				if( NULL != pCase->mtx_extra )
				{
					// check that condition-1 < condition-2
					if( FastAbs(pCase->comp1 - pCase->comp2) <
						0.5*dErr*(FastAbs(pCase->comp1) + FastAbs(pCase->comp2)) )
					{
						CMatrix3x3 premul(pCase->mtx_extra);
						amConv[iStep] = premul * amConv[iStep];
					}
				}
//				break;
			}
		}
//		if( dErr  )
//		{
//		}
		// if we are here check if we have a good solution
//		if( (iBest >= 41) && (dErr <= 0.1) )
		{
			// make it slightly higher
			dErr += 0.02;
//			continue;
		}
		aiBest[iStep] = iBest;
		// break otherwise
//		break;
	}
	iSol = 0;
	iBest = aiBest[iSol];
	for(iStep = 0; iStep < N_CYCLES; iStep++)
	{
		if( (aiBest[iStep] > 0) && (aiLatt[iSol] > aiLatt[iStep]) )
		{
			iBest = aiBest[iStep];
			iSol = iStep;
		}
	}
	conv.LoadIdentity();
	if( iBest >= 0 )
	{
		strncpy(pszCellType, asBravais[aiLatt[iSol]], 31);
		strncpy(pszCentring, asCentr[aiCentr[iSol]], 31);
		// copy the matrix
		conv = amConv[iSol];
	}
	::OutputDebugString("   ------------\n");
	return bRes;
}
