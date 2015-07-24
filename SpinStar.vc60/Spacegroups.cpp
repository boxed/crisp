#include "StdAfx.h"

#pragma warning( disable : 4786 )
#pragma warning( disable : 4305 )

#include "shString.h"

#include "SpinningStar/UnitCell.h"
#include "SpinningStar/SpaceGroups.h"

using namespace std;

//#include "asymm.inc"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

////////////////////////////////////////////////////////////
// default ASU
//static const AS_UNIT s_DefaultASU = asymm_units[0];

////////////////////////////////////////////////////////////
LPCTSTR EI_Name[] =
{
	_T("Unknown"),
	_T("Enantiomorphic"),
	_T("Obverse"),
	_T("Reverse"),
};

LPCTSTR XS_Name[] =
{
	_T("Unknown"),
	_T("Triclinic"),
	_T("Monoclinic"),
	_T("Orthorhombic"),
	_T("Tetragonal"),
	_T("Trigonal"),
	_T("Hexagonal"),
	_T("Cubic"),
};

LPCTSTR PXS_Name[] =
{
	_T("Unknown"),
	_T("Oblique"),
	_T("Rectangular"),
	_T("Square"),
	_T("Hexagonal"),
};

LPCTSTR Centr_Name[] =
{
	_T("Primitive"),
	_T("face centered"),
	_T("Body centered"),
	_T("Hexagonal obverse"),
	_T("All-face centered"),
};

LPCTSTR S_Unknown = _T("Unknown");

#if 0
static LPCTSTR ExtendedSG_UniqueAxis[] =
{
	/*  0 */ _T("abc"),
	/*  1 */ _T("ba-c"),
	/*  2 */ _T("cab"),
	/*  3 */ _T("-cba"),
	/*  4 */ _T("bca"),
	/*  5 */ _T("a-cb"),
	/*  6 */ _T("bac"), /* 6 -> 1 */
	/*  7 */ _T("cba"), /* 7 -> 3 */
	/*  8 */ _T("acb"), /* 8 -> 5 */
	/*  9 */ _T("-b"), _T("b-"), _T("bb"), _T("bb"), /* 10, 11, 12 ->  9 */
	/* 13 */ _T("-c"), _T("c-"), _T("bc"), _T("cb"), /* 14, 15, 16 -> 13 */
	/* 17 */ _T("-a"), _T("a-"), _T("ba"), _T("ab"), /* 18, 19, 20 -> 17 */
	/* 21 */ _T("b"),
	/* 22 */ _T("c"),
	/* 23 */ _T("a"),
	NULL,
};
#endif

// Extern arrays and variables
extern SG_ENTRY sg_tables_plane[];
extern SG_ENTRY sg_tables[];
//extern BRIK asymm_units[];

class InitSgInfo
{
private:
	typedef multimap<String, SG_ENTRY*> Map;
	typedef Map::iterator MapIt;
	typedef Map::const_iterator MapCit;
	typedef pair<String, SG_ENTRY*> Pair;

	typedef struct
	{
		size_t nMaxSgNum;
		SG_ENTRY *pTables;
		Map *pSgMap;

	} SG_TABLES;

	Map SgMap;
	Map SgMapPlane;

public:
	static SG_TABLES tables_def[3];

	static size_t GetSgMapSize()		{ return theInitSgInfo.SgMap.size(); }
	static size_t GetSgMapPlaneSize()	{ return theInitSgInfo.SgMapPlane.size(); }

private:
	void MakeSgName(LPCTSTR pszName, LPTSTR pszOut)
	{
		while( *pszName )
		{
			if( TCHAR('=') == *pszName )
			{
				break;
			}
			else if( (TCHAR(' ') != *pszName) && (TCHAR('_') != *pszName) )
			{
				*pszOut = *pszName;
				pszOut = _tcsinc(pszOut);
			}
			pszName = _tcsinc(pszName);
		}
		*pszOut = 0;
	}

public:
	InitSgInfo()
	{
		InitSgInfo::MapCit cit;
		TCHAR sgname[32];
		SG_ENTRY *pEntry = sg_tables;
		Map *pMap = &SgMap;
		int i;

		// initialize some pointers
		tables_def[1].pSgMap = &SgMapPlane;
		tables_def[2].pSgMap = &SgMap;
		// initialize Space and Plane groups
		for(i = 0; i < 2; i++)
		{
			while( 0 != pEntry->nSgNum )
			{
				// make a regular name
				MakeSgName(pEntry->psSgLabel, sgname);
				cit = pMap->find(sgname);
				if( cit == pMap->end() )
				{
					// found it
					pMap->insert(Pair(sgname, pEntry));
				}
				pMap->insert(Pair(pEntry->psSgName, pEntry));
				pEntry++;
			}
			// second cycle - change references and pointers
			pEntry = sg_tables_plane;
			pMap = &SgMapPlane;
		}
	}
	~InitSgInfo()
	{
		SgMap.clear();
		SgMapPlane.clear();
	}

//////////////////////////////////////////////////////////////////////////

	static bool GetSgInfo(size_t nSgNum, SG_ENTRY &SgInfo, SG_TYPE nType)
	{
		if( (nType != SG_PlaneGroup) && (nType != SG_SpaceGroup) )
		{
			TRACE(_T("GetSgInfo() -> wrong space group type (can be plane or space).\n"));
			return false;
		}
		// check the space group number
		if( nSgNum > tables_def[(int)nType].nMaxSgNum )
		{
			return false;
		}
		SG_ENTRY *pEntry = tables_def[(int)nType].pTables;
		while( (0 != pEntry->nSgNum) && (nSgNum != pEntry->nSgNum) )
		{
			pEntry++;
		}
		memcpy(&SgInfo, pEntry, sizeof(SgInfo));
		return true;
	}

	static bool GetSgInfoExt(size_t nSgNum, LPCTSTR pszExtension,
		SG_ENTRY &SgInfo, SG_TYPE nType)
	{
		SG_ENTRY *pEntry;
		bool bFindExt = false;

		if( (nType != SG_PlaneGroup) && (nType != SG_SpaceGroup) )
		{
			TRACE(_T("GetSgInfo() -> wrong space group type (can be plane or space).\n"));
			return false;
		}
		pEntry = tables_def[(int)nType].pTables;
		// check if the SG number is valid
		if( nSgNum > tables_def[(int)nType].nMaxSgNum )//230 )
		{
			return false;
		}
		if( (NULL != pszExtension) && (0 != pszExtension[0]) )
		{
			bFindExt = true;
		}
		while( 0 != pEntry->nSgNum )
		{
			// found the SG number?
			if( nSgNum == pEntry->nSgNum )
			{
				// check the extension if needed
				if( true == bFindExt )
				{
					if( 0 == _tcsicmp(pszExtension, pEntry->psExtension) )
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			pEntry++;
		}
		if( 0 != pEntry->nSgNum )
		{
			memcpy(&SgInfo, pEntry, sizeof(SgInfo));
			return true;
		}
		memset(&SgInfo, 0, sizeof(SgInfo));
		return false;
	}

	static bool GetSgInfo(LPCTSTR psSgName, SG_ENTRY &SgInfo, SG_TYPE nType)
	{
		SG_ENTRY *pEntry;
		InitSgInfo::MapCit it_lower, it_upper;
		vector<TCHAR> sSgName;
		size_t nSize;

		if( (nType != SG_PlaneGroup) && (nType != SG_SpaceGroup) )
		{
			TRACE(_T("GetSgInfo() -> wrong space group type (can be plane or space).\n"));
			return false;
		}
		if( NULL == psSgName )
		{
			return false;
		}
		if( 0 == psSgName[0] )
		{
			return false;
		}
		nSize = _tcslen(psSgName);
		sSgName.resize(nSize+2);
		fill_n(sSgName.begin(), nSize+2, 0);
		_tcscpy(&*sSgName.begin(), psSgName);
		// zero the structure
		memset(&SgInfo, 0, sizeof(SgInfo));
		// look the lower and upper bound for the search symbol
		InitSgInfo::Map &theMap = *tables_def[(int)nType].pSgMap;
		String sgName(&*sSgName.begin());
		it_lower = theMap.lower_bound(sgName);
		it_upper = theMap.upper_bound(sgName);
//		if( (it_lower == theInitSgInfo.SgMap.end()) ||
		if( (it_lower == theMap.end()) ||
			(0 != _tcscmp(it_lower->first/*.c_str()*/, psSgName)) )
		{
			// check some special cases
			TCHAR cLast = sSgName[nSize-1];// psSgName[nSize-1];
			switch( cLast )
			{
			case TCHAR('H'):
			case TCHAR('h'):
				sSgName[nSize-1] = 0;
				cLast = TCHAR('H');
				break;
			default:
				break;
			}
			it_lower = theMap.lower_bound(sgName/*sSgName.begin()*/);
			it_upper = theMap.upper_bound(sgName/*sSgName.begin()*/);
			if( (it_lower == theMap.end()) ||
				(0 != _tcscmp(it_lower->first/*.c_str()*/, &*sSgName.begin())) )
			{
				return false;
			}
			if( it_lower->second->psExtension[0] != cLast )
			{
				return false;
			}
		}
		pEntry = it_lower->second;
		// check the entry
		if( NULL == pEntry )
		{
			return false;
		}

		memcpy(&SgInfo, pEntry, sizeof(SgInfo));
		return true;
	}

} theInitSgInfo;

//InitSgInfo::Map InitSgInfo::SgMap;
//InitSgInfo::Map InitSgInfo::SgMapPlane;
InitSgInfo::SG_TABLES InitSgInfo::tables_def[3] =
{
	{0,   NULL,				NULL},	// nothing, not defined
	{17,  sg_tables_plane,	NULL},	// SG_PlaneGroup
	{230, sg_tables,		NULL},	// SG_SpaceGroup
};

//////////////////////////////////////////////////////////////////////////

static int TrVectorsP[] =
{
	0,0,0,									// 'P'
};
static int TrVectorsA[] =
{
	0,0,0, 0,STBF/2,STBF/2,					// 'A'
};
static int TrVectorsB[] =
{
	0,0,0, STBF/2,0,STBF/2,					// 'B'
};
static int TrVectorsC[] =
{
	0,0,0, STBF/2,STBF/2,0,					// 'C'
};
static int TrVectorsI[] =
{
	0,0,0, STBF/2,STBF/2,STBF/2,			// 'I'
};
static int TrVectorsR[] =
{
	0,0,0, 2*STBF/3,STBF/3,STBF/3, STBF/3,2*STBF/3,2*STBF/3,	// 'R'
};
static int TrVectorsS[] =
{
	0,0,0, STBF/3,STBF/3,2*STBF/3, 2*STBF/3,2*STBF/3,STBF/3,	// 'S'
};
static int TrVectorsT[] =
{
	0,0,0, STBF/3,2*STBF/3,STBF/3, 2*STBF/3,STBF/3,2*STBF/3,	// 'T'
};
static int TrVectorsF[] =
{
	0,0,0, 0,STBF/2,STBF/2, STBF/2,0,STBF/2, STBF/2,STBF/2,0,	// 'F'
};

////////////////////////////////////////////////////////////////////////////////

void SetMtx(const SG_MATRIX &mtx, CMatrix4x4 &out, double dMul1, double dMul2)
{
	out.LoadIdentity();

	out.d[0][0] = mtx.s.R[0] * dMul1;
	out.d[0][1] = mtx.s.R[1] * dMul1;
	out.d[0][2] = mtx.s.R[2] * dMul1;
	out.d[0][3] = mtx.s.T[0] * dMul2;

	out.d[1][0] = mtx.s.R[3] * dMul1;
	out.d[1][1] = mtx.s.R[4] * dMul1;
	out.d[1][2] = mtx.s.R[5] * dMul1;
	out.d[1][3] = mtx.s.T[1] * dMul2;

	out.d[2][0] = mtx.s.R[6] * dMul1;
	out.d[2][1] = mtx.s.R[7] * dMul1;
	out.d[2][2] = mtx.s.R[8] * dMul1;
	out.d[2][3] = mtx.s.T[2] * dMul2;
}

////////////////////////////////////////////////////////////////////////////////

SpaceGroup::SpaceGroup(size_t nSgNum, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	SetSgNum(nSgNum, nType);
}

SpaceGroup::SpaceGroup(size_t nSgNum, size_t nSgExt, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	TCHAR inum[64];
	bool bFound;
	int nCount;

	// check the sg type
	if( (nType != SG_SpaceGroup) && (nType != SG_PlaneGroup) )
	{
		nSgNum = 1;
		nSgExt = 0;
		nType = SG_SpaceGroup;
	}
	if( (0 == nSgNum) || (nSgNum > InitSgInfo::tables_def[(int)nType].nMaxSgNum) )
	{
		nSgNum = 1;
		nSgExt = 0;
	}

	if( 0 == nSgExt )
	{
		_stprintf(inum, _T("%d"), nSgNum);
	}
	else
	{
		const SG_ENTRY *pSgEntry = InitSgInfo::tables_def[(int)nType].pTables;
		bFound = false;
		// check every space group
		while( 0 != pSgEntry->nSgNum )
		{
			if( nSgNum == pSgEntry->nSgNum )
			{
				// find the extension number
				nCount = 0;
				while( nSgNum == pSgEntry->nSgNum )
				{
					if( nCount == nSgExt )
					{
						bFound = true;
						break;
					}
					// next entry
					pSgEntry++;
					nCount++;
				}
			}
			if( true == bFound )
			{
				_stprintf(inum, _T("%d:%s"), nSgNum, pSgEntry->psExtension);
				break;
			}
			// next entry
			pSgEntry++;
		}
	}
	if( false == bFound )
	{
		_stprintf(inum, _T("%d"), nSgNum);
	}

	CommonConstruct(inum, nType);
}

SpaceGroup::SpaceGroup(LPCTSTR pSgName, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	CommonConstruct(pSgName, nType);
	m_bInitialized = false;
}

SpaceGroup::SpaceGroup(const SG_ENTRY *pSgEntry)
{
	if( NULL != pSgEntry )
	{
		memcpy(&m_SgInfo, pSgEntry, sizeof(m_SgInfo));
		m_bInitialized = true;
	}
}

SpaceGroup::~SpaceGroup(void)
{
}

void SpaceGroup::CommonConstruct(LPCTSTR pSgName, SG_TYPE nSgType)
{
	if( false == InitSgInfo::GetSgInfo(pSgName, m_SgInfo, nSgType) )
	{
		LPCTSTR ptr;
		// may be a number?
		long nSg = _ttol(pSgName);
		// check the SG number
		if( (nSg > 0) && (nSg <= InitSgInfo::tables_def[(int)nSgType].nMaxSgNum) )
		{
			if( NULL != (ptr = _tcschr(pSgName, TCHAR(':'))) )
			{
				// a space group number + extension
				SetSgNumExt( nSg, _tcsinc(ptr), nSgType );
			}
			else
			{
				// just a space group number
				SetSgNum( nSg, nSgType );
			}
		}
		else
		{
			// ERROR
			TCHAR msg[256];
			_stprintf(msg, _T("SpaceGroup::CommonConstruct() -> failed to find symbol: %s.\n"), pSgName);
#ifdef _MFC_VER
			::OutputDebugString(msg);
#else
			fprintf(ferror, msg);
#endif
			m_bInitialized = false;
			return;
		}
	}
	// expand symmetry operations
	ExpandMatrices();
	m_bInitialized = true;
}

UINT SpaceGroup::GetExtNum()
{
	int nCount;
	int nSgNum = m_SgInfo.nSgNum;
//	const SG_ENTRY *pSgEntry = sg_tables;
	const SG_ENTRY *pSgEntry = InitSgInfo::tables_def[m_SgInfo.nSgType].pTables;

	// check every space group
	while( 0 != pSgEntry->nSgNum )
	{
		// found the desired SG number
		if( nSgNum == pSgEntry->nSgNum )
		{
			// find the extension number
			nCount = 0;
			while( nSgNum == pSgEntry->nSgNum )
			{
				if( 0 == _tcscmp(m_SgInfo.psExtension, pSgEntry->psExtension) )
				{
					return nCount;
				}
				// next entry
				pSgEntry++;
				nCount++;
			}
		}
		// next entry
		pSgEntry++;
	}
	return 0;
}

void SpaceGroup::ExpandMatrices()
{
	SG_MATRIX mtx;
	CMatrix4x4 sgMtx;
	double dMul = 1;
	static const double invSTBF = 1.0 / double(STBF);
	bool bContinue = true;
	int *pTr = GetTrVectors();
	int i, j, k;

	m_vSymmOps.clear();
//Continue_Centric:
	// apply all translation vectors
	for(j = 0; j < m_SgInfo.nTrVector; j++)
	{
		dMul = 1;
		bContinue = true;
Continue_Centric:
		for(i = 0; i < m_SgInfo.nList; i++)
		{
			sgMtx.LoadIdentity();
			mtx = m_SgInfo.Matrices[i];
			for(k = 0; k < 3; k++)
			{
				mtx.s.T[k] += pTr[j*3+k];
				while( mtx.s.T[k] < 0 ) mtx.s.T[k] += STBF;
				mtx.s.T[k] %= STBF;
			}
			// initialize the matrix class
			SetMtx(mtx, sgMtx, dMul, invSTBF);
			// add it to the symm-operations vector
			m_vSymmOps.push_back(sgMtx);
		}
		// continue with Centrosymmetric?
		if( (0 != m_SgInfo.nCentric) && (true == bContinue) )
		{
			bContinue = false;
			dMul = -1;
			goto Continue_Centric;
		}
	}
}

bool SpaceGroup::Copy(const SpaceGroup &SG)
{
	memcpy(&m_SgInfo, &SG.m_SgInfo, sizeof(m_SgInfo));
	ExpandMatrices();
	m_bInitialized = true;
	return m_bInitialized;
}

bool SpaceGroup::Copy(const SG_ENTRY *pSgEntry)
{
	m_bInitialized = false;
	if( NULL != pSgEntry )
	{
		memcpy(&m_SgInfo, pSgEntry, sizeof(m_SgInfo));
		ExpandMatrices();
		m_bInitialized = true;
	}
	return m_bInitialized;
}

bool SpaceGroup::SetSgNum(size_t nSgNum, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	m_bInitialized = InitSgInfo::GetSgInfo((int)nSgNum, m_SgInfo, nType);
	ExpandMatrices();

	return m_bInitialized;
}

bool SpaceGroup::SetSgNumExt(size_t nSgNum, LPCTSTR pszExtension,
							 SG_TYPE nType/*= SG_SpaceGroup*/)
{
	m_bInitialized = InitSgInfo::GetSgInfoExt((int)nSgNum, pszExtension, m_SgInfo, nType);
	ExpandMatrices();

	return m_bInitialized;
}

bool SpaceGroup::SetSgNumExt(size_t nSgNum, size_t nSgExt, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	int nCount;
	const SG_ENTRY *pSgEntry = InitSgInfo::tables_def[(int)nType].pTables;
	// check every space group
	while( 0 != pSgEntry->nSgNum )
	{
		// found the desired SG number
		if( nSgNum == pSgEntry->nSgNum )
		{
			// find the extension number
			nCount = 0;
			while( nSgNum == pSgEntry->nSgNum )
			{
				if( nSgExt == nCount )
				{
					m_bInitialized = InitSgInfo::GetSgInfoExt((int)nSgNum,
						pSgEntry->psExtension, m_SgInfo, nType);
					goto Exit_Init;
//					break;
				}
				// next entry
				pSgEntry++;
				nCount++;
			}
		}
		// next entry
		pSgEntry++;
	}
Exit_Init:
	ExpandMatrices();

	return m_bInitialized;
}

bool SpaceGroup::SetSgName(LPCTSTR pSgName, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	CommonConstruct(pSgName, nType);
//	m_bInitialized = true;

	return m_bInitialized;
}

LPCTSTR SpaceGroup::GetSgName(size_t nSgNum, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	if( (nType != SG_SpaceGroup) && (nType != SG_PlaneGroup) )
	{
		return S_Unknown;
	}
	if( nSgNum > InitSgInfo::tables_def[nType].nMaxSgNum )
		return S_Unknown;

	SG_ENTRY *pEntry = InitSgInfo::tables_def[nType].pTables;
	while( (0 != pEntry->nSgNum) && (nSgNum != pEntry->nSgNum) )
	{
		pEntry++;
	}

	return pEntry->psSgName;
}

LPCTSTR SpaceGroup::GetAbsSgName(size_t nSgNum, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	if( SG_SpaceGroup == nType )
	{
		if( nSgNum >= InitSgInfo::GetSgMapSize() )
		{
			return S_Unknown;
		}
	}
	else if( SG_PlaneGroup == nType )
	{
		if( nSgNum >= InitSgInfo::GetSgMapPlaneSize() )
		{
			return S_Unknown;
		}
	}
	return InitSgInfo::tables_def[nType].pTables[nSgNum].psSgName;
}
// iCentric = -1 - acentric, 1 - centric, 0 - no info
// bExpandInv - expand inverse
// bExpandCentr - expand if centering is known
bool SpaceGroup::SetSgFromSymmOp(SG_MATRIX *pSymmOps, int nOps, int iCentric, TCHAR cCode,
								 bool bExpandInv, bool bExpandCentr, SG_TYPE nType/*= SG_SpaceGroup*/)
{
	SpaceGroup sg(1, nType);
	SG_ENTRY *pEntry = InitSgInfo::tables_def[nType].pTables;
	int nMtx, nSymmOp;
	SymmOpVec vMtx, SymmOps;
	SymmOpIt mit, it;

	// convert all matrices into CMatrix4x4
	SymmOps.resize(nOps);
	for(nSymmOp = 0; nSymmOp < nOps; nSymmOp++)
	{
		SetMtx(pSymmOps[nSymmOp], SymmOps[nSymmOp], 1.0, 1.0 / double(STBF));
	}
	// check every space group
	while( 0 != pEntry->nSgNum )
	{
		sg.Copy(pEntry);
		vMtx.clear();
		if( 0 != iCentric )
		{
			if( (1 == iCentric) && (0 == pEntry->nCentric) )
			{
				// our SG is centric, but pEntry is acentric
				goto Continue_Search;
			}
			if( (-1 == iCentric) && (0 != pEntry->nCentric) )
			{
				// our SG is acentric, but pEntry is centric
				goto Continue_Search;
			}
		}
		// lattice centering
		if( 0 != cCode )
		{
			if( cCode != pEntry->cCode )
			{
				goto Continue_Search;
			}
		}
		// total # of matrices
		nMtx = pEntry->nList;
		if( true == bExpandCentr )
		{
			nMtx *= pEntry->nTrVector;
		}
		if( (-1 == pEntry->nCentric) && (true == bExpandInv) )	// requires Inverse operation
		{
			nMtx *= 2;
		}
		if( nMtx != nOps )
		{
			goto Continue_Search;		// different number of matrices
		}
		// now we have all matrices expanded, compare them
		vMtx.resize(nMtx);
		// copy ONLY those we need (maybe no need in INVERSED!!!)
		copy(sg.m_vSymmOps.begin(), sg.m_vSymmOps.begin() + nMtx, vMtx.begin());
		// now we have all matrices expanded, compare them
		for(it = SymmOps.begin(); it != SymmOps.end(); ++it)
		{
			// check all the current space group matrices
			mit = vMtx.begin();
			while( mit != vMtx.end() )
			{
				// just equal
				if( 0 == memcmp((void*)&*mit, (void*)&*it, sizeof(CMatrix4x4)) )
				{
					vMtx.erase(mit);	// erase it
					break;				// stop because they are equal
				}
				++mit;
			}
			if( mit == vMtx.end() )
			{
				// continue with next space group
				break;
			}
		}
		// found???
		if( vMtx.empty() )
		{
			break;
		}
Continue_Search:
		pEntry++;
	}
	if( vMtx.empty() && (NULL != pEntry) && (0 != pEntry->nSgNum) )
	{
		Copy(pEntry);
		m_bInitialized = true;
		return true;
	}
	return false;
}

static LPTSTR PrintOffset(LPTSTR pszString, int nValue, bool bHadSymbol, bool bPrint_0_1)
{
	LPTSTR ptr = pszString;
	int n1, n2;

	if( true == bPrint_0_1 )
	{
		if( nValue == 0 )
		{
			*ptr++ = TCHAR('0');
			/* *ptr++ = 0; */
			return ptr;
		}
		if( nValue == 12 )
		{
			*ptr++ = TCHAR('1');
			/* *ptr++ = 0; */
			return ptr;
		}
	}
	switch(nValue)
	{
	case 0: /* *ptr++ = 0; */return ptr;
	case 1: n1 = 1; n2 = 12; break;
	case 2: n1 = 1; n2 = 6; break;
	case 3: n1 = 1; n2 = 4; break;
	case 4: n1 = 1; n2 = 3; break;
	case 5: n1 = 5; n2 = 12; break;
	case 6: n1 = 1; n2 = 2; break;
	case 7: n1 = 7; n2 = 12; break;
	case 8: n1 = 2; n2 = 3; break;
	case 9: n1 = 3; n2 = 4; break;
	case 10: n1 = 5; n2 = 6; break;
	case 11: n1 = 11; n2 = 12; break;
	case 12: /* *ptr++ = 0; */return ptr;
	default: TRACE(_T("unrecognized translation: %d/12.\n"), nValue);
	}
	if( nValue < 0 )
	{
		*ptr++ = TCHAR('-');
	}
	else if( nValue > 0 )
	{
		if( true == bHadSymbol )
			*ptr++ = TCHAR('+');
	}
	_stprintf(ptr, _T("%d/%d"), n1, n2);
	return ptr += _tcslen(ptr);
}

bool SpaceGroup::PrintXYZ(LPTSTR pszSymmOp, SG_MATRIX &m,
		int nMulR/*= (double)STBF*/, int nMulT/*= (double)STBF*/)
{
	LPTSTR ptr = pszSymmOp;
	int i, j;
	bool bHadSymbol;
	static LPCTSTR coord = _T("xyz");

	for(i = 0; i < 3; i++)
	{
		bHadSymbol = false;
		for(j = 0; j < 3; j++)
		{
			if( m.s.R[i*3+j] < 0 )
			{
				*ptr++ = TCHAR('-');
				bHadSymbol = true;
				*ptr++ = coord[j];
			}
			else if( m.s.R[i*3+j] > 0 )
			{
				if( true == bHadSymbol )
					*ptr++ = TCHAR('+');
				bHadSymbol = true;
				*ptr++ = coord[j];
			}
		}
		ptr = PrintOffset(ptr, abs(m.s.T[i]), bHadSymbol, false);
		if( i != 2 )
			*ptr++ = TCHAR(',');
	}
	// zero-end
	*ptr = 0;
	return true;
}

bool SpaceGroup::ParseSymmOpText(LPCTSTR pSymmOp, SG_MATRIX &mtx,
		int nMulR/*= (double)STBF*/, int nMulT/*= (double)STBF*/)
{
	// examples
	// '-x+1/2, y+1/2, -z+1/2'
	TCHAR *ptr, cCurSymb;
	double dOffset, dOffsetDen;
	bool bWork, bFirstNum, bRealOff, bHasScale;
	int nSign, nX, nY, nZ, nRow, nDigits;
	// initialize to zeros
	memset(&mtx, 0, sizeof(mtx));

	// initialize
	ptr = (TCHAR*)pSymmOp;
	nRow = 0;
	bFirstNum = true;
	bRealOff = false;		// not a normal real number
	nX = nY = nZ = 0;
	dOffset = dOffsetDen = 0.0;
	nSign = 1;
	nDigits = 0;
	bHasScale = false;
	cCurSymb = TCHAR('\0');
	// work until finished
	bWork = true;
	while( bWork )
	{
		switch( *ptr )
		{
		case 0:
			// finished
			bWork = false;
		case TCHAR(','):
			// new field, store previous
			mtx.s.R[nRow*3]   = nX;
			mtx.s.R[nRow*3+1] = nY;
			mtx.s.R[nRow*3+2] = nZ;
			if( true == bRealOff )
			{
				// 0.00001 for example
//				mtx.s.T[nRow] = floor(STBF * nSign * (dOffset+dOffsetDen*pow(10.0, -nDigits)) + 0.5);
				mtx.s.T[nRow] = floor(nMulT * nSign * (dOffset+dOffsetDen*pow(10.0, -nDigits)) + 0.5);
			}
			else
			{
				// X/Y for example
				if( (0.0 != dOffsetDen) && (0.0 != dOffset) )
				{
//					mtx.s.T[nRow] = floor(STBF * nSign * (dOffset/dOffsetDen) + 0.5);
					mtx.s.T[nRow] = floor(nMulT * nSign * (dOffset/dOffsetDen) + 0.5);
				}
			}
			while( mtx.s.T[nRow] < 0 )
				mtx.s.T[nRow] += nMulT;
//				mtx.s.T[nRow] += STBF;
//			mtx.s.T[nRow] %= STBF;
			mtx.s.T[nRow] %= nMulT;
			nRow++;
			// initialize values
			nSign = 1;
			nX = nY = nZ = 0;
			dOffset = dOffsetDen = 0.0;
			bFirstNum = true;
			bRealOff = false;
			nDigits = 0;
			bHasScale = false;
			cCurSymb = TCHAR('\0');
			break;
		case TCHAR('x'):
		case TCHAR('X'):
			cCurSymb = TCHAR('x');
			nX = nSign*1;
			if( dOffset != 0 )
				nX *= dOffset;
			break;
		case TCHAR('y'):
		case TCHAR('Y'):
			cCurSymb = TCHAR('y');
			nY = nSign*1;
			if( dOffset != 0 )
				nY *= dOffset;
			break;
		case TCHAR('z'):
		case TCHAR('Z'):
			cCurSymb = TCHAR('z');
			nZ = nSign*1;
			if( dOffset != 0 )
				nZ *= dOffset;
			break;
		case TCHAR('+'):
			nSign = 1;
			break;
		case TCHAR('-'):
			nSign = -1;
			break;
		case TCHAR('0'):
		case TCHAR('1'):
		case TCHAR('2'):
		case TCHAR('3'):
		case TCHAR('4'):
		case TCHAR('5'):
		case TCHAR('6'):
		case TCHAR('7'):
		case TCHAR('8'):
		case TCHAR('9'):
			if( true == bFirstNum )		// first number, before '/' or '.'
			{
				dOffset = dOffset * 10.0 + (int)(*ptr - TCHAR('0'));
				if( true == bHasScale )
				{
					switch( cCurSymb )
					{
					case TCHAR('x'):
						nX *= dOffset;
						break;
					case TCHAR('y'):
						nY *= dOffset;
						break;
					case TCHAR('z'):
						nZ *= dOffset;
						break;
					}
					bHasScale = false;
				}
			}
			else
			{
				dOffsetDen = dOffsetDen * 10.0 + (int)(*ptr - TCHAR('0'));
				nDigits++;
			}
			break;
		case TCHAR('.'):
			bRealOff = true;			// we have a number like 0.00001
			bFirstNum = false;			// we need to read a second number after the '.'
			break;
		case TCHAR('/'):
			bFirstNum = false;			// we need to read a second number in 'X/Y'
			break;
		case TCHAR('*'):
			bHasScale = true;
			break;
		default:
			// skip any other symbol
			break;
		}
		ptr++;
	}
	return true;
}

void SpaceGroup::PaintSpaceGroup(CDC *pDC, CFont &Font, const CRect &WndRect)
{
//	CWnd *pWnd = GetDlgItem(nDlgItemID);
//	if( pWnd )
//	{
		// init the space group
//		SpaceGroup sg(1);
		TCHAR sgName[32], *ptr;
		CRect rc(WndRect);
		CBrush br;

//		sg.Copy(m_pWizard->m_Data.m_sg);

		// copy for temporary use
//		_tcscpy(sgName, sg.m_SgInfo.psSgLabel);
		if(NULL == m_SgInfo.psSgLabel)
		{
			TRACE(_T("SpaceGroup::PaintSpaceGroup() -> wrong space group label (NULL).\n"));
			return;
		}
		_tcscpy(sgName, m_SgInfo.psSgLabel);

		// find equal sign if it is
		if( NULL != (ptr = _tcschr(sgName, TCHAR('='))) )
		{
			// remove spaces from the right
			while( TCHAR(' ') == *ptr )
			{
				ptr--;
			}
			*ptr = 0;
		}
		// remove all '_'
		ptr = sgName;
		while( *ptr != 0 )
		{
			if( TCHAR('_') == *ptr )
			{
				*ptr = TCHAR(' ');
			}
			ptr++;
		}

//		pWnd->GetClientRect(rc);
//		pWnd->ClientToScreen(rc);
//		ScreenToClient(rc);

		// fill the background by white color
		br.CreateSolidBrush(RGB(255,255,255));
		pDC->FillRect(rc, &br);

		COLORREF nOldColor = pDC->SetTextColor(RGB(0, 0, 0));
		int nOldMode = pDC->SetBkMode(TRANSPARENT);
		CFont *pOldFont = pDC->SelectObject(&Font);
		rc.top += 2;
		pDC->DrawText(sgName, rc, DT_CENTER | DT_VCENTER);
		if( pOldFont )
		{
			pDC->SelectObject(pOldFont);
		}
		pDC->SetTextColor(nOldColor);
		pDC->SetBkMode(nOldMode);
//	}
}
/*
bool SpaceGroup::MakeCoordinates(const CVector3d &pos,
								 std::vector<CVector3d> &vCoords,
								 bool bClip) const
{
//	std::vector<CVector3d>::iterator it;
	std::vector<CVector3d> tmp;
	CVector3d cur, diff;
	SymmOpCit mit;
	bool bFound;
	size_t nCount = 0, i;

	tmp.resize(m_vSymmOps.size());
	for(mit = m_vSymmOps.begin(); mit != m_vSymmOps.end(); ++mit)
	{
		cur = (*mit) * pos;
		// clip if needed
		if( true == bClip )
		{
			while( cur.x < 0.0 ) cur.x += 1.0;
			while( cur.y < 0.0 ) cur.y += 1.0;
			while( cur.z < 0.0 ) cur.z += 1.0;
			while( cur.x >= 1.0 ) cur.x -= 1.0;
			while( cur.y >= 1.0 ) cur.y -= 1.0;
			while( cur.z >= 1.0 ) cur.z -= 1.0;
		}
		// look up coordinates
		bFound = false;
//		for(it = tmp.begin(); it != tmp.end(); ++it)
		for(i = 0; i < nCount; i++)
		{
//			diff = cur - (*it);
			diff = cur - tmp[i];
			if( (diff & diff) < 1e-8 )
			{
				bFound = true;
				break;
			}
		}
		if( false == bFound )
		{
			tmp[nCount++] = cur;
		}
	}
	vCoords.resize(nCount);
	copy(tmp.begin(), tmp.begin() + nCount, vCoords.begin());
	return true;
}
*/
void SpaceGroup::MakePositiveHKL(HKLMiller &hkl)
{
	if( hkl.x < 0 )
	{
		hkl = -hkl;
	}
	else if( 0 == hkl.x )
	{
		if( hkl.y < 0 )
		{
			hkl = -hkl;
		}
		else if( 0 == hkl.z )
		{
			if( hkl.z < 0 )
			{
				hkl = -hkl;
			}
		}
	}
}

#if 0
bool SpaceGroup::MergeReflections(TReflsVec &vReflList)
{
//#ifdef _DEBUG
//	TCHAR tmp[256];
//#endif
	TReflection refl, *pRefl;
	TReflsVecIt it;
	Eq_hkl hkl;
	int i, nEq, iList, M, mult;
	bool bFound, bFriedel;
	double OldPha, Pha, Dif1, Dif2, dSin, dCos;
	long nDval;
	HKLindex H, K, L, h, k, l, restriction;
	HKLMiller hklm;
	TReflsList new_list;
	TReflsListIt pos;
	TReflsMapIt d_lower, d_upper, dpos;
	TReflsMap MapDvalues;

	sort(vReflList.begin(), vReflList.end(), TReflection::sort_hkl());

	for(it = vReflList.begin(); it != vReflList.end(); ++it)
	{
//		H = it->hkl.x;
//		K = it->hkl.y;
//		L = it->hkl.z;
		hklm = it->hkl;
//		MakePositiveHKL(hklm);
		H = hklm.x; K = hklm.y; L = hklm.z;

		// For non-absent, i.e. permitted reflections 0 is returned.
//		iList = IsSysAbsent_hkl(H, K, L, restriction);
		iList = IsSysAbsent_hkl(hklm, restriction);
		if( 0 == iList )
		{
			// indicates that the reflection has not been found yet
			bFound = false;
			// look up if it is already in the list
			nDval = (long)::floor(it->d * 10000 + 0.5);
			d_lower = MapDvalues.lower_bound(nDval);
			d_upper = MapDvalues.upper_bound(nDval);
//			M = BuildEq_hkl(hkl, H, K, L);
			M = BuildEq_hkl(hkl, hklm);
			// copy the reflection
			refl = *it;
			refl.hkl = hkl.hkl[0];
			refl.HKL = M_2PI * refl.hkl;
//			refl.hkl = hklm;
//			refl.HKL = M_2PI * hklm;
			refl.mult = 1 | (M << 16);		// Low word - reflections count; upper word - real multiplicity
			// found?
			if( d_lower->first != nDval )
				goto Continue_Add_Refl;
			// look up the reflection in the list
			nEq = hkl.N;
//			bFound = false;
			for(i = 0; i < nEq; i++)
			{
//				h = hkl.h[i], k = hkl.k[i], l = hkl.l[i];
				h = hkl.hkl[i].x, k = hkl.hkl[i].y, l = hkl.hkl[i].z;
				bFriedel = false;
				if( h < 0 )
				{
					h = -h, k = -k, l= -l;
					bFriedel = true;
				}
				else if( (0 == h) && (k < 0) )
				{
					h = -h, k = -k, l= -l;
					bFriedel = true;
				}
				else if( (0 == h) && (0 == k) && (l < 0) )
				{
					h = -h, k = -k, l= -l;
					bFriedel = true;
				}
/*				// look up if it is already in the list
				nDval = (long)::floor(refl.d * 10000 + 0.5);
				d_lower = MapDvalues.lower_bound(nDval);
				d_upper = MapDvalues.upper_bound(nDval);
				// found?
				if( d_lower->first != nDval )
					continue;
*/
				for(dpos = d_lower; dpos != d_upper; dpos++)
				{
					// found, compare HKL indices
					pRefl = dpos->second;
					if( ((h == pRefl->hkl.x) && (k == pRefl->hkl.y) && (l == pRefl->hkl.z)) ||
						((h == -pRefl->hkl.x) && (k == -pRefl->hkl.y) && (l == -pRefl->hkl.z)) )
					{
						bFound = true;
						pRefl->Fhkl += refl.Fhkl;		// add Fhkl
						// calculate the base phase
//						OldPha = refl.Pha + 360.0f * (hkl.TH[i] / 12.0f);
						OldPha = refl.Pha + 30.0f * hkl.TH[i];
						if( bFriedel )
							OldPha = -OldPha;
//						if( OldPha < 0 )
//							OldPha += 360;
						if( OldPha > 360 )
							OldPha -= 360;
						else if( OldPha < 0 )
							OldPha += 360;
						OldPha = DEG2RAD(OldPha);
//						pRefl->A += refl.Fhkl * FastCos(OldPha);
//						pRefl->B += refl.Fhkl * FastSin(OldPha);
//						pos->Pha += OldPha;
						pRefl->mult++;				// count the reflections properly
						break;
					}
				}
			}
Continue_Add_Refl:
			// add if it was not found
			if( false == bFound )
			{
				pRefl = new TReflection;
				*pRefl = refl;
				new_list.push_back(pRefl);
				MapDvalues.insert(pair<long,TReflection*>(nDval, pRefl));
			}
		}
	}
	// update the data
	for(pos = new_list.begin(); pos != new_list.end(); ++pos)
	{
		pRefl = *pos;
		mult = pRefl->mult & 0xFFFF;
		pRefl->Fhkl /= mult;
//		pRefl->Fhkl = FastSqrt(SQR(pRefl->Ah) + SQR(pRefl->Bh)) / mult;
		pRefl->Pha = RAD2DEG(atan2(pRefl->B, pRefl->A));
		while( pRefl->Pha >= 360 )
			pRefl->Pha -= 360;
//		pRefl->Pha  /= mult;
		pRefl->mult >>= 16;			// restore the multiplicity
		iList = IsSysAbsent_hkl(pRefl->hkl.x, pRefl->hkl.y, pRefl->hkl.z, restriction);
		// If the reflection hkl has no phase restriction, *TH_Restriction is set to -1.
		// For values >= 0 the restricted phase angles
		// in degrees are given by P = (*TH_Restriction) * (180 / STBF), and P + 180,
		// respectively
		if( restriction >= 0 )
		{
			// the phase restriction
			Pha = 180 * restriction / 12.0f;
			// check where reflection's Phase is closer - to Pha or Pha+180
			Dif1 = FastAbs(pRefl->Pha - Pha);
//				if( Dif1 >= 180 )
//					Dif1 -= 180;
			Dif2 = FastAbs(pRefl->Pha - (Pha+180));
			if( Dif2 >= 180 )
				Dif2 = FastAbs(pRefl->Pha - (Pha-180));
			if( Dif1 < Dif2 )
				pRefl->Pha = Pha;
			else
				pRefl->Pha = Pha + 180;
		}
		// the phase shift
		//			pRefl->Pha -= 360.0f * (hkl.TH[0] / 12.0f);
		// clip the phase in the [0; 180] interval
		if( pRefl->Pha < 0 )
			pRefl->Pha += 360;
		if( pRefl->Pha > 180 )
			pRefl->Pha -= 360;
		FastSinCos(DEG2RAD(pRefl->Pha), dSin, dCos);
//		pRefl->A = pRefl->Fhkl * FastCos(DEG2RAD(pRefl->Pha));
//		pRefl->B = pRefl->Fhkl * FastSin(DEG2RAD(pRefl->Pha));
		pRefl->A = pRefl->Fhkl * dCos;
		pRefl->B = pRefl->Fhkl * dSin;
		//			new_list.push_back(refl);
		// add it into the list with correct phase
//#ifdef _DEBUG
//		_stprintf(tmp, _T("%f  %f  %f  %.2f  %.2f\n"),
//			pRefl->h, pRefl->k, pRefl->l, pRefl->Fhkl, pRefl->Pha);
//		OutputDebugString(tmp);
//#endif
	}
//#ifdef _DEBUG
//	_stprintf(tmp, _T("Initial list %d refls; merged into %d refls.\n"),
//		vReflList.size(), new_list.size());
//	OutputDebugString(tmp);
//#endif
	vReflList.clear();
	vReflList.resize(new_list.size());
	// copy the data
//	copy(new_list.begin(), new_list.end(), vReflList.begin());
	for(i = 0, pos = new_list.begin(); pos != new_list.end(); ++pos, i++)
	{
		pRefl = *pos;
		vReflList[i] = *pRefl;
		delete pRefl;
	}
	return true;
}

bool SpaceGroup::ExpandReflections(TReflsVec &vReflList)
{
	TReflection refl;
	TReflsVecIt it;
	TReflsVec new_list;
	Eq_hkl hkl;
	int i, nEq, iList, M;
	HKLindex H, K, L, h, k, l, restriction;
	double BasPha, dSin, dCos;
	bool bFriedel;

	if( false == MergeReflections(vReflList) )
	{
		return false;
	}

	for(it = vReflList.begin(); it != vReflList.end(); ++it)
	{
		H = it->hkl.x;
		K = it->hkl.y;
		L = it->hkl.z;
		// For non-absent, i.e. permitted reflections 0 is returned.
		iList = IsSysAbsent_hkl(it->hkl, restriction);
		if( 0 == iList )
		{
			M = BuildEq_hkl(hkl, it->hkl);
			// copy the reflection
			refl = *it;
			// If the reflection hkl has no phase restriction, *TH_Restriction is set to -1.
			// For values >= 0 the restricted phase angles
			// in degrees are given by P = (*TH_Restriction) * (180 / STBF), and P + 180,
			// respectively
			BasPha = refl.Pha;
			if( refl.Pha < 0 )
				refl.Pha += 360;
			else if( refl.Pha > 180 )
				refl.Pha -= 360;
/*			// copy to the list
			FastSinCos(DEG2RAD(refl.Pha), dSin, dCos);
			refl.A = refl.Fhkl * dCos;
			refl.B = refl.Fhkl * dSin;
			new_list.push_back(refl);
			nEq = hkl.N;
			for(i = 1; i < nEq; i++)
*/
			nEq = hkl.N;
			for(i = 0; i < nEq; i++)
			{
//				h = hkl.h[i], k = hkl.k[i], l = hkl.l[i];
				h = hkl.hkl[i].x, k = hkl.hkl[i].y, l = hkl.hkl[i].z;
				bFriedel = false;
				if( h < 0 )
				{
					h = -h, k = -k, l= -l;
					bFriedel = true;
				}
				else if( (0 == h) && (k < 0) )
				{
					h = -h, k = -k, l= -l;
					bFriedel = true;
				}
				else if( (0 == h) && (0 == k) && (l < 0) )
				{
					h = -h, k = -k, l= -l;
					bFriedel = true;
				}
				// phase shift
//				refl.Pha = BasPha - 360.0f * (hkl.TH[i] / 12.0f);
				refl.Pha = BasPha - 30.0f * hkl.TH[i];
				if( bFriedel )
					refl.Pha = -refl.Pha;
				if( refl.Pha < 0 )
					refl.Pha += 360;
				else if( refl.Pha > 180 )
					refl.Pha -= 360;
//				Pha = DEG2RAD(refl.Pha);
//				refl.Ah = refl.Fhkl * FastCos(Pha);
//				refl.Bh = refl.Fhkl * FastSin(Pha);
				FastSinCos(DEG2RAD(refl.Pha), dSin, dCos);
				refl.A = refl.Fhkl * dCos;
				refl.B = refl.Fhkl * dSin;

				refl.hkl.x = h;
				refl.hkl.y = k;
				refl.hkl.z = l;

				refl.HKL = M_2PI * refl.hkl;

				new_list.push_back(refl);
			}
		}
	}
	vReflList.clear();
	vReflList = new_list;
	return true;
}
#endif // #if 0

int SpaceGroup::AreSymmEquivalent(HKLindex h1, HKLindex k1, HKLindex l1,
								  HKLindex h2, HKLindex k2, HKLindex l2,
								  bool bCheckFriedel/* = true*/)
{
	HKLMiller hkl1(h1, k1, l1), hkl2(h2, k2, l2);
	return AreSymmEquivalent(hkl1, hkl2, bCheckFriedel);
}

// Check Friedel is required ONLY for HOLZ for 2D images.
// since their Friedel pairs come on the opposite side of reciprocal space and
// thus cannot be seen.
int SpaceGroup::AreSymmEquivalent(const HKLMiller &hkl1, const HKLMiller &hkl2,
								  bool bCheckFriedel/* = true*/)
{
	if( false == m_bInitialized )
	{
		// not found
		return 0;
	}
	// TODO:
	int iList;
	HKLindex mh2, mk2, ml2, hm, km, lm;
	HKLindex h1 = hkl1.x, k1 = hkl1.y, l1 = hkl1.z;
	HKLindex h2 = hkl2.x, k2 = hkl2.y, l2 = hkl2.z;
	SG_MATRIX *pMtx = m_SgInfo.Matrices;
	char *pPtr;

	// Friedel pair
	mh2 = -hkl2.x;
	mk2 = -hkl2.y;
	ml2 = -hkl2.z;
	// check list of symmetry operations 

	for(iList = 0; iList < m_SgInfo.nList; iList++, pMtx++)
	{
		pPtr = pMtx->s.R;
		hm = pPtr[0] * h1 + pPtr[3] * k1 + pPtr[6] * l1;
		km = pPtr[1] * h1 + pPtr[4] * k1 + pPtr[7] * l1;
		lm = pPtr[2] * h1 + pPtr[5] * k1 + pPtr[8] * l1;
		// compare indices
		if( (h2 == hm) && (k2 == km) && (l2 == lm) )
		{
			return (iList + 1);
		}
		// Friedel pair
		else/* if( true == bCheckFriedel )*/
		{
			if( (mh2 == hm) && (mk2 == km) && (ml2 == lm) )
			{
				return -(iList + 1);
			}
		}
	}
	// not found
	return 0;
//	Eq_hkl list;

	// returns multiplicity of hkl
//	if( 0 == (M = BuildEq_hkl(&list, h, hkl1.k, hkl1.l)) )
//		return false;
/*
	List.resize(list.N);
	for(i = 0; i < list.N; i++)
	{
		List[i].h = list.h[i];
		List[i].k = list.k[i];
		List[i].l = list.l[i];
		List[i].M = list.N;
	}
	return true;
*/
}

int SpaceGroup::AreSymmEnhanced(const HKLMiller &hkl)
{
	return AreSymmEnhanced(hkl.x, hkl.y, hkl.z);
}

int SpaceGroup::AreSymmEnhanced(HKLindex h, HKLindex k, HKLindex l)
{
	if(false == m_bInitialized)
		return false;

//	SG_MATRIX *pMtx = m_SgInfo.Matrices;
//	char *pM, *pT;
//	bool bContinue = true;
	int iSE;//, iList;
	HKLindex tr;//, hm, km, lm, dMul;
	HKLMiller hkl(h, k, l), hklm(0);
	SymmOpIt mit;
	CMatrix3x3 R;
	CVector3d T;

	// check list of symmetry operations 
	iSE = 0;
	for(mit = m_vSymmOps.begin(); mit != m_vSymmOps.end(); ++mit)
	{
		R = mit->GetRotation();
		T = mit->GetTranslation();
		hklm = hkl * R;
		tr = hkl.x * T.x + hkl.y * T.y + hkl.z * T.z;
		while( tr >= 1 ) tr -= 1;
		while( tr < 0 ) tr += 1;
		// compare indices
		if ( (hkl == hklm) && (0 == tr) )
		{
			iSE++;
		}
	}
	return iSE;
/*	iSE = 0;
	dMul = 1;
//	while( bContinue )
	{
Continue_Centric:
		for(iList = 0; iList < m_SgInfo.nList; iList++, pMtx++)
		{
			pM = pMtx->s.R;
			pT = pMtx->s.T;
			hm = dMul*(pM[0] * h + pM[3] * k + pM[6] * l);
			km = dMul*(pM[1] * h + pM[4] * k + pM[7] * l);
			lm = dMul*(pM[2] * h + pM[5] * k + pM[8] * l);
			tr = pT[0] * h + pT[1] * k + pT[2] * l;
			while( tr >= STBF )
				tr -= STBF;
			while( tr < 0 )
				tr += STBF;
			// compare indices
			if ( (h == hm) && (k == km) && (l == lm) && (0 == tr) )
			{
				iSE++;
			}
		}
		if( (true == bContinue) && (0 != m_SgInfo.nCentric) )
		{
			dMul = -1;
			bContinue = false;
			goto Continue_Centric;
		}
//		bContinue = false;
	}
	return iSE;
*/
}
int *SpaceGroup::GetTrVectors() const
{
	// the look up table
	static int *lut[] =
	{
		TrVectorsA, TrVectorsB, TrVectorsC,
		0, 0, TrVectorsF, 0, 0, TrVectorsI,
		0, 0, 0, 0, 0, 0, TrVectorsP, 0,
		TrVectorsR, TrVectorsS, TrVectorsT
	};
	if( (m_SgInfo.cCode >= TCHAR('A')) &&
		(m_SgInfo.cCode <= TCHAR('T')) )
	{
		return lut[m_SgInfo.cCode-TCHAR('A')];
	}
	return NULL;
}

void SpaceGroup::GetMaxHKL(double dMinD, double ar, double br, double cr,
						   HKLindex &MaxH, HKLindex &MaxK, HKLindex &MaxL)
{
	MaxH = (HKLindex)::floor(1.0 / (dMinD * ar));
	MaxK = (HKLindex)::floor(1.0 / (dMinD * br));
	MaxL = (HKLindex)::floor(1.0 / (dMinD * cr));
}

void SpaceGroup::SetListMin_hkl(HKLindex Maxk, HKLindex Maxl,
								HKLindex &Minh, HKLindex &Mink, HKLindex &Minl)
{
	Minh = 0;

	switch( m_SgInfo.nXtalSystem )
	{
	case XS_Triclinic:
		Mink = -Maxk;
		Minl = -Maxl;
		break;
	case XS_Monoclinic:
		if( (TCHAR)'z' == m_SgInfo.nUniqueRefAxis )
		{
			Mink = -Maxk;
			Minl = 0;
		}
		else
		{
			Mink = 0;
			Minl = -Maxl;
		}
		break;
	default:
		if( (XS_Trigonal == m_SgInfo.nXtalSystem) &&
			((TCHAR)'*' == m_SgInfo.nUniqueDirCode) )
			Mink = -Maxk;
		else
			Mink = 0;
		Minl = 0;
		break;
	}
}

int SpaceGroup::IsSysAbsent_hkl(HKLindex h, HKLindex k, HKLindex l, HKLindex &THr) const
{
	HKLMiller hkl(h, k, l);
	return IsSysAbsent_hkl(hkl, THr);
}

int SpaceGroup::IsSysAbsent_hkl(const HKLMiller &hkl, HKLindex &THr) const
{
	int           iTrV, nTrV;
	const int     *TrV, *pTrVecs;
	int           iList;
	HKLindex	  mh, mk, ml, hm, km, lm, TH;
	HKLindex	  h = -hkl.x, k = -hkl.y, l = -hkl.z;
	int           FlagMismatch;
	const SG_MATRIX *lsmx;
	int iRet = 0;

	mh = -h;
	mk = -k;
	ml = -l;

	// check list of symmetry operations
	// take care of lattice type and "centric" flag
	THr = -1;
//	if (TH_Restriction != NULL) *TH_Restriction = THr;
	FlagMismatch = 0;

	// fast access pointers and constants
	nTrV = m_SgInfo.nTrVector;
	lsmx = m_SgInfo.Matrices;
	pTrVecs = GetTrVectors();

	for(iList = 0; iList < m_SgInfo.nList; iList++, lsmx++)
	{
		hm = lsmx->s.R[0] * h + lsmx->s.R[3] * k + lsmx->s.R[6] * l;
		km = lsmx->s.R[1] * h + lsmx->s.R[4] * k + lsmx->s.R[7] * l;
		lm = lsmx->s.R[2] * h + lsmx->s.R[5] * k + lsmx->s.R[8] * l;

		TrV = pTrVecs;//GetTrVectors();

		for (iTrV = 0; iTrV < nTrV; iTrV++)
		{
			TH =  (lsmx->s.T[0] + *TrV++) * h;
			TH += (lsmx->s.T[1] + *TrV++) * k;
			TH += (lsmx->s.T[2] + *TrV++) * l;
			TH = TH - STBF * (int)(TH / STBF);
//			TH %= STBF;
			if (TH < 0) TH += STBF;

			if( mh == hm && mk == km && ml == lm )
			{
				if (TH != 0 && m_SgInfo.nCentric == -1)
					return -(iList + 1 + iTrV * m_SgInfo.nList);

				if (THr < 0)
					THr = TH;
				else if (THr != TH)
					FlagMismatch = 1; // must be systematic absent
				// will check later ...
			}
			else if ( h == hm &&  k == km &&  l == lm)
			{
				if (TH != 0)
					return  (iList + 1 + iTrV * m_SgInfo.nList);
			}
			else
				break;
		}
	}

	if (THr >= 0 && FlagMismatch) // ... consistency check
	{
		iRet = -1;
//		SetSgError(IErr_Inc_SymMx);
	}

//	if (TH_Restriction != NULL)
//	{
		if( -1 == m_SgInfo.nCentric )
			THr = 0;
//		else
//			*TH_Restriction = THr;
//	}

	return iRet;
}

int SpaceGroup::IsSuppressed_hkl(HKLindex Minh, HKLindex Mink, HKLindex Minl,
								 HKLindex Maxk, HKLindex Maxl,
								 HKLindex h, HKLindex k, HKLindex l)
{
	int     iList, mate;
	HKLindex	hm, km, lm;
	SG_MATRIX  *lsmx;

	// check for Friedel mate first
	hm = -h, km = -k, lm = -l;

	if( (Minh <= hm && hm <=    h) &&
		(Mink <= km && km <= Maxk) &&
		(Minl <= lm && lm <= Maxl) )
	{
		if (hm < h)
			return -1;
		else // if (h == 0)
		{
			if (km < k)
				return -1;
			else
			{
				if (k == 0)
				if (lm < l) return -1;
			}
		}
	}

	lsmx = &m_SgInfo.Matrices[1]; // skip first = identity matrix

	for(iList = 1; iList < m_SgInfo.nList; iList++, lsmx++)
	{
		// check if equivalent hm, km, lm are inside loop range ...

		hm = lsmx->s.R[0] * h + lsmx->s.R[3] * k + lsmx->s.R[6] * l;
		km = lsmx->s.R[1] * h + lsmx->s.R[4] * k + lsmx->s.R[7] * l;
		lm = lsmx->s.R[2] * h + lsmx->s.R[5] * k + lsmx->s.R[8] * l;

		for (mate = 0; mate < 2; mate++)
		{
			if (mate) hm = -hm, km = -km, lm = -lm; // ... or Friedel mate

			if (   Minh <= hm && hm <=    h
				&& Mink <= km && km <= Maxk
				&& Minl <= lm && lm <= Maxl)
			{
				// ... and were processed before

				if (hm < h)
					return (mate ? -(iList + 1) : iList + 1);
				else /* if (hm == h) */
					if (km < k)
						return (mate ? -(iList + 1) : iList + 1);
					else if (km == k)
						if (lm < l)
							return (mate ? -(iList + 1) : iList + 1);
			}
		}
	}

	return 0;
}

bool SpaceGroup::BuildHklList(HKLvec &List, HKLindex MaxH, HKLindex MaxK, HKLindex MaxL,
							  const HKLMiller &zone, double *padCellParams/*= NULL*/)
{
	UnitCell::CUnitCell *pCell = UnitCell::CUnitCell::CreateUnitCell(UnitCell::CELL_3D);
	bool bCheckZoneAxis, bFound;
	Eq_hkl		tmp_hkl;
	HKLindex	h, k, l, restriction;
	int			i, iList;
	HKLindex	Minh, Mink, Minl;
	HKL_index	hkl, *pHKL;
	HKLvecIt	it;
	double		d;
	HKLlist		new_list;
	HKLlistIt	pos;

	typedef multimap<long, HKL_index*> MapD;
	typedef multimap<long, HKL_index*>::iterator MapDit;
	MapDit d_lower, d_upper, dpos;
	MapD MapDvalues;
	long nDval;

	if(false == m_bInitialized)
		return false;

	if( NULL != padCellParams )
	{
		pCell->SetParameters(padCellParams, UnitCell::RADIANS);
	}

	bCheckZoneAxis = false;
	if( (zone.x != 0) || (zone.y != 0) || (zone.z != 0) )
	{
		bCheckZoneAxis = true;
		Minh = -MaxH;
		Mink = -MaxK;
		Minl = -MaxL;
	}
	else
	{
		SetListMin_hkl(MaxK, MaxL, Minh, Mink, Minl);
	}

	for(h = Minh; h <= MaxH; h++)
	{
		for(k = Mink; k <= MaxK; k++)
		{
			for(l = Minl; l <= MaxL; l++)
			{
				// check the zone axis
				if( bCheckZoneAxis )
				{
					if( zone.x * h + zone.y * k + zone.z * l != 0 )
						continue;
				}
				iList = IsSysAbsent_hkl(h, k, l, restriction);
				if( -1 == iList )
					return false;
				// we must process the reflection
				if( 0 == iList )
				{
					// returns multiplicity of hkl
					hkl.M = BuildEq_hkl(tmp_hkl, h, k, l);
					// initialize the flag
 					bFound = false;
					// if we have the unit cell - use d-spacing for fast search
					if( NULL != padCellParams )
					{
						d = pCell->CalculateH(HKLMiller(h, k, l));
						if( !((0 == h) && (0 == k) && (0 == l)) )
						{
							hkl.d = 1.0 / d;
						}
						else
						{
							hkl.d = 0;
						}
						// look up if it is already in the list
						nDval = (long)::floor(hkl.d * 10000 + 0.5);
						d_lower = MapDvalues.lower_bound(nDval);
						d_upper = MapDvalues.upper_bound(nDval);
						if( d_lower->first != nDval )
							bFound = false;
						else
						{
							// look each
							for(dpos = d_lower; dpos != d_upper; dpos++)
							{
								// is it symmetrically equivalent
								pHKL = dpos->second;
								if( 0 != AreSymmEquivalent(h, k, l, pHKL->h, pHKL->k, pHKL->l) )
								{
									bFound = true;
									break;
								}
							}
						}
					}
					else
					{
						for(it = List.begin(); it != List.end(); ++it)
						{
							if( NULL != padCellParams )
							{
								if( FastAbs(it->d - d) > 1e-4 )
									continue;
							}
							// is it symmetrically equivalent
							if( 0 != AreSymmEquivalent(h, k, l, it->h, it->k, it->l) )
							{
								bFound = true;
								break;
							}
						}
					}
					// if not found
					if( false == bFound )
					{
						pHKL = new HKL_index;
						*pHKL = hkl;
						pHKL->h = h;
						pHKL->k = k;
						pHKL->l = l;
						pHKL->eps = AreSymmEnhanced(h, k, l);
						pHKL->restriction = restriction;
						new_list.push_back(pHKL);
						MapDvalues.insert(pair<long,HKL_index*>(nDval,pHKL));
					}
				}
			}
		}
	}
	// copy them to the main list
	List.resize(new_list.size());
	for(i = 0, pos = new_list.begin(); pos != new_list.end(); ++pos, i++)
	{
		List[i] = *(*pos);
		delete *pos;
	}

	if( NULL != pCell )
	{
		delete pCell;
	}

	return true;
}

// sort HKL
struct sort_eq_hkl : binary_function<HKLMiller, HKLMiller, bool>
{
	bool operator() (const HKLMiller &hkl1, const HKLMiller &hkl2) const
	{
		if( hkl1.x < hkl2.x ) return false;
		else if( hkl1.x > hkl2.x ) return true;
		if( hkl1.y < hkl2.y ) return false;
		else if( hkl1.y > hkl2.y ) return true;
		if( hkl1.z < hkl2.z ) return false;
		else if( hkl1.z > hkl2.z ) return true;
		return true;
/*
		if( hkl1.x < hkl2.x ) return false;
		else return true;
		if( hkl1.y <= hkl2.y ) return false;
		else return true;
		if( hkl1.z <= hkl2.z ) return false;
		return true;
*/
	}
};

int SpaceGroup::BuildEq_hkl(Eq_hkl &Eq_hkl, const HKLMiller &hkl, bool bMakePositive/* = true*/) const
{
	return BuildEq_hkl(Eq_hkl, hkl.x, hkl.y, hkl.z, bMakePositive);
}

int SpaceGroup::BuildEq_hkl(Eq_hkl &Eq_hkl, HKLindex h, HKLindex k, HKLindex l,
							bool bMakePositive/* = true*/) const
{
	int			iList, i;
//	HKLindex	hm, km, lm;
	HKLMiller	hkl(h, k, l), hklm;
	SG_MATRIX	*lsmx;

	if( true == bMakePositive )
	{
		if( (h < 0) || ((0 == h) && ((k < 0) || ((0 == k) && (l < 0)))) )
		{
			hkl = -hkl;
		}
/*
		if( h < 0 )
		{
			hkl = -hkl;
		}
		else if( 0 == h )
		{
			if( k < 0 )
			{
				hkl = -hkl;
			}
			else if( 0 == k )
			{
				if( l < 0 )
				{
					hkl = -hkl;
				}
			}
		}
*/
//		h = hkl.x, k = hkl.y, l = hkl.z; // addition on [10/8/2005]
	}

	Eq_hkl.M = 1;
	Eq_hkl.N = 1;
	Eq_hkl.hkl[0] = hkl;
	Eq_hkl.TH[0] = 0;

	if( !(h || k || l) )
		return Eq_hkl.M; // this is 000

	Eq_hkl.M++;

	// check list of symmetry operations

	lsmx = &m_SgInfo.Matrices[1]; // skip first = identity matrix

	for(iList = 1; iList < m_SgInfo.nList; iList++, lsmx++)
	{
		hklm.x = lsmx->s.R[0] * h + lsmx->s.R[3] * k + lsmx->s.R[6] * l;
		hklm.y = lsmx->s.R[1] * h + lsmx->s.R[4] * k + lsmx->s.R[7] * l;
		hklm.z = lsmx->s.R[2] * h + lsmx->s.R[5] * k + lsmx->s.R[8] * l;

		// Peter's addition. Through all "-H" reflections
//		if( hm < 0 )
//			continue;

		for(i = 0; i < Eq_hkl.N; i++)
		{
			if ( (hklm == Eq_hkl.hkl[i]) || (-hklm == Eq_hkl.hkl[i]) )
			{
				break;
			}
		}
		if( i == Eq_hkl.N )
		{
			if( Eq_hkl.N >= 24 )
			{
//				SetSgError(IErr_Inc_SymMx);
				return 0;
			}
			// copy indices
			Eq_hkl.hkl[i] = hklm;

			Eq_hkl.TH[i] = (lsmx->s.T[0] * h + lsmx->s.T[1] * k + lsmx->s.T[2] * l);// % STBF;
			Eq_hkl.TH[i] = Eq_hkl.TH[i] - STBF * (int)(Eq_hkl.TH[i] / STBF);
			if (Eq_hkl.TH[i] < 0)
				Eq_hkl.TH[i] += STBF;

			Eq_hkl.M += 2;
			Eq_hkl.N++;
		}
	}

	if( m_SgInfo.nList % Eq_hkl.N ) // another error trap
	{
//		SetSgError(IErr_Inc_SymMx);
		return 0;
	}

//	sort(Eq_hkl.hkl, Eq_hkl.hkl + Eq_hkl.N, sort_eq_hkl());

	return Eq_hkl.M;
}
/*
void SpaceGroup::Convert2HermannMauguin(String &sSymbol)
{
	int nPos = sSymbol.Find(TCHAR('='));
	if( nPos > 0 )
		sSymbol = sSymbol.Right(sSymbol.GetLength()-nPos-2);
	sSymbol.Replace(TCHAR('_'), TCHAR(' '));
}
*/
LPCTSTR SpaceGroup::GetCentring(TCHAR nCode)
{
	switch(nCode)
	{
	case TCHAR('P'): return Centr_Name[BL_Primitive];
	case TCHAR('A'):
	case TCHAR('B'):
	case TCHAR('C'): return Centr_Name[BL_Face_centered];
	case TCHAR('I'): return Centr_Name[BL_Body_centered];
	case TCHAR('R'): return Centr_Name[BL_Hexagonal_obverse];
	case TCHAR('F'): return Centr_Name[BL_All_face_centered];
	}
	return S_Unknown;
}
/*
bool SpaceGroup::GetAsymmBox(AsymmUnit &ASU)
{
	const AS_UNIT *pAsuEntry = asymm_units;
	LPCTSTR pszExt = m_SgInfo.psExtension;
//	bool bFound;
	int nSgNum = m_SgInfo.nSgNum;

	UINT nExtNum = GetExtNum();
	// look for a corresponding ASU
//	bFound = false;
	while( 0 != pAsuEntry->nSgNum )
	{
		if( pAsuEntry->nSgNum == nSgNum )
		{
			while( nSgNum == pAsuEntry->nSgNum )
			{
				if( nExtNum == pAsuEntry->nSgExtra )
				{
					ASU.SetBox(pAsuEntry->brik);
//					bFound = true;
//					break;
					return true;
				}
				// next entry
				pAsuEntry++;
			}
			// check extended information also
//			if( pEntry->nSgExtra <= 23 )
//			{
//				if( 0 == _tcscmp(ExtendedSG_UniqueAxis[pEntry->nSgExtra], pszExt) )
//				{
//					break;
//				}
//			}
//		}
//		if( true == bFound )
//		{
//			break;
//		}
		// next entry
		pAsuEntry++;
	}
	// set to default (0,0,0)-(1,1,1)
	ASU.SetBox(s_DefaultASU.brik);
	return false;
}
*/
/*
bool SpaceGroup::CheckForAsu(const CVector3d &pos,
							 const AsymmUnit &ASU)
{
	return ASU.CheckForAsu(pos);
}
*/
/*
static int ParseExtension(const char *Ext, T_ExtInfo *ExtInfo)
{
	int         i, mode;
	const char  *e, *t;
	
	
	ExtInfo->OriginChoice =
		ExtInfo->CellChoice =
		ExtInfo->BasisChoice = ' ';
	ExtInfo->BT_or_UA = "";
	
	mode = 0;
	
	while (*Ext)
	{
		if      (strchr("12",   *Ext) != NULL)
		{
			ExtInfo->CellChoice   =
				ExtInfo->OriginChoice = *Ext++;
		}
		else if (strchr("3",    *Ext) != NULL)
		{
			ExtInfo->CellChoice   = *Ext++;
		}
		else if (strchr("Ss",   *Ext) != NULL)
		{
			ExtInfo->OriginChoice = '1';
			Ext++;
		}
		else if (strchr("Zz",   *Ext) != NULL)
		{
			ExtInfo->OriginChoice = '2';
			Ext++;
		}
		else if (strchr("Hh",   *Ext) != NULL)
		{
			ExtInfo->BasisChoice = 'H';
			Ext++;
		}
		else if (strchr("Rr",   *Ext) != NULL)
		{
			ExtInfo->BasisChoice = 'R';
			Ext++;
		}
		else if (mode == 0)
			mode = 1;
		
		if (mode == 2)
			break;
		
		for (i = 0; Ext_BT_or_UA[i]; i++)
		{
			for (e = Ext, t = Ext_BT_or_UA[i]; *t; e++, t++)
				if (toupper(*e) != toupper(*t))
					break;
				
				if (*t == '\0')
				{
					if      (6 <= i && i <=  8)
						i = 2 * i - 11;
					else if (9 <= i && i <= 20)
						i = 9 + ((i - 9) / 4) * 4;
					
					ExtInfo->BT_or_UA = Ext_BT_or_UA[i];
					Ext = e;
					break;
				}
		}
		
		if (mode == 0)
			break;
		
		mode = 2;
	}
	
	if (*Ext)
		return -1;
	
	return 0;
}
*/
