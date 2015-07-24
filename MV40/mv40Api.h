 
#ifndef __MV40API_H
#define __MV40API_H

#ifdef MV40API_EXPORTS
#define MV40_API __declspec(dllexport)
#else
#define MV40_API __declspec(dllimport)
#endif

// winbase.h has IN and OUT but not both
#ifndef INOUT
#define INOUT
#endif

//
typedef int MV40_RETURN;
typedef unsigned long U32, *PU32;
typedef UCHAR U8, *PU8;

// Eror codes
#define MV40_OK 			0
#define MV40_INVALID_HANDLE	1
#define MV40_READREG		2
#define MV40_WRITEREG		3
#define	MV40_FREE_RESOURCES	4
#define	MV40_FREE_CHANNEL	5
#define	MV40_FREE_BANDWIDTH	6
#define	MV40_READBLK		7
#define	MV40_WRITEBLK		8
#define	MV40_NO_IMAGE		9
#define	MV40_TIMEOUT		10
#define	MV40_INVALID_ARG	11


#define	szMV40_Name		"SoftHard Technology MV4.0 Camera"


#ifdef __cplusplus
extern "C" {
#endif

// Get Number of devices with same friendly name
MV40_API MV40_RETURN mv40GetNumberDevices ( 
			IN LPCSTR pFriendlyName,			// Device name
			OUT LPDWORD pNumberDevices);		// Ptr to number


// Get Device - returns camera unique number
MV40_API MV40_RETURN mv40GetDevice ( 
			IN  DWORD nIndex,					// Index
			OUT LPDWORD pChipId);				// Ptr to chip ID

// Initialize
MV40_API MV40_RETURN mv40Initialize(
			IN LPCSTR pFriendlyName,			// Device friendly name 	
			IN DWORD lChipId,					// chip ID
			OUT PHANDLE drvHandle);				// Returned driver handle

MV40_API MV40_RETURN mv40IsDeviceExist			(IN HANDLE hDevice);

// Uninitialize
MV40_API MV40_RETURN mv40Uninitialize			(IN HANDLE hDevice);

VOID                 mv40Abandon				(VOID);

MV40_API MV40_RETURN mv40StartImageAcquisition	(IN HANDLE hDevice);
MV40_API MV40_RETURN mv40AcquireImage			(IN HANDLE hDevice, OUT PVOID *ppData );
MV40_API MV40_RETURN mv40StopImageAcquisition	(IN HANDLE hDevice);
MV40_API MV40_RETURN mv40IsImageReady			(IN HANDLE hDevice);
MV40_API MV40_RETURN mv40WaitForImage			(IN HANDLE hDevice, IN  DWORD dwTimeOut );

MV40_API MV40_RETURN mv40StartImageCapture		(IN HANDLE hDevice, OUT PVOID *ppData );
MV40_API MV40_RETURN mv40CaptureImage			(IN HANDLE hDevice, OUT PVOID *ppData );
MV40_API MV40_RETURN mv40StopImageCapture		(IN HANDLE hDevice);

MV40_API MV40_RETURN mv40GetModeCount			(IN HANDLE hDevice, OUT LPDWORD lpdwModeCount );
MV40_API MV40_RETURN mv40SetMode				(IN HANDLE hDevice, IN  DWORD dwMode);
MV40_API MV40_RETURN mv40SetModeEx				(IN HANDLE hDevice, IN  DWORD dwMode, IN DWORD dwX0, IN DWORD dwY0, IN DWORD dwCX, IN DWORD dwCY);
MV40_API MV40_RETURN mv40GetMode				(IN HANDLE hDevice, OUT LPDWORD lpdwMode );
MV40_API MV40_RETURN mv40SetExposure			(IN HANDLE hDevice, IN  DWORD dwExposure);
MV40_API MV40_RETURN mv40GetExposure			(IN HANDLE hDevice, OUT LPDWORD lpdwExposure );
MV40_API MV40_RETURN mv40SetTemperature			(IN HANDLE hDevice, IN  DWORD dwTemp );
MV40_API MV40_RETURN mv40GetTemperature			(IN HANDLE hDevice, OUT LPDWORD lpdwTemp );
MV40_API MV40_RETURN mv40GetCurrTemp			(IN HANDLE hDevice, OUT int * lpTChip, OUT int * lpTHousing );
MV40_API MV40_RETURN mv40SetOffset				(IN HANDLE hDevice, IN  DWORD dwOffset );
MV40_API MV40_RETURN mv40GetOffset				(IN HANDLE hDevice, OUT LPDWORD lpdwOffset );
MV40_API MV40_RETURN mv40SetGain				(IN HANDLE hDevice, IN  DWORD dwGain );
MV40_API MV40_RETURN mv40GetGain				(IN HANDLE hDevice, OUT LPDWORD lpdwGain );

MV40_API MV40_RETURN mv40GetSerialNumber		(IN HANDLE hDevice, OUT LPDWORD lpdwSern );
MV40_API MV40_RETURN mv40SetSerialNumber		(IN HANDLE hDevice, IN  DWORD dwSern );
MV40_API MV40_RETURN mv40GetModelName			(IN HANDLE hDevice, OUT LPDWORD lpdwModel );

MV40_API MV40_RETURN mv40GetHWN2				(IN HANDLE hDevice, OUT PDWORD pExten );
#define MV40_HWN2_CHIP(dw)		(((dw)>>20) & 0xFFF)
#define MV40_HWN2_MOSAIC(dw)	(((dw)>>16) & 0xF)
#define	MV40_HWN2_MOSAIC_BW		0x0
#define	MV40_HWN2_MOSAIC_RGB	0x1
#define	MV40_HWN2_MOSAIC_CMYG	0x2
#define MV40_HWN2_ADC(dw)		((dw) & 0xF)
#define MV40_HWN2_ADC_9224		0x1
#define MV40_HWN2_ADC_9844		0x2

MV40_API MV40_RETURN mv40GetModeExtension		(IN HANDLE hDevice, OUT PDWORD pExten );
#define	MV40_MODEXT_BWHBIN	0x00	// B/W mode of multicolor shift
#define	MV40_MODEXT_RGGB	0x01	// Color mode RG/GB mosaic filter (ICX252AK,etc)
#define	MV40_MODEXT_CMYG	0x02	// Color mode CM/YG mosaic filter (ICX252AQ,etc)
#define	MV40_MODEXT_MAX		3
#define	MV40_MODEXT_MASK	0x03

MV40_API MV40_RETURN mv40EnableTrigger			(IN HANDLE hDevice, IN BOOL bEnable, IN BOOL bNegative, IN BOOL bOutput );
MV40_API MV40_RETURN mv40SetTrigger				(IN HANDLE hDevice, IN DWORD dwMode, IN BOOL bNegative );
MV40_API MV40_RETURN mv40GetTrigger				(IN HANDLE hDevice, OUT DWORD *lpdwMode, OUT DWORD * lpbNegative );

#define	MV40_TRIG_OFF		0		// No trigger
#define	MV40_TRIG_IN_EDGE	1		// Input positive/negative edge starts exposure
#define	MV40_TRIG_IN_LEVEL	2		// Input positive/negative level defines exposure
#define	MV40_TRIG_OUT		3		// Output is high when exposure is active
#define	MV40_TRIG_MAX		4		// Maximum mode

MV40_API MV40_RETURN mv40PingTrigger			(IN HANDLE hDevice );

MV40_API BOOL mv40IsBufferOK					( LPVOID pBuf, LPDWORD pFrameNum );
//****************************************************************************

// Get Firmware Version
MV40_API MV40_RETURN mv40GetFWVersion( 
						IN HANDLE drvHandle, 			// driver handle
						OUT LPDWORD  pSoftwareVersion);	// Returned firmware version

// Get mv40api.dll Version
MV40_API MV40_RETURN mv40GetDLLVersion( 
						OUT LPDWORD pSoftwareVersion);	// Returned software version

// Get mv40dcam.sys version
MV40_API MV40_RETURN mv40GetSYSVersion( 
						IN HANDLE drvHandle, 			// driver handle
						OUT LPDWORD pSoftwareVersion);	// Returned software version

// Get camera resolution in human readable form
MV40_API MV40_RETURN mv40GetResolution( 
						IN HANDLE	hDevice,			// driver handle
						OUT PSIZE	pResolution,		// mode resolution
						OUT PDWORD	uBufwidth,			// line pitch in bytes
						OUT PDWORD	uF1offs,			// offset to the second field
						OUT PDWORD	uPixsize);			// bits per pixel

MV40_API MV40_RETURN mv40ReadQlet( 
						IN HANDLE	hDevice,
						IN DWORD	Offset,
						OUT LPDWORD	pData);

MV40_API MV40_RETURN mv40WriteQlet(
						IN HANDLE	hDevice,
						IN DWORD	Offset,
						IN DWORD	uData);

MV40_API MV40_RETURN mv40ReadBlock( 
						IN HANDLE	hDevice,
						IN DWORD	Offset,
						IN DWORD    Count,
						OUT PVOID	pData);

MV40_API MV40_RETURN mv40WriteBlock( 
						IN HANDLE	hDevice,
						IN DWORD	Offset,
						IN DWORD    Count,
						IN PVOID	pData);

MV40_API MV40_RETURN mv40ReadBlockRaw( 
						IN HANDLE	hDevice,
						IN WORD		OffsetHi,
						IN DWORD	OffsetLo,
						IN DWORD    Count,
						OUT PVOID	pData);

MV40_API MV40_RETURN mv40WriteBlockRaw( 
						IN HANDLE	hDevice,
						IN WORD		OffsetHi,
						IN DWORD	OffsetLo,
						IN DWORD    Count,
						IN PVOID	pData);

#ifdef __cplusplus
}
#endif

#endif /* __MV40API_H */
