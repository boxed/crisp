/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.Information panel * by ML *****************************************/
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

#include "ft32util.h"

#include "scaling.h"

#include "HelperFn.h"
#include "eld.h"
/****************************************************************************/
/* Information panel ********************************************************/
/****************************************************************************/

static	NEWINFO	i_NI;						// Copies for Cancel

extern NEWINFO	NIDefault;					// From CRISP.CPP
extern double	fDigit, fAspectXY;			// Exported from 1D tools OneD.CPP
extern double	eldXcal, eldYcal;			// Exported from Calibr.CPP

/****************************************************************************/
// u in volts
double	CRISP_WaveLength(double u)
{
	double	h	=	6.626176e-34;
	double	me	=	9.109534e-31;
	double	c	=	2.99792458e8;
	double	e	=	1.6021892e-19;
	double	E, m, l;

	E = e*u/2.;
	m = me + E/c/c;
	l = h/sqrt(m*E)/2.;

	return l;	// return in m
}
/****************************************************************************/
static	VOID CorrectImageAspect(HWND hWnd)
{
	double	dx, dy, asp;
	OBJ*	O = GetBaseObject(OBJ::GetOBJ(hWnd));
	LPBYTE	lpD=NULL, lpS=NULL;
	LPBYTE	dst, lpSrc, lpDst;
	int		x0, y0, x1, y1, i, srcw, dstw, xbw;
	int		sy=0;
	// Added by Peter on 08 Apr 2003
	int ps = IMAGE::PixFmt2Bpp(O->npix);

	dx = 1./O->x;
	dy = 1./O->y;
	asp = O->NI.AspectXY;

	if( 1 == ps )
	{
		if (fabs(asp-1.)>__min(dx,dy) && asp>=.5 && asp<=2.)
		{
			x0=O->x;
			y0=O->y;
			if (asp>=1.) { x1 = x0; y1 = round(y0*asp); }
			else         { x1 = round(x0/asp); y1 = y0; }

			x0 *= ps;		// Added by Peter on 08 Apr 2003
			x1 *= ps;		// Added by Peter on 08 Apr 2003

			srcw = R4(x0);
			dstw = R4(x1);

			sc1Init( x1, y1, x0, y0 );

			xbw = R4(__max(x1,x0));
			lpS = (LPBYTE)malloc( xbw+1 );	// Source for scaler
			lpD = (LPBYTE)malloc( xbw*2 );	// Dest for scaler
			dst = (LPBYTE)malloc( R4(x1)*y1);

			lpDst = dst; lpSrc = O->dp;

			if (lpS==NULL || lpD==NULL || dst==NULL) goto ex;

			memcpy(lpS, lpSrc, x0);
			lpSrc+=srcw;

			for(i=0; i<y1; i++ ) {
				while( !sc1Line (lpD, lpS)) {
					if (sy<y0-1){
						memcpy(lpS, lpSrc, x0);
						lpSrc+=srcw;
						sy++;
					}
				}
				memcpy( lpDst, lpD, x1 );
				lpDst += dstw;
			}
			free( O->dp );
			O->dp = dst;
			O->x = x1/ps;
			O->y = y1;	// Added by Peter on 08 Apr 2003
			//		O->x = x1; O->y = y1;
			O->xas = O->x;	O->yas = O->y;
			O->NI.AspectXY = 1.;
			wndSetZoom( hWnd, O->scu, O->scd );
ex:
			sc1Free();
			if (lpD) free( lpD );
			if (lpS) free( lpS );
			objInformChildren( hWnd, FM_Update, 0, 0);
		}
		else
		{
			MessageBeep(-1);
		}
	}
	// added by Peter on 1 July 2003
	else if( 2 == ps )
	{
		int j, xi, yi;
		LPWORD pDst = NULL, ptr, pSrc, src;
		double x, y, u, v;
		int a00, a01, a10, a11;
		if( (fabs(asp-1.)>__min(dx,dy)) && (asp>=.5) && (asp<=2.) )
		{
			x0 = O->x;
			y0 = O->y;
			// zooming up
			if (asp>=1.) { x1 = x0; y1 = round(y0*asp); }
			else         { x1 = round(x0/asp); y1 = y0; }

//			x0 *= ps;
//			x1 *= ps;

			srcw = R4(x0);	// source width in bytes
			dstw = R4(x1);	// destination width in bytes
			pDst = (LPWORD)malloc(2*dstw*y1);
			pSrc = (LPWORD)O->dp;

			dx = double(srcw) / double(dstw);
			dy = double(y0) / double(y1);
			y = 0.0;
			ptr = pDst;
			for(i = 0; i < y1; i++, y += dy )
			{
				yi = (int)y;
				if( yi >= y0-1 )
					break;
				v = y - yi;
				src = (LPWORD)(((LPBYTE)pSrc) + 2*srcw*yi);
				x = 0.0;
				for(j = 0; j < x1; j++, x += dx )
				{
					xi = (int)x;
					u = x - xi;
					a00 = src[xi];
					a01 = src[xi+1];
					a10 = src[xi+srcw];
					a11 = src[xi+1+srcw];
					ptr[j] = a00*(1.0-u)*(1.0-v)+a01*u*(1.0-v)+
						a10*(1.0-u)*v+a11*u*v;
				}
				ptr = (LPWORD)(((LPBYTE)ptr)+2*dstw);
			}

			free( O->dp );
			O->dp = (LPBYTE)pDst;
			O->x = x1;
			O->y = y1;	// Added by Peter on 08 Apr 2003
			//		O->x = x1; O->y = y1;
			O->xas = O->x;
			O->yas = O->y;
			O->NI.AspectXY = 1.;
			wndSetZoom( hWnd, O->scu, O->scd );

			objInformChildren( hWnd, FM_Update, 0, 0);
		}
		else
		{
			MessageBeep(-1);
		}
	}
}
/****************************************************************************/
// Added by Peter on 03 Feb 2003
void CRISP_ReadNIDefault()
{
	FILE * in;
	char	fname[MAX_PATH];
	LPSTR	s, ptr;
	float 	Vacc=0.0, Cs=0.0, Cc=0.0, Sch=1.0, Limit=1.0, def;

	GetModuleFileName( hInst, fname, sizeof(fname) );
	strlwr(fname);
	strcpy(strrchr(fname,'\\'), "\\elmicro.tbl");

	if( NULL == (in = fopen(fname,"rt")) )
		return;	// dlgError( IDS_ERRELMICRONOTFOUND, hDlg);
	while( !feof(in) )
	{
		fgets(TMP, 80, in);
		if( ';' == TMP[0] )
			continue;

		if( NULL == (ptr = strstr(TMP, "ParentWnd=")) )
			continue;
		// succeeded here so - read only first line - it is default
		s = ptr + strlen("ParentWnd=") + 1;
		sscanf(s, "%f%f%f%f%f", &Vacc, &Cs, &Cc, &Sch, &Limit);

		def = round(-sqrt(4./3. * Cs*1e-3 * CRISP_WaveLength(Vacc*1e3))*1e10);

		NIDefault.Vacc = Vacc*1e3;
		NIDefault.Cs = Cs*1e-3;
		NIDefault.Defocus0 = def*1e-10;
		NIDefault.Defocus1 = def*1e-10;

		break;
	}
	fclose(in);
}
/****************************************************************************/
// Added by Peter on 03 Feb 2003
void CRISP_SaveNIDefault(HWND hDlg)
{
	FILE   *in, *out;
	char	fname[MAX_PATH], foname[MAX_PATH];
	char	buf[1024];
	LPSTR	ptr, s;
	float 	Vacc=0.0, Cs=0.0, Cc=0.0, Sch=1.0, Limit=1.0, def;
	bool bDefaultSaved = false;

	GetModuleFileName( hInst, fname, sizeof(fname) );
	strlwr(fname);
	strcpy(foname, fname);
	strcpy(strrchr(fname,'\\'), "\\elmicro.tbl");
	strcpy(strrchr(foname,'\\'), "\\elmicro_tmp.tbl");

	int i = (int)SendDlgItemMessage( hDlg, IDC_INF_EMLIST, CB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hDlg, IDC_INF_EMLIST, CB_GETLBTEXT, i, (LPARAM)(LPSTR)buf);
	if( NULL == (ptr = strstr(buf, "ParentWnd=")) )
		return;
	// update dafault values
	s = ptr + strlen("ParentWnd=") + 1;
	sscanf(s, "%f%f%f%f%f", &Vacc, &Cs, &Cc, &Sch, &Limit);

	def = round(-sqrt(4./3. * Cs*1e-3 * CRISP_WaveLength(Vacc*1e3))*1e10);

	NIDefault.Vacc = Vacc*1e3;
	NIDefault.Cs = Cs*1e-3;
	NIDefault.Defocus0 = def*1e-10;
	NIDefault.Defocus1 = def*1e-10;

	if( NULL == (in = fopen(fname,"rt")) )
		return;
	if( NULL == (out = fopen(foname,"wt")) )
		return;
	while( !feof(in) )
	{
		fgets(TMP, 80, in);
		if( ';' == TMP[0] )
		{
			fprintf(out, TMP);
			continue;
		}
		if( false == bDefaultSaved )
		{
			// succeeded here so - read only first line - it is default
			buf[27] = 0;
			fprintf(out, "%27sPar=%6d   %4.2f %4.2f  %4.2f     %4.2f\n", buf, int(NIDefault.Vacc*1e-3),
				NIDefault.Cs*1e3, Cc, Sch, Limit);
			bDefaultSaved = true;
		}
		else
		{
			fprintf(out, TMP);
		}
	}
	fclose(in);
	fclose(out);

	remove(fname);
	rename(foname, fname);
}
/****************************************************************************/
static BOOL FillEMList( HWND hDlg )
{
	FILE * in;
	char	fname[MAX_PATH];

	GetModuleFileName( hInst, fname, sizeof(fname));
	strlwr(fname);
	strcpy(strrchr(fname,'\\'), "\\elmicro.tbl");

	if( NULL == (in = fopen(fname,"r")) )
		return FALSE;	// dlgError( IDS_ERRELMICRONOTFOUND, hDlg);
	while( !feof(in) )
	{
		fgets(TMP, 80, in);
		if (TMP[0] == ';') continue;
		SendDlgItemMessage( hDlg, IDC_INF_EMLIST, CB_INSERTSTRING, -1, (LPARAM)(LPSTR)TMP);
	}
	fclose(in);
	return TRUE;
}
/****************************************************************************/
static	void	UpdateInfo( HWND hDlg, OBJ* O)
{
	if (O->NI.Digit == NIDefault.Digit && fDigit>0. && fAspectXY>0.)
	{
		O->NI.Digit = fDigit;
		O->NI.AspectXY = fAspectXY;
	}
	SetDlgItemDouble( hDlg, IDC_INF_VACC,	  O->NI.Vacc			/1e3,  0);	// V -> kV
	SetDlgItemDouble( hDlg, IDC_INF_CS,   	  O->NI.Cs				*1e3,  0);  // m -> mm
	SetDlgItemDouble( hDlg, IDC_INF_DEFOCUS0, O->NI.Defocus0		*1e10, 0); // m -> Ang
	SetDlgItemDouble( hDlg, IDC_INF_DEFOCUS1, O->NI.Defocus1		*1e10, 0); // m -> Ang
	SetDlgItemDouble( hDlg, IDC_INF_AZIMUTH,  R2D(O->NI.Azimuth),          0);
	SetDlgItemDouble( hDlg, IDC_INF_DEFSPREAD,O->NI.Def_Spread		*1e10, 0); // m -> Ang
	SetDlgItemDouble( hDlg, IDC_INF_CONVER,	  R2D(O->NI.Conver),           0);

	SetDlgItemDouble( hDlg, IDC_INF_SCALE,	  O->NI.Scale			*1e10, 0); // m -> Ang
	SetDlgItemDouble( hDlg, IDC_INF_BEAMTILT, R2D(O->NI.Beam_Tilt),        0);
	SetDlgItemDouble( hDlg, IDC_INF_SPECTILT, R2D(O->NI.Spec_Tilt),        0);
	SetDlgItemDouble( hDlg, IDC_INF_THICKNESS,O->NI.Thickness		*1e10, 0); // m -> Ang

	CheckDlgButton( hDlg, IDC_INF_ELD, O->NI.Flags & NI_DIFFRACTION_PATTERN );
	SetDlgItemDouble( hDlg, IDC_INF_DIG,      O->NI.Digit*1e-3,            0);
	SetDlgItemDouble( hDlg, IDC_INF_ASPECT,   O->NI.AspectXY,              0);
	if (O->NI.Flags & NI_DIFFRACTION_PATTERN) {
		SetDlgItemDouble( hDlg, IDC_INF_MAG, O->NI.CamLength*1e3,          0);	// m->mm
	} else {
		SetDlgItemDouble( hDlg, IDC_INF_MAG, O->NI.Magnif,                 0);	// x
	}
}
/****************************************************************************/
static	void	ChangeEM( HWND hDlg, OBJ* O )
{
	int i = (int)SendDlgItemMessage( hDlg, IDC_INF_EMLIST, CB_GETCURSEL, 0, 0);
	float 	Vacc=0., Cs=0., Cc=0., Sch=1., Limit=1., def;
	LPSTR	s;

	SendDlgItemMessage(hDlg, IDC_INF_EMLIST, CB_GETLBTEXT, i, (LPARAM)(LPSTR)TMP);

	s = strstr(TMP, "Par=") + strlen("Par=") + 1;
	sscanf(s, "%f%f%f%f%f", &Vacc, &Cs, &Cc, &Sch, &Limit);
	def = round(-sqrt(4./3. * Cs*1e-3 * CRISP_WaveLength(Vacc*1e3))*1e10);
	SetDlgItemDouble( hDlg, IDC_INF_VACC,	 Vacc, 0);
	SetDlgItemDouble( hDlg, IDC_INF_CS,   	 Cs,   0);
	SetDlgItemDouble( hDlg, IDC_INF_DEFOCUS0, def, 0);
	SetDlgItemDouble( hDlg, IDC_INF_DEFOCUS1, def, 0);
	SendDlgItemMessage( hDlg, IDC_INF_VACC,     EM_SETMODIFY, 1, 0);
	SendDlgItemMessage( hDlg, IDC_INF_CS,       EM_SETMODIFY, 1, 0);
	SendDlgItemMessage( hDlg, IDC_INF_DEFOCUS0, EM_SETMODIFY, 1, 0);
	SendDlgItemMessage( hDlg, IDC_INF_DEFOCUS1, EM_SETMODIFY, 1, 0);

	SendDlgItemMessage( hDlg, IDC_INF_SCALE,    EM_SETMODIFY, 1, 0);	// to update values
}
/****************************************************************************/
static void UpdateFromScale( HWND hDlg, OBJ* O)
{
	if (IsDlgButtonChecked(hDlg, IDC_INF_DIGFIX))
	{
		if (O->NI.Flags & NI_DIFFRACTION_PATTERN) { // Change CamLength
			O->NI.CamLength = O->NI.Scale/(CRISP_WaveLength(O->NI.Vacc)*O->NI.Digit);
			SetDlgItemDouble( hDlg, IDC_INF_MAG, O->NI.CamLength*1e3, 0);
		} else {					// Change Magnif
			O->NI.Magnif = 1./(O->NI.Scale*O->NI.Digit);
			SetDlgItemDouble( hDlg, IDC_INF_MAG, O->NI.Magnif, 0);
		}
	}
	else
	{
		if (O->NI.Flags & NI_DIFFRACTION_PATTERN) { // Change Digit ED
			O->NI.Digit = O->NI.Scale/(CRISP_WaveLength(O->NI.Vacc)*O->NI.CamLength);
			SetDlgItemDouble( hDlg, IDC_INF_DIG, O->NI.Digit*1e-3, 0);
		} else {					// Change Digit IMG
			O->NI.Digit = 1./(O->NI.Scale*O->NI.Magnif);
			SetDlgItemDouble( hDlg, IDC_INF_DIG, O->NI.Digit*1e-3, 0);
		}
	}
}
/****************************************************************************/
static void UpdateFromDigit( HWND hDlg, OBJ* O)
{
	if (IsDlgButtonChecked(hDlg, IDC_INF_SCALEFIX))
	{
		if (O->NI.Flags & NI_DIFFRACTION_PATTERN) { // Change CamLength
			O->NI.CamLength = O->NI.Scale/(CRISP_WaveLength(O->NI.Vacc)*O->NI.Digit);
			SetDlgItemDouble( hDlg, IDC_INF_MAG, O->NI.CamLength*1e3, 0);
		} else {					// Change Magnif
			O->NI.Magnif = 1./(O->NI.Scale*O->NI.Digit);
			SetDlgItemDouble( hDlg, IDC_INF_MAG, O->NI.Magnif, 0);
		}
	}
	else
	{
		if (O->NI.Flags & NI_DIFFRACTION_PATTERN) { // Change Digit ED
			O->NI.Scale = O->NI.Digit*CRISP_WaveLength(O->NI.Vacc)*O->NI.CamLength;
			SetDlgItemDouble( hDlg, IDC_INF_SCALE, O->NI.Scale*1e10, 0);
		} else {					// Change Digit IMG
			O->NI.Scale = 1./(O->NI.Digit*O->NI.Magnif);
			SetDlgItemDouble( hDlg, IDC_INF_SCALE, O->NI.Scale*1e10, 0);
		}
	}
}
/****************************************************************************/
static void UpdateFromMagnif( HWND hDlg, OBJ* O)
{
	if (IsDlgButtonChecked(hDlg, IDC_INF_SCALEFIX))
	{
		O->NI.Digit = 1./(O->NI.Scale*O->NI.Magnif);
		SetDlgItemDouble( hDlg, IDC_INF_DIG, O->NI.Digit*1e-3, 0);
	}
	else
	{
		O->NI.Scale = 1./(O->NI.Digit*O->NI.Magnif);
		SetDlgItemDouble( hDlg, IDC_INF_SCALE, O->NI.Scale*1e10, 0);
	}
}
/****************************************************************************/
static void UpdateFromCamLength( HWND hDlg, OBJ* O)
{
	if (IsDlgButtonChecked(hDlg, IDC_INF_SCALEFIX))
	{
		O->NI.Digit = O->NI.Scale/(CRISP_WaveLength(O->NI.Vacc)*O->NI.CamLength);
		SetDlgItemDouble( hDlg, IDC_INF_DIG, O->NI.Digit*1e-3, 0);
	}
	else
	{
		O->NI.Scale = O->NI.Digit*CRISP_WaveLength(O->NI.Vacc)*O->NI.CamLength;
		SetDlgItemDouble( hDlg, IDC_INF_SCALE, O->NI.Scale*1e10, 0);
	}
}
/****************************************************************************/
static BOOL mGetDlgItemFloat( HWND hDlg, WORD ID, LPDOUBLE dd )
{
	return GetDlgItemDouble( hDlg, ID, dd, 0 );
}
/****************************************************************************/
static	void	ScanInfo( HWND hDlg, OBJ* O)
{
double	dd;

	if (mGetDlgItemFloat( hDlg, IDC_INF_VACC,     &dd)) {
		if (dd<20 )  { dd=20;    SetDlgItemDouble(hDlg,IDC_INF_VACC,dd, 0); }
		if (dd>1500) { dd=1500;  SetDlgItemDouble(hDlg,IDC_INF_VACC,dd, 0); }
		O->NI.Vacc = dd*1e3;
	}
	if (mGetDlgItemFloat( hDlg, IDC_INF_CS,   	 &dd)) {
		if (dd  <0.4) { dd  =0.4;  SetDlgItemDouble(hDlg,IDC_INF_CS,  dd, 0); }
		if (dd  >9.0) { dd  =9.0;  SetDlgItemDouble(hDlg,IDC_INF_CS,  dd, 0); }
		O->NI.Cs = dd*1e-3;
	}
	if (mGetDlgItemFloat( hDlg, IDC_INF_DEFOCUS0, &dd)) {
		if (dd <-50000.) { dd=-50000.; SetDlgItemDouble(hDlg,IDC_INF_DEFOCUS0,dd, 0); }
		if (dd > 50000.) { dd= 50000.; SetDlgItemDouble(hDlg,IDC_INF_DEFOCUS0,dd, 0); }
		O->NI.Defocus0	= dd*1e-10;
	}
	if (mGetDlgItemFloat( hDlg, IDC_INF_DEFOCUS1, &dd)) {
		if (dd <-50000.) { dd=-50000.; SetDlgItemDouble(hDlg,IDC_INF_DEFOCUS1,dd, 0); }
		if (dd > 50000.) { dd= 50000.; SetDlgItemDouble(hDlg,IDC_INF_DEFOCUS1,dd, 0); }
		O->NI.Defocus1	= dd*1e-10;
	}

	if (mGetDlgItemFloat( hDlg, IDC_INF_AZIMUTH,	 &dd)) O->NI.Azimuth	= D2R(dd);	// deg  -> rad
	if (mGetDlgItemFloat( hDlg, IDC_INF_DEFSPREAD,&dd))    O->NI.Def_Spread	= dd*1e-10;	// A  -> m
	if (mGetDlgItemFloat( hDlg, IDC_INF_CONVER,	 &dd))     O->NI.Conver		= D2R(dd);	// deg  -> rad

	if (mGetDlgItemFloat( hDlg, IDC_INF_SCALE,	 &dd)) {
		if (dd <0.05) { dd=0.05; SetDlgItemDouble(hDlg,IDC_INF_SCALE,dd, 0); }
		if (dd >2000) { dd=2000; SetDlgItemDouble(hDlg,IDC_INF_SCALE,dd, 0); }
		O->NI.Scale		= dd*1e-10; // A  -> m
		UpdateFromScale( hDlg, O );
	}
	if (mGetDlgItemFloat( hDlg, IDC_INF_DIG,	 &dd)) {
		if (dd <0.05) { dd=0.05; SetDlgItemDouble(hDlg,IDC_INF_DIG,dd, 0); }
		if (dd >2000.) { dd=2000; SetDlgItemDouble(hDlg,IDC_INF_DIG,dd, 0); }
		dd = dd*1e3;
		O->NI.Digit		= dd; //
		UpdateFromDigit( hDlg, O );
	}
	if (mGetDlgItemFloat( hDlg, IDC_INF_MAG,	 &dd)) {
		if (O->NI.Flags & NI_DIFFRACTION_PATTERN) {
			if (dd <1.)   { dd=1.; SetDlgItemDouble(hDlg,IDC_INF_MAG,dd, 0); }
			if (dd >1e6)  { dd=1e6; SetDlgItemDouble(hDlg,IDC_INF_MAG,dd, 0); }
			O->NI.CamLength	= dd*1e-3; //
			UpdateFromCamLength( hDlg, O );
		} else {
			if (dd <1.)   { dd=1.; SetDlgItemDouble(hDlg,IDC_INF_MAG,dd, 0); }
			if (dd >1e10)  { dd=1e10; SetDlgItemDouble(hDlg,IDC_INF_MAG,dd, 0); }
			O->NI.Magnif	= dd; //
			UpdateFromMagnif( hDlg, O );
		}
	}
	if (mGetDlgItemFloat( hDlg, IDC_INF_ASPECT,	 &dd)) {
		if (dd <0.1) { dd=0.1; SetDlgItemDouble(hDlg,IDC_INF_ASPECT,dd, 0); }
		if (dd >10.) { dd=10.; SetDlgItemDouble(hDlg,IDC_INF_ASPECT,dd, 0); }
		O->NI.AspectXY	= dd; //
	}

	if (mGetDlgItemFloat( hDlg, IDC_INF_BEAMTILT, &dd)) O->NI.Beam_Tilt	= D2R(dd);// °  -> rad
	if (mGetDlgItemFloat( hDlg, IDC_INF_SPECTILT, &dd)) O->NI.Spec_Tilt	= D2R(dd);// °  -> rad
	if (mGetDlgItemFloat( hDlg, IDC_INF_THICKNESS,&dd)) O->NI.Thickness	= dd/1e10; // Angstrom  -> m
	O->NI.EM_ID = (int)SendDlgItemMessage( hDlg, IDC_INF_EMLIST, CB_GETCURSEL, 0, 0);

}
/****************************************************************************/
static void ChangeImageType( HWND hDlg, OBJ* O, BOOL bChange )
{
int res;

	if (O->NI.Flags & NI_DIFFRACTION_PATTERN)
	{
		if (bChange && (eldXcal>0.) && (eldYcal>0.))
		{ // Calibration exist
			if ((fabs(O->NI.Scale-1e-10)>1e-12))
			{
				res = MessageBox( hDlg, "This diffraction pattern already has been calibrated.\n\
					Would you like overwrite it?", "Calibration", MB_ICONQUESTION | MB_YESNO);
				if (res==IDYES)
				{
					if (O->NI.Digit == NIDefault.Digit && fDigit>0. && fAspectXY>0.)
					{
						O->NI.Digit = fDigit;
					}
					O->NI.Scale = eldXcal *1e-10;
					O->NI.AspectXY = eldXcal/eldYcal;
				}
			}
			else
			{
				if (O->NI.Digit == NIDefault.Digit && fDigit>0. && fAspectXY>0.)
				{
					O->NI.Digit = fDigit;
				}
				O->NI.Scale = eldXcal *1e-10;
				O->NI.AspectXY = eldXcal/eldYcal;
			}
		}
		SetDlgItemText( hDlg, IDC_INF_SCALEBOX, "ED constant (d*R)" );
		TextToDlgItemW( hDlg, IDC_INF_SCALEUNITS, "(%c*pix)", ANGSTR_CHAR );
//		SetDlgItemText( hDlg, IDC_INF_SCALEUNITS, "(\302*pixel)" );
		SetDlgItemText( hDlg, IDC_INF_MAGBOX, "Camera length" );
		SetDlgItemText( hDlg, IDC_INF_MAGUNITS, "mm" );
	}
	else
	{
		SetDlgItemText( hDlg, IDC_INF_SCALEBOX, "Scale" );
//		SetDlgItemText( hDlg, IDC_INF_SCALEUNITS, "(\302/pixel)" );
		TextToDlgItemW( hDlg, IDC_INF_SCALEUNITS, "(%c/pix)", ANGSTR_CHAR );
		SetDlgItemText( hDlg, IDC_INF_MAGBOX, "Image magnification" );
		SetDlgItemText( hDlg, IDC_INF_MAGUNITS, "x" );
	}
//	SetDlgItemText( hDlg, IDC_DEFOCUS_U, "(\302)" );
//	SetDlgItemText( hDlg, IDC_DEFOCUS_V, "(\302)" );
//	SetDlgItemText( hDlg, IDC_DEF_SPREAD, "(\302)" );
//	SetDlgItemText( hDlg, IDC_THICKNESS, "(\302)" );
	TextToDlgItemW( hDlg, IDC_DEFOCUS_U,  "(%c)", ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_DEFOCUS_V,  "(%c)", ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_DEF_SPREAD, "(%c)", ANGSTR_CHAR );
	TextToDlgItemW( hDlg, IDC_THICKNESS,  "(%c)", ANGSTR_CHAR );

	SendDlgItemMessage( hDlg, IDC_INF_SCALE,    EM_SETMODIFY, 1, 0);	// to update values
	UpdateInfo( hDlg, O );
	SetDlgItemText(hDlg, IDC_INF_MAGFIX, "");
	SetDlgItemText(hDlg, IDC_INF_DIGFIX, "");
	SetDlgItemText(hDlg, IDC_INF_SCALEFIX, "");
}
/****************************************************************************/
static void ChangeFixedItem( HWND hDlg, WORD ID )
{
	static	WORD oldID;

	switch (oldID)
	{
		case IDC_INF_MAGFIX:
			EnableWindow(GetDlgItem(hDlg, IDC_INF_MAG), TRUE);
			break;
		case IDC_INF_DIGFIX:
			EnableWindow(GetDlgItem(hDlg, IDC_INF_DIG), TRUE);
			break;
		case IDC_INF_SCALEFIX:
			EnableWindow(GetDlgItem(hDlg, IDC_INF_SCALE), TRUE);
			break;
	}
	switch (ID)
	{
		case IDC_INF_MAGFIX:
			EnableWindow(GetDlgItem(hDlg, IDC_INF_MAG), FALSE);
			break;
		case IDC_INF_DIGFIX:
			EnableWindow(GetDlgItem(hDlg, IDC_INF_DIG), FALSE);
			break;
		case IDC_INF_SCALEFIX:
			EnableWindow(GetDlgItem(hDlg, IDC_INF_SCALE), FALSE);
			break;
		default: return;
	}
	oldID = ID;
}
/****************************************************************************/
BOOL CALLBACK InfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND parentWnd = (HWND)GetWindowLong( hDlg, DWL_USER );
	OBJ*	O = NULL;

	if (parentWnd) O = GetBaseObject(OBJ::GetOBJ(parentWnd));

	switch (message)
	{
		case WM_INITINFO:
		case WM_INITDIALOG:
			if (hInfo==NULL)
			{
				FillEMList( hDlg );
				regReadWindowPos( hDlg, "Information");
			}
			if (lParam != 0)
			{
				O = OBJ::GetOBJ((HWND)lParam);
			}
			O = GetBaseObject(O);

			if (O !=NULL && O->ID == OT_IMG)
			{
				SetWindowLong( hDlg, DWL_USER, lParam);
				sprintf(TMP,"Information on <%s>",O->title);
				SetWindowText(hDlg,TMP);
				i_NI = O->NI;	// Copy for Cancel
				SendDlgItemMessage( hDlg, IDC_INF_EMLIST, CB_SETCURSEL, O->NI.EM_ID, 0);
				UpdateInfo( hDlg, O );
				ChangeImageType( hDlg, O, FALSE );
				CheckDlgButton( hDlg, IDC_INF_MAGFIX, TRUE);
				ChangeFixedItem( hDlg, IDC_INF_MAGFIX );
				ShowWindow( hDlg, SW_SHOWNOACTIVATE );
			}
			else
			{
				SetWindowLong( hDlg, DWL_USER, 0);
				SetWindowText(hDlg,"Information");
				ShowWindow( hDlg, SW_HIDE );
			}
			SetWindowLong( hDlg, DWL_USER, lParam);

			{
#ifdef USE_FONT_MANAGER
				HFONT hFont = FontManager::GetFont(FM_NORMAL_FONT);
#else
				HFONT hFont = hfNormal;
#endif
				SendDlgItemMessage(hDlg, IDC_DEFOCUS_U, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_DEFOCUS_V, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_DEF_SPREAD, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_THICKNESS, WM_SETFONT, (WPARAM)hFont, 0);
				SendDlgItemMessage(hDlg, IDC_INF_SCALEUNITS, WM_SETFONT, (WPARAM)hFont, 0);
			}

			return TRUE;

		case WM_NCDESTROY:
			regWriteWindowPos( hDlg, "Information", FALSE);
			break;

		case WM_COMMAND:
			if (LOWORD(wParam)==IDCANCEL || LOWORD(wParam)==IDOK)
			{
				DestroyWindow(hDlg);
				hInfo = NULL;
				break;
			}
			if (O == NULL || O->hWnd == NULL) break;
			switch(LOWORD(wParam))
			{
//				case IDC_INF_HELP:
//					WinHelp(hDlg, "crispd.hlp", HELP_KEY, (DWORD)(LPSTR)"Information");
//					break;
				case IDC_INF_AZIMUTH:
				case IDC_INF_CS:
				case IDC_INF_DEFSPREAD:
				case IDC_INF_DEFOCUS0:
				case IDC_INF_CONVER:
				case IDC_INF_DEFOCUS1:
				case IDC_INF_BEAMTILT:
				case IDC_INF_VACC:
				case IDC_INF_SPECTILT:
				case IDC_INF_THICKNESS:
				case IDC_INF_SCALE:
				case IDC_INF_DIG:
				case IDC_INF_MAG:

					if (HIWORD(wParam) == EN_KILLFOCUS) ScanInfo( hDlg, O );
					break;

				case IDC_INF_CORRECT:
					ScanInfo( hDlg, O );
					CorrectImageAspect( O->hWnd );
					UpdateInfo( hDlg, O );
					break;

				case IDC_INF_EMLIST:
					switch (HIWORD(wParam))
					{
						case CBN_SELCHANGE:
							ChangeEM( hDlg, O );
							ScanInfo( hDlg, O );
							break;
					}
					break;

				case IDC_INF_ELD:
					if (IsDlgButtonChecked(hDlg, IDC_INF_ELD))
						O->NI.Flags |=  NI_DIFFRACTION_PATTERN;
					else
						O->NI.Flags &= ~NI_DIFFRACTION_PATTERN;
					ChangeImageType( hDlg, O, TRUE );
					break;

				case IDC_INF_MAGFIX:
				case IDC_INF_DIGFIX:
				case IDC_INF_SCALEFIX:
					ChangeFixedItem( hDlg, wParam );
					break;

				case IDC_INF_UPDATE:
					ScanInfo( hDlg, O );
					objInformChildren( O->hWnd, FM_UpdateNI, 0, 0);
					break;

					// added by Peter 03 Feb 2003
				case IDC_INF_SETASDEFAULT:
					::CRISP_SaveNIDefault( hDlg );
					break;
			}
			break;
	}
	return FALSE;
}
/****************************************************************************/

