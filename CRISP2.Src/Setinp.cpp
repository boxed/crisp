#include "StdAfx.h"

#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "rgconfig.h"

#include "..\shark49x\pcidev.h"
#include "..\shark49x\shark4.h"

typedef struct {
	BOOL	exist;			// Input exist
	UINT	ccd;			// CCD type
	UINT	input;			// Physical input number
	char	name[32];		// Input name
	BOOL	tbc;			// Time base correction
	BOOL	ctrap;			// Color trap
	BOOL	gpo1;			// GPO1 state
	BOOL	gpo2;			// GPO2 state
	UINT	exposure;		// Exposure time
	UINT	x,y;			// Total resolution
	UINT	x0,y0,x1,y1;	// Clipping
	UINT	amode;			// Amode: 0-Normal, 1-Averaging, 2-Integration
	UINT	adepth;			// Adepth: 1,2,4,8,16,32,64,128
	BOOL	b16;			// 16 bit mode
	UINT	gain;			// Gain value
	UINT	offs;			// offset value
	UINT	brig;			// Brigtness
	UINT	cont;			// Contrast
} INPUT_t, * LPINPUT_t;
							//  pos def  min   max step jump
static	SCROLLBAR	sbGain = {   0,  -1, -1,   255, 1, 16, IDC_I_GAINT};	//
static	SCROLLBAR	sbOffs = {   0,  64,  0,   255, 1, 16, IDC_I_OFFST};	//
static	SCROLLBAR	sbBrig = {   0, 128,  0,   255, 1, 16, IDC_I_BRIGT};	//
static	SCROLLBAR	sbCont = {   0,  64,  0,   255, 1, 16, IDC_I_CONTT};	//

#define	NINPUTS		16
static INPUT_t	inputs[NINPUTS];	// Input structures

static INPUT_t	defTV  = {FALSE,0,0,"TV monochrome",FALSE,FALSE,FALSE,FALSE,0, 768, 576,0,0, 768, 576,0,1,FALSE,-1,64,128,64};
static INPUT_t	defDIG = {FALSE,4,4,"Digital"      ,FALSE,FALSE,FALSE,FALSE,0,1024,1024,0,0,1024,1024,0,1,FALSE,-1,64,128,64};

#define	CCDs	6
static LPSTR cCCDs[CCDs] = {"Mono", "CVBS", "S-Video", "RGB", "Digital", "LVDS_link"};

#define INPUTs	6
static LPSTR cInputs[INPUTs] = {"CINCH1", "CINCH2", "SVHS1", "SVHS2", "RGB", "Adapter"};

#define AMODEs	3
static LPSTR cAModes[AMODEs] = {"Normal", "Averaging", "Integration"};

#define ADEPTHs	8
static UINT uADepths[ADEPTHs] = {1,2,4,8,16,32,64,128};

UINT	uiInput=0;

static char	szExist[] = "Exist";
static char	szCCD[] = "CCD";
static char	szInput[] = "Input";
static char	szName[] = "Name";
static char	szTBC[] = "TBC";
static char	szCTrap[] = "CTrap";
static char	szExposure[] = "Exposure";
static char	szResolution[] = "Resolution";
static char	szClipping[] = "Clipping";
static char	szGPO1[] = "GPO1";
static char	szGPO2[] = "GPO2";
static char	szFmt2d[] = "%d,%d";
static char	szFmt4d[] = "%d,%d,%d,%d";
static char	szGlobal[] = "Global";
static char	szActive[] = "Active";
static char	szTotal[] = "Total";
static char	szAMode[] = "AMode";
static char	szADepth[] = "ADepth";
static char	sz16bit[] = "16bit";

#pragma optimize ("",off)

/*******************************************************************************/
static void UpdateInputInfo( HWND hDlg )
{
int		i, iind;
BOOL	bTV=FALSE, bRGB=FALSE, bDig=FALSE;
LPINPUT_t inp = &inputs[uiInput];

	SendDlgItemMessage( hDlg,IDC_I_INPUT,CB_RESETCONTENT,0,0);

	switch(inp->ccd) {
	case 0: case 1:	// Mono, CVBS
		for(i=0;i<4;i++) SendDlgItemMessage(hDlg,IDC_I_INPUT,CB_INSERTSTRING,-1,(LPARAM)cInputs[i]);
		if (inp->input>4) inp->input=0;
		iind = inp->input;
		bTV=TRUE;
		break;

	case 2: // S-Video
		for(i=2;i<4;i++) SendDlgItemMessage(hDlg,IDC_I_INPUT,CB_INSERTSTRING,-1,(LPARAM)cInputs[i]);
		if (inp->input>4 || inp->input<2) inp->input=2;
		iind = inp->input-2;
		bTV=TRUE;
		break;
	
	case 3: // RGB
		SendDlgItemMessage(hDlg,IDC_I_INPUT,CB_INSERTSTRING,-1,(LPARAM)cInputs[4]);
		if (inp->input!=4) inp->input=4;
		iind = 0;
		bRGB=TRUE;
		break;
	
	case 4: // Digital
		SendDlgItemMessage(hDlg,IDC_I_INPUT,CB_INSERTSTRING,-1,(LPARAM)cInputs[5]);
		if (inp->input!=5) inp->input=5;
		iind = 0;
		bDig=TRUE;
		break;

	case 5: // LVDS link
		SendDlgItemMessage(hDlg,IDC_I_INPUT,CB_INSERTSTRING,-1,(LPARAM)cInputs[5]);
		if (inp->input!=8) inp->input=8;
		iind = 0;
		bDig=TRUE;
		break;
	}

	SendDlgItemMessage(hDlg,IDC_I_INPUT,   CB_SETCURSEL, iind, 0);
	SendDlgItemMessage(hDlg,IDC_I_CCDTYPE, CB_SETCURSEL, inp->ccd, 0);
	EnableWindow( GetDlgItem(hDlg,IDC_I_INPUT), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_XR), bDig );
	EnableWindow( GetDlgItem(hDlg,IDC_I_YR), bDig );
	EnableWindow( GetDlgItem(hDlg,IDC_I_GPO1), bDig );
	EnableWindow( GetDlgItem(hDlg,IDC_I_GPO2), bDig );
	EnableWindow( GetDlgItem(hDlg,IDC_I_CX0), TRUE );
	EnableWindow( GetDlgItem(hDlg,IDC_I_CX1), TRUE );
	EnableWindow( GetDlgItem(hDlg,IDC_I_CY0), TRUE );
	EnableWindow( GetDlgItem(hDlg,IDC_I_CY1), TRUE );
	EnableWindow( GetDlgItem(hDlg,IDC_I_EXPOSURETIME), bDig );
	EnableWindow( GetDlgItem(hDlg,IDC_I_TBC), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_CTRAP), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_GAIN), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_OFFS), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_BRIG), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_CONT), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_FULLG), bTV );
	EnableWindow( GetDlgItem(hDlg,IDC_I_DEFAULT), bTV );
	wndChangeDlgSize( hDlg, hDlg, 0, bTV ? 0:IDC_I_GAIN, "SETINPUT", hInst);

	SetDlgItemText( hDlg, IDC_I_NAME,  inp->name );
	CheckDlgButton( hDlg, IDC_I_TBC,   inp->tbc );
	CheckDlgButton( hDlg, IDC_I_CTRAP, inp->ctrap );
	CheckDlgButton( hDlg, IDC_I_GPO1,  inp->gpo1 );
	CheckDlgButton( hDlg, IDC_I_GPO2,  inp->gpo2 );
	SetDlgItemInt ( hDlg, IDC_I_EXPOSURETIME, inp->exposure, FALSE );
	SetDlgItemInt ( hDlg, IDC_I_XR,  inp->x, FALSE );
	SetDlgItemInt ( hDlg, IDC_I_YR,  inp->y, FALSE );
	SetDlgItemInt ( hDlg, IDC_I_CX0, inp->x0, FALSE );
	SetDlgItemInt ( hDlg, IDC_I_CY0, inp->y0, FALSE );
	SetDlgItemInt ( hDlg, IDC_I_CX1, inp->x1, FALSE );
	SetDlgItemInt ( hDlg, IDC_I_CY1, inp->y1, FALSE );

	SendDlgItemMessage(hDlg,IDC_I_AMODE, CB_SETCURSEL, inp->amode, 0);
	SendDlgItemMessage(hDlg,IDC_I_ADEPTH, CB_SETCURSEL, inp->adepth, 0);
	CheckDlgButton( hDlg, IDC_I_16BIT,  inp->b16 );

	InitSliderCtl( hDlg, IDC_I_GAIN, &sbGain);
	InitSliderCtl( hDlg, IDC_I_OFFS, &sbOffs);
	InitSliderCtl( hDlg, IDC_I_BRIG, &sbBrig);
	InitSliderCtl( hDlg, IDC_I_CONT, &sbCont);
}

/*******************************************************************************/
static void ScanInput( HWND hDlg )
{
int		i;
BOOL	bTV=FALSE, bRGB=FALSE, bDig=FALSE;
LPINPUT_t inp = &inputs[uiInput];

	inp->tbc = FALSE;
	inp->ctrap = FALSE;
	inp->exposure = 0;
	inp->gpo1 = FALSE;
	inp->gpo2 = FALSE;
	inp->b16 = FALSE;
	switch(inp->ccd) {
	case 0: case 1:	// Mono, CVBS
		inp->input = SendDlgItemMessage(hDlg,IDC_I_INPUT,CB_GETCURSEL,0,0);
		bTV=TRUE;
		break;

	case 2: // S-Video
		inp->input = SendDlgItemMessage(hDlg,IDC_I_INPUT,CB_GETCURSEL,0,0)+2;
		bTV=TRUE;
		break;
	
	case 3: // RGB
		inp->input=4;
		bRGB=TRUE;
		break;
	
	case 4: // Digital
		inp->input=5;
		bDig=TRUE;
		break;
	case 5: // LVDS_link
		inp->input=8;
		bDig=TRUE;
		break;
	}
	if (bTV) {
		inp->tbc = IsDlgButtonChecked( hDlg, IDC_I_TBC );
		inp->ctrap = IsDlgButtonChecked( hDlg, IDC_I_CTRAP );
	}
	if (bDig) {
		inp->exposure = GetDlgItemInt( hDlg, IDC_I_EXPOSURETIME, &i, FALSE );
		inp->x = GetDlgItemInt( hDlg, IDC_I_XR, &i, FALSE );
		inp->y = GetDlgItemInt( hDlg, IDC_I_YR, &i, FALSE );
		inp->gpo1 = IsDlgButtonChecked( hDlg, IDC_I_GPO1 );
		inp->gpo2 = IsDlgButtonChecked( hDlg, IDC_I_GPO2 );
	}
	inp->x0 = GetDlgItemInt( hDlg, IDC_I_CX0, &i, FALSE );
	inp->y0 = GetDlgItemInt( hDlg, IDC_I_CY0, &i, FALSE );
	inp->x1 = GetDlgItemInt( hDlg, IDC_I_CX1, &i, FALSE );
	inp->y1 = GetDlgItemInt( hDlg, IDC_I_CY1, &i, FALSE );

	inp->b16 = IsDlgButtonChecked( hDlg, IDC_I_16BIT );
	inp->amode = SendDlgItemMessage(hDlg,IDC_I_AMODE,CB_GETCURSEL,0,0);
	inp->adepth = SendDlgItemMessage(hDlg,IDC_I_ADEPTH,CB_GETCURSEL,0,0);

	GetDlgItemText( hDlg, IDC_I_NAME, inp->name, sizeof(inp->name) );
}

/*******************************************************************************/
static void FillInputList( HWND hDlg )
{
int		i;

	SendDlgItemMessage( hDlg,IDC_I_LIST,LB_RESETCONTENT,0,0);
	for(i=0;i<NINPUTS;i++) {
		if (!inputs[i].exist) break;
		SendDlgItemMessage( hDlg,IDC_I_LIST,LB_INSERTSTRING,-1,(LPARAM)inputs[i].name);
	}
	SendDlgItemMessage( hDlg,IDC_I_LIST,LB_SETCURSEL,uiInput,0);
}

/*******************************************************************************/
void siInitInputs( void )
{
int		i, j;
char	iname[20], fname[_MAX_PATH], tmp[128];

	sbGain.def =  -1; sbGain.p = g_pfnS4GETPARAMETER( SH4_P_GAIN );
	sbOffs.def =  64; sbOffs.p = g_pfnS4GETPARAMETER( SH4_P_OFFS );
	sbBrig.def = 128; sbBrig.p = g_pfnS4GETPARAMETER( SH4_P_BRIG );
	sbCont.def =  64; sbCont.p = g_pfnS4GETPARAMETER( SH4_P_CONT );

	GetModuleFileName( hInst, fname, sizeof(fname));
	strlwr(fname);
	strcpy(strrchr(fname,'\\'), "\\inputs.ini");

	uiInput = GetPrivateProfileInt   ( szGlobal, szActive, uiInput, fname);
	j       = GetPrivateProfileInt   ( szGlobal, szTotal,  0, fname);

	for(i=0;i<NINPUTS;i++) {
		sprintf(iname,"Input%d",i);
		inputs[i]=defTV;
		if (GetPrivateProfileInt(iname,szExist,FALSE,fname) == FALSE) continue;
		if (i>=j) continue;
		inputs[i].exist   = TRUE;
		inputs[i].ccd     = GetPrivateProfileInt(iname,szCCD,  inputs[i].ccd,  fname);
		inputs[i].input   = GetPrivateProfileInt(iname,szInput,inputs[i].input,fname);
		inputs[i].tbc     = GetPrivateProfileInt(iname,szTBC,  inputs[i].tbc,  fname);
		inputs[i].ctrap   = GetPrivateProfileInt(iname,szCTrap,inputs[i].ctrap,fname);
		inputs[i].exposure= GetPrivateProfileInt(iname,szExposure,inputs[i].exposure,fname);
		inputs[i].gpo1    = GetPrivateProfileInt(iname,szGPO1, inputs[i].gpo1,fname);
		inputs[i].gpo2    = GetPrivateProfileInt(iname,szGPO2, inputs[i].gpo2,fname);
		inputs[i].b16     = GetPrivateProfileInt(iname,sz16bit,inputs[i].b16,fname);
		inputs[i].amode   = GetPrivateProfileInt(iname,szAMode,inputs[i].amode,fname);
		inputs[i].adepth  = GetPrivateProfileInt(iname,szADepth,inputs[i].adepth,fname);
		GetPrivateProfileString( iname, szResolution, "768,576", tmp, sizeof(tmp), fname);
		sscanf(tmp, szFmt2d, &inputs[i].x, &inputs[i].y);
		GetPrivateProfileString( iname, szClipping, "0,0,768,576", tmp, sizeof(tmp), fname);
		sscanf(tmp, szFmt4d, &inputs[i].x0, &inputs[i].y0, &inputs[i].x1, &inputs[i].y1);
		GetPrivateProfileString( iname, szName, iname, inputs[i].name, sizeof(inputs[i].name), fname);
	}
}

/*******************************************************************************/
void siSaveInputs( void )
{
int		i;
char	iname[20], fname[_MAX_PATH], tmp[128];
#define WritePrivateProfileInt(a,b,c,d) sprintf(tmp,"%d",c),WritePrivateProfileString(a,b,tmp,d)
	
	GetModuleFileName( hInst, fname, sizeof(fname));
	strlwr(fname);
	strcpy(strrchr(fname,'\\'), "\\inputs.ini");

	WritePrivateProfileInt   ( szGlobal, szActive, uiInput, fname);

	for(i=0;i<NINPUTS;i++) {
		if (inputs[i].exist==FALSE) break;
		sprintf(iname,"Input%d",i);
		WritePrivateProfileString( iname, szExist, "1",  fname);
		WritePrivateProfileInt   ( iname, szCCD,  inputs[i].ccd,  fname);
		WritePrivateProfileInt   ( iname, szInput,inputs[i].input,fname);
		WritePrivateProfileInt   ( iname, szTBC,  inputs[i].tbc,  fname);
		WritePrivateProfileInt   ( iname, szCTrap,inputs[i].ctrap,fname);
		WritePrivateProfileInt   ( iname, szExposure,inputs[i].exposure,fname);
		WritePrivateProfileInt   ( iname, szGPO1, inputs[i].gpo1,fname);
		WritePrivateProfileInt   ( iname, szGPO2, inputs[i].gpo2,fname);
		WritePrivateProfileInt   ( iname, sz16bit,  inputs[i].b16,fname);
		WritePrivateProfileInt   ( iname, szAMode,  inputs[i].amode,fname);
		WritePrivateProfileInt   ( iname, szADepth, inputs[i].adepth,fname);

		sprintf(tmp,szFmt2d,inputs[i].x, inputs[i].y);
		WritePrivateProfileString( iname, szResolution, tmp, fname);
		sprintf(tmp,szFmt4d,inputs[i].x0, inputs[i].y0, inputs[i].x1, inputs[i].y1);
		WritePrivateProfileString( iname, szClipping, tmp, fname);
		WritePrivateProfileString( iname, szName, inputs[i].name, fname);
	}
	WritePrivateProfileInt   ( szGlobal, szTotal,  i, fname);
}

/*******************************************************************************/
UINT siGetBitRes( void )
{
	if (inputs[uiInput].b16) return 16;
	else                     return 8;
}
/*******************************************************************************/
/*******************************************************************************/
BOOL siSetSharkInput( VOID )
{
	LPINPUT_t inp = &inputs[uiInput];
	UINT cinput = g_pfnS4GETINPUT();
	BOOL	bTV=FALSE, bRGB=FALSE, bDig=FALSE;

	g_pfnS4SETINPUT(inp->input+1);

	switch(inp->ccd) {
	case 0:	// Mono
		g_pfnS4SETSOURCETYPE( SH4_CCD_MONO );
		goto ccc;
	case 1:	// CVBS
		g_pfnS4SETSOURCETYPE( SH4_CCD_COMPOSITE );
		goto ccc;
	case 2: // S-Video
		g_pfnS4SETSOURCETYPE( SH4_CCD_SVIDEO );
ccc:	if (!g_pfnS4SETPARAMETER(SH4_P_TBC,   inp->tbc)) goto errex;
		if (!g_pfnS4SETPARAMETER(SH4_P_CTRAP, inp->ctrap)) goto errex;
		break;
	case 3: // RGB
		break;
	
	case 4: // Digital
	case 5: // LVDS_link
		if (!g_pfnS4SETRESOLUTION(inp->x,inp->y)) goto errex;	//Digital, set resolution
//		s4SetParameter(SH4_P_KODAKTRIG, TRUE);
		g_pfnS4SETPARAMETER(SH4_P_EXPTIME,  inp->exposure);
		g_pfnS4SETPARAMETER(SH4_P_GPO1,     inp->gpo1);
		g_pfnS4SETPARAMETER(SH4_P_GPO2,     inp->gpo2);
		break;
	}
	// Peter on 2004 June 28
	if( g_pfnS4SETCLIPPING && g_pfnS4SETPARAMETER )
	{
		if( !g_pfnS4SETCLIPPING(inp->x0, inp->y0, inp->x1, inp->y1) )
			goto errex;
		g_pfnS4SETPARAMETER( SH4_P_AMODE, inp->amode );
		g_pfnS4SETPARAMETER( SH4_P_ADEPTH, uADepths[inp->adepth] );
	}
//	if (!s4SetClipping(inp->x0, inp->y0, inp->x1, inp->y1 )) goto errex;
//	s4SetParameter( SH4_P_AMODE, inp->amode );
//	s4SetParameter( SH4_P_ADEPTH, uADepths[inp->adepth] );
	return TRUE;
errex:
	if( g_pfnS4SETINPUT )
	{
		g_pfnS4SETINPUT(cinput);
//		s4SetInput(cinput);
	}
	return FALSE;
}
/*******************************************************************************/
BOOL    CALLBACK SetInputDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
int	i;
static UINT oldinput;
static BOOL bRet;

	switch (message) {
		case WM_INITDIALOG:
			for(i=0;i<CCDs;i++) SendDlgItemMessage(hDlg,IDC_I_CCDTYPE,CB_INSERTSTRING,-1,(LPARAM)cCCDs[i]);
			for(i=0;i<AMODEs;i++) SendDlgItemMessage(hDlg,IDC_I_AMODE,CB_INSERTSTRING,-1,(LPARAM)cAModes[i]);
			for(i=0;i<ADEPTHs;i++) { 
				sprintf(TMP, "%d", uADepths[i]);
				SendDlgItemMessage(hDlg,IDC_I_ADEPTH,CB_INSERTSTRING,-1,(LPARAM)TMP);
			}
			EnableWindow( GetDlgItem(hDlg,IDC_I_APPLY), FALSE );
			UpdateInputInfo(hDlg);
			FillInputList(hDlg);
			oldinput = uiInput;
			regReadWindowPos( hDlg, "S4Inputs");
			bRet = FALSE;	// Could be without updates
			return TRUE;

		case WM_NCDESTROY:
			regWriteWindowPos( hDlg, "S4Inputs", FALSE);
			break;

		case WM_HSCROLL:
			switch (GetDlgCtrlID ( (HWND)lParam ))
			{
			case IDC_I_GAIN: UpdateSliderCtl ( hDlg, wParam,lParam, &sbGain ); /*s4SetParameter( SH4_P_GAIN, sbGain.p); */break;
			case IDC_I_OFFS: UpdateSliderCtl ( hDlg, wParam,lParam, &sbOffs ); /*s4SetParameter( SH4_P_OFFS, sbOffs.p); */break;
			case IDC_I_BRIG: UpdateSliderCtl ( hDlg, wParam,lParam, &sbBrig ); /*s4SetParameter( SH4_P_BRIG, sbBrig.p); */break;
			case IDC_I_CONT: UpdateSliderCtl ( hDlg, wParam,lParam, &sbCont ); /*s4SetParameter( SH4_P_CONT, sbCont.p); */break;
			}
			return TRUE;

		case WM_COMMAND:
			EnableWindow( GetDlgItem(hDlg,IDC_I_APPLY), TRUE );
		switch (LOWORD(wParam)) {
			case IDC_I_FULLG: 
				g_pfnS4SETPARAMETER( SH4_P_FULLGO, IsDlgButtonChecked( hDlg, IDC_I_FULLG ) );
				break;

			case IDC_I_LIST:
				if (HIWORD(wParam)==LBN_SELCHANGE) {
					uiInput = SendDlgItemMessage( hDlg, IDC_I_LIST, LB_GETCURSEL, 0,0 );
					UpdateInputInfo( hDlg );
				}
			case IDC_I_CCDTYPE:
				if (HIWORD(wParam)==CBN_SELCHANGE) {
					inputs[uiInput].ccd = SendDlgItemMessage( hDlg, IDC_I_CCDTYPE, CB_GETCURSEL, 0,0 );
					UpdateInputInfo( hDlg );
				}
				break;
			case IDC_I_DELETE:
				inputs[uiInput].exist=FALSE;
				for(i=uiInput;i<NINPUTS-1;i++) {
					inputs[i] = inputs[i+1];
				}
				if (uiInput) uiInput--;
				UpdateInputInfo( hDlg );
				FillInputList(hDlg);
				break;

			case IDC_I_NEW:
				for(i=0;i<NINPUTS;i++) {
					if(!inputs[i].exist) break;
				}
				if (i==NINPUTS) {MessageBeep(0xFFFF); break;}
				uiInput = i;
				inputs[i]=defTV;
				inputs[i].exist = TRUE;
				UpdateInputInfo( hDlg );
				FillInputList(hDlg);
				break;

			case IDC_I_DEFAULT:
				sbGain.p = sbGain.def;
				sbOffs.p = sbOffs.def;
				sbBrig.p = sbBrig.def;
				sbCont.p = sbCont.def;
				UpdateInputInfo( hDlg );
				break;

			case IDC_I_APPLY:
			case IDOK:
				ScanInput( hDlg );
				UpdateInputInfo( hDlg );
				siSetSharkInput();
				FillInputList(hDlg);
				siSaveInputs();
				oldinput = uiInput;
				bRet = TRUE;
				EnableWindow( GetDlgItem(hDlg,IDC_I_APPLY), FALSE );
				if (LOWORD(wParam)==IDC_I_APPLY) break;
				DestroyWindow(hDlg);
				return TRUE;

			case IDCANCEL:
				siInitInputs();
				uiInput = oldinput;
				siSetSharkInput();
				DestroyWindow(hDlg);
				return TRUE;
		}
	}
	return FALSE;
}
