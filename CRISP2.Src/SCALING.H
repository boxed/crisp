/****************************************************************************/
/* Copyright© 1994 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Image scaling functions definitions * by ML ******************************/
/****************************************************************************/

// RGB image scaling
BOOL	sc3Init( int iXdst, int iYdst, int iXsrc, int iYsrc );
BOOL	sc3Line( LPBYTE lpDst, LPBYTE lpSrc );
VOID	sc3Free( VOID );

// Grayscale image scaling
BOOL	sc1Init( int iXdst, int iYdst, int iXsrc, int iYsrc );
BOOL	sc1Line( LPBYTE lpDst, LPBYTE lpSrc );
VOID	sc1Free( VOID );
