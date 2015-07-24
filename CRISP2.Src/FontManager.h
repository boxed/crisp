// FontManager.h: interface for the FontManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define USE_FONT_MANAGER

#define		FM_SMALL_FONT			"small"
#define		FM_MEDIUM_FONT			"medium"
#define		FM_LARGE_FONT			"large"
#define		FM_NORMAL_FONT			"normal"
#define		FM_GREEK_FONT			"greek"

#define		FM_SMALL_FONT_CW		"smallcw"
#define		FM_MEDIUM_FONT_CW		"mediumcw"
#define		FM_LARGE_FONT_CW		"largecw"
#define		FM_NORMAL_FONT_CW		"normalcw"
#define		FM_GREEK_FONT_CW		"greekcw"

class FontManager  
{
public:
	FontManager();
	virtual ~FontManager();

	static void Initialize();
	static void Uninitialize();

	static HFONT GetFont(LPCTSTR pszFontName, int &nSizeX, int &nSizeY);
	static HFONT GetFont(LPCTSTR pszFontName, SIZE &szSize);
	static HFONT GetFont(LPCTSTR pszFontName);
	static SIZE GetFontDim(LPCTSTR pszFontName);
};
