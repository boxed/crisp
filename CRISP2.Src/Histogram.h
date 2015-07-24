#pragma once

class Histogram
{
public:
	Histogram();
	~Histogram();

	double	dScaleX;		// the scale of the x-axis
	double	dScaleY;		// data-scale (y-axis)
	double	dMinX;			// the min value of the x-axis
	double	dMaxX;			// the max value of the x-axis
	int		xw;
	int		yw;
	int		hsize;			// histogram size
	LPLONG	hist;
	double	_left;
	double	_right;
	double	dZoomY;
	BOOL	bZoomX;
	BOOL	bZoomY;
	int		lclick, rclick, tclick;
	int		x, y;		// Click position
	BOOL	bSmooth;	// Smoothing
	double	mean;
	double	rms;
	double	dva;
	int		fwidth;
	SCROLLBAR SB_Left, SB_Right, SB_Top;

	int ConvertX(double _x)
	{
 		return _x * (dMaxX - dMinX) / double(hsize - 1) + dMinX;
	}
	int ConvertFromX(double _x)
	{
		return (_x - dMinX) * double(hsize - 1) / (dMaxX - dMinX);
	}

	template <typename _Tx>
		double GetBoxHisto_t(_Tx *img, int imgw, int imgh, int _stride, DWORD neg,
		int x, int y, int xs, int ys)
	{
		typedef _Tx type;
		typedef type* pointer;
		pointer ptr = pointer(LPBYTE(img) + _stride * y) + x, ptr2 = ptr;
		double tVal, _max_, dSum = 0.0;
		int _x, _y, pos;

		_max_ = numeric_limits<_Tx>::max();
		dMinX = _max_;
		dMaxX = (true == numeric_traits<_Tx>::is_integral) ? numeric_limits<_Tx>::min() : -dMinX;
		// find Min-Max inside the box
		if( 0 == neg )
		{
			for(_y = 0; _y < ys; _y++)
			{
				for(_x = 0; _x < xs; _x++)
				{
					tVal = ptr2[_x];
					dSum += tVal;
					if( dMinX > tVal ) { dMinX = tVal; }
					if( dMaxX < tVal ) { dMaxX = tVal; }
				}
				ptr2 = pointer(LPBYTE(ptr2) + _stride);
			}
			// the scale factor for the histogram offset
			dScaleX = double(hsize - 1) / (dMaxX - dMinX);
			ptr2 = ptr;
			for(_y = 0; _y < ys; _y++)
			{
				for(_x = 0; _x < xs; _x++)
				{
					pos = (int)((ptr2[_x] - dMinX) * dScaleX);
					hist[pos]++;
				}
				ptr2 = pointer(LPBYTE(ptr2) + _stride);
			}
		}
		else
		{
			// integral types
			if( true == numeric_traits<_Tx>::is_integral )
			{
				for(_y = 0; _y < ys; _y++)
				{
					for(_x = 0; _x < xs; _x++)
					{
						tVal = _max_ - ptr2[_x];
						dSum += tVal;
						if( dMinX > tVal ) { dMinX = tVal; }
						if( dMaxX < tVal ) { dMaxX = tVal; }
					}
					ptr2 = pointer(LPBYTE(ptr2) + _stride);
				}
				// the scale factor for the histogram offset
				dScaleX = double(hsize - 1) / (dMaxX - dMinX);
				ptr2 = ptr;
				for(_y = 0; _y < ys; _y++)
				{
					for(_x = 0; _x < xs; _x++)
					{
						pos = (int)((_max_ - ptr2[_x] + 1 - dMinX) * dScaleX);
						hist[pos]++;
					}
					ptr2 = pointer(LPBYTE(ptr2) + _stride);
				}
			}
			else
			{
				// real types
				for(_y = 0; _y < ys; _y++)
				{
					for(_x = 0; _x < xs; _x++)
					{
						tVal = -ptr2[_x];
						dSum += tVal;
						if( dMinX > tVal ) { dMinX = tVal; }
						if( dMaxX < tVal ) { dMaxX = tVal; }
					}
					ptr2 = pointer(LPBYTE(ptr2) + _stride);
				}
				// the scale factor for the histogram offset
				dScaleX = double(hsize - 1) / (dMaxX - dMinX);
				ptr2 = ptr;
				for(_y = 0; _y < ys; _y++)
				{
					for(_x = 0; _x < xs; _x++)
					{
						pos = (int)((-ptr2[_x] + 1 - dMinX) * dScaleX);
						hist[pos]++;
					}
					ptr2 = pointer(LPBYTE(ptr2) + _stride);
				}
			}
		}
		mean = dSum / (imgw * imgh);
		return *std::max_element(hist, hist + hsize);
	}
	//****************************************************************************//
	template <typename _Tx>
		void imGetRMS_t(_Tx *src, int _stride, int x0, int y0, int xs, int ys, double mean, double &rms, double &adev)
	{
		_Tx *ptr = (_Tx*)(((LPBYTE)src) + x0 + y0*_stride);
		double val;
		int x, y;
		__int64 count = 0;
		rms = adev = 0;
		for(y = 0; y < ys; y++)
		{
			for(x = 0; x < xs; x++, count++)
			{
				val = ptr[x] - mean;
				rms += val*val;
				adev += fabs(val);
			}
			ptr = (_Tx*)(((LPBYTE)ptr) + _stride);
		}
		if( count > 0 )
		{
			rms = FastSqrt(rms / count);
			adev = adev / count;
		}
	}

	template <typename _Tx>
		void UpdateHist_t(_Tx *src, OBJ* pObj, int x0, int y0, int xs, int ys)
	{
		int i;

		memset(hist, 0, hsize * sizeof(long));
		dScaleY = GetBoxHisto_t(src, pObj->x, pObj->y, pObj->stride, 0, x0, y0, xs, ys);
		for(i = hsize; i >= 0; i--)
		{
			if( hist[i] ) break;
		}
		fwidth = i;
		for(i = 0; i < hsize+1; i++)
		{
			if( hist[i] ) break;
		}
		fwidth -= i;
		imGetRMS_t(src, pObj->stride, x0, y0, xs, ys, mean, rms, dva);
	}

	void UpdateHist(HWND hWnd, OBJ* O);
	void TrackHistoMenu(HWND hWnd);
	void PaintHist(HWND hWnd, OBJ* O);
};

typedef Histogram* LPHIST;

__forceinline LPHIST GetHIST(OBJ* pObj) { return (LPHIST)pObj->dummy; }
