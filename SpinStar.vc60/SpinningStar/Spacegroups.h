#pragma once

#include <list>
#include <vector>

#include "../typedefs.h"
#include "LinAlg.h"

#define		STBF				12

// Plane groups
#define		SYMM_P1				1
#define		SYMM_P2				2
#define		SYMM_PMa			3
#define		SYMM_PMb			4
#define		SYMM_PGa			5
#define		SYMM_PGb			6
#define		SYMM_CMa			7
#define		SYMM_CMb			8
#define		SYMM_PMM			9
#define		SYMM_PMGa			10
#define		SYMM_PMGb			11
#define		SYMM_PGG			12
#define		SYMM_CMM			13
#define		SYMM_P4				14
#define		SYMM_P4M			15
#define		SYMM_P4G			16
#define		SYMM_P3				17
#define		SYMM_P3M1			18
#define		SYMM_P31M			19
#define		SYMM_P6				20
#define		SYMM_P6M			21

// Plane Point groups
#define		SYMM_PG_1			1
#define		SYMM_PG_2			2
#define		SYMM_PG_M			3
#define		SYMM_PG_2MM			4
#define		SYMM_PG_4			5
#define		SYMM_PG_4MM			6
#define		SYMM_PG_3			7
#define		SYMM_PG_3M			8
#define		SYMM_PG_6			9
#define		SYMM_PG_6MM			10

// Plane Point groups (extended)
#define		SYMM_PG_EX_1			1
#define		SYMM_PG_EX_2			2
#define		SYMM_PG_EX_M_X			3
#define		SYMM_PG_EX_M_Y			4
#define		SYMM_PG_EX_2MM			5
#define		SYMM_PG_EX_4			6
#define		SYMM_PG_EX_4MM			7
#define		SYMM_PG_EX_3			8
#define		SYMM_PG_EX_3M1			9
#define		SYMM_PG_EX_31M			10
#define		SYMM_PG_EX_6			11
#define		SYMM_PG_EX_6MM			12

enum
{
	BL_Primitive = 0,
	BL_Face_centered = 1,
	BL_Body_centered = 2,
	BL_Hexagonal_obverse = 3,
	BL_All_face_centered = 4,
};

enum
{
	EI_Unknown = 0,
	EI_Enantiomorphic = 1,
	EI_Obverse = 2,
	EI_Reverse = 3,
};

typedef enum
{
	SG_PlaneGroup = 1,
	SG_SpaceGroup = 2,

} SG_TYPE;

// NOTE: Trigonal with hexagonal axis has cCode == 'R', psExtension == "H"
// NOTE: Trigonal with rhombohedral axis has cCode == 'P', psExtension == "R"
enum
{
	XS_Unknown     = 0,
	XS_Triclinic   = 1,
	XS_Monoclinic  = 2,
	XS_Orthorhombic= 3,
	XS_Tetragonal  = 4,
	XS_Trigonal    = 5,
	XS_Hexagonal   = 6,
	XS_Cubic       = 7,
};

// Plane lattices
enum
{
	PXS_Unknown     = 0,
	PXS_Oblique     = 1,
	PXS_Rectangular = 2,
	PXS_Square      = 3,
	PXS_Hexagonal   = 4,
};

extern LPCTSTR XS_Name[];			// Space groups
extern LPCTSTR PXS_Name[];			// Plane groups
extern LPCTSTR EI_Name[];
extern LPCTSTR Centr_Name[];
extern LPCTSTR Plane_PG_Names[];	// Plane Point groups
extern LPCTSTR Plane_PG_Names_Ext[];// Plane Point groups (including Mx, My, 3M1, 31M)
extern LPCTSTR PG_Names[];
extern LPCTSTR SchoenfliesSymbols[];

/*
Code  nTrVector  TrVector
 'P'      1      0,0,0
 'A'      2      0,0,0   0 ,1/2,1/2
 'B'      2      0,0,0  1/2, 0 ,1/2
 'C'      2      0,0,0  1/2,1/2, 0
 'I'      2      0,0,0  1/2,1/2,1/2
 'R'      3      0,0,0  2/3,1/3,1/3  1/3,2/3,2/3
 'S'      3      0,0,0  1/3,1/3,2/3  2/3,2/3,1/3
 'T'      3      0,0,0  1/3,2/3,1/3  2/3,1/3,2/3
 'F'      4      0,0,0   0 ,1/2,1/2  1/2, 0 ,1/2  1/2,1/2, 0
*/

typedef union
{
    struct { char R[9], T[3]; } s;
    char                        a[12];
} SG_MATRIX;

// dMul1 - multiplier for the ROTAT part (for example, +/-1, depending on Centrosymmetric)
// dMul2 - scale-factor for translational part (for example, 1/STBF)
//void SetMtx(const SG_MATRIX &mtx, CMatrix4x4 &out, double dMul1, double dMul2);

typedef struct
{
	int		nSgNum;				// 1-based sg number
	int		nRefSgNum;			// reference space group number
	int		nCentric;			// 0 = acentric, -1 = inversion operation removed from Matrices
	int		nList;				// # of matrices in Matrices
	SG_MATRIX	*Matrices;		// pointer to matrices
	int		nOrderL;			// holds the order of the space group,
								// i.e. the maximum number of symmetry equivalent positions
	int		nOrderP;			// holds the order of the primitive subgroup of the space group
	int		nXtalSystem;		// Constant, one of XS_Name[]
	int		nUniqueRefAxis;		// holds a code for the unique axis,
								// For triclinic and orthorhombic space groups - 0
	int		nUniqueDirCode;		// holds a code for the unique axis,
								// For triclinic and orthorhombic space groups - 0
	int		nExtraInfo;			// Constant, one of EI_Name[]
	TCHAR	*psPointGroup;		// name of point group
	int		nLaueCode;			// Laue code number
	TCHAR	*psHallSymbol;		// string with Hall symbol
	TCHAR	*psExtension;		// string with extension
	TCHAR	*psSgLabel;			// string with space group label
	TCHAR	*psSgName;			// string with space group name only (from label)
	TCHAR	cCode;				// one of 'P' etc.
	int		nTrVector;			// # of translation vectors
	SG_TYPE	nSgType;			// SG_PlaneGroup or SG_SpaceGroup
//	LPVOID	pExtraInfo;			// can be setup freely (for ex, by Plane Groups)

} SG_ENTRY, *LPSG_ENTRY;

class SpaceGroup
{
public:
	// the 1st constructor cannot use default parameter - ambiguous with 2nd constructor
	SpaceGroup(size_t nSgNum, SG_TYPE nType);// = SG_SpaceGroup);
	SpaceGroup(size_t nSgNum, size_t nSgExt, SG_TYPE nType = SG_SpaceGroup);
	SpaceGroup(LPCTSTR pSgName, SG_TYPE nType = SG_SpaceGroup);
	SpaceGroup(const SG_ENTRY *pSgEntry);
	~SpaceGroup(void);

	struct HKL_index
	{
		HKLindex	h;
		HKLindex	k;
		HKLindex	l;
		short		M;				// multiplicity
		size_t		eps;			// systematically enhanced factor
		double		d;				// d-value if the unit cell is available
		HKLindex	restriction;	// phase restriction, PHA = 180 * restriction / 12.0 (radians)
	};

	typedef struct Eq_hkl_
	{
		int  M;      			// Multiplicity
		int  N;      			// Number of equivalent hkl to follow
		HKLMiller hkl[192];
		HKLindex  TH[192]; 		// Phase shift relative to h[0], k[0], l[0]
	} Eq_hkl;
	
	typedef std::vector<HKL_index> HKLvec;
	typedef std::vector<HKL_index>::iterator HKLvecIt;
	typedef std::list<HKL_index*> HKLlist;
	typedef std::list<HKL_index*>::iterator HKLlistIt;

	void PaintSpaceGroup(CDC *pDC, CFont &Font, const CRect &rc);

	static void GetMaxHKL(double dMinD, double ar, double br, double cr,
		HKLindex &MaxH, HKLindex &MaxK, HKLindex &MaxL);
	void SetListMin_hkl(HKLindex Maxk, HKLindex Maxl, HKLindex &Minh, HKLindex &Mink, HKLindex &Minl);

	// if the unit cell is NOT NULL, the comparison can be performed faster for the list
	bool BuildHklList(HKLvec &List, HKLindex MaxH, HKLindex MaxK, HKLindex MaxL,
		const HKLMiller &zone, double *padCellParams = NULL);
	int BuildEq_hkl(Eq_hkl &Eq_hkl, HKLindex h, HKLindex k, HKLindex l, bool bMakePositive = true) const;
	int BuildEq_hkl(Eq_hkl &Eq_hkl, const HKLMiller &hkl, bool bMakePositive = true) const;

	int IsSuppressed_hkl(HKLindex Minh, HKLindex Mink, HKLindex Minl, HKLindex Maxk, HKLindex Maxl,
						 HKLindex h, HKLindex k, HKLindex l);
	int IsSysAbsent_hkl(HKLindex h, HKLindex k, HKLindex l, HKLindex &TH_Restriction) const;
	int IsSysAbsent_hkl(const HKLMiller &hkl, HKLindex &TH_Restriction) const;
	int AreSymmEquivalent(HKLindex h1, HKLindex k1, HKLindex l1,
		HKLindex h2, HKLindex k2, HKLindex l2, bool bCheckFriedel = true);
	int AreSymmEquivalent(const HKLMiller &hkl1, const HKLMiller &hkl2, bool bCheckFriedel = true);
	int	AreSymmEnhanced(HKLindex h, HKLindex k, HKLindex l);
	int	AreSymmEnhanced(const HKLMiller &hkl);

	bool SetSgNum(size_t nSgNum, SG_TYPE nType = SG_SpaceGroup);
	bool SetSgNumExt(size_t nSgNum, size_t nSgExt, SG_TYPE nType = SG_SpaceGroup);
	bool SetSgNumExt(size_t nSgNum, LPCTSTR pszExtension, SG_TYPE nType = SG_SpaceGroup);
	bool SetSgName(LPCTSTR pSgName, SG_TYPE nType = SG_SpaceGroup);
	// iCentric = -1 - acentric, 1 - centric, 0 - no info
	// cCode = 0 - unknown, 'P' or etc. otherwise
	bool SetSgFromSymmOp(SG_MATRIX *pSymmOps, int nOps, int iCentric, TCHAR cCode,
		bool bExpandInv, bool bExpandCentr, SG_TYPE nType);
	static bool ParseSymmOpText(LPCTSTR pSymmOp, SG_MATRIX &mtx,
		int nMulR = (double)STBF, int nMulT = (double)STBF);
	static bool PrintXYZ(LPTSTR pSymmOp, SG_MATRIX &mtx,
		int nMulR = (double)STBF, int nMulT = (double)STBF);
//	bool ParseHallSymbol(const TCHAR *pHall);
	bool Copy(const SpaceGroup &SG);
	bool Copy(const SG_ENTRY *pSgEntry);

//	bool MergeExpandReflections(ReflList &vReflList, bool bExpand);
//	bool CollapseReflections(ReflList &vReflList);
//	bool ExpandReflections(TReflsVec &vReflList);
//	bool MergeReflections(TReflsVec &vReflList);

//	static void Convert2HermannMauguin(String &sSymbol);
	static LPCTSTR GetCentring(TCHAR nCode);
	int *GetTrVectors() const;

//	bool GetAsymmBox(AsymmUnit &ASU);
//	bool CheckForAsu(const CVector3d &pos, const AsymmUnit &ASU);
//	bool MakeCoordinates(const CVector3d &pos,
//		std::vector<CVector3d> &vCoords, bool bClip = true) const;

	// 1-based number
	size_t GetSgNum() const
	{
		if( (true == m_bInitialized) && (NULL != m_SgInfo.psSgName) )
			return m_SgInfo.nSgNum;
		return 0;
	}
	LPCTSTR GetXtalSystemName()
	{
		return XS_Name[m_SgInfo.nXtalSystem];
	}
	// one of XS_Unknown, XS_Triclinic etc., defined in SgInfo.h
	int GetXtalSystem()
	{
		return m_SgInfo.nXtalSystem;
	}
	UINT GetExtNum();

	// static functions
public:
	static LPCTSTR GetSgName(size_t nSgNum, SG_TYPE nType);
	// The internal SG number, not related to real SG number
	static LPCTSTR GetAbsSgName(size_t nSgNum, SG_TYPE nType);
	static void MakePositiveHKL(HKLMiller &hkl);

private:
	void CommonConstruct(LPCTSTR pSgName, SG_TYPE nSgType);
	void ExpandMatrices();

public:
	typedef std::vector<CMatrix4x4> SymmOpVec;
	typedef std::vector<CMatrix4x4>::iterator SymmOpIt;
	typedef std::vector<CMatrix4x4>::const_iterator SymmOpCit;

	SG_ENTRY m_SgInfo;
	bool m_bInitialized;
	SymmOpVec m_vSymmOps;
};

extern LPCTSTR PG_Names[];
extern const int LG_Code_of_PG_Index[];
extern const int PlaneGr_to_Plane_PG[];

#define             Make_PG_Code( i,  p,  l) (((i) * 33 + (p)) * 12 + (l))
#define PG_Unknown  Make_PG_Code( 0,  0,  0)
#define PG_1        Make_PG_Code( 1,  1,  1)
#define PG_1b       Make_PG_Code( 2,  2,  1)
#define PG_2        Make_PG_Code( 3,  3,  2)
#define PG_m        Make_PG_Code( 4,  4,  2)
#define PG_2_m      Make_PG_Code( 5,  5,  2)
#define PG_222      Make_PG_Code( 6,  6,  3)
#define PG_mm2      Make_PG_Code( 7,  7,  3)
#define PG_mmm      Make_PG_Code( 8,  8,  3)
#define PG_4        Make_PG_Code( 9,  9,  4)
#define PG_4b       Make_PG_Code(10, 10,  4)
#define PG_4_m      Make_PG_Code(11, 11,  4)
#define PG_422      Make_PG_Code(12, 12,  5)
#define PG_4mm      Make_PG_Code(13, 13,  5)
#define PG_4b2m     Make_PG_Code(14, 14,  5)
#define PG_4bm2     Make_PG_Code(15, 14,  5)
#define PG_4_mmm    Make_PG_Code(16, 15,  5)
#define PG_3        Make_PG_Code(17, 16,  6)
#define PG_3b       Make_PG_Code(18, 17,  6)
#define PG_321      Make_PG_Code(19, 18,  7)
#define PG_312      Make_PG_Code(20, 18,  7)
#define PG_32       Make_PG_Code(21, 18,  7)
#define PG_3m1      Make_PG_Code(22, 19,  7)
#define PG_31m      Make_PG_Code(23, 19,  7)
#define PG_3m       Make_PG_Code(24, 19,  7)
#define PG_3bm1     Make_PG_Code(25, 20,  7)
#define PG_3b1m     Make_PG_Code(26, 20,  7)
#define PG_3bm      Make_PG_Code(27, 20,  7)
#define PG_6        Make_PG_Code(28, 21,  8)
#define PG_6b       Make_PG_Code(29, 22,  8)
#define PG_6_m      Make_PG_Code(30, 23,  8)
#define PG_622      Make_PG_Code(31, 24,  9)
#define PG_6mm      Make_PG_Code(32, 25,  9)
#define PG_6bm2     Make_PG_Code(33, 26,  9)
#define PG_6b2m     Make_PG_Code(34, 26,  9)
#define PG_6_mmm    Make_PG_Code(35, 27,  9)
#define PG_23       Make_PG_Code(36, 28, 10)
#define PG_m3b      Make_PG_Code(37, 29, 10)
#define PG_432      Make_PG_Code(38, 30, 11)
#define PG_4b3m     Make_PG_Code(39, 31, 11)
#define PG_m3bm     Make_PG_Code(40, 32, 11)

#define PG_Index(PG_Code)   ((PG_Code) / (33 * 12))
#define PG_Number(PG_Code) (((PG_Code) / 12) % 33)
#define LG_Number(PG_Code)  ((PG_Code) % (33 * 12))
