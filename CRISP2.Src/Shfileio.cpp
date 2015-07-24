/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.shfileio * by ML **************************************************/
/****************************************************************************/

#pragma warning ( disable : 4786 )

#include "StdAfx.h"

#include "FEI.h"
#include "RawDlg.h"

#include "..\shjpeg32\shjpeg32.h"

#include "objects.h"
#include "common.h"
#include "shtools.h"
#include "globals.h"
#include "const.h"
#include "commondef.h"
#include "resource.h"
#include "blowfish\blowfish.h"
#include "rgconfig.h"

#define		_NDR_24BIT_

BOOL CRISP_InitObjFromImage(OBJ* pObj, SAVEIMAGE &S);
//OBJ* CRISP_CreateObject(OBJ* pParent, OBJ* OE, BOOL &bExternalObject, HWND hParent);

// uncommented by Peter on 6 Nov 2001
#ifdef _DEBUG
#pragma optimize ("",off)
#endif

#pragma pack(push, 1)

#define	_ltell(a) _llseek(a,0,FILE_CURRENT)

#pragma optimize ("",off)

// added by Peter on 10 Oct 2005 for FEI .ser file support
/*
#if defined __FEI__

typedef enum
{
	ESD_1D_ARRAY = 0x4120,		// data elements are 1D arrays
	ESD_2D_ARRAY = 0x4122,		// data elements are 2D arrays

} ESD_SERIES_TYPE_ID;

typedef enum
{
	ESD_TIME_TAG = 0x4152,		// tag is time only
	ESD_TIME_POS_ARRAY = 0x4142,// tag is 2D position with time

} ESD_TAG_TYPE_ID;

typedef enum
{
	ESD_DATA_UBYTE	= 1,
	ESD_DATA_UINT2	= 2,
	ESD_DATA_UINT4	= 3,
	ESD_DATA_BYTE	= 4,
	ESD_DATA_INT2	= 5,
	ESD_DATA_INT4	= 6,
	ESD_DATA_FLT4	= 7,
	ESD_DATA_FLT8	= 8,
	ESD_DATA_CMPLX4	= 9,
	ESD_DATA_CMPLX8	= 10,

} ESD_DATA_TYPE_ID;

typedef struct
{
	WORD	wByteOrder;			// 'II' = 0x4949, little endian (PC)
	WORD	wSeriesID;			// 0x0197 - indicates the ES version of the file
	WORD	wSeriesVer;			// version of the series data file (right now is 0x0210)
	DWORD	dwDataTypeID;		// series type, one of ESD_SERIES_TYPE_ID
	DWORD	dwTagTypeID;		// the type of tag in the series, one of ESD_TAG_TYPE_ID
	DWORD	dwTotalElements;	// total number of data elements and tags = width*height
	DWORD	dwValidNumElements;	// valid number of elements and tags, if not all data
								// were written then can be < dwTotalElements
	DWORD	dwOffsetArray;		// absolute offset (in bytes) in the ESD file
	DWORD	dwNumberDims;		// number of dimensions of the series data file

} FEI_ESD_HEADER;

typedef struct
{
	DWORD	dwDimSize;			// number of elements in this dimension
	double	dCalibrOffset;		// calibration values at element dwCalibrElement
	double	dCalibrDelta;		// calibration delta between elements of the series
	DWORD	dwCalibrElement;	// the element in the series which has a calibration values of CalibrOffset
	DWORD	dwDescrLength;		// the length of the description string
//	char	*pszDescription;	// sizeof Description length
//	DWORD	dwUnitLength;
//	char	*pszUnits;

} FEI_ESD_DIM_ARRAY_header;

class FEI_ESD_DIM_ARRAY
{
public:
	FEI_ESD_DIM_ARRAY(const FEI_ESD_DIM_ARRAY_header &hdr, LPTSTR pszDescr, DWORD _dwUnitLength)
		: sDescription(pszDescr)
	{
		dwDimSize = hdr.dwDimSize;
		dCalibrOffset = hdr.dCalibrOffset;
		dCalibrDelta = hdr.dCalibrDelta;
		dwCalibrElement = hdr.dwCalibrElement;
		dwDescrLength = hdr.dwDescrLength;
		dwUnitLength = _dwUnitLength;
	}
	~FEI_ESD_DIM_ARRAY()
	{
		std::swap(sDescription, std::string(""));
		std::swap(sUnits, std::string(""));
	}
	DWORD	dwDimSize;			// number of elements in this dimension
	double	dCalibrOffset;		// calibration values at element dwCalibrElement
	double	dCalibrDelta;		// calibration delta between elements of the series
	DWORD	dwCalibrElement;	// the element in the series which has a calibration values of CalibrOffset
	DWORD	dwDescrLength;		// the length of the description string
	std::string	sDescription;	// sizeof Description length
	DWORD	dwUnitLength;
	std::string	sUnits;
//protected:
	int _destroy(/ *FEI_ESD_DIM_ARRAY *ptr* /)
	{
		delete this;
		return 0;
	}
};

typedef std::vector<FEI_ESD_DIM_ARRAY*>					DataArrayVec;
typedef std::vector<FEI_ESD_DIM_ARRAY*>::iterator		DataArrayVecIt;
typedef std::vector<FEI_ESD_DIM_ARRAY*>::const_iterator	DataArrayVecCit;

typedef struct
{
	double	dCalibrOffset;
	double	dCalibrDelta;
	DWORD	dwCalibrElement;
	WORD	wDataType;			// one of ESD_DATA_TYPE_ID
	DWORD	dwArrayLength;		// the number of elements in the array
//	data values follow

} FEI_ESD_1D_DATA;

typedef struct
{
	double	dCalibrOffsetX;
	double	dCalibrDeltaX;
	DWORD	dwCalibrElementX;
	double	dCalibrOffsetY;
	double	dCalibrDeltaY;
	DWORD	dwCalibrElementY;
	WORD	wDataType;			// one of ESD_DATA_TYPE_ID
	DWORD	dwArraySizeX;		// the number of elements in the array along X
	DWORD	dwArraySizeY;		// the number of elements in the array along Y
//	data values follow

} FEI_ESD_2D_DATA;

typedef struct
{
	WORD	wTagTypeID;			// right now is 0x4152
	float	fTime;				// the time in ANSI standard (secs since 1 Jan 1970)

} FEI_ESD_TIME_TAG;

typedef struct
{
	WORD	wTagTypeID;			// right now is 0x4152
	float	fTime;				// the time in ANSI standard (secs since 1 Jan 1970)
	double	dPositionX;			// the position of the tag in X & Y directions
	double	dPositionY;

} FEI_ESD_TIME_POS_TAG;

#endif // __FEI__
*/

// added by Peter on 20 Aug 2002 for DITABIS file support
#define IDENTIFIER			258
#define NUMBER				259
#define DITABIS_XPIXEL		260
#define DITABIS_YPIXEL		261
#define DITABIS_BYTEPP		262
#define DITABIS_XRESOL		263
#define DITABIS_YRESOL		264
#define DITABIS_HEADER		265
#define DITABIS_COMMENT		270

/*
typedef struct
{
	short width;		// XPIXEL field
	short height;		// YPIXEL field
	short bps;			// BYTE PER PIXEL field
	short xres;			// XRESOLUTION field
	short yres;			// YRESOLUTION field
	// the rest of the header is just skipped

} DITABIS_HEADER;
*/

//////////////////////////////////////////////////////////////////////////

// Added by Peter on [18/4/2006]
// This is Nonius IPF data file support
typedef struct _CO_TAIL
{
	char    id[4];              /* data mark (general = DIP0)         */
	long    m_type1;            /* machine reading mode:              */
								/* == 0:line                          */
								/* else:spiral(IP diameter[0.1mm]     */
	long    m_type2;            /* 0: flat cassette                   */
								/* 1: Cylindrical cassette (Para)     */
								/* 2: Cylindrical cassette (Perp)     */
	long    dtype;              /* 0: short integer(4+12bits)         */
								/* 1: unsigned short(16bits)          */
								/* 2: short integer(1+15bits)         */
								/* n ... ... ... ... ... ... ...      */
	long    pixelsize;          /* pixel size (main) (um)             */
	long    pixelsize2;         /* pixel size (sub)  (um)             */
	float   radius;             /* radius of cylinder (mm)            */
	long    xsize;              /* Number of pixels along x coord     */
	long    ysize;              /* Number of pixels along y coord     */
	long    ipno;               /* imaging plate No.(1/2)             */
	char    comment[80];        /* comment message                    */
	float   x_lamda;            /* X-ray wave length  (A)             */
	float   cdist;              /* camera distance (mm)               */
	char    monochro[32];       /*  monochro parameter                */
	float   pttheta;            /* 2Theta angle (deg)                 */
								/* in Weissenberg configuration       */
								/* this is used to define mu angle    */
	long    ipx;                /* P position (x) of pixel adress     */
	long    ipy;                /* P position (y) of pixel adress     */
	float   exposure;           /* Exposure time (sec)                */
	float   kv;                 /* X.G. voltage (kv)                  */
	float   ma;                 /* X.G. current (mA)                  */
	float   collimator;         /* collimator diameter (mm)           */
	float   coupling;           /* ==  0: No weissenberg motion       */
								/* not 0: Weissenberg motion          */
								/*    for DIP2000 unit: deg/deg       */
								/*    for DIP3000 orcylindrical       */
								/*           type unit: mm/deg        */
	float    phi1;              /* Phi start angle (deg)              */
	float    phi2;              /* Phi ended angle (deg)              */
	float    phispeed;          /* Phi speed (deg/min)                */
	long     repet;             /* repetition number                  */
	long     osc_axis;          /* oscillation axis                   */
								/* 0: phi                             */
								/* 1: omega                           */
								/* 2: kappa                           */
	float    g_omega;           /* omega angle (deg)                  */
	float    g_kappa;           /* kappa angle (deg)                  */
	float    g_phi;             /* phi   angle (deg)                  */
	long     xstart;            /* start pixel position along x coord */
	long     ystart;            /* start pixel position along y coord */
	char     dummy[152];        /* for fist part (384 bytes)          */
} CO_TAIL;

typedef struct _SR_TAIL
{
	long colour;                    /* Current color mode             */
	long max_val;                   /* Current Max_Show_Value         */
	long min_val;                   /* Current Min_Show_Value         */
	long start_x;                   /* AOI start point ipx value      */
	long start_y;                   /* AOI start point ipy value      */
	long AOI_w;                     /* AOI width (in IP pixel)        */
	long AOI_h;                     /* AOI width (in IP pixel)        */
	long f1x;                       /* Fiducial point 1 in x          */
	long f1y;                       /* Fiducial point 1 in y          */
	long f2x;                       /* Fiducial point 2 in x          */
	long f2y;                       /* Fiducial point 2 in y          */
	char dummy[596];                /* part 2 for screen parameters   */
} SR_TAIL;

typedef struct _TAILER
{
	CO_TAIL part1;                  /* part1  284 bytes               */
	SR_TAIL part2;                  /* part2  640 bytes               */
	/* Total 1024 bytes               */
} TAILER;

// End of addition [18/4/2006]
/****************************************************************************/
typedef struct {
	char	manuf, hard, encod, bitpix;
	short	x1, y1, x2, y2, hres, vres;
	char	clrma[48], vmode, nplanes;
	short	bplin, palinfo, shres, svres;
	char	extra[54];
} PCX_HEADER;
/*--------------------------------------------------------------------------*/
typedef struct {
	char	sign[10];	// Always "cCrRiIsSpP"
	WORD	CRC, min, max;
}  IMG_HEADER;
/*--------------------------------------------------------------------------*/
typedef struct {
	char	logo[2];		// Always "IM"
	short	com_length;		// Length of comment ( followed by this header )
	short	width;			//
	short	height;			//
	short	startx;			//
	short	starty;			//
	short	type;			// I suppose that 0 means EIGHT_BIT
	char	ImageDevice[4];	//
} IMG_WINSTOR;
static int winstortype;
#define	EIGHT_BIT	0
#define	COMPRESSION	1
#define	SIXTEEN_BIT	2
#define	TWELVE_BIT	4
//#define	BYTESWAP	0
/*--------------------------------------------------------------------------*/
typedef struct {
	BYTE	id;			/* 0xAB */
	BYTE	f_t;		/* 0x01 */
	BYTE	rel;		/* 0x01 */
	BYTE	ver;		/* 0x03 */
	char	cmt[512];	/* Comment */
	DWORD	cpu;		/* 0 */
	DWORD	row_size, col_size;	/* X,Y dim */
	DWORD	subrow_size;		/* ignore */
	DWORD	startx, starty;		/* ignore */
	float	pixsizx, pixsizy;	/* actual pixel size ignore */
	DWORD	location_type;		/* only 1 (VFF_LOC_EXPLICIT?) */
	DWORD	location_dim;		/* only 0 */
	DWORD	num_of_images;		/* only 1 */
	DWORD	num_data_bands;		/* only 1 */
	DWORD	data_storage_type;	/* only 1 (VFF_TYP_1_BYTE?) */
	DWORD	data_encode_scheme;	/* only 0 (VFF_DES_RAW?) */
	DWORD	map_scheme;			/* only 1 (VFF_MS_ONEPERBAND?) */
	DWORD	map_storage_type;	/* only 1 (VFF_MAPTYP_1_BYTE?) */
	DWORD	map_row_size;		/* no of LUTs 1 or 3 */
	DWORD	map_col_size;		/* LUTs size, only 256 */
	DWORD	map_subrow_size;	/* 0 */
	DWORD	map_enable;			/* 1 */
	DWORD	maps_per_cycle;		/* 0 */
	DWORD	color_space_model;	/* 0 for 1 LUT, F for 3 LUTs */
} VIFFHEADER;
/*--------------------------------------------------------------------------*/
typedef struct {
	WORD	len;		// Number of 128byte header blocks
	WORD	id1;		// Must be 0x1247
	WORD	id2;		// Must be 0xB06D
	WORD	width;		// Image width
	WORD	height;		// Image height
	WORD	id3;		// Must be 0x1234
	WORD	imgcnt;		// Number of images in file
} KONTRONHDR;

/****************************************************************************/
static	PCX_HEADER pcx_header;

static	char	WINSTORCOM[] = "CRISP export to WINSTOR";
static	int		MRC_Mode;


#define	_PCX_		1
#define	_WINSTOR_	2
#define	_CRISP_		3
#define	_TIFF_		4
#define	_JPEG_		5
#define	_MRC_		6
#define	_BMP_		7
#define	_VIF_		8
#define	_KONTRON_	9
#define	_SPE_		10
#define	_TIF_CRY_	11
#define _DITABIS_	12
#define _RAW_		13
#define _DM2_		14		// Gatan v.2
#define _DM3_		15		// Gatan v.3
#ifdef __FEI__
#define _SER_		16		// FEI ESD .ser files
#define _EMI_		17		// FEI .emi files
#endif
#define _IPF_		18		// Nonius IPF file format
#define _NDR_		19		// NanoMEGAS drive file format

/****************************************************************************/
#define	BUFFSIZE	4096			// Temp. buffer size
static	char	filebuff[BUFFSIZE];	//
static	int		filebuffptr;		// Pointer to the next byte in buffer

static	BOOL	bFirst;		// On first callback we have nothing to save
static	DWORD	dwCnt;		// Counter of saved bytes
static	HFILE	hFile;		// File handle

static	BOOL	bDemoPCX;
/****************************************************************************/
BOOL fioFixImageMemory( LPSAVEIMAGE I )
{
	DWORD	size;

// 	I->xb = R4( I->x );
// 	size = I->xb * ((I->y+7)&0xFFF8) * (I->ps); // Rounding Y for JPEG coder
#ifdef USE_XB_LENGTH
	I->xb = R4( I->x );
	size = I->xb * ((I->y+7)&0xFFF8) * (I->ps); // Rounding Y for JPEG coder
#else
	I->stride = R4( I->x * I->ps );
	size = I->stride * ((I->y+7) & 0xFFF8); // Rounding Y for JPEG coder
#endif
	if ((I->lpData) && (size != GlobalSize(GlobalPtrHandle(I->lpData))))
	{
		free( I->lpData );
		I->lpData = NULL;
	}
	if( NULL == I->lpData )
	{
		if( NULL == (I->lpData = (LPBYTE)calloc(1, size)) )
		{
			MessageBeep(0xFFFF);
			tobSetInfoText(ID_INFO, "Not enough memory!");
			return FALSE;
		}
	}
	return TRUE;
}
/****************************************************************************/
void fioReportPercent( int c, int tot, BOOL bRead )
{
	static int	perc0=-1, perc;
	char	str[100];

	if (tot) perc = (c*100)/(tot);
	if (perc==perc0) return;
	perc0=perc;

	if (tot) {
		if (bRead) sprintf(str,"Reading at %d%%", perc );
		else       sprintf(str,"Writing at %d%%", perc );
	} else {
		if (bRead) sprintf(str,"Reading done");
		else       sprintf(str,"Writing done");
		perc = 100;
	}
	tobSetInfoTextAndBar(ID_INFO, str, perc);
}
/****************************************************************************/
static DWORD dwLeft, dwRight;
static void initcypher( BOOL bEncrypt )
{
static BYTE szPSF[] = "SoftHard Technology, Ltd.";

	Blowfish_SetRounds(32);
	Blowfish_Init( szPSF, sizeof(szPSF)-1 );
	if (Blowfish_WeakKey()) MessageBox(NULL,"WeakKey","Privet", MB_ICONSTOP);
	dwLeft = 0x13071963;
	dwRight = 0x2173AA55;
}
/*--------------------------------------------------------------------------*/
static void encrypt( LPBYTE out, LPBYTE in, UINT cnt )
{
//	Blowfish_CBCEncrypt( (LPDWORD)out, (LPDWORD)in, cnt, &dwLeft, &dwRight );
	Blowfish_ECBEncrypt( (LPDWORD)out, (LPDWORD)in, cnt );
}
/*--------------------------------------------------------------------------*/
static void decrypt( LPBYTE out, LPBYTE in, UINT cnt )
{
//	Blowfish_CBCDecrypt( (LPDWORD)out, (LPDWORD)in, cnt, &dwLeft, &dwRight );
	Blowfish_ECBDecrypt( (LPDWORD)out, (LPDWORD)in, cnt );
}
/****************************************************************************/
/* Formerly ASM routines *****************************************************/
/****************************************************************************/
void XlatMove (LPBYTE dst, LPBYTE src, LPBYTE xtbl, int cnt )
{
int		i;
	for(i=0;i<cnt;i++,dst++,src++) *dst = xtbl[*src];
}
/****************************************************************************/
/* JPEG coder ***************************************************************/
/****************************************************************************/
static int CALLBACK fnSetData( LPDWORD lplpBuf, LPDWORD lpdwSize )
{
	DWORD	cnt=BUFFSIZE;
	if (bFirst)	bFirst=FALSE;		// First call
	else if (BUFFSIZE != _lwrite(hFile, filebuff, BUFFSIZE)) cnt = 0;
	dwCnt += cnt;
	*lpdwSize = (DWORD)cnt;
	*lplpBuf = (DWORD)filebuff;
	return cnt != 0;
}

/****************************************************************************/
static BOOL fioSaveJPG( LPSTR szFname, LPSAVEIMAGE I, WORD wJQ )
{
	BOOL	ret=TRUE;
	JINFO	J;
	WORD	h, bw;
	DWORD	dwRest;
	BYTE	* lpImg;
	int		iPixSize;

	if ((hFile = _lcreat(szFname,0)) == HFILE_ERROR) return FALSE;

	J.image_width = I->x;
	J.image_height = I->y;
	J.quality = wJQ;
	J.density_unit = 0;
	J.X_density = 1;
	J.Y_density = 1;
	J.color_space = (I->ps==3) ? CS_RGB : CS_GRAYSCALE;
	J.precision = 8;

	bFirst = TRUE;
	dwCnt = 0;
	if (jpgCodOpen(&J, (LPBYTE)I->lpAdd, I->iAddSize, fnSetData ) == 0) {
		_lclose(hFile);
		return FALSE;
	}
	lpImg = I->lpData;
#ifdef USE_XB_LENGTH
	bw = I->xb;
#else
	bw = R4(I->x);
#endif
	iPixSize = I->ps;
	I->strip = J.strip_height;

	for( h=0; h<I->y; h+=I->strip )
	{
		if (jpgCodStrip(lpImg + bw*h*iPixSize,bw*iPixSize) == 0)
			{ ret = FALSE; break; }
		fioReportPercent( h, I->y-I->strip, FALSE );
	}
	fioReportPercent( 0, 0, FALSE );

	dwRest = jpgCodClose();	// Get the number of bytes in buffer, after last flush
	if (_lwrite(hFile, filebuff, dwRest) != dwRest) ret = FALSE;	// and save it
	_lclose(hFile);

	return ret;
}
/****************************************************************************/
/* JPEG decoder *************************************************************/
/****************************************************************************/
/***************************************************************/
BOOL CALLBACK fnGetData( LPDWORD lplpBuf, LPDWORD lpdwSize )
{
UINT	cnt;
	cnt = _lread(hFile, filebuff, BUFFSIZE);
	*lpdwSize = (DWORD)cnt;
	*lplpBuf = (DWORD)filebuff;
	return (cnt!=0);
}
/****************************************************************************/
static BOOL	fioCheckJPEG( LPSAVEIMAGE I )
{
	JINFO	J;
	DWORD	y = sizeof(NEWINFO);
	NEWINFO	NI;

	y = 0;
	if (! jpgDecOpen(&J, (LPBYTE)&NI, &y, fnGetData ) || ((J.precision!=8) && (J.precision!=12)) ) {
		jpgDecClose();
		return FALSE;
	}
	I->x = J.image_width;
	I->y = J.image_height;
	I->strip = J.strip_height;
	I->ps = (J.precision==8) ? 1:2;
	if( 1 == I->ps )
	{
		I->_pix = PIX_BYTE;
	}
	else if( 2 == I->ps )
	{
		I->_pix = PIX_WORD;
	}
	if (!fioFixImageMemory( I ))
	{
		jpgDecClose();
		return FALSE;
	}
	if ((NI.logo[0] == 'N') && (NI.logo[1] == 'I')) memcpy(I->lpAdd, &NI, I->iAddSize);
	return TRUE;
}
/****************************************************************************/
static BOOL fioReadJPEG( LPSAVEIMAGE I )
{
	int		y;
	BOOL	ret=TRUE;

	for( y=0; y<I->y; y+=I->strip )
	{
#ifdef USE_XB_LENGTH
		if (jpgDecStrip(I->lpData + I->xb*y*I->ps, I->xb*I->ps, CS_GRAYSCALE) == 0)
#else
		if( 0 == jpgDecStrip(I->lpData + I->stride*y, I->stride, CS_GRAYSCALE) )
#endif
		{
			ret = FALSE;
			break;
		}
		fioReportPercent( y, I->y-I->strip, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	jpgDecClose();
	return ret;
}
/****************************************************************************/
/* TIFF coder ***************************************************************/
/****************************************************************************/
/* Externals from FILEUTIL.ASM **********************************************/

static BYTE	x_lookup[256];

#define	INTELTIFF		(0x4949)
#define	MOTOROLATIFF	(0x4d4d)

typedef	int		RC;
#define	SUCCESS	0

// TIFF data types
#define TIFFBYTE		1
#define TIFFASCII		2
#define TIFFSHORT		3
#define TIFFLONG		4
#define TIFFRATIONAL	5

// TIFF tag constants
#define TGNEWSUBFILETYPE			254
#define TGSUBFILETYPE				255
#define TGIMAGEWIDTH				256
#define TGIMAGELENGTH				257
#define TGBITSPERSAMPLE				258
#define TGCOMPRESSION				259

#define TGPHOTOMETRICINTERPRETATION	262
#define TGTHRESHHOLDING				263
#define TGCELLWIDTH					264
#define TGCELLLENGTH				265
#define TGFILLORDER					266
#define TGDOCUMENTNAME				269
#define TGIMAGEDESCRIPTION			270
#define TGMAKE						271
#define TGMODEL						272

#define TGSTRIPOFFSETS				273
#define TGORIENTATION				274

#define TGSAMPLESPERPIXEL			277
#define TGROWSPERSTRIP				278
#define TGSTRIPBYTECOUNTS			279
#define TGMINSAMPLEVALUE			280
#define TGMAXSAMPLEVALUE			281
#define TGXRESOLUTION				282
#define TGYRESOLUTION				283
#define TGPLANARCONFIGURATION		284
#define TGPAGENAME					285
#define TGXPOSITION					286
#define TGYPOSITION					287
#define TGFREEOFFSETS				288
#define TGFREEBYTECOUNTS			289

#define TGGARYRESPONSEUNIT			290
#define TGGARYRESPONSECURVE			291

#define TGRESOLUTIONUNIT			296
#define TGSOFTWARE					305
#define TGDATETIME					306

#define	TGPREDICTOR					317

#define TGCOLORMAP					320

#define	TGSAMPLEFORMAT				339

#define TGPROJINFO					33650

// TIFF "header" (8 bytes)
// note: GtTiffHdr plays a little loose with this structure.
typedef struct {
		WORD	thByteOrder;
		WORD	thVersion;
		DWORD	thIfdOffset;
} TIFFHDR;

// IFD entry
// note: GtTiffEntry plays a little loose with this structure.
typedef struct {
		WORD  deTag;
		WORD  deType;
		DWORD deLength;
		DWORD deVal;
} DIRENTRY;

// image data location
typedef struct {
	DWORD	dlWhere;
	#define		INFILE	1
	#define		INTABLE	2

	HFILE	hFile;
//	FILE	*dlFp;
	LPSTR	dlTable;				// address of locked-down table bytes
	DWORD	dlOrder;				// INTELTIFF or MOTOROLATIFF relevant only when reading data.
} DLOC;

static	DWORD	tifX, tifY;			// Image dimensions
static	DWORD	tifNStrips;			// Strip count
static	DWORD	tifStrip;			// Current strip no
static	DWORD	tifLine;			// Current line no
static	DWORD	tifLineInStrip;		// Line no in strip
static	DWORD	tifRowsPerStrip;	//
static	DWORD	tifRWptr;			// Position in file
static	DWORD	tifOffsets;			// Position of "StripOffsets" data in file
static	DWORD	tifOffsetsSize;		// Size of strip offsets TIFFSHORT or TIFFLONG
static	DWORD	tifBitsPerPixel;	//
static	DWORD	tifSamplesPerPixel;	//

static	DWORD	tifSampleFormat;	// Samples format
#define		SfUNSIGNED	1
#define		SfTWOSCOMPL	2
#define		SfIEEE		3			// IEEE floating point
#define		SfUNDEFINED	4			//

static	DWORD	tifCompr;			// TIFF compression scheme
#define		CoNOCOMP	1			// No compression
#define		CoLZW		5			// LZW
#define		CoPACKBIT	32773		// Pack Bits

static	DWORD	tifPhoto;			// PhotometricInterpretation
#define		PhZEROWHITE		0
#define		PhZEROBLACK		1
#define		PhRGB			2
#define		PhPALETTE		3
#define		PhTRANSP		4

static	DWORD	tifPrediction;		// Horizontal prediction is used

static	char	tifMAKE[] = "SoftHard Technology, Ltd.";

static	DLOC	Dloc;

//---------------------------- subroutines ----------------------------------
// swap bytes -- overlapping arrays are handled properly
static void tswab (LPSTR lpSrc, LPSTR lpDst, int nbytes)
{
register WORD words;
union {
	char c[2];
	WORD w;
} wrd;

	words = nbytes/2;

	if ((lpDst <= lpSrc) || (lpDst >= lpSrc + nbytes)) {
		for (; words--; lpSrc += 2) {
			wrd.w = *(WORD *)lpSrc;
			*lpDst++ = *(LPSTR)(wrd.c + 1);	/* W2 doesn't like wrd.c[1] */
			*lpDst++ = *(LPSTR)(wrd.c);
		}
	}
	else {		/* we'll have to go backward */
		lpSrc += nbytes - sizeof(WORD);
		lpDst += nbytes - 1;
		for (; words--; lpSrc -= 2) {
			wrd.w = *(WORD *)lpSrc;
			*lpDst-- = *(LPSTR)(wrd.c);
			*lpDst-- = *(LPSTR)(wrd.c + 1);
		}
	}
}
//---------------------------------------------------------------------------
// swap words -- overlapping ranges are handled properly
static void swaw (LPSTR lpSrc, LPSTR lpDst, int nbytes)
{
register WORD dwords;
union {
	char c[4];
	DWORD dw;
} dwrd;

	dwords = nbytes/4;

	if ((lpDst <= lpSrc) || (lpDst >= lpSrc + nbytes)) {
		for (; dwords--; lpSrc += 4) {
			dwrd.dw = *(DWORD *)lpSrc;
			*lpDst++ = *(LPSTR)(dwrd.c + 3);
			*lpDst++ = *(LPSTR)(dwrd.c + 2);
			*lpDst++ = *(LPSTR)(dwrd.c + 1);
			*lpDst++ = *(LPSTR)(dwrd.c);
		}
	}
	else {		/* we'll have to go backward */
		lpSrc += nbytes - sizeof(DWORD);
		lpDst += nbytes - 1;
		for (; dwords--; lpSrc -= 4) {
			dwrd.dw = *(DWORD *)lpSrc;
			*lpDst-- = *(LPSTR)(dwrd.c);
			*lpDst-- = *(LPSTR)(dwrd.c + 1);
			*lpDst-- = *(LPSTR)(dwrd.c + 2);
			*lpDst-- = *(LPSTR)(dwrd.c + 3);
		}
	}
}
//---------------------------------------------------------------------------
//
static RC GtTiffSizeof (int n, int * p)
{
RC	err = SUCCESS;

	switch (n) {
	case TIFFBYTE:
	case TIFFASCII:
		*p = 1;
		break;
	case TIFFSHORT:
		*p = 2;
		break;
	case TIFFLONG:
		*p = 4;
		break;
	case TIFFRATIONAL:
		*p = 8;
		break;
	default:
		*p = 1;
		err = -1;
		break;
	}
	return err;
}
//---------------------------------------------------------------------------
// get data -- handles file/table and byte-order problems
// 64K max
static	RC GtData (DLOC *pDloc, DWORD pos, int n, int dtype, LPSTR lpData)
//DLOC	*pDloc;		/* data location - open file or locked-down table */
//DWORD	pos;		/* file/table position, with respect to its beginning */
//WORD	n;			/* number of data elements to read */
//WORD	dtype;		/* data type: TIFFSHORT, etc */
//LPSTR	lpData;		/* where to put the data */
{
RC		err;
int		tsize;
int		BytesToRead;
int		red;		// # of bytes read

	// read the data
	if (err = GtTiffSizeof (dtype, &tsize)) return err;
	BytesToRead = tsize * n;
	if (pDloc->dlWhere == INFILE) {
		if ((err = _llseek (pDloc->hFile, pos, FILE_BEGIN)) == HFILE_ERROR )
			return err;
		if ((red = _lread (pDloc->hFile, lpData, BytesToRead )) == 0)
			return -1;
	}
	else if (pDloc->dlWhere == INTABLE)	return -1;
	else return -1;

	// change the byte order, if necessary
	if (pDloc->dlOrder == MOTOROLATIFF) {
		if (dtype == TIFFSHORT)
			tswab (lpData, lpData, BytesToRead);
		else if (dtype == TIFFLONG)
			swaw (lpData, lpData, BytesToRead);
		else if (dtype == TIFFRATIONAL)
			swaw (lpData, lpData, BytesToRead);
	}

	// return
	return SUCCESS;
}

//---------------------------------------------------------------------------
// get TIFF 8-byte header
// currently only probably portable.  depends somewhat on compiler's
// structure organization.
static RC GtTiffHdr (DLOC *pDloc, TIFFHDR *pHdr)
{
RC err;

	// get the first 2 words
	if (err = GtData (pDloc, (DWORD)0, 2, TIFFSHORT, (LPSTR)&pHdr->thByteOrder))
		return err;

	// get the double word (IFD offset)
	if (err = GtData (pDloc, (DWORD)4, 1, TIFFLONG,	 (LPSTR)&pHdr->thIfdOffset))
		return err;

	// return
	return SUCCESS;
}

//---------------------------------------------------------------------------
// get TIFF directory entry
static RC GtTiffEntry (DLOC *pDloc, DWORD EntryOffset, DIRENTRY *pDe)
{
RC err;

	// get the 2 words beginning with deTag
	if (err = GtData (pDloc, EntryOffset, 2, TIFFSHORT,	 (LPSTR)&pDe->deTag))
		return err;

	// get the 2 dwords, beginning with deLength
	if (err = GtData (pDloc, EntryOffset + 4, 2, TIFFLONG, (LPSTR)&pDe->deLength))
		return err;

	// return
	return SUCCESS;
}

//---------------------------------------------------------------------------
// Process TIFF entry
#define MAXVAL sizeof(TMP)
static RC PrTiffEntry (DLOC *pDloc, DWORD pos, DIRENTRY *pde, LPSAVEIMAGE I)
{
RC		err;
int		tsize, BytesToRead, maxitems, item;
DWORD	valpos;
LPSTR 	buf = TMP;
LPWORD	wbuf = (LPWORD) TMP;
LPDWORD dwbuf = (LPDWORD) TMP;

	if (err = GtTiffSizeof (pde->deType, &tsize)) return err;
	BytesToRead = tsize * pde->deLength;
	if (BytesToRead==0) return SUCCESS;
	maxitems = MAXVAL / tsize;
	maxitems = (pde->deLength < (DWORD) maxitems) ? (WORD)(pde->deLength) : maxitems;
	// careful here: we can't just use deVal to grab data out of, since
	// may already have been byte-reversed!
	if (BytesToRead <= 4)
		valpos = pos + 8;	// deVal starts on byte 8, wit de
	else
		valpos = pde->deVal;
	if (err = GtData (pDloc, valpos, maxitems, pde->deType, buf)) return err;

	switch ( pde->deTag ) {
		case TGIMAGEWIDTH:
			if (( pde->deType == TIFFSHORT ) || ( pde->deType == TIFFLONG ))
				tifX = *wbuf;
			else return -1;
			break;
		case TGIMAGELENGTH:
			if (( pde->deType == TIFFSHORT ) || ( pde->deType == TIFFLONG ))
				tifY = *wbuf;
			else return -1;
			break;
		case TGBITSPERSAMPLE:
			if ( pde->deType == TIFFSHORT ) {
				for (tifBitsPerPixel=0,item=0; item<maxitems; item++, wbuf++ )
					tifBitsPerPixel += *wbuf;
			} else if ( pde->deType == TIFFLONG ) {
				for (tifBitsPerPixel=0,item=0; item<maxitems; item++, wbuf++ )
					tifBitsPerPixel += *dwbuf;
			} else return -1;
			if ((tifBitsPerPixel != 8)  && (tifBitsPerPixel != 24) && (tifBitsPerPixel != 16) &&
				(tifBitsPerPixel != 32) && (tifBitsPerPixel != 48))
				return -1;
			break;

		case TGCOMPRESSION:
			if      ( pde->deType == TIFFSHORT ) tifCompr = *wbuf;
			else if ( pde->deType == TIFFLONG ) tifCompr = *dwbuf;
			else return -1;
			if (( tifCompr != CoNOCOMP ) && ( tifCompr != CoPACKBIT ) &&( tifCompr != CoLZW ))
				return -1;
			break;

		case TGPHOTOMETRICINTERPRETATION:
			if      ( pde->deType == TIFFSHORT ) tifPhoto = *wbuf;
			else if ( pde->deType == TIFFLONG )  tifPhoto = *dwbuf;
			else return -1;
			break;

		case TGSTRIPOFFSETS:
			tifOffsets = valpos;
			tifOffsetsSize = pde->deType;
			if (((tifOffsetsSize != TIFFLONG) && (tifOffsetsSize != TIFFSHORT)) ||
				(tifOffsets == 0))
				return -1;
			break;

		case TGSAMPLESPERPIXEL:
			if ( pde->deType != TIFFSHORT && pde->deType != TIFFLONG) return -1;
			tifSamplesPerPixel = *wbuf;
			if ((tifSamplesPerPixel != 1) && (tifSamplesPerPixel != 3) && (tifSamplesPerPixel != 4))
				return -1;
			break;

		case TGROWSPERSTRIP:
			if (( pde->deType == TIFFSHORT ) || ( pde->deType == TIFFLONG ))
				tifRowsPerStrip = *wbuf;
			else return -1;
			if ( tifRowsPerStrip == 0 )
				return -1;
			break;

		case TGPLANARCONFIGURATION:
			if ( pde->deType != TIFFSHORT && pde->deType != TIFFLONG) return -1;
			if ( *wbuf != 1 ) return -1;
			break;

		case TGGARYRESPONSECURVE:
			break;

		case TGCOLORMAP:
			for (item=0; item<256; item++, wbuf++)
				x_lookup[item] = (BYTE)(((*wbuf)*0.287 + (*(wbuf+256))*0.599 + (*(wbuf+512))*0.114+0.5) /256. );
			break;

		case TGPREDICTOR:
			if ( pde->deType != TIFFSHORT && pde->deType != TIFFLONG) return -1;
			if ( *wbuf != 2 && *wbuf != 1 ) return -1;
			if ( *wbuf == 2 ) tifPrediction = 1;
			break;

		case TGSAMPLEFORMAT:
			if ( pde->deType != TIFFSHORT && pde->deType != TIFFLONG) return -1;
			tifSampleFormat = *wbuf;
			break;

		case TGPROJINFO:
		{
			NEWINFO* NI = (NEWINFO*) buf;
			if (err = GtData (pDloc, *dwbuf, sizeof(NEWINFO), TIFFASCII, buf)) return err;
			// If logo OK then copy PrjInfo
			if ((NI->logo[0] == 'N') && (NI->logo[1] == 'I'))
				memcpy(I->lpAdd, NI, I->iAddSize);
		}
			break;
	}
	return SUCCESS;
}

//---------------------------------------------------------------------------
static BOOL	CheckTIFFFile( LPSAVEIMAGE I, HFILE hFile )
{
RC			err;
TIFFHDR		th;
DIRENTRY	de;
WORD		entries;
WORD		entry;
DWORD		location;
WORD		wtemp;

	// Set defaults;
	tifX = tifY = 0;
	tifRowsPerStrip = 0xFFFF;
	tifOffsets = 0;
	tifOffsetsSize = 0;
	tifBitsPerPixel = 1;
	tifSamplesPerPixel = 1;
	tifCompr = CoNOCOMP;
	tifPhoto = PhZEROBLACK;
	tifSampleFormat = SfUNSIGNED;
	tifPrediction = 0;

	Dloc.hFile = hFile;
	Dloc.dlWhere = INFILE;

	// read the first word, and determine the byte order
	Dloc.dlOrder = INTELTIFF;	/* arbitrary -- I'll change it below */
	if (err = GtData (&Dloc, 0, 1, TIFFSHORT, (LPSTR)&wtemp)) goto quit;
	Dloc.dlOrder = wtemp;

	// read the 8-byte header, and dump it
	if (err = GtTiffHdr (&Dloc, &th)) goto quit;

	location = th.thIfdOffset;
	Dloc.dlOrder = th.thByteOrder;

	// if ifd location is 0, quit
	if (location == 0) goto quit;

	// read the number of entries, and process it
	if (err = GtData (&Dloc, location, 1, TIFFSHORT, (LPSTR)&entries)) goto quit;
	if (entries == 0) goto quit;
	location += 2;

	// loop through the entries
	for (entry = 0; entry < entries; entry++) {

		// read the entry, and process it
		if (err = GtTiffEntry (&Dloc, location, &de)) goto quit;
		// "goto quit;" commented by Peter Oleynikov on 28 May 2002
//		if (err = PrTiffEntry (&Dloc, location, &de, I)) goto quit;
		err = PrTiffEntry (&Dloc, location, &de, I);

		// adjust the current location
		location += sizeof (DIRENTRY);

	} // end of entry loop

	// read the location of the next ifd
//	if (err = GtData(&Dloc, location, 1, TIFFLONG, (LPSTR)&dwtemp)) goto quit;

	// Check consistancy
	if ((tifX <= 0) || (tifY <= 0) || (tifOffsets == 0) ||
		(tifBitsPerPixel == 1) || (tifPhoto == 0xFFFFFFFF)) goto quit;
	if((tifBitsPerPixel == 16 ) && ( tifCompr != CoNOCOMP) && ( tifCompr != CoLZW)) goto quit;

	I->x = tifX;
	I->y = tifY;

	switch( tifBitsPerPixel )
	{
		case 16:
		case 48:
			I->_pix = PIX_WORD;
			I->ps = 2;
			break;
		default:
			I->_pix = PIX_BYTE;
			I->ps = 1;
			break;
	}

	if ( tifRowsPerStrip > tifY ) tifRowsPerStrip = tifY;
	if ( tifCompr == CoLZW ) I->strip = tifRowsPerStrip;
	else					 I->strip = 1;
	tifNStrips = (tifY + tifRowsPerStrip - 1) / tifRowsPerStrip;
	tifStrip = 0xFFFFFFFF;
	tifLineInStrip = 0xFFFFFFFF;
	tifLine = 0;
	tifRWptr = 0;
	if ( tifPhoto == PhZEROWHITE )
		for (entry=0; entry<256; entry++) x_lookup[entry] =(BYTE)(255-entry);
	return TRUE;

quit:
	return FALSE;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static	LPBYTE	LZWstring[4096];
static	WORD	LZWlength[4096];
static	WORD	MaxCode;
static	LPBYTE	EmptyBuf;
static	BYTE	LZWBuffer[16384*2];

static	BYTE	c_byte;
static	BYTE	c_bit;
static	BYTE	CodeLen;

#define	ClearCode	256
#define	EoiCode		257

static	BYTE	GetFileByte( void )
{
	if (filebuffptr>=BUFFSIZE) {
		_lread(hFile, filebuff, BUFFSIZE );
		filebuffptr = 0;
	}
	return filebuff[filebuffptr++];
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//#pragma optimize ("",off)
static	WORD GetNextCode( void )
{
WORD cc;

	if (c_bit==0) {	c_byte=GetFileByte(); c_bit = 8; }
_asm {
	mov		bl,	c_byte
	mov		cl,	CodeLen
	mov		ch,	c_bit
	xor		ax,	ax
xx:
	shl		bl,	1
	rcl		ax,	1
	dec		cl
	jz		ex
	dec		ch
	jnz		xx
	push	eax
	push	ecx
	push	ebx
}
	c_byte = GetFileByte();
_asm{
	pop		ebx
	pop		ecx
	pop		eax
	mov		ch,	8
	mov		bl,	c_byte
	jmp		xx
ex:
	dec		ch
	mov		cc, ax
	mov		c_bit,	ch
	mov		c_byte, bl
}
	return	cc;
}
//---------------------------------------------------------------------------
static	void InitializeTable( void )
{
int	i;

	EmptyBuf = LZWBuffer;		// !!!!!!
	for( i=0; i<256; i++) {
		LZWstring[i] = EmptyBuf;
		LZWlength[i] = 1;
		*LZWstring[i] = (BYTE)i;
		EmptyBuf++;
	}
	MaxCode = 258;
	CodeLen = 9;
}

//---------------------------------------------------------------------------
static	void ReadLZWstring( LPBYTE buff )
{
	int		cnt = 0;
	WORD	OldCode, Code;

	while ((Code = GetNextCode()) != EoiCode )
	{
 		if (Code>MaxCode)
			cnt = cnt;
		if (Code == ClearCode)
		{
			InitializeTable();
			Code = GetNextCode();
			if (Code == EoiCode) break;
			memcpy( buff, LZWstring[Code], LZWlength[Code]);
			buff += LZWlength[Code];
			cnt += LZWlength[Code];
			OldCode = Code;
		}
		else
		{
			if (Code < MaxCode)
			{
				memcpy( buff, LZWstring[Code], LZWlength[Code]);
				buff += LZWlength[Code];
				cnt += LZWlength[Code];

				LZWstring[MaxCode] = EmptyBuf;
				memcpy( LZWstring[MaxCode], LZWstring[OldCode], LZWlength[OldCode]);
				*(LZWstring[MaxCode] + LZWlength[OldCode]) = *LZWstring[Code];
				LZWlength[MaxCode] = LZWlength[OldCode] + 1;
				EmptyBuf += LZWlength[MaxCode];

				MaxCode ++;
				if (MaxCode > 2046) CodeLen = 12;
				else if (MaxCode > 1022) CodeLen = 11;
				else if (MaxCode > 510) CodeLen = 10;
				OldCode = Code;
			}
			else
			{
				LZWstring[MaxCode] = EmptyBuf;
				memcpy( LZWstring[MaxCode], LZWstring[OldCode], LZWlength[OldCode]);
				*(LZWstring[MaxCode] + LZWlength[OldCode]) = *LZWstring[OldCode];
				LZWlength[MaxCode] = LZWlength[OldCode] + 1;
				EmptyBuf += LZWlength[MaxCode];

				memcpy( buff, LZWstring[MaxCode], LZWlength[MaxCode]);
				buff += LZWlength[MaxCode];
				cnt += LZWlength[MaxCode];

				MaxCode ++;
				if (MaxCode > 2046) CodeLen = 12;
				else if (MaxCode > 1022) CodeLen = 11;
				else if (MaxCode > 510) CodeLen = 10;
				OldCode = Code;
			}
			// to be safe extract 256 from the actual length
			if (EmptyBuf-LZWBuffer > sizeof(LZWBuffer) - 256) return;
//			if (EmptyBuf-LZWBuffer > sizeof(LZWBuffer)) return;
		}
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void ReadPACKBITSstring( LPBYTE buff, int width )
{
BYTE	a;
int		sofar = 0, i;

	while(sofar<width) {
		a = GetFileByte();
		if ((a&0x80)!=0) {
			a = -a+1;
			if ((sofar+a) > width) return;
			memset(buff, GetFileByte(), a );
			buff+=a;
		} else {
			a+=1;
			if ((sofar+a) > width) return;
			for(i=0;i<a;i++) *buff++ = GetFileByte();
		}
		sofar += a;
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void SwapbMove( LPWORD dst, LPWORD src, UINT cnt )
{
	_asm {
	mov		esi,	[dst]
	mov		edi,	[src]
	mov		ecx,	[cnt]
aa:
	lods	word ptr [esi]
	xchg	al,		ah
	stos	word ptr [edi]
	loop	aa
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void WordXorMove( LPWORD dst, LPWORD src, UINT cnt, WORD mask)
{
	_asm {
	mov		esi,	[dst]
	mov		edi,	[src]
	mov		ecx,	[cnt]
	mov		bx,		[mask]
	shl		ebx,	16
	mov		bx,		[mask]
	shr		ecx,	1
aa:
	lods	dword ptr [esi]
	xor		eax,	ebx
	stos	dword ptr [edi]
	loop	aa
	test	[cnt],	1
	jz		aa1
	lods	word ptr [esi]
	xor		eax,	ebx
	stos	word ptr [edi]
aa1:
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Unpredict8( LPBYTE dst, LPBYTE src, UINT cnt )
{
UINT	i;
	*dst++ = *src++; cnt--;
	for(i=0;i<cnt;i++,dst++,src++) *dst = *(dst-1)+*src;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void Unpredict16( LPWORD dst, LPWORD src, UINT cnt )
{
UINT	i;
	*dst++ = *src++; cnt--;
	for(i=0;i<cnt;i++,dst++,src++) *dst = *(dst-1)+*src;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void RGB2Gray( LPBYTE dst, LPBYTE src, UINT cnt )
{
	_asm {
		mov		edi,	[dst]			;
		mov		esi,	[src]			;
		mov		ecx,	[cnt]			;
__10:
		movzx	edx,	byte ptr [esi]	; R
		inc		esi						;
		lods	word ptr [esi]			; B.G
		xor		ebx,	ebx				;
		xchg	ah,		bl				; edx:R, eax:G, ebx:B
		add		ebx,	eax				; ebx:B+G
		shl		eax,	1				;
		add		eax,	edx				;
		shl		eax,	1				;
		add		eax,	ebx				;
		shr		eax,	3				;
		adc		eax,	0				;
		stos	byte ptr [edi]			;
		dec		ecx						;
		jnz		__10					;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void RGBA2Gray( LPBYTE dst, LPBYTE src, UINT cnt )
{
	_asm {
		mov		edi,	[dst]			;
		mov		esi,	[src]			;
		mov		ecx,	[cnt]			;
__10:
		movzx	edx,	byte ptr [esi]	; R
		inc		esi						;
		lods	word ptr [esi]			; B.G
		xor		ebx,	ebx				;
		xchg	ah,		bl				; edx:R, eax:G, ebx:B
		add		ebx,	eax				; ebx:B+G
		shl		eax,	1				;
		add		eax,	edx				;
		shl		eax,	1				;
		add		eax,	ebx				;
		shr		eax,	3				;
		adc		eax,	0				;
		stos	byte ptr [edi]			;
		inc		esi						;
		dec		ecx						;
		jnz		__10					;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void RGB16Gray( LPBYTE dst, LPBYTE src, UINT cnt )
{
	_asm {
		mov		edi,	[dst]			;
		mov		esi,	[src]			;
		mov		ecx,	[cnt]			;
__10:
		movzx	edx,	word ptr [esi]	; R
		movzx	eax,	word ptr [esi+2]; G
		movzx	ebx,	word ptr [esi+4]; B
										; edx:R, eax:G, ebx:B
		add		ebx,	eax				; ebx:B+G
		shl		eax,	1				;
		add		eax,	edx				;
		shl		eax,	1				;
		add		eax,	ebx				;
		shr		eax,	3				;
		adc		eax,	0				;
		stos	word ptr [edi]			;
		add		esi,	6				;
		dec		ecx						;
		jnz		__10					;
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static BOOL	ReadTIFFLine( LPBYTE buff, size_t w, LPBYTE tmp, LPBYTE filbuf, BOOL bCrypt)
{
	int	tsize;

	if( tifLine >= tifY )
		return FALSE;	// Out of real size

	hFile = Dloc.hFile;
	// Out of current strip ?
	if (tifLineInStrip >= tifRowsPerStrip )
	{
		tifStrip++;
		GtTiffSizeof(tifOffsetsSize, &tsize);
		if ( GtData (&Dloc, tifOffsets+tifStrip*tsize, 1, tifOffsetsSize, (LPSTR)&tifRWptr))
			  return FALSE;
		tifLineInStrip = 0;
		if ( _llseek(Dloc.hFile, tifRWptr, FILE_BEGIN) == HFILE_ERROR ) return FALSE;
		filebuffptr = BUFFSIZE+1;
		CodeLen = 9;
		c_bit = 0;
	}
	switch( tifCompr )
	{
		case CoNOCOMP:
			if ( tifBitsPerPixel == 8 )	if ( _lread( Dloc.hFile, buff, 1*w ) != w*1 ) return FALSE;
			if ( tifBitsPerPixel == 16)	if ( _lread( Dloc.hFile, buff, 2*w ) != w*2 ) return FALSE;
			if ( tifBitsPerPixel == 24 )if ( _lread( Dloc.hFile, tmp,  3*w ) != w*3 ) return FALSE;
			if ( tifBitsPerPixel == 32 )if ( _lread( Dloc.hFile, tmp,  4*w ) != w*4 ) return FALSE;
			if ( tifBitsPerPixel == 48 )if ( _lread( Dloc.hFile, tmp,  6*w ) != w*6 ) return FALSE;
			break;
		case CoPACKBIT:
			if ( tifBitsPerPixel == 8 )	 ReadPACKBITSstring( buff, w  );
			if ( tifBitsPerPixel == 24 ) ReadPACKBITSstring( tmp,  w*3 );
			if ( tifBitsPerPixel == 32 ) ReadPACKBITSstring( tmp,  w*4 );
			if ( tifBitsPerPixel == 48 ) ReadPACKBITSstring( tmp,  w*6 );
			break;
		case CoLZW:
			{
				if( 0 == tifLineInStrip )
					ReadLZWstring( tmp );
				LPBYTE hh = tmp;
				if ( tifBitsPerPixel == 8 )	 { hh += tifLineInStrip * w;     memcpy(buff, hh, w);   }
				else if ( tifBitsPerPixel == 16 ) { hh += tifLineInStrip * w * 2; memcpy(buff, hh, w*2); }
				else if ( tifBitsPerPixel == 24 ) { hh += tifLineInStrip * w * 3; tmp = hh; }
				else if ( tifBitsPerPixel == 32 ) { hh += tifLineInStrip * w * 4; tmp = hh; }
			}
			break;
	}
	if (Dloc.dlOrder == MOTOROLATIFF) switch( tifBitsPerPixel )
	{
		case 16: SwapbMove( (LPWORD)buff, (LPWORD)buff, w     ); break;
		case 24: SwapbMove( (LPWORD)tmp,  (LPWORD)tmp,  w*3/2 ); break;
		case 32: SwapbMove( (LPWORD)tmp,  (LPWORD)tmp,  w*4/2 ); break;
		case 48: SwapbMove( (LPWORD)tmp,  (LPWORD)tmp,  w*6/2 ); break;
	}
	switch ( tifPhoto )
	{
		case PhZEROWHITE:
		case PhPALETTE:
			if ( tifBitsPerPixel == 8 )	XlatMove( buff, buff, x_lookup, w );
			break;
		case PhRGB:
			if ( tifBitsPerPixel == 24 ) RGB2Gray  ( (LPBYTE)buff, (LPBYTE)tmp, w );
			if ( tifBitsPerPixel == 32 ) RGBA2Gray ( (LPBYTE)buff, (LPBYTE)tmp, w );
			if ( tifBitsPerPixel == 48 ) RGB16Gray ( (LPBYTE)buff, (LPBYTE)tmp, w );
			break;
	}
/*	// REVISITED by Peter on 25 May 2004
	if( 16 == tifBitsPerPixel )
	{
		// if we do NOT have any "negative" numbers
		// (which are >= 32768 in the UNSIGNED notation)
		// then do NOT execute WordXorMove
		WORD wMax = *std::max_element<LPWORD>((LPWORD)buff, (LPWORD)(buff+size));
		if( wMax >= 32768 )
		{
			if (tifSampleFormat == SfTWOSCOMPL)
				WordXorMove( (LPWORD)buff, (LPWORD)buff, size, 0x8000);
		}
	}
*/
//	if (tifBitsPerPixel == 16) {
//		if (tifSampleFormat == SfTWOSCOMPL) WordXorMove( (LPWORD)buff, (LPWORD)buff, w, 0x8000);
//	}
	if ( tifPrediction )
	{
		if (tifBitsPerPixel == 16) 
			Unpredict16 ( (LPWORD)buff, (LPWORD)buff, w );
		else if (tifBitsPerPixel ==  8) 
			Unpredict8  ( (LPBYTE)buff, (LPBYTE)buff, w );

	}
	tifLineInStrip++;
	tifLine++;
	if (bCrypt) decrypt( buff, buff, w*tifBitsPerPixel/8 );
	return TRUE;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static LPBYTE	tifftmp;

static BOOL	fioCheckTIFF( LPSAVEIMAGE I )
{
	CheckTIFFFile( I, hFile );
	if (!fioFixImageMemory( I )) return FALSE;
#ifdef USE_XB_LENGTH
	if ((tifftmp = (LPBYTE)calloc(1, I->y*I->xb*6 )) == NULL) return FALSE;
#else
	if( NULL == (tifftmp = (LPBYTE)calloc(1, I->y*I->stride*6 )) )
		return FALSE;
#endif
	filebuffptr = BUFFSIZE+1;
	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL fioReadTIFF( LPSAVEIMAGE pImg )
{
	int		y, stride;
	LPBYTE	ptr;

#ifdef USE_XB_LENGTH
	stride = pImg->xb * pImg->ps;
#else
	stride = pImg->stride;
#endif
	ptr = pImg->lpData;
	for(y = 0; y < pImg->y; y++)
	{
		try
		{
//			ReadTIFFLine( pImg->lpData + pImg->xb*y*pImg->ps, pImg->x, tifftmp, NULL, FALSE);
			ReadTIFFLine( ptr, pImg->x, tifftmp, NULL, FALSE);
		}
		catch (std::exception& e)
		{
			Trace(_FMT(_T("fioReadTIFF() -> y=%d '%s'"), y, e.what()));
		}
		ptr += stride;
		fioReportPercent( y, pImg->y - 1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	free(tifftmp);
	return TRUE;
}

//---------------------------------------------------------------------------
static BOOL fioReadTIFFCrypt ( LPSAVEIMAGE I )
{
	int		y;

	initcypher(TRUE);
	for(y=0; y<I->y; y++)
	{
#ifdef USE_XB_LENGTH
		ReadTIFFLine( I->lpData+I->xb*y*I->ps, I->x, tifftmp, NULL, TRUE);
#else
		ReadTIFFLine(I->lpData + I->stride*y, I->x, tifftmp, NULL, TRUE);
#endif
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	free(tifftmp);
	return TRUE;
}

/****************************************************************************/
// added by Peter on 20 Aug 2002 for DITABIS file support
static char token[256];
static int token_pos;
static int token_len;
static char ch;
static int DITABIS_Header = 0;
static int DITABIS_PS = 0;

typedef struct
{
	char name[32];
	short id;

} TOKEN;

TOKEN tokens[] = {
	{"XPIXEL", DITABIS_XPIXEL},
	{"YPIXEL", DITABIS_YPIXEL},
	{"BYTE", DITABIS_BYTEPP},
	{"XRESOLUTION", DITABIS_XRESOL},
	{"YRESOLUTION", DITABIS_YRESOL},
	{"HEADER", DITABIS_HEADER},
	{"COMMENT", DITABIS_COMMENT},
	{"", 0},
};

static void ScanWord()
{
	token_pos = 1;
	token[0] = ch;
	while( 1 )
	{
		_lread(hFile, &ch, 1);
		switch(ch)
		{
		case '\0': break;
		case EOF: break;
		}
		if( (0 != isalpha(ch)) || ('-' == ch) )
		{
//			if( TRUE == bBreak )
//				break;
			token[token_pos++] = ch;
		}
		else
		{
			break;
		}
	}
	token[token_pos] = '\0';
	token_len = token_pos;
}

static void ScanNumber()
{
	token_pos = 1;
	token[0] = ch;
	while( 1 )
	{
		_lread(hFile, &ch, 1);
		switch(ch)
		{
		case '\0': break;
		case EOF: break;
		}
		if( (0 != isdigit(ch)) || ('.' == ch) )
		{
			token[token_pos++] = ch;
		}
		else
		{
			break;
		}
	}
	token[token_pos] = '\0';
	token_len = token_pos;
	token_pos = 0;
}

static int GetToken()
{
	BOOL bBreak = FALSE;
	while( 1 )
	{
		_lread(hFile, &ch, 1);
		switch(ch)
		{
		case ' ': bBreak = TRUE; continue;
		case '\0': continue;
		case EOF: continue;
		}

		if( 0 != isalpha(ch) )
		{
			if( TRUE == bBreak )
				return IDENTIFIER;
			ScanWord();
			return IDENTIFIER;
		}
		else if( 0 != isdigit(ch) )
		{
			ScanNumber();
			return NUMBER;
		}
		else
		{
			break;
		}
	}

	return ch;
}

static _inline void SkipTillSymbol(char s)
{
	do {
		_lread(hFile, &ch, 1);
	} while( (s != ch) && (0 != ch) );		// skip till line end
}

static _inline void SkipLineEnd()
{
	do {
		_lread(hFile, &ch, 1);
	} while( ('\n' != ch) && (0 != ch) );		// skip till line end
}

static int _nRealDitabisBPP = 2;

static BOOL fioCheckIP( LPSAVEIMAGE I )
{
	int nToken, i;
	token_len = 0;
	token_pos = 0;
	token[0] = '\0';

	_llseek(hFile, 0, FILE_BEGIN);
	I->x = I->y = I->ps = 0;

	ch = 1;
	while( '\0' != ch )
	{
		nToken = GetToken();
		if( IDENTIFIER == nToken )
		{
			i = 0;
			while(tokens[i].id != 0)
			{
				if( 0 == strcmp(tokens[i].name, token) )
					break;
				i++;
			}
			nToken = (tokens[i].id == 0) ? IDENTIFIER : tokens[i].id;
			switch(nToken)
			{
			case IDENTIFIER:
				SkipLineEnd();		// we do not know what to do with it
				break;
			case DITABIS_XPIXEL:
				SkipTillSymbol('=');
				if( NUMBER == GetToken() )
				{
					I->y = atoi(token);
				}
				else
				{
					return FALSE;
				}
				break;
			case DITABIS_YPIXEL:
				SkipTillSymbol('=');
				if( NUMBER == GetToken() )
				{
					I->x = atoi(token);
				}
				else
				{
					return FALSE;
				}
				break;
			case DITABIS_HEADER:
				SkipTillSymbol('=');
				if( NUMBER == GetToken() )
				{
					DITABIS_Header = atoi(token);
				}
				else
				{
					return FALSE;
				}
				break;
			case DITABIS_COMMENT:
				SkipTillSymbol('\0');
				break;
			case DITABIS_BYTEPP:
				SkipTillSymbol('=');
				if( NUMBER == GetToken() )
				{
					_nRealDitabisBPP = DITABIS_PS = atoi(token);
				}
				else
				{
					return FALSE;
				}
				break;
			default: break;
			}
		}
		else if( '\n' != ch )
		{
			SkipLineEnd();
		}
	}
	I->_pix = PIX_WORD;
	I->ps = 2;
	I->strip = 1;
	if( I->x == 0 || I->y == 0 || DITABIS_PS == 0 || DITABIS_Header == 0 )
		return FALSE;

	if (!fioFixImageMemory( I ))
		return FALSE;

	return TRUE;
}

static BOOL fioReadDITABIS( LPSAVEIMAGE I )
{
	int		off, stride, diff, x, y, extra;
	BYTE *buffer;
	BYTE *pData;
	double dScale;

	buffer = new BYTE[4*I->x];
	diff = DITABIS_PS - I->ps;
	off = 0;
	stride = I->x * I->ps;
#ifdef USE_XB_LENGTH
	extra = (I->xb * I->ps) - stride;
#else
	extra = I->stride - stride;
#endif
	// scan for min max
	_llseek(hFile, DITABIS_Header, FILE_BEGIN);
	DWORD _min, _max, val;
	_min = (DWORD)-1L;
	_max = 0;
	if( 2 != DITABIS_PS )
	{
		for(y = 0; y < I->y; y++)
		{
			pData = buffer;
			_lread( hFile, buffer, DITABIS_PS*I->x );
			for(x = 0; x < I->x; x++)
			{
				switch( DITABIS_PS )
				{
				case 2:
					val = *(WORD*)pData;
					_min = __min(_min, val);
					_max = __max(_max, val);
					pData += 2;
					break;
				case 3:
					val = (*(DWORD*)pData) & 0x00FFFFFF;
					_min = __min(_min, val);
					_max = __max(_max, val);
					pData += 3;
					break;
				case 4:
					val = (*(DWORD*)pData);
					_min = __min(_min, val);
					_max = __max(_max, val);
					pData += 4;
					break;
				default: return FALSE;
				}
			}
			fioReportPercent( y, I->y-1, TRUE );
		}
		dScale = 65535. / double(_max - _min);
	}
	// read and scale the data
	_llseek(hFile, DITABIS_Header, FILE_BEGIN);
	for(y = 0; y < I->y; y++)
	{
		pData = buffer;
		_lread( hFile, buffer, DITABIS_PS*I->x );
		if( DITABIS_PS == 2 )
		{
			memcpy( I->lpData + off, pData, stride );
			off += stride + extra;
		}
		else if( DITABIS_PS == 3 )
		{
			for(x = 0; x < I->x; x++)
			{
				val = (*(DWORD*)pData) & 0x00FFFFFF;
				*(WORD*)(I->lpData+off) = (val-_min) * dScale;
				pData += 3;
				off += 2;
			}
			off += extra;
		}
		else if( DITABIS_PS == 4 )
		{
			for(x = 0; x < I->x; x++)
			{
				val = *(DWORD*)pData;
				*(WORD*)(I->lpData+off) = (val-_min) * dScale;
				pData += 4;
				off += 2;
			}
			off += extra;
		}
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	delete[] buffer;

	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static BOOL fioCheckRAWData( LPSAVEIMAGE I )
{
	// do not need to check
	return TRUE;
}

static BOOL fioReadRAWData( LPSAVEIMAGE I )
{
#ifdef _MFC_VER
	int y;

	RawDlg dlg;

	if( IDOK == dlg.DoModal() )
	{
		// read and scale the data
/*		_llseek(hFile, 0, FILE_BEGIN);
		for(y = 0; y < I->y; y++)
		{
			pData = buffer;
			_lread( hFile, buffer, DITABIS_PS*I->x );
			fioReportPercent( y, I->y-1, TRUE );
		}
*/		fioReportPercent( 0, 0, TRUE );
	}
#endif // _MFC_VER

	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/* Added by Peter on 8 Feb 2004 */

typedef struct DM2_tag_
{
	WORD tagType;
	DWORD length;

} DM2_tag;

typedef struct DM3_header_
{
	DWORD version;
	DWORD length;
	DWORD order;
	DWORD unknown;
	WORD  numtags;

} DM3_header;

typedef struct DM3_tag_
{
	char tag;
	WORD length;

} DM3_tag;

typedef struct DM3_GROUP_
{
	int data_type;
	int length;
	int multiple;
	LPVOID pData;

} DM3_GROUP;

typedef std::vector<DM3_GROUP>				groupVec;
typedef std::vector<DM3_GROUP>::iterator	groupVecIt;

typedef struct DM3_MAP_
{
	char tag;
	groupVec groups;
	std::string name;
	struct DM3_MAP_ *pNext;

} DM3_MAP;

typedef std::map<std::string, DM3_MAP*>				DM3map;
typedef std::map<std::string, DM3_MAP*>::iterator	DM3mapIt;
// static DM3map	s_mapDM3tags;
static DM3map &GetDm3TagMap()
{
	static DM3map s_mapDM3tags;
	return s_mapDM3tags;
}

typedef std::vector<DM3_MAP*> TagVec;
typedef TagVec::iterator TagVecIt;

static TagVec &GetDm3TagVec()
{
	static TagVec s_vTags;
	return s_vTags;
}

enum BYTE_ORDER
{
	BIG_ENDIAN = 1,			// MAC
	LITTLE_ENDIAN = 2,		// PC

} ;

static BYTE_ORDER s_nOrderDM3 = BIG_ENDIAN;
static DWORD s_nDM3ver = 0;
static char s_sDM3name[256];

typedef struct DWORD__
{
	union DWORD_DATA
	{
		DWORD dw;
		BYTE bt[4];
	} dword;

} DWORD_;

typedef struct WORD__
{
	union WORD_DATA
	{
		WORD w;
		BYTE bt[2];
	} word;

} WORD_;

__forceinline void SwapDWORD(DWORD &dw)
{
	DWORD_ *pDw = (DWORD_*)&dw;
	std::swap(pDw->dword.bt[0], pDw->dword.bt[3]);
	std::swap(pDw->dword.bt[1], pDw->dword.bt[2]);
}

__forceinline void SwapLONG(long &dw)
{
	DWORD_ *pDw = (DWORD_*)&dw;
	std::swap(pDw->dword.bt[0], pDw->dword.bt[3]);
	std::swap(pDw->dword.bt[1], pDw->dword.bt[2]);
}

__forceinline void SwapINT(int &dw)
{
	DWORD_ *pDw = (DWORD_*)&dw;
	std::swap(pDw->dword.bt[0], pDw->dword.bt[3]);
	std::swap(pDw->dword.bt[1], pDw->dword.bt[2]);
}

__forceinline void SwapFLOAT(float &dw)
{
	DWORD_ *pDw = (DWORD_*)&dw;
	std::swap(pDw->dword.bt[0], pDw->dword.bt[3]);
	std::swap(pDw->dword.bt[1], pDw->dword.bt[2]);
}

__forceinline void SwapWORD(WORD &dw)
{
	WORD_ *pDw = (WORD_*)&dw;
	std::swap(pDw->word.bt[0], pDw->word.bt[1]);
}

__forceinline void SwapSHORT(short &dw)
{
	WORD_ *pDw = (WORD_*)&dw;
	std::swap(pDw->word.bt[0], pDw->word.bt[1]);
}

static int GetDataTypeSize(int nDataType)
{
	int nSize = 0;
	switch(nDataType)
	{
	case 2:
	case 4: nSize = 2; break;
	case 3:
	case 5:
	case 6: nSize = 4; break;
	case 7: nSize = 8; break;
	case 8:
	case 9:
	case 10: nSize = 1; break;
	}
	return nSize;
}

BOOL ReadDM3tag( std::string sGroupName, DM3_MAP *pTag = NULL );

static BOOL ReadGroupData( DM3_MAP *pTag, bool b14_type )
{
	groupVec groups;
	DM3_GROUP group;
	DWORD data_type, null, ngroup;

	group.multiple = 1;
	_lread(hFile, &null, 4);
	if( 0 != null )
	{
		MessageBox(NULL, "Error tag '15'", "ERROR", MB_OK);
		return FALSE;
	}
	_lread(hFile, &ngroup, 4);
	SwapDWORD(ngroup);
	for(int j = 0; j < ngroup; j++)
	{
		_lread(hFile, &null, 4);
		if( 0 != null )
		{
			MessageBox(NULL, "Error tag '15'", "ERROR", MB_OK);
			return FALSE;
		}
		_lread(hFile, &data_type, 4);
		SwapDWORD(data_type);
		// create a group member
		group.data_type = data_type;
		group.length = GetDataTypeSize(data_type);
		group.pData = new BYTE[group.length];
		// temp groups
		groups.push_back(group);
	}
	groupVecIt it;
	if( true == b14_type )
	{
		LPBYTE ptr;
		// total # of groups to read
		_lread(hFile, &ngroup, 4);
		SwapDWORD(ngroup);
		group.multiple = ngroup;
		// resize each group according to 'ngroup', so we'll have an array
		for(it = groups.begin(); it != groups.end(); ++it)
		{
			delete it->pData;
			it->pData = new BYTE[it->length*ngroup];
		}
		// read them now
		for(int j = 0; j < ngroup; j++)
		{
			for(it = groups.begin(); it != groups.end(); ++it)
			{
				ptr = (LPBYTE)it->pData;
				_lread(hFile, ptr + (it->length*j), it->length);
			}
		}
	}
	else
	{
		for(it = groups.begin(); it != groups.end(); ++it)
		{
			_lread(hFile, it->pData, it->length);
		}
	}
	for(it = groups.begin(); it != groups.end(); ++it)
	{
		pTag->groups.push_back(*it);
	}
	return TRUE;
}

static BOOL ReadDM3tag15( DM3_MAP *pTag )
{
	DM3_GROUP group;
	char unkn1[4];
	DWORD nnum, data_type, ngroup;
	DWORD nSize;

	group.multiple = 1;
	_lread(hFile, &unkn1, 4);
	if( 0 != strncmp(unkn1, "%%%%", 4) )
	{
		MessageBox(NULL, "Couldn't find '%%%%' tag", "ERROR", MB_OK);
		return FALSE;
	}
	// nnum
	_lread(hFile, &nnum, 4);
	SwapDWORD(nnum);
	// read nnum I4 numbers
	// data type first
	_lread(hFile, &data_type, 4);
	SwapDWORD(data_type);
	if( 0x14 == data_type )
	{
		// data type
		_lread(hFile, &data_type, 4);
		SwapDWORD(data_type);
		if( 0xF == data_type )
		{
			ReadGroupData( pTag, true );
		}
		else
		{
			// # of elements in the group
			_lread(hFile, &ngroup, 4);
			SwapDWORD(ngroup);
			// read them
			nSize = GetDataTypeSize(data_type);
			group.multiple = ngroup;
			nSize *= ngroup;
			group.data_type = data_type;
			group.length = nSize;
			group.pData = NULL;
			if( nSize > 0 )
			{
				group.pData = new BYTE[nSize];
				// the array size is too big
				if( NULL == group.pData )
				{
					// skip the data
					// TODO: set the data pointer, if larger than 4Gb?
					ngroup = _ltell(hFile);
					ngroup = _lseek(hFile, nSize, SEEK_CUR);
				}
				else
				{
					_lread(hFile, group.pData, nSize);
				}
			}
			pTag->groups.push_back(group);
		}
	}
	else if( 0xF == data_type )
	{
		ReadGroupData( pTag, false );
	}
	else
	{
		nSize = GetDataTypeSize(data_type);
		group.data_type = data_type;
		group.length = nSize;
		group.pData = new BYTE[nSize];
		_lread(hFile, group.pData, nSize);
		pTag->groups.push_back(group);
	}
	return TRUE;
}

static BOOL ReadDM3tag14( DM3_MAP *pTag )
{
	DWORD unkn;
	WORD nTags;

	_lread(hFile, &unkn, 4);
	_lread(hFile, &nTags, 2);
	SwapWORD(nTags);
	for(int i = 0; i < nTags; i++)
	{
		if( FALSE == ReadDM3tag( pTag->name, pTag ) )
		{
			return FALSE;
		}
	}
	return TRUE;
}

static BOOL ReadDM3tag( std::string sGroupName, DM3_MAP *pTag )
{
	DM3_tag tag;
	WORD length;
	std::vector<char> name;

	if( NULL == pTag )
	{
		pTag = new DM3_MAP;
		pTag->pNext = NULL;
	}
	else
	{
		while( pTag->pNext )
			pTag = pTag->pNext;
		pTag->pNext = new DM3_MAP;
		pTag = pTag->pNext;
		pTag->pNext = NULL;
	}
	_lread(hFile, &tag, sizeof(tag));
	length = tag.length;
	SwapWORD(length);
	pTag->tag = tag.tag;
	if( 0 == tag.tag )
	{
		// EOF
		return FALSE;
	}
	if( length > 0 )
	{
		// get the name of the tag
		name.resize(length+1);
		memset(&*name.begin(), 0, length+1);
		_lread(hFile, &*name.begin(), length);
		strcpy(s_sDM3name, &*name.begin());
		if( sGroupName.empty() )
		{
			pTag->name = s_sDM3name;
		}
		else
		{
			pTag->name = sGroupName;
			pTag->name += ".";
			pTag->name += s_sDM3name;
		}
// 		::OutputDebugString("Reading tag: '");
// 		::OutputDebugString(pTag->name.c_str());
// 		::OutputDebugString("'\n");
// 		if( NULL != strstr(pTag->name.c_str(), "ImageData.Calibrations.Dimension") )
// 		{
// 			::OutputDebugString("'\n");
// 		}
	}
	// "directory"
	if( 0x14 == tag.tag )
	{
		ReadDM3tag14( pTag );
	}
	else if ( 0x15 == tag.tag )
	{
		ReadDM3tag15( pTag );
	}
	if( !pTag->name.empty() )
	{
		DM3mapIt mit = /*s_mapDM3tags*/GetDm3TagMap().find(pTag->name);
		// work only with very first image
//		if( mit == s_mapDM3tags.end() )
		{
			/*::s_mapDM3tags*/GetDm3TagMap()[pTag->name] = pTag;
		}
	}

	return TRUE;
}

template <typename _Tx, typename _Ty>
bool CopyImageData(_Tx *pDst, _Ty *pSrc, LPSAVEIMAGE I, DWORD nMaxDst,
				   bool bRescale, bool bSigned)
{
#ifdef USE_XB_LENGTH
	int nStride = I->xb*I->ps;
#else
	int nStride = I->stride;
#endif
//	int nStride = R4(I->x*I->ps);
//	int nSize = I->x*I->y;
	int x, y;
	int off = 0;
	if( true == bRescale )
	{
		_Ty nMin, nMax;
		// find min/max elements in each row
		nMin = nMax = pSrc[0];
		LPBYTE ptr = (LPBYTE)pSrc;
		for(y = 0; y < I->y; y++)
		{
			nMin = __min(nMin, *std::min_element((_Ty*)ptr, (_Ty*)(ptr+nStride)));
			nMax = __max(nMax, *std::max_element((_Ty*)ptr, (_Ty*)(ptr+nStride)));
			ptr += nStride;
		}
		double dScale = double(nMaxDst) / double(nMax - nMin);
		for(y = 0; y < I->y; y++)
		{
			for(x = 0; x < I->x; x++, off++)
			{
				pDst[x] = (_Tx)(dScale*(pSrc[off]-nMin) + 0.5);
			}
			pDst = (_Tx*)(((LPBYTE)pDst) + nStride);
		}
	}
	else
	{
		if( true == bSigned )
		{
			DWORD nMin = -int((nMaxDst >> 1) + 1);
			for(y = 0; y < I->y; y++)
			{
				for(x = 0; x < I->x; x++, off++)
				{
					pDst[x] = (_Tx)((DWORD)(pSrc[off]) - nMin);
				}
				pDst = (_Tx*)(((LPBYTE)pDst) + nStride);
			}
		}
		else	// not signed - just copy
		{
			int nLength = I->x*I->ps;
			for(y = 0; y < I->y; y++)
			{
				memcpy(pDst, pSrc, nLength);
				pDst = (_Tx*)(((LPBYTE)pDst) + nStride);
				pSrc += I->x;
			}
		}
	}
	return true;
}

static BOOL fioCheckDM2Data( LPSAVEIMAGE I )
{
	DM2_tag tag;
	WORD width = 0, height = 0, bpp = 0, data_type = 0;
	int nSize = sizeof(tag);
	int y;
	LPVOID ptr;
	bool bSeek;
	BOOL bRet = FALSE;

	_llseek(hFile, 0, FILE_BEGIN);
	while( nSize == _lread(hFile, &tag, nSize) )
	{
		bSeek = true;
		SwapWORD(tag.tagType);
		SwapDWORD(tag.length);
		// look up the tag
		switch( tag.tagType )
		{
		case 0xFFFF:		// Image data
			_lread(hFile, &width, sizeof(width));
			_lread(hFile, &height, sizeof(height));
			_lread(hFile, &bpp, sizeof(bpp));
			_lread(hFile, &data_type, sizeof(data_type));
			SwapWORD(width);
			SwapWORD(height);
			SwapWORD(bpp);
			SwapWORD(data_type);
			if( (0 == width) || (0 == height) || (0 == bpp) || (0 == data_type) )
			{
				return FALSE;
			}
			I->x = width;
			I->y = height;
			I->ps = bpp;
			I->strip = 1;
			switch( data_type )
			{
			case 1:			// 2 byte integer signed
				I->_pix = PIX_SHORT;
				if( I->ps != 2 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for SHORT(=2).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 2:			// 4 byte real (IEEE 754)
				I->_pix = PIX_FLOAT;
				if( I->ps != 4 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for FLOAT(=4).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 3:
				break;
			case 4:
				break;
			case 5:
				break;
			case 6:			// 1 byte integer unsigned ("byte")
				I->_pix = PIX_BYTE;
				if( I->ps != 1 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for BYTE(=1).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 7:			// 4 byte integer signed ("long")
				I->_pix = PIX_LONG;
				if( I->ps != 4 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for LONG(=4).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 8:
				break;
			case 9:			// 1 byte integer signed ("char")
				I->_pix = PIX_CHAR;
				if( I->ps != 1 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for CHAR(=1).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 10:		// 2 byte unsigned integer
				I->_pix = PIX_WORD;
				if( I->ps != 2 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for WORD(=2).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 11:		// 4 byte integer unsigned
				I->_pix = PIX_DWORD;
				if( I->ps != 4 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for DWORD(=4).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 12:		// 8 byte real (double)
				I->_pix = PIX_DOUBLE;
				if( I->ps != 8 )
				{
					sprintf(TMP, "Wrong number of Bytes-Per-Pixel(%d) for DOUBLE(=8).\n", I->ps);
					::OutputDebugString(TMP);
					return FALSE;
				}
				break;
			case 13:
				break;
			case 14:
				break;
			default:
				return FALSE;
			}
			if( (0 == I->x) || (0 == I->y) )
				return FALSE;
			if (!fioFixImageMemory( I ))
				return FALSE;
			ptr = I->lpData;
			for(y = 0; y < I->y; y++)
			{
				_lread(hFile, ptr, I->x * I->ps);
				if( 2 == I->ps )
				{
					tswab((LPSTR)ptr, (LPSTR)ptr, I->x * I->ps);
				}
				else if( 4 == I->ps )
				{
					swaw((LPSTR)ptr, (LPSTR)ptr, I->x * I->ps);
				}
				ptr = ((LPBYTE)ptr) + /*R4*/(I->x * I->ps);
			}
			// do net seek, because we read the data
			bSeek = false;
			bRet = TRUE;
			break;
		default:
			sprintf(TMP, "Unrecognized GATAN DM2 tag 0x%04X.\n", tag.tagType);
			::OutputDebugString(TMP);
		}
		if( true == bSeek )
		{
			if( HFILE_ERROR == _llseek(hFile, tag.length, FILE_CURRENT) )
			{
				sprintf(TMP, "Failed to seek to the position for the GATAN DM2 tag 0x%04X.\n", tag.tagType);
				::OutputDebugString(TMP);
				return FALSE;
			}
		}
	}
	// read the data
	return bRet;
}

static BOOL fioReadDM2Data( LPSAVEIMAGE I )
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

static BOOL fioCheckDM3Data( LPSAVEIMAGE I )
{
	DM3_header header;
	int i;
	WORD nTags;

	_llseek(hFile, 0, FILE_BEGIN);
	_lread(hFile, &header, sizeof(header));
	switch( header.version  )
	{
	case 3:
	case 0x03000000:
		s_nDM3ver = 3;
		break;
	default:
		MessageBox(NULL, "Unrecognized GATAN file format version", "ERROR", MB_OK);
		return FALSE;
	}
	// set the order
	if( 0 == header.order )
	{
		s_nOrderDM3 = BIG_ENDIAN;
	}
	else if( 0x01000000 == header.order )
	{
		s_nOrderDM3 = LITTLE_ENDIAN;
	}
	I->x = I->y = I->ps = 0;
	if( BIG_ENDIAN == s_nOrderDM3 )
	{
	}
	else if( LITTLE_ENDIAN == s_nOrderDM3 )
	{
		nTags = header.numtags;
		SwapWORD(nTags);
	}
	// read all tags simultaneously
	for(i = 0; i < nTags; i++)
	{
		if( FALSE == ReadDM3tag( "" ) )
			return FALSE;
	}

	DM3mapIt dim = /*::s_mapDM3tags*/GetDm3TagMap().find("ImageData.Dimensions");
	DM3mapIt data = /*::s_mapDM3tags*/GetDm3TagMap().find("ImageData.Data");
	DM3mapIt pix = /*::s_mapDM3tags*/GetDm3TagMap().find("ImageData.PixelDepth");
	DM3mapIt data_type = GetDm3TagMap().find("ImageData.DataType");

	DM3_MAP *pTag = dim->second->pNext;
	char tmp[256];
	if( pTag )
	{
		if( pTag->groups.size() > 0 )
			I->x = *(DWORD*)pTag->groups[0].pData;
		sprintf(tmp, "x = %d\n", *(int*)pTag->groups[0].pData);
		::OutputDebugString(tmp);
		if( pTag->pNext )
		{
			pTag = pTag->pNext;
				I->y = *(DWORD*)pTag->groups[0].pData;
			if( pTag->groups.size() > 0 )
			sprintf(tmp, "y = %d\n", *(int*)pTag->groups[0].pData);
			::OutputDebugString(tmp);
		}
	}
	
	// the pixel depth
	pTag = pix->second;
	if( (NULL == pTag) || (pTag->groups.empty()) )
	{
		MessageBox(NULL, "ImageData.PixelDepth tag is empty", "ERROR", MB_OK);
		return FALSE;
	}
	I->ps = *(int*)pTag->groups[0].pData;

	// default pixel type
	I->_pix = PIX_BYTE;
	// check the data type
	pTag = data_type->second;
	if( (NULL == pTag) || (pTag->groups.empty()) )
	{
		MessageBox(NULL, "ImageData.DataType tag is empty", "ERROR", MB_OK);
		return FALSE;
	}
	int nDataType = *(int*)pTag->groups[0].pData;
	switch( nDataType )
	{
	case 1: //	SIGNED_INT16_DATA
		I->_pix = PIX_SHORT; I->ps = 2; break;
	case 10: //	UNSIGNED_INT16_DATA 
		I->_pix = PIX_WORD; I->ps = 2; break;
		
	case 2: // REAL4_DATA
		I->_pix = PIX_FLOAT; I->ps = 4; break;
		
	case 9:	// or SIGNED_INT8_DATA
		I->_pix = PIX_CHAR; I->ps = 1; break;
	case 6: // UNSIGNED_INT8_DATA
		I->_pix = PIX_BYTE; I->ps = 1; break;
		
	case 7: // SIGNED_INT32_DATA
		I->_pix = PIX_LONG; I->ps = 4; break;
	case 11: // UNSIGNED_INT32_DATA
		I->_pix = PIX_DWORD; I->ps = 4; break;

	case 12: // REAL8_DATA
		I->_pix = PIX_DOUBLE; I->ps = 8; break;
/*	
	case 8: // RGB_DATA
		I->_pix = PIX_RGB;
		break;
		
	case 14: // BINARY_DATA
		I->_pix = PIX_BITMAP;
		break;
	case 23: // RGBA_UINT8_3_DATA
		// NB it is uncertain if this data type corresponds exactly to ImageJ's ARGB
		// A definitely comes first but the RGB values could be scrambled
		// (since they were all equal on my test image)
		I->_pix = PIX_ARGB;
		break;
*/		
	default:
		MessageBox(NULL, "DM3 ImageData.DataType is unsupported", "ERROR", MB_OK);
		return FALSE;
	}

	I->strip = 1;
	if( I->x == 0 || I->y == 0 )
		return FALSE;

	if (!fioFixImageMemory( I ))
		return FALSE;

	pTag = data->second;
	if( (NULL == pTag) || (pTag->groups.empty()) || (NULL == pTag->groups[0].pData) )
	{
		MessageBox(NULL, "ImageData.Data tag is empty", "ERROR", MB_OK);
		return FALSE;
	}
	LPBYTE pData = (LPBYTE)pTag->groups[0].pData;
	// copy the image data
	switch( pTag->groups[0].data_type )
	{
	case 2: // 2 bytes signed
		I->_pix = PIX_SHORT;
		CopyImageData((SHORT*)I->lpData, (short*)pData, I, 0xFFFF, false, false);
		break;
	case 3: // 4 bytes signed
		I->_pix = PIX_LONG;
		CopyImageData((LPLONG)I->lpData, (long*)pData, I, 0xFFFFFFFF, false, false);
		break;
	case 4:	// 2 bytes unsigned
		I->_pix = PIX_WORD;
		CopyImageData((LPWORD)I->lpData, (LPWORD)pData, I, 0xFFFF, false, false);
		break;
	case 5: // 4 bytes unsigned
		I->_pix = PIX_DWORD;
		CopyImageData((LPDWORD)I->lpData, (LPDWORD)pData, I, 0xFFFFFFFF, false, false);
		break;
	case 6: // 4 bytes unsigned float
		I->_pix = PIX_FLOAT;
		CopyImageData((LPFLOAT)I->lpData, (LPFLOAT)pData, I, 0xFFFF, false, false);
		break;
	case 7: // 8 bytes unsigned double
		I->_pix = PIX_DOUBLE;
		CopyImageData((LPDOUBLE)I->lpData, (LPDOUBLE)pData, I, 0xFFFF, false, false);
		break;
	case 8: // 1 byte unsigned int (boolean)
		I->_pix = PIX_BYTE;
		CopyImageData((LPBYTE)I->lpData, (LPBYTE)pData, I, 0xFF, false, false);
		break;
	case 9: // 1 byte signed char
		I->_pix = PIX_CHAR;
		CopyImageData((LPSTR)I->lpData, (LPSTR)pData, I, 0xFF, false, false);
		break;
	case 10: // 1 byte unsigned char
		I->_pix = PIX_BYTE;
		CopyImageData((LPBYTE)I->lpData, (LPBYTE)pData, I, 0xFF, false, false);
		break;
	default:
		MessageBox(NULL, "ImageData.Data tag is of unknown format", "ERROR", MB_OK);
		return FALSE;
	}

	return TRUE;
}

static BOOL fioReadDM3Data( LPSAVEIMAGE I )
{
	// clean up
	DM3_MAP *pTag;
	groupVecIt git;
	TagVecIt it;
	for(it = GetDm3TagVec().begin(); it != GetDm3TagVec().end(); ++it)
	{
		pTag = *it;
		for(git = pTag->groups.begin(); git != pTag->groups.end(); ++git)
		{
			if( NULL != git->pData )
			{
				delete[] git->pData;
				git->pData = NULL;
			}
		}
		pTag->groups.clear();
		delete *it;
	}
// 	DM3mapIt it;
// 	groupVecIt git;
// 	for(it = s_mapDM3tags.begin(); it != s_mapDM3tags.end(); ++it)
// 	{
// 		DM3_MAP *pTag = it->second;
// 		while( pTag )
// 		{
// 			for(git = pTag->groups.begin(); git != pTag->groups.end(); ++git)
// 			{
// 				delete[] git->pData;
// 			}
// 			pTag->groups.clear();
// 			pTag = pTag->pNext;
// 		}
// 	}
// 	s_mapDM3tags.clear();

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
// Added by Peter on [16/82006]
static BOOL fioCheckNDRData ( LPSAVEIMAGE I )
{
	WORD wID;
	WORD wNumEntr;
	WORD wEntrySize;
	DWORD dwShiftX, dwShiftY, dwWidth, dwHeight;
	int nSize, x, y, iEntry;
	BOOL bRet = FALSE;

	_llseek(hFile, 0, FILE_BEGIN);
	if( 2 != _lread(hFile, &wID, 2) )
	{
		return FALSE;
	}
	if( 2 != _lread(hFile, &wNumEntr, 2) )
	{
		return FALSE;
	}
	for(iEntry = 0; iEntry < wNumEntr; iEntry++)
	{
		if( 2 != _lread(hFile, &wEntrySize, 2) )
		{
			return FALSE;
		}
		if( wEntrySize != _lread(hFile, TMP, wEntrySize) )
		{
			return FALSE;
		}
	}
	if( 4 != _lread(hFile, &dwShiftX, 4) )
		return FALSE;
	if( 4 != _lread(hFile, &dwShiftY, 4) )
		return FALSE;
	if( 4 != _lread(hFile, &dwWidth, 4) )
		return FALSE;
	if( 4 != _lread(hFile, &dwHeight, 4) )
		return FALSE;
	I->x = dwWidth - dwShiftX;
	I->y = dwHeight - dwShiftY;
	I->strip = 1;
#if defined _NDR_24BIT_
	I->ps = 4;
	I->_pix = PIX_DWORD;
#else
	I->ps = 2;
	I->_pix = PIX_WORD;
#endif
	if( (0 == I->x) || (0 == I->y) )
		return FALSE;
	if (!fioFixImageMemory( I ))
		return FALSE;
	nSize = I->y * I->x * 4;
	_llseek(hFile, -nSize, FILE_END);
	LPDWORD src = new DWORD[I->y * I->x], tmp;
	nSize = I->x * 4;
	tmp = src;
	for(y = 0; y < I->y; y++)
	{
		if( nSize != _lread(hFile, tmp, nSize) )
		{
			return FALSE;
		}
		fioReportPercent( y, I->y - 1, TRUE );
		tmp += I->x;
	}
	fioReportPercent( 0, 0, TRUE );
	tmp = src;
#if defined _NDR_24BIT_
	LPDWORD ptr = (LPDWORD)I->lpData;
	DWORD dwMax, dwMin;
#else
	LPWORD ptr = (LPWORD)I->lpData;
	WORD dwMax, dwMin;
	double dScale = (double)0xFFFF / (dwMax - dwMin);
#endif
	dwMin = *min_element(src, src + I->x * I->y);
	dwMax = *max_element(src, src + I->x * I->y);
	if( dwMax == dwMin )
	{
		return FALSE;
	}
	for(y = 0; y < I->y; y++)
	{
		for(x = 0; x < I->x; x++)
		{
#if defined _NDR_24BIT_
			ptr[x] = tmp[x];
#else
			ptr[x] = (tmp[x] - dwMin) * dScale;
#endif
		}
		tmp += I->x;
#if defined _NDR_24BIT_
		ptr = (LPDWORD)(((LPBYTE)ptr) + R4(I->x * I->ps));
#else
		ptr = (LPWORD)(((LPBYTE)ptr) + R4(I->x * I->ps));
#endif
	}
	delete[] src;
/*
	I->ps = 4;
	I->strip = 1;
	I->_pix = PIX_DWORD;
	if( (0 == I->x) || (0 == I->y) )
		return FALSE;
	if (!fioFixImageMemory( I ))
		return FALSE;
	nSize = I->y * I->x * I->ps;
	_llseek(hFile, -nSize, FILE_END);
	nSize = I->x * I->ps;
	LPBYTE ptr = (LPBYTE)I->lpData;
	for(y = 0; y < I->y; y++)
	{
		if( nSize != _lread(hFile, ptr, nSize) )
		{
			return FALSE;
		}
		fioReportPercent( y, I->y - 1, TRUE );
		ptr += nSize;
	}
	fioReportPercent( 0, 0, TRUE );
*/
	bRet = TRUE;

	return bRet;
}

static BOOL fioReadNDRData( LPSAVEIMAGE I )
{
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// Added by Peter on [18/4/2006]
static BOOL fioCheckIPFData( LPSAVEIMAGE I, HFILE hFile )
{
	TAILER	tailer;
	int nBytes;

	// the header is 1024 bytes from the end
	if( HFILE_ERROR == (nBytes = _llseek(hFile, -1024L, FILE_END)) )
	{
		return FALSE;
	}
	// check the signature
	memset(&tailer, 0, 1024);
	nBytes = _lread(hFile, &tailer, 1024);
	if( (1024 != nBytes) ||
		(0 != strnicmp(tailer.part1.id, "DIP0", sizeof(tailer.part1.id))) )
	{
		return FALSE;
	}
	// set the image sizes, swapping data (LINUX case)
	I->x = tailer.part1.xsize; SwapINT(I->x);
	I->y = tailer.part1.ysize; SwapINT(I->y);
	// use 16-bit for fixing the memory
	I->ps = 2;
	I->strip = 1;
	if( !fioFixImageMemory( I ) )
	{
		return FALSE;
	}
	// reassign the real data type
	I->ps = tailer.part1.dtype;
	SwapINT(I->ps);
	I->_pix = PIX_WORD;

	return TRUE;
}

static BOOL fioReadIPFData( LPSAVEIMAGE I )
{
	int		x, y;
	LPBYTE	buff;
	short	*ptr, sval;
	LPWORD	dst;

	_llseek(hFile, 0, FILE_BEGIN);
	for(y = 0; y < I->y; y++)
	{
#ifdef USE_XB_LENGTH
		buff = I->lpData + (I->xb<<1)*(I->y - 1 - y);
#else
		buff = I->lpData + I->stride*(I->y - 1 - y);
#endif
		_lread( hFile, buff, I->x<<1 );
		ptr = (short*)buff;
		dst = (LPWORD)buff;
		if( 0 == I->ps )
		{
			// 12 ADC type
			for(x = 0; x < I->x; x++)
			{
				sval = ptr[x];
				SwapSHORT(sval);
				if( sval >= 0 )	dst[x] = (WORD)(long)sval;
				else						dst[x] = (WORD)(((long)(~sval)<<8)+32768);
			}
		}
		else if( I->ps > 100 )
		{
			// 16 ADC type
			for(x = 0; x < I->x; x++)
			{
				sval = ptr[x];
				SwapSHORT(sval);
				if( sval >= 0 )	dst[x] = (WORD)(long)sval;
				else
					dst[x] = (WORD)((long)(~sval)<<5);
			}
		}
//		if (I->x%4) _lread(hFile, tmp, 4-I->x%4);
//		XlatMove( buff, buff, x_lookup, I->x );
		fioReportPercent( y, I->y-1, TRUE );
	}
	// set it back
	I->ps = 2;
	I->_pix = PIX_WORD;
	fioReportPercent( 0, 0, TRUE );
	return TRUE;
}

// End of addition [18/4/2006]
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static int pcx_cnt, pcx_byte;

static _inline BYTE GetPCXbyte( void )
{
	BYTE	aa;

	if (pcx_cnt==0)
	{
		aa = GetFileByte();
		if (aa<0xC0) return aa;
		pcx_cnt = aa & 0x3F;
		pcx_byte = GetFileByte();
	}
	pcx_cnt--;
	return pcx_byte;
}

static void ReadPCXLine( LPBYTE dst, int w, LPBYTE xlt )
{
	int	i;
	for(i=0;i<w;i++) *dst++ = x_lookup[GetPCXbyte()];
	for(   ;i<pcx_header.bplin;i++) GetPCXbyte();
}
//---------------------------------------------------------------------------
static BOOL	fioCheckPCX( LPSAVEIMAGE I )
{
	int	i;
	BYTE	rgb[3];
	NEWINFO NI;

	_llseek(hFile, 0, FILE_BEGIN);

	if((_lread(hFile, &pcx_header, 128) != 128) ||
	   (pcx_header.manuf != 10)  || (pcx_header.hard    != 5) || // Magic numbers
	   (pcx_header.encod !=  1)  || (pcx_header.bitpix  != 8) ||
	   (pcx_header.nplanes != 1) ||
	   ((pcx_header.x2 - pcx_header.x1)  < 100) ||				// At least 100x100
	   ((pcx_header.y2 - pcx_header.y1)  < 100))
	   return FALSE;
	I->x = pcx_header.x2 - pcx_header.x1 + 1;
	I->y = pcx_header.y2 - pcx_header.y1 + 1;
	if (pcx_header.palinfo != 2) {								// If not grayscale image
		_llseek( hFile, -769, FILE_END);
		i=0; _lread( hFile, &i, 1 ); if (i != 12) return FALSE;	// Magic number
		for (i=0; i<256; i++) {
			_lread( hFile, &rgb, 3 );
			x_lookup[i] = (BYTE) (rgb[0]*0.287 + rgb[1]*0.599 + rgb[2]*0.114+0.5);
		}
	} else 	for (i=0; i<256; i++) x_lookup[i] = (BYTE) i;

	if ((pcx_header.extra[0] == 'P') && (pcx_header.extra[1] == 'I'))
		bDemoPCX = TRUE;	// Acceptable for demo version, it means old style Project Info

	_llseek( hFile, -769-(LONG)sizeof(NEWINFO), FILE_END);
	_lread( hFile, &NI, sizeof(NEWINFO) );
	if ((NI.logo[0]=='N') && (NI.logo[1]=='I')) {
		memcpy(I->lpAdd, &NI, I->iAddSize);		// If logo is OK then copy NEWINFO
		bDemoPCX = TRUE;	// Acceptable for demo version
	}
	_llseek(hFile, 128, FILE_BEGIN);
	filebuffptr = BUFFSIZE+1;
	pcx_cnt = 0;
	I->ps = 1;				// 8 bits only
	I->strip = 1;
	I->_pix = PIX_BYTE;
	if (!fioFixImageMemory( I )) return FALSE;
	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL fioReadPCX ( LPSAVEIMAGE I )
{
	int		y;

	for(y=0; y<I->y; y++)
	{
#ifdef USE_XB_LENGTH
		ReadPCXLine( I->lpData+I->xb*y, I->x, x_lookup);
#else
		ReadPCXLine( I->lpData + I->stride*y, I->x, x_lookup);
#endif
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static BOOL fioCheckBMP( LPSAVEIMAGE I )
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bih;
	int		i;
	BYTE	rgb[4];

	_llseek(hFile, 0, FILE_BEGIN);

	if((_lread( hFile, &bmfh, sizeof(BITMAPFILEHEADER)) != sizeof(BITMAPFILEHEADER)) ||
		(strnicmp( (LPSTR)&bmfh.bfType, "BM", sizeof(bmfh.bfType))!=0) )
		 return FALSE;

	if((_lread( hFile, &bih, sizeof(BITMAPINFOHEADER)) != sizeof(BITMAPINFOHEADER)) ||
		(bih.biSize != sizeof(BITMAPINFOHEADER)) ||
		(bih.biBitCount != 8) || (bih.biCompression != BI_RGB) || (bih.biPlanes != 1))
		 return FALSE;

	I->x = bih.biWidth;
	I->y = bih.biHeight;

	for (i=0; i<256; i++) {
		_lread( hFile, &rgb, 4 );
		x_lookup[i] = (BYTE) (rgb[0]*0.287 + rgb[1]*0.599 + rgb[2]*0.114+0.5);
	}
	I->_pix = PIX_BYTE;
	I->ps = 1;
	I->strip = 1;
	if (!fioFixImageMemory( I )) return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL fioReadBMP ( LPSAVEIMAGE I )
{
	int		y;
	LPBYTE	buff;
	char tmp[4*3];
	for(y=0; y<I->y; y++)
	{
#ifdef USE_XB_LENGTH
		buff = I->lpData+I->xb*(I->y-1-y);
#else
		buff = I->lpData + I->stride*(I->y-1-y);
#endif
		_lread( hFile, buff, I->x );
		if (I->x%4) _lread(hFile, tmp, 4-I->x%4);
		XlatMove( buff, buff, x_lookup, I->x );
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static BOOL fioCheckSPE( LPSAVEIMAGE I )
{
	WORD	hdr[4100/2];

	_llseek(hFile, 0, FILE_BEGIN);

	if(_lread( hFile, &hdr, sizeof(hdr)) != sizeof(hdr))  return FALSE;

	if( (hdr[2996/2]!=0x4567) || (hdr[2998/2]!=0x0123) ) return FALSE;

	switch(hdr[108/2])
	{
		case 2:	I->bSigned = TRUE; break;
		case 3: I->bSigned = FALSE; break;
		default: return FALSE;
	}

	I->x = hdr[42/2];
	I->y = hdr[656/2];
	I->ps = 2;
	I->_pix = PIX_WORD;
	_llseek(hFile, 4100, FILE_BEGIN);
	if (!fioFixImageMemory( I ))
		return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL fioReadSPE ( LPSAVEIMAGE I )
{
	int		y;
	LPBYTE	buff;

	for(y=0; y<I->y; y++) {
#ifdef USE_XB_LENGTH
		buff = I->lpData+I->xb*y*I->ps;
#else
		buff = I->lpData+I->stride*y*I->ps;
#endif
		_lread( hFile, buff, I->x*I->ps );
		if (I->bSigned) WordXorMove( (LPWORD)buff, (LPWORD)buff, I->x, 0x8000);
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
DWORD	getMlong( HFILE in )
{
BYTE	br[4];
	_lread( hFile, br, 4 );
	return ((((((br[0]<<8)+br[1])<<8)+br[2])<<8)+br[3]);
}
//---------------------------------------------------------------------------
static BOOL fioCheckVIF( LPSAVEIMAGE I )
{
	int		i;
	LPDWORD lpw;
	LPBYTE	mp = (LPBYTE)TMP;
	VIFFHEADER	vf;

	_llseek(hFile, 0, FILE_BEGIN);

	_lread(hFile, &vf, 512+4);
	lpw = &vf.cpu;
	for(i=0;i<22;i++) *(lpw++) = getMlong(hFile);
	if (vf.id!=0xAB || vf.f_t!=1 || vf.rel!=1 || vf.ver!=3 ||
		vf.cpu!=0 || vf.location_type!=1 || vf.location_dim!=0 ||
		vf.num_of_images!=1 || vf.num_data_bands!=1 || vf.data_storage_type!=1 ||
		vf.data_encode_scheme!=0 || vf.map_scheme!=1 || vf.map_storage_type!=1 ||
		vf.map_col_size!=256 || vf.map_subrow_size!=0 || vf.map_enable!=1 ||
		vf.maps_per_cycle!=0) return FALSE;

	_llseek(hFile, 1024, FILE_BEGIN);
	if (vf.map_row_size==3 && vf.color_space_model==0xF) {
		_lread(hFile, mp,256*3);
		for (i=0; i<256; i++) x_lookup[i] = (BYTE) (mp[i]*0.287 + mp[i+256]*0.599 + mp[i+512]*0.114+0.5);
	} else if (vf.map_row_size==1 && vf.color_space_model==0) {
		_lread(hFile, x_lookup,256);
	} else return FALSE;

	I->x = (int)vf.row_size;
	I->y = (int)vf.col_size;
	I->ps = 1;
	I->_pix = PIX_BYTE;
	I->strip = 1;
	if (!fioFixImageMemory( I )) return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL fioReadVIF ( LPSAVEIMAGE I )
{
int		y;
LPBYTE	buff;

	for(y=0; y<I->y; y++) {
#ifdef USE_XB_LENGTH
		buff = I->lpData+I->xb*y;
#else
		buff = I->lpData+I->stride*y;
#endif
		_lread( hFile, buff, I->x );
		XlatMove( buff, buff, x_lookup, I->x );
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static BOOL fioCheckIMG_Kontron( LPSAVEIMAGE I )
{
KONTRONHDR	hdr;

	_llseek(hFile, 0, FILE_BEGIN);

	if(_lread( hFile, &hdr, sizeof(KONTRONHDR)) != sizeof(KONTRONHDR))  return FALSE;

	if( (hdr.id1 != 0x1247) || (hdr.id2 != 0xB06D)) return FALSE;

	I->x = hdr.width;
	I->y = hdr.height;
	I->ps = 1;
	I->strip = 1;
	I->_pix = PIX_BYTE;
	_llseek(hFile, hdr.len*128, FILE_BEGIN);
	if (!fioFixImageMemory( I )) return FALSE;

	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL fioReadRAW ( LPSAVEIMAGE I )
{
int		y;

	for(y=0; y<I->y; y++) {
#ifdef USE_XB_LENGTH
		_lread( hFile, I->lpData+I->xb*y, I->x );
#else
		_lread( hFile, I->lpData+I->stride*y, I->x );
#endif
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static BOOL fioCheckIMG_WINSTOR( LPSAVEIMAGE I )
{
IMG_WINSTOR winstor_header;

	_llseek(hFile, 0, FILE_BEGIN);

	if((_lread(hFile, &winstor_header, sizeof(IMG_WINSTOR)) != sizeof(IMG_WINSTOR)) ||
		(winstor_header.logo[0] != 'I') ||
		(winstor_header.logo[1] != 'M'))
		 return(FALSE);

	// Now supporting only EIGHT_BIT with/without COMPRESSION
	if ( winstor_header.type > 1 ) return myError( IDS_ERRUNSUPPORTEDFORMAT );
	winstortype = winstor_header.type;

	I->x = winstor_header.width;
	I->y = winstor_header.height;
	I->ps = 1;
	I->strip = 1;
	I->_pix = PIX_BYTE;
	_llseek(hFile, 64+winstor_header.com_length, FILE_BEGIN);
	filebuffptr = BUFFSIZE+1;

	if (!fioFixImageMemory( I )) return FALSE;
	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL fioReadWinstorLine( LPBYTE buff, int cnt )
{
short lincnt;
BYTE	aa;
WORD	bb;

	lincnt = GetFileByte();
	lincnt = lincnt + (GetFileByte()<<8);

	*buff++ = GetFileByte(); lincnt--; cnt--;
	while (lincnt>0 && cnt>0) {
		bb = aa = GetFileByte(); lincnt--;
		if ((aa&0xF) == 0xF) {	// overflow
			bb |= (WORD)GetFileByte()<<8; lincnt--;
			bb >>= 4;
			*buff++ = (BYTE)bb; cnt--;
			bb >>= 4;
		} else {
			aa &= 0xF;
			if (aa & 8) { aa-=7; aa=-aa; }
			*buff = *(buff-1)+aa; cnt--;
			buff++;
		}
		if ((bb&0xF0) == 0xF0) {
			*buff++ = GetFileByte(); lincnt--; cnt--;
		} else {
			bb>>=4;
			if (bb&8) { bb-=7; bb=-bb; }
			*buff = *(buff-1)+bb; cnt--;
			buff++;
		}
	}
	return TRUE;
}

//---------------------------------------------------------------------------
static BOOL fioReadIMG_WINSTOR( LPSAVEIMAGE I)
{
int		y;

	if (winstortype==EIGHT_BIT) return fioReadRAW( I );

	for(y=0; y<I->y; y++) {
#ifdef USE_XB_LENGTH
		fioReadWinstorLine( I->lpData+I->xb*y, I->x );
#else
		fioReadWinstorLine( I->lpData+I->stride*y, I->x );
#endif
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	return TRUE;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static BOOL fioCheckMRC( LPSAVEIMAGE I )
{
	// http://bio3d.colorado.edu/imod/doc/mrc_format.txt
	long	v;

#define	GetV(n)	 if ((_llseek(hFile, (n-1)*4, FILE_BEGIN) == HFILE_ERROR) || (4 != _lread(hFile, &v, 4))) return FALSE

	_llseek(hFile, 0, FILE_BEGIN);

	GetV(1);	
	if (v>8000) 
		return FALSE;	// # of Columns, and some check
	I->x = (int)v;

	GetV(2);	
	if (v>8000) 
		return FALSE;	// # of Rows
	I->y = (int)v;

	/*
	Data type/mode
     0 = unsigned or signed bytes depending on flag in imodStamp
             only unsigned bytes before IMOD 4.2.23
   	 1 = signed short integers (16 bits)
   	 2 = float
   	 3 = short * 2, (used for complex data)
   	 4 = float * 2, (used for complex data)
     6 = unsigned 16-bit integers (non-standard)
   	16 = unsigned char * 3 (for rgb data, non-standard)
	*/
	GetV(4);
	MRC_Mode = (int)v;
	switch (MRC_Mode) {
		case 0:
			I->_pix = PIX_BYTE;
			I->ps = 1;	// Byte and float
			break;
		case 1:
		case 2:
			I->_pix = PIX_WORD;
			I->ps = 2;	// Int
			break;
		case 4:
			return FALSE;
		case 6:
			I->_pix = PIX_WORD;
			I->ps = 2;
			break;
		case 16:
			return FALSE; // don't support RGB data for now
	}


	GetV(24);	// # of extra bytes used for storing symm. oper.

	if (_llseek(hFile, 256*4+v, FILE_BEGIN) == HFILE_ERROR) return FALSE;
	I->strip = 1;
	if (!fioFixImageMemory( I )) return FALSE;

	return TRUE;
#undef	GetV
}

//---------------------------------------------------------------------------
static BOOL fioReadMRC( LPSAVEIMAGE I)
{
	LPBYTE		b;
	int			y;
	float		fmin, fmax, /*fsub, */fmul;
	int			itemsize;
	LONG		filepos;
	std::vector<float> temp;

	switch ( MRC_Mode )
	{
		case 0: 
			itemsize = 1;
			for (y = 0; y < 256; y++)
				x_lookup[y] = (BYTE)(y ^ 0x80);
			break;
		case 1: 
		case 6:
			itemsize = 2; 
			break;
		case 2:
			itemsize = 4;
			temp.resize(I->x*I->y);
			break;
	}

	filepos = _ltell( hFile );
	for(y=0; y<I->y; y++)
	{
#ifdef USE_XB_LENGTH
		b = I->lpData + I->xb*(I->y-1-y)*I->ps;
#else
		b = I->lpData + I->stride*(I->y-1-y);
#endif
		switch( itemsize )
		{
			case 1:
				_lread(hFile, b, I->x);
				XlatMove(b, b, x_lookup, I->x );
				break;
			case 2:
				_lread(hFile, b, I->x*2);
				WordXorMove( (LPWORD)b, (LPWORD)b, I->x, 0x8000);
				break;
			case 4:
				_lread(hFile, &*temp.begin() + I->x*y, I->x*4);
//				GetFloatMinMax( (float*)tmp, I->x, &fmin, &fmax );
			break;
		}
		fioReportPercent( y, I->y-1, TRUE );
	}
	fioReportPercent( 0, 0, TRUE );
	if( 4 == itemsize )
	{
		fmin = *std::min_element(temp.begin(), temp.end());
		fmax = *std::max_element(temp.begin(), temp.end());
		fmul = 65535.0 / (fmax - fmin);
		for(y = 0; y < I->x*I->y; y++)
		{
			((LPWORD)I->lpData)[y] = (temp[y] - fmin)*fmul;
		}
	}
/*
	if (itemsize == 4) {
		_llseek( hFile, filepos, FILE_BEGIN );
		if ((fmax-fmin)<.1) fmax=fmin*1.1; fsub = fmin;
		fmul = 255./(fmax-fmin);
		for(y=0; y<S->y; y++) {
			b = I->lpData + I->xb*(I->y - 1 - y);
			_lread(hFile, tmp, 4*I->x);
			Float2ByteMove( b, (float far *)tmp, w, fsub, fmul);
		}
	}
*/
	return TRUE;
}
/****************************************************************************/
// added by Peter on 10 Oct 2005 for FEI
/*
#ifdef __FEI__

static std::vector<DWORD> s_FEI_vDataOffs;
static std::vector<DWORD> s_FEI_vTagOffs;
static std::vector<FEI_ESD_1D_DATA> s_FEI_vData1D;
static std::vector<FEI_ESD_2D_DATA> s_FEI_vData2D;
static FEI_ESD_HEADER s_FEI_header;

static BOOL fioCheckSERData(LPSAVEIMAGE I)
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
		return FALSE;
	}
	// check SeriesID
	if( 0x0197 != s_FEI_header.wSeriesID )
	{
		MessageBox(NULL, "FEI SER file has invalid SeriesID in the s_FEI_header", "ERROR", MB_OK);
		return FALSE;
	}
	// check SeriesVer
	if( 0x0210 != s_FEI_header.wSeriesVer )
	{
		MessageBox(NULL, "FEI SER file has invalid SeriesVersion in the header", "ERROR", MB_OK);
		return FALSE;
	}
	if( 0 == s_FEI_header.dwTotalElements )
	{
		MessageBox(NULL, "FEI SER file has zero total number of entries", "ERROR", MB_OK);
		return FALSE;
	}
	if( 0 == s_FEI_header.dwValidNumElements )
	{
		MessageBox(NULL, "FEI SER file has zero valid number of entries", "ERROR", MB_OK);
		return FALSE;
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
	return TRUE;

Error_Exit:
	return FALSE;
}

static BOOL fioReadSER(OBJ* pObj, SAVEIMAGE &I, std::vector<OBJ*> &vObjects)
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
			I.ps = 1;
		}
		else if( (ESD_DATA_UINT2 == data2D.wDataType) ||
			(ESD_DATA_INT2 == data2D.wDataType) )
		{
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
			if( I.xb*I.ps != _lread(hFile, I.lpData+I.xb*y*I.ps, I.xb*I.ps) )
			{
				return FALSE;
			}
			fioReportPercent(y, I.y - 1, TRUE);
		}
		CRISP_InitObjFromImage(pCurObj, I);
		vObjects.push_back(pCurObj);
		// zero the progress bar
		fioReportPercent(0, 0, TRUE);
	}
	return TRUE;

Error_Exit:
	return FALSE;
}
#endif // __FEI__
*/

/****************************************************************************/
// added by Peter on 10 Oct 2001
BOOL WINAPI	fioFreeImageFile( LPSAVEIMAGE pS )
{
	if(pS)
	{
		free(pS->lpData);
		ZeroMemory(pS, sizeof(SAVEIMAGE));
		return TRUE;
	}
	return FALSE;
}
/****************************************************************************/
BOOL WINAPI fioOpenImageFile( LPSTR fname, OBJ* pObj, LPSAVEIMAGE pI,
							 std::vector<OBJ*> &vObjects )
{
	// NOTE: don't use this function directly, instead use _OpenShellFile
	char	drv[_MAX_DRIVE], dir[_MAX_DIR], file[_MAX_DIR];
	char	ext[_MAX_EXT];
	int		iError = 0, iRMode = 0;
//	int		sernumb;

//	_splitpath( fname, &TMP[0], &TMP[100], &TMP[200], ext );
	_splitpath( fname, drv, dir, file, ext );

	if ((hFile = _lopen(fname, OF_READ)) == HFILE_ERROR)
	{
		::MessageBox(NULL, GetUserErrorAndTrace(_FMT(_T("Failed to open file '%s': '%s'"),
			fname, GetLastErrorString().c_str())).c_str(), "CRISP2 file open error", MB_OK);
		return FALSE;
	}

	if ( bDemo || strlen(szSerNo) == 0 )
	{
		if (stricmp( ext, ".XIF" ) == 0 && fioCheckTIFF( pI ))
			iRMode = _TIF_CRY_;
		else
			iError = IDS_ERRUNKNOWNFORMAT;
		goto xxx;
	}
	// Not in demo mode
	if (stricmp( ext, ".PCX" ) == 0)
	{
		if (fioCheckPCX( pI )) iRMode = _PCX_;
		else				  iError = IDS_ERRINVALIDPCX;
	}
	else if (stricmp( ext, ".IMG" )==0)
	{
		if      (fioCheckIMG_WINSTOR( pI )) iRMode = _WINSTOR_;
		else if (fioCheckIMG_Kontron( pI )) iRMode = _KONTRON_;
//		else if (fioCheckIMG_CRISP  ( pI )) iRMode = _CRISP_;
		else	                           iError = IDS_ERRINVALIDIMG;
	}
	else if (stricmp( ext, ".XIF" ) == 0)
	{
		if (fioCheckTIFF( pI )) iRMode = _TIF_CRY_;
		else				   iError = IDS_ERRINVALIDTIF;
	}
	else if (stricmp( ext, ".TIF" )==0)
	{
		if (fioCheckTIFF( pI )) iRMode = _TIFF_;
		else				   iError = IDS_ERRINVALIDTIF;
	}
	else if (stricmp( ext, ".JPG" )==0)
	{
		if (fioCheckJPEG( pI )) iRMode = _JPEG_;
		else				       iError = IDS_ERRINVALIDJPG;
	}
	else if (stricmp( ext, ".MRC" )==0)
	{
		if (fioCheckMRC ( pI )) iRMode = _MRC_;
		else				   iError = IDS_ERRINVALIDMRC;
	}
	else if (stricmp( ext, ".BMP" )==0)
	{
		if (fioCheckBMP ( pI )) iRMode = _BMP_;
		else				   iError = IDS_ERRINVALIDBMP;
	}
	else if (stricmp( ext, ".VIF" )==0)
	{
		if (fioCheckVIF ( pI )) iRMode = _VIF_;
		else				   iError = IDS_ERRINVALIDVIF;
	}
	else if (stricmp( ext, ".SPE" )==0)
	{
		if (fioCheckSPE ( pI )) iRMode = _SPE_;
		else				   iError = IDS_ERRINVALIDSPE;
	}
	else if( 0 == stricmp( ext, ".IPF" ) ) // Before IP*!!!
	{
		if( fioCheckIPFData( pI, hFile )) iRMode = _IPF_;
		else				   iError = IDS_ERRINVALIDIPF;
	}
	else if (strnicmp( ext, ".IP", 3)==0)	// can be IPL, IPH, IPR or IPC
	{
		if( fioCheckIP ( pI ))  iRMode = _DITABIS_;
		else				   iError = IDS_ERRINVALIDIP;
	}
	else if (stricmp( ext, ".RAW")==0)	// can be raw
	{
		if( fioCheckRAWData ( pI )) iRMode = _RAW_;
		else				   iError = IDS_ERRINVALIDRAW;
	}
	else if( 0 == stricmp( ext, ".DM2") )
	{
		if( fioCheckDM2Data ( pI )) iRMode = _DM2_;
		else				   iError = IDS_ERRINVALIDDM2;
	}
	else if( 0 == stricmp( ext, ".DM3") )
	{
		if( fioCheckDM3Data ( pI )) iRMode = _DM3_;
		else				   iError = IDS_ERRINVALIDDM3;
	}
	else if( 0 == stricmp( ext, ".NDR") )
	{
		if( fioCheckNDRData ( pI )) iRMode = _NDR_;
		else				   iError = IDS_ERRINVALIDNDR;
	}
#ifdef __FEI__
	else if( 0 == stricmp( ext, ".SER" ) )
	{
		if( FEI_fioCheckSERData( pI, hFile )) iRMode = _SER_;
		else				   iError = IDS_ERRINVALIDSER;
	}
	else if( 0 == stricmp( ext, ".EMI" ) )
	{
		if( FEI_fioCheckEMIData( pI, hFile )) iRMode = _EMI_;
		else				   iError = IDS_ERRINVALIDSER;
	}
#endif
	else
	{
		_lclose (hFile);
		hFile = 0;
		return myError(IDS_ERRUNKNOWNFORMAT);
	}
xxx:
	if (iError)
	{
		_lclose( hFile );
		hFile = 0;
		return myError(iError);
	}

	switch( iRMode )
	{
		case _PCX_:		iError = fioReadPCX ( pI ); break;
		case _WINSTOR_:	iError = fioReadIMG_WINSTOR ( pI ); break;
		case _KONTRON_:	iError = fioReadRAW ( pI ); break;
		case _TIFF_:	iError = fioReadTIFF( pI ); break;
		case _TIF_CRY_:	iError = fioReadTIFFCrypt ( pI ); break;
		case _JPEG_:	iError = fioReadJPEG( pI ); break;
		case _MRC_:		iError = fioReadMRC ( pI ); break;
		case _BMP_:		iError = fioReadBMP ( pI ); break;
		case _VIF_:		iError = fioReadVIF ( pI ); break;
		case _SPE_:		iError = fioReadSPE ( pI ); break;
		case _DITABIS_: iError = fioReadDITABIS( pI ); break;
		case _RAW_:		iError = fioReadRAWData( pI ); break;
		case _DM2_:		iError = fioReadDM2Data( pI ); break;
		case _DM3_:		iError = fioReadDM3Data( pI ); break;
		case _NDR_:		iError = fioReadNDRData( pI ); break;
#ifdef __FEI__
		case _SER_:
			iError = FEI_fioReadSER( pObj, *pI, vObjects, hFile );
			pObj = NULL; // we do not need object initialization
			break;
		case _EMI_:
			// EMI opens file itself
			_lclose( hFile );
			hFile = 0;
			iError = FEI_fioReadEMI( fname, pObj, *pI, vObjects, hFile );
			pObj = NULL; // we do not need object initialization
			break;
#endif
		case _IPF_:		iError = fioReadIPFData( pI ); break;
	}
	// close if possible
	if( 0 != hFile )
	{
		_lclose( hFile );
	}
	hFile = 0;
	if( !iError )
		return myError( IDS_ERRREADFILE );
	else
	{
		CRISP_InitObjFromImage(pObj, *pI);
	}
	regUpdateRecentList(fname);
	return TRUE;
}
/****************************************************************************/
static BOOL	WriteTIFFHead( LPSAVEIMAGE I, HFILE hFile )
{
	Dloc.hFile = hFile;

	tifRWptr = (sizeof(TIFFHDR) + sizeof(tifMAKE) + 3)&(~3);	// Offset for real data
	if (_llseek( Dloc.hFile, tifRWptr, FILE_BEGIN ) == HFILE_ERROR) return FALSE;	// Reserve space for header+MAKE
	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL	WriteTIFFLine( LPBYTE buff, size_t w )
{
	if (_lwrite( Dloc.hFile, (LPCSTR)buff, w ) != w) return FALSE;
	return TRUE;
}
//---------------------------------------------------------------------------
static BOOL	WriteTIFFTail( LPSAVEIMAGE I )
{
	DWORD	oMake = sizeof(TIFFHDR);
	DWORD	oXres, oYres, oSoft, oPI, oIFD;
	DWORD	dwtmp;
	WORD	wtmp;
	char	Soft[]="*CRISP 2*";
	DIRENTRY de;
	TIFFHDR	th = {INTELTIFF, 42, 0};

#define	putl(ll)	dwtmp = ll; if (_lwrite(Dloc.hFile, (LPCSTR)&dwtmp, 4) != 4) return FALSE;
#define	putw(ww)	 wtmp = ww; if (_lwrite(Dloc.hFile,  (LPCSTR)&wtmp, 2) != 2) return FALSE;
#define	putx(st,si)	if (_lwrite(Dloc.hFile, (LPCSTR)st, (si+1)&(~1)) != (UINT)((si+1)&(~1))) return FALSE;
#define	putz(ss)	if (_lwrite(Dloc.hFile, (LPCSTR)ss, ((sizeof(ss)+1)&(~1))) != ((sizeof(ss)+1)&(~1))) return FALSE;
#define putE(tg, ty, le, vl) de.deTag=tg; de.deType=ty; de.deLength=le; de.deVal=vl;\
							 if (_lwrite(Dloc.hFile, (LPCSTR)&de, sizeof(DIRENTRY)) != sizeof(DIRENTRY)) return FALSE;

	if( 0 != (_ltell(Dloc.hFile) & 1) )
		_lwrite(Dloc.hFile, (LPCSTR)&dwtmp, 1);	// Make fprt even
	oXres = _ltell( Dloc.hFile ); putl(200); putl(1);
	oYres = _ltell( Dloc.hFile ); putl(200); putl(1);
	oSoft = _ltell( Dloc.hFile ); putz( Soft );
	oPI   = _ltell( Dloc.hFile ); putx( I->lpAdd, I->iAddSize );

	if( 0 != (_ltell(Dloc.hFile) & 1) )
		_lwrite(Dloc.hFile, (LPCSTR)&dwtmp, 1 ); // Make fprt even

	oIFD  = _ltell( Dloc.hFile );
	putw(14);		// !!! Number of IFD entries

	putE(TGNEWSUBFILETYPE, 				TIFFLONG,		1,				0			);
	putE(TGIMAGEWIDTH,					TIFFSHORT,		1,				I->x		);
	putE(TGIMAGELENGTH,					TIFFSHORT,		1,	 			I->y		);
	putE(TGBITSPERSAMPLE,				TIFFSHORT,		1,	 			I->ps*8		);
	putE(TGCOMPRESSION,					TIFFSHORT,		1,	 			CoNOCOMP	);
	putE(TGPHOTOMETRICINTERPRETATION,	TIFFSHORT,		1,	 			PhZEROBLACK );
	putE(TGMAKE,						TIFFASCII,		sizeof(tifMAKE), oMake		);
	putE(TGSTRIPOFFSETS,				TIFFLONG,		1,				tifRWptr	);
	putE(TGSAMPLESPERPIXEL,				TIFFSHORT,		1,				1			);
	putE(TGXRESOLUTION,					TIFFRATIONAL,	1,				oXres		);
	putE(TGYRESOLUTION,					TIFFRATIONAL,	1,				oYres		);
	putE(TGRESOLUTIONUNIT,				TIFFSHORT,		1,				1			);
	putE(TGSOFTWARE,					TIFFASCII,		sizeof(Soft),	oSoft		);
	putE(TGPROJINFO,					TIFFLONG,		1,				oPI			);

	putl( 0 );	// Next IFD
	_llseek(Dloc.hFile, 0, FILE_BEGIN);
	th.thIfdOffset = oIFD;
	putx( &th, sizeof(TIFFHDR) );
	putz( tifMAKE );
	return TRUE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
static BOOL fioSaveTIF( LPSTR szFname, LPSAVEIMAGE I, BOOL bCrypt )
{
	BYTE *	buf = I->lpData;
	BYTE *	b;
#ifdef USE_XB_LENGTH
	int		w = I->xb, y;
#else
	int		w = R4(I->x), y;
#endif
	BOOL	ret=FALSE;
	HFILE 	hFile;
	LPBYTE	pbuf;

	if ((hFile = _lcreat(szFname,0)) == HFILE_ERROR) return FALSE;

	WriteTIFFHead( I, hFile );

#ifdef USE_XB_LENGTH
	pbuf = (LPBYTE)malloc( I->x*I->ps );
#else
	pbuf = (LPBYTE)malloc( I->stride );
#endif

	initcypher(FALSE);

	try
	{
		// Added by Peter on 22 Feb 2006
		b = buf;
		// end of addition
		for(y = 0; y < I->y; y++)
		{
			// Modified by Peter on 22 Feb 2006
//			b = buf + w*y;//*I->ps;
			if (bCrypt) encrypt( pbuf, b, I->x*I->ps );
			else        memcpy ( pbuf, b, I->x*I->ps );
			if (( ret = WriteTIFFLine( pbuf, I->x*I->ps )) == FALSE ) goto quit;
			fioReportPercent( y, I->y-1, FALSE );
			// Added by Peter on 22 Feb 2006
			b += R4(w*I->ps);
			// end of addition
		}
		ret = WriteTIFFTail( I );
	}
	catch (std::exception& e)
	{
		Trace(_FMT(_T("fioSaveTIF() -> %s"), e.what()));
	}
	fioReportPercent( 0, 0, FALSE );
quit:
	_lclose( hFile );
	return ret;
}
/****************************************************************************/
static	BOOL	fioSaveBMP( LPSTR fname, LPSAVEIMAGE I)
{
	LPBYTE 	buf = I->lpData;
	LPBYTE 	b;
#ifdef USE_XB_LENGTH
	int			w = I->xb, y;
#else
	int			w = R4(I->x), y;
#endif
	BOOL		ret = FALSE;
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER bih;
	FILE 	* File;

	if ((File = fopen( fname, "wb")) == NULL) return FALSE;

	bmfh.bfType = 'MB';
	bmfh.bfSize = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256+I->x*I->y;
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256;
	fwrite( &bmfh, sizeof(BITMAPFILEHEADER), 1, File);

	memset( &bih, 0, sizeof(BITMAPINFOHEADER));
	bih.biSize = sizeof(BITMAPINFOHEADER);
	bih.biWidth = I->x;
	bih.biHeight = I->y;
	bih.biPlanes = 1;
	bih.biBitCount = 8;
	bih.biCompression = BI_RGB;
	fwrite( &bih, sizeof(BITMAPINFOHEADER), 1, File);

	for(y=0;y<256;y++) {
		fputc(y, File);
		fputc(y, File);
		fputc(y, File);
		fputc(0, File);
	}

	for(y=0;y<I->y;y++) {
		b = buf + w*(I->y - y - 1);
		fioReportPercent( y, I->y-1, FALSE );
		fwrite( b, 1, I->x, File );
	}
	ret = ! ferror( File );
	fclose( File );
	fioReportPercent( 0, 0, FALSE );
	return ret;
}
/****************************************************************************/
// added by Peter on 10 Oct 2005 for FEI
#ifdef __FEI__
static	BOOL	fioSaveSER( LPSTR fname, LPSAVEIMAGE I)
{
	FILE 	*pFile;

	if( NULL == (pFile = fopen(fname, "wb")) )
		return FALSE;

	fclose(pFile);

	return TRUE;
}
#endif
/****************************************************************************/
BOOL WINAPI fioSaveImageFile( LPSTR fname, LPSAVEIMAGE I, WORD wJPGq )
{
	char	ext[_MAX_EXT];
	int		i;

	_splitpath( fname, &TMP[0], &TMP[100], &TMP[200], ext );
	strupr( ext );

	// Added by Peter on 12 April 2007
	if( (0 == ext[0]) )
	{
		strcpy(ext, ".TIF");
	}
	// End of addition by Peter on 12 April 2007

	if( 0==stricmp( ext, ".XIF" ) && 0!=strlen(szSerNo) && 1==sscanf(szSerNo,"%d",&i) && i < 5 )
		return fioSaveTIF( fname, I, TRUE  );	// Crypted
	else if( 0 == stricmp( ext, ".TIF" ) ) return fioSaveTIF( fname, I, FALSE );
	else if( 0 == stricmp( ext, ".JPG" ) ) return fioSaveJPG( fname, I, wJPGq );
	else if( 0 == stricmp( ext, ".BMP" ) ) return fioSaveBMP( fname, I );
#ifdef __FEI__
	else if( 0 == stricmp( ext, ".SER" ) ) return fioSaveSER( fname, I );
#endif
	else myError(IDS_ERRUNSUPPORTEDFORMAT);
	return FALSE;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
// Added by Peter on 6 Aug 2003
static BOOL BrowseForFile(HWND hOwner, LPTSTR pszOutputFile, DWORD nMaxFName)
{
	OPENFILENAME ofn;

	memset(&ofn, 0, sizeof(ofn));
#if (_WIN32_WINNT >= 0x0500)
	// in order to avoid Windows NT4,95/98 misunderstanding
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

#pragma pack(pop)