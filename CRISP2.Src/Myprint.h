/****************************************************************************/
/* Copyright© 1994 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Printing functions * by ML ***********************************************/
/****************************************************************************/
VOID	prnPrintImage (LPBYTE lpBits, int iWidth, int iHeight, int iBufWidth, LPSTR  lpName, int ps);
VOID	prnPrintSetup( HWND hWnd );
BOOL	prnPageSetup( HWND hWnd, BOOL bNegative );
VOID	prnSaveConfig( VOID );
VOID	prnLoadConfig( VOID );
