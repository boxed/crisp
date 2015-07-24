#pragma once

typedef enum
{
	PIX_UNKNOWN = 0,
	PIX_BYTE,
	PIX_CHAR,
	PIX_WORD,
	PIX_SHORT,
	PIX_DWORD,
	PIX_LONG,
	PIX_FLOAT,
	PIX_DOUBLE,
		
} PIXEL_FORMAT;

__forceinline BYTE Negate(BYTE val, int neg) { return (val ^ neg); }
__forceinline CHAR Negate(CHAR val, int neg) { return (0 == neg) ? val : -val; }
__forceinline WORD Negate(WORD val, int neg) { return (val ^ neg); }
__forceinline SHORT Negate(SHORT val, int neg) { return (0 == neg) ? val : -val; }
__forceinline DWORD Negate(DWORD val, int neg) { return (val ^ neg); }
__forceinline LONG Negate(LONG val, int neg) { return (0 == neg) ? val : -val; }
__forceinline float Negate(float val, int neg) { return (0 == neg) ? val : -val; }
__forceinline double Negate(double val, int neg) { return (0 == neg) ? val : -val; }

class IMAGE
{
public:
	IMAGE()
	{
		Init();
	}
	virtual ~IMAGE()
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!
		// IMAGEs don't own their data, the copy constructor and assignment operator will just copy the pointer!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!
//		if( NULL != pData )
//		{
//			delete[] pData;
//			pData = NULL;
//		}
	}

	IMAGE &operator=(const IMAGE &image);

	void Init();
	void SetData(LPVOID _data, int _w, int _h, PIXEL_FORMAT _pix);
	void CreateImage(int w, int h, PIXEL_FORMAT pixfmt);
	IMAGE Get512x512RescaledImage(double* outScale = NULL) const;
	IMAGE GetScaledImage(double dScale) const;
	bool ChangeCanvas(size_t NewW, size_t NewH);
	bool AutoCorrelate(IMAGE &CorrImage);
	IMAGE CrossCorrelate(IMAGE &Image2, bool bNormalize = false);
	void SaveImage(LPCSTR pszPath);
	//bool Rotate180(IMAGE &DstImage) const;
	void CalcMinMax();
	void InvertImage();
	bool CalculateHisto(std::vector<long> &vHisto);
	bool IsNegative();
	void SetNegative(bool bNegative = false);

	int ExtractLine(double *dst, long *wgt, double x1, double y1, double x2, double y2, int len, int width);

	// returns double
	double GetPixel(int x, int y)
	{
		if( (x < 0) || (x >= this->w) || (y < 0) || (y >= this->h) )
		{
			return 0.0;
		}
		double value = 0.0;
		switch( npix )
		{
		case PIX_BYTE:	value = Negate(((BYTE*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		case PIX_CHAR:	value = Negate(((CHAR*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		case PIX_WORD:	value = Negate(((WORD*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		case PIX_SHORT:	value = Negate(((SHORT*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		case PIX_DWORD:	value = Negate(((DWORD*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		case PIX_LONG:	value = Negate(((LONG*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		case PIX_FLOAT:	value = Negate(((FLOAT*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		case PIX_DOUBLE:value = Negate(((DOUBLE*)(((LPBYTE)pData) + stride * y))[x], neg); break;
		}
		return value;
	}

	IMAGE MedianFilter(int nSize = 3);

	static void InvertImage(LPVOID _pData, PIXEL_FORMAT _npix, int _stride, int _w, int _h);
	static int PixFmt2Bpp(PIXEL_FORMAT nPixFmt);

	LPVOID	pData;
	int		w;
	int		h;
	int		stride;
	int		neg;
	double	m_dMin;
	double	m_dMax;
	double	m_dIMin;
	double	m_dIMax;
	double	m_dIMinA;
	double	m_dIMaxA;
	PIXEL_FORMAT npix;
};

class OBJ;
void PaintImage(HWND hWnd, OBJ* pObj);

template <typename _Tx>
_Tx *GetDataPtr(_Tx *ptr, int x, int y, int stride)
{
	return (_Tx*)(((LPBYTE)ptr) + y * stride) + x;
}

/*template <class T> 
class MessageHandler
{
public:
	UINT Handle(HWND wnd, UINT message, WPARAM wparam, LPARAM lparam)
	{
		T t;
	}
};*/

#if _MSC_VER > 1200

template <template<class A> class T> 
typename T<BYTE>::TReturn ProcessImage(IMAGE& inImage)
{
	switch (inImage.npix)
	{
	default:
		throw std::exception("unknown pixel format");
	case PIX_BYTE:		{ return T<BYTE>	().Handle(inImage); }
	case PIX_CHAR:		{ return T<CHAR>	().Handle(inImage); }
	case PIX_WORD:		{ return T<WORD>	().Handle(inImage); }
	case PIX_SHORT:		{ return T<SHORT>	().Handle(inImage); }
	case PIX_DWORD:		{ return T<DWORD>	().Handle(inImage); }
	case PIX_LONG:		{ return T<LONG>	().Handle(inImage); }
	case PIX_FLOAT:		{ return T<FLOAT>	().Handle(inImage); }
	case PIX_DOUBLE:	{ return T<DOUBLE>	().Handle(inImage); }
	}
}

template <template<class A> class T, typename TParam1> 
typename T<BYTE>::TReturn ProcessImage(IMAGE& inImage, TParam1 p1)
{
	switch (inImage.npix)
	{
	default:
		throw std::exception("unknown pixel format");
	case PIX_BYTE:		{ return T<BYTE>	().Handle(inImage, p1); }
	case PIX_CHAR:		{ return T<CHAR>	().Handle(inImage, p1); }
	case PIX_WORD:		{ return T<WORD>	().Handle(inImage, p1); }
	case PIX_SHORT:		{ return T<SHORT>	().Handle(inImage, p1); }
	case PIX_DWORD:		{ return T<DWORD>	().Handle(inImage, p1); }
	case PIX_LONG:		{ return T<LONG>	().Handle(inImage, p1); }
	case PIX_FLOAT:		{ return T<FLOAT>	().Handle(inImage, p1); }
	case PIX_DOUBLE:	{ return T<DOUBLE>	().Handle(inImage, p1); }
	}
}

template <template<class A> class T, typename TParam1, typename TParam2> 
typename T<BYTE>::TReturn ProcessImage(IMAGE& inImage, TParam1 p1, TParam2 p2)
{
	switch (inImage.npix)
	{
	default:
		throw std::exception("unknown pixel format");
	case PIX_BYTE:		{ return T<BYTE>	().Handle(inImage, p1, p2); }
	case PIX_CHAR:		{ return T<CHAR>	().Handle(inImage, p1, p2); }
	case PIX_WORD:		{ return T<WORD>	().Handle(inImage, p1, p2); }
	case PIX_SHORT:		{ return T<SHORT>	().Handle(inImage, p1, p2); }
	case PIX_DWORD:		{ return T<DWORD>	().Handle(inImage, p1, p2); }
	case PIX_LONG:		{ return T<LONG>	().Handle(inImage, p1, p2); }
	case PIX_FLOAT:		{ return T<FLOAT>	().Handle(inImage, p1, p2); }
	case PIX_DOUBLE:	{ return T<DOUBLE>	().Handle(inImage, p1, p2); }
	}
}

template <template<class A> class T, typename TParam1, typename TParam2, typename TParam3> 
typename T<BYTE>::TReturn ProcessImage(IMAGE& inImage, TParam1 p1, TParam2 p2, TParam3 p3)
{
	switch (inImage.npix)
	{
	default:
		throw std::exception("unknown pixel format");
	case PIX_BYTE:		{ return T<BYTE>	().Handle(inImage, p1, p2, p3); }
	case PIX_CHAR:		{ return T<CHAR>	().Handle(inImage, p1, p2, p3); }
	case PIX_WORD:		{ return T<WORD>	().Handle(inImage, p1, p2, p3); }
	case PIX_SHORT:		{ return T<SHORT>	().Handle(inImage, p1, p2, p3); }
	case PIX_DWORD:		{ return T<DWORD>	().Handle(inImage, p1, p2, p3); }
	case PIX_LONG:		{ return T<LONG>	().Handle(inImage, p1, p2, p3); }
	case PIX_FLOAT:		{ return T<FLOAT>	().Handle(inImage, p1, p2, p3); }
	case PIX_DOUBLE:	{ return T<DOUBLE>	().Handle(inImage, p1, p2, p3); }
	}
}

template <template<class A> class T, typename TParam1, typename TParam2, typename TParam3, typename TParam4> 
typename T<BYTE>::TReturn ProcessImage(IMAGE& inImage, TParam1 p1, TParam2 p2, TParam3 p3, TParam4 p4)
{
	switch (inImage.npix)
	{
	default:
		throw std::exception("unknown pixel format");
	case PIX_BYTE:		{ return T<BYTE>	().Handle(inImage, p1, p2, p3, p4); }
	case PIX_CHAR:		{ return T<CHAR>	().Handle(inImage, p1, p2, p3, p4); }
	case PIX_WORD:		{ return T<WORD>	().Handle(inImage, p1, p2, p3, p4); }
	case PIX_SHORT:		{ return T<SHORT>	().Handle(inImage, p1, p2, p3, p4); }
	case PIX_DWORD:		{ return T<DWORD>	().Handle(inImage, p1, p2, p3, p4); }
	case PIX_LONG:		{ return T<LONG>	().Handle(inImage, p1, p2, p3, p4); }
	case PIX_FLOAT:		{ return T<FLOAT>	().Handle(inImage, p1, p2, p3, p4); }
	case PIX_DOUBLE:	{ return T<DOUBLE>	().Handle(inImage, p1, p2, p3, p4); }
	}
}

#endif
