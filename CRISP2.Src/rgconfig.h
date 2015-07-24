/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* Registry functions * by ML ***********************************************/
/****************************************************************************/

// Added by Peter on [8/8/2005]
BOOL regCheckKeyAccess();
// end of addition by Peter [8/8/2005]

BOOL regSetData( LPSTR szSubKey, LPSTR szValue, LPVOID lpData, DWORD size );
BOOL regGetData( LPSTR szSubKey, LPSTR szValue, LPVOID lpData, DWORD size );
BOOL regGetDataDef( LPCSTR szSubKey, LPCSTR szValue, LPVOID lpDef, LPVOID lpData, DWORD size );
BOOL regSetInt( LPCSTR szSubKey, LPCSTR szValue, int val);
int  regGetInt( LPCSTR szSubKey, LPCSTR szValue, int defval );

BOOL regWriteWindowPos(HWND hWnd, LPSTR szName, BOOL bSize);
BOOL regReadWindowPos (HWND hWnd, LPSTR szName);

VOID regWriteRecentFiles( VOID );
VOID regReadRecentFiles ( VOID );
VOID regUpdateFilesMenu ( HWND hWnd );
VOID regUpdateRecentList( LPSTR lpName );

