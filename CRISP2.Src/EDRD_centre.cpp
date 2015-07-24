/****************************************************************************/
/* Copyright (c) 2006 Ananlitex *********************************************/
/****************************************************************************/
/* CRISP2.EDRD_center * by Peter Oleynikov **********************************/
/****************************************************************************/

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
#include "EDRD.h"

#pragma warning(disable:4305)

// uncommented by Peter
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

typedef std::complex<double> complexd;

// TODO: other formats
extern UINT	g_Clipboard_16u;		// Clipboard format for 16 bit image
extern UINT	g_Clipboard_32u;		// Clipboard format for 32 bit image
//extern HWND objCreateObject( UINT uType, OBJ* OE );
//extern bool Fft2D_Calculate(double *pData, size_t nn, size_t mm, int iSign);
extern void fioReportPercent( int c, int tot, BOOL bRead );

CVector2d GetImageCenter(const IMAGE& inImage)
{
	// prepare an image
	double dScale = 0;
	IMAGE Image = inImage.Get512x512RescaledImage(&dScale);
	IMAGE CorImage;
	if( false == Image.AutoCorrelate(CorImage) )
	{
		if( (NULL != Image.pData) && (inImage.pData != Image.pData) )
			delete[] Image.pData;
		if( NULL != CorImage.pData )
			delete[] CorImage.pData;

		throw std::exception(_T("Failed to run correlation in GetImageCenter"));
	}

	if( NULL != CorImage.pData )
	{
		CVector2d pos = ProcessImage<GetMaxPeakCentre_t>(CorImage);
		if( pos.x < (CorImage.w>>1) )
		{
			pos.x = CorImage.w + pos.x;
		}
		if( pos.y < (CorImage.h>>1) )
		{
			pos.y = CorImage.h + pos.y;
		}
		return pos * 0.5f * dScale;
		
	}

	if( (NULL != Image.pData) && (inImage.pData != Image.pData) )
		delete[] Image.pData;
	if( NULL != CorImage.pData )
		delete[] CorImage.pData;
	throw std::exception(_T("Failed to get image data in GetImageCenter"));
}

BOOL GetAutoconvCentre( HWND hDlg, OBJ* pObj, LPEDRD pEDRD )
{
	CVector2d pos = CVector2d(double(pEDRD->image.w>>1), double(pEDRD->image.h>>1));
	
	ClrBits( pEDRD->flags, CRDONE );
	try
	{
		pos = GetImageCenter(pEDRD->image);
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("GetAutoconvCentre() -> %s"), e.what()));
		return FALSE;
	}
	SetBits( pEDRD->flags, CRDONE );
	pEDRD->xc = pos.x;
	pEDRD->yc = pos.y;

	return TRUE;
}

void CalculteImageDim(IMAGE &In, CVector2f &center, double dAng, double dXscale,
					  CVector2f &new_center, int &new_w, int &new_h, CMatrix3x3 &mtx)
{
//	double dSin, dCos, _x, _y;
	int /*cx, cy, */w2 = In.w, h2 = In.h;
	CVector2f sbox[] = {CVector2f(0, 0), CVector2f(0, h2), CVector2f(w2, h2), CVector2f(w2, 0)};
	CVector2f box[4];
	double dMinX, dMaxX, dMinY, dMaxY;
	CMatrix3x3 sh, sh2, rt, rt2, sc;
	CVector3d pos;

	dMaxX = dMaxY = -(dMinX = dMinY =  numeric_limits<double>::max());
//	FastSinCos(-dAng, dSin, dCos);
//	cx = center.x;
//	cy = center.y;
	sh.Translate(CVector3d(-center.x, -center.y, 0));
	rt.RotateZ(-dAng);
	rt2.RotateZ(0);//dAng);
	sc.Scale(CVector3d(1.0/dXscale, 1, 1));
	mtx = rt2*(sc*(rt*sh));
	for(int i = 0; i < 4; i++)
	{
		pos = mtx * CVector3d(sbox[i].x, sbox[i].y, 1);
		box[i].x = pos.x;
		box[i].y = pos.y;
		if( pos.x < dMinX ) dMinX = pos.x;
		if( pos.x > dMaxX ) dMaxX = pos.x;
		if( pos.y < dMinY ) dMinY = pos.y;
		if( pos.y > dMaxY ) dMaxY = pos.y;
/*
		_x = box[i].x - cx;
		_y = box[i].y - cy;
		box[i].x = ( _x*dCos + _y*dSin) * dXscale;
		box[i].y =  -_x*dSin + _y*dCos;
		if( box[i].x < dMinX ) dMinX = box[i].x;
		if( box[i].x > dMaxX ) dMaxX = box[i].x;
		if( box[i].y < dMinY ) dMinY = box[i].y;
		if( box[i].y > dMaxY ) dMaxY = box[i].y;
*/
	}
	new_center = CVector2f(-dMinX, -dMinY);
	new_w = (int)floor(dMaxX - dMinX + 0.5);
	new_h = (int)floor(dMaxY - dMinY + 0.5);
	sh2.Translate(CVector3d(-dMinX, -dMinY, 0));
	mtx = sh2 * mtx;
}
template <typename _Tx>
bool transform_image_t(_Tx *pIn, _Tx *pOut, int strideIn, int strideOut,
					   int w_old, int h_old, int w_new, int h_new, double *ptr_mtx)
{
	double _x, _y, dy1, dy2;
	int x, y;

	try
	{
		// rotate the image around the center and save it
		for(y = 0; y < h_new; y++)
		{
			dy1 = y*ptr_mtx[1] + ptr_mtx[2];
			dy2 = y*ptr_mtx[4] + ptr_mtx[5];
			for(x = 0; x < w_new; x++)
			{
				_x = x*ptr_mtx[0] + dy1;
				if( (_x < 0.0) || (_x > w_old-1) )
					continue;
				_y = x*ptr_mtx[3] + dy2;
				if( (_y < 0.0) || (_y > h_old-1) )
					continue;
				pOut[x] = (_Tx)floor(Bilinear<_Tx>(pIn, strideIn, w_old, h_old, _x, _y) + 0.5);
			}
			pOut = (_Tx*)(((LPBYTE)pOut) + strideOut);
			fioReportPercent(y, h_new, FALSE);
		}
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("transform_image_t() -> %s"), e.what()));
	}
	fioReportPercent(0, 0, FALSE);
	return true;
}

void CorrectEllipticalDistortion(HWND hDlg, OBJ* pObj, LPEDRD pEDRD)
{
	HANDLE	h;
	IMAGE Image, Image3;
	CVector2f center = CVector2f((float)pEDRD->xc, (float)pEDRD->yc);
	CVector2f new_center;
	int new_w, new_h;
	CMatrix3x3 mtx, inv_mtx;

	Image.npix = pEDRD->image.npix;//pix;
	Image.w = pEDRD->image.w;//iw;
	Image.h = pEDRD->image.h;//ih;
	Image.stride = pEDRD->image.stride;//R4(Image.w*Image.npix);
	Image.pData = pEDRD->image.pData;//lpImg;

	try
	{
		double a = pEDRD->dDistA, b = pEDRD->dDistB;
		//////////////////////////////////////////////////////////////////////////
		CalculteImageDim(Image, center, pEDRD->dDistortionDir, a/b, new_center, new_w, new_h, mtx);
		Image3.CreateImage(new_w, new_h, Image.npix);
		mtx.Invert(inv_mtx);
		//////////////////////////////////////////////////////////////////////////
		switch( Image.npix )
		{
		case PIX_BYTE:
			transform_image_t<BYTE>((LPBYTE)Image.pData, (LPBYTE)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		case PIX_CHAR:
			transform_image_t<CHAR>((LPSTR)Image.pData, (LPSTR)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		case PIX_WORD:
			transform_image_t<WORD>((LPWORD)Image.pData, (LPWORD)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		case PIX_SHORT:
			transform_image_t<SHORT>((SHORT*)Image.pData, (SHORT*)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		case PIX_DWORD:
			transform_image_t<DWORD>((LPDWORD)Image.pData, (LPDWORD)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		case PIX_LONG:
			transform_image_t<LONG>((LPLONG)Image.pData, (LPLONG)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		case PIX_FLOAT:
			transform_image_t<float>((float*)Image.pData, (float*)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		case PIX_DOUBLE:
			transform_image_t<double>((double*)Image.pData, (double*)Image3.pData, Image.stride, Image3.stride,
				Image.w, Image.h, Image3.w, Image3.h, &inv_mtx.d[0][0]);
			break;
		default:
			break;
		}
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("CorrectEllipticalDistortion() while rotating -> %s"), e.what()));
	}
	try
	{
		int i;
		if( Image3.pData )
		{
			int xs = Image3.w, ys = Image3.h, ps = Image3.npix;
			int stride = Image3.stride;
			h = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,
				stride * ys + sizeof(BITMAPINFOHEADER) + 256*sizeof(RGBQUAD));
			if( NULL == h )
				goto Error_Exit;
			LPBITMAPINFOHEADER lpBM = (LPBITMAPINFOHEADER)GlobalLock(h);
			if( NULL == lpBM )
				goto Error_Exit;
			memset( lpBM, 0, stride * ys + sizeof(BITMAPINFOHEADER)+256*sizeof(RGBQUAD) );
			lpBM->biSize = sizeof(BITMAPINFOHEADER);
			lpBM->biWidth = xs;
			lpBM->biHeight = ys;
			lpBM->biSizeImage = 0;
			lpBM->biPlanes = 1;
			lpBM->biBitCount = 8*ps;
			lpBM->biCompression = BI_RGB;
			lpBM->biClrUsed = lpBM->biClrImportant = 256;
			lpBM->biXPelsPerMeter = lpBM->biYPelsPerMeter = 0;
			LPRGBQUAD lpRGBQ = (LPRGBQUAD) ((LPSTR)lpBM + sizeof(BITMAPINFOHEADER));
			for(i = 0; i < 256; i++)
				lpRGBQ[i].rgbRed = lpRGBQ[i].rgbGreen = lpRGBQ[i].rgbBlue = i;
			LPBYTE lpDst = (LPBYTE)(lpRGBQ + 256) + (ys-1)*stride;
			LPBYTE lpSrc = (LPBYTE)Image3.pData;
			int srcw = Image3.stride;
			for(i = 0; i < Image3.h; i++)
			{
				memcpy( lpDst, lpSrc, Image3.w*ps );
				lpDst -= stride;
				lpSrc += srcw;
			}
			GlobalUnlock( h );
			if( OpenClipboard(MainhWnd) )
			{
				EmptyClipboard();
				if      ( 1 == ps ) SetClipboardData( CF_DIB, h );
				else if ( 2 == ps ) SetClipboardData( g_Clipboard_16u, h );
				else if ( 4 == ps ) SetClipboardData( g_Clipboard_32u, h );
				CloseClipboard();
			}
			else
				goto Error_Exit;
			HWND hWnd = objCreateObject( IDM_E_PASTE, NULL );
			if( hWnd )
			{
				OBJ* pNewObj = OBJ::GetOBJ(hWnd);
				if( pNewObj )
				{
					strcpy(pNewObj->title, pObj->title);
					strcat(pNewObj->title, " corrected");
					wndSetInitialWindowSize(hWnd);
				}
				ActivateObj(hWnd);
				hWnd = objCreateObject( IDM_E_EDRD, NULL );
				if( hWnd )
				{
					pNewObj = OBJ::GetOBJ(hWnd);
					if( pNewObj )
					{
						LPEDRD pEDRD2 = (LPEDRD)&pNewObj->dummy;
						if( pEDRD2 )
						{
							pEDRD2->xc = new_center.x;
							pEDRD2->yc = new_center.y;
							pEDRD2->r = pEDRD->r;//__min(__min(pEDRD2->xc - 2,pEDRD2->iw - pEDRD2->xc - 2),
								//__min(pEDRD2->yc - 2,pEDRD2->ih - pEDRD2->yc - 2));
							UpdateCoordinatesText(hWnd, pNewObj, pEDRD2);
						}
					}
					ActivateObj(hWnd);
				}
			}
		}
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("CorrectEllipticalDistortion() -> %s"), e.what()));
	}
Error_Exit:
	delete[] Image3.pData;
	GlobalFree( h );
}

bool Estimate1DPeakPosition(double *pProfile, int nPoints, double &pos)
{
 	bool bRes = false;
	try
	{
		CNonlinearFit nlfit;
		double adParams[32];
		double sx, sy, sx2, sxy;
		bool abParams[32];
		vector<double> vXcoords, deriv(nPoints, 0);
		int i;

		// nothing to calculate
		if( nPoints < 7 )
			return false;
		// calculate the derivative
		// 3-point slope == derivative
		for(i = 1; i < nPoints-1; i++)
		{
			sx = 3*i;
			sy = pProfile[i-1] + pProfile[i] + pProfile[i+1];
			sx2 = 3*i*i + 2;
			sxy = pProfile[i-1]*(i-1) + pProfile[i]*i + pProfile[i+1]*(i+1);
			deriv[i] = (3*sxy - sx*sy) / (3*sx2 - sx*sx);
		}
		deriv[0] = deriv[1];
		deriv[nPoints-1] = deriv[nPoints-2];
		// analyze the derivative - find the closest zero-cross
		int nClosestD = nPoints, nClosestPos = -1;
		for(i = 1; i < nPoints-1; i++)
		{
			if( (deriv[i] > 0) && (deriv[i+1] <= 0) && (abs(pos - i) < nClosestD) )
			{
				nClosestPos = i;
				nClosestD = abs(pos - i);
			}
		}
		int nStart = 0, nEnd = nPoints, nSize = nPoints;
		if( (-1 != nClosestPos) && (nClosestPos < nPoints) )
		{
			// get the starting and ending point
			for(nStart = nClosestPos-1; nStart > 0; nStart--)
			{
				if( pProfile[nStart] < pProfile[nStart-1] )
				{
					break;
				}
			}
			for(nEnd = nClosestPos+1; nEnd < nPoints-1; nEnd++)
			{
				if( pProfile[nEnd] < pProfile[nEnd+1] )
				{
					break;
				}
			}
			if( nClosestPos - nStart < 3 )
			{
				nStart = nClosestPos - 3;
			}
			if( nEnd - nClosestPos < 3 )
			{
				nEnd = nClosestPos + 3;
			}
			nSize = nEnd - nStart + 1;
		}
		// failed to find the derivative, fit as-it-is
		// TODO: if missing???
		double *max_el = max_element(pProfile + nStart, pProfile + nEnd + 1);
		double *min_el = min_element(pProfile + nStart, pProfile + nEnd + 1);
		pos = max_el - pProfile;
		if( nSize < 7 )
		{
			return true;
		}
		vXcoords.resize(nPoints);
		for(i = 0; i < vXcoords.size(); i++)
		{
			vXcoords[i] = double(i);
		}
		adParams[0] = *min_el;						// background
		adParams[1] = 0.0;							// background
		adParams[2] = 0.0;							// background
		adParams[3] = 0.5;							// M ratio
		adParams[4] = 0.5;							// M ratio
		adParams[5] = *max_el - *min_el;			// amplitude
		adParams[6] = max_el - pProfile;			// peak position
		adParams[7] = adParams[8] = nSize >> 2;	// peak FWHM
		if( adParams[7] < 1 )
		{
			adParams[7] = adParams[8] = 1;
		}
//		adParams[4] = *max_el - *min_el;			// amplitude
//		adParams[5] = max_el - pProfile;			// peak position
//		adParams[6] = adParams[7] = nSize >> 2;	// peak FWHM
//		if( adParams[6] < 1 )
//		{
//			adParams[6] = adParams[7] = 1;
//		}
		abParams[0] = 1;
		abParams[1] = 0;
		abParams[2] = 0;
		abParams[3] = 1;
		abParams[4] = 1;
		abParams[5] = 1;
		abParams[6] = 1;
		abParams[7] = 1;
		abParams[8] = 1;
		bRes = nlfit.Fit(pProfile + nStart, &*vXcoords.begin() + nStart, NULL, nSize,
			adParams, abParams, NL_PseudoVoigt1DAs2.GetNumParams(1), &NL_PseudoVoigt1DAs2);
		if( (adParams[6] >= nStart) && (adParams[6] < nEnd) )
		{
			pos = adParams[6];
		}
	}
	catch( std::exception& e )
	{
		Trace(_FMT(_T("Estimate1DPeakPosition() -> %s"), e.what()));
	}
	return bRes;
}

/*
IMAGE scale_rotate_B(IMAGE &In, CVector2f &center, CVector2f &new_center, double dAng, double dXscale)
{
//	double dSin, dCos;
	double _x, _y, dy1, dy2;//, cx, cy;
	int x, y, stride, new_w, new_h;
	int w2 = In.w;
	int h2 = In.h;
	IMAGE Out;
	LPBYTE src, dst;
//	CVector2f new_center;
	CMatrix3x3 mtx, inv_mtx;
	double *ptr = &inv_mtx.d[0][0];

	CalculteImageDim(In, center, dAng, dXscale, new_center, new_w, new_h, mtx);
	mtx.Invert(inv_mtx);
//	FastSinCos(-dAng, dSin, dCos);
//	cx = center.x;
//	cy = center.y;
	Out.CreateImage(new_w, new_h, In.npix);
	stride = In.stride;
	// rotate the image around the center and save it
	src = (LPBYTE)In.pData;
	dst = (LPBYTE)Out.pData;
	for(y = 0; y < new_h; y++)
	{
//		dy = (double)y - new_center.y;
		dy1 = y*ptr[1] + ptr[2];
		dy2 = y*ptr[4] + ptr[5];
		for(x = 0; x < new_w; x++)
		{
//			dx = (double)x * dXscale - new_center.x;
//			_x = dCos * dx - dSin * dy + cx;
//			_y = dSin * dx + dCos * dy + cy;
//			pos = inv_mtx * CVector3d(x, y, 1);
			_x = x*ptr[0] + dy1;
			if( (_x < 0.0) || (_x > w2-1) )
				continue;
			_y = x*ptr[3] + dy2;
			if( (_y < 0.0) || (_y > h2-1) )
				continue;
			dst[x] = (BYTE)floor(Bilinear<BYTE>(src, stride, w2, h2, _x, _y) + 0.5);
		}
		dst = dst + Out.stride;
		fioReportPercent(y, new_h, FALSE);
	}
	fioReportPercent(0, 0, FALSE);
	return Out;
}

IMAGE scale_rotate_W(IMAGE &In, CVector2f &center, CVector2f &new_center, double dAng, double dXscale)
{
	double _x, _y, dy1, dy2;
	int x, y, stride, new_w, new_h;
	int w2 = In.w;
	int h2 = In.h;
	IMAGE Out;
	LPWORD src, dst;
	CMatrix3x3 mtx, inv_mtx;
	double *ptr = &inv_mtx.d[0][0];

	CalculteImageDim(In, center, dAng, dXscale, new_center, new_w, new_h, mtx);
	Out.CreateImage(new_w, new_h, In.npix);
	mtx.Invert(inv_mtx);
	stride = In.stride;
	// rotate the image around the center and save it
	src = (LPWORD)In.pData;
	dst = (LPWORD)Out.pData;
	for(y = 0; y < new_h; y++)
	{
		dy1 = y*ptr[1] + ptr[2];
		dy2 = y*ptr[4] + ptr[5];
		for(x = 0; x < new_w; x++)
		{
			_x = x*ptr[0] + dy1;
			if( (_x < 0.0) || (_x > w2-1) )
				continue;
			_y = x*ptr[3] + dy2;
			if( (_y < 0.0) || (_y > h2-1) )
				continue;
			dst[x] = (WORD)floor(Bilinear<WORD>(src, stride, w2, h2, _x, _y) + 0.5);
		}
		dst = (LPWORD)(((LPBYTE)dst) + Out.stride);
		fioReportPercent(y, new_h, FALSE);
	}
	fioReportPercent(0, 0, FALSE);
	return Out;
}
*/
/*
template <typename _Tx>
IMAGE rotate_t(IMAGE &In, CVector2f center, double dAng)
{
	double dSin, dCos;
	double _x, _y, dx, dy, cx, cy;
	int x, y, stride;
	int w = In.w;
	int h = In.h;
	IMAGE Out = CreateImage(w, h, In.npix);
	memset(Out.pData, 0, Out.stride*Out.h);
	_Tx *src, *dst;

	stride = In.stride;
	cx = center.x;
	cy = center.y;
	FastSinCos(dAng, dSin, dCos);
	// rotate the image around the center and save it
	src = (_Tx*)In.pData;
	dst = (_Tx*)Out.pData;
	for(y = 0; y < h; y++)
	{
		dy = (double)y - cy;
		for(x = 0; x < w; x++, dst++)
		{
			dx = (double)x - cx;
//			_x =  dCos * dx + dSin * dy + center.x;
//			_y = -dSin * dx + dCos * dy + center.y;
			_x = dCos * dx - dSin * dy + cx;
			_y = dSin * dx + dCos * dy + cy;
			if( (_x < 0.0) || (_x > w-1) )
				continue;
			if( (_y < 0.0) || (_y > h-1) )
				continue;
			*dst = (_Tx)floor(Bilinear<_Tx>(src, stride, _x, _y) + 0.5);
		}
	}
	return Out;
}

template <typename _Tx>
IMAGE scale_t(IMAGE &In, double dXscale)
{
	double _x, _y, inv_;
	int x, y, sc;
	int w2 = In.w;
	int w = (int)floor(w2 * dXscale + 0.5);
	int h = In.h;
	IMAGE Out = CreateImage(w, h, In.npix);
	_Tx *src, *dst;

	sc = Out.stride;
	// scale the image along X axis only
	src = (_Tx*)In.pData;
	dst = (_Tx*)Out.pData;
	inv_ = 1.0 / dXscale;
	_y = 0.0;
	for(y = 0; y < h-1; y++)
	{
		_x = 0.0;
		for(x = 0; x < w; x++)
		{
			dst[x] = (_Tx)floor(Bilinear<_Tx>(src, w2, _x, _y) + 0.5);
			_x += inv_;
		}
		dst = (_Tx*)(((LPBYTE)dst) + sc);
		_y += 1.0;
	}
	return Out;
}
*/
