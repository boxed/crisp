// LinearProfile.cpp: implementation of the CLinearProfile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "LinearProfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLinearProfile::~CLinearProfile()
{
}

bool CLinearProfile::Initialize()
{
	// calculate the direction
	m_vfDir = m_vfEnd - m_vfBegin;
	double len = !m_vfDir;
	m_vfDir.Normalize();
	m_vfPerpDir = CVector2f(-m_vfDir.y, m_vfDir.x);
	// allocate the memory
	resize((int)floor(len + 0.5));

	// set the current point
	m_vfCurPt = m_vfBegin;

	return (m_bInitialized = true);
}
//////////////////////////////////////////////////////////////////////////////////////
//
// bool GetPoints()
// IN:	none
//
// OUT: Vector &pts - a vector, which fills with coordinates for points lies on the
//		vector perpendicular to the profile path, number of the points is m_nWidth.
//
// RETURN:	none
//
/////////////////////////////////////////////////////////////////////////////////////
void CLinearProfile::GetPoints(Vector &pts)
{
	// check the size
	if( (size_t)m_nWidth < pts.size() )
		pts.resize(m_nWidth);
	// half-width
	int len = m_nWidth >> 1;
	
	CVector2f pt = m_vfCurPt - ((float)len * m_vfPerpDir);
	for(int i = 0; i < m_nWidth; i++)
	{
		pts[i] = pt;
		pt += m_vfPerpDir;
	}
}
//////////////////////////////////////////////////////////////////////////////////////
//
// bool SingleStep() - performs a single step along the profile
// IN:	none
//
// OUT: none
//
// RETURN:	none
//
/////////////////////////////////////////////////////////////////////////////////////
void CLinearProfile::SingleStep()
{
	// do nothing
	m_vfCurPt += m_vfDir;
}

void CLinearProfile::Reset()
{
	// reset the current point
	m_vfCurPt = m_vfBegin;
}
