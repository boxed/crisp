#pragma once

#include "objects.h"

#define ATHR .7				// Level for width detection
#define NPROF 30			// Maximum number of profiles for calc of preferred orrientation

enum
{
	CRDONE = 1,				// Center of the ED pattern refined
	RDFDONE	= 2,			// RDF function extracted
	PRFDONE	= 4,			// Profiles extracted
};

typedef struct
{
	double	rad;			// Arc radius in pixels
	double	io;				// Scaled intensity
	double	intens;			// Observed intensity
	double	wid;			// Arc width in pixels
	double	a0, a1;			// Arc angular coordiantes
	BOOL	flg;			// True if maximum exist
	int		arcno;			// Arc number
	int		prfno;			// Profile number
} ARC, *LPARC;

typedef struct
{
	double	xc, yc;			// Coordinates of the center point
	double	ae0;			// Angle to the first extreme
	double	ae1;			// Angle to the second extreme
	int		r;				// Radius for Center refinement routine
	IMAGE	image;			// the image
//	int		neg;			// Positive or negative print
//	LPVOID	lpImg;			// Image data
//	int		iw, ih;			// Image width and height
//	int		pix;			// pixel size in bytes
	WORD	flags;			//

	LPDOUBLE lpRDF;			// RDF data
	BOOL	bCalibrated;	// ED pattern was calibrated
	double	scale;			// Scaling factor for pix->Å
	double	rdmax;			// maximum value in RDF
	double	rdmin;			// minimum value in RDF
	int		np;				// number of pixels used in RDF calculation
	LPARC	lpArc;			// Arc structures
	int		arccnt;			// Count of rings in the list
	int		profcnt;		// Profiles conut

	LPDOUBLE lpProf[NPROF];	// Profiles data

	// Commented by Peter on 07 Mar 2006
//	POINT	p4[4];			// 4 point clicks for center/radius determination
//	int		p4cnt;			// current click number
	// End of comments by Peter on 07 Mar 2006

	// added by Peter on 21 Feb 2006
	std::vector<CVector2d> vUserClicks;	// points for the elliptical distortion or manual center
	double dDistortionDir;				// the main angle
	double dDistA;						// distortion minor axis
	double dDistB;						// distortion major axis
	bool bEstimated;					// if true then the distortion has been estimated

	HANDLE _handle;
	DWORD _id;
	// end of addition by Peter on 21 Feb 2006

} EDRD, *LPEDRD;

typedef struct
{
	double dX0;
	double dAmpl;
	double dIntens;
	double dSigma;
	double dFWHM;

} EDRD_REFL;

void UpdateCoordinatesText(HWND hDlg, OBJ* pObj, LPEDRD pEDRD);
bool Estimate1DPeakPosition(double *pProfile, int nPoints, double &pos);

#define	SetBits(m,b) m|=(b)
#define	ClrBits(m,b) m&=(~(b))
#define	AddProtocolString(pp)	SendDlgItemMessage(hDlg,IDC_EDRD_LIST,CB_INSERTSTRING, \
									(WPARAM)-1,(LPARAM)(LPSTR)pp)


#define ClearProtocol()	SendDlgItemMessage( hDlg, IDC_EDRD_LIST, CB_RESETCONTENT, 0, 0);


template <typename _Tx>
CVector2d GetMassCentre_t(const _Tx *pData, UINT nWidth, UINT nHeight, UINT nStride, UINT nThresh)
{
	UINT x, y, off;
	double sumx, sumy, sum;
	double val;
	CVector2d pos;

	off = 0;
	sumx = sumy = sum = 0.0;
	for(y = 0; y < nHeight; y++)
	{
		for(x = 0; x < nWidth; x++)
		{
			if( (val = pData[x]) < nThresh )
				continue;
			sumx += x * val;
			sumy += y * val;
			sum  += val;
		}
		pData += nStride;
	}
	pos.x = sumx / sum;
	pos.y = sumy / sum;

	return pos;
}

template<typename _Tx>
class GetMaxPeakCentre_t
{
public:
	typedef CVector2d TReturn;
	TReturn Handle(const IMAGE& inImage)
	{
		_Tx *pData = (_Tx*)inImage.pData;
		int w = inImage.w;
		int h = inImage.h;
		TReturn pos;
		int max_pos, x, y;
		_Tx *pMax = std::max_element(pData, pData + w * h );

		max_pos = pMax - pData;
		x = max_pos % w;
		y = max_pos / w;
		if( ((w>>1) == x) && ((h>>1) == y) )
		{
			_Tx *tmp_max = pMax, tmp_val = *pMax;
			*(pData+max_pos-w-1) = *(pData+max_pos-w) = *(pData+max_pos-w+1) = 0;
			*(pData+max_pos-1) = *(pData+max_pos) = *(pData+max_pos+1) = 0;
			*(pData+max_pos+w-1) = *(pData+max_pos+w) = *(pData+max_pos+w+1) = 0;
			pMax = std::max_element(pData, pData + w * h );
			if( (*pMax) * 4.0 < tmp_val )
			{
				pMax = tmp_max;
				*pMax = tmp_val;
				x = max_pos % w;
				y = max_pos / w;
				max_pos = pMax - pData;
			}
			else
			{
				max_pos = pMax - pData;
				x = max_pos % w;
				y = max_pos / w;
			}
		}
		if (y < 2)
		{
			y = 2;
		}

		if (x < 2)
		{
			x = 2;
		}
		// TODO: wrap around
		Trace(Format("%1% %2% %3%" ) % x % y % (int)pData);
		pos = GetMassCentre_t(pData + w*(y-2) + x-2, 5, 5, inImage.stride, 0.8*(*pMax) );
		pos += TReturn(float(x-2), float(y-2));
		return pos;
	}
};

CVector2d GetImageCenter(const IMAGE& inImage);