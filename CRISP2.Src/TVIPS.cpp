#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
#include "rgconfig.h"

// Added by Peter on 24 February 2006
#include "Crash.h"

// #define _USE_COM
//
// #ifdef _USE_COM

#import "EMMENU4.tlb" no_namespace named_guids raw_interfaces_only

BOOL AcquireImage(LPVOID pData, IEMMenuApplication *ptrEMMENU);
BOOL GetImage(LPVOID pData, IEMMenuApplication *ptrEMMENU);
BOOL GetEmVector(LPVOID pData, IEMMenuApplication *ptrEMMENU);

extern BOOL CRISP_InitObjFromImage(OBJ* pObj, SAVEIMAGE &S);
extern BOOL fioFixImageMemory( LPSAVEIMAGE I );
//extern UINT	cf16bit;		// Clipboard format for 16 bit image
extern NEWINFO		NIDefault;

static IEMMenuApplication *g_ptrEMMENU = NULL;

BOOL TVIPS_Initialize()
{
	return TRUE;
}

BOOL TVIPS_Uninitialize()
{
	BOOL bRes = FALSE;
	try
	{
		if( NULL != g_ptrEMMENU )
		{
			g_ptrEMMENU->Release();
			g_ptrEMMENU = NULL;
			bRes = TRUE;
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("TVIPS_Uninitialize() -> %s"), e.what()));
	}
	return bRes;
}

BOOL TVIPS_AcquireImage(LPVOID pData)
{
	BOOL bRes = FALSE;
	try
	{
		HRESULT hr;

		if( NULL != g_ptrEMMENU )
		{
			g_ptrEMMENU->Release();
		}
		hr = ::CoCreateInstance(CLSID_EMMenuApplication, NULL,
			CLSCTX_LOCAL_SERVER, IID_IEMMenuApplication, (LPVOID *)&g_ptrEMMENU);
		if( FAILED(hr) )
		{
			MessageBox(NULL, "Failed to create instance of EMMENU", "ERROR", MB_OK);
		}
		if( NULL != g_ptrEMMENU )
		{
			bRes = AcquireImage(pData, g_ptrEMMENU);
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("TVIPS_AcquireImage() -> %s"), e.what()));
	}
	return bRes;
}

BOOL TVIPS_GetImage(LPVOID pData)
{
	BOOL bRes = FALSE;
	try
	{
		HRESULT hr;

		if( NULL != g_ptrEMMENU )
		{
			g_ptrEMMENU->Release();
		}
		hr = ::CoCreateInstance(CLSID_EMMenuApplication, NULL,
			CLSCTX_LOCAL_SERVER, IID_IEMMenuApplication, (LPVOID *)&g_ptrEMMENU);
		if( FAILED(hr) )
		{
			MessageBox(NULL, "Failed to create instance of EMMENU", "ERROR", MB_OK);
		}
		if( NULL != g_ptrEMMENU )
		{
			bRes = GetImage(pData, g_ptrEMMENU);
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("TVIPS_GetImage() -> %s"), e.what()));
	}
	return bRes;
}

BOOL TVIPS_GetEmVector(LPVOID pData)
{
	BOOL bRes = FALSE;
	try
	{
		HRESULT hr;

		if( NULL != g_ptrEMMENU )
		{
			g_ptrEMMENU->Release();
		}
		hr = ::CoCreateInstance(CLSID_EMMenuApplication, NULL,
			CLSCTX_LOCAL_SERVER, IID_IEMMenuApplication, (LPVOID *)&g_ptrEMMENU);
		if( FAILED(hr) )
		{
			MessageBox(NULL, "Failed to create instance of EMMENU", "ERROR", MB_OK);
			return FALSE;
		}
		if( NULL != g_ptrEMMENU )
		{
			if( NULL != pData )
			{
				bRes = GetEmVector(pData, g_ptrEMMENU);
			}
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("TVIPS_GetEmVector() -> %s"), e.what()));
	}
	return bRes;
}

//////////////////////////////////////////////////////////////////////////

static BOOL GetFirstEMMenuVP(IImageVP **ptrView, IEMMenuApplication *ptrEMMENU)
{
	BOOL bOK = FALSE;

	if( ptrEMMENU == NULL )
		return FALSE;

	IViewports *ptrViews = NULL;

	try
	{
		HRESULT hr = ptrEMMENU->get_Viewports(&ptrViews);

		if(SUCCEEDED(hr) && ptrViews)
		{
			_variant_t vPos;
			vPos.vt = VT_I4;
			vPos.lVal = 1;
			hr = ptrViews->get_Item(vPos, ptrView);
			if(SUCCEEDED(hr))
				bOK = TRUE;
		}
	}
	catch(_com_error err)
	{
		Trace(_FMT(_T("GetFirstEMMenuVP() -> com error: %s"), (TCHAR*)err.Description()));
//		MessageBox( CString((wchar_t *)err.Description()) );
		bOK = FALSE;
	}
	if( NULL != ptrViews )
	{
		ptrViews->Release();
	}
	return bOK;
}

static BOOL AcquireImage(LPVOID pData, IEMMenuApplication *ptrEMMENU)
{
	BOOL bRes = FALSE;
	IImageVP *ptrImgView = NULL;
	try
	{
		if( GetFirstEMMenuVP(&ptrImgView, ptrEMMENU) )
		{
			try
			{
				ptrImgView->AcquireAndDisplayImage();
				bRes = TRUE;
			}
			catch(_com_error err)
			{
				Trace(_FMT(_T("AcquireImage() -> com error: %s"), (TCHAR*)err.Description()));
//				AfxMessageBox( CString((wchar_t *)err.Description()) );
			}
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("AcquireImage() -> %s"), e.what()));
	}
	if( NULL != ptrImgView )
	{
		ptrImgView->Release();
	}
	return bRes;
}

BOOL PreCreateImage(OBJ* pObj, LPVOID pExtraData )
{
	extern NEWINFO		NIDefault;
	BOOL bRes = FALSE;
	LPSAVEIMAGE pImg = (LPSAVEIMAGE)pExtraData;
	try
	{
		if( (NULL == pImg) || (NULL == pObj) )
		{
			throw std::exception("Image or Object pointer is NULL");
		}
		if( FALSE == fioFixImageMemory(pImg) )
		{
			throw std::exception("failed to allocate memory");
		}
		CRISP_InitObjFromImage(pObj, *pImg);
		bRes = TRUE;
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("PreCreateImage() -> %s"), e.what()));
	}
	return bRes;
}

static BOOL GetImage(LPVOID pData, IEMMenuApplication *ptrEMMENU)
{
	SAVEIMAGE	Img;
	HRESULT		hr;
	IMAGE_VECTOR emVector;
	BOOL		bRes = FALSE;
	IImageVP	*ptrImgView = NULL;
	IEMImage	*ptrImage = NULL;
	OBJ*		pObj;
	long		stride, lDimX, lDimY, ps;
	SAFEARRAY	*pSA = NULL;
	unsigned char *pImg;
	HWND		hWnd;
	DataType	data_type;

	try
	{
		if( GetFirstEMMenuVP(&ptrImgView, ptrEMMENU) )
		{
//			ptrImgView->AcquireAndDisplayImage();
			hr = ptrImgView->get_Image(&ptrImage);
			ptrImage->get_EMVector(&emVector);
			// Pixel sizes X/Y (um)
			double dCamPixX = 0.001 * emVector.fCamPixel[0];
			double dCamPixY = 0.001 * emVector.fCamPixel[1];
			double dCamLength = 1000;
			if( SUCCEEDED(hr) && (NULL != ptrImage) )
			{
				ptrImage->get_DimX(&lDimX);
				ptrImage->get_DimY(&lDimY);
				ptrImage->get_DataType(&data_type);
				// get the data
				switch( data_type )
				{
				case IMG_UCHAR:
					ps = 1;
					hr = ptrImage->GetDataByte(&pSA);
					break;
				case IMG_USHORT:
					ps = 2;
					hr = ptrImage->GetDataUShort(&pSA);
					break;
				case IMG_SHORT:
					ps = 2;
					hr = ptrImage->GetDataShort(&pSA);
					break;
// 				case IMG_LONG:
// 					ps = 4;
// 					hr = ptrImage->GetDataLong(&pSA);
// 					break;
				default:
					MessageBox(NULL, "CRISP supports only 8-bit or 16-bit cameras for EM-MENU", "ERROR", MB_OK);
					goto Error_Exit;
				}
				if( (NULL == pSA) || (!SUCCEEDED(hr)) )
				{
					goto Error_Exit;
				}
				// access the data
				hr = SafeArrayAccessData( pSA, (LPVOID*)&pImg );
				// prepare the memory
				memset(&Img, 0, sizeof(Img));
				stride = R4(ps * lDimX);
				Img.ps = ps;
				Img.x = lDimX;
				Img.y = lDimY;

				hWnd = objCreateObject(CRISP_EMPTY_IMG, NULL, PreCreateImage, &Img);
				if( hWnd )
				{
					if( NULL != (pObj = OBJ::GetOBJ(hWnd)) )
					{
						memcpy(&pObj->NI, &NIDefault, sizeof(NIDefault));
						// microscope is not known yet
						pObj->NI.EM_ID = -1;
 						// the accelerating voltage, in V
						pObj->NI.Vacc = emVector.fTemHT;
						dCamLength = emVector.fTemMagScr * emVector.fTemMagCor * emVector.fTemMagPst;
						pObj->NI.CamLength = 0.001 * dCamLength;
						// Image mode - 0, Diffraction mode - 3
						if( 0 == emVector.lImgType )
						{
							pObj->NI.Digit = 1e6 / (dCamPixX * emVector.lCamBinX);
						}
						else if( 3 == emVector.lImgType )
						{
//							dCamLength = emVector.fTemMagScr * emVector.fTemMagCor * emVector.fTemMagPst;
//							pObj->NI.CamLength = 0.001 * dCamLength;
							pObj->NI.Digit = 1e6 / dCamPixX;
//							pObj->NI.Scale = pObj->NI.Digit*CRISP_WaveLength(pObj->NI.Vacc)*pObj->NI.CamLength;
							pObj->NI.Flags |= NI_DIFFRACTION_PATTERN;
						}
						pObj->NI.Scale = pObj->NI.Digit*CRISP_WaveLength(pObj->NI.Vacc)*pObj->NI.CamLength;

						LPBYTE lpDst = (LPBYTE)pObj->dp;// + (lDimY - 1)*stride);
						LPBYTE lpSrc = (LPBYTE)pImg;
						for(int i = 0; i < lDimY; i++)
						{
							memcpy( lpDst, lpSrc, lDimX*ps );
							if( IMG_SHORT == data_type )
							{
								LPWORD pPtr = (LPWORD)lpDst;
								for(int j = 0; j < lDimX; j++)
								{
									// invert the sing -> signed to unsigned conversion
									pPtr[j] = (pPtr[j] ^ 0x8000);
								}
							}
							lpDst += stride;
							lpSrc += stride;
						}
						imgCalcMinMax( pObj );
						memset(pObj->title, 0, _MAX_PATH);
						int wclen = wcslen(emVector.bstrImgName);
						wcsrtombs(pObj->title, (const wchar_t **)&emVector.bstrImgName, wclen, NULL);
						wndSetInitialWindowSize(hWnd);
					}
					ActivateObj(hWnd);
				}
				bRes = TRUE;
			}
		}
	}
	catch(std::exception& e)
	{
		MessageBox(NULL, GetUserErrorAndTrace(_FMT(_T("GetImage() -> %s"), e.what())).c_str(), "ERROR", MB_OK);
	}
Error_Exit:
	if( NULL != pSA )
	{
		SafeArrayUnaccessData( pSA );
		SafeArrayDestroy( pSA );
	}
	if( NULL != ptrImgView )
	{
		ptrImgView->Release();
	}
	if( NULL != ptrImage )
	{
		ptrImage->Release();
	}
	return bRes;
}

static BOOL GetEmVector(LPVOID pData, IEMMenuApplication *ptrEMMENU)
{
	IMAGE_VECTOR emVector;
	BOOL bRes = FALSE;
	IImageVP	*ptrImgView = NULL;;
	IEMImage	*ptrImage = NULL;;
	HRESULT hr;

	try
	{
		if( GetFirstEMMenuVP(&ptrImgView, ptrEMMENU) )
		{
			hr = ptrImgView->get_Image(&ptrImage);
			if( SUCCEEDED(hr) && ptrImage != NULL )
			{
				ptrImage->get_EMVector(&emVector);
				bRes = TRUE;
//				CDlgEMVector dlgEMVect(emVector);
//				dlgEMVect.DoModal();
			}
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("GetEmVector() -> %s"), e.what()));
	}
	if( NULL != ptrImgView )
	{
		ptrImgView->Release();
	}
	if( NULL != ptrImage )
	{
		ptrImage->Release();
	}
	return bRes;
}

// #else
//
// BOOL TVIPS_Initialize()
// {
// 	return TRUE;
// }
//
// BOOL TVIPS_Uninitialize()
// {
// 	return FALSE;
// }
//
// BOOL TVIPS_AcquireImage(LPVOID pData)
// {
// 	return FALSE;
// }
//
// BOOL TVIPS_GetImage(LPVOID pData)
// {
// 	return FALSE;
// }
//
// BOOL TVIPS_GetEmVector(LPVOID pData)
// {
// 	return FALSE;
// }
//
// #endif
