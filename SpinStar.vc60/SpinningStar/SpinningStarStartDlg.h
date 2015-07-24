#pragma once

// SpinningStarStartDlg.h : header file
//

#include "SymmListBox.h"

#include "commondef.h"
#include "objects.h"
#include "Eld.h"

/////////////////////////////////////////////////////////////////////////////
// CSpinningStarStartDlg dialog

class CSpinningStarStartDlg : public CDialog
{
	typedef enum
	{
		ZERO_ORDER_LZ = 0,
		FIRST_ORDER_LZ = 1,

	} LAUE_ZONE;
	
//	BOOL Create();

public:
	typedef enum
	{
		HOLZ_MODE = 1,			// single pattern
		TWO_PATTERNS_MODE = 2,	// two different patterns
		DISABLE_BROWSE_BTNS = 0x80000000,

	} DIALOG_MODE;

// Construction
public:
	CSpinningStarStartDlg(CWnd* pParent = NULL);   // standard constructor
	~CSpinningStarStartDlg();

//	static CSpinningStarStartDlg *Create(CWnd* pParentWnd);
	BOOL Create(CWnd* pParentWnd);

// Dialog Data
	CWnd	*m_pParent;		// ptr to parent window (Image window usually)

	//{{AFX_DATA(CSpinningStarStartDlg)
	enum { IDD = IDD_SPIN_STAR_SYMMETRY_TEST_START };
	CComboBox	m_ctrlZoneAxis2;
	CComboBox	m_ctrlZoneAxis1;
	CComboBox	m_ctrlCrystSystemClass;
	CSymmListBox m_ctrlSymmLB1;
	CSymmListBox m_ctrlSymmLB2;
	CString	m_sZolzFile;
	CString	m_sFolzFile;
//	CString	m_sZolzChoice;
//	CString	m_sFolzChoice;
	CString m_sResultSymm;
	CString m_sResultPartSymb;
	int		m_nPatternMode;
	//}}AFX_DATA

	CString m_sZolzFilePath;
	CString m_sFolzFilePath;
//	fmPath m_sZolzFilePath;
//	fmPath m_sFolzFilePath;
	// Thread handling
	HANDLE m_ahThreads[2];
	DWORD m_adwThreadIDs[2];
	SpinningStar::TestSymmetriesParams m_aThreadParams[2];
	SpinningStar::TableMatchVec m_vMatches;

	LaueZone *m_pLZ1;
	LaueZone *m_pLZ2;
	TReflsVec m_vRefls1;
	TReflsVec m_vRefls2;
	UnitCell::CUnitCell *m_pCell1;
	UnitCell::CUnitCell *m_pCell2;
	
//	StringVec m_vSymmResZ;
//	StringVec m_vSymmResF;

//	CrystalStructure m_aCSs[2];

	int m_nCrystalSystem;
	SpinningStar::ZONE_AXIS m_n1stZA;
	SpinningStar::ZONE_AXIS m_n2ndZA;

	DIALOG_MODE m_nDialogMode;

	bool m_abZAready[2];

	void Finalize();
	SpinningStar::ZONE_AXIS CBindex2ZA(int iIndex);
	bool TestSymmetries2(const CString &path, LAUE_ZONE nZone);
//	bool TestSymmetries2(const fmPath &path, LAUE_ZONE nZone);
//	bool BrowseForFile(fmPath &sFileName);
	bool BrowseForFile(CString &sFileName);
	void OpenHideExtraDialog(bool bOpen);
	void ClearSymmLists();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpinningStarStartDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpinningStarStartDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnBrowseFolz();
	afx_msg void OnBrowseZolz();
	afx_msg void OnRunTests();
	afx_msg void OnClose();
	virtual void OnCancel();
	virtual void OnOK(void);
	afx_msg LRESULT OnFinishedRsymm(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnBestChoiceChanged(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelchangeSymmList();
	afx_msg void OnChooseFOLZorZA();
	afx_msg void OnSelchangeZoneAxisCombo1();
	afx_msg void OnSelchangeZoneAxisCombo2();
	afx_msg void OnSelchangeCrystalSystemClassCombo();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
