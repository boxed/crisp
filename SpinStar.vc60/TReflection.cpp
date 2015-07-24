// TReflection.cpp: implementation of the TReflection class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "typedefs.h"
//#include "SpinningStar/SpaceGroups.h"
#include "SpinningStar/TReflection.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static int FindGCD2(int ri, int rj)
{
	int  rk;
	if (ri < 0) ri = -ri;

	if (rj)
	{
		for (;;)
		{
			rk = ri % rj; if (rk == 0) { ri = rj; break; }
			ri = rj % rk; if (ri == 0) { ri = rk; break; }
			rj = rk % ri; if (rj == 0) {          break; }
		}
		if (ri < 0) ri = -ri;
	}

	return ri;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TReflection::TReflection()
{
	d = Theta = Fhkl = Pha = 0;
	A = B = LP = Int = 0;
	mult = 1;
	p = 1;
	eps = 0;
	Sg = 0;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
/*
bool TReflsVec::CheckReflsType(CVector3d &zone, int &nZoneLayer)
{
	TReflsVecIt rit;
	CVector3d v1, v2, v3, norm, uvw, tmp;//, tmp2;
	bool bFound;
	double val, init_val;//, det1, det2, det3;
	int gcd1, gcd2, gcd3;

	zone = CVector3d(0, 0, 1);
	nZoneLayer = 0;
	// first, we need 3 reflections (not parallel)
	if( _refls.size() < 3 )
	{
		// not enough reflections
		return false;
	}
	// assign 1st reflection
	v1 = _refls.front().hkl;
	// look for the 2nd
	bFound = false;
	for(rit = _refls.begin() + 1; rit != _refls.end(); ++rit)
	{
		// vector product
		tmp = v1 ^ rit->hkl;
		// check if its length is not zero
		if( tmp.Sqr() > 1e-4 )
		{
			v2 = rit->hkl;
			bFound = true;
			break;
		}
	}
	// found any?
	if( false == bFound )
	{
		// all reflections are on the same line
		return false;
	}
	norm = v1 ^ v2;
	// look for the 3d
	bFound = false;
	for(rit = _refls.begin() + 1; rit != _refls.end(); ++rit)
	{
		// vector products
		if( (norm & rit->hkl) > 1e-4 )
		{
			v3 = rit->hkl;
			bFound = true;
			break;
		}
	}
	// found any?
	if( true == bFound )
	{
		// this is 3D refls list
		return true;
	}

	// test u, v, 1
	gcd1 = FindGCD2(norm.x, norm.y);
	gcd2 = FindGCD2(norm.x, norm.z);
	gcd3 = FindGCD2(gcd1, gcd2);
	if( gcd3 > 1 )
	{
		norm /= gcd3;
	}
	uvw = norm;
//Finish_ZA:
	// get integers for the zone indices
	// test all reflections
	init_val = _refls.begin()->hkl & uvw;
	bFound = false;
	for(rit = _refls.begin()+1; rit != _refls.end(); ++rit)
	{
		// get fast access
		tmp = rit->hkl;
		val = tmp & uvw;
		if( init_val != val )
		{
			uvw = CVector3d(0);
			nZoneLayer = 0;
			bFound = true;
			break;
		}
	}
	// copy zone
	zone = uvw;
	nZoneLayer = init_val;
	return bFound;
}
/*
bool TReflsVec::GetZoneRefls(const CVector3d &zone, TReflsVec &vRefls,
							 bool bUseZoneLayer, int nIndexNum, int nZoneLayer)
{
	TReflection Refl;
	TReflsVecIt rit;

	// empty first
	vRefls.clear();

	// iterate through the list
	for(rit = _refls.begin(); rit != _refls.end(); ++rit)
	{
		// fast access through the reference
		CVector3d &tmp = rit->hkl;
		if( true == bUseZoneLayer )
		{
			if( nZoneLayer == (tmp & zone) )
			{
//				Refl = *rit;
//				Refl.hkl = tmp;
				vRefls.push_back(*rit);//Refl);
			}
		}
		else
		{
			if( 0 == (tmp & zone) )
			{
				vRefls.push_back(*rit);
			}
		}
	}

	return true;
}
*/
bool TReflsVec::MergeReflections(const SpaceGroup &SG, TReflsVec &vReflList,
								 bool bMergeFriedel/* = true*/)
{
	TReflection refl, *pRefl;
	TReflsVecIt it;
	SpaceGroup::Eq_hkl hkl;
	int i, nEq, iList, M, mult;
	bool bFound, bFriedel;
	double OldPha, Pha, Dif1, Dif2, dSin, dCos;
	long nDval;
	HKLindex H, K, L, h, k, l, restriction;
//	HKLMiller hklm;
	TReflsList new_list;
	TReflsListIt pos;
	TReflsMapIt d_lower, d_upper, dpos;
	TReflsMap MapDvalues;

	// sort reflections by HKL
	std::sort(_refls.begin(), _refls.end(), TReflection::sort_hkl());
	// DO NOT CLEAR THE LIST vReflList, because it can be the same as *this.

	// for all reflections
	for(it = _refls.begin(); it != _refls.end(); ++it)
	{
		// fast access
		HKLMiller &hklm = it->hkl;
//		MakePositiveHKL(hklm);
		H = hklm.x, K = hklm.y, L = hklm.z;

		// For non-absent, i.e. permitted reflections 0 is returned.
		if( 0 == (iList = SG.IsSysAbsent_hkl(hklm, restriction)) )
		{
			// indicates that the reflection has not been found yet
			bFound = false;
			// look up if it is already in the list
			nDval = (long)::floor(it->d * 10000 + 0.5);
			d_lower = MapDvalues.lower_bound(nDval);
			d_upper = MapDvalues.upper_bound(nDval);
			// make a list of equivalent reflections
			M = SG.BuildEq_hkl(hkl, hklm, bMergeFriedel);
			// copy the reflection
			refl = *it;
			refl.hkl = hkl.hkl[0];
			refl.mult = 1 | (M << 16);		// Low word - reflections count; upper word - real multiplicity
			// found?
			if( d_lower->first != nDval )
				goto Continue_Add_Refl;
			// look up the reflection in the list
			nEq = hkl.N;
			for(i = 0; i < nEq; i++)
			{
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
				// look up if it is already in the list
				for(dpos = d_lower; dpos != d_upper; dpos++)
				{
					// found, compare HKL indices
					pRefl = dpos->second;
//					if( ((h == pRefl->hkl.x) && (k == pRefl->hkl.y) && (l == pRefl->hkl.z)) ||
//						((h == -pRefl->hkl.x) && (k == -pRefl->hkl.y) && (l == -pRefl->hkl.z)) )
					if( ((h == pRefl->hkl.x) && (k == pRefl->hkl.y) && (l == pRefl->hkl.z)) ||
						((h == -pRefl->hkl.x) && (k == -pRefl->hkl.y) && (l == -pRefl->hkl.z)) )// &&
//						(true == bMergeFriedel)) )
					{
						bFound = true;
						pRefl->Fhkl += refl.Fhkl;		// add-up the |Fhkl|
						// calculate the base phase
						OldPha = refl.Pha + 30.0f * hkl.TH[i];
						if( bFriedel )
							OldPha = -OldPha;
						if( OldPha > 360 )
							OldPha -= 360;
						else if( OldPha < 0 )
							OldPha += 360;
						// convert into radians
						OldPha = DEG2RAD(OldPha);
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
				MapDvalues.insert(std::pair<long,TReflection*>(nDval, pRefl));
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
//		pRefl->Pha = RAD2DEG(FastAtan2(pRefl->B, pRefl->A));
		pRefl->Pha = RAD2DEG(atan2(pRefl->B, pRefl->A));
		while( pRefl->Pha >= 360 )
			pRefl->Pha -= 360;
//		pRefl->Pha  /= mult;
		pRefl->mult >>= 16;			// restore the multiplicity
		iList = SG.IsSysAbsent_hkl(pRefl->hkl.x, pRefl->hkl.y, pRefl->hkl.z, restriction);
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
	// clean the output list here!
	vReflList.clear();
	// resize the vector
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

/*
bool TReflsVec::ExpandReflections(const SpaceGroup &SG, TReflsVec &vReflList)
{
	TReflection refl;
	TReflsVecIt it;
	TReflsVec new_list;
	SpaceGroup::Eq_hkl hkl;
	int i, nEq, iList, M;
	HKLindex H, K, L, h, k, l, restriction;
	double BasPha, dSin, dCos;
	bool bFriedel;

	// merge first
	if( false == MergeReflections(SG, new_list) )
	{
		return false;
	}
	// clear the list
	vReflList.clear();
	// for all merged reflections
	for(it = new_list.begin(); it != new_list.end(); ++it)
	{
		H = it->hkl.x;
		K = it->hkl.y;
		L = it->hkl.z;
		// For non-absent, i.e. permitted reflections 0 is returned.
		if( 0 == (iList = SG.IsSysAbsent_hkl(it->hkl, restriction)) )
		{
			M = SG.BuildEq_hkl(hkl, it->hkl);
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
			nEq = hkl.N;
			for(i = 0; i < nEq; i++)
			{
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
				refl.Pha = BasPha - 30.0f * hkl.TH[i]; // 360 / 12
				if( bFriedel )
					refl.Pha = -refl.Pha;
				if( refl.Pha < 0 )
					refl.Pha += 360;
				else if( refl.Pha > 180 )
					refl.Pha -= 360;
//				Pha = DEG2RAD(refl.Pha);
				FastSinCos(DEG2RAD(refl.Pha), dSin, dCos);
				refl.A = refl.Fhkl * dCos;
				refl.B = refl.Fhkl * dSin;

				refl.hkl.x = h;
				refl.hkl.y = k;
				refl.hkl.z = l;

				vReflList.push_back(refl);
			}
		}
	}
//	vReflList.clear();
//	vReflList = new_list;
	return true;
}
*/
