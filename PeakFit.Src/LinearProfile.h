// LinearProfile.h: interface for the CLinearProfile class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Profile.h"

class CLinearProfile : public CProfile
{
public:
	CLinearProfile(const CVector2f &vfBegin, const CVector2f &vfEnd, UINT nWidth = 1)
		: CProfile(), m_vfBegin(vfBegin), m_vfEnd(vfEnd)
	{
		m_nWidth = nWidth;
		Initialize();
	}
	virtual ~CLinearProfile();

	virtual bool Initialize();
	virtual void Reset();
	virtual void GetPoints(Vector &pts);
	virtual void SingleStep();

public:
	CVector2f m_vfBegin;		// the start point
	CVector2f m_vfEnd;			// the end point

private:
	CVector2f m_vfDir;			// the direction of the line
	CVector2f m_vfPerpDir;		// perpendicular direction
	CVector2f m_vfCurPt;		// current point on the profile
};
