/****************************************************************************/
/* Copyright(c) 1997,98,99  SoftHard Technology, Ltd. ***********************/
/****************************************************************************/
/* Shark 4 frame grabber DLL definitions 32 bit version * by ML *************/
/****************************************************************************/

#define	EXPAPI	__declspec( dllexport ) WINAPI

typedef BOOL (__stdcall *pfnS4INIT)(VOID);
typedef VOID (__stdcall *pfnS4CLOSE)(VOID);
typedef VOID (__stdcall *pfnS4GETREALRESOLUTION)(LPINT, LPINT);
typedef UINT (__stdcall *pfnS4START)(VOID);
typedef VOID (__stdcall *pfnS4STOP)(VOID);
typedef UINT (__stdcall *pfnS4GETSTATUS)(VOID);
typedef BOOL (__stdcall *pfnS4LIVEBUFFERREADY)(VOID);
typedef BOOL (__stdcall *pfnS4SELECTLIVEBUFFER)(BOOL);
typedef BOOL (__stdcall *pfnS4HOOKLIVEBUFFER)( LPVOID lpBuff, UINT dwBuffWidth,
							 LPRECT lpr, UINT wFmt, DWORD dwSecondHalf );
typedef UINT (__stdcall *pfnS4GETPARAMETER)( UINT wParam );
typedef BOOL (__stdcall *pfnS4SETCLIPPING)( int x0, int y0, int x1, int y1 );
typedef BOOL (__stdcall *pfnS4SETRESOLUTION)( int x, int y );
typedef BOOL (__stdcall *pfnS4SETPARAMETER)( UINT wParam, UINT wVal );
typedef VOID (__stdcall *pfnS4SETSOURCETYPE)( UINT wType );
typedef VOID (__stdcall *pfnS4SETINPUT)( UINT wInput );
typedef UINT (__stdcall *pfnS4GETINPUT)( VOID );

extern pfnS4INIT g_pfnS4INIT;
extern pfnS4CLOSE g_pfnS4CLOSE;
extern pfnS4GETREALRESOLUTION g_pfnS4GETREALRESOLUTION;
extern pfnS4START g_pfnS4START;
extern pfnS4STOP g_pfnS4STOP;
extern pfnS4GETSTATUS g_pfnS4GETSTATUS;
extern pfnS4LIVEBUFFERREADY g_pfnS4LIVEBUFFERREADY;
extern pfnS4SELECTLIVEBUFFER g_pfnS4SELECTLIVEBUFFER;
extern pfnS4HOOKLIVEBUFFER g_pfnS4HOOKLIVEBUFFER;
extern pfnS4GETPARAMETER g_pfnS4GETPARAMETER;
extern pfnS4SETCLIPPING g_pfnS4SETCLIPPING;
extern pfnS4SETRESOLUTION g_pfnS4SETRESOLUTION;
extern pfnS4SETPARAMETER g_pfnS4SETPARAMETER;
extern pfnS4SETSOURCETYPE g_pfnS4SETSOURCETYPE;
extern pfnS4SETINPUT g_pfnS4SETINPUT;
extern pfnS4GETINPUT g_pfnS4GETINPUT;

#ifdef __cplusplus
extern "C" {
#endif

	// Initialization of the hardware
	// Function returns FALSE, if SHARK4 could not be detected.
BOOL EXPAPI	s4Init(VOID);

	// Initialization of the hardware
UINT EXPAPI	s4InitEx(VOID);
	// Function returns one of the following error codes:
#define	SH4_ERR_NOBOARD			1	// No Shark4 boards found, or PCI BIOS not present
#define	SH4_ERR_UNKNOWNBOARD	2	// Unknown model of SHark4 found, update software
#define	SH4_ERR_IOCONFLICT		3	// Settings from Shark432.INI causes a conflict on I/O address
#define	SH4_ERR_MEMTEST			4	// Shark4 onboard memory test failed
#define	SH4_ERR_VXDVERSION		5	// VXD/KMD driver version is wrong
#define	SH4_ERR_IOWRONG			6	// I/O settings from Shark432.INI are invalid (e.g. >0x400)
													   
	// Freeing of the all SHARK4.DLL internal resources
VOID EXPAPI	s4Close(VOID);

	// Function returns the device model
BYTE EXPAPI	s4GetModel(VOID);

	// Function returns the device config address
DWORD EXPAPI s4GetConfigAddress(VOID);

	// Function returns the device IO base adress
WORD EXPAPI s4GetIOAddress( VOID );

	// Function returns the size of onboard memory (either 4 or 8 MB)
DWORD EXPAPI s4GetMemorySize( VOID );

	// Returns TRUE if KX chipset has been found and SERR and/or PERR bits are set
BOOL EXPAPI	s4GetParityCheck( VOID );

	// Hooks a device for displaying of the live image
	// Parameter 'lpd' of this function must point to the structure filled by vg2Init or vg1Init
	// If 'lpd' is NULL then there is no live display, but frame buffer will be filled with digitized data on s4Start();
	// Function returns TRUE in case the specified structure and device is valid
BOOL EXPAPI s4SetHook( LPPCIDEV lpd );

	// Hooks a memory buffer for live image transfer
BOOL EXPAPI s4HookLiveBuffer( LPVOID lpBuff, UINT dwBuffWidth, LPRECT lpr, UINT wFmt, DWORD dwSecondHalf );

	// Selects first (FALSE) or second (TRUE) half for writing
BOOL EXPAPI s4SelectLiveBuffer( BOOL bHalf );

	// Enables writes to the previosly hooked live buffer
BOOL EXPAPI s4LiveBufferReady( VOID );

	// Set zoom for image acquisition, available numbers are 1,2,3 and 4
BOOL EXPAPI	s4SetZoom( UINT wZoom );

	// Returns current zoom value
UINT EXPAPI	s4GetZoom( VOID );

	// Set zoom for image acquisition (1...256),
	//  actual downscaling will be wZoomEx/256
BOOL EXPAPI	s4SetZoomEx( UINT wZoomEx );

	// Returns current wZoomEx value
UINT EXPAPI	s4GetZoomEx( VOID );

	// Function starts an image acquisition
UINT EXPAPI	s4Start( VOID );

	// Function stops an image acquisition
	// Acquisition of the current frame will be finished
	// To find out when acquisition is finished use s4GetStatus, and test for S4_S_GDONE and S4_S_DDONE
	// Another way is to use s4WaitFlags( S4_S_GDONE | S4_S_DDONE );
VOID EXPAPI s4Stop( VOID );

	// Function stops an image acquisition immediatly
VOID EXPAPI s4Abrupt( VOID );

	// Temporary suspends image update in Live mode.
VOID EXPAPI s4SuspendLive( BOOL bDoHWMutex );

	// Resumes previously suspended update of the Live image.
VOID EXPAPI s4ResumeLive( BOOL bDoHWMutex );

	// Function sets active input in range 1..20
	// 1 - CINCH#1
	// 2 - CINCH#2
	// 3 - SVHS#1
	// 4 - SVHS#2
	// 5 - RGB/DB9
	// 6 - DIGITAL
	// 7 - Serial (ADDA link)
	// 8 - D32x4
	// 9 - Digital LVDS link
	// 11..22 serialized CVBS/Mono
VOID EXPAPI	s4SetInput(UINT wInput);

	// Same as above, also sets brightness, contrast and saturation
VOID EXPAPI	s4SetInputBCS(UINT wInput, UINT uBrig, UINT uCont, UINT uSatn);

	// Function sets active input in range 1..22
	// Also, if wNext is >0 it specifies the input which will be used next.
	// This allows to shortcut the time for switching inputs.
	// bAbrupt parameter tells whether grabber have to "shut up" (TRUE)
	//  or just wait for the ready condition.
VOID EXPAPI	s4SetInputEx(UINT wInput, UINT wNext, BOOL bAbrupt);

	// Same as above, also sets brightness, contrast and saturation
VOID EXPAPI	s4SetInputExBCS(UINT wInput, UINT wNext, BOOL bAbrupt, UINT uBrig, UINT uCont, UINT uSatn);

	// Returns active input No
UINT EXPAPI s4GetInput( VOID );

	// Function sets type of the source signal
VOID EXPAPI	s4SetSourceType(UINT wType);
#define	SH4_CCD_COMPOSITE	0			// Composite source signal (CVBS)
#define	SH4_CCD_MONO		1			// Monochrome signal
#define	SH4_CCD_SVIDEO		2			// S-Video source signal (Hi-8 or SVHS)

	// Returns current source type
UINT EXPAPI	s4GetSourceType( VOID );

	// Parameters
#define	SH4_P_TBC		1		// Turn Time Base Correction ON (1) or OFF (0) 
#define	SH4_P_ADEPTH	2		// Set the averaging/integration depth: 1,2,4,8,16,32,64,128
#define	SH4_P_AMODE		3		// Set the mode to (0) OFF, (1) averaging, (2) integration
#define	SH4_P_AUFD		4		// Enables (1) or disables (0) automatic field detection
#define	SH4_P_FSEL		5		// Set 50Hz (0) or 60Hz (1) TV system
#define	SH4_P_VNOI		6		// Set Vnoise to Normal (0), Search(1), Auto(2), OFF(3)
#define	SH4_P_HPLL		7		// Turn HPLL OFF(1) or ON(0)
#define	SH4_P_SHIFT50	8		// Shift image horizontaly in 50Hz mode
#define	SH4_P_SHIFT60	9		// Shift image horizontaly in 60Hz mode
#define	SH4_P_CTRAP		10		// Color trap filter on (1) off (0)
#define	SH4_P_FCOLOR	11		// Force color on (1) normal (0)
#define	SH4_P_BKGUSED	12		// Use Background plane
#define	SH4_P_ASYNDMA	13		// Use asynchonous DMA transfer
#define	SH4_P_FDELAY	14		// Frame delay counter
#define	SH4_P_ETRIGA	15		// Enable external triggering on the first SAA7110 
#define	SH4_P_ETRIGB	16		// Enable external triggering on the second SAA7110 

#define	SH4_P_BRIG		20		// Brightness 0..255 (CCIR=139)
#define	SH4_P_CONT		21		// Contrast -2.0(-128)..-1.0(-64)..0(0)..1.0(64)..2.0(127), (CCIR=64)
#define	SH4_P_SATN		22		// Saturation -2.0(-128)..-1.0(-64)..0(0)..1.0(64)..2.0(127), (CCIR=64)
#define	SH4_P_GAIN		23		// Gain -2.8dB(0)..+9.15dB(255), (-1: auto)
#define	SH4_P_OFFS		24		// Offset 1..254 (CCIR=64) (50..120)
#define	SH4_P_FULLGO	25		// Use full range of Gain and Offset, does not work for CVBS and SVHS inputs
#define	SH4_P_PROSCAN8	26		// Shift input data by 6 bits. Used with Proscan 8bit mode.

#define	SH4_P_PA		30		// Read/write port A on 'I/O expander'
#define	SH4_P_PB		31		// Read/write port B on 'I/O expander'
#define	SH4_P_PC		32		// Read/write port C on 'I/O expander'
#define	SH4_P_PD		33		// Read/write port D on 'I/O expander'
#define	SH4_P_PK		34		// Read/write port K on adpater 'S4_422MP' version X1.2
#define	SH4_P_GPI		35		// Read port GPI on adpater 'S4_422MP'
#define	SH4_P_GPO1		36		// Write port GPO1 on adpater 'S4_422MP'
#define	SH4_P_GPO2		37		// Write port GPO2 on adpater 'S4_422MP'
#define	SH4_P_EXPOSURE	38		// Write port Exposure on adpater 'S4_422MP'
#define	SH4_P_KODAKTRIG	39		// 'Trigger mode' of the Kodak 1.6i
#define	SH4_P_EXPTIME	40		// Duration of the exposure signal in 'ms', 
								// used for Photonics "Normal" and Kodak's 'Control mode'
#define	SH4_P_EXPDELAY	41		// Delay between high to low transition of the Exposure to the moment
								// when Shark starts to poll the FRE input signal

#define	SH4_P_CLIPSC	42		// Turn clipping of 32 system colors On/Off (default is off)

#define	SH4_P_SWAPFLD	43		// Swaps fileds in one frame

#define	SH4_P_XOFFS		44		// Offsets the beginning of the acquired video, must be a multiple of 8

#define	SH4_P_PARITYCHECK 45	// Activates or deactivates ParityCheck in PCI chip set, also modifies respective item in .INI file


	// Set the Shark parameter wParam to the wVal value
BOOL EXPAPI s4SetParameter( UINT wParam, UINT wVal );

	// Returns the current value of the parameter
UINT EXPAPI s4GetParameter( UINT wParam );

	// Function returns FALSE, if a camera signal could not be detected
BOOL EXPAPI	s4GetGenLockStatus( VOID );

	// Returns current field number
BOOL EXPAPI	s4GetField( VOID );

	// Function returns the stautus of the SHARK4 board
UINT EXPAPI	s4GetStatus( VOID );

#define	SH4_S_COLOR		0x01	// Color detected. Set if Color signal detected.
#define	SH4_S_60		0x02	// System type. Set if 60Hz signal detected, reset if 50Hz.
#define	SH4_S_LOCK		0x04	// PLL locked. Set if locked.
#define SH4_S_GDONE		0x10	// Grabber cycle finished
#define	SH4_S_DDONE		0x20	// DMA cycle finished
#define	SH4_S_LBHALF	0x40	// which half of LB is used
#define	SH4_S_LBBUSY	0x80	// filling of LB is not finished yet

	// Function waits for specified flag to appear
BOOL EXPAPI	s4WaitFlags( UINT wFlags );

	// Functions fills supplied memory buffer with digitized data.
	// The wBuffWidth parameter specifies buffer width in BYTES!
	// The lprcRect parameter specifies the source rect in grabber frame buffer
	// The wFmt parameter specifies format of the data in destination buffer:
#define	S4_SF_R8	0		// Destination buffer will receive raw 8bpp data 
#define	S4_SF_R16	1		// Destination buffer will receive raw 16bpp data, 
							// it represent either 16bpp mono (in case of averaging)
							// or it is YU (YV) pairs (if averaging is turned off)
#define	S4_SF_HiC	2		// Destination buffer will receive 16bpp (5R,6G,5B) data,
							// obtained as a result of conversion of (YU.YV) -> RGB
#define	S4_SF_HiM	3		// Same as above, but U and V are set to 0 (i.e. Y0.Y0)
#define	S4_SF_R32	4		// Destination buffer will receive raw 32bpp data, 
							// it represent 32bpp color image from 3-CCD camera

	// Returns FALSE in case of errors.
BOOL EXPAPI	s4Snap( LPVOID lpBuff, UINT wBuffWidth, LPRECT lprcRect, UINT wFmt);

	// Functions fills supplied memory buffer with digitized data.
	// The wFmt parameter specifies format of the data in destination buffer:
	// See s4Snap();
	// Returns FALSE in case of errors.
BOOL EXPAPI	s4GetLine(LPVOID lpBuff, int y, int pcnt, UINT wFmt );

	// Moves data from lpBuff to the acquisition buffer
	// Data are 16bit words pre pixel
	// Returns FALSE in case of errors.
BOOL EXPAPI	s4SetLine(LPVOID lpBuff, int y, int pcnt );

	// Moves data from lpBuff to the background buffer
	// Data are 16bit words pre pixel, where:
	//	12MSB are intensity data and will be subtracted from the output value of ADC;
	//	4LSB are control bits
	// Returns FALSE in case of errors.
BOOL EXPAPI	s4SetBkgLine(LPVOID lpBuff, int y, int pcnt );

	// Converts YU.YV data to 24bits RGB
VOID EXPAPI s4YUV2RGB( LPVOID lpDst, LPVOID lpSrc, UINT cnt);
	// Converts YU.YV data to 24bits BGR
VOID EXPAPI s4YUV2BGR( LPVOID lpDst, LPVOID lpSrc, UINT cnt);

	// Converts YU.YV data to 32bits RGB
VOID EXPAPI s4YUV2RGB32( LPVOID lpDst, LPVOID lpSrc, UINT cnt);
	// Converts YU.YV data to 32bits BGR
VOID EXPAPI s4YUV2BGR32( LPVOID lpDst, LPVOID lpSrc, UINT cnt);

	// Sets resolution (works only for input #6)
BOOL EXPAPI	s4SetResolution( int x, int y );
	// Returns current device resolution
VOID EXPAPI	s4GetResolution( LPINT x, LPINT y );
	// Returns current resolution (including clipping and zoom)
VOID EXPAPI	s4GetRealResolution( LPINT x, LPINT y );

	// Sets resolution together with external buffer.
BOOL EXPAPI	s4SetResolutionExBuff( int x, int y, LPVOID lpBuff, UINT wFmt );

	// Sets resolution together with external buffer and contignous flag
BOOL EXPAPI	s4SetResolutionExBuffEx( int x, int y, LPVOID lpBuff, UINT wFmt, BOOL bContig );

	// Sets current resolution
BOOL EXPAPI	s4SetClipping( int x0, int y0, int x1, int y1 );

	// Sets current resolution
VOID EXPAPI	s4GetClipping( LPINT x0, LPINT y0, LPINT x1, LPINT y1 );

	// returns frame rate
DWORD EXPAPI s4GetFrameRate( VOID );

	// Sets clipping for live mode both for primary and secondary VGA
	// lpr->left must be aligned on a 4 pixel boundary
	// Number of pixels, number of lines, starting line number are without restrictions.
	// If lpr==NULL, there is no clipping, and image will be displaed from 0,0 up to the limit.
	// iXoffset and iYoffset specifies image shift (scroll) in pixels/lines relative to its origin 
	// iXoffset must be a multiple of 4
VOID EXPAPI s4SetLiveView( LPRECT lpr, int iXoffset, int iYoffset );

	// retrives whether live clipping is active, respective rectangle and offset parameters
BOOL EXPAPI s4GetLiveView( LPRECT lpr, LPINT lpXoffset, LPINT lpYoffset );

/************************************************************************************/

	// Set offsets for R,G and B channels
VOID EXPAPI s4SetRGBOffset(int iOffsetR, int iOffsetG, int iOffsetB );

	// Retrieves offsets of R,G and B channels
VOID EXPAPI s4GetRGBOffset(LPINT iOffsetR, LPINT iOffsetG, LPINT iOffsetB );

	// Set gain for R,G and B channels
VOID EXPAPI s4SetRGBGain(int iGainR, int iGainG, int iGainB );

	// Retrieves gain of R,G and B channels
VOID EXPAPI s4GetRGBGain(LPINT iGainR, LPINT iGainG, LPINT iGainB );

	// Set dithering On/Off for live image display in HiColor modes (15bpp and 16bpp)
VOID EXPAPI s4SetRGBDither( BOOL bUseDithering );

	// Retrives the state of dithering option 
BOOL EXPAPI s4GetRGBDither( VOID );

	// Turning RGB guns ON or OFF 
VOID EXPAPI s4SetRGBGun( BOOL bR, BOOL bG, BOOL bB );

	// Retriving the current state of guns ON/OFF option
VOID EXPAPI s4GetRGBGun(LPINT bR, LPINT bG, LPINT bB );

/************************************************************************************/

	// Function initializes primary VGA board, to be ready for window hooking by s4HookLiveWindow
	// Returns an error code
	//	0 - OK
#define	VG1_ERR_BADPRIMARY	1			//	Bad Primary VGA card, or Primary VGA driver does not use linear window
#define	VG1_ERR_DCIHDC		2			//	DCI reports an error in attempt to create an hDC
#define	VG1_ERR_LINWINDOW	3			//	Primary VGA driver does not use linear window, i.e. bad driver
#define	VG1_ERR_DDCLIPPER	4			//	DDRAW can't create clipper
#define	VG1_ERR_DDDDRAW		5			//	DDRAW can't create ddraw
#define	VG1_ERR_DDSETCOOP	6			//	DDRAW can't set cooperative level
#define	VG1_ERR_DDSURFACE	7			//	DDRAW can't create surface
#define	VG1_ERR_DCIBEGINACCESS	8		//  BeginAccess fails
#define	VG1_ERR_DCISURFACE		9		//  DCI Failed to create primary surface
#define	VG1_ERR_DCIPROVIDER		10		//  DCI open provider fails
#define	VG1_ERR_DDMONITORFROMP	11		//  MonitoFromPoint fails
#define	VG1_ERR_DDSURFACELOCK	12		//  DDRAW surface lock fails
#define	VG1_ERR_DDHDC			13		//  Get HDC fails
#define	VG1_ERR_BADDATA			14		//  Bad/inconsistent data obtained from DCI
#define	VG1_ERR_DDNOHARDWARE	15		//  DDCAPS_NOHARDWARE set in DD object caps

int EXPAPI	vg1Init( LPPCIDEV lpd );

/************************************************************************************/
	// Retrieving Class Code and Revision ID for specified PCI device
	// Class code is in 24MSB and Rev is in 8LSB
DWORD EXPAPI s4GetCCRev( LPPCIDEV lpd );

/************************************************************************************/

	// Sends a control DWORD via serail link
BOOL EXPAPI s4Tx_CTRL( DWORD d );

	// Sends a data DWORD via serail link
BOOL EXPAPI s4Tx_DATA ( DWORD d );

	// Starts accumulation of DWORDs sent via s4Tx_xxxx() in memory
	// no actual send happens at that time
BOOL EXPAPI s4Tx_BlockStart( UINT cnt );

	// Flashes (sends to ADDA box) all the data accumulated since last
	// call to s4Tx_BlockStart() and frees internal buffer
BOOL EXPAPI s4Tx_BlockFlash( void );

	// Receives a DWORD from serial link with defined timout.
	// If lpdwData is NULL, then function flashes data from input register.
BOOL EXPAPI s4Rx_RECV_DWORD( LPDWORD lpdwData, int timeout);

	// Receives a WORD from serial link with defined timout.
	// If lpwData is NULL, then function flashes data from input register.
BOOL EXPAPI s4Rx_RECV_WORD( LPWORD lpwData, int timeout);

	// Downloads device via serail link
int EXPAPI s4Tx_Download( LPVOID lpBits );

	// Inits shark4 into Xmitter mode, all other functions are
	// unavailable until s4ResetXmit() is called
BOOL EXPAPI	s4InitXmit( VOID );

	// Resets shark4 into normal operation mode
BOOL EXPAPI	s4ResetXmit( VOID );

	// Returns the buffers size (in pixels) for Xmitter mode
UINT EXPAPI s4GetOutBufferSize( VOID );

	// Writes respective number of pixels into respective buffer
BOOL EXPAPI s4WriteOutBuffer(UINT uiBuffNumber, LPVOID lpData, UINT uiCount, BOOL bLast);

	// Activates Xmitter
BOOL EXPAPI	s4StartXmit( VOID );

	// Deactivates Xmitter
BOOL EXPAPI s4StopXmit( VOID );

	// Returns TRUE if respective buffer is ready to receive data
BOOL EXPAPI s4IsOutHalfReady( UINT uiBuffNumber );

	// Returns TRUE is Xmitter finished with sending all the data
BOOL EXPAPI s4IsXmitDone( VOID );

#define	s4XMTDATA(a,b)	((DWORD)((((DWORD)(((DWORD)(a)>>4)&0xFFF))<<20) | (((DWORD)(((DWORD)(b)>>4)&0xFFF))<<8)))

LPBYTE EXPAPI phyCreateAlias( DWORD phys, DWORD siz );


#define	NBOARDS		4	// Maximum number of boards supported

	// Select board nNewBoard as active one, nNewBoard is 0 based index.
	// Returns FALSE in case if nNewBoard exceeds the number of available boards.
BOOL EXPAPI s4SelectBoard( UINT nNewBoard );

	// Returns the number of currently active board.
UINT EXPAPI s4GetBoard( VOID );

	// Returns the number of boards detected.
UINT EXPAPI s4GetNumberOfBoards( VOID );

	// Assigns lpBuff as an external buffer for image acquisition
	// At present, supports only D32X4 input (8) and 'S4_SF_R32' buffer format
BOOL EXPAPI s4SetExternalBuffer( LPVOID lpBuff, UINT wFmt );

	// Returns adapter ID (if any)
	// If there is now adapter found function returns 0
UINT EXPAPI s4GetAdapterID( void );

#define	S4_AID_LVDS		8		// LVDS adapter
#define	S4_AID_MP422	9		// MP422 adapter
#define	S4_AID_D32X4	10		// D32X4 adapter

	// Returns versions numbers:
	//  expected by DLL in lpdwDLL
	//  and obtained from KMD in lpdwKMD
	// returns FLASE if either parameters are invalid or DeviceIoControl fails
BOOL EXPAPI s4GetKMDVersions( DWORD * lpdwDLL, DWORD * lpdwKMD );

	// Returns last error code (see below)
DWORD EXPAPI s4GetLastError( void );

#define	S4_OK							0
#define	S4ERR_INVALID_VXD_HANDLE		1
#define	S4ERR_INVALID_VXD_VERSION		2
#define	S4ERR_DDS_ALREADY_INUSE			3
#define	S4ERR_KMD_FAILS					4
#define	S4ERR_VXD_FAILS					5

#ifdef __cplusplus
}
#endif
