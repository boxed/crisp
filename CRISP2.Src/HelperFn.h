#pragma once

// Print any string in UNICODE format
// #define		DEGREE_CHAR			186
// #define		ANGSTR_CHAR			197
// #define		ALPHA_CHAR			0x3b1
// #define		BETA_CHAR			0x3b2
// #define		GAMMA_CHAR			0x3b3
// #define		PHI_CHAR			0x3C6
// #define		PERPEND_CHAR		0x27C2

// #define		DEGREE_CHAR			0xB0
// #define		ANGSTR_CHAR			0x415
// #define		ALPHA_CHAR			0x431
// #define		BETA_CHAR			0x432
// #define		GAMMA_CHAR			0x433
// #define		PHI_CHAR			0x446
// #define		PERPEND_CHAR		0x27C2

#define		DEGREE_CHAR			(char)0xB0
#define		ANGSTR_CHAR			(char)0xC5
#define		ALPHA_CHAR			(char)225
#define		BETA_CHAR			(char)226
#define		GAMMA_CHAR			(char)227
#define		PHI_CHAR			(char)246
#define		PERPEND_CHAR		(wchar_t)0x27C2

void TextToDlgItemA(HWND hDlg, UINT nID, const char *lpszFormat, ...);
void TextToDlgItemW(HWND hDlg, UINT nID, const char *lpszFormat, ...);
// void TextToDlgItemW(HWND hDlg, UINT nID, const wchar_t *lpszFormat, ...);

void PrintWCharText(LPTSTR pszBuffer, LPCTSTR pszFormat, ...);
