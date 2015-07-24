#pragma once

//void SpinningStar_TestSymmetries(CrystalStructure *pCryst);

#include "shString.h"
#include "SpinningStar/UnitCell.h"
#include "SpinningStar/Treflection.h"

// this message is to be send when the Thread has finished its work to calculate Rsymm
#define		WMU_FINISHED_SYMMETRY	(WM_USER + 100)
#define		WMU_SET_BEST_CHOICE		(WM_USER + 101)

namespace SpinningStar
{
#	define		MAX_ZOLZ_SYMMETRIES		6
#	define		MAX_HOLZ_SYMMETRIES		12

	extern const int s_aiZOLZ_symmetries[MAX_ZOLZ_SYMMETRIES];
	extern const int s_aiHOLZ_symmetries[MAX_HOLZ_SYMMETRIES];
	extern const int s_aiPlane2PointGroup[SYMM_P6M+1];

	// Morniroli tables, Ultramicroscopy (1992) 45, pp. 219-239.
	typedef enum
	{
		ZA_UNKNOWN = 0,
		ZA_100,
		ZA_010,
		ZA_001,
		ZA_110,
		ZA_101,
		ZA_011,
		ZA_111,
		ZA_0001,
		ZA_11_20,
		ZA_1_100,
		ZA_UV0,
		ZA_U0W,
		ZA_UUW,
		ZA_UVW,
		ZA_UVx0,
		ZA_UUxW,
		ZA_U_UxW,
		ZA_UVxW,

	} ZONE_AXIS;

	// Zone axes names
	extern LPCTSTR pszZoneAxes[];

	typedef struct
	{
		int nPlaneSymm;		// one of PXS_Square etc.
		int nFOLZsymm;
		int nZOLZsymm;
		ZONE_AXIS nZA;

	} Morniroli_entry;

	typedef struct
	{
		int nCrystalSystem;					// one of XS_<...>
		int nPointGroup;					// one of SYMM_PG_<...>
		const Morniroli_entry *pEntries;	// Must finish with {0,0,ZA_UNKNOWN}

	} Morniroli_table;

	typedef struct
	{
		const Morniroli_table *pTableEntry;
		const Morniroli_entry *pEntry;

	} TableMatch;

	typedef std::vector<TableMatch> TableMatchVec;
	typedef std::vector<TableMatch>::iterator TableMatchVecIt;
	typedef std::vector<TableMatch>::const_iterator TableMatchVecCit;

	extern const Morniroli_table s_MorniroliTables[];

	// End of Morniroli tables definitions

	typedef struct
	{
		TReflsVec *pvRefls;
		UnitCell::CUnitCell *pBasicCell;
		HANDLE *pThread;					// the thread HANDLE to be initialized to NULL after finishing
		HWND hListCtrl;						// the handle to the ListCtrl to add Rsymm
		HWND hMainWnd;						// the main Spinnig Star window handle
		int nLaueZone;						// the Laue Zone, 0 - ZOLZ, 1,2... - HOLZ
		bool bCheckFriedel;					// false if no check for Friedel pairs in LZ > 0

	} TestSymmetriesParams;

	typedef enum
	{
		A_EQ_B = 0,
		A_NE_B = 1,
		// ---
	} CELL_RELATION;
	
	typedef enum
	{
		GAMMA_90 = 0,						// the angle is 90 degrees
		GAMMA_60_120 = 1,					// the angle is 60/120 degrees
		GAMMA_ARBITR = 2,					// the angle is different from 60/90/120
		// ---
	} ANGLE_RELATION;
	
	typedef struct
	{
		int nSgNum;			// one of 21 (17+different axis choise) plane space groups, SYMM_<...>
		double dRsymm;		// Rsymm

	} PlaneSgRsymmEntry;

	typedef std::vector<PlaneSgRsymmEntry> PlaneSgRsymmVec;
	typedef PlaneSgRsymmVec::iterator PlaneSgRsymmVecIt;
	typedef PlaneSgRsymmVec::const_iterator PlaneSgRsymmVecCit;

	// Threaded execution
	DWORD WINAPI TestSymmetries(LPVOID pParams);

//	void TestUnitCell(const UnitCell::CUnitCell *pCell,
//		CELL_RELATION &nCell, ANGLE_RELATION &nAngle);
//
//	void Guess2dSymmetry(const PlaneSgRsymmVec &vRsymm, int &nBestGuess,
//		double dAparam, double dBparam, double dGamma, int nLaueZone);

	void GuessPointGroup(int nBestGuess1, int nBestGuess2, int nCrystalSystem,
		const UnitCell::CUnitCell *pCell1, const UnitCell::CUnitCell *pCell2,
		ZONE_AXIS n1stZA, ZONE_AXIS n2ndZA, CString &sResults, TableMatchVec &vMatches);

	void GuessPartialSymbol(TReflsVec *pvRefls1, TReflsVec *pvRefls2,
		const UnitCell::CUnitCell *pCell1, const UnitCell::CUnitCell *pCell2,
		CString &strResult, const TableMatchVec &vMatches);

}; // namespace SpinningStar
