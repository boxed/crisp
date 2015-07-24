/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Some usefull CODE32 routines * by ML *************************************/
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

// extern WORD	im8GetFrameDC( LPBYTE src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid);
// extern WORD	im8CountFrameDC( LPBYTE src, LPBYTE msk, DWORD cnt, DWORD maskval);
// extern VOID	im8MulMove   ( LPBYTE img, LPBYTE msk, LONG stride, LONG x, LONG y, BYTE imdc);
// extern VOID	im8MaskCopy  ( LPBYTE dst, LPBYTE src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc);
// extern VOID	im8MaskCut   ( LPBYTE dst, LPWORD msk, DWORD imdc);
extern VOID	im8Xlat      ( LPBYTE dst, LPBYTE src, DWORD soff, LPBYTE ylat, DWORD width, DWORD height);
extern VOID	im8Hi15      ( LPBYTE dst, LPBYTE src, DWORD soff, DWORD width, DWORD height);
extern VOID	im8Hi16      ( LPBYTE dst, LPBYTE src, DWORD soff, DWORD width, DWORD height);
extern VOID	im8Hi24      ( LPBYTE dst, LPBYTE src, DWORD soff, DWORD width, DWORD height);
extern VOID	im8Hi32      ( LPBYTE dst, LPBYTE src, DWORD soff, DWORD width, DWORD height);

//extern WORD	im16GetFrameDC( LPWORD src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid);
// extern WORD	im16CountFrameDC( LPWORD src, LPBYTE msk, DWORD cnt, DWORD maskval);
// extern VOID	im16MulMove   ( LPWORD img, LPBYTE msk, LONG stride, LONG x, LONG y, WORD imdc);
// extern VOID	im16MaskCopy  ( LPWORD dst, LPWORD src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc);
// extern VOID	im16MaskCut   ( LPWORD dst, LPWORD msk, DWORD imdc);
extern VOID	im16Xlat      ( LPWORD dst, LPWORD src, DWORD soff, LPWORD ylat, DWORD width, DWORD height);

// extern WORD im32GetFrameDC(LPDWORD src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid);
// extern WORD	im32CountFrameDC(LPDWORD src, LPBYTE msk, DWORD cnt, DWORD maskval);
// extern VOID	im32MaskCopy  ( LPDWORD dst, LPDWORD src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc);
// extern VOID	im32MaskCut   ( LPDWORD dst, LPWORD msk, DWORD imdc);
// extern VOID	im32MulMove   ( LPDWORD img, LPBYTE msk, LONG stride, LONG x, LONG y, DWORD imdc);

// extern WORD imFGetFrameDC(LPFLOAT src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid);
// extern WORD	imFCountFrameDC(LPFLOAT src, LPBYTE msk, DWORD cnt, DWORD maskval);
// extern VOID	imFMaskCopy  ( LPFLOAT dst, LPFLOAT src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc);
// extern VOID	imFMaskCut   ( LPFLOAT dst, LPWORD msk, DWORD imdc);
// extern VOID	imFMulMove   ( FLOAT *img, LPBYTE msk, LONG stride, LONG x, LONG y, FLOAT imdc);

// extern WORD imDGetFrameDC(LPDOUBLE src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid);
// extern WORD	imDCountFrameDC(LPDOUBLE src, LPBYTE msk, DWORD cnt, DWORD maskval);
// extern VOID	imDMaskCopy  ( LPDOUBLE dst, LPDOUBLE src, LONG soff, LPWORD msk, DWORD iwid, DWORD fwid, WORD imdc);
// extern VOID	imDMaskCut   ( LPDOUBLE dst, LPWORD msk, DWORD imdc);
// extern VOID	imDMulMove   ( DOUBLE *img, LPBYTE msk, LONG stride, LONG x, LONG y, DOUBLE imdc);

// extern int	im8GetHisto	  (LPLONG hist, LPBYTE src, DWORD bufwidth, DWORD x0, DWORD y0, DWORD xs, DWORD ys, BOOL bSmooth);
// extern int	im16GetHisto  (LPLONG hist, LPWORD src, DWORD bufwidth, DWORD x0, DWORD y0, DWORD xs, DWORD ys, BOOL bSmooth);

// extern int	im8GetRMS   (LPWORD src, DWORD bufwidth, DWORD x0, DWORD y0, DWORD xs, DWORD ys, DWORD mean, LPDOUBLE rms, LPDOUBLE adev);
// extern int	im16GetRMS  (LPWORD src, DWORD bufwidth, DWORD x0, DWORD y0, DWORD xs, DWORD ys, DWORD mean, LPDOUBLE rms, LPDOUBLE adev);

extern VOID im8CreateCtrMap( LPVOID lpimg, LPVOID lptmp, int xs, int ys, int steps );


extern VOID im8GetProfile  (LPBYTE src, int stride, int width, int height, int neg, 
						    double x0, double y0, double x1,double y1, int cnt, 
						    LPDOUBLE dst, LPLONG wgt);

extern VOID im8sGetProfile  (LPSTR src, int stride, int width, int height, int neg, 
							double x0, double y0, double x1,double y1, int cnt, 
							LPDOUBLE dst, LPLONG wgt);

extern  VOID im16GetProfile(LPWORD src, int stride, int width, int height, int neg,
							double x0, double y0, double x1,double y1, int cnt,
							LPDOUBLE dst, LPLONG wgt);

extern  VOID im16sGetProfile(SHORT *src, int stride, int width, int height, int neg,
							double x0, double y0, double x1,double y1, int cnt,
							LPDOUBLE dst, LPLONG wgt);

extern  VOID im32GetProfile(LPDWORD src, int stride, int width, int height, int neg,
							double x0, double y0, double x1, double y1, int cnt,
							LPDOUBLE dst, LPLONG wgt);

extern  VOID im32sGetProfile(LONG *src, int stride, int width, int height, int neg,
							double x0, double y0, double x1, double y1, int cnt,
							LPDOUBLE dst, LPLONG wgt);

// neg == 0 then positive, neg != 0 then negative
extern  VOID imFGetProfile(LPFLOAT src, int stride, int width, int height, int neg,
						   double x0, double y0, double x1, double y1, int cnt,
						   LPDOUBLE dst, LPLONG wgt);

// neg == 0 then positive, neg != 0 then negative
extern  VOID imDGetProfile(LPDOUBLE src, int stride, int width, int height, int neg,
						   double x0, double y0, double x1, double y1, int cnt,
						   LPDOUBLE dst, LPLONG wgt);

extern VOID im8Descale(LPBYTE dst, LPDOUBLE src, LPLONG wgt, int cnt );

extern	VOID FFTD1R (LPVOID data, int ofs, int dim, int stp);
extern	VOID FFTD1  (LPVOID data, int ofs, int dim, int stp);
extern	VOID IFFTD1 (LPVOID data, int ofs, int dim, int stp);
extern	VOID IFFTD1R(LPVOID data, int ofs, int dim, int stp);
#ifdef __cplusplus
}
#endif

extern int	round	( double a );
