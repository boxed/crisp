/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Xutil.ass routines * by ML ***********************************************/
/****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
extern	void Conv5x5( LPDOUBLE , LPDOUBLE);
extern	void Residual(LPRESID S, int ns, LPDOUBLE a);
extern	void X64_Copy(LPBYTE src, LPVOID dst, int dstw, int dsth,
					  int x_a, int y_a, int x_b, int y_b, int s_a0, int s_b0, int neg);
extern	void X256_Copy(LPBYTE src, LPVOID dst, int dstw, int dsth,
					  int x_a, int y_a, int x_b, int y_b, int s_a0, int s_b0, int neg);

extern	int		GetShape8 (LPVOID img, int imw, int imh, int neg, long min, float k, int x, int y, 
						 LPVOID mask, int xms, int yms);
extern	int		GetShape16(LPVOID img, int imw, int imh, int neg, long min, float k, int x, int y, 
						 LPVOID mask, int xms, int yms);
/*---------------------------------------------------------*/
extern	int		AddFPix8   (LPVOID fmap, int imw, int	imh, int x, int y, float pix);
extern	void	FMapToImg  (LPVOID fmap, LPVOID img, int imw, int imh, int neg, int min, int max);
extern	void	FMapToImg16(LPVOID fmap, LPVOID img, int imw, int imh, int neg, DWORD min, DWORD max);
extern	float	AGauss      (float s, float x);
//extern	int		GetImgMinMax(LPVOID img, int imw, int imh, LPINT min, LPINT max);
extern double	fpCalcDistance(int x0, int y0, int x1, int y1);
extern	int		GetRDF8	    (LPVOID img, int imw, int imh, int neg, int x, int y, int rad, LPDOUBLE lpRDF, LPLONG lpWGT);
extern	int		GetRDFangl8 (LPVOID img, int imw, int imh, int neg, int x, int y, int rad, LPDOUBLE lpRDF, LPLONG lpWGT, float a0, float a1);
extern	int		GetRDF16    (LPVOID img, int imw, int imh, int neg, int x, int y, int rad, LPDOUBLE lpRDF, LPLONG lpWGT);
extern	int		GetRDFangl16(LPVOID img, int imw, int imh, int neg, int x, int y, int rad, LPDOUBLE lpRDF, LPLONG lpWGT, float a0, float a1);

extern	void	EstTP_BG( LPVOID fft, int xs, int rad, float * tp);
extern	void	Init_Flt(LPVOID flt, int xs, int dd);

#ifdef __cplusplus
}
#endif
