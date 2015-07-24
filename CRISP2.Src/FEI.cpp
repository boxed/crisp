#include "StdAfx.h"

#include "objects.h"
#include "common.h"
#include "shtools.h"
#include "globals.h"
#include "const.h"
#include "commondef.h"

#ifdef __FEI__

#include "FEI.h"

typedef void (*pfnEMILOAD)(LPCTSTR pFilename,
						   LONG *pResult,
						   LPCTSTR pSeriesFile,
						   LPCTSTR pMicroscope,
						   LPCTSTR pUser,
						   LPCTSTR pMode,
						   LONG *pGunType,
						   LONG *pGunLens,
						   LONG *pWehneltIndex,
						   LONG *pSpotsize,
						   double *pHighTension,
						   double *pExtrVoltage,
						   double *pEmission,
						   double *pDefocus,
						   double *pMagnification,
						   double *pCameraLength,
						   double *pIntensity,
						   double *pObjective,
						   double *pDifLens,
						   double *pImageShiftX,
						   double *pImageShiftY,
						   double *pStageX,
						   double *pStageY,
						   double *pStageZ,
						   double *pStageA,
						   double *pStageB);

HMODULE			hEMI_FEI = NULL;			// EMI file support library
pfnEMILOAD		g_pfnEMILOAD = NULL;

extern char		TMP[65536];
extern OBJ* CRISP_CreateObject(OBJ* pParent, OBJ* OE, BOOL &bExternalObject, HWND hParent);
extern BOOL CRISP_InitObjFromImage(OBJ* pObj, SAVEIMAGE &S);
extern BOOL fioFixImageMemory( LPSAVEIMAGE I );
extern void fioReportPercent( int c, int tot, BOOL bRead );

bool FEI_Init()
{
	hEMI_FEI = ::LoadLibrary("emiextractor.dll");
	if( NULL != hEMI_FEI )
	{
		g_pfnEMILOAD = (pfnEMILOAD)::GetProcAddress(hEMI_FEI, "ExtractInfo");
	}
	else
	{
		Trace(_FN("Failed to load emiextractor library: %s"), GetLastErrorString().c_str());
	}
	return true;
}

static std::vector<DWORD> s_FEI_vDataOffs;
static std::vector<DWORD> s_FEI_vTagOffs;
static std::vector<FEI_ESD_1D_DATA> s_FEI_vData1D;
static std::vector<FEI_ESD_2D_DATA> s_FEI_vData2D;
static FEI_ESD_HEADER s_FEI_header;

bool FEI_fioCheckSERData(LPSAVEIMAGE I, HFILE hFile)
{
	FEI_ESD_DIM_ARRAY_header dim_arr_hdr;
	DWORD dwUnitsLength, dwOffset;
	long nOff;
	int nSize, iArray;

	nOff = _llseek(hFile, 0, FILE_BEGIN);
	_lread(hFile, &s_FEI_header, sizeof(s_FEI_header));
	// check the ByteOrder
	if( 0x4949 != s_FEI_header.wByteOrder )
	{
		MessageBox(NULL, "FEI SER file has invalid ByteOrder in the header", "ERROR", MB_OK);
		return false;
	}
	// check SeriesID
	if( 0x0197 != s_FEI_header.wSeriesID )
	{
		MessageBox(NULL, "FEI SER file has invalid SeriesID in the s_FEI_header", "ERROR", MB_OK);
		return false;
	}
	// check SeriesVer
	if( 0x0210 != s_FEI_header.wSeriesVer )
	{
		MessageBox(NULL, "FEI SER file has invalid SeriesVersion in the header", "ERROR", MB_OK);
		return false;
	}
	if( 0 == s_FEI_header.dwTotalElements )
	{
		MessageBox(NULL, "FEI SER file has zero total number of entries", "ERROR", MB_OK);
		return false;
	}
	if( 0 == s_FEI_header.dwValidNumElements )
	{
		MessageBox(NULL, "FEI SER file has zero valid number of entries", "ERROR", MB_OK);
		return false;
	}
	// read the Dimension array
	nSize = sizeof(FEI_ESD_DIM_ARRAY_header);
	if( nSize != _lread(hFile, &dim_arr_hdr, nSize) )
	{
		MessageBox(NULL, "FEI SER file seems to be damaged", "ERROR", MB_OK);
		goto Error_Exit;
	}
	// read the description string first
	nSize = sizeof(TMP) - 1;
	if( dim_arr_hdr.dwDescrLength >= nSize )
	{
		_lread(hFile, TMP, nSize);
		// skip the rest of the data
		nOff = _llseek(hFile, dim_arr_hdr.dwDescrLength - nSize, SEEK_CUR);
	}
	else
	{
		nSize = dim_arr_hdr.dwDescrLength;
		_lread(hFile, TMP, nSize);
	}
	TMP[nSize] = 0;
	// read the value of UnitsLength
	_lread(hFile, &dwUnitsLength, 4);
	if( dwUnitsLength > 0 )
	{
		// read the Units Length string finally
		nSize = sizeof(TMP) - 1;
		if( dwUnitsLength >= nSize )
		{
			_lread(hFile, TMP, nSize);
			// skip the rest of the data
			nOff = _llseek(hFile, dwUnitsLength - nSize, SEEK_CUR);
		}
		else
		{
			nSize = dwUnitsLength;
			_lread(hFile, TMP, nSize);
		}
		TMP[nSize] = 0;
	}
	// seek to the data
	nOff = _llseek(hFile, s_FEI_header.dwOffsetArray, SEEK_SET);
	{
		nSize = s_FEI_header.dwTotalElements;
		s_FEI_vDataOffs.resize(nSize);
		s_FEI_vTagOffs.resize(nSize);
		_lread(hFile, &s_FEI_vDataOffs[0], 4*nSize);
		_lread(hFile, &s_FEI_vTagOffs[0], 4*nSize);
		nSize = s_FEI_header.dwValidNumElements;
		s_FEI_vData1D.resize(nSize);
		s_FEI_vData2D.resize(nSize);
		if( ESD_1D_ARRAY == s_FEI_header.dwDataTypeID )
		{
			for(iArray = 0; iArray < nSize; iArray++)
			{
				nOff = _llseek(hFile, s_FEI_vDataOffs[iArray], SEEK_SET);
				_lread(hFile, &s_FEI_vData1D[iArray], sizeof(s_FEI_vData1D[0]));
			}
		}
		else if( ESD_2D_ARRAY == s_FEI_header.dwDataTypeID )
		{
			int nRead = sizeof(s_FEI_vData2D[0]);
			for(iArray = 0; iArray < nSize; iArray++)
			{
				dwOffset = s_FEI_vDataOffs[iArray];
				if( -1 == (nOff = _llseek(hFile, dwOffset, SEEK_SET)) )
				{
					MessageBox(NULL, "FEI SER file seems to be damaged (3)", "ERROR", MB_OK);
					goto Error_Exit;
				}
				if( nRead != _lread(hFile, &s_FEI_vData2D[iArray], nRead) )
				{
					MessageBox(NULL, "FEI SER file seems to be damaged (2)", "ERROR", MB_OK);
					goto Error_Exit;
				}
			}
		}
	}
	return true;

Error_Exit:
	return false;
}

bool FEI_fioCheckEMIData(LPSAVEIMAGE I, HFILE hFile)
{
	return true;
}

bool FEI_fioReadSER(OBJ* pObj, SAVEIMAGE &I, std::vector<OBJ*> &vObjects, HFILE hFile)
{
	long nOff;
	int iArray;
	OBJ* pCurObj = NULL;
	BOOL bExternalObj = FALSE;

	if( ESD_2D_ARRAY != s_FEI_header.dwDataTypeID )
	{
		MessageBox(NULL, "Only 2D data can be read into CRISP", "ERROR", MB_OK);
		goto Error_Exit;
	}
	for(iArray = 0; iArray < s_FEI_header.dwValidNumElements; iArray++)
	{
		if( 0 == iArray )
		{
			pCurObj = pObj;
		}
		else
		{
			// create a new object
			if( NULL == (pCurObj = CRISP_CreateObject(pObj, NULL, bExternalObj, pObj->ParentWnd)) )
				goto Error_Exit;
		}
		if( s_FEI_header.dwValidNumElements > 1 )
		{
			sprintf(pCurObj->title + strlen(pCurObj->title), " [%d]", iArray+1);
		}
		FEI_ESD_2D_DATA &data2D = s_FEI_vData2D[iArray];
		I.x = data2D.dwArraySizeX;
		I.y = data2D.dwArraySizeY;
		if( (ESD_DATA_UBYTE == data2D.wDataType) ||
			(ESD_DATA_BYTE == data2D.wDataType) )
		{
			I._pix = PIX_BYTE;
			I.ps = 1;
		}
		else if( (ESD_DATA_UINT2 == data2D.wDataType) ||
			(ESD_DATA_INT2 == data2D.wDataType) )
		{
			I._pix = PIX_WORD;
			I.ps = 2;
		}
		else
		{
			MessageBox(NULL, "Only 1 byte/2 bytes data type are supported", "ERROR", MB_OK);
			free( pCurObj );
			continue;
		}
		// fix the memory
		I.lpData = NULL;
		if( !fioFixImageMemory(&I) )
			goto Error_Exit;
		nOff = s_FEI_vDataOffs[iArray] + sizeof(s_FEI_vData2D[0]);
		nOff = _llseek(hFile, nOff, SEEK_SET);
		for(int y = 0; y < I.y; y++)
		{
#ifdef USE_XB_LENGTH
			if( I.xb*I.ps != _lread(hFile, I.lpData+I.xb*y*I.ps, I.xb*I.ps) )
#else
			if( I.stride*I.ps != _lread(hFile, I.lpData+I.stride*y*I.ps, I.stride*I.ps) )
#endif
			{
				return false;
			}
			fioReportPercent(y, I.y - 1, TRUE);
		}
		CRISP_InitObjFromImage(pCurObj, I);
		vObjects.push_back(pCurObj);
		// zero the progress bar
		fioReportPercent(0, 0, TRUE);
	}
	return true;

Error_Exit:
	return false;
}

bool FEI_fioReadEMI(LPSTR fname, OBJ* pObj, SAVEIMAGE &I, std::vector<OBJ*> &vObjects, HFILE hFile)
{
	bool bRes = false;
	if( NULL != g_pfnEMILOAD )
	{
		char drv[_MAX_DRIVE], dir[_MAX_DIR], file[_MAX_DIR], ext[_MAX_EXT];
		char pFilename[_MAX_PATH], pSeriesFile[256], pMicroscope[256], pUser[256], pMode[256];
		LONG pResult, pGunType, pGunLens, pWehneltIndex, pSpotsize;
		double pHighTension, pExtrVoltage, pEmission, pDefocus;
		double pMagnification, pCameraLength, pIntensity, pObjective;
		double pDifLens, pImageShiftX, pImageShiftY, pStageX;
		double pStageY, pStageZ, pStageA, pStageB;

//		sprintf(pFilename, "d:\\200 0_25 spot 10.emi");
		memset(pFilename, 0, 256);
		strncpy(pFilename, fname, 255);

		// secured dll-function call
		try
		{
			g_pfnEMILOAD(pFilename, &pResult, pSeriesFile, pMicroscope, pUser, pMode,
				&pGunType, &pGunLens, &pWehneltIndex, &pSpotsize,
				&pHighTension, &pExtrVoltage, &pEmission, &pDefocus, &pMagnification,
				&pCameraLength, &pIntensity, &pObjective, &pDifLens, &pImageShiftX,
				&pImageShiftY, &pStageX, &pStageY, &pStageZ, &pStageA, &pStageB);
			// now we must open SER file if possible
			_splitpath( fname, drv, dir, file, ext );
			_makepath(pFilename, drv, dir, pSeriesFile, NULL);
			if( HFILE_ERROR == (hFile = _lopen(pFilename, OF_READ)) )
			{
				return false;
			}
			else
			{
				std::vector<OBJ*>::iterator cit;
				FEI_fioCheckSERData(&I, hFile);
				bRes = FEI_fioReadSER(pObj, I, vObjects, hFile);
				if( pCameraLength > 0 )
				{
					for(cit = vObjects.begin(); cit != vObjects.end(); ++cit)
					{
						pObj->NI.CamLength = pCameraLength;
					}
				}
//				pObj->NI.
			}
		}
		catch (std::exception& e)
		{
			Trace(_FN("FEI_fioReadEMI() -> message %s: %s"), message, e.what()));
		}
	}
	if( 0 != hFile )
	{
		_lclose(hFile);
		hFile = 0;
	}
	return bRes;
}

#endif // __FEI__
