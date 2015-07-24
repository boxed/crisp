#include "StdAfx.h"

#include "resource.h"

#include "SpinningStar/Spacegroups.h"
#include "SpinningStar/SpinningStar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace SpinningStar
{
	void Guess2dSymmetry(const PlaneSgRsymmVec &vRsymm, int &nBestGuess,
		double dAparam, double dBparam, double dGamma, int nLaueZone);

	const int s_aiZOLZ_symmetries[MAX_ZOLZ_SYMMETRIES] =
	{
		SYMM_PG_EX_2,
		SYMM_PG_EX_2MM,
		SYMM_PG_EX_4,
		SYMM_PG_EX_4MM,
		SYMM_PG_EX_6,
		SYMM_PG_EX_6MM,
	};
	const int s_aiHOLZ_symmetries[MAX_HOLZ_SYMMETRIES] =
	{
		SYMM_PG_EX_1,
		SYMM_PG_EX_2,
		SYMM_PG_EX_M_X,
		SYMM_PG_EX_M_Y,
		SYMM_PG_EX_2MM,
		SYMM_PG_EX_4,
		SYMM_PG_EX_4MM,
		SYMM_PG_EX_3,
		SYMM_PG_EX_3M1,
		SYMM_PG_EX_31M,
		SYMM_PG_EX_6,
		SYMM_PG_EX_6MM,
	};

	const int s_aiPlane2PointGroup[SYMM_P6M+1] =
	{
		/*0*/			SYMM_PG_EX_1,
		/*SYMM_P1*/		SYMM_PG_EX_1,
		/*SYMM_P2*/		SYMM_PG_EX_2,
		/*SYMM_PMa*/	SYMM_PG_EX_M_X,
		/*SYMM_PMb*/	SYMM_PG_EX_M_Y,
		/*SYMM_PGa*/	SYMM_PG_EX_M_X,
		/*SYMM_PGb*/	SYMM_PG_EX_M_Y,
		/*SYMM_CMa*/	SYMM_PG_EX_M_X,
		/*SYMM_CMb*/	SYMM_PG_EX_M_Y,
		/*SYMM_PMM*/	SYMM_PG_EX_2MM,
		/*SYMM_PMGa*/	SYMM_PG_EX_2MM,
		/*SYMM_PMGb*/	SYMM_PG_EX_2MM,
		/*SYMM_PGG*/	SYMM_PG_EX_2MM,
		/*SYMM_CMM*/	SYMM_PG_EX_2MM,
		/*SYMM_P4*/		SYMM_PG_EX_4,
		/*SYMM_P4M*/	SYMM_PG_EX_4MM,
		/*SYMM_P4G*/	SYMM_PG_EX_4MM,
		/*SYMM_P3*/		SYMM_PG_EX_3,
		/*SYMM_P3M1*/	SYMM_PG_EX_3M1,
		/*SYMM_P31M*/	SYMM_PG_EX_31M,
		/*SYMM_P6*/		SYMM_PG_EX_6,
		/*SYMM_P6M*/	SYMM_PG_EX_6MM,
	};

	const int s_aiPtExt2PointGroup[SYMM_PG_EX_6MM+1] =
	{
		/*0*/				SYMM_PG_1,
		/*SYMM_PG_EX_1*/	SYMM_PG_1,
		/*SYMM_PG_EX_2*/	SYMM_PG_2,
		/*SYMM_PG_EX_M_X*/	SYMM_PG_M,
		/*SYMM_PG_EX_M_Y*/	SYMM_PG_M,
		/*SYMM_PG_EX_2MM*/	SYMM_PG_2MM,
		/*SYMM_PG_EX_4*/	SYMM_PG_4,
		/*SYMM_PG_EX_4MM*/	SYMM_PG_4MM,
		/*SYMM_PG_EX_3*/	SYMM_PG_3,
		/*SYMM_PG_EX_3M1*/	SYMM_PG_3M,
		/*SYMM_PG_EX_31M*/	SYMM_PG_3M,
		/*SYMM_PG_EX_6*/	SYMM_PG_6,
		/*SYMM_PG_EX_6MM*/	SYMM_PG_6MM,
	};

	// SubGroups: (7 total)
	// 0-a) pm, pm, pg, pg, cm and cm
	// 1-b) pmm, pmg, pmg, pgg and cmm
	// 2-c) p4
	// 3-d) p4m and p4g
	// 4-e) p3
	// 5-f) p3m1, p31m and p6
	// 6-g) p6m
#define	S_GR	0
#define	A_GR	1
#define	B_GR	2
#define	C_GR	3
#define	D_GR	4
#define	E_GR	5
#define	F_GR	6
#define	G_GR	7

	// The list of plane point groups for tests
	typedef struct
	{
		int nSgNum;
		int nSgTableNum;
		int nPtGroup;
		bool bZOLZ;			// true if valid for all Laue Zones, false if for HOLZ only
		LPCTSTR pszExt;
	} SG_List;

	// oblique lattice
	static const SG_List sg_oblique[] =
	{
		{1, SYMM_P1, SYMM_PG_EX_1, false, NULL},		{2, SYMM_P2, SYMM_PG_EX_2, true, NULL},
	};
	// rectangular lattice
	static const SG_List sg_rectangular[] =
	{
		{3, SYMM_PMa, SYMM_PG_EX_M_X, false, _T("a")},	{3, SYMM_PMb, SYMM_PG_EX_M_X, false, _T("b")},
		{6, SYMM_PMM, SYMM_PG_EX_2MM, true, NULL},
	};
	// squared lattice
	static const SG_List sg_square[] =
	{
		{10, SYMM_P4, SYMM_PG_EX_4, true, NULL},		{11, SYMM_P4M, SYMM_PG_EX_4MM, true, NULL},
	};
	// hexagonal lattice
	static const SG_List sg_hexagonal[] =
	{
		{13, SYMM_P3, SYMM_PG_EX_3, false, NULL},
		{14, SYMM_P3M1, SYMM_PG_EX_3M1, false, NULL},	{15, SYMM_P31M, SYMM_PG_EX_31M, false, NULL},
		{16, SYMM_P6, SYMM_PG_EX_6, true, NULL},		{17, SYMM_P6M, SYMM_PG_EX_6MM, true, NULL},
	};
/*
	// The list of plane space groups for tests
	typedef struct
	{
		int nSgNum;
		int nSgTableNum;
		LPCTSTR pszExt;
	} SG_List;

	static const SG_List sg_oblique[] =
	{
		{1, SYMM_P1, NULL},	{2, SYMM_P2, NULL},
	};

	static const SG_List sg_rectangular[] =
	{
		{3, SYMM_PMa, _T("a")},		{3, SYMM_PMb, _T("b")},
		{4, SYMM_PGa, _T("a")},		{4, SYMM_PGb, _T("b")},
		{5, SYMM_CMa, _T("a")},		{5, SYMM_CMb, _T("b")},
		{6, SYMM_PMM, NULL},
		{7, SYMM_PMGa, _T("a")},	{7, SYMM_PMGb, _T("b")},
		{8, SYMM_PGG, NULL},
		{9, SYMM_CMM, NULL},
	};

	static const SG_List sg_square[] =
	{
		{10, SYMM_P4, NULL},	{11, SYMM_P4M, NULL},	{12, SYMM_P4G, NULL},
	};

	static const SG_List sg_hexagonal[] =
	{
		{13, SYMM_P3, NULL},	{14, SYMM_P3M1, NULL},	{15, SYMM_P31M, NULL},
		{16, SYMM_P6, NULL},	{17, SYMM_P6M, NULL},
	};
*/
	// Morniroli tables, Ultramicroscopy (1992) 45, pp. 219-239.
	LPCTSTR pszZoneAxes[] =
	{
		_T("unknown"),
		_T("100"),
		_T("010"),
		_T("001"),
		_T("110"),
		_T("101"),
		_T("011"),
		_T("111"),
		_T("0001"),
		_T("11-20"),
		_T("1-100"),
		_T("uv0"),
		_T("u0w"),
		_T("uuw"),
		_T("uvw"),
		_T("uv.0"),
		_T("uu.w"),
		_T("u-u.w"),
		_T("uv.w"),
	};

#define PG_entry_name(Name)				s_##Name##_entry
#define Morniroli_entry_name(Name)		Name,s_##Name##_entry
#define End_Morniroli_entry				{PXS_Unknown,0,0,ZA_UNKNOWN}

	// TRICLINIC
	static const Morniroli_entry PG_entry_name(PG_1)[] =
	{
		{PXS_Oblique, SYMM_PG_1, SYMM_PG_1, ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_1b)[] =
	{
		{PXS_Oblique, SYMM_PG_1, SYMM_PG_2, ZA_UVW},
		End_Morniroli_entry,
	};
	// MONOCLINIC
	static const Morniroli_entry PG_entry_name(PG_2)[] =
	{
		{PXS_Oblique, SYMM_PG_2, SYMM_PG_2, ZA_010},
		{PXS_Oblique, SYMM_PG_1, SYMM_PG_M, ZA_U0W},
		{PXS_Oblique, SYMM_PG_1, SYMM_PG_1, ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_m)[] =
	{
		{PXS_Oblique, SYMM_PG_1, SYMM_PG_1, ZA_010},
		{PXS_Oblique, SYMM_PG_M, SYMM_PG_M, ZA_U0W},
		{PXS_Oblique, SYMM_PG_1, SYMM_PG_1, ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_2_m)[] =
	{
		{PXS_Oblique, SYMM_PG_2, SYMM_PG_2,   ZA_010},
		{PXS_Oblique, SYMM_PG_M, SYMM_PG_2MM, ZA_U0W},
		{PXS_Oblique, SYMM_PG_1, SYMM_PG_2,   ZA_UVW},
		End_Morniroli_entry,
	};
	// ORTHORHOMBIC
	static const Morniroli_entry PG_entry_name(PG_222)[] =
	{
		{PXS_Rectangular,	SYMM_PG_2, SYMM_PG_2MM, ZA_001},
		{PXS_Rectangular,	SYMM_PG_2, SYMM_PG_2MM, ZA_100},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_U0W},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_UV0},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_mm2)[] =
	{
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_001},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_100},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_U0W},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_UV0},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_mmm)[] =
	{
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_001},
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_100},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_U0W},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_UV0},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_2,   ZA_UVW},
		End_Morniroli_entry,
	};
	// TETRAGONAL
	static const Morniroli_entry PG_entry_name(PG_4)[] =
	{
		{PXS_Square,		SYMM_PG_4, SYMM_PG_4,  ZA_001},
		{PXS_Rectangular,	0,         0,          ZA_100},
		{PXS_Rectangular,	0,         0,          ZA_110},
		{PXS_Unknown,		0,         0,          ZA_U0W},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,  ZA_UV0},
		{PXS_Unknown,		0,         0,          ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1,  ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_4b)[] =
	{
		{PXS_Square,		SYMM_PG_2, SYMM_PG_4,  ZA_001},
		{PXS_Rectangular,	0,         0,          ZA_100},
		{PXS_Rectangular,	0,         0,          ZA_110},
		{PXS_Unknown,		0,         0,          ZA_U0W},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,  ZA_UV0},
		{PXS_Unknown,		0,         0,          ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1,  ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_4_m)[] =
	{
		{PXS_Square,		SYMM_PG_4, SYMM_PG_4,   ZA_001},
		{PXS_Rectangular,	0,         0,           ZA_100},
		{PXS_Rectangular,	0,         0,           ZA_110},
		{PXS_Unknown,		0,         0,           ZA_U0W},
		{PXS_Unknown,		SYMM_PG_M, SYMM_PG_2MM, ZA_UV0},
		{PXS_Unknown,		0,         0,           ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_2,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_422)[] =
	{
		{PXS_Square,		SYMM_PG_4, SYMM_PG_4MM, ZA_001},
		{PXS_Rectangular,	SYMM_PG_2, SYMM_PG_2MM, ZA_100},
		{PXS_Rectangular,	SYMM_PG_2, SYMM_PG_2MM, ZA_110},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_U0W},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_UV0},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_4mm)[] =
	{
		{PXS_Square,		SYMM_PG_4MM, SYMM_PG_4MM, ZA_001},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_100},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_110},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_U0W},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_UV0},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_4b2m)[] =
	{
		{PXS_Square,		SYMM_PG_2MM, SYMM_PG_4MM, ZA_001},
		{PXS_Rectangular,	SYMM_PG_2,   SYMM_PG_2MM, ZA_100},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_110},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_U0W},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_UV0},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
//	static const Morniroli_entry PG_entry_name(PG_4bm2)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_001},
//		{SYMM_PG_, SYMM_PG_, ZA_100},
//		{SYMM_PG_, SYMM_PG_, ZA_110},
//		{SYMM_PG_, SYMM_PG_, ZA_U0W},
//		{SYMM_PG_, SYMM_PG_, ZA_UV0},
//		{SYMM_PG_, SYMM_PG_, ZA_UUW},
//		{SYMM_PG_, SYMM_PG_, ZA_UVW},
//		End_Morniroli_entry,
//	};
	static const Morniroli_entry PG_entry_name(PG_4_mmm)[] =
	{
		{PXS_Square,		SYMM_PG_4MM, SYMM_PG_4MM, ZA_001},
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_100},
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_110},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_U0W},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_UV0},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_2,   ZA_UVW},
		End_Morniroli_entry,
	};
	// TRIGONAL
	static const Morniroli_entry PG_entry_name(PG_3)[] =
	{
		// 1st LZ	0th LZ
		{PXS_Hexagonal,		SYMM_PG_3, SYMM_PG_3, ZA_0001},
		{PXS_Rectangular,	0,         0,         ZA_11_20},
		{PXS_Unknown,		0,         0,         ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1, ZA_UVxW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_3b)[] =
	{
		// 1st LZ	0th LZ
		{PXS_Hexagonal,		SYMM_PG_3, SYMM_PG_6, ZA_0001},
		{PXS_Rectangular,	0,         0,         ZA_11_20},
		{PXS_Unknown,		0,         0,         ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_2, ZA_UVxW},
		End_Morniroli_entry,
	};
//	static const Morniroli_entry PG_entry_name(PG_321)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_0001},
//		{SYMM_PG_, SYMM_PG_, ZA_UVxW},
//		End_Morniroli_entry,
//	};
//	static const Morniroli_entry PG_entry_name(PG_312)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_},
//		End_Morniroli_entry,
//	};
	static const Morniroli_entry PG_entry_name(PG_32)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3, SYMM_PG_3M, ZA_0001},
		{PXS_Rectangular,	SYMM_PG_2, SYMM_PG_2,  ZA_11_20},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,  ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1,  ZA_UVxW},
		End_Morniroli_entry,
	};
//	static const Morniroli_entry PG_entry_name(PG_3m1)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_},
//		End_Morniroli_entry,
//	};
//	static const Morniroli_entry PG_entry_name(PG_31m)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_},
//		End_Morniroli_entry,
//	};
	static const Morniroli_entry PG_entry_name(PG_3m)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3M, SYMM_PG_3M, ZA_0001},
		{PXS_Rectangular,	SYMM_PG_1,  SYMM_PG_1,  ZA_11_20},
		{PXS_Unknown,		SYMM_PG_M,  SYMM_PG_M,  ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1,  SYMM_PG_1,  ZA_UVxW},
		End_Morniroli_entry,
	};
//	static const Morniroli_entry PG_entry_name(PG_3bm1)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_},
//		End_Morniroli_entry,
//	};
//	static const Morniroli_entry PG_entry_name(PG_3b1m)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_},
//		End_Morniroli_entry,
//	};
	static const Morniroli_entry PG_entry_name(PG_3bm)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3M, SYMM_PG_6MM, ZA_0001},
		{PXS_Rectangular,	SYMM_PG_2,  SYMM_PG_2,   ZA_11_20},
		{PXS_Unknown,		SYMM_PG_M,  SYMM_PG_2MM, ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1,  SYMM_PG_2,   ZA_UVxW},
		End_Morniroli_entry,
	};
	// HEXAGONAL
	static const Morniroli_entry PG_entry_name(PG_6)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_6, SYMM_PG_6, ZA_0001},
		{PXS_Rectangular,	0,         0,         ZA_11_20},
		{PXS_Rectangular,	0,         0,         ZA_1_100},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M, ZA_UVx0},
		{PXS_Unknown,		0,         0,         ZA_UUxW},
		{PXS_Unknown,		0,         0,         ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1, ZA_UVxW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_6b)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3, SYMM_PG_3, ZA_0001},
		{PXS_Rectangular,	0,         0,         ZA_11_20},
		{PXS_Rectangular,	0,         0,         ZA_1_100},
		{PXS_Unknown,		SYMM_PG_M, SYMM_PG_M, ZA_UVx0},
		{PXS_Unknown,		0,         0,         ZA_UUxW},
		{PXS_Unknown,		0,         0,         ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1, ZA_UVxW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_6_m)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_6, SYMM_PG_6,   ZA_0001},
		{PXS_Rectangular,	0,         0,           ZA_11_20},
		{PXS_Rectangular,	0,         0,           ZA_1_100},
		{PXS_Unknown,		SYMM_PG_M, SYMM_PG_2MM, ZA_UVx0},
		{PXS_Unknown,		0,         0,           ZA_UUxW},
		{PXS_Unknown,		0,         0,           ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_2,   ZA_UVxW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_622)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_6, SYMM_PG_6MM, ZA_0001},
		{PXS_Rectangular,	SYMM_PG_2, SYMM_PG_2MM, ZA_11_20},
		{PXS_Rectangular,	SYMM_PG_2, SYMM_PG_2MM, ZA_1_100},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_UVx0},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_UUxW},
		{PXS_Unknown,		SYMM_PG_1, SYMM_PG_M,   ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1, SYMM_PG_1,   ZA_UVxW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_6mm)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_6MM, SYMM_PG_6MM, ZA_0001},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_11_20},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_1_100},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_UVx0},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_UUxW},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_1,   ZA_UVxW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_6bm2)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3M,  SYMM_PG_3M,  ZA_0001},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_11_20},
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_1_100},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_UVx0},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_UUxW},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_1,   ZA_UVxW},
		End_Morniroli_entry,
	};
//	static const Morniroli_entry PG_entry_name(PG_6b2m)[] =
//	{
//		{SYMM_PG_, SYMM_PG_, ZA_},
//		{0, 0, ZA_UNKNOWN},
//	};
	static const Morniroli_entry PG_entry_name(PG_6_mmm)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_6MM, SYMM_PG_6MM, ZA_0001},
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_11_20},
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_1_100},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_UVx0},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_UUxW},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_U_UxW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_2,   ZA_UVxW},
		End_Morniroli_entry,
	};
	// CUBIC
	static const Morniroli_entry PG_entry_name(PG_23)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3,   SYMM_PG_3,   ZA_111},
		{PXS_Square,		SYMM_PG_2,   SYMM_PG_2MM, ZA_100},
		{PXS_Rectangular,	0,           0,           ZA_110},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_UV0},
		{PXS_Unknown,		0,           0,           ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_m3b)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3,   SYMM_PG_6,   ZA_111},
		{PXS_Square,		SYMM_PG_2MM, SYMM_PG_2MM, ZA_100},
		{PXS_Rectangular,	0,           0,           ZA_110},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_UV0},
		{PXS_Unknown,		0,           0,           ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_2,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_432)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3,	 SYMM_PG_3M,  ZA_111},
		{PXS_Square,		SYMM_PG_4,	 SYMM_PG_4MM, ZA_100},
		{PXS_Rectangular,	SYMM_PG_2,	 SYMM_PG_2MM, ZA_110},
		{PXS_Unknown,		SYMM_PG_1,	 SYMM_PG_M,   ZA_UV0},
		{PXS_Unknown,		SYMM_PG_1,	 SYMM_PG_M,   ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1,	 SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_4b3m)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3M,  SYMM_PG_3M,  ZA_111},
		{PXS_Square,		SYMM_PG_2MM, SYMM_PG_4MM, ZA_100},
		{PXS_Rectangular,	SYMM_PG_M,   SYMM_PG_M,   ZA_110},
		{PXS_Unknown,		SYMM_PG_1,   SYMM_PG_M,   ZA_UV0},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_M,   ZA_UUW},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_1,   ZA_UVW},
		End_Morniroli_entry,
	};
	static const Morniroli_entry PG_entry_name(PG_m3bm)[] =
	{
		{PXS_Hexagonal,		SYMM_PG_3M,  SYMM_PG_6MM, ZA_111},
		{PXS_Square,		SYMM_PG_4MM, SYMM_PG_4MM, ZA_100},
		{PXS_Rectangular,	SYMM_PG_2MM, SYMM_PG_2MM, ZA_110},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_UV0},
		{PXS_Unknown,		SYMM_PG_M,   SYMM_PG_2MM, ZA_100},
		{PXS_Oblique,		SYMM_PG_1,   SYMM_PG_2,   ZA_UVW},
		End_Morniroli_entry,
	};

	const Morniroli_table s_MorniroliTables[] =
	{
		// TRICLINIC
		{XS_Triclinic, Morniroli_entry_name(PG_1)},
		{XS_Triclinic, Morniroli_entry_name(PG_1b)},
		// MONOCLINIC
		{XS_Monoclinic, Morniroli_entry_name(PG_2)},
		{XS_Monoclinic, Morniroli_entry_name(PG_m)},
		{XS_Monoclinic, Morniroli_entry_name(PG_2_m)},
		// ORTHORHOMBIC
		{XS_Orthorhombic, Morniroli_entry_name(PG_222)},
		{XS_Orthorhombic, Morniroli_entry_name(PG_mm2)},
		{XS_Orthorhombic, Morniroli_entry_name(PG_mmm)},
		// TETRAGONAL
		{XS_Tetragonal, Morniroli_entry_name(PG_4)},
		{XS_Tetragonal, Morniroli_entry_name(PG_4b)},
		{XS_Tetragonal, Morniroli_entry_name(PG_4_m)},
		{XS_Tetragonal, Morniroli_entry_name(PG_422)},
		{XS_Tetragonal, Morniroli_entry_name(PG_4mm)},
		{XS_Tetragonal, Morniroli_entry_name(PG_4b2m)},
//		{XS_Tetragonal, Morniroli_entry_name(PG_4bm2)},
		{XS_Tetragonal, Morniroli_entry_name(PG_4_mmm)},
		// TRIGONAL
		{XS_Trigonal, Morniroli_entry_name(PG_3)},
		{XS_Trigonal, Morniroli_entry_name(PG_3b)},
//		{XS_Trigonal, Morniroli_entry_name(PG_321)},
//		{XS_Trigonal, Morniroli_entry_name(PG_312)},
		{XS_Trigonal, Morniroli_entry_name(PG_32)},
//		{XS_Trigonal, Morniroli_entry_name(PG_3m1)},
//		{XS_Trigonal, Morniroli_entry_name(PG_31m)},
		{XS_Trigonal, Morniroli_entry_name(PG_3m)},
//		{XS_Trigonal, Morniroli_entry_name(PG_3bm1)},
//		{XS_Trigonal, Morniroli_entry_name(PG_3b1m)},
		{XS_Trigonal, Morniroli_entry_name(PG_3bm)},
		// HEXAGONAL
		{XS_Hexagonal, Morniroli_entry_name(PG_6)},
		{XS_Hexagonal, Morniroli_entry_name(PG_6b)},
		{XS_Hexagonal, Morniroli_entry_name(PG_6_m)},
		{XS_Hexagonal, Morniroli_entry_name(PG_622)},
		{XS_Hexagonal, Morniroli_entry_name(PG_6mm)},
		{XS_Hexagonal, Morniroli_entry_name(PG_6bm2)},
//		{XS_Hexagonal, Morniroli_entry_name(PG_6b2m)},
		{XS_Hexagonal, Morniroli_entry_name(PG_6_mmm)},
		// CUBIC
		{XS_Cubic, Morniroli_entry_name(PG_23)},
		{XS_Cubic, Morniroli_entry_name(PG_m3b)},
		{XS_Cubic, Morniroli_entry_name(PG_432)},
		{XS_Cubic, Morniroli_entry_name(PG_4b3m)},
		{XS_Cubic, Morniroli_entry_name(PG_m3bm)},
		// Last entry
		{XS_Unknown, 0, NULL},
	};

	typedef enum
	{
		EXT_NONE = 0,				// none
		EXT_H_EV = 1,				// h=2n
		EXT_K_EV = 2,				// k=2n
		EXT_H_K_EV = 3,				// h,k=2n
		EXT_HPK_EV = 4,				// h+k=2n
		EXT_H_K_EV_HPK_4N = 5,		// h,k=2n,h+k=4n
		EXT_HPK_L_3N = 6,			// h+k-l=3n
		EXT_H_K_L_3N = 7,			// h-k-l=3n
		EXT_HPKPL_3N = 8,			// h+k+l=3n
		EXT_H_KPL_3N = 9,			// h-k+l=3n
		EXT_H_ODD = 10,				// h=2n+1
		EXT_K_ODD = 11,				// k=2n+1
		EXT_HPK_ODD = 12,			// h+k=2n+1
		EXT_H_K_ODD = 13,			// h,k=2n+1
		// ---
	} EXT_RULE_TYPE;

	static const LPCTSTR s_pszExtRule[] =
	{
		_T("none"),
		_T("h=2n"),
		_T("k=2n"),
		_T("h,k=2n"),
		_T("h+k=2n"),
		_T("h,k=2n,h+k=4n"),
		_T("h+k-l=3n"),
		_T("h-k-l=3n"),
		_T("h+k+l=3n"),
		_T("h-k+l=3n"),
		_T("h=2n+1"),
		_T("k=2n+1"),
		_T("h+k=2n+1"),
		_T("h,k=2n+1"),
	};

	enum
	{
		EVEN  = 0,
		ODD   = 1,
		ODD3N = 2,
	};

	typedef struct
	{
		int _test;				// the index will be taken (Ind MOD _test)
		int _mod_N;				// EVEN(0) if (Ind MOD _test) == 0, ODD(1) otherwise
	} TEST_HK;

	typedef struct
	{
		EXT_RULE_TYPE nRule;
		// ZOLZ & HOLZ
		TEST_HK _test_h[2];
		TEST_HK _test_k[2];
		TEST_HK _test_hk[2];
		double _scales[2];		// this part of reflections should be concidered
		// ---
	} EXT_RULE;

	typedef struct
	{
		// ZOLZ & HOLZ
		int nPlaneSymm[2];		// PXS_Unknown, PXS_Oblique etc.
		EXT_RULE_TYPE nRuleZOLZ;
		EXT_RULE_TYPE nRuleHOLZ;
		LPCTSTR pszSymbol;
		// ---
	} PART_SYMBOL;

	static bool TestRule(int h, int k, const EXT_RULE &Rule, int nLZ)
	{
		bool b1, b2, b3;
		int nRes;
		int nLayer = nLZ;

		b1 = b2 = b3 = true;
		nLZ = abs(nLZ);
		// TEST h indices (if required)
		if( 0 != Rule._test_h[nLZ]._test )
		{
			nRes = h & Rule._test_h[nLZ]._test;
			b1 = ((0 == nRes) && (EVEN == Rule._test_h[nLZ]._mod_N)) ||
				((0 != nRes) && (ODD == Rule._test_h[nLZ]._mod_N));
		}
		// TEST k indices (if required)
		if( 0 != Rule._test_k[nLZ]._test )
		{
			nRes = k & Rule._test_k[nLZ]._test;
			b2 = ((0 == nRes) && (EVEN == Rule._test_k[nLZ]._mod_N)) ||
				((0 != nRes) && (ODD == Rule._test_k[nLZ]._mod_N));
		}
		// TEST (h + k) indices (if required)
		if( Rule._test_hk[nLZ]._test > 0 )
		{
			nRes = (h+k) & Rule._test_hk[nLZ]._test;
			b3 = ((0 == nRes) && (EVEN == Rule._test_hk[nLZ]._mod_N)) ||
				((0 != nRes) && (ODD == Rule._test_hk[nLZ]._mod_N)) ||
				((0 == h+k-nLayer-3*((h+k-nLayer)/3)) && (ODD3N == abs(Rule._test_hk[nLZ]._mod_N)));
		}
		// TEST (h - k) indices (if required)
		else if( Rule._test_hk[nLZ]._test < 0 )
		{
			nRes = (h-k) & Rule._test_hk[nLZ]._test;
			b3 = ((0 == nRes) && (EVEN == Rule._test_hk[nLZ]._mod_N)) ||
				((0 != nRes) && (ODD == Rule._test_hk[nLZ]._mod_N)) ||
				((0 == h-k-nLayer-3*((h-k-nLayer)/3)) && (ODD3N == abs(Rule._test_hk[nLZ]._mod_N)));
		}
		return b1 & b2 & b3;
	}

	static bool CalculateCentring(const TReflsVec &refls, double &Rsymm,
		const EXT_RULE &Rule, int nLZ)
	{
		TReflsVecCit rit2;
		double dPassSum, dAllSum;
		int N1, N2;

		N1 = N2 = 0;
		dPassSum = dAllSum = 0.0;
		// try to estimate the R-symm
		for(rit2 = refls.begin(); rit2 != refls.end(); ++rit2)
		{
			if( true == TestRule(rit2->hkl.x, rit2->hkl.y, Rule, nLZ) )
			{
				dPassSum += rit2->Fhkl;
				N1++;
			}
			else
			{
				dAllSum += rit2->Fhkl;
				N2++;
			}
		}
		Rsymm = 0.0;
		if( (N1 > 0) && (N2 > 0) && (dPassSum > 0.0) )
		{
			Rsymm = (dAllSum / N2) / (dPassSum / N1);
//			Rsymm = (dPassSum / N1) / (dAllSum / N2);
		}
		return true;
	}

	// refls - the list loaded from the file
	static bool CalculateRsymm(SpaceGroup &sg, UnitCell::CUnitCell *pCell,
							   const TReflsVec &refls, double &Rsymm, bool bCheckFriedel)
	{
		TReflsVec merged(refls);
		TReflsVecIt rit;
		TReflsVecCit rit2;
		HKLindex restriction;
		bool bFound;
		double dFobs, dSumEven, dSumOdd;

		// initialize
		dSumEven = dSumOdd = 0.0;
		Rsymm = 0.0;
		dFobs = 0.0;

		// recalculate d-values
		for(rit = merged.begin(); rit != merged.end(); ++rit)
		{
			rit->d = 1.0 / pCell->CalculateH(rit->hkl);
		}
#ifdef _DEBUG
		if( (true == bCheckFriedel) && (13 == sg.m_SgInfo.nRefSgNum) )
		{
			TRACE(_T("----\n"));
		}
#endif // _DEBUG
		// try to merge them first according to the space group
		if( false == merged.MergeReflections(sg, merged, bCheckFriedel) )
		{
			return false;
		}
/*		for(rit = merged.begin(); rit != merged.end(); ++rit)
		{
			if( rit->hkl.z < 0 )
			{
				rit->hkl *= -1;
			}
		}
//		if( (false == bCheckFriedel) && (18 == sg.m_SgInfo.nRefSgNum) )
		if( (false == bCheckFriedel) && (1 == sg.m_SgInfo.nRefSgNum) )
		{
			TRACE(_T("----\n"));
			for(rit = merged.begin(); rit != merged.end(); ++rit)
			{
				TRACE(_T("%g\t%g\t%g\n"), rit->hkl.x, rit->hkl.y, rit->hkl.z);
			}
			TRACE(_T("----%d refls----\n"), merged.size());
		}
*/
		// try to locate each merged reflection in the initial list
		for(rit2 = refls.begin(); rit2 != refls.end(); ++rit2)
		{
			if( 0 == ((int)(rit2->hkl.x + rit2->hkl.y) & 1) )
			{
				dSumEven += rit2->Fhkl;
			}
			else
			{
				dSumOdd += rit2->Fhkl;
			}
			dFobs += rit2->Fhkl;
			bFound = false;
			// is it forbidden?
			// look for the reflection
			for(rit = merged.begin(); rit != merged.end(); ++rit)
			{
				// generate all symmetrically equivalent reflections
				// do not check Friedel pairs if the Laue Zone number differs from 0!!!
				if( 0 != sg.AreSymmEquivalent(rit->hkl, rit2->hkl, bCheckFriedel) )
				{
					Rsymm += FastAbs(rit->Fhkl - rit2->Fhkl);
					bFound = true;
					break;
				}
			}
			// if not found add it to Rsymm, can be systematically absent
			if( false == bFound )
			{
				if( 0 != sg.IsSysAbsent_hkl(rit2->hkl, restriction) )
				{
					Rsymm += rit2->Fhkl;
				}
			}
		}
		Rsymm *= 100.0 / dFobs;
#ifdef _DEBUG
		if( true == bCheckFriedel )
		{
			TRACE(_T("sg %s: SumOdd/SumEven = %g\n"), sg.m_SgInfo.psHallSymbol,
				dSumOdd/dSumEven);
		}
#endif // _DEBUG
		return true;
	}
	// informs the specified control that we need to insert/modify the string
	static void AddString(HWND hCtrl, int iItem, int iColText, LPCTSTR pszText, bool bInsert = true)
	{
		LVITEM item;
		UINT uiMsg;

		item.mask = LVIF_TEXT;
		item.iItem = iItem;
		item.iSubItem = iColText;
		item.pszText = (LPTSTR)pszText;
		uiMsg = (true == bInsert) ? LVM_INSERTITEM : LVM_SETITEM;
		::SendMessage(hCtrl, uiMsg, 0, (LPARAM)&item);
//		TRACE(_T("Set item (%d), sub (%d) to %s.\n"), iItem, iColText, pszText);
	}

	static void TestSymmetry(const SG_List *pList, int nSize, int &nCount,
		UnitCell::CUnitCell *pCell, PlaneSgRsymmVec &vRsymm, TReflsVec &vRefls,
		bool bCheckFriedel, HWND hListBox, int nColNr, int nLaueZone)
	{
		CString str;
		PlaneSgRsymmEntry RsymmEntry;
		SpaceGroup sg(1, SG_PlaneGroup);
		double Rsymm;
		int i;

		if( NULL == pList )
		{
			return;
		}
		for(i = 0; i < nSize; i++, /*nCount++, */pList++)
		{
			// we must skip ZOLZ
			if( (false == pList->bZOLZ) && (0 == nLaueZone) )
			{
				continue;
			}
			sg.SetSgNumExt(pList->nSgNum, pList->pszExt, SG_PlaneGroup);
			CalculateRsymm(sg, pCell, vRefls, Rsymm, bCheckFriedel);
			str.Format(_T("%.2f"), Rsymm);
			AddString(hListBox, nCount, nColNr, str, false);
			RsymmEntry.nSgNum = pList->nSgTableNum;
			RsymmEntry.dRsymm = Rsymm;
			vRsymm.push_back(RsymmEntry);
			nCount++;
		}
	}

	static void SkipSymmetry(const SG_List *pList, int nSize, int &nCount,
		PlaneSgRsymmVec &vRsymm, HWND hListBox, int nColNr, int nLaueZone)
	{
		PlaneSgRsymmEntry RsymmEntry;
		int i;

		if( NULL == pList )
		{
			return;
		}
		for(i = 0; i < nSize; i++, /*nCount++, */pList++)
		{
			// we must skip ZOLZ
			if( (false == pList->bZOLZ) && (0 == nLaueZone) )
			{
				continue;
			}
			AddString(hListBox, nCount, nColNr, _T("---"), false);
			RsymmEntry.nSgNum = pList->nSgTableNum;
			RsymmEntry.dRsymm = 1000.0;	// means TOO BAD
			vRsymm.push_back(RsymmEntry);
			nCount++;
		}
	}

	DWORD WINAPI TestSymmetries(LPVOID pParams)
	{
//		SpaceGroup sg(1, SG_PlaneGroup);
//		PlaneSgRsymmEntry RsymmEntry;
//		CString str;
		PlaneSgRsymmVec vRsymm;
		TestSymmetriesParams *pTSparams = (TestSymmetriesParams*)pParams;
		double a, b, gamma;
		const double dAngErr = 0.01, dParamErr = 0.05;
		double /*Rsymm, */dGamma90, dGamma120, dAB, params[3];
		UnitCell::CUnitCell *pCell, *pBasicCell = pTSparams->pBasicCell;
		int /*i, */nCount, nColNr;
		HWND hListBox = pTSparams->hListCtrl;
		bool bPutSquare = true, bCheckFriedel;
		DWORD dwRet = -1;

		if( NULL == pTSparams )
		{
			goto Error_Exit;
		}
		// check the cell type
		if( UnitCell::CELL_2D != pBasicCell->GetCellDim() )
		{
			goto Error_Exit;
		}
		// create a temporary cell
		pCell = UnitCell::CUnitCell::CreateUnitCell(UnitCell::CELL_2D);

		// 1. check the unit cell
		//		Gamma is "too far" from 90 or 120 -> Oblique
		//		Gamma is close to 90, but a is "too far" from b -> Rectangular
		//		Gamma is close to 90, a ~ b -> Squared
		//		Gamma is close to 120, a ~ b -> Hexagonal
		a = pBasicCell->GetParameter(0);
		b = pBasicCell->GetParameter(1);
		gamma = RAD2DEG(pBasicCell->GetParameter(2));
		dGamma90 = FastAbs(1.0 - 90.0/gamma);
		dGamma120 = FastAbs(1.0 - 120.0/gamma);
		dAB = FastAbs(1.0 - a / b);

		// column number with RA%
		nColNr = 1;//pTSparams->nColRA;

		// average the unit cell parameters according to the symmetry
		params[0] = a;
		params[1] = b;
		params[2] = gamma; // gamma is in Radians!
		pCell->SetParameters(params, UnitCell::RADIANS);

		nCount = 0;
		bCheckFriedel = pTSparams->bCheckFriedel;

		TestSymmetry(sg_oblique, sizeof(sg_oblique)/sizeof(SG_List),
			nCount, pCell, vRsymm, *pTSparams->pvRefls, bCheckFriedel,
			hListBox, nColNr, pTSparams->nLaueZone);
/*
		for(i = 0; i < sizeof(sg_oblique)/sizeof(SG_List); i++, nCount++)
		{
//			if( (false == bCheckFriedel) && (0 == i) )
//			{
//				// P1 has no Friedel pairs if we do not check them!!!
//				AddString(hListBox, nCount, nColNr, _T("---"), false);
//				RsymmEntry.dRsymm = 1000.0;	// means TOO BAD
//			}
//			else
//			{
				sg.SetSgNumExt(sg_oblique[i].nSgNum,
					sg_oblique[i].pszExt, SG_PlaneGroup);
				CalculateRsymm(sg, pCell, *pTSparams->pvRefls, Rsymm, bCheckFriedel);
				str.Format(_T("%.2f"), Rsymm);
				AddString(hListBox, nCount, nColNr, str, false);
				RsymmEntry.dRsymm = Rsymm;
//			}
			RsymmEntry.nSgNum = sg_oblique[i].nSgTableNum;
			vRsymm.push_back(RsymmEntry);
		}
*/
		// Gamma ~ 90
		if( dGamma90 < dAngErr )
		{
			// a != b -> Rectangular, try it as well
			// average the unit cell parameters according to the symmetry
			params[0] = a;
			params[1] = b;
			params[2] = 90.0;
			pCell->SetParameters(params);

			TestSymmetry(sg_rectangular, sizeof(sg_rectangular)/sizeof(SG_List),
				nCount, pCell, vRsymm, *pTSparams->pvRefls, bCheckFriedel,
				hListBox, nColNr, pTSparams->nLaueZone);
//			for(i = 0; i < sizeof(sg_rectangular)/sizeof(SG_List); i++, nCount++)
//			{
//				sg.SetSgNumExt(sg_rectangular[i].nSgNum,
//					sg_rectangular[i].pszExt, SG_PlaneGroup);
//				CalculateRsymm(sg, pCell, *pTSparams->pvRefls, Rsymm, bCheckFriedel);
//				str.Format(_T("%.2f"), Rsymm);
//				AddString(hListBox, nCount, nColNr, str, false);
//				RsymmEntry.nSgNum = sg_rectangular[i].nSgTableNum;
//				RsymmEntry.dRsymm = Rsymm;
//				vRsymm.push_back(RsymmEntry);
//			}
			// a ~ b -> Square
			if( dAB < dParamErr )
			{
				// average the unit cell parameters according to the symmetry
				params[0] = params[1] = 0.5*(a + b);
				params[2] = 90.0;
				pCell->SetParameters(params);
				// try all
				TestSymmetry(sg_square, sizeof(sg_square)/sizeof(SG_List),
					nCount, pCell, vRsymm, *pTSparams->pvRefls, bCheckFriedel,
					hListBox, nColNr, pTSparams->nLaueZone);
//				for(i = 0; i < sizeof(sg_square)/sizeof(SG_List); i++, nCount++)
//				{
//					sg.SetSgNumExt(sg_square[i].nSgNum,
//						sg_square[i].pszExt, SG_PlaneGroup);
//					CalculateRsymm(sg, pCell, *pTSparams->pvRefls, Rsymm, bCheckFriedel);
//					str.Format(_T("%.2f"), Rsymm);
//					AddString(hListBox, nCount, nColNr, str, false);
//					RsymmEntry.nSgNum = sg_square[i].nSgTableNum;
//					RsymmEntry.dRsymm = Rsymm;
//					vRsymm.push_back(RsymmEntry);
//				}
				bPutSquare = false;
			}
		}
		else
		{
			SkipSymmetry(sg_rectangular, sizeof(sg_rectangular)/sizeof(SG_List),
				nCount, vRsymm, hListBox, nColNr, pTSparams->nLaueZone);
//			for(i = 0; i < sizeof(sg_rectangular)/sizeof(SG_List); i++, nCount++)
//			{
//				AddString(hListBox, nCount, nColNr, _T("---"), false);
//				RsymmEntry.nSgNum = sg_rectangular/*sg_hexagonal*/[i].nSgTableNum;
//				RsymmEntry.dRsymm = 1000.0;	// means TOO BAD
//				vRsymm.push_back(RsymmEntry);
//			}
		}
		if( true == bPutSquare )
		{
			SkipSymmetry(sg_square, sizeof(sg_square)/sizeof(SG_List),
				nCount, vRsymm, hListBox, nColNr, pTSparams->nLaueZone);
//			for(i = 0; i < sizeof(sg_square)/sizeof(SG_List); i++, nCount++)
//			{
//				AddString(hListBox, nCount, nColNr, _T("---"), false);
//				RsymmEntry.nSgNum = sg_square/*sg_hexagonal*/[i].nSgTableNum;
//				RsymmEntry.dRsymm = 1000.0;	// means TOO BAD
//				vRsymm.push_back(RsymmEntry);
//			}
		}
		// Gamma ~ 120, a ~ b -> Hexagonal
		if( (dGamma120 < dAngErr) && (dAB < dParamErr) )
		{
			// average the unit cell parameters according to the symmetry
			params[0] = params[1] = 0.5*(a + b);
			params[2] = 120.0;
			pCell->SetParameters(params);

			TestSymmetry(sg_hexagonal, sizeof(sg_hexagonal)/sizeof(SG_List),
				nCount, pCell, vRsymm, *pTSparams->pvRefls, bCheckFriedel,
				hListBox, nColNr, pTSparams->nLaueZone);
//			for(i = 0; i < sizeof(sg_hexagonal)/sizeof(SG_List); i++, nCount++)
//			{
//				sg.SetSgNumExt(sg_hexagonal[i].nSgNum,
//					sg_hexagonal[i].pszExt, SG_PlaneGroup);
//				CalculateRsymm(sg, pCell, *pTSparams->pvRefls, Rsymm, bCheckFriedel);
//				str.Format(_T("%.2f"), Rsymm);
//				AddString(hListBox, nCount, nColNr, str, false);
//				RsymmEntry.nSgNum = sg_hexagonal[i].nSgTableNum;
//				RsymmEntry.dRsymm = Rsymm;
//				vRsymm.push_back(RsymmEntry);
//			}
		}
		else
		{
			SkipSymmetry(sg_hexagonal, sizeof(sg_hexagonal)/sizeof(SG_List),
				nCount, vRsymm, hListBox, nColNr, pTSparams->nLaueZone);
//			for(i = 0; i < sizeof(sg_hexagonal)/sizeof(SG_List); i++, nCount++)
//			{
//				AddString(hListBox, nCount, nColNr, _T("---"), false);
//				RsymmEntry.nSgNum = sg_hexagonal[i].nSgTableNum;
//				RsymmEntry.dRsymm = 1000.0;	// means TOO BAD
//				vRsymm.push_back(RsymmEntry);
//			}
		}
		if( NULL != pCell )
		{
			UnitCell::CUnitCell::DestroyUnitCell(pCell);
		}

		// now do our best choice
//		SYMM_2D_GUESS BestGuess;
		int nBestPG_Guess;
		// guess the best point group
		Guess2dSymmetry(vRsymm, nBestPG_Guess, a, b, gamma, pTSparams->nLaueZone);
		TRACE(_T("Best point group for %s is %d(%s)\n"),
			(0 == pTSparams->nLaueZone) ? _T("ZOLZ") : _T("HOLZ"),
			nBestPG_Guess, Plane_PG_Names_Ext[nBestPG_Guess]);
		if( (nBestPG_Guess >= 0) && (nBestPG_Guess <= MAX_HOLZ_SYMMETRIES) )
		{
			::SendMessage(pTSparams->hMainWnd, WMU_FINISHED_SYMMETRY,
				nBestPG_Guess, pTSparams->nLaueZone);
			::InvalidateRect(pTSparams->hMainWnd, NULL, TRUE);
			::RedrawWindow(hListBox, NULL, 0, RDW_INVALIDATE);
			dwRet = 0;
		}
Error_Exit:
		(*pTSparams->pThread) = NULL;
		::ExitThread(dwRet);
		return dwRet;
	}

//	static int auiSubGroups[] =	// '-1' shows the end of each subgroup, '0' - end of the table
//	{
//		SYMM_P1, SYMM_P2, -1,
//		SYMM_PMa, SYMM_PMb, SYMM_PGa, SYMM_PGb, SYMM_CMa, SYMM_CMb, -1,
//		SYMM_PMM, SYMM_PMGa, SYMM_PMGb, SYMM_PGG, SYMM_CMM, -1,
//		SYMM_P4, -1,
//		SYMM_P4M, SYMM_P4G, -1,
//		SYMM_P3, -1,
//		SYMM_P3M1, SYMM_P31M, SYMM_P6, -1,
//		SYMM_P6M,
//		0
//	};

	static const int auiSubGroups[] =	// '-1' shows the end of each subgroup, '0' - end of the whole table
	{
		SYMM_P1, SYMM_P2, -1,
		SYMM_PMa, SYMM_PMb, -1,
		SYMM_PMM, -1,
		SYMM_P4, -1,
		SYMM_P4M, -1,
		SYMM_P3, -1,
		SYMM_P3M1, SYMM_P31M, SYMM_P6, -1,
		SYMM_P6M,
		0,
	};

	void TestUnitCell(const UnitCell::CUnitCell *pCell,
		CELL_RELATION &nCell, ANGLE_RELATION &nAngle)
	{
		nCell = SpinningStar::A_NE_B;
		nAngle = SpinningStar::GAMMA_ARBITR;
		if( NULL == pCell )
		{
			return;
		}
		double dAparam, dBparam, dGamma, dRatio;

		dAparam = pCell->GetParameter(0);
		dBparam = pCell->GetParameter(1);
		dGamma = RAD2DEG(pCell->GetParameter(2));
		dRatio = dAparam / dBparam;
		if( (dRatio <= 1.2) && (dRatio >= 1./1.2) )		// a != b
		{
			nCell = SpinningStar::A_EQ_B;
		}
		if( (FastAbs(dGamma - 90) <= 10) )	// gamma ~ 90
		{
			nAngle = SpinningStar::GAMMA_90;
		}
		else if( (FastAbs(dGamma - 60) <= 10) || (FastAbs(dGamma - 120) <= 10) )
		{
			nAngle = SpinningStar::GAMMA_60_120;
		}
	}

	static bool TestSingleCell(int nSymmetry, CELL_RELATION nCell, ANGLE_RELATION nAngle)
	{
		bool bRet = false;
		switch( nSymmetry )
		{
		case PXS_Square:
			bRet = (A_EQ_B == nCell) && (GAMMA_90 == nAngle);
			break;
		case PXS_Hexagonal:
			bRet = (A_EQ_B == nCell) && (GAMMA_60_120 == nAngle);
			break;
		case PXS_Rectangular:
			// for rectangular can be both (a==b) or (a!=b)
			bRet = (GAMMA_90 == nAngle);
			break;
		case PXS_Oblique:
			// oblique can be anything
			bRet = true;
			break;
		}
		return bRet;
	}

	void Guess2dSymmetry(const PlaneSgRsymmVec &vRsymm, int &nBestGuess,
		double dAparam, double dBparam, double dGamma, int nLaueZone)
	{
		int		auiBest[8];		// winners in each group (with min residual)
		double	adResid[8];		// array of min residual for each group
		int		i, nSubGroup, nIndex, nCurSG, nOffset;
		bool	bWork;
		double	dRatio = dAparam / dBparam;
		PlaneSgRsymmVecCit cit;

		// P2/P1 is the basic guess for ZOLZ/HOLZ respectively
//		nBestGuess = 0;
		/*BestGuess.nBestPtGrpGuess*/nBestGuess = 0;
//		BestGuess.nCell = SpinningStar::A_NE_B;
//		BestGuess.nAngle = SpinningStar::GAMMA_ARBITR;
		// initialize
		for(i = 0; i < 8; i++)
		{
			adResid[i] = 1000.;		// >= than 1000 - means that it is not initialized
			auiBest[i] = -1;		// none of SG is best
		}
		// initial values
		nSubGroup = 0;
		nOffset = 0;
		nIndex = 0;
		// start
		bWork = true;
		while( bWork )
		{
			// get the current sub-group
			nCurSG = auiSubGroups[nIndex];
			if( -1 == nCurSG )
			{
				nSubGroup++;			// goto next group
			}
			else if( 0 == nCurSG )
			{
				bWork = false;			// stop working
			}
			else
			{
				// check if it is allowed for the current Laue Zone
				int nPtGroup = s_aiPlane2PointGroup[nCurSG];
				if( 0 == nLaueZone )
				{
					const int *pPtr = find(s_aiZOLZ_symmetries,
						s_aiZOLZ_symmetries+MAX_ZOLZ_SYMMETRIES, nPtGroup);
					// coudn't find?
					if( s_aiZOLZ_symmetries+MAX_ZOLZ_SYMMETRIES == pPtr )
					{
						goto Continue_test;
					}
				}
				cit = vRsymm.begin() + nOffset;//nCurSG - 1;
				nOffset++;
//				SI = &L->SI[nCurSG];
				if( cit->dRsymm <= 50.0 )//SI->Rr <= 0.5 && SI->Nr)
				{
					if( adResid[nSubGroup] > cit->dRsymm )//(adResid[nSubGroup] > SI->RFp) && (SI->flg & OR_DONE) && (SI->NFp))
					{
						adResid[nSubGroup] = cit->dRsymm;//SI->RFp;
						auiBest[nSubGroup] = nPtGroup/*nCurSG*//* - 1*/;
					}
				}
//				else
//				{
//					if((adPhResA[nSubGroup] > SI->RFp) && (SI->flg & OR_DONE) && (SI->NFp))
//					{
//						adPhResA[nSubGroup] = SI->RFp;
//						auiBestA[nSubGroup] = nCurSG;
//					}
//				}
			}
Continue_test:
			nIndex++;	// next in the group
		} // while( bWork )
		// compare results
/*		for(i = 0; i < 8; i++)
		{
			if( (0 == auiBest[i]) )//&& (auiBestA[i] != 0) )
			{
				auiBest[i] = auiBestA[i];
				adPhRes[i] = adPhResA[i];
			}
		}
*/
		const double dRatiErr = 35.0;
		// gamma != 90 && gamma != 60 && gamma != 120 - only P1 & P2 are possible
		if( (FastAbs(dGamma - 90) > 10) && (FastAbs(dGamma - 60) > 10) && 
			(FastAbs(dGamma - 120) > 10) )
		{
			nBestGuess = auiBest[S_GR];
		}
		else if( (FastAbs(dGamma - 90) <= 10) )	// gamma ~ 90
		{
			int opt_AB = -1, opt_CD = -1;
			int grp_AB = -1, grp_CD = -1;
			// look through A & B - we need this in both cases
//			if( auiBest[A_GR] != 0 || auiBest[B_GR] != 0)
			if( auiBest[A_GR] >= 0 || auiBest[B_GR] >= 0)
			{
				if( adResid[B_GR] > dRatiErr && adResid[A_GR] > dRatiErr )
				{
					grp_AB = (adResid[A_GR] < adResid[B_GR]) ? A_GR : B_GR;
//					opt_AB = (adResid[A_GR] < adResid[B_GR]) ? auiBest[A_GR] : auiBest[B_GR];
				}
				else
				{
					grp_AB = ( adResid[B_GR] <= dRatiErr && adResid[B_GR] < 1.5*adResid[A_GR] ) ? B_GR : A_GR;
//					opt_AB = ( adResid[B_GR] <= dRatiErr && adResid[B_GR] < 1.5*adResid[A_GR] ) ? auiBest[B_GR] : auiBest[A_GR];
				}
				opt_AB = auiBest[grp_AB];
			}
			// look through C & D
//			if( auiBest[C_GR] != 0 || auiBest[D_GR] != 0)
			if( auiBest[C_GR] >= 0 || auiBest[D_GR] >= 0)
			{
				if( adResid[D_GR] > dRatiErr && adResid[C_GR] > dRatiErr )
				{
					grp_CD = (adResid[C_GR] < adResid[D_GR]) ? C_GR : D_GR;
//					opt_CD = (adResid[C_GR] < adResid[D_GR]) ? auiBest[C_GR] : auiBest[D_GR];
				}
				else
				{
					grp_CD = ( adResid[D_GR] <= dRatiErr && adResid[D_GR] < 1.5*adResid[C_GR] ) ? D_GR : C_GR;
//					opt_CD = ( adResid[D_GR] <= dRatiErr && adResid[D_GR] < 1.5*adResid[C_GR] ) ? auiBest[D_GR] : auiBest[C_GR];
				}
				opt_CD = auiBest[grp_CD];
			}
			if( dRatio > 1.2 || dRatio < 1./1.2 )		// a != b	=> only A_GR & B_GR
			{
				nBestGuess = opt_AB;
			}
			else									// a == b	=> A_GR & B_GR & C_GR & D_GR
			{
				if( opt_AB >= 0 && opt_CD >= 0 && grp_AB >= 0 && grp_CD >= 0 )
				{
//					nBestGuess = ( (adResid[opt_CD] <= dRatiErr) && (adResid[opt_CD] <= 1.5*adResid[opt_AB]) ) ? opt_CD : opt_AB;
					nBestGuess = ( (adResid[grp_CD] <= dRatiErr) &&
						(adResid[grp_CD] <= 1.5*adResid[grp_AB]) ) ? opt_CD : opt_AB;
				}
				else if ( opt_AB >= 0 )
				{
					nBestGuess = opt_AB;
				}
				else if ( opt_CD >= 0 )
				{
					nBestGuess = opt_CD;
				}
			}
		}
		else if( (FastAbs(dGamma - 60) <= 10) || (FastAbs(dGamma - 120) <= 10) )
		{
			if( adResid[F_GR] < 1.5*adResid[E_GR] )
			{
				nBestGuess = auiBest[F_GR];
				if( adResid[G_GR] < 1.2*adResid[F_GR] )
				{
					nBestGuess = auiBest[G_GR];
				}
			}
			else
			{
				nBestGuess = auiBest[E_GR];
			}
		}
	}

	void GetPointGroupsList(int nPlanePG, int nCrystalSystem,
		CELL_RELATION nCell1, ANGLE_RELATION nAngle1,
		CELL_RELATION nCell2, ANGLE_RELATION nAngle2,
		ZONE_AXIS nZA, TableMatchVec &vMatches)
	{
		const Morniroli_table *pTable = s_MorniroliTables;
		const Morniroli_entry *pEntry;
		TableMatch match;

		// iterate until the table is over
		while( XS_Unknown != pTable->nCrystalSystem )
		{
			// Check the crystal system, if defined but not the same as in table - continue
			if( (XS_Unknown != nCrystalSystem) && (nCrystalSystem != pTable->nCrystalSystem) )
			{
				goto Continue_Table;
			}
			// initialize the table entry pointer
			pEntry = pTable->pEntries;
			// iterate through all entries looking for matches for the ZOLZ
			while( ZA_UNKNOWN != pEntry->nZA )
			{
				// check the zone axis and...
				if( (ZA_UNKNOWN != nZA) && (nZA != pEntry->nZA) )
				{
					goto Continue_Entries;
				}
				// ... check ZOLZ symmetry
				if( pEntry->nZOLZsymm == nPlanePG )
				{
					// check both unit cells for this projections
					if( TestSingleCell(pEntry->nPlaneSymm, nCell1, nAngle1) &&
						TestSingleCell(pEntry->nPlaneSymm, nCell2, nAngle2) )
					{
						// memorize that
						match.pTableEntry = pTable;
						match.pEntry = pEntry;
						vMatches.push_back(match);
					}
				}
Continue_Entries:
				// next entry
				pEntry++;
			}
Continue_Table:
			// Goto next table entry
			pTable++;
		}
	}

	// Get use of Morniroli tables
	void GuessPointGroup(int nBestGuess1, int nBestGuess2, int nCrystalSystem,
		const UnitCell::CUnitCell *pCell1, const UnitCell::CUnitCell *pCell2,
		ZONE_AXIS n1stZA, ZONE_AXIS n2ndZA, CString &sResults, TableMatchVec &vMatches)
	{
		const Morniroli_entry *pEntry;
		TableMatchVec vMatches1st, vMatches2nd;
		TableMatchVecIt cit, cit2;
		CString str;
		int nPgIndex;
		int nPtGrp1, nPtGrp2;
		int nCurCrystSys = -1;
		CELL_RELATION nCell1, nCell2;
		ANGLE_RELATION nAngle1, nAngle2;

		if( -1 == nBestGuess1 )
		{
			sResults.Format(_T("Error (1) -> no matching entries for the 1st file."));
			return;
		}
		sResults = _T("");
		// test both unit cells
		TestUnitCell(pCell1, nCell1, nAngle1);
		TestUnitCell(pCell2, nCell2, nAngle2);
		// both nBestGuessX are indices in the list of Plane Point Groups
		// get now the Plane Point Group Number
		// get matching list for the 1st reflections list
		nPtGrp1 = s_aiPtExt2PointGroup[s_aiZOLZ_symmetries[nBestGuess1]];
		GetPointGroupsList(nPtGrp1, nCrystalSystem, nCell1, nAngle1, nCell2, nAngle2, n1stZA, vMatches1st);
		// No matches???
		if( true == vMatches1st.empty() )
		{
			sResults.Format(_T("Error (1) -> no matching entries for the 1st file."));
			return;
		}
		// now if we do not have the 2nd file
		if( -1 == nBestGuess2 )
		{
			// check that we have not more than 7 possibilities
			if( vMatches1st.size() <= 7 )
			{
				// list all of them
				for(cit = vMatches1st.begin(); cit != vMatches1st.end(); ++cit)
				{
					nPgIndex = PG_Index(cit->pTableEntry->nPointGroup);
					// if we have a new crystal system, output its name
					if( nCurCrystSys != cit->pTableEntry->nCrystalSystem )
					{
						sResults += XS_Name[cit->pTableEntry->nCrystalSystem];
						sResults += _T("\r\n");
					}
//					str.Format(_T("%s: %s [%s]\r\n"),
//						XS_Name[cit->pTableEntry->nCrystalSystem],
//						PG_Names[nPgIndex],//PG_Names[cit->pTableEntry->nPointGroup],
//						pszZoneAxes[cit->pEntry->nZA]);
					str.Format(_T("        %s [%s]\r\n"),
						PG_Names[nPgIndex], pszZoneAxes[cit->pEntry->nZA]);
					sResults += str;
					// remember the current crystal system
					nCurCrystSys = cit->pTableEntry->nCrystalSystem;
					// add the match to the list
					vMatches.push_back(*cit);
				}
			}
			else
			{
				sResults.Format(_T("You have too many possibilities (%d in total)."),
					vMatches1st.size());
				// if user has Crystal System
				if( XS_Unknown != nCrystalSystem )
				{
					// if user has the Zone Axis
					if( ZA_UNKNOWN != n1stZA )
					{
						sResults += _T(" Try to obtain different Zone Axis pattern.");
					}
					else
					{
						sResults += _T(" Try to choose some Zone Axis for this projection.");
					}
				}
				else
				{
					sResults += _T(" Try to choose some crystal system (cubic etc.).");
				}
			}
		}
		else
		{
			nPtGrp2 = s_aiPtExt2PointGroup[s_aiHOLZ_symmetries[nBestGuess2]];
			// Try to analyze the matching list using the second file.
			// If we have 2nd list of reflections, but the n2ndZA == ZA_UNKNOWN
			// that means we are trying to analyze the HOLZ.
			if( ZA_UNKNOWN == n2ndZA )
			{
				// look through the matching list trying to locate HOLZ symmetry
				for(cit = vMatches1st.begin(); cit != vMatches1st.end(); ++cit)
				{
					if( cit->pEntry->nFOLZsymm == nPtGrp2/*PlaneGr_to_Plane_PG[nBestGuess2+1]*/ )
					{
						nPgIndex = PG_Index(cit->pTableEntry->nPointGroup);
						// if we have a new crystal system, output its name
						if( nCurCrystSys != cit->pTableEntry->nCrystalSystem )
						{
							sResults += XS_Name[cit->pTableEntry->nCrystalSystem];
							sResults += _T("\r\n");
						}
//						str.Format(_T("%s: %s [%s]\r\n"),
//							XS_Name[cit->pTableEntry->nCrystalSystem],
//							PG_Names[nPgIndex],
//							pszZoneAxes[cit->pEntry->nZA]);
						str.Format(_T("        %s [%s]\r\n"),
							PG_Names[nPgIndex], pszZoneAxes[cit->pEntry->nZA]);
						sResults += str;
						// remember the current crystal system
						nCurCrystSys = cit->pTableEntry->nCrystalSystem;
						// add the match to the list
						vMatches.push_back(*cit);
					}
				}
			}
			else
			{
				// get matching list for the 2nd reflections list
				GetPointGroupsList(nPtGrp2, nCrystalSystem, nCell1, nAngle1, nCell2, nAngle2, n2ndZA, vMatches2nd);
				if( true == vMatches2nd.empty() )
				{
					sResults.Format(_T("Error (2) -> no matching entries for the 2nd file."));
					return;
				}
				// try to find correspondence
				for(cit = vMatches1st.begin(); cit != vMatches1st.end(); ++cit)
				{
					for(cit2 = vMatches2nd.begin(); cit2 != vMatches2nd.end(); ++cit2)
					{
						if( (cit->pTableEntry->nCrystalSystem != cit2->pTableEntry->nCrystalSystem) ||
							(cit->pTableEntry->nPointGroup != cit2->pTableEntry->nPointGroup) )
						{
							continue;
						}
						pEntry = cit->pTableEntry->pEntries;
						while( ZA_UNKNOWN != pEntry->nZA )
						{
							if( pEntry == cit2->pEntry )
							{
								nPgIndex = PG_Index(cit->pTableEntry->nPointGroup);
								// if we have a new crystal system, output its name
								if( nCurCrystSys != cit->pTableEntry->nCrystalSystem )
								{
									sResults += XS_Name[cit->pTableEntry->nCrystalSystem];
									sResults += _T("\r\n");
								}
//								str.Format(_T("%s: %s\r\n"),
//									XS_Name[cit->pTableEntry->nCrystalSystem],
//									PG_Names[nPgIndex]);
								str.Format(_T("        %s\r\n"), PG_Names[nPgIndex]);
								sResults += str;
								// remember the current crystal system
								nCurCrystSys = cit->pTableEntry->nCrystalSystem;
								// add the match to the list
								vMatches.push_back(*cit);
							}
							// next entry
							pEntry++;
						}
					}
				}
			}
			if( 0 == sResults.GetLength() )
			{
				sResults = _T("No matches found. Try to change crystal system/"
					"projection symmetry/zone axis.");
			}
		}
	}

	typedef struct
	{
		int nSymm;					// XS_Cubic etc.
		const ZONE_AXIS *paiZA;
		int nZA;					// the total number of zone axes in the pavZA list
		const PART_SYMBOL *pTables;	// corresponding tables
		int nTables;
		// ---
	} PARTIAL_SYMB_TABLE;

	typedef struct
	{
		const PARTIAL_SYMB_TABLE *pTable;
		const PART_SYMBOL *pSymbol;

	} SUCCEEDED_PART_SYMB;

//////////////////////////////////////////////////////////////////////////
//								ORTHORHOMBIC
//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Orthorhombic_001[] =
	{
		// orthorhombic <001> ZA
		// ZOLZ - none, HOLZ - none
		/* 1*/{{PXS_Rectangular, PXS_Rectangular},	EXT_NONE,	EXT_NONE,	_T("P-../P.-./P..-")},
		// ZOLZ - h=2n, HOLZ - none
		/* 2*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_EV,	EXT_NONE,	_T("Pb../P.a./P..b")},
		// ZOLZ - k=2n, HOLZ - none
		/* 3*/{{PXS_Rectangular, PXS_Rectangular},	EXT_K_EV,	EXT_NONE,	_T("Pc../P.c./P..a")},
		// ZOLZ - h+k=2n, HOLZ - none
		/* 4*/{{PXS_Rectangular, PXS_Rectangular},	EXT_HPK_EV,	EXT_NONE,	_T("Pn../P.n./P..n")},
		// ZOLZ - k=2n, HOLZ - k=2n+1
		/* 5*/{{PXS_Rectangular, PXS_Rectangular},	EXT_K_EV,	EXT_K_ODD,	_T("B-../A.-./B..-")},
		// ZOLZ - h=2n, k=2n, HOLZ - k=2n+1
		/* 6*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_K_ODD,	_T("Bb../A.a./B..b")},
		// ZOLZ - h=2n, HOLZ - k=2n+1
		/* 7*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_EV,	EXT_K_ODD,	_T("C-../C.-./A..-")},
		// ZOLZ - h=2n, k=2n, HOLZ - k=2n+1
		/* 8*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_K_ODD,	_T("Cc../C.c./A..a")},
		// ZOLZ - h+k=2n, HOLZ - h+k=2n+1
		/* 9*/{{PXS_Rectangular, PXS_Rectangular},	EXT_HPK_EV,	EXT_HPK_ODD,_T("I-../I.-./I..-")},
		// ZOLZ - h+k=2n, HOLZ - h+k=2n
		/*10*/{{PXS_Rectangular, PXS_Rectangular},	EXT_HPK_EV,	EXT_HPK_EV,	_T("A-../B.-./C..-")},
		// ZOLZ - h=2n, k=2n, HOLZ - h=2n+1
		/*11*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_H_ODD,	_T("Cc../C.c./A..a")},
		// ZOLZ - h=2n, HOLZ - h=2n+1
		/*12*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_EV,	EXT_H_ODD,	_T("C-../C.-./A..-")},
		// ZOLZ - h=2n, k=2n, HOLZ - h+k=2n
		/*13*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_HPK_EV, _T("Ab.. or Ac../B.a. or B.c./C..a or C..b")},
		// ZOLZ - h=2n, k=2n, HOLZ - h+k=2n+1
		/*14*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV, EXT_HPK_ODD,_T("Ib.. or Ic../I.a. or I.c./I..a or I..b")},
		// ZOLZ - h=2n, k=2n, HOLZ - h=2n+1, k=2n+1
		/*15*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_H_K_ODD,_T("F-../F.-./F..-")},
		// ZOLZ - h=2n, k=2n, h+k=4n, HOLZ - h=2n+1, k=2n+1
		/*16*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV_HPK_4N, EXT_H_K_ODD,_T("Fd../F.d./F..d")},
	};
	static const ZONE_AXIS s_auiZA_Orthorhombic_001[] = {ZA_100/*, ZA_010, ZA_001*/};
//////////////////////////////////////////////////////////////////////////
//								TETRAGONAL
//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Tetragonal_001[] =
	{
		// tetragonal <001> ZA
		// ZOLZ - none, HOLZ - none
//		/* 1*/{{PXS_Square, PXS_Square},
//			{0, EVEN, 0, EVEN}, {0, EVEN, 0, EVEN}, {0, EVEN, 0, EVEN}, _T("P-..")},
		// ZOLZ - h+k=2n, HOLZ - none
		/* 2*/{{PXS_Square, PXS_Square},	EXT_HPK_EV,		EXT_NONE,		_T("Pn..")},
		// ZOLZ - h+k=2n, HOLZ - h+k=2n+1
		/* 3*/{{PXS_Square, PXS_Square},	EXT_HPK_EV,		EXT_HPK_ODD,	_T("I-..")},
		// ZOLZ - h=2n, k=2n, h+k=4n, HOLZ - h+k=2n+1
		/* 4*/{{PXS_Square, PXS_Square},	EXT_H_K_EV_HPK_4N, EXT_HPK_ODD,	_T("Ia..")},
	};
	static const ZONE_AXIS s_auiZA_Tetragonal_001[] = {ZA_001};
	//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Tetragonal_100[] =
	{
		// tetragonal <100> ZA
		// ZOLZ - none, HOLZ - none
		/* 5*/{{PXS_Rectangular, PXS_Rectangular},	EXT_NONE,	EXT_NONE,	_T("P.-.")},
		// tetragonal <100> ZA
		// ZOLZ - k=2n, HOLZ - none
		/* 6*/{{PXS_Rectangular, PXS_Rectangular},	EXT_K_EV,	EXT_NONE,	_T("P.b.")},
		// tetragonal <100> ZA
		// ZOLZ - h=2n, HOLZ - none
		/* 7*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_EV,	EXT_NONE,	_T("P.c.")},
		// tetragonal <100> ZA
		// ZOLZ - h+k=2n, HOLZ - none
		/* 8*/{{PXS_Rectangular, PXS_Rectangular},	EXT_HPK_EV,	EXT_NONE,	_T("P.n.")},
		// tetragonal <100> ZA
		// ZOLZ - h+k=2n, HOLZ - h+k=2n+1
		/* 9*/{{PXS_Rectangular, PXS_Rectangular},	EXT_HPK_EV, EXT_HPK_ODD,_T("I.-.")},
		// tetragonal <100> ZA
		// ZOLZ - h,k=2n, HOLZ - h+k=2n+1
		/*10*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_HPK_ODD,_T("I.c.")},
	};
	static const ZONE_AXIS s_auiZA_Tetragonal_100[] = {ZA_100/*, ZA_010*/};
	//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Tetragonal_110[] =
	{
		// tetragonal <110> ZA
		// ZOLZ - k=2n, HOLZ - k=2n+1
		/*11*/{{PXS_Rectangular, PXS_Rectangular},	EXT_K_EV,		EXT_K_ODD,		_T("P..-")},
		// ZOLZ - h,k=2n, HOLZ - k=2n+1
		/*12*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,		EXT_K_ODD,		_T("P..c")},
		// ZOLZ - h+k=2n, HOLZ - h+k=2n+1
		/*13*/{{PXS_Rectangular, PXS_Rectangular},	EXT_HPK_EV,		EXT_HPK_ODD,	_T("I..-")},
		// ZOLZ - h,k=2n, h+k=4n, HOLZ - h,k=2n+1
		/*14*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV_HPK_4N,EXT_H_K_ODD,	_T("I..d")},
	};
	static const ZONE_AXIS s_auiZA_Tetragonal_110[] = {ZA_110};
	//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//								HEXAGONAL
//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Tri_0001[] =
	{
		// trigonal(rhomboherdic) <0001> ZA
		// ZOLZ - h+k=3n, HOLZ - h-k=3n
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_HPKPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_HPK_L_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_H_KPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_H_K_L_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_HPKPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_HPK_L_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_H_KPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_H_K_L_3N,	_T("hR..")},
	};
	static const ZONE_AXIS s_auiZA_Tri_0001[] = {ZA_0001};
	//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_HexTri_0001[] =
	{
		// hexagonal/trigonal <0001> ZA
		// ZOLZ - h+k=3n, HOLZ - h-k=3n
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_HPKPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_HPK_L_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_H_KPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_H_K_L_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_HPKPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_HPK_L_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_H_KPL_3N,	_T("hR..")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_H_K_L_3N,	_T("hR..")},
	};
	static const ZONE_AXIS s_auiZA_HexTri_0001[] = {ZA_0001};
	//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_HexTri_11_20[] =
	{
		// hexagonal/trigonal <11-200> ZA
		// ZOLZ - none, HOLZ - k=2n+1
		/* 1*/{{PXS_Rectangular, PXS_Rectangular},	EXT_NONE,		EXT_K_ODD,	_T("P.-.")},
		// ZOLZ - h,k=2n, HOLZ - k=2n+1
		/* 2*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,		EXT_K_ODD,	_T("P.c.")},
	};
	static const ZONE_AXIS s_auiZA_HexTri_11_20[] = {ZA_11_20};
	//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_HexTri_1_100[] =
	{
		// hexagonal/trigonal <1-100> ZA
		// ZOLZ - k=2n, HOLZ - k=2n+1
		/* 3*/{{PXS_Rectangular, PXS_Rectangular},	EXT_K_EV,	EXT_K_ODD,	_T("P..-")},
		// ZOLZ - h,k=2n, HOLZ - k=2n+1
		/* 4*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_K_ODD,	_T("P..c")},
	};
	static const ZONE_AXIS s_auiZA_HexTri_1_100[] = {ZA_1_100};
	//////////////////////////////////////////////////////////////////////////
//	static const PART_SYMBOL s_PS_Rhombo_11_20[] =
//	{
//	};
//	static const ZONE_AXIS s_auiZA_Rhombo_11_20[] = {ZA_11_20};

//////////////////////////////////////////////////////////////////////////
//								CUBIC
//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Cubic_100[] =
	{
		// cubuc <110> ZA
		// ZOLZ - none, HOLZ - none
		/* 1*/{{PXS_Square, PXS_Square},	EXT_NONE,	EXT_NONE,	_T("P-..")},
		// ZOLZ - h=2n, HOLZ - none
		/* 2*/{{PXS_Square, PXS_Square},	EXT_H_EV,	EXT_NONE,	_T("Pa-3")},
		// ZOLZ - h+k=2n, HOLZ - none
		/* 3*/{{PXS_Square, PXS_Square},	EXT_HPK_EV,	EXT_NONE,	_T("Pn..")},
		// ZOLZ - h+k=2n, HOLZ - h+k=2n+1
		/* 4*/{{PXS_Square, PXS_Square},	EXT_HPK_EV,	EXT_HPK_ODD,_T("I-../F-..")},
		// ZOLZ - h,k=2n, h+k=4n, HOLZ - h+k=2n+1
		/* 5*/{{PXS_Square, PXS_Square},	EXT_H_K_EV_HPK_4N, EXT_HPK_ODD,	_T("Ia../Fd..")},
	};
	static const ZONE_AXIS s_auiZA_Cubic_100[] = {ZA_100, ZA_010, ZA_001};
	//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Cubic_110[] =
	{
		// cubuc <110> ZA
		// ZOLZ - h=2n, HOLZ - h=2n+1
		/* 6*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_EV,	EXT_H_ODD,	_T("P..-")},
		// ZOLZ - h,k=2n, HOLZ - h=2n+1
		/* 7*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_H_ODD,	_T("P..n")},
		// ZOLZ - h,k=2n, HOLZ - h,k=2n+1
		/* 8*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_H_K_ODD,_T("I..-")},
		// ZOLZ - h,k=2n, h+k=4n, HOLZ - h,k=2n+1
		/* 9*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV_HPK_4N, EXT_H_K_ODD,	_T("I..d")},
		// ZOLZ - h+k=2n, HOLZ - h+k=2n+1
		/*10*/{{PXS_Rectangular, PXS_Rectangular},	EXT_HPK_EV,	EXT_HPK_ODD,_T("F..-")},
		// ZOLZ - h,k=2n, HOLZ - h+k=2n+1
		/*11*/{{PXS_Rectangular, PXS_Rectangular},	EXT_H_K_EV,	EXT_HPK_ODD,_T("F..c")},
	};
	static const ZONE_AXIS s_auiZA_Cubic_110[] = {ZA_110, ZA_101, ZA_011};
	//////////////////////////////////////////////////////////////////////////
	static const PART_SYMBOL s_PS_Cubic_111[] =
	{
		// TODO: partial symbol???
		// cubic <111> ZA
		// ZOLZ - h+k=3n, HOLZ - h-k=3n
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_HPKPL_3N,	_T("I...")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_HPK_L_3N,	_T("I...")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_H_KPL_3N,	_T("I...")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_HPK_L_3N,	EXT_H_K_L_3N,	_T("I...")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_HPKPL_3N,	_T("I...")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_HPK_L_3N,	_T("I...")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_H_KPL_3N,	_T("I...")},
		/* 0*/{{PXS_Hexagonal, PXS_Hexagonal},	EXT_H_K_L_3N,	EXT_H_K_L_3N,	_T("I...")},
	};
	static const ZONE_AXIS s_auiZA_Cubic_111[] = {ZA_111};

	//////////////////////////////////////////////////////////////////////////

#ifndef _countof
#define	_countof(x)		(sizeof(x)/sizeof(x[0]))
#endif

#define MakeSymb(x)		&x[0], _countof(x)
	
	static const PARTIAL_SYMB_TABLE s_aPartTables[] =
	{
		// monoclinic
		// orthorhombic
		{XS_Orthorhombic,	MakeSymb(s_auiZA_Orthorhombic_001), MakeSymb(s_PS_Orthorhombic_001)},
		// tetragonal
		{XS_Tetragonal,		MakeSymb(s_auiZA_Tetragonal_001), MakeSymb(s_PS_Tetragonal_001)},
		{XS_Tetragonal,		MakeSymb(s_auiZA_Tetragonal_100), MakeSymb(s_PS_Tetragonal_100)},
		{XS_Tetragonal,		MakeSymb(s_auiZA_Tetragonal_110), MakeSymb(s_PS_Tetragonal_110)},
		// hexagonal/trigonal
		{XS_Hexagonal,		MakeSymb(s_auiZA_HexTri_11_20), MakeSymb(s_PS_HexTri_11_20)},
		{XS_Hexagonal,		MakeSymb(s_auiZA_HexTri_1_100), MakeSymb(s_PS_HexTri_1_100)},
		{XS_Trigonal,		MakeSymb(s_auiZA_Tri_0001), MakeSymb(s_PS_Tri_0001)},
		{XS_Trigonal,		MakeSymb(s_auiZA_HexTri_11_20), MakeSymb(s_PS_HexTri_11_20)},
		{XS_Trigonal,		MakeSymb(s_auiZA_HexTri_1_100), MakeSymb(s_PS_HexTri_1_100)},
		// rhombohedral
//		{XS_Trigonal,		MakeSymb(s_auiZA_Rhombo_11_20), MakeSymb(s_PS_Rhombo_11_20)},
		// cubic
		{XS_Cubic,			MakeSymb(s_auiZA_Cubic_100), MakeSymb(s_PS_Cubic_100)},
		{XS_Cubic,			MakeSymb(s_auiZA_Cubic_110), MakeSymb(s_PS_Cubic_110)},
		{XS_Cubic,			MakeSymb(s_auiZA_Cubic_111), MakeSymb(s_PS_Cubic_111)},
	};

#undef	max

	static bool CanTestPartSymb(const TableMatch *pMatch,
		const PARTIAL_SYMB_TABLE *pTable)
	{
		int iZA;
		// ... and compare the Crystal Symmetry ...
		if( pMatch->pTableEntry->nCrystalSystem != pTable->nSymm )
		{
			return false;
		}
		// ... and compare the Zone Axis
		for(iZA = 0; iZA < pTable->nZA; iZA++)
		{
			if( pMatch->pEntry->nZA == pTable->paiZA[iZA] )
			{
				return true;
			}
		}
		return false;
	}

	static bool CanTestPartSymbWithCell(const PART_SYMBOL *pSymbol,
		const UnitCell::CUnitCell *pCell1, const UnitCell::CUnitCell *pCell2)
	{
		CELL_RELATION nCell1, nCell2;
		ANGLE_RELATION nAngle1, nAngle2;

		// test both cells
		TestUnitCell(pCell1, nCell1, nAngle1);
		TestUnitCell(pCell2, nCell2, nAngle2);
		if( TestSingleCell(pSymbol->nPlaneSymm[0], nCell1, nAngle1) &&
			TestSingleCell(pSymbol->nPlaneSymm[1], nCell2, nAngle2) )
		{
			return true;
		}
		return false;
	}

	void GuessPartialSymbol(TReflsVec *pvRefls1, TReflsVec *pvRefls2,
		const UnitCell::CUnitCell *pCell1, const UnitCell::CUnitCell *pCell2,
		CString &strResult, const TableMatchVec &vMatches)
	{
		// no lists?
		if( (NULL == pvRefls1) && (NULL == pvRefls2) )
		{
			return;
		}
		// no cells?
		if( (NULL == pCell1) && (NULL == pCell2) )
		{
			return;
		}
		TableMatchVecCit mit;
		vector<SUCCEEDED_PART_SYMB> vSuccess;
		SUCCEEDED_PART_SYMB succ_symb;
		const PART_SYMBOL *pSymbol;
		const PARTIAL_SYMB_TABLE *pTable;
		int nTables, iTable, iSymbol, iBestSymbol;
		double Rsymm1, Rsymm2, dMinRzolz, dMinRholz;
//		bool bPassed1, bPassed2;
		CString str;
		EXT_RULE_TYPE nZolz, nHolz;

		static const EXT_RULE s_ZOLZ[] =
		{
			{EXT_H_EV,   {1, EVEN, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}},// _T("h=2n")
			{EXT_K_EV,   {0, 0, 0, 0}, {1, EVEN, 0, 0}, {0, 0, 0, 0}},// _T("k=2n")},
			{EXT_HPK_EV, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, EVEN, 0, 0}},// _T("h+k=2n")},
			{EXT_H_K_EV, {1, EVEN, 0, 0}, {1, EVEN, 0, 0}, {0, 0, 0, 0}},// _T("h,k=2n")},
			{EXT_H_K_EV_HPK_4N, {1, EVEN, 0, 0}, {1, EVEN, 0, 0}, {3, EVEN, 0, 0}},// _T("h,k=2n,h+k=4n")},
			{EXT_HPK_L_3N, {0, 0, 0, 0}, {0, 0, 0, 0}, {1, ODD3N, 0, 0}},// _T("h+k-l=3n")},
			{EXT_H_K_L_3N, {0, 0, 0, 0}, {0, 0, 0, 0}, {-1, ODD3N, 0, 0}},// _T("h-k-l=3n")},
		};
		static const EXT_RULE s_HOLZ[] =
		{
			{EXT_H_ODD, {0, 0, 1, ODD}, {0, 0, 0, 0}, {0, 0, 0, 0}},// _T("h=2n+1")},
			{EXT_K_ODD, {0, 0, 0, 0}, {0, 0, 1, ODD}, {0, 0, 0, 0}},// _T("k=2n+1")},
			{EXT_HPK_EV, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, EVEN}},// _T("h+k=2n")},
			{EXT_HPK_ODD, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, ODD}},// _T("h+k=2n+1")},
			{EXT_H_K_ODD, {0, 0, 1, ODD}, {0, 0, 1, ODD}, {0, 0, 0, 0}},// _T("h,k=2n+1")},
			{EXT_HPK_L_3N, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, ODD3N}},// _T("h+k-l=3n")},
			{EXT_H_K_L_3N, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, -1, ODD3N}},// _T("h-k-l=3n")},
			{EXT_HPKPL_3N, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 1, -ODD3N}},// _T("h+k+l=3n")},
			{EXT_H_KPL_3N, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, -1, -ODD3N}},// _T("h-k+l=3n")},
		};
		dMinRzolz = dMinRholz = std::numeric_limits<double>::max();
		nZolz = nHolz = EXT_NONE;
		if( (NULL != pvRefls1) && (NULL != pCell1) )
		{
			TRACE(_T("------- ZOLZ --------\n"));
			for(int i = 0; i < _countof(s_ZOLZ); i++)
			{
				CalculateCentring(*pvRefls1, Rsymm1, s_ZOLZ[i], 0);
				TRACE(_T("%s: R=%g\n"), s_pszExtRule[s_ZOLZ[i].nRule], Rsymm1);
				if( Rsymm1*0.5 < dMinRzolz )
				{
					nZolz = s_ZOLZ[i].nRule;
					dMinRzolz = Rsymm1;
				}
			}
		}
		if( (NULL != pvRefls2) && (NULL != pCell2) )
		{
			TRACE(_T("------- HOLZ --------\n"));
			for(int i = 0; i < _countof(s_HOLZ); i++)
			{
				if( s_HOLZ[i]._test_hk[1]._mod_N < 0 )
				{
					CalculateCentring(*pvRefls2, Rsymm2, s_HOLZ[i], 1);
				}
				else
				{
					CalculateCentring(*pvRefls2, Rsymm2, s_HOLZ[i], -1);
				}
				TRACE(_T("%s: R=%g\n"), s_pszExtRule[s_HOLZ[i].nRule], Rsymm2);
				if( Rsymm2*0.5 < dMinRholz )
				{
					nHolz = s_HOLZ[i].nRule;
					dMinRholz = Rsymm2;
				}
			}
		}
		strResult = _T("");
		if( (EXT_NONE != nZolz) && (EXT_NONE != nHolz) )
		{
			TRACE(_T("best R(ZOLZ) = %4.1f%% for '%s'\n"), dMinRzolz*100, s_pszExtRule[nZolz]);
			TRACE(_T("best R(HOLZ) = %4.1f%% for '%s'\n"), dMinRholz*100, s_pszExtRule[nHolz]);
			TRACE(_T("-------------------------------\n"));
			str.Format(_T("R(ZOLZ)=%4.1f%%, %s\r\nR(HOLZ)=%4.1f%%, %s\r\n"),
				dMinRzolz*100, s_pszExtRule[nZolz], dMinRholz*100, s_pszExtRule[nHolz]);
			strResult += str;
			strResult += _T("-------------------------------\r\n");
		}
		// the total number of tables
		nTables = _countof(s_aPartTables);
		// iterate through all matched symmetries
		for(mit = vMatches.begin(); mit != vMatches.end(); ++mit)
		{
			// iterate through all the symmetries to check their partial symbols
			pTable = s_aPartTables;
			for(iTable = 0; iTable < nTables; iTable++, pTable++)
			{
				// check that we can test this table
				if( false == CanTestPartSymb(&*mit, pTable) )
				{
//					TRACE(_T("symmetry match '%s' <%s> failed for %s <%s>\n"),
//						XS_Name[mit->pTableEntry->nCrystalSystem],
//						pszZoneAxes[mit->pEntry->nZA],
//						XS_Name[pTable->nSymm],
//						pszZoneAxes[pTable->paiZA[0]]);
					continue;
				}
				TRACE(_T("symmetry match '%s' <%s> succeeded for %s <%s>\n"),
					XS_Name[mit->pTableEntry->nCrystalSystem],
					pszZoneAxes[mit->pEntry->nZA],
					XS_Name[pTable->nSymm],
					pszZoneAxes[pTable->paiZA[0]]);
				// initialize the current best symbol
				iBestSymbol = -1;
//				dMinRzolz = std::numeric_limits<double>::max();
//				dMinRholz = std::numeric_limits<double>::max();
				// get the pointer to all partial symbols
				pSymbol = pTable->pTables;
				// iterate through all symbols
				for(iSymbol = 0; iSymbol < pTable->nTables; iSymbol++, pSymbol++)
				{
					// test the cell
					if( false == CanTestPartSymbWithCell(pSymbol, pCell1, pCell2) )
					{
						TRACE(_T("\tsymbol match '%s' failed\n"), pSymbol->pszSymbol);
						continue;
					}
					TRACE(_T("\tsymbol match '%s' succeeded\n"), pSymbol->pszSymbol);
					if( (nZolz == pSymbol->nRuleZOLZ) &&
						(nHolz == pSymbol->nRuleHOLZ) )
					{
						TRACE(_T("R(ZOLZ)=%g, R(HOLZ)=%g, Partial symbol found: %s, <%s>\n"),
							Rsymm1, Rsymm2, pSymbol->pszSymbol, pszZoneAxes[pTable->paiZA[0]]);
						iBestSymbol = iSymbol;
						succ_symb.pSymbol = pSymbol;
						succ_symb.pTable = pTable;
						vSuccess.push_back(succ_symb);
					}
					// all preliminary tests succeeded
/*					bPassed1 = bPassed2 = true;
					if( (NULL != pvRefls1) && (NULL != pCell1) )
					{
						// get the R-symm
						if( true == CalculateCentring(*pvRefls1, Rsymm1, *pSymbol, 0) )
						{
							bPassed1 = true;
						}
					}
					if( (NULL != pvRefls2) && (NULL != pCell2) )
					{
						// get the R-symm
						if( true == CalculateCentring(*pvRefls2, Rsymm2, *pSymbol, 1) )
						{
							bPassed2 = true;
						}
					}
					// both tests must pass
					if( bPassed1 && bPassed2 )
					{
						succ_symb.pSymbol = pSymbol;
						succ_symb.pTable = pTable;
						vSuccess.push_back(succ_symb);
						Rsymm1 = (1.0 - Rsymm1) / pSymbol->_scales[0];
						Rsymm2 = (1.0 - Rsymm2) / pSymbol->_scales[1];
						TRACE(_T("R(ZOLZ)=%g, R(HOLZ)=%g, Partial symbol found: %s, <%s>\n"),
							Rsymm1, Rsymm2, pSymbol->pszSymbol, pszZoneAxes[pTable->paiZA[0]]);
						if( dMinRzolz > Rsymm1 )
						{
							dMinRzolz = Rsymm1;
							dMinRholz = Rsymm2;
							iBestSymbol = iSymbol;
						}
					}
*/
				} // for(iSymbol...)
				if( iBestSymbol >= 0 )
				{
					str.Format(_T("%s <%s>, %s\r\n"), XS_Name[pTable->nSymm],
						pszZoneAxes[pTable->paiZA[0]],
						pTable->pTables[iBestSymbol].pszSymbol);
					strResult += str;
				}
			}
		}
		if( TRUE == strResult.IsEmpty() )
		{
			strResult = _T("--- no partial symbol was found ---");
		}
	}

/*
	bool TestSymmetries2(const fmPath &path, StringVec &vSymmRes)
	{
		HKE_FileOpenInfo hke;
		CrystalStructure cs;

		// check the extension first
		if( 0 != _tcscmp(path.ext(), _T(".hke")) )
		{
			EMAP_DebugOutputString(0, COLOR_RED, _T("Spinning Star error: "
				"file '%s' is not HKE file."), path.c_str());
			return false;
		}
		hke.m_pCS = &cs;
		cs.m_pCell = UnitCell::CUnitCell::CreateUnitCell(UnitCell::CELL_2D);
		if( false == fmFileSystem::instance().OpenFile(path, &hke) )
		{
			EMAP_DebugOutputString(0, COLOR_RED, _T("Spinning Star error: "
				"faild to load file '%s'."), path.c_str());
			return false;
		}
	//	hke.m_pCS->
		// start the thread
		::CreateThread()
		if( NULL != cs.m_pCell )
		{
			UnitCell::CUnitCell::DestroyUnitCell(cs.m_pCell);
			cs.m_pCell = NULL;
		}
		return true;
	}
/*
	void SpinningStar_TestSymmetries(TReflsVec *pvRefls, UnitCell::CUnitCell *pCell)
	{
		try
		{
			CSpinStarWizard wizard(_T("Symmetry check"));
			CSpinStarSymmPage symm;

			wizard.AddPage(&symm);

			wizard.m_pCell = pCell;
			wizard.m_pvRefls = pvRefls;

			wizard.SetWizardMode();
			if( ID_WIZFINISH == wizard.DoModal() )
			{
			}
		}
		catch(...)
		{
			EMAP_DebugOutputString(0, COLOR_RED,
				_T("SpinningStar_TestSymmetries(): unhandled exception."));
		}
	//	::AfxGetApp()->OnCmdMsg(ID_FILE_NEW_WIZ, 0, NULL, NULL);
	}
*/

}; // namespace SpinningStar
