// FontManager.cpp: implementation of the FontManager class.
//
// Created by Peter Oleynikov, 10 August 2009
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "FontManager.h"

#include "Globals.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

class StaticFontManager
{
public:
	StaticFontManager()
	{
		m_bInitialized = false;
	}
	~StaticFontManager()
	{
		Uninit();
	}

	void Init()
	{
		if( true == m_bInitialized )
		{
			return;
		}
		HDC hdc = GetDC( MainhWnd );

		// CW stands for Constant Width
		static LPCTSTR pszFontNameCW = _T("Courier New");
		static LPCTSTR pszFontName = _T("Arial");

		createFont(hdc, pszFontNameCW, 12, "smallcw");
		createFont(hdc, pszFontNameCW, 14, "mediumcw");
		createFont(hdc, pszFontNameCW, 16, "largecw");
		createFont(hdc, pszFontNameCW, 14, "normalcw", BALTIC_CHARSET);
		createFont(hdc, pszFontNameCW, 14, "greekcw", GREEK_CHARSET);

		createFont(hdc, pszFontName, 11, "small");
		createFont(hdc, pszFontName, 13, "medium");
		createFont(hdc, pszFontName, 16, "large");
		createFont(hdc, pszFontName, 14, "normal", BALTIC_CHARSET);
		createFont(hdc, pszFontName, 14, "greek", GREEK_CHARSET);
		
		ReleaseDC( MainhWnd, hdc );

		m_bInitialized = true;
	}
	
	void Uninit()
	{
		for(FontMapIt it = m_fontMap.begin(); it != m_fontMap.end(); ++it)
		{
			if( NULL != it->second.hFont )
			{
				::DeleteFont(it->second.hFont);
				it->second.hFont = NULL;
			}
		}
		m_fontMap.clear();
	}
	
protected:
	typedef struct FONT_DESC_t
	{
		HFONT hFont;
		SIZE szSize;
		
	} FONT_DESC;

	bool createFont(HDC hdc, LPCTSTR pszFontName, int nSize, LPCTSTR keyName, int CHAR_SET = DEFAULT_CHARSET)
	{
		FONT_DESC font_desc;

		TEXTMETRIC	tm;
		font_desc.hFont = CreateFont( nSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, CHAR_SET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_MODERN, pszFontName);
		HFONT hOldFont = SelectFont( hdc, font_desc.hFont );
		if( !GetTextMetrics( hdc, &tm ) )
		{
			DWORD err = GetLastError();
		}
		SelectObject( hdc, hOldFont );

		font_desc.szSize.cx = tm.tmAveCharWidth;
		font_desc.szSize.cy = tm.tmHeight;

		m_fontMap[keyName] = font_desc;

		return (NULL != font_desc.hFont);
	}

	bool m_bInitialized;
	
// 	HFONT	hfSmall, hfMedium, hfLarge, hfNormal, hfGreek;
// 	int		fnts_w, fnts_h;
// 	int		fntm_w, fntm_h;
// 	int		fntn_w, fntn_h;
// 	int		fntg_w, fntg_h;
// 	int		fntl_w, fntl_h;
public:
	typedef std::map<std::string, FONT_DESC> FontMap;
	typedef FontMap::iterator FontMapIt;
	typedef FontMap::const_iterator FontMapCit;
	FontMap m_fontMap;

} theStaticFontManager;

//////////////////////////////////////////////////////////////////////////

FontManager::FontManager()
{

}

FontManager::~FontManager()
{

}

void FontManager::Initialize()
{
	theStaticFontManager.Init();
}

void FontManager::Uninitialize()
{
	theStaticFontManager.Uninit();
}

HFONT FontManager::GetFont(LPCTSTR pszFontName, int &nSizeX, int &nSizeY)
{
	theStaticFontManager.Init();
	StaticFontManager::FontMapIt cit = theStaticFontManager.m_fontMap.find(pszFontName);
	HFONT hFont = NULL;
	nSizeX = nSizeY = 0;
	if( theStaticFontManager.m_fontMap.end() != cit )
	{
		hFont = cit->second.hFont;
		nSizeX = cit->second.szSize.cx;
		nSizeY = cit->second.szSize.cy;
	}
	return hFont;
}

HFONT FontManager::GetFont(LPCTSTR pszFontName, SIZE &szSize)
{
	theStaticFontManager.Init();
	StaticFontManager::FontMapIt cit = theStaticFontManager.m_fontMap.find(pszFontName);
	HFONT hFont = NULL;
	szSize.cx = szSize.cy = 0;
	if( theStaticFontManager.m_fontMap.end() != cit )
	{
		hFont = cit->second.hFont;
		szSize = cit->second.szSize;
	}
	return hFont;
}

HFONT FontManager::GetFont(LPCTSTR pszFontName)
{
	theStaticFontManager.Init();
	StaticFontManager::FontMapIt cit = theStaticFontManager.m_fontMap.find(pszFontName);
	HFONT hFont = NULL;
	if( theStaticFontManager.m_fontMap.end() != cit )
	{
		hFont = cit->second.hFont;
	}
	return hFont;
}

SIZE FontManager::GetFontDim(LPCTSTR pszFontName)
{
	theStaticFontManager.Init();
	StaticFontManager::FontMapIt cit = theStaticFontManager.m_fontMap.find(pszFontName);
	SIZE szSize = {0, 0};
	if( theStaticFontManager.m_fontMap.end() != cit )
	{
		szSize = cit->second.szSize;
	}
	return szSize;
}
