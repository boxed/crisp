#ifndef __PIXFILT_H
#define __PIXFILT_H

#ifdef PIXFILT_EXPORTS
#define PIXFILT_API __declspec(dllexport)
#else
#define PIXFILT_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif

PIXFILT_API void fltCMYG2BGR          ( LPBYTE pSrc, int sX, int sY, int iSrcWidth, LPBYTE pDst,  int iDstWidth, double kR, double kG, double kB);
PIXFILT_API void fltCMYG2BW8          ( LPBYTE pSrc, int sX, int sY, int iSrcWidth, LPBYTE pDst, int iDstWidth, WORD vmax, WORD vmin);
PIXFILT_API void fltRGGB2BGRmult      ( LPBYTE pSrc, int sX, int sY, int iSrcWidth, LPBYTE pDst,  int iDstWidth, double kR, double kG, double kB);
PIXFILT_API void fltRGGB2BGR_9331_filt( LPWORD pSrc, int iWidth, int iHeight, int iSrcWidth, int iField1, LPBYTE pDst, int iDstWidth, int koef, double Kr, double Kg, double Kb, int kShift, BOOL bDoXLAT, int offR, int offG, int offB );
PIXFILT_API void fltRGGB2BGR_9331     ( LPWORD pSrc, int iWidth, int iHeight, int iSrcWidth, int iField1, LPBYTE pDst, int iDstWidth, double Kr, double Kg, double Kb );
PIXFILT_API void fltGetLutAddress	  ( LPBYTE * lpLut );

#ifdef __cplusplus
}
#endif

#endif /* __PIXFILT_H */
