/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.FFTmap * by ML*****************************************************/
/****************************************************************************/

#include "StdAfx.h"

// #include <CommDlg.h>

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"

#include "fft.h"

// Defaults
#define	imgN	5	// field width
#define	imgN16	7	// field width for 16bits images
#define	imgN32	8	// field width for 32bits images
#define	imgNF	8	// field width for float images
#define	imgND	12	// field width for double images
#define	imgX	6	// number of fields / 2 on X
#define	imgY	6	// number of fields / 2 on Y

#define	fftN	8	// field width
#define	fftX	4	// number of fields / 2 on X
#define	fftY	4	// number of fields / 2 on Y

class DIGMAP
{
public:
	DIGMAP()
	{
		ZeroMemory(this, sizeof(this));
	}
	~DIGMAP()
	{

	}

	BOOL InitFMapObject( HWND hWnd, OBJ* obj );
	void SmallTextPrint(HDC hdc, int x, int y, LPCTSTR str, COLORREF fore, COLORREF back);
	BOOL ChangeFont( HWND hWnd, WPARAM wP );
	void AdjustDigiMap( HWND hWnd, LPWINDOWPOS wp);
	void UpdateFMap( HWND hWnd, FILE *pOut = NULL );

	int		m_nFontWidth, m_nFontHeight;	// Font dimensions
	HFONT	m_hFont;						// Handle to font
	WORD	fidc;							// Sys menu item id for resp. font
	int		fieldWidth;						// field width
	int		halfWidth, halfHeight;			// # of fields / 2 on X and Y
	WORD	ID;
	OBJ		*m_pObj;
};

/****************************************************************************/
/* Digital Map window ***********************************************************/
/****************************************************************************/
void DIGMAP::SmallTextPrint(HDC hdc, int x, int y, LPCTSTR str, COLORREF fore, COLORREF back/*, DIGMAP *S*/)
{
	HBRUSH hBrush;
	hBrush = CreateSolidBrush(back);
	SetBkColor  (hdc, back);
	SetTextColor(hdc, fore);
	x = x*this->m_nFontWidth;
	y = y*this->m_nFontHeight;
	int len = strlen(str);
	RECT rect = {x, y, x+len*this->m_nFontWidth, y+this->m_nFontHeight};
	FillRect(hdc, &rect, hBrush);
	DrawText(hdc, str, len, &rect, DT_RIGHT);
	::DeleteBrush(hBrush);
}
/****************************************************************************/
// Added by Peter on 6 Aug 2003
BOOL BrowseForFile(HWND hOwner, LPTSTR pszOutputFile, DWORD nMaxFName)
{
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(ofn));
#if (_WIN32_WINNT >= 0x0500)
	// if order to avoid Windows NT4,95/98 misunderstanding
	DWORD dwVersion;
	dwVersion = ::GetVersion();
	ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400W;
	if (dwVersion < 0x80000000)              // Windows NT/2000/XP
	{
		if( (dwVersion & 0xFF) > 4 )
		{
			ofn.lStructSize = sizeof(ofn);
		}
	}
#else
	ofn.lStructSize = sizeof(ofn);
#endif
	static TCHAR filter[] = _T("Text files\0*.txt\0\0");
	ofn.hwndOwner = hOwner;
	ofn.lpstrFilter = filter;
	ofn.lpstrFileTitle = pszOutputFile;
	ofn.nMaxFileTitle = nMaxFName;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	return GetSaveFileName(&ofn);
}
/****************************************************************************/
void DIGMAP::UpdateFMap( HWND hWnd, /*OBJ* obj, */FILE *pOut /*= NULL*/ )
{
	int		h, k, i, j, cn;
//	WORD	dat, maxw, minw;
	int		h0, k0, hh, kk, s2;
	double	a, p, mina,maxa;
	HDC		hdc = GetDC( hWnd );
	OBJ*	pr = OBJ::GetOBJ( m_pObj->ParentWnd );	// Exists for sure
	LPFFT   Spr = (LPFFT) &pr->dummy;

	if( NULL != m_pObj->sysmenu )
	{
		CheckMenuItem(m_pObj->sysmenu, this->fidc, MF_BYCOMMAND|MF_CHECKED);
	}

	static COLORREF	nf[4] = {RGB(0,0,0), RGB(0,128,0), RGB(0,0,255), RGB(255,0,0)}, // Normal fore
		af[4] = {RGB(0,0,0), RGB(0,128,0), RGB(0,0,255), RGB(255,0,0)}, // Active fore
		nb = RGB(192,192,192), 	/* Normal 			 back	*/
		ab = RGB(255,255,255), 	/* Active (on cross) back	*/
		cf = RGB(255,255,255), 	/* Comment			 fore	*/
		cb = RGB(128,128,128); 	/* Comment			 back	*/

// 	SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));//this->m_hFont);
	SelectObject(hdc, this->m_hFont);

	switch ( pr->ID )
	{
	case OT_FFT:
		h0 = hh = m_pObj->xo;
		k0 = kk = m_pObj->yo;
		s2 = Spr->fs/2;
		if ((abs(h0)<=s2) && (abs(k0)<=s2))
		{
			if ( h0+this->halfWidth >= s2  ) h0 =  s2 - this->halfWidth - 1;
			if ( k0+this->halfHeight >= s2  ) k0 =  s2 - this->halfHeight - 1;
			if ( h0-this->halfWidth < -s2 )  h0 = -s2 + this->halfWidth;
			if ( k0-this->halfHeight < -s2 )  k0 = -s2 + this->halfHeight;

			Cpyo(m_pObj, pr, ui);		// FFT size
			Cpyo(m_pObj, pr, bp);		// HFFT
			this->SmallTextPrint(hdc, 0,            0, " Ampl ", nf[0], nb/*, this */);
			this->SmallTextPrint(hdc, 0,(this->halfHeight*2+1+1), " Phas ", nf[0], nb/*, this */);
			for ( i=1,h=h0-this->halfWidth; i<=(this->halfWidth*2+1); i++,h++)
			{
				sprintf(TMP, "%7d ", h);
// 				sprintf(TMP, "%5d ", h);
				this->SmallTextPrint(hdc, i*this->fieldWidth,             0, TMP, cf, cb/*, this*/);
				this->SmallTextPrint(hdc, i*this->fieldWidth, (this->halfHeight*2+1+1), TMP, cf, cb/*, this*/);
			}
			mina=1e30; maxa=0.;
			for ( k=k0-this->halfHeight, j=1; k<=k0+this->halfHeight; k++,j++)
			{
				for ( h=h0-this->halfWidth, i=1; h<=h0+this->halfWidth; h++,i++)
				{
					{ FCOMPLEX m_nFontWidth = Spr->ft2.GetVal(h,k); a=abs(m_nFontWidth); p=atan2(m_nFontWidth); }
					if (a>maxa) maxa=a;
					if (a<mina) mina=a;
				}
			}
			if(mina+1e-7>maxa) maxa = mina+1.;
			maxa = maxa-mina;
			for ( k=k0-this->halfHeight, j=1; k<=k0+this->halfHeight; k++,j++)
			{
// 				sprintf( TMP, "%5d ", k);
				sprintf( TMP, "%7d ", k);
				this->SmallTextPrint(hdc, 0, j,               TMP, cf, cb/*, this*/);
				this->SmallTextPrint(hdc, 0, j+(this->halfHeight*2+1+1), TMP, cf, cb/*, this*/);
				for ( h=h0-this->halfWidth, i=1; h<=h0+this->halfWidth; h++,i++)
				{
					{
						FCOMPLEX m_nFontWidth = Spr->ft2.GetVal(h,k);
						a=abs(m_nFontWidth);
						p=atan2(m_nFontWidth);
					}
// 					sprintf( TMP, "%5.0f ", a*1e-3);
					sprintf( TMP, "%7.0f ", a*1e-3);
//					if( a > 1e5 )
//						sprintf( TMP, "%5.1fe6 ", a*1e-6);
//					else
//						sprintf( TMP, "%5.0f ", a);
					if ((maxa>2.) && (a-mina)>2.)
						cn = (int)((log(a-mina)/log(maxa)*.4 + (a-mina)/maxa)/1.4 *4);
					else cn=0;
					if (cn>3) cn=3;
					if ( k==kk || h==hh)
						this->SmallTextPrint(hdc, i*this->fieldWidth, j, TMP, af[cn], ab/*, this*/);
					else
					    this->SmallTextPrint(hdc, i*this->fieldWidth, j, TMP, nf[cn], nb/*, this*/);
// 					sprintf( TMP, " %4.0f ", R2D(p));
					sprintf( TMP, " %6.0f ", R2D(p));
					if ( k==kk || h==hh)
						this->SmallTextPrint(hdc, i*this->fieldWidth, j+(this->halfHeight*2+1+1), TMP, af[0], ab/*, this*/);
					else
						this->SmallTextPrint(hdc, i*this->fieldWidth, j+(this->halfHeight*2+1+1), TMP, nf[0], nb/*, this*/);
				}
			}
		}
		break;

	case OT_IMG:
		{
			LPBYTE d = pr->dp;
			int nStride = pr->stride;
			int	x =  m_pObj->xo * pr->scd / pr->scu + pr->xo;
			int	y =  m_pObj->yo * pr->scd / pr->scu + pr->yo;
			hh = x, kk = y;
			double	dat, maxw, minw;

			if ( x+this->halfWidth >= pr->x ) x = pr->x - this->halfWidth - 1;
			if ( y+this->halfHeight >= pr->y ) y = pr->y - this->halfHeight - 1;
			if ( x < this->halfWidth ) x = this->halfWidth;
			if ( y < this->halfHeight ) y = this->halfHeight;

			this->SmallTextPrint(hdc, 0, 0, "Int.   ", nf[0], nb);
			for (h=x-this->halfWidth, i=1; h<=x+this->halfWidth; h++, i++)
			{
				sprintf(TMP, "%*d", this->fieldWidth, h);
				this->SmallTextPrint(hdc, i*this->fieldWidth, 0, TMP, cf, cb);
			}
			maxw = -(minw = numeric_limits<double>::max());
			for(k = y - this->halfHeight, j = 1; k<=y+this->halfHeight; k++, j++)
			{
				for(h = x - this->halfWidth, i = 1; h<=x+this->halfWidth; h++, i++)
				{
					switch( m_pObj->npix )
					{
					case PIX_BYTE:   dat = *GetDataPtr<BYTE>(d, h, k, nStride); break;
					case PIX_CHAR:   dat = *GetDataPtr<CHAR>((LPSTR)d, h, k, nStride); break;
					case PIX_WORD:   dat = *GetDataPtr<WORD>((LPWORD)d, h, k, nStride); break;
					case PIX_SHORT:  dat = *GetDataPtr<SHORT>((SHORT*)d, h, k, nStride); break;
					case PIX_DWORD:  dat = *GetDataPtr<DWORD>((LPDWORD)d, h, k, nStride); break;
					case PIX_LONG:   dat = *GetDataPtr<LONG>((LPLONG)d, h, k, nStride); break;
					case PIX_FLOAT:	 dat = *GetDataPtr<FLOAT>((LPFLOAT)d, h, k, nStride); break;
					case PIX_DOUBLE: dat = *GetDataPtr<DOUBLE>((LPDOUBLE)d, h, k, nStride); break;
					}
					if( minw > dat ) minw = dat;
					if( maxw < dat ) maxw = dat;
				}
			}
			maxw -= minw;
			maxw = maxw/4+1;
			double dLimit1 = pow(10.0, 6.0);//this->fieldWidth);
			for(k = y - this->halfHeight, j = 1; k <= y + this->halfHeight; k++, j++)
			{
// 				sprintf( TMP, "%*d", this->fieldWidth, k);
// 				this->SmallTextPrint(hdc, 0, j,    TMP, cf, cb/*, this*/);
				this->SmallTextPrint(hdc, 0, j,    _tstd::format("%*d", this->fieldWidth, k).c_str(), cf, cb/*, this*/);
				for(h = x - this->halfWidth, i = 1; h <= x + this->halfWidth; h++, i++)
				{
					switch( m_pObj->npix )
					{
					case PIX_BYTE:
						{
							sprintf( TMP, " %*u", this->fieldWidth-1, (int)(dat = *GetDataPtr<BYTE>(d, h, k, nStride)) );
							break;
						}
					case PIX_CHAR:
						{
							sprintf( TMP, " %*d", this->fieldWidth-1, (int)(dat = *GetDataPtr<CHAR>((LPSTR)d, h, k, nStride))  );
							break;
						}
					case PIX_WORD:
						{
							sprintf( TMP, " %*u", this->fieldWidth-1, (int)(dat = *GetDataPtr<WORD>((LPWORD)d, h, k, nStride)) );
							break;
						}
					case PIX_SHORT:
						{
							sprintf( TMP, " %*d", this->fieldWidth-1, (int)(dat = *GetDataPtr<SHORT>((SHORT*)d, h, k, nStride)) );
							break;
						}
					case PIX_DWORD:
						{
							sprintf( TMP, " %*g", this->fieldWidth-1, dat = *GetDataPtr<DWORD>((LPDWORD)d, h, k, nStride) );
							break;
						}
					case PIX_LONG:
						{
							sprintf( TMP, " %*g", this->fieldWidth-1, dat = *GetDataPtr<LONG>((LPLONG)d, h, k, nStride) );
							break;
						}
					case PIX_FLOAT:
						{
							dat = *GetDataPtr<FLOAT>((LPFLOAT)d, h, k, nStride);
							if( dat >= dLimit1 )
							{
								sprintf( TMP, " %g", /*this->fieldWidth-1, */dat );
								TMP[6] = 'e';
								TMP[7] = TMP[strlen(TMP)-1];
								TMP[8] = 0;
							}
							else
							{
								sprintf( TMP, " %7g", /*this->fieldWidth-1, */dat );
							}
							break;
						}
					case PIX_DOUBLE:
						{
							dat = *GetDataPtr<DOUBLE>((LPDOUBLE)d, h, k, nStride);
							if( dat >= dLimit1 )
							{
								sprintf( TMP, " %g", /*this->fieldWidth-1, */dat );
								TMP[6] = 'e';
								TMP[7] = TMP[strlen(TMP)-1];
								TMP[8] = 0;
							}
							else
							{
								sprintf( TMP, " %11g", /*this->fieldWidth-1, */dat );
							}
							break;
						}
					}
//					sprintf( TMP, " %*u", this->fieldWidth-1, dat );
					if( dat == 0xFFFF )
						maxw = maxw;
					cn = (dat - minw)/maxw;
					if ((h == hh) || (k == kk))
						this->SmallTextPrint(hdc, i*this->fieldWidth, j, TMP, af[cn], ab/*, this*/);
					else
						this->SmallTextPrint(hdc, i*this->fieldWidth, j, TMP, nf[cn], nb/*, this*/);
					if( pOut )
						_ftprintf(pOut, _T("%s\t"), TMP);
				}
				if( pOut )
					_ftprintf(pOut, _T("\n"));
			}
		}
		break;
	}
	ReleaseDC( hWnd, hdc );
	ValidateRect( hWnd, NULL);
}
/****************************************************************************/
BOOL DIGMAP::ChangeFont( HWND hWnd, WPARAM wP)
{
	RECT	r;

#ifdef USE_FONT_MANAGER
	switch (wP & 0xFFF0)
	{
	case SC_FSMALL:
		this->m_hFont = FontManager::GetFont(FM_SMALL_FONT, this->m_nFontWidth, this->m_nFontHeight);
		break;
	case SC_FMEDIUM:
		this->m_hFont = FontManager::GetFont(FM_MEDIUM_FONT, this->m_nFontWidth, this->m_nFontHeight);
		break;
	case SC_FLARGE:
		this->m_hFont = FontManager::GetFont(FM_LARGE_FONT, this->m_nFontWidth, this->m_nFontHeight);
		break;
	default:	return FALSE;
	}
#else
	switch (wP & 0xFFF0)
	{
	case SC_FSMALL:
		this->m_nFontWidth = fnts_w;
		this->m_nFontHeight = fnts_h;
		this->m_hFont = hfSmall;
		break;
	case SC_FMEDIUM:
		this->m_nFontWidth = fntm_w;
		this->m_nFontHeight = fntm_h;
		this->m_hFont = hfMedium;
		break;
	case SC_FLARGE:
		this->m_nFontWidth = fntl_w;
		this->m_nFontHeight = fntl_h;
		this->m_hFont = hfLarge;
		break;
	default:	return FALSE;
	}
#endif
	CheckMenuItem(m_pObj->sysmenu, this->fidc, MF_BYCOMMAND|MF_UNCHECKED);
	this->fidc = wP & 0xFFF0;
	CheckMenuItem(m_pObj->sysmenu, this->fidc, MF_BYCOMMAND|MF_CHECKED);
	GetWindowRect( hWnd, &r);

	SetWindowPos ( hWnd, 0, 0, 0, r.right-r.left, r.bottom-r.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );
	InvalidateRect( hWnd, NULL, FALSE );
	return TRUE;
}
/****************************************************************************/
void DIGMAP::AdjustDigiMap( HWND hWnd, LPWINDOWPOS wp )
{
	int		xs = wp->cx, ys = wp->cy;
	int		ix, iy;
	RECT	r={0,0,0,0};
	int		width, height;

	//Trace(_FN("AdjustDigiMap: x:%1%, y:%2%, cx:%3%, cy:%4%, S->m_nFontWidth:%5%, S->fieldWidth:%6%") % wp->x % wp->y % wp->cx % wp->cy % S->m_nFontWidth % S->fieldWidth);

	// S->halfWidth is the number of numbers to print on the x axis from the center in the digimap, so the total number of digits is S->halfWidth*2
	// S->halfHeight is the same as S->halfWidth on the y axis
	// when viewing a FFT the display shows ampl and phase for every pixel, that's why there are a lot of multiplication by two below

	AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
	width = r.right - r.left;
	height = r.bottom - r.top;

	if (xs!=0)
	{
		Trace(Format("AdjustDigiMap %1% %2%") % m_nFontWidth % fieldWidth);
		ix = ((xs-width)/(this->m_nFontWidth*this->fieldWidth) - 2)/2;
		if (ix<=0) ix = 1;
		if (ix>(m_pObj->x-1)/2) ix = (m_pObj->x-1)/2;
		this->halfWidth = ix;
	}
	xs = this->m_nFontWidth*this->fieldWidth*(this->halfWidth*2+1+1)+width;
	if (ys!=0)
	{
		if (this->ID == OT_FFT) iy = ((ys-height)/(this->m_nFontHeight*2) - 2)/2;
		else				 iy = ((ys-height)/(this->m_nFontHeight) - 2)/2;
		if (iy<=0) iy = 1;
		if (iy>(m_pObj->y-1)/2) iy = (m_pObj->y-1)/2;
		this->halfHeight = iy;
	}
	if (this->ID == OT_FFT) ys = this->m_nFontHeight * (this->halfHeight*2+1+1)*2 + height;
	else				 ys = this->m_nFontHeight * (this->halfHeight*2+1+1)   + height;

	if (xs <= 100)
		xs = 400;
	if (ys <= 100)
		ys = 400;

	wp->cx = xs; wp->cy = ys;
}
/****************************************************************************/
BOOL DIGMAP::InitFMapObject( HWND hWnd, OBJ* obj )
{
	OBJ*	pr  = OBJ::GetOBJ(obj->ParentWnd);
	int		xs, ys;
	RECT	r={0,0,0,0};

	obj->xo = obj->yo = 0;		// Initial map positions

	this->m_pObj = obj;

	this->fidc = SC_FMEDIUM;
#ifdef USE_FONT_MANAGER
	this->m_hFont = FontManager::GetFont(FM_MEDIUM_FONT, this->m_nFontWidth, this->m_nFontHeight);
#else
	this->m_hFont = hfMedium;
	this->m_nFontWidth = fntm_w;
	this->m_nFontHeight = fntm_h;
#endif

	switch( pr->ID )
	{
		case OT_FFT:
//			this->m_nFontWidth = fntm_w;
//			this->m_nFontHeight = fntm_h;
			this->fieldWidth = fftN;
			this->halfWidth = fftX;
			this->halfHeight = fftY;
			this->ID = OT_FFT;
			ys = this->m_nFontHeight * (this->halfHeight*2+1+1)*2;
			break;
		case OT_IMG:
//			this->m_nFontWidth = fntm_w;
//			this->m_nFontHeight = fntm_h;
			switch( obj->npix )
			{
			case PIX_BYTE:
			case PIX_CHAR:
				this->fieldWidth = imgN;
				break;
			case PIX_WORD:
			case PIX_SHORT:
				this->fieldWidth = imgN16;
				break;
			case PIX_DWORD:
			case PIX_LONG:
				this->fieldWidth = imgN32;
				break;
			case PIX_FLOAT:
				this->fieldWidth = imgNF;
				break;
			case PIX_DOUBLE:
				this->fieldWidth = imgND;
				break;
			default:
				return FALSE;
			}
//			if (obj->pix==1) this->fieldWidth = imgN;
//			if (obj->pix==2) this->fieldWidth = imgN16;
			this->halfWidth = imgX;
			this->halfHeight = imgY;
			this->ID = OT_IMG;
			ys = this->m_nFontHeight * (this->halfHeight*2+1+1);
			break;
	}

	xs = this->m_nFontWidth*this->fieldWidth*(this->halfWidth*2+1+1);
	SetRect( &r, 0,0, xs, ys);
	AdjustWindowRectEx( &r, GetWindowLong(hWnd,GWL_STYLE), FALSE, GetWindowLong(hWnd,GWL_EXSTYLE));
	xs = r.right - r.left;
	ys = r.bottom - r.top;

	SetWindowPos ( hWnd, 0, 0, 0, xs, ys, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE );

	// added by Peter on 3 Feb 2003
	if( NULL != obj->sysmenu )
	{
		CheckMenuItem(obj->sysmenu, this->fidc, MF_BYCOMMAND|MF_CHECKED);
	}
	return TRUE;
}
/****************************************************************************/
LRESULT CALLBACK DigiWndProc (HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	OBJ*	pObj = OBJ::GetOBJ(hWnd);
	if( NULL == pObj )
	{
		return -1;
	}
	DIGMAP* pDigiMap = (DIGMAP*)&pObj->dummy;
	Trace(Format("DigWndProc %1%") % (int)pDigiMap);

	switch(message)
	{
	case WM_CREATE:
		// ADDED BY PETER ON 29 JUNE 2004
		// VERY IMPORTATNT CALL!!! EACH DIALOG MUST CALL IT HERE
		// BE CAREFUL HERE, SINCE SOME POINTERS CAN DEPEND ON OBJECT (lParam) value!
		//pObj = (OBJ*)lParam;
		//InitializeObjectDialog(hWnd, pObj);

		if ( ! pDigiMap->InitFMapObject( hWnd, pObj )) return -1;
		break;

	case WM_NCDESTROY:
		pDigiMap->~DIGMAP();
		objFreeObj( hWnd );
		break;

	case FM_ParentInspect:
		pObj->xo = LOINT(lParam);
		pObj->yo = HIINT(lParam);
	case FM_Update:
	case WM_PAINT:
		pDigiMap->UpdateFMap( hWnd );
    	break;

	case FM_UpdateNI:
		pObj->NI = OBJ::GetOBJ(pObj->ParentWnd)->NI;
    	objInformChildren( hWnd, FM_UpdateNI, wParam, lParam );
    	break;

	case WM_WINDOWPOSCHANGING:
		pDigiMap->AdjustDigiMap( hWnd, (LPWINDOWPOS)lParam );
		return 0;

	case WM_SYSCOMMAND:
		if ( pDigiMap->ChangeFont( hWnd, wParam )) return 0;
		break;

	// ADDED by Peter 06 Aug 2003
	case IDM_F_SAVE:
	case IDM_F_SSAVE:
		static TCHAR szBuffer[_MAX_PATH];
		if( TRUE == BrowseForFile(hWnd, szBuffer, _MAX_PATH) )
		{
			TCHAR drv[_MAX_DRIVE], dir[_MAX_DIR], fname[_MAX_FNAME], ext[_MAX_EXT];
			_tsplitpath(szBuffer, drv, dir, fname, ext);
			_tmakepath(szBuffer, drv, dir, fname, _T(".txt"));
			FILE *s_FileOut;
			if( NULL != (s_FileOut = _tfopen(szBuffer, _T("wt"))) )
			{
				pDigiMap->UpdateFMap( hWnd, s_FileOut );
				fclose(s_FileOut);
			}
		}
		break;
		// END ADD

	case WM_ACTIVATE: if ( WA_INACTIVE == wParam ) break;
	case WM_QUERYOPEN:
	case WM_MOUSEACTIVATE:
		ActivateObj( hWnd );
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
