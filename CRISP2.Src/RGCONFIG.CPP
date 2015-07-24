/****************************************************************************/
/* Copyright© 1998 SoftHard Technology, Ltd. ********************************/
/****************************************************************************/
/* CRISP2.Registry * by ML **************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "rgconfig.h"

char	szRecentFiles[MAXRECENTFILES][_MAX_PATH];
// Added by Peter on [8/8/2005]
static HKEY s_hKeyToUse = HKEY_LOCAL_MACHINE;
/****************************************************************************/
BOOL regCheckKeyAccess()
{
	char	fkey[_MAX_PATH];
	HKEY	hKey;
	DWORD	disp;
	LONG	rcr;

	sprintf(fkey, "Software\\SoftHard\\%s\\%d.%d", szProg, iVersion, iRelease);
	s_hKeyToUse = HKEY_LOCAL_MACHINE;
	if( ERROR_SUCCESS != (rcr = RegCreateKeyEx( s_hKeyToUse, fkey, 0, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp )) )
	{
		s_hKeyToUse = HKEY_CURRENT_USER;
		if( ERROR_SUCCESS != (rcr = RegCreateKeyEx( s_hKeyToUse, fkey, 0, NULL,
			REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, &disp )) )
		{
			MessageBox(NULL, "Error writing the Windows registry.\r\n"
				"Recent files information will not be saved.", "* CRISP2 ERROR *",
				MB_ICONERROR | MB_OK);
			return FALSE;
		}
	}
	::RegCloseKey(hKey);
	return TRUE;
}
/****************************************************************************/
// end of addition by Peter [8/8/2005]

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
BOOL regSetData( LPSTR szSubKey, LPSTR szValue, LPVOID lpData, DWORD size )
{
	char	fkey[_MAX_PATH];
	HKEY	hKey;
	DWORD	disp;
	LONG	rcr;

	sprintf(fkey,"Software\\SoftHard\\%s\\%d.%d\\%s", szProg, iVersion, iRelease, szSubKey );
	rcr = RegCreateKeyEx( s_hKeyToUse, fkey, 0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, NULL, &hKey, &disp );
	if (rcr != ERROR_SUCCESS) return FALSE;

	if (size) rcr = RegSetValueEx( hKey, szValue, 0, REG_BINARY, (CONST BYTE *)lpData, size );
	else      rcr = RegSetValueEx( hKey, szValue, 0, REG_SZ,     (CONST BYTE *)lpData, strlen((LPCSTR)lpData)+1 );
	RegCloseKey( hKey );

	if (rcr != ERROR_SUCCESS) return FALSE;
	return TRUE;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
BOOL regSetInt( LPCSTR szSubKey, LPCSTR szValue, int val)
{
char	fkey[_MAX_PATH];
HKEY	hKey;
DWORD	disp;
LONG	rcr;


	sprintf(fkey,"Software\\SoftHard\\%s\\%d.%d\\%s", szProg, iVersion, iRelease, szSubKey );
	rcr = RegCreateKeyEx( s_hKeyToUse, fkey, 0, NULL, REG_OPTION_NON_VOLATILE, 
							KEY_ALL_ACCESS, NULL, &hKey, &disp );
	if (rcr != ERROR_SUCCESS) return FALSE;

	sprintf(fkey,"%d",val);
	rcr = RegSetValueEx( hKey, szValue, 0, REG_SZ, (CONST BYTE *)fkey, strlen((LPCSTR)fkey)+1 );
	RegCloseKey( hKey );

	if (rcr != ERROR_SUCCESS) return FALSE;
	return TRUE;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
BOOL regGetData( LPSTR szSubKey, LPSTR szValue, LPVOID lpData, DWORD size )
{
char	fkey[_MAX_PATH];
HKEY	hKey;
DWORD	type;
LONG	rcr;


	sprintf(fkey,"Software\\SoftHard\\%s\\%d.%d\\%s", szProg, iVersion, iRelease, szSubKey );
	rcr = RegOpenKeyEx( s_hKeyToUse, fkey, 0, KEY_READ, &hKey );
	if (rcr != ERROR_SUCCESS) return FALSE;

	rcr = RegQueryValueEx( hKey, szValue, 0, &type, (LPBYTE)lpData, &size );
	RegCloseKey( hKey );

	if (rcr != ERROR_SUCCESS) return FALSE;
	return TRUE;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
int regGetInt( LPCSTR szSubKey, LPCSTR szValue, int defval )
{
char	fkey[_MAX_PATH];
HKEY	hKey;
DWORD	type;
LONG	rcr;
char	str[1000];
DWORD	size = sizeof(str);


	sprintf(fkey,"Software\\SoftHard\\%s\\%d.%d\\%s", szProg, iVersion, iRelease, szSubKey );
	rcr = RegOpenKeyEx( s_hKeyToUse, fkey, 0, KEY_READ, &hKey );
	if (rcr != ERROR_SUCCESS) return defval;

	rcr = RegQueryValueEx( hKey, szValue, 0, &type, (LPBYTE)str, &size );
	RegCloseKey( hKey );

	if (rcr != ERROR_SUCCESS) return defval;
	if (sscanf(str,"%d",&size) == 1) return size;
	return defval;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
BOOL regGetDataDef( LPCSTR szSubKey, LPCSTR szValue, LPVOID lpDef, LPVOID lpData, DWORD size )
{
static char	fkey[_MAX_PATH];
HKEY	hKey;
DWORD	type;
LONG	rcr;

	strcpy((LPSTR)lpData, (LPSTR)lpDef);
	sprintf(fkey,"Software\\SoftHard\\%s\\%d.%d\\%s", szProg, iVersion, iRelease, szSubKey );
	rcr = RegOpenKeyEx( s_hKeyToUse, fkey, 0, KEY_READ, &hKey );
	if (rcr != ERROR_SUCCESS) return FALSE;

	rcr = RegQueryValueEx( hKey, szValue, 0, &type, (LPBYTE)lpData, &size );
	RegCloseKey( hKey );

	if (rcr != ERROR_SUCCESS) return FALSE;
	return TRUE;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
BOOL regWriteWindowPos(HWND hWnd, LPSTR szName, BOOL bSize)
{
RECT	r;

	if (GetWindowRect( hWnd, &r )) {
		if (!bSize) { r.right = -1; r.bottom = -1; }
		return regSetData("Windows position", szName, &r, sizeof(r) );
	}
	return FALSE;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
BOOL regReadWindowPos(HWND hWnd, LPSTR szName)
{
RECT	r, ra;
int		wax, way;

	if (regGetData("Windows position", szName, &r, sizeof(r) )) {
		if (r.right==-1 || r.bottom == -1) {
			GetWindowRect( hWnd, &ra );
			r.right = r.left + (ra.right-ra.left);
			r.bottom = r.top + (ra.bottom-ra.top);
		}
		SystemParametersInfo( SPI_GETWORKAREA, 0, &ra, 0 );
		wax = ra.right - ra.left;
		way = ra.bottom - ra.top;
		if(r.right>wax) { 
			r.right-=r.left; r.left=wax-r.right; r.right = wax;
		}
		if(r.bottom>way) { 
			r.bottom-=r.top; r.top=way-r.bottom; r.bottom = way;
		}
		if(r.left<ra.left) {r.right-=r.left-ra.left; r.left=ra.left;}
		if(r.top<ra.top) {r.bottom-=r.top-ra.top; r.top=ra.top;}
		SetWindowPos(hWnd, NULL, r.left, r.top, r.right-r.left, r.bottom-r.top, SWP_NOZORDER | SWP_NOACTIVATE );
		return TRUE;
	}
	return FALSE;
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void regReadRecentFiles( void )
{
int		i;
	memset(szRecentFiles,0,sizeof(szRecentFiles));
	for(i=0;i<MAXRECENTFILES;i++) {
		sprintf(TMP, "%d", i);
		regGetData("Recent files", TMP, szRecentFiles[i], _MAX_PATH);
	}
}

/****************************************************************************/
void regWriteRecentFiles( void )
{
int		i;
	for(i=0;i<MAXRECENTFILES;i++) {
		sprintf(TMP, "%d", i);
		regSetData("Recent files", TMP, szRecentFiles[i], 0);
	}
}

/****************************************************************************/
void regUpdateFilesMenu( HWND hWnd )
{
int	i;
HMENU	hmenu = GetSubMenu(GetMenu(hWnd), 0);

	if (hmenu==NULL) return;

	for(i=0;i<MAXRECENTFILES;i++) {
		DeleteMenu( hmenu, IDM_F_FILE0+i, MF_BYCOMMAND );
		if (strlen(szRecentFiles[i])) {
			sprintf(TMP, "&%d ", i+1);
			strcat(TMP, szRecentFiles[i]);
			AppendMenu( hmenu, MF_STRING, IDM_F_FILE0+i, TMP);
		}
	}
}

/****************************************************************************/
void regUpdateRecentList( LPSTR lpName )
{
int		i,j;

	for(i=0;i<MAXRECENTFILES;i++) if(strnicmp(lpName,szRecentFiles[i], strlen(lpName))==0) break;

	if (i<MAXRECENTFILES) {
		for(j=i;j>0;j--) strcpy(szRecentFiles[j],szRecentFiles[j-1]);
		strcpy( szRecentFiles[0], lpName );
		return;
	}

	for(j=MAXRECENTFILES-1;j>0;j--) strcpy(szRecentFiles[j],szRecentFiles[j-1]);
	strcpy( szRecentFiles[0], lpName );
}
