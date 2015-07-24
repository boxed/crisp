#pragma once

#include <vector>
#include <string>

#include "objects.h"
//#include "common.h"
#include "shtools.h"

#ifdef __FEI__

bool FEI_Init();

bool FEI_fioCheckSERData(LPSAVEIMAGE I, HFILE hFile);
bool FEI_fioCheckEMIData(LPSAVEIMAGE I, HFILE hFile);

bool FEI_fioReadEMI(LPSTR fname, OBJ* pObj, SAVEIMAGE &I, std::vector<OBJ*> &vObjects, HFILE hFile);
bool FEI_fioReadSER(OBJ* pObj, SAVEIMAGE &I, std::vector<OBJ*> &vObjects, HFILE hFile);

typedef enum
{
	ESD_1D_ARRAY = 0x4120,		// data elements are 1D arrays
	ESD_2D_ARRAY = 0x4122,		// data elements are 2D arrays

} ESD_SERIES_TYPE_ID;

typedef enum
{
	ESD_TIME_TAG = 0x4152,		// tag is time only
	ESD_TIME_POS_ARRAY = 0x4142,// tag is 2D position with time

} ESD_TAG_TYPE_ID;

typedef enum
{
	ESD_DATA_UBYTE	= 1,
	ESD_DATA_UINT2	= 2,
	ESD_DATA_UINT4	= 3,
	ESD_DATA_BYTE	= 4,
	ESD_DATA_INT2	= 5,
	ESD_DATA_INT4	= 6,
	ESD_DATA_FLT4	= 7,
	ESD_DATA_FLT8	= 8,
	ESD_DATA_CMPLX4	= 9,
	ESD_DATA_CMPLX8	= 10,

} ESD_DATA_TYPE_ID;

#pragma pack(push, 1)

typedef struct
{
	WORD	wByteOrder;			// 'II' = 0x4949, little endian (PC)
	WORD	wSeriesID;			// 0x0197 - indicates the ES version of the file
	WORD	wSeriesVer;			// version of the series data file (right now is 0x0210)
	DWORD	dwDataTypeID;		// series type, one of ESD_SERIES_TYPE_ID
	DWORD	dwTagTypeID;		// the type of tag in the series, one of ESD_TAG_TYPE_ID
	DWORD	dwTotalElements;	// total number of data elements and tags = width*height
	DWORD	dwValidNumElements;	// valid number of elements and tags, if not all data
								// were written then can be < dwTotalElements
	DWORD	dwOffsetArray;		// absolute offset (in bytes) in the ESD file
	DWORD	dwNumberDims;		// number of dimensions of the series data file

} FEI_ESD_HEADER;

typedef struct
{
	DWORD	dwDimSize;			// number of elements in this dimension
	double	dCalibrOffset;		// calibration values at element dwCalibrElement
	double	dCalibrDelta;		// calibration delta between elements of the series
	DWORD	dwCalibrElement;	// the element in the series which has a calibration values of CalibrOffset
	DWORD	dwDescrLength;		// the length of the description string
//	char	*pszDescription;	// sizeof Description length
//	DWORD	dwUnitLength;
//	char	*pszUnits;

} FEI_ESD_DIM_ARRAY_header;

class FEI_ESD_DIM_ARRAY
{
public:
	FEI_ESD_DIM_ARRAY(const FEI_ESD_DIM_ARRAY_header &hdr, LPTSTR pszDescr, DWORD _dwUnitLength)
		: sDescription(pszDescr)
	{
		dwDimSize = hdr.dwDimSize;
		dCalibrOffset = hdr.dCalibrOffset;
		dCalibrDelta = hdr.dCalibrDelta;
		dwCalibrElement = hdr.dwCalibrElement;
		dwDescrLength = hdr.dwDescrLength;
		dwUnitLength = _dwUnitLength;
	}
	~FEI_ESD_DIM_ARRAY()
	{
		std::swap(sDescription, _tstd::string(""));
		std::swap(sUnits, _tstd::string(""));
	}
	DWORD	dwDimSize;			// number of elements in this dimension
	double	dCalibrOffset;		// calibration values at element dwCalibrElement
	double	dCalibrDelta;		// calibration delta between elements of the series
	DWORD	dwCalibrElement;	// the element in the series which has a calibration values of CalibrOffset
	DWORD	dwDescrLength;		// the length of the description string
	std::string	sDescription;	// sizeof Description length
	DWORD	dwUnitLength;
	std::string	sUnits;
//protected:
	int _destroy(/*FEI_ESD_DIM_ARRAY *ptr*/)
	{
		delete this;
		return 0;
	}
};

typedef std::vector<FEI_ESD_DIM_ARRAY*>					DataArrayVec;
typedef std::vector<FEI_ESD_DIM_ARRAY*>::iterator		DataArrayVecIt;
typedef std::vector<FEI_ESD_DIM_ARRAY*>::const_iterator	DataArrayVecCit;

typedef struct
{
	double	dCalibrOffset;
	double	dCalibrDelta;
	DWORD	dwCalibrElement;
	WORD	wDataType;			// one of ESD_DATA_TYPE_ID
	DWORD	dwArrayLength;		// the number of elements in the array
//	data values follow

} FEI_ESD_1D_DATA;

typedef struct
{
	double	dCalibrOffsetX;
	double	dCalibrDeltaX;
	DWORD	dwCalibrElementX;
	double	dCalibrOffsetY;
	double	dCalibrDeltaY;
	DWORD	dwCalibrElementY;
	WORD	wDataType;			// one of ESD_DATA_TYPE_ID
	DWORD	dwArraySizeX;		// the number of elements in the array along X
	DWORD	dwArraySizeY;		// the number of elements in the array along Y
//	data values follow

} FEI_ESD_2D_DATA;

typedef struct
{
	WORD	wTagTypeID;			// right now is 0x4152
	float	fTime;				// the time in ANSI standard (secs since 1 Jan 1970)

} FEI_ESD_TIME_TAG;

typedef struct
{
	WORD	wTagTypeID;			// right now is 0x4152
	float	fTime;				// the time in ANSI standard (secs since 1 Jan 1970)
	double	dPositionX;			// the position of the tag in X & Y directions
	double	dPositionY;

} FEI_ESD_TIME_POS_TAG;

#pragma pack(pop)

#endif // __FEI__
