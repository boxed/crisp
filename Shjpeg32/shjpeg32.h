/****************************************************************************/
/* SOFTHARD JPEG Library definitions * by ML ********************************/
/****************************************************************************/
/* Designed for SIS's *******************************************************/
/* analySIS system    *******************************************************/
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// Color spaces defininition
typedef enum
{	CS_UNKNOWN,		// error/unspecified
	CS_GRAYSCALE,	// monochrome (only 1 component)
	CS_RGB,			// red/green/blue
	CS_YCbCr,		// Y/Cb/Cr (also known as YUV)
	CS_YIQ,			// Y/I/Q
	CS_CMYK			// C/M/Y/K
} CSPACE;

// Description of the JPEG task
//
typedef struct
{	DWORD	image_width;	// I/O+ overall image width
	DWORD	image_height;	// I/O+ overall image height
	DWORD	strip_height;	// O/O  image strip height
	DWORD	num_components;	// O/O  # of color components in JFIF image
	CSPACE	color_space;	// I/O+ CS of JFIF file (Dec), or CS for data buffer (Cod)
	DWORD	quality;		// I/x+ Quality parameter
	DWORD	density_unit;	// I/O- JFIF code for pixel size units
	DWORD	X_density;		// I/O- Horizontal pixel density
	DWORD	Y_density;		// I/O- Vertical pixel density
	DWORD	precision;		// I/O+ Sample precision, can be 8 in any CS and 12 ONLY for CS_GRAYSCALE
} JINFO;
// Where first letter in comments means direction for coder,
// the second for decoder (I-Input parameter, O-Output parameter, x-N.A.)
// '+' means essential, '-' non essential parameter.
typedef	JINFO FAR * LPJINFO;

// Some useful pointers for TIFF 6.0 implementation
//
typedef struct {
	DWORD	dwSOI;			// JPEGInterchangeFormat
	DWORD	dwQTBL[3];		// JPEGQTables
	DWORD	dwDCTBL[3];		// JPEGDCTables
	DWORD	dwACTBL[3];		// JPEGACTables
	DWORD	dwData;			// Probably useful for TAG273
	DWORD	dwTotal;		// JPEGInterchangeFormatLength
} JPEGOFFSETS;
typedef	JPEGOFFSETS FAR * LPJPEGOFFSETS;

// Callback function to get data buffer.
// Returns pointer to memory block and its size in dwSize
// in case of error dwSize=0.
typedef	BOOL	(CALLBACK * JPGCBACK) ( LPDWORD lpBuf, LPDWORD dwSize );

// Version function returns 0x100 for Version 1.00
DWORD WINAPI	jpgGetVersion(VOID);

/****************************************************************************/
/* CODER ********************************************************************/
/****************************************************************************/
// JPEG coder init
// Returns the number of bytes emmited since last callback, or zero in case of error.
DWORD WINAPI jpgCodOpen(LPJINFO	lpJINFO,			//
						LPBYTE	lpAddon,			// Pointer to additional data
						DWORD	wAddonSize,			// Size of additional data
						JPGCBACK lpfnSetData);		// Callback to get new data buffer

// Encodes one strip from lpPixel buffer,
// buffer width = wWidth.
// Returns the number of byte emitted since last callback, or zero in case of error
DWORD WINAPI jpgCodStrip(BYTE *lpPixel,		// Far pointer to src pixels
													// e.g. 24bits: RGB,RGB,RGB,...
													// or    8bits: Y0,Y1,Y2,...
													// or   YYUV  : Y0,Y1,U0,V0,...
						 DWORD	wWidth);			// Src buffer width

// Encodes JFIF file trailer.
// Returns the number of byte emitted since last callback, or zero in case of error
DWORD WINAPI jpgCodClose( void );		//

// Fills supplied structure with current pointers values.
// It is possible to call this function at any time,
// but all pointers are valid only after call to jpgCodClose
VOID WINAPI jpgGetJPEGOffsets( LPJPEGOFFSETS lpJo );

/****************************************************************************/
/* DECODER ******************************************************************/
/****************************************************************************/
// JPEG decoder init, fills out lpwColorSpace, lpImageWidth, lpImageHeight.
// lpAddon buffer will be filled with information if :
// 		a) This information exists in the header;
//		b) Buffer size is enogh to store these data from header.
// Returns FALSE in case of any error
BOOL WINAPI	jpgDecOpen(	LPJINFO	lpJINFO,			// Strip height for decoding
						LPBYTE	lpAddon,			// Pointer to additional data
						LPDWORD	lpwAddonSize,		// Size of additional data buffer on input
													// Actual size of additional data copied
													// or zero in case of error
						JPGCBACK lpfnGetData);		// Callback to get encoded data ptr

// Decodes one strip (strip_height lines).
// Stores Y,Y,... 	if cSpace = CS_GRAYSCALE
//        RGB,RGB,... if cSpace = CS_RGB
//        Y0,Y1,U0,V0... if cSpace = CS_YCbCr
// Returns the number of lines decoded, or zero in case of error
DWORD WINAPI jpgDecStrip(BYTE *lpPixel,			// Ptr to dst pixels
						DWORD	wWidth,				// lpPixel buffer width
						CSPACE	cSpace);			// Color space

// Closes the DECODER.
VOID WINAPI	jpgDecClose( void );

#ifdef __cplusplus
}
#endif
