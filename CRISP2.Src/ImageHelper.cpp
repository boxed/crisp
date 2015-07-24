/****************************************************************************/
/* Copyright (c) 2006 Calidris.					*****************************/
/****************************************************************************/
/* Written by Peter Oleynikov, Analitex *************************************/
/****************************************************************************/

#include "StdAfx.h"

// #define  cimg_use_visualcpp6
// #include <cimg.h>

#include "commondef.h"
#include "objects.h"
#include "shtools.h"
#include "image.h"
#include "ft32util.h"
#include "eld.h"

#define		RESAMPLING_SIZE		512

typedef std::complex<double> complexd;

extern bool Fft2D_Calculate(double *pData, size_t nn, size_t mm, int iSign);
extern void fioReportPercent(int c, int tot, BOOL bRead);

/****************************************************************************/
/* IMAGE class						*****************************************/
/****************************************************************************/

void IMAGE::Init()
{
	pData = NULL;
	npix = PIX_BYTE;
	w = 0;
	h = 0;
	stride = 0;
	neg = 0;
	m_dMin = 0.0;
	m_dMax = 0.0;
	m_dIMin = 0.0;
	m_dIMax = 0.0;
	m_dIMinA = 0.0;
	m_dIMaxA = 0.0;
}

IMAGE &IMAGE::operator=(const IMAGE &image)
{
	pData = image.pData;
	npix = image.npix;
	w = image.w;
	h = image.h;
	stride = image.stride;
	neg = image.neg;
	m_dMin = image.m_dMin;
	m_dMax = image.m_dMax;
	m_dIMin = image.m_dIMin;
	m_dIMax = image.m_dIMax;
	m_dIMinA = image.m_dIMinA;
	m_dIMaxA = image.m_dIMaxA;
	return *this;
}

int IMAGE::PixFmt2Bpp(PIXEL_FORMAT nPixFmt)
{
	int nBpp = 1;
	switch( nPixFmt )
	{
	case PIX_BYTE:
	case PIX_CHAR:
		nBpp = 1;
		break;
	case PIX_WORD:
	case PIX_SHORT:
		nBpp = 2;
		break;
	case PIX_DWORD:
	case PIX_LONG:
	case PIX_FLOAT:
		nBpp = 4;
		break;
	case PIX_DOUBLE:
		nBpp = 8;
		break;
	}
	return nBpp;
}

void IMAGE::SetData(LPVOID _data, int _w, int _h, PIXEL_FORMAT _pix)
{
	pData = _data;
	w = _w;
	h = _h;
	npix = _pix;
	neg = 0;
	stride = R4(w * PixFmt2Bpp(npix));
}

void IMAGE::CreateImage(int _w, int _h, PIXEL_FORMAT bpp)
{
	w = _w;
	h = _h;
	npix = bpp;
	neg = 0;
	stride = R4(w * PixFmt2Bpp(bpp));
	pData = new BYTE[stride*h];
	if( NULL != pData )
	{
		memset(pData, 0, stride*h);
	}
}

void IMAGE::SaveImage(LPCSTR pszPath)
{
	SAVEIMAGE si;
	NEWINFO ni;

	memset(&ni, 0, sizeof(ni));
	si.lpData = (LPBYTE)pData;
	si.ps = PixFmt2Bpp(npix);
	si.x = w;
	si.y = h;
	si.x0 = si.y0 = 0;
#ifdef USE_XB_LENGTH
	si.xb = w;
#else
	si.stride = R4(w * si.ps);
#endif
	si.lpAdd = &ni;
	si.iAddSize = sizeof(NEWINFO);
	fioSaveImageFile((LPSTR)pszPath, &si, 0);
}

static DWORD GetNegateFlag(PIXEL_FORMAT npix)
{
	DWORD neg = 0;
	switch( npix )
	{
	case PIX_BYTE: neg = 0xFF; break;
	case PIX_CHAR: neg = 0xFF; break;
	case PIX_WORD: neg = 0xFFFF; break;
	case PIX_SHORT: neg = 0xFFFF; break;
	case PIX_DWORD: neg = 0xFFFFFFFF; break;
	case PIX_LONG: neg = 0xFFFFFFFF; break;
	case PIX_FLOAT: neg = 1; break;
	case PIX_DOUBLE: neg = 1; break;
	}
	return neg;
}

void IMAGE::SetNegative(bool bNegative /*= false*/)
{
	neg = 0;
	if( true == bNegative )
	{
		neg = GetNegateFlag(npix);
	}
}

template <typename _Tx>
void InvertImage_t(_Tx *_pData, int _stride, int _w, int _h, int neg)
{
	int x, y;
	_Tx *ptr = _pData;
	for(y = 0; y < _h; y++)
	{
		for(x = 0; x < _w; x++)
		{
			ptr[x] = Negate(ptr[x], neg);
// 			ptr[x] = -ptr[x];
		}
		ptr = (_Tx*)(((LPBYTE)ptr) + _stride);
	}
}

void IMAGE::InvertImage(LPVOID _pData, PIXEL_FORMAT _npix, int _stride, int _w, int _h)
{
	switch( _npix )
	{
	case PIX_BYTE: InvertImage_t((LPBYTE)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	case PIX_CHAR: InvertImage_t((LPSTR)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	case PIX_WORD: InvertImage_t((LPWORD)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	case PIX_SHORT: InvertImage_t((SHORT*)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	case PIX_DWORD: InvertImage_t((LPDWORD)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	case PIX_LONG: InvertImage_t((LPLONG)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	case PIX_FLOAT: InvertImage_t((LPFLOAT)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	case PIX_DOUBLE: InvertImage_t((LPDOUBLE)_pData, _stride, _w, _h, GetNegateFlag(_npix)); break;
	default:
		break;
	}
}

void IMAGE::InvertImage()
{
	InvertImage(pData, npix, stride, w, h);
}

IMAGE IMAGE::Get512x512RescaledImage(double *outScale/*=NULL*/) const
{
	double dScale;
	IMAGE ScaledImg;

	try
	{
		// scale it to 512x512 first
		dScale = __max(double(w) / double(RESAMPLING_SIZE), double(h) / double(RESAMPLING_SIZE));
		if (outScale != NULL)
			*outScale = dScale;
		ScaledImg = GetScaledImage(dScale);
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("Get512x512RescaledImage() -> failed to scale: %s"), e.what()));
	}
	try
	{
		if( (ScaledImg.w != RESAMPLING_SIZE) || (ScaledImg.h != RESAMPLING_SIZE) )
		{
			ScaledImg.ChangeCanvas(RESAMPLING_SIZE, RESAMPLING_SIZE);
//#ifdef _DEBUG
//			SaveImage("D:\\rescaled.bmp", ScaledImg);
//#endif
		}
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("Get512x512RescaledImage() -> failed to change canvas: %s"), e.what()));
	}
//#ifdef _DEBUG
//	ScaledImg.SaveImage("D:\\rescaled.tif");
//#endif
	return ScaledImg;
}

template <typename _Tx>
bool Canvas_tx(_Tx* pSrc, size_t OldW, size_t OldH, size_t OldStride,
			   _Tx *pDst, size_t NewH, size_t NewW, size_t NewStride)
{
	_Tx *ptr;
	size_t y, size_x, size_y, stride;

	//	memset(pDst, 0, NewStride*NewH);		// empty the array
	// min sizes which fit the image
	size_x = (OldW < NewW) ? OldW : NewW;
	size_y = (OldH < NewH) ? OldH : NewH;
	stride = (OldStride < NewStride) ? OldStride : NewStride;
	ptr = pDst;
	for(y = 0; y < size_y; y++)
	{
		memcpy(ptr, pSrc, stride);
		ptr = (_Tx*)(((LPBYTE)ptr) + NewStride);
		pSrc = (_Tx*)(((LPBYTE)pSrc) + OldStride);
	}
	for(; y < NewH; y++)
	{
		memset(ptr, 0, NewStride);
		ptr = (_Tx*)(((LPBYTE)ptr) + NewStride);
	}

	return true;
}

bool IMAGE::ChangeCanvas(size_t NewW, size_t NewH)
{
	bool bRes = false;
	IMAGE Image;

	Image.CreateImage(NewW, NewH, npix);
	if( NULL == Image.pData )
	{
		goto Error_Exit;
	}
	switch( npix )
	{
	case PIX_BYTE:
		bRes = Canvas_tx<BYTE>((LPBYTE)pData, w, h, stride, (LPBYTE)Image.pData, Image.w, Image.h, Image.stride);
		break;
	case PIX_CHAR:
		bRes = Canvas_tx<CHAR>((LPSTR)pData, w, h, stride, (LPSTR)Image.pData, Image.w, Image.h, Image.stride);
		break;
	case PIX_WORD:
		bRes = Canvas_tx<WORD>((LPWORD)pData, w, h, stride, (LPWORD)Image.pData, Image.w, Image.h, Image.stride);
		break;
	case PIX_SHORT:
		bRes = Canvas_tx<SHORT>((SHORT*)pData, w, h, stride, (SHORT*)Image.pData, Image.w, Image.h, Image.stride);
		break;
	case PIX_DWORD:
		bRes = Canvas_tx<DWORD>((LPDWORD)pData, w, h, stride, (LPDWORD)Image.pData, Image.w, Image.h, Image.stride);
		break;
	case PIX_LONG:
		bRes = Canvas_tx<LONG>((LPLONG)pData, w, h, stride, (LPLONG)Image.pData, Image.w, Image.h, Image.stride);
		break;
	case PIX_FLOAT:
		bRes = Canvas_tx<FLOAT>((LPFLOAT)pData, w, h, stride, (LPFLOAT)Image.pData, Image.w, Image.h, Image.stride);
		break;
	case PIX_DOUBLE:
		bRes = Canvas_tx<DOUBLE>((LPDOUBLE)pData, w, h, stride, (LPDOUBLE)Image.pData, Image.w, Image.h, Image.stride);
		break;
	}
	if( bRes )
	{
		// delete current image
		w = Image.w;
		h = Image.h;
		stride = Image.stride;
		npix = Image.npix;
		pData = Image.pData;
	}
Error_Exit:
	return bRes;
}

template <typename _Tx>
void Scale2_tx(_Tx *src, _Tx *dst, size_t src_w, size_t src_h, size_t stride_src,
			   size_t dst_w, size_t dst_h, size_t stride_dst, double Scale)
{
	size_t x, y;
	double X, Y;

	Y = 0.0;
	for(y = 0; (Y < src_h) && (y < dst_h); y++, Y += Scale)
	{
		X = 0.0;
		for(x = 0; (X < src_w) && (x < dst_w); x++, X += Scale)
		{
			dst[x] = (_Tx)(Bilinear<_Tx>(src, stride_src, src_w, src_h, X, Y) + 0.5);
		}
		dst = (_Tx*)(((LPBYTE)dst) + stride_dst);
	}
}

static IMAGE ScaleImage2(IMAGE &SrcImage, double dScale)
{
	IMAGE ScImage;
	size_t w, h, stride_src, new_W, new_H;
	PIXEL_FORMAT bpp;

	bpp = SrcImage.npix;
	w = SrcImage.w;
	h = SrcImage.h;
	stride_src = SrcImage.stride;

	new_W = (size_t)floor(w * dScale + 0.5);
	new_H = (size_t)floor(h * dScale + 0.5);

	ScImage.CreateImage(new_W, new_H, bpp);

	if( NULL == ScImage.pData )
		return ScImage;
	memset(ScImage.pData, 0, ScImage.stride*new_H);
	switch( bpp )
	{
	case PIX_BYTE:
		Scale2_tx<BYTE>((LPBYTE)SrcImage.pData, (LPBYTE)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	case PIX_CHAR:
		Scale2_tx<CHAR>((LPSTR)SrcImage.pData, (LPSTR)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	case PIX_WORD:
		Scale2_tx<WORD>((LPWORD)SrcImage.pData, (LPWORD)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	case PIX_SHORT:
		Scale2_tx<SHORT>((SHORT*)SrcImage.pData, (SHORT*)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	case PIX_DWORD:
		Scale2_tx<DWORD>((LPDWORD)SrcImage.pData, (LPDWORD)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	case PIX_LONG:
		Scale2_tx<LONG>((LPLONG)SrcImage.pData, (LPLONG)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	case PIX_FLOAT:
		Scale2_tx<FLOAT>((LPFLOAT)SrcImage.pData, (LPFLOAT)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	case PIX_DOUBLE:
		Scale2_tx<DOUBLE>((LPDOUBLE)SrcImage.pData, (LPDOUBLE)ScImage.pData, w, h, stride_src,
			new_W, new_H, ScImage.stride, 1.0 / dScale);
		break;
	}

	return ScImage;
}

template <typename _Tx>
void Scale_tx(_Tx *src, _Tx *dst, size_t w, size_t h,
			  size_t stride_src, size_t stride_dst,
			  size_t Scale/*, size_t PixSize*/)
{
	double sum;
	size_t i, j, X, Y, x, y, sx, sy, Pixels;
	_Tx *ptr, *ptr2;
	size_t nPixSz = sizeof(_Tx);

	ptr = (_Tx*)src;

	for(Y = 0, y = 0; y < h; y+=Scale, Y++)
	{
		sy = (y + Scale < h) ? Scale : h - y;
		for(X = 0, x = 0; x < w; x += Scale, X++)
		{
			sx = (x+Scale < w) ? Scale : w - x;
			Pixels = 0;
			sum = 0.0;
			ptr2 = (_Tx*)(((LPBYTE)src) + y*stride_src + x*nPixSz/*PixSize*/);
			for(j = 0; j < sy; j++)
			{
				for(i = 0; i < sx; i++)
				{
					Pixels++;
					sum += ptr2[i];
				}
				ptr2 = (_Tx*)(((LPBYTE)ptr2) + stride_src);
			}
			dst[X] = (_Tx)ceil(sum / Pixels);
		}
		dst = (_Tx*)(((LPBYTE)dst) + stride_dst);
		ptr = (_Tx*)(((LPBYTE)ptr) + stride_src*Scale);
	}
}

IMAGE IMAGE::GetScaledImage(double dScale) const
{
	IMAGE ScImage;
	double dIntScale;
	bool bDelete = true;

	dIntScale = (int)floor(dScale);
	if( 0 == dIntScale )
	{
		dIntScale = 1;
	}
	try
	{
		if( dScale > 1 )
		{
			size_t new_w, new_h;
			new_w = (size_t)ceil(w / double(dIntScale));
			new_h = (size_t)ceil(h / double(dIntScale));
			ScImage.CreateImage(new_w, new_h, npix);
			switch( npix )
			{
			case PIX_BYTE:
				Scale_tx<BYTE>((LPBYTE)pData, (LPBYTE)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(BYTE));
				break;
			case PIX_CHAR:
				Scale_tx<CHAR>((LPSTR)pData, (LPSTR)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(BYTE));
				break;
			case PIX_WORD:
				Scale_tx<WORD>((LPWORD)pData, (LPWORD)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(WORD));
				break;
			case PIX_SHORT:
				Scale_tx<SHORT>((SHORT*)pData, (SHORT*)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(WORD));
				break;
			case PIX_DWORD:
				Scale_tx<DWORD>((LPDWORD)pData, (LPDWORD)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(DWORD));
				break;
			case PIX_LONG:
				Scale_tx<LONG>((LPLONG)pData, (LPLONG)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(DWORD));
				break;
			case PIX_FLOAT:
				Scale_tx<FLOAT>((LPFLOAT)pData, (LPFLOAT)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(float));
				break;
			case PIX_DOUBLE:
				Scale_tx<DOUBLE>((LPDOUBLE)pData, (LPDOUBLE)ScImage.pData, w, h, stride,
					ScImage.stride, dIntScale);//, sizeof(float));
				break;
			}
		}
		else
		{
			ScImage = *this;
			bDelete = false;
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("GetScaledImage() -> failed to scale (1): %s"), e.what()));
	}
	try
	{
		if( ScImage.pData != NULL )
		{
			dScale = dIntScale / dScale;
			if( dScale != 1 )
			{
				IMAGE Image = ScaleImage2(ScImage, dScale);
				if( true == bDelete )
				{
					delete[] ScImage.pData;
				}
				ScImage = Image;
			}
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("GetScaledImage() -> failed to scale (2): %s"), e.what()));
	}
	return ScImage;
}

template<typename _Tx>
class CopyFromFft_t
{
public:
	typedef bool TReturn;
	TReturn Handle(const IMAGE& inImage, double dScale, double dMin, double *pSrc)
	{
		_Tx *pDst = (_Tx*)inImage.pData;

		int x, y;
		for(y = 0; y < inImage.h; y++)
		{
			for(x = 0; x < inImage.w; x++, pSrc+=2)
			{
				pDst[x] = (*pSrc - dMin) * dScale;
			}
			pDst = (_Tx*)(((LPBYTE)pDst) + inImage.stride);
		}
		return true;
	}
};

template<typename _Tx>
class CopyFft_t
{
public:
	typedef bool TReturn;
	TReturn Handle(const IMAGE& inImage, double *pDst)
	{
		const _Tx *pSrc = (_Tx*)inImage.pData; 
		int nSize = inImage.w*inImage.h;
		for(int x = 0; x < nSize; x++, pDst+=2, pSrc++)
		{
			*pDst = *pSrc;
			pDst[1] = 0;
		}
		return true;
	}
};

template<typename _Tx>
class GetPixelScaleFactor
{
public:
	typedef double TReturn;
	TReturn Handle(const IMAGE& inImage, double dMax, double dMin)
	{
		switch( inImage.npix )
		{
		case PIX_FLOAT:
		case PIX_DOUBLE:
			// we use the maximum value of DWORD here, not of a float which is ~10^38
			return double(numeric_limits<DWORD>::max() - numeric_limits<DWORD>::min()) / (dMax - dMin);

		default:
			return double(numeric_limits<_Tx>::max() - numeric_limits<_Tx>::min()) / (dMax - dMin);
		}
	}
};

bool IMAGE::AutoCorrelate(IMAGE &CorrImage)
{
	bool bNormalize = true, bRet = false;
	int nSteps = 1 +		// 1 FFT
		+ 2*h +				// number of rows in correlation
		+ 1					// IFFT
		;
	int nCount = 0;

	try
	{
		std::vector<double> fft(w*h*2);
		// prepare ROI
		// TODO: PIX_CHAR etc.
		ProcessImage<CopyFft_t>(*this, &*fft.begin());
		// do FFT
		if( false == Fft2D_Calculate(&*fft.begin(), w, h, 1) )
		{
			OutputDebugString("AutoCorrelate() -> cannot perform FFT (1).\n");
			goto Error_Exit;
		}
		fioReportPercent( ++nCount, nSteps, TRUE );
		complexd *ptr;
		if( true ==  bNormalize )
		{
			ptr = (complexd*)&*fft.begin();
			double len;
			// NOTE: we are removing the center point
			for(int y = 0; y < h; y++)
			{
				for(int x = 0; x < w; x++, ptr++)
				{
					if( (len = abs(*ptr)) > 1e-6 )
						*ptr /= len;
					else
						*ptr = 0.0;
				}
				fioReportPercent( ++nCount, nSteps, TRUE );
			}
		}
		ptr = (complexd*)&*fft.begin();
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++, ptr++)
			{
				*ptr *= *ptr;
			}
			fioReportPercent( ++nCount, nSteps, TRUE );
		}
		// do IFFT
		if( FALSE == Fft2D_Calculate(&*fft.begin(), w, h, -1) )
		{
			OutputDebugString("AutoCorrelate() -> cannot perform IFFT.\n");
			goto Error_Exit;
		}
		fioReportPercent( 0, 0, TRUE );
		double *pSrc = &*fft.begin();
		double dMin, dMax, dVal, dScale;
		dMax = -(dMin =  numeric_limits<double>::max());
		for(int i = 0; i < w*h; i++)
		{
			dVal = pSrc[i<<1];
			if( dMin > dVal ) dMin = dVal;
			if( dMax < dVal ) dMax = dVal;
		}
		CorrImage.CreateImage(w, h, npix);
		// TODO: PIX_CHAR etc.
		dScale = ProcessImage<GetPixelScaleFactor>(CorrImage, dMax, dMin);
		ProcessImage<CopyFromFft_t>(CorrImage, dScale, dMin, pSrc);

		bRet = true;
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("AutoCorrelate() -> %s"), e.what()));
	}
Error_Exit:
	fioReportPercent( 0, 0, TRUE );
	return bRet;
}

IMAGE IMAGE::CrossCorrelate(IMAGE &Image2, bool bNormalize/* = false*/)
{
	int i, max_size;
	IMAGE Image;

	max_size = __max(__max(w,Image2.w),__max(h,Image2.h));
	// is it p.o.2?
	i = 1;
	while( i < max_size )
	{
		i <<= 1;
	}

	int nSteps = 2 +		// 2 FFTs
		+ 2*i +				// number of rows in correlation
		+ 1					// IFFT
		;
	int nCount = 0;

	try
	{
		std::vector<double> fft1(w*h*2);//, fft2(w2*h2*2);
		// prepare ROI
		ProcessImage<CopyFft_t>(*this, &*fft1.begin());
		// do FFT
		if( false == Fft2D_Calculate(&*fft1.begin(), w, h, 1) )
		{
			OutputDebugString("CrossCorrelate() -> cannot perform FFT (1).\n");
			goto Error_Exit;
		}
		fioReportPercent( ++nCount, nSteps, TRUE );
		complexd *ptr;
		if( true ==  bNormalize )
		{
			ptr = (complexd*)&*fft1.begin();
			double len;
			// NOTE: we are removing the center point
			for(int y = 0; y < h; y++)
			{
				for(int x = 0; x < w; x++, ptr++)
				{
					if( (len = abs(*ptr)) > 1e-6 )
						*ptr /= len;
					else
						*ptr = 0.0;
				}
				fioReportPercent( ++nCount, nSteps, TRUE );
			}
		}
		ptr = (complexd*)&*fft1.begin();
		for(int y = 0; y < h; y++)
		{
			for(int x = 0; x < w; x++, ptr++)
			{
				*ptr *= *ptr;
			}
			fioReportPercent( ++nCount, nSteps, TRUE );
		}
		// do IFFT
		if( FALSE == Fft2D_Calculate(&*fft1.begin(), w, h, -1) )
		{
			OutputDebugString("CrossCorrelate() -> cannot perform IFFT.\n");
			goto Error_Exit;
		}
		fioReportPercent( 0, 0, TRUE );
		double *pSrc = &*fft1.begin();
		double dMin, dMax, dVal, dScale;
		dMax = -(dMin =  numeric_limits<double>::max());
		for(int i = 0; i < w*h; i++)
		{
			dVal = pSrc[i<<1];
			if( dMin > dVal ) dMin = dVal;
			if( dMax < dVal ) dMax = dVal;
		}
		Image.CreateImage(w, h, npix);
		dScale = ProcessImage<GetPixelScaleFactor>(Image, dMax, dMin);
		ProcessImage<CopyFromFft_t>(Image, dScale, dMin, pSrc);
	}
	catch(std::exception& e)
	{
		Trace(_FMT(_T("CrossCorrelate() -> %s"), e.what()));
	}
Error_Exit:
	fioReportPercent( 0, 0, TRUE );
	return Image;
}
/*
template<typename _Tx>
bool Rotate180_t(const _Tx *pSrc, UINT width, UINT height, _Tx *pDst)
{
	int nSize = width*height;
	pDst += nSize-1;
	for(int x = 0; x < nSize; x++)
	{
		*pDst-- = *pSrc++;
	}
	return true;
}

bool IMAGE::Rotate180(IMAGE &DstImage) const
{
	bool bReady = false;

	switch( npix )
	{
	case PIX_BYTE:
		bReady = Rotate180_t<BYTE>((LPBYTE)pData, w, h, (LPBYTE)DstImage.pData);
		break;
	case PIX_CHAR:
		bReady = Rotate180_t<CHAR>((LPSTR)pData, w, h, (LPSTR)DstImage.pData);
		break;
	case PIX_WORD:
		bReady = Rotate180_t<WORD>((LPWORD)pData, w, h, (LPWORD)DstImage.pData);
		break;
	case PIX_SHORT:
		bReady = Rotate180_t<SHORT>((SHORT*)pData, w, h, (SHORT*)DstImage.pData);
		break;
	case PIX_DWORD:
		bReady = Rotate180_t<DWORD>((LPDWORD)pData, w, h, (LPDWORD)DstImage.pData);
		break;
	case PIX_LONG:
		bReady = Rotate180_t<LONG>((LPLONG)pData, w, h, (LPLONG)DstImage.pData);
		break;
	case PIX_FLOAT:
		bReady = Rotate180_t<FLOAT>((LPFLOAT)pData, w, h, (LPFLOAT)DstImage.pData);
		break;
	case PIX_DOUBLE:
		bReady = Rotate180_t<DOUBLE>((LPDOUBLE)pData, w, h, (LPDOUBLE)DstImage.pData);
		break;
	}
	return bReady;
}
*/

IMAGE IMAGE::MedianFilter(int nSize/* = 3*/)
{
	IMAGE Image;
	std::vector<double> pixels(SQR(nSize));
	double *pPixels = &*pixels.begin();
//	size_t nLen, nMed;
//	cimg_library::CImg<BYTE> img1((const LPBYTE)pData, w, h, 1, 1);
/*
	cimg_library::CImg<BYTE> img1, median;

	img1.assign((const LPBYTE)pData, w, h, 1, 1);
	median = img1.get_blur_median(nSize);
	median.save_raw("test.raw");
	nLen = SQR(nSize);
	nMed = (nLen >> 1) + 1;
	std::sort(pPixels, pPixels + nSize);
*/
	Image.CreateImage(w, h, npix);
	return Image;
}

// Added by Peter on 01 Mar 2006
int IMAGE::ExtractLine(double *dst, long *wgt, double x1, double y1, double x2, double y2, int len, int width)
{
	double	x, y, dx, dy;
	int		i;
	int iw = this->w;
	int xsize = this->w;
	int ysize = this->h;

	try
	{
		x = x2 - x1;
		y = y2 - y1;
		len = ceil(FastSqrt(x*x + y*y));
		if( len < 2 ) len = 2;
		dx = y / len;
		dy = -x / len;
		if( width < 2 )
		{
			switch( this->npix )
			{
			case PIX_BYTE:
				im8GetProfile( (LPBYTE)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_CHAR:
				im8sGetProfile( (LPSTR)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_WORD:
				im16GetProfile( (LPWORD)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_SHORT:
				im16sGetProfile( (SHORT*)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_DWORD:
				im32GetProfile( (LPDWORD)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_LONG:
				im32sGetProfile( (LPLONG)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_FLOAT:
				imFGetProfile( (LPFLOAT)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			case PIX_DOUBLE:
				imDGetProfile( (LPDOUBLE)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
				break;
			default:
				MessageBox(NULL, "Extract line mode - unsupported pixel format", "EDRD ERROR!", MB_OK);
				return 0;
				break;
			}
		}
		else
		{
			x1 -= dx*width*.5;
			y1 -= dy*width*.5;
			x2 -= dx*width*.5;
			y2 -= dy*width*.5;
			for(i=0;i<width;i++)
			{
				switch( this->npix )
				{
				case PIX_BYTE:
					im8GetProfile( (LPBYTE)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				case PIX_CHAR:
					im8sGetProfile( (LPSTR)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				case PIX_WORD:
					im16GetProfile( (LPWORD)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				case PIX_SHORT:
					im16sGetProfile( (SHORT*)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				case PIX_DWORD:
					im32GetProfile( (LPDWORD)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				case PIX_LONG:
					im32sGetProfile( (LPLONG)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				case PIX_FLOAT:
					imFGetProfile( (LPFLOAT)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				case PIX_DOUBLE:
					imDGetProfile( (LPDOUBLE)pData, stride, iw, ysize, neg, x1,y1,x2,y2, len, dst, wgt);
					break;
				default:
					MessageBox(NULL, "Extract line mode - unsupported pixel format", "EDRD ERROR!", MB_OK);
					return 0;
					break;
				}
				x1+=dx; y1+=dy;
				x2+=dx; y2+=dy;
			}
		}
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("ExtractLine() -> %s"), e.what()));
	}
	return len;
}


//////////////////////////////////////////////////////////////////////////

#define		MAKEPTR(src,x,y,stride)		((_Tx*)(((LPBYTE)src) + (y) * stride))[(x)]

template <typename _Tx>
void imageGetProfile_t(const _Tx *src, int stride, int width, int height, int neg, 
					   double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	double xx, yy, pa, pb, pc, pd;
	
	if( cnt > 1 )
	{
		xx = (x1 - x0) / (cnt - 1);
		yy = (y1 - y0) / (cnt - 1);
	}
	else
	{
		xx = 0, yy = 0;
		cnt = 1;
	}
	for(int pos = 0; pos < cnt; pos++)
	{
		int x = floor(x0);
		int y = floor(y0);
		if( FastAbs(x - x0) > 0.999999 )
		{
			x = ceil(x0);
		}
		if( FastAbs(y - y0) > 0.999999 )
		{
			y = ceil(y0);
		}
// 		int x = floor(x0 + 0.5);
// 		int y = floor(y0 + 0.5);
		if( (x >= 0) && (y >= 0) && (x < width) && (y < height) )
		{
			pa = Negate(MAKEPTR(src,x,y,stride), neg);
			if( x + 1 < width ) pb = Negate(MAKEPTR(src,x+1,y,stride), neg);
			else pb = pa;
			if( y + 1 < height )
			{
				pc = Negate(MAKEPTR(src,x,y+1,stride), neg);
				if( x + 1 < width ) pd = Negate(MAKEPTR(src,x+1,y+1,stride), neg);
				else pd = pc;
			}
			else
			{
				pc = pa;
				pd = pb;
			}
			double dx = x0 - x;
			double dy = y0 - y;
			dst[pos] = (1.0 - dx) * ((1.0 - dy) * pa + dy * pc) + dx * ((1.0 - dy) * pb + dy * pd);
			wgt[pos]++;
		}
		else
		{
			dst[pos] = 0;
		}
		x0 += xx;
		y0 += yy;
	}
}

void im8GetProfile(LPBYTE src, int stride, int width, int height, int neg, 
				   double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}

void im8sGetProfile(LPSTR src, int stride, int width, int height, int neg, 
				   double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}

void im16GetProfile(LPWORD src, int stride, int width, int height, int neg, 
				   double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}

void im16sGetProfile(SHORT *src, int stride, int width, int height, int neg, 
					double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}

void im32GetProfile(LPDWORD src, int stride, int width, int height, int neg, 
					double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}

void im32sGetProfile(LONG *src, int stride, int width, int height, int neg, 
					double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}

void imFGetProfile(FLOAT *src, int stride, int width, int height, int neg, 
					double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}

void imDGetProfile(DOUBLE *src, int stride, int width, int height, int neg, 
					double x0, double y0, double x1, double y1, int cnt, LPDOUBLE dst, LPLONG wgt)
{
	imageGetProfile_t(src, stride, width, height, neg, x0, y0, x1, y1, cnt, dst, wgt);
}
