/****************************************************************************/
/* Copyright (c) 1998 Calidris **********************************************/
/****************************************************************************/
/* CRISP2.ImageCalculator * by ML *******************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "rgconfig.h"

#include "ICalc.h"

#include "numeric_traits.h"

#pragma optimize ("",off)

/****************************************************************************/

typedef struct {
	LPSTR	name;
	void	(* func)( PREAL dst, PREAL src1, PREAL src2, int cnt );
	BOOL	b2;
} FUNC;

static	void	DummyPre(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub) {}
static	void 	DummyPost(LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd ) {}
static	void	icNone( PREAL dst, PREAL src1, PREAL src2, int cnt ) {}

void icLog   ( PREAL dst, PREAL src1, PREAL src2, int cnt );
void icInv   ( PREAL dst, PREAL src1, PREAL src2, int cnt );

static	FUNC	func[] = {
	{	"Copy",		icNone,	FALSE },
	{	"Log",		icLog,	FALSE },
	{	"Invert",	icInv,	FALSE },
	{	"Add",		icAdd,	TRUE },
	{	"Subtract",	icSub,	TRUE },
	{	"Multiply",	icMul,	TRUE },
	{	"Divide",	icDiv,	TRUE },
	{	NULL,		NULL,	FALSE }
};

typedef struct {
	LPSTR	name;
	void	(* func)(HWND);
	BOOL	b2;
} MODE;

static	MODE	mode[] = {
	{	"Fit",		NULL,	FALSE },
	{	"Clip",		NULL,	TRUE },
	{	NULL,		NULL,	FALSE }
};

PIXEL_FORMAT GetDestType(HWND hDlg)
{
	PIXEL_FORMAT nPix = PIX_BYTE;
	int nBits = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_IC_816));
	switch( nBits )
	{
	case 0: nPix = PIX_BYTE;   break;
	case 1: nPix = PIX_WORD;   break;
	case 2: nPix = PIX_DWORD;  break;
	case 3: nPix = PIX_CHAR;   break;
	case 4: nPix = PIX_SHORT;  break;
	case 5: nPix = PIX_LONG;   break;
	case 6: nPix = PIX_FLOAT;  break;
	case 7: nPix = PIX_DOUBLE; break;
	}
	return nPix;
}

static	LONG	s_lAdd;
static	REAL	fMin, fMax;
static	REAL	fAdd, fMul;
static	REAL	fMaximum;
static	REAL	fMean1, fMean2;
static	PREAL	lps1 = NULL;
static	PREAL	lps2 = NULL;
static	PREAL	lpdst = NULL;
static	WORD	wCX, wCY;
static	WORD	wShft1, wShft2;
static	int		lSbtr1, lSbtr2;
static	void	(* pres1)(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
static	void	(* pres2)(PREAL dst, LPVOID src, DWORD ofs, int iwid, int shft, int sub);
static	WORD	wAdd1, wAdd2, wAddd;
static	void	(* ariph)( PREAL dst, PREAL src1, PREAL src2, int cnt );
static	void 	(* post) (LPVOID dst, DWORD ofs, PREAL src, int wid, REAL fAdd, REAL fMul, LONG lAdd );
static	int		iShft1 = 0, iShft2 = 0;
static	int		ifunc  = 0, imode  = 0, ibits = 0;

OBJ		ODST;			// Resulting image object
HWND	hWndDST=NULL;	// Resulting image HWND

BOOL	bStopALU = TRUE;

/****************************************************************************/
static	void icLog( PREAL dst, PREAL src1, PREAL src2, int cnt )
{
	int	i;
	for (i=0;i<cnt;i++,src1++,dst++)
	{
		if (*src1>1.) *dst=(REAL)log10(*src1);
		else          *dst=0;
	}
}

/****************************************************************************/
static	void icInv( PREAL dst, PREAL src1, PREAL src2, int cnt )
{
	int	i;
	for (i=0;i<cnt;i++,src1++)
	{
		*src1 = fMaximum - *src1;
	}
}

/****************************************************************************/
static HWND FindArg( int n )
{
	OBJ*	pr;

	pr = OBJ::GetOBJ( MDIhWnd );
	if( NULL == pr )
	{
		return NULL;
	}
	// modified to remove MAXCHI by Peter on 06 July 2006
	if( (n >= 0) && (n < pr->m_vChildren.size()) )
	{
		return pr->m_vChildren[n];
	}
//	int		i, j;
//	for(i = 0, j = 0; i < MAXCHI; i++)
//	{
//		if( pr->Chi[i] )
//		{
//			if (j==n) return pr->Chi[i];
//			j++;
//		}
//	}
	// end of modification by Peter on 06 July 2006
	return NULL;
}
/****************************************************************************/
static BOOL PrepareFinal( HWND hDlg )
{
	double	dlt = fMax - fMin;
	BOOL	bClip = ComboBox_GetCurSel(GetDlgItem(hDlg,IDC_IC_MODE));
	// Modified by Peter on 28 April 2008
	PIXEL_FORMAT nPix = GetDestType(hDlg);
	// End of modification on 28 April 2008
	static int imageserial = 1;

	post = DummyPost;

	s_lAdd = 0;
	fAdd = 0.;
	fMul = 1.;
	if( bClip )											// Clip
	{
		// Modified by Peter on 28 April 2008
		switch( nPix )
		{
		case PIX_BYTE:   { post = icPost8c; break; }
		case PIX_WORD:   { post = icPost16c; break; }
		case PIX_DWORD:  { post = icPost32c; break; }
		case PIX_CHAR:   { post = icPost8sc; break; }
		case PIX_SHORT:  { post = icPost16sc; break; }
		case PIX_LONG:   { post = icPost32sc; break; }
		case PIX_FLOAT:  { post = icPostFc; break; }
		case PIX_DOUBLE: { post = icPostDc; break; }
		}
		// End of modification on 28 April 2008
	}
	else
	{													// Fit
		fAdd = -fMin;
		if( dlt > 1e-5 )
		{
			// Modified by Peter on 28 April 2008
			switch( nPix )
			{
			case PIX_BYTE:   { fMul = ((REAL)(BYTE)0xFF)/dlt; break; }
			case PIX_CHAR:   { s_lAdd = 0x00000080; fMul = ((REAL)(BYTE)0xFF)/dlt; break; }
			case PIX_WORD:   { fMul = ((REAL)(WORD)0xFFFF)/dlt; break; }
			case PIX_SHORT:  { s_lAdd = 0x00008000; fMul = ((REAL)(WORD)0xFFFF)/dlt; break; }
			case PIX_DWORD:  { fMul = ((REAL)(DWORD)0xFFFFFFFF)/dlt; break; }
			case PIX_LONG:   { s_lAdd = 0x7FFFFFFF; fMul = ((REAL)(DWORD)0xFFFFFFFF)/dlt; break; }
			case PIX_FLOAT:  { fMul = 1.0; break; }
			case PIX_DOUBLE: { fMul = 1.0; break; }
			}
			// End of modification on 28 April 2008
		}
		// Modified by Peter on 28 April 2008
		switch( nPix )
		{
		case PIX_BYTE:   { post = icPost8c; break; }
		case PIX_WORD:   { post = icPost16c; break; }
		case PIX_DWORD:  { post = icPost32c; break; }
		case PIX_CHAR:   { post = icPost8sc; break; }
		case PIX_SHORT:  { post = icPost16sc; break; }
		case PIX_LONG:   { post = icPost32sc; break; }
		case PIX_FLOAT:  { post = icPostFc; break; }
		case PIX_DOUBLE: { post = icPostDc; break; }
		}
		// End of modification on 28 April 2008
	}
	// Modified by Peter on 28 April 2008
	switch( nPix )
	{
	case PIX_BYTE:
	case PIX_CHAR:   { wAddd = R4(wCX); break; }
	case PIX_WORD:
	case PIX_SHORT:  { wAddd = R4(wCX*2); break; }
// 	case PIX_SHORT:  { wAddd = R4(wCX)*2; break; }
	case PIX_DWORD:
// 	case PIX_LONG:   { wAddd = R4(wCX)*4; break; }
	case PIX_LONG:   { wAddd = R4(wCX*4); break; }
// 	case PIX_FLOAT:  { wAddd = R4(wCX)*4; break; }
	case PIX_FLOAT:  { wAddd = R4(wCX*4); break; }
// 	case PIX_DOUBLE: { wAddd = R4(wCX)*8; break; }
	case PIX_DOUBLE: { wAddd = R4(wCX*8); break; }
	}
	// End of modification on 28 April 2008
	if( hWndDST )
	{
		DestroyWindow(hWndDST);
		hWndDST = NULL;
	}

	// Fill in the header
	ODST.ID = OT_IMG;
	ODST.x = wCX;
	ODST.y = wCY;
	ODST.xas = wCX;
	ODST.yas = wCY;
	ODST.npix = nPix;
#ifdef USE_XB_LENGTH
	ODST.xb = R4(wCX);
	ODST.dp = (LPBYTE)calloc(1, ODST.xb*ODST.y*IMAGE::PixFmt2Bpp(ODST.npix));
#else
	ODST.stride = R4(ODST.x*IMAGE::PixFmt2Bpp(ODST.npix));
	ODST.dp = (LPBYTE)calloc(1, ODST.stride*ODST.y);
#endif
	if( NULL == ODST.dp )
	{
		return FALSE;
	}
	ODST.scu = 1;
	ODST.scd = 1;
	ODST.maxcontr = TRUE;
	ODST.bright = 128;
	ODST.contrast = 128;
	ODST.palno = -1;

	sprintf( ODST.title, "Calculated image #%d", imageserial );
	imageserial++;
	strcpy( ODST.fname, "" );
	hWndDST = objCreateObject( IDM_F_OPEN, &ODST );
//	ODST.flags |= OF_someglags;
//	UpdateWindow(back_hwnd);
	if( NULL == hWndDST )
		return FALSE;
	ODST = *(OBJ::GetOBJ(hWndDST));
	return TRUE;
}
/****************************************************************************/
static void doCleanup( void )
{
	if (lps1)  { free(lps1);  lps1  = NULL; }
	if (lps2)  { free(lps2);  lps2  = NULL; }
	if (lpdst) { free(lpdst); lpdst = NULL; }
}
/****************************************************************************/
static BOOL doPrepare( OBJ* s1, OBJ* s2 )
{
	doCleanup();
	pres1 = DummyPre;
	pres2 = DummyPre;

	if( hWndDST )
	{
		DestroyWindow(hWndDST);
		hWndDST = NULL;
	}
	wCX = s1->x;
	wCY = s1->y;
	switch(s1->npix)
	{
	case PIX_BYTE:
		wAdd1 = R4(s1->x);
		if (iShft1>=0) pres1 = im8PreUp;
		else pres1 = im8PreDown;
		break;
	case PIX_CHAR:
		wAdd1 = R4(s1->x);
		if (iShft1>=0) pres1 = im8sPreUp;
		else pres1 = im8sPreDown;
		break;
	case PIX_WORD:
// 		wAdd1 = R4(s1->x)*2;
		wAdd1 = R4(s1->x*2);
		if (iShft1>=0) pres1 = im16PreUp;
		else pres1 = im16PreDown;
		break;
	case PIX_SHORT:
// 		wAdd1 = R4(s1->x)*2;
		wAdd1 = R4(s1->x*2);
		if (iShft1>=0) pres1 = im16sPreUp;
		else pres1 = im16sPreDown;
		break;
	case PIX_DWORD:
// 		wAdd1 = R4(s1->x)*4;
		wAdd1 = R4(s1->x*4);
		if (iShft1>=0) pres1 = im32PreUp;
		else pres1 = im32PreDown;
		break;
	case PIX_LONG:
//		wAdd1 = R4(s1->x)*4;
		wAdd1 = R4(s1->x*4);
		if (iShft1>=0) pres1 = im32sPreUp;
		else pres1 = im32sPreDown;
		break;
	case PIX_FLOAT:
// 		wAdd1 = R4(s1->x)*4;
		wAdd1 = R4(s1->x*4);
		if (iShft1>=0) pres1 = imFPreUp;
		else pres1 = imFPreDown;
	    break;
	case PIX_DOUBLE:
// 		wAdd1 = R4(s1->x)*8;
		wAdd1 = R4(s1->x*8);
		if (iShft1>=0) pres1 = imDPreUp;
		else pres1 = imDPreDown;
		break;
	}
	wShft1 = abs(iShft1);
	lps1 = (PREAL)malloc(s1->x*sizeof(REAL) );
	if (lps1==NULL) return FALSE;

	if( s2 )
	{
		wCX = __min(wCX,s2->x);
		wCY = __min(wCY,s2->y);
		switch(s2->npix)
		{
		case PIX_BYTE:
			wAdd2 = R4(s2->x);
			pres2 = (iShft2 >= 0) ? im8PreUp : im8PreDown;
			break;
		case PIX_CHAR:
			wAdd2 = R4(s2->x);
			if (iShft2>=0) pres2 = im8sPreUp;
			else pres2 = im8sPreDown;
			break;
		case PIX_WORD:
// 			wAdd2 = R4(s2->x)*2;
			wAdd2 = R4(s2->x*2);
			if (iShft2>=0) pres2 = im16PreUp;
			else pres2 = im16PreDown;
			break;
		case PIX_SHORT:
// 			wAdd2 = R4(s2->x)*2;
			wAdd2 = R4(s2->x*2);
			if (iShft2>=0) pres2 = im16sPreUp;
			else pres2 = im16sPreDown;
			break;
		case PIX_DWORD:
// 			wAdd2 = R4(s2->x)*4;
			wAdd2 = R4(s2->x*4);
			if (iShft2>=0) pres2 = im32PreUp;
			else pres2 = im32PreDown;
			break;
		case PIX_LONG:
// 			wAdd2 = R4(s2->x)*4;
			wAdd2 = R4(s2->x*4);
			if (iShft2>=0) pres2 = im32sPreUp;
			else pres2 = im32sPreDown;
			break;
		case PIX_FLOAT:
// 			wAdd2 = R4(s2->x)*4;
			wAdd2 = R4(s2->x*4);
			if (iShft2>=0) pres2 = imFPreUp;
			else pres2 = imFPreDown;
			break;
		case PIX_DOUBLE:
// 			wAdd2 = R4(s2->x)*8;
			wAdd2 = R4(s2->x*8);
			if (iShft2>=0) pres2 = imDPreUp;
			else pres2 = imDPreDown;
			break;
		}
		wShft2 = abs(iShft2);
		lps2 = (PREAL)malloc(wCX*sizeof(REAL) );
		if (lps2==NULL) goto fex;

		lpdst = (PREAL)malloc(wCX*sizeof(REAL) );
		if (lpdst==NULL) goto fex;
	}

	return TRUE;
fex:
	doCleanup();
	return FALSE;
}
/****************************************************************************/
static void	doIt( HWND hDlg, BOOL tst )
{
	HWND	s1, s2;
	OBJ*	o1;
	OBJ*	o2;
	DWORD	ofs1, ofs2, ofsd;
	int		i;
	HCURSOR	hoc;
	LPBYTE	ddp;
	int		fn = ComboBox_GetCurSel(GetDlgItem( hDlg, IDC_IC_FUNC ));
	BOOL	b2 = func[fn].b2;

	if (bStopALU) return;

	ariph = func[fn].func;

	s1 = FindArg(ComboBox_GetCurSel(GetDlgItem( hDlg, IDC_IC_SRC1 )));
	if( NULL == s1 ) return;
	o1 = OBJ::GetOBJ(s1);
	// maximum possible value
	fMaximum = 255;
	switch( o1->npix )
	{
	case PIX_BYTE:
		fMaximum = 0xFF;
		break;
	case PIX_CHAR:
		fMaximum = 0x7F;
		break;
	case PIX_WORD:
		fMaximum = 0xFFFF;
		break;
	case PIX_SHORT:
		fMaximum = 0x7FFF;
		break;
	case PIX_DWORD:
		fMaximum = (double)(DWORD)0xFFFFFFFF;
		break;
	case PIX_LONG:
		fMaximum = (double)(DWORD)0x7FFFFFFF;
		break;
		// TODO: check this
	case PIX_FLOAT:
		fMaximum = (double)(DWORD)0xFFFFFFFF;
		break;
	case PIX_DOUBLE:
		fMaximum = (double)(DWORD)0xFFFFFFFF;
		break;
	}
// 	fMaximum = o1->npix == 2 ? 65535 : 255;	// maximum possible value

	s2 = NULL;
	o2 = NULL;
	if( b2 )
	{
		s2 = FindArg(ComboBox_GetCurSel(GetDlgItem( hDlg, IDC_IC_SRC2 )));
		if (s2==NULL) return;
		o2 = OBJ::GetOBJ(s2);
	}

	if( FALSE == doPrepare(o1, o2) )
		return;

	hoc = SetCursor( hcWait );
	if( tst )
	{
		fMax = -(fMin = std::numeric_limits<double>::max());
		if( b2 )
		{
			for(i=0,ofs1=0,ofs2=0; i<wCY; ofs1+=wAdd1,ofs2+=wAdd2,i++)
			{
				pres1(lps1, o1->dp, ofs1, wCX, wShft1, lSbtr1 );
				pres2(lps2, o2->dp, ofs2, wCX, wShft2, lSbtr2 );
				ariph(lpdst,lps1,lps2,wCX);
				if (i==0) { fMin = *lpdst; fMax = *lpdst; }
				GetFloatMinMax_t<double>( lpdst, wCX, fMin, fMax );
			}
		}
		else
		{
			for(i=0,ofs1=0; i<wCY; i++,ofs1+=wAdd1)
			{
				pres1(lps1, o1->dp, ofs1, wCX, wShft1, lSbtr1 );
				ariph(lps1,lps1,NULL,wCX);
				if (i==0) { fMin = *lps1; fMax = *lps1; }
				GetFloatMinMax_t<double>( lps1, wCX, fMin, fMax );
			}
		}
	}
	else
	{
		if (!PrepareFinal(hDlg)) goto fex;
		ddp = ODST.dp;
		if (b2)
		{
			for(i=0,ofs1=0,ofs2=0,ofsd=0; i<wCY; ofs1+=wAdd1,ofs2+=wAdd2,ofsd+=wAddd,i++)
			{
				pres1(lps1, o1->dp, ofs1, wCX, wShft1, lSbtr1 );
				pres2(lps2, o2->dp, ofs2, wCX, wShft2, lSbtr2 );
				ariph(lpdst,lps1,lps2,wCX);
				post (ddp, ofsd, lpdst, wCX, fAdd, fMul, s_lAdd );
			}
		}
		else
		{
			for(i=0,ofs1=0,ofsd=0; i<wCY; ofs1+=wAdd1,ofsd+=wAddd,i++)
			{
				pres1(lps1, o1->dp, ofs1, wCX, wShft1, lSbtr1 );
				ariph(lps1,lps1,NULL,wCX);
				post(ddp, ofsd, lps1, wCX, fAdd, fMul, s_lAdd );
			}
		}
		imgCalcMinMax( OBJ::GetOBJ(hWndDST) );
		ActivateObj( hWndDST );
	}
fex:
	doCleanup();
	if (hWndDST && o1) OBJ::GetOBJ(hWndDST)->NI = o1->NI;
	if (hoc) SetCursor(hoc);
}
/****************************************************************************/
static void FillICsrcs( HWND hDlg )
{
	HWND	hs1, hs2;
	int		hp1, hp2;//, i;
	OBJ*	pObj;
	OBJ*	pPar;
	HwndVecIt it;

	hs1 = GetDlgItem( hDlg, IDC_IC_SRC1 );
	hs2 = GetDlgItem( hDlg, IDC_IC_SRC2 );
	hp1 = ComboBox_GetCurSel( hs1 );
	hp2 = ComboBox_GetCurSel( hs2 );
	ComboBox_ResetContent( hs1 );
	ComboBox_ResetContent( hs2 );

	pPar = OBJ::GetOBJ( MDIhWnd );
	if( NULL != pPar )
	{
		// modified to remove MAXCHI by Peter on 06 July 2006
		for(it = pPar->m_vChildren.begin(); it != pPar->m_vChildren.end(); ++it)
		{
			if( NULL != *it )
			{
				pObj = OBJ::GetOBJ( *it );
				sprintf(TMP, "%s - %dx%dx%d",
					pObj->title,
					pObj->x,
					pObj->y,
					8*IMAGE::PixFmt2Bpp(pObj->npix));
				ComboBox_InsertString( hs1, -1, TMP );
				ComboBox_InsertString( hs2, -1, TMP );
			}
		}
//		for(i=0;i<MAXCHI;i++)
//		{
//			if (pr->Chi[i])
//			{
//				O = OBJ::GetOBJ( pr->Chi[i] );
//				sprintf(TMP, "%s - %dx%dx%d", O->title,O->x,O->y, 8*O->pix);
//				ComboBox_InsertString( hs1, -1, TMP );
//				ComboBox_InsertString( hs2, -1, TMP );
//			}
//		}
	}
	if (hp1==LB_ERR) hp1=0;
	if (hp2==LB_ERR) hp2=0;
	ComboBox_SetCurSel( hs1, hp1 );
	ComboBox_SetCurSel( hs2, hp2 );
}

/****************************************************************************/

template <typename _Tx>
void UpdateRanges_t( HWND hDlg, _Tx unused )
{
	BOOL bClip = ComboBox_GetCurSel(GetDlgItem(hDlg, IDC_IC_MODE));
	_Tx _min_, _max_;
	if( true == numeric_traits<_Tx>::is_integral )
	{
		// for integral types
		_min_ = std::numeric_limits<_Tx>::min();
		_max_ = std::numeric_limits<_Tx>::max();
		// integer types will be scaled to the whole range in case of FIT
		if( TRUE == bClip )
		{
			_min_ = (_Tx)((fMin <  _min_) ? _min_ : fMin);
			_max_ = (_Tx)((fMax >= _max_) ? _max_ : fMax);
		}
		if( true == numeric_traits<_Tx>::is_signed )
		{
			sprintf(TMP, "%d", _min_);
			SetDlgItemText( hDlg, IDC_IC_RESULTRANGEL, TMP );
			sprintf(TMP, "%d", _max_);
			SetDlgItemText( hDlg, IDC_IC_RESULTRANGEH, TMP );
		}
		else
		{
			sprintf(TMP, "%u", _min_);
			SetDlgItemText( hDlg, IDC_IC_RESULTRANGEL, TMP );
			sprintf(TMP, "%u", _max_);
			SetDlgItemText( hDlg, IDC_IC_RESULTRANGEH, TMP );
		}
	}
	else
	{
		// for non-integral types (float, double)
		_min_ = -(_max_ = std::numeric_limits<_Tx>::max());
		sprintf(TMP, "%g", (_Tx)((fMin < _min_) ? _min_ : fMin));
		SetDlgItemText( hDlg, IDC_IC_RESULTRANGEL, TMP );
		sprintf(TMP, "%g", (_Tx)((fMax >= _max_) ? _max_ : fMax));
		SetDlgItemText( hDlg, IDC_IC_RESULTRANGEH, TMP );
	}
}

static void UpdateRanges( HWND hDlg )
{
	PIXEL_FORMAT nPix = GetDestType(hDlg);
	if( fMin > fMax + 1e-5 )
		fMax = fMin + 1e-5;
	switch( nPix )
	{
	case PIX_BYTE:   UpdateRanges_t<BYTE>(hDlg, (BYTE)0); break;
	case PIX_CHAR:   UpdateRanges_t<CHAR>(hDlg, (CHAR)0); break;
	case PIX_WORD:   UpdateRanges_t<WORD>(hDlg, (WORD)0); break;
	case PIX_SHORT:  UpdateRanges_t<SHORT>(hDlg, (SHORT)0); break;
	case PIX_DWORD:  UpdateRanges_t<DWORD>(hDlg, (DWORD)0); break;
	case PIX_LONG:   UpdateRanges_t<LONG>(hDlg, (LONG)0); break;
	case PIX_FLOAT:  UpdateRanges_t<float>(hDlg, (float)0); break;
	case PIX_DOUBLE: UpdateRanges_t<double>(hDlg, (double)0); break;
	}
}
/****************************************************************************/
static void ChangeFunc( HWND hDlg )
{
	int	i = SendDlgItemMessage( hDlg, IDC_IC_FUNC, CB_GETCURSEL, 0, 0);

	if (i==CB_ERR) return;
	EnableWindow( GetDlgItem(hDlg,IDC_IC_SRC2), func[i].b2 );
	doIt( hDlg, TRUE );
	UpdateRanges( hDlg );
}
/****************************************************************************/
static void ChangeSrc( HWND hDlg, WORD ida, WORD idb, BOOL bActivate )
{
	int		n = SendDlgItemMessage( hDlg, ida, CB_GETCURSEL, 0, 0);
	OBJ*	pObj;
	HWND	hChild;
	HCURSOR	hoc;

	if( n == CB_ERR)
		return;

	hoc = SetCursor( hcWait );
	hChild = FindArg( n );
	if( NULL != hChild )
	{
		pObj = OBJ::GetOBJ(hChild);
		if( NULL != pObj )
		{
			imgCalcMinMax( pObj );
			// Modified by Peter on 10 Feb 2007
			TMP[0] = 0;
			switch(pObj->npix)
			{
			case PIX_BYTE:
			case PIX_WORD:
			case PIX_DWORD:
				sprintf( TMP, "%u..%u", (DWORD)pObj->imina, (DWORD)pObj->imaxa);
				break;
			case PIX_CHAR:
			case PIX_SHORT:
			case PIX_LONG:
				sprintf( TMP, "%d..%d", (LONG)pObj->imina, (LONG)pObj->imaxa);
				break;
			case PIX_FLOAT:
			case PIX_DOUBLE:
				sprintf( TMP, "%g..%g", (REAL)pObj->imina, (REAL)pObj->imaxa);
				break;
			}
			// End of Peters modification on 10 Feb 2007
			SetDlgItemText(hDlg, idb, TMP);
			if( bActivate )
				ActivateObj(hChild);
		}
	}
	ChangeFunc( hDlg );
	if( hoc )
		SetCursor(hoc);
}
/****************************************************************************/
static void InitICdlg( HWND hDlg )
{
	int	i;

	SetDlgItemInt( hDlg, IDC_IC_SUBTRACT1, lSbtr1, TRUE );
	SetDlgItemInt( hDlg, IDC_IC_SUBTRACT2, lSbtr2, TRUE );
	FillICsrcs( hDlg );

	i = SendDlgItemMessage( hDlg, IDC_IC_FUNC, CB_GETCURSEL, 0, 0);	if (i != CB_ERR ) ifunc = i;
	SendDlgItemMessage( hDlg, IDC_IC_FUNC, CB_RESETCONTENT, 0, 0);
	for(i=0; func[i].name!=NULL; i++)
		SendDlgItemMessage( hDlg, IDC_IC_FUNC, CB_INSERTSTRING, -1, (LPARAM)func[i].name);
	SendDlgItemMessage( hDlg, IDC_IC_FUNC, CB_SETCURSEL, ifunc, 0);

	i = SendDlgItemMessage( hDlg, IDC_IC_MODE, CB_GETCURSEL, 0, 0);	if (i != CB_ERR ) imode = i;
	SendDlgItemMessage( hDlg, IDC_IC_MODE, CB_RESETCONTENT, 0, 0);
	for(i=0; mode[i].name!=NULL; i++)
		SendDlgItemMessage( hDlg, IDC_IC_MODE, CB_INSERTSTRING, -1, (LPARAM)mode[i].name);
	SendDlgItemMessage( hDlg, IDC_IC_MODE, CB_SETCURSEL, imode, 0);

	i = SendDlgItemMessage( hDlg, IDC_IC_816, CB_GETCURSEL, 0, 0);	if (i != CB_ERR ) ibits = i;
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_RESETCONTENT, 0, 0);
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"8bit");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"16bit");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"32bit");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"8bit signed");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"16bit signed");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"32bit signed");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"float");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_INSERTSTRING, -1, (LPARAM)"double");
	SendDlgItemMessage( hDlg, IDC_IC_816, CB_SETCURSEL, ibits, 0);

	bStopALU = TRUE;
	ChangeSrc( hDlg, IDC_IC_SRC1, IDC_IC_SRC1INFO, FALSE );
	ChangeSrc( hDlg, IDC_IC_SRC2, IDC_IC_SRC2INFO, FALSE );
	bStopALU = FALSE;
	ChangeFunc( hDlg );
	SetDlgItemInt( hDlg, IDC_IC_SHFT1, iShft1, TRUE);
	SetDlgItemInt( hDlg, IDC_IC_SHFT2, iShft2, TRUE);
}

/****************************************************************************/
BOOL CALLBACK ICalcDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL	f;

	switch (message)
	{
		case WM_INITDIALOG:
			InitICdlg( hDlg );
			regReadWindowPos( hDlg, "ICalculator");
			return FALSE;

		case WM_NCDESTROY:
			bStopALU = TRUE;
			regWriteWindowPos( hDlg, "ICalculator", FALSE);
			break;

		case WM_ACTIVATE:
			if (LOWORD(wParam)==WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
				InitICdlg( hDlg );
			break;

		case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDC_IC_SHFT1:
				switch(HIWORD(wParam))
				{
				case SE_MOVE:
					iShft1 += (LOWORD(lParam)==JC_UP) ? 1 : -1;
					SetDlgItemInt( hDlg, IDC_IC_SHFT1, iShft1, TRUE );
					goto cnt;
				case EN_CHANGE:
					iShft1 = GetDlgItemInt( hDlg, IDC_IC_SHFT1, &f, TRUE );
					goto cnt;
				}
				break;

			case IDC_IC_SHFT2:
				switch(HIWORD(wParam))
				{
				case SE_MOVE:
					iShft2 += (LOWORD(lParam)==JC_UP) ? 1 : -1;
					SetDlgItemInt( hDlg, IDC_IC_SHFT2, iShft2, TRUE );
					goto cnt;
				case EN_CHANGE:
					iShft2 = GetDlgItemInt( hDlg, IDC_IC_SHFT2, &f, TRUE );
					goto cnt;
				}
				break;
cnt:			ChangeFunc( hDlg );
				break;

			case IDC_IC_FUNC:
				if (HIWORD(wParam)==CBN_SELCHANGE)
					ChangeFunc( hDlg );
				break;
			case IDC_IC_SRC1:
				if (HIWORD(wParam)==CBN_SELCHANGE)
					ChangeSrc( hDlg, LOWORD(wParam), IDC_IC_SRC1INFO, TRUE );
				break;
			case IDC_IC_SRC2:
				if (HIWORD(wParam)==CBN_SELCHANGE)
					ChangeSrc( hDlg, LOWORD(wParam), IDC_IC_SRC2INFO, TRUE );
				break;

			case IDC_IC_SUBTRACT1:
				if (HIWORD(wParam)==EN_KILLFOCUS)
				{
					int f;
					lSbtr1 = GetDlgItemInt(hDlg, LOWORD(wParam), &f, TRUE);
				}
				break;
			case IDC_IC_SUBTRACT2:
				if (HIWORD(wParam)==EN_KILLFOCUS)
				{
					int f;
					lSbtr2 = GetDlgItemInt(hDlg, LOWORD(wParam), &f, TRUE);
				}
				break;

			case IDC_IC_RESULTRANGEL:
				if (HIWORD(wParam)==EN_KILLFOCUS)
				{
					double a=fMin;
					GetDlgItemDouble(hDlg,LOWORD(wParam), &a, 0);
					fMin = a;
					UpdateRanges(hDlg);
				}
				break;

			case IDC_IC_RESULTRANGEH:
				if (HIWORD(wParam)==EN_KILLFOCUS)
				{
					double a=fMax;
					GetDlgItemDouble(hDlg,LOWORD(wParam), &a, 0);
					fMax = a;
					UpdateRanges(hDlg);
				}
				break;

			case IDC_IC_DOIT:
				if (hWndDST==0)
				{
					ReplyMessage( FALSE );
					doIt( hDlg, FALSE );
				}
				if (hWndDST) InvalidateRect( hWndDST, NULL, FALSE );
				hWndDST = NULL;
				InitICdlg( hDlg );
				break;

			case IDCANCEL:
				DestroyWindow(hDlg);
				hICalc = NULL;
				return TRUE;
		}
	}
	return FALSE;
}

