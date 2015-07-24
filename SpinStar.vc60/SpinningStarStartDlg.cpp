// SpinningStarStartDlg.cpp : implementation file
//

#include "stdafx.h"

#include "resource.h"

#include "SpinningStar/SpinningStar.h"
#include "SpinningStar/SpinningStarStartDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
/*
class StaticSingleton
{
public:
	static StaticSingleton &instance()
	{
		static StaticSingleton inst;
		return inst;
	}
	~StaticSingleton()
	{
		s_CreatedDialogs.clear();
	}

	void AddPointer(CSpinningStarStartDlg *pDialog)
	{
		s_CreatedDialogs.push_back(pDialog);
	}
	void RemovePointer(CSpinningStarStartDlg *pDialog)
	{
		std::vector<CSpinningStarStartDlg*>::iterator it;
		
		for(it = s_CreatedDialogs.begin(); it != s_CreatedDialogs.end(); ++it)
		{
			if( *it == pDialog )
			{
				s_CreatedDialogs.erase(it);
				break;
			}
//			delete *it;
		}
	}

protected:
private:
	StaticSingleton() {}
	std::vector<CSpinningStarStartDlg*>	s_CreatedDialogs;
};
*/
/*
class StaticSingleton
{
public:
	StaticSingleton() {}
	~StaticSingleton()
	{
		s_CreatedDialogs.clear();
	}

	void AddPointer(CSpinningStarStartDlg *pDialog)
	{
		s_CreatedDialogs.push_back(pDialog);
	}
	void RemovePointer(CSpinningStarStartDlg *pDialog)
	{
		std::vector<CSpinningStarStartDlg*>::iterator it;
		
		for(it = s_CreatedDialogs.begin(); it != s_CreatedDialogs.end(); ++it)
		{
			if( *it == pDialog )
			{
				s_CreatedDialogs.erase(it);
				break;
			}
		}
	}

protected:
private:
	static std::vector<CSpinningStarStartDlg*>	s_CreatedDialogs;

} theStaticSingleton;

std::vector<CSpinningStarStartDlg*>	StaticSingleton::s_CreatedDialogs;
*/
/////////////////////////////////////////////////////////////////////////////
// CSpinningStarStartDlg dialog

CSpinningStarStartDlg::CSpinningStarStartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSpinningStarStartDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpinningStarStartDlg)
	m_sZolzFile = _T("");
	m_sFolzFile = _T("");
//	m_sZolzChoice = _T("");
//	m_sFolzChoice = _T("");
	m_nPatternMode = 0;		// Means Single pattern
	m_sResultSymm = _T("");
	m_sResultPartSymb = _T("");
	//}}AFX_DATA_INIT
	m_ahThreads[0] = NULL;
	m_ahThreads[1] = NULL;
	m_adwThreadIDs[0] = 0;
	m_adwThreadIDs[1] = 0;
	memset(&m_aThreadParams[0], 0, sizeof(m_aThreadParams[0]));
	memset(&m_aThreadParams[1], 0, sizeof(m_aThreadParams[1]));

	m_nCrystalSystem = XS_Unknown;
	m_n1stZA = SpinningStar::ZA_UNKNOWN;
	m_n2ndZA = SpinningStar::ZA_UNKNOWN;

	m_abZAready[0] = false;
	m_abZAready[1] = false;

	m_pParent = NULL;

	m_nDialogMode = (DIALOG_MODE)(HOLZ_MODE | DISABLE_BROWSE_BTNS);

	m_pLZ1 = NULL;
	m_pLZ2 = NULL;

	m_pCell1 = UnitCell::CUnitCell::CreateUnitCell(UnitCell::CELL_2D);
	m_pCell2 = UnitCell::CUnitCell::CreateUnitCell(UnitCell::CELL_2D);
}

CSpinningStarStartDlg::~CSpinningStarStartDlg()
{
	Finalize();
}
/*
CSpinningStarStartDlg *CSpinningStarStartDlg::Create(CWnd* pParentWnd)
{
	CSpinningStarStartDlg *pDialog = NULL;

	pDialog = new CSpinningStarStartDlg;
	if( NULL != pDialog )
	{
		if( NULL != pDialog->m_pParent )
			pDialog->m_pParent = pParentWnd;
		pDialog->Create();
//		StaticSingleton::instance().AddPointer(pDialog);
		theStaticSingleton.AddPointer(pDialog);
	}
	return pDialog;
}
*/
BOOL CSpinningStarStartDlg::Create(CWnd* pParentWnd)
{
	return CDialog::Create(CSpinningStarStartDlg::IDD, m_pParent);
}

void CSpinningStarStartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpinningStarStartDlg)
	DDX_Control(pDX, IDC_ZONE_AXIS_COMBO_2, m_ctrlZoneAxis2);
	DDX_Control(pDX, IDC_ZONE_AXIS_COMBO_1, m_ctrlZoneAxis1);
	DDX_Control(pDX, IDC_CRYSTAL_SYSTEM_CLASS_COMBO, m_ctrlCrystSystemClass);
	DDX_Control(pDX, IDC_SYMM_LIST, m_ctrlSymmLB1);
	DDX_Control(pDX, IDC_SYMM_LIST2, m_ctrlSymmLB2);
	DDX_Text(pDX, IDC_ZOLZ_FILE, m_sZolzFile);
	DDX_Text(pDX, IDC_FOLZ_FILE, m_sFolzFile);
//	DDX_Text(pDX, IDC_ZOLZ_CHOICE, m_sZolzChoice);
//	DDX_Text(pDX, IDC_FOLZ_CHOICE, m_sFolzChoice);
	DDX_Text(pDX, IDC_RESULT_TEXT, m_sResultSymm);
	DDX_Text(pDX, IDC_PARTIAL_SYMBOLS_TEXT, m_sResultPartSymb);
	DDX_Radio(pDX, IDC_SINGLE_PATT_RADIO, m_nPatternMode);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSpinningStarStartDlg, CDialog)
	//{{AFX_MSG_MAP(CSpinningStarStartDlg)
	ON_BN_CLICKED(IDC_BROWSE_FOLZ, OnBrowseFolz)
	ON_BN_CLICKED(IDC_BROWSE_ZOLZ, OnBrowseZolz)
	ON_BN_CLICKED(IDC_RUN_TESTS, OnRunTests)
	ON_WM_CLOSE()
	ON_MESSAGE(WMU_FINISHED_SYMMETRY, OnFinishedRsymm)
	ON_MESSAGE(WMU_SET_BEST_CHOICE, OnBestChoiceChanged)
	ON_LBN_SELCHANGE(IDC_SYMM_LIST, OnSelchangeSymmList)
	ON_LBN_SELCHANGE(IDC_SYMM_LIST2, OnSelchangeSymmList)
	ON_CBN_SELCHANGE(IDC_ZONE_AXIS_COMBO_1, OnSelchangeZoneAxisCombo1)
	ON_CBN_SELCHANGE(IDC_ZONE_AXIS_COMBO_2, OnSelchangeZoneAxisCombo2)
	ON_CBN_SELCHANGE(IDC_CRYSTAL_SYSTEM_CLASS_COMBO, OnSelchangeCrystalSystemClassCombo)
	ON_BN_CLICKED(IDC_SINGLE_PATT_RADIO, OnChooseFOLZorZA)
	ON_BN_CLICKED(IDC_TWO_PATTS_RADIO, OnChooseFOLZorZA)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpinningStarStartDlg message handlers

BOOL CSpinningStarStartDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	LVITEM item;
	int i;
//	SpaceGroup sg(1, SG_PlaneGroup);
/*
	CFont font;

	// create the "Arial" font for the ZOLZ-FOLZ static text boxes
	font.CreateFont(36, 0, 0, 0,
		FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH | FF_ROMAN,
		_T("Arial"));

	GetDlgItem(IDC_ZOLZ_CHOICE)->SetFont(&font);
	GetDlgItem(IDC_FOLZ_CHOICE)->SetFont(&font);
//	GetDlgItem(IDC_RESULT_SYMM)->SetFont(&font);
*/
	// Add all possible crystal systems
	for(i = XS_Unknown; i <= XS_Cubic; i++)
	{
		m_ctrlCrystSystemClass.InsertString(i, XS_Name[i]);
	}
	m_ctrlCrystSystemClass.SetCurSel(0);

	if( m_nDialogMode & DISABLE_BROWSE_BTNS )
	{
		static UINT aCtrlIDs[] =
		{
			IDC_BROWSE_ZOLZ,
			IDC_BROWSE_FOLZ,
			IDC_STATIC_ZA2_TEXT,
			IDC_ZONE_AXIS_COMBO_2,
			IDC_SINGLE_PATT_RADIO,
			IDC_TWO_PATTS_RADIO,
		};
		for(i = 0; i < sizeof(aCtrlIDs) / sizeof(UINT); i++)
		{
			GetDlgItem(aCtrlIDs[i])->EnableWindow(FALSE);
			GetDlgItem(aCtrlIDs[i])->ShowWindow(SW_HIDE);
		}
	}

	// set column names
	m_ctrlSymmLB1.InsertColumn(0, _T("Symm"), LVCFMT_LEFT, 50);
	m_ctrlSymmLB2.InsertColumn(0, _T("Symm"), LVCFMT_LEFT, 50);
	if( 0 == m_nPatternMode )
	{
		m_ctrlSymmLB1.InsertColumn(1, _T("RA%"), LVCFMT_LEFT, 50);
		m_ctrlSymmLB2.InsertColumn(1, _T("RA%"), LVCFMT_LEFT, 50);
	}
	else
	{
		m_ctrlSymmLB1.InsertColumn(1, _T("RA%"), LVCFMT_LEFT, 50);
		m_ctrlSymmLB2.InsertColumn(1, _T("RA%"), LVCFMT_LEFT, 50);
	}
//	m_ctrlSymmLB.InsertColumn(2, _T("RA%(2)"), LVCFMT_LEFT, 50);

	// add plane group names to the 1st column
//	for(i = 0; i < SYMM_P6M; i++)
//	{
//		item.mask = LVIF_TEXT;
//		item.iItem = i;
//		item.iSubItem = 0;
//		item.pszText = (LPTSTR)sg.GetAbsSgName(i, SG_PlaneGroup);
//		m_ctrlSymmLB1.InsertItem(&item);
//		m_ctrlSymmLB2.InsertItem(&item);
//	}
	for(i = 0; i < MAX_ZOLZ_SYMMETRIES; i++)
	{
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = 0;
		item.pszText = (LPTSTR)Plane_PG_Names_Ext[SpinningStar::s_aiZOLZ_symmetries[i]];
		m_ctrlSymmLB1.InsertItem(&item);
	}
	for(i = 0; i < MAX_HOLZ_SYMMETRIES; i++)
	{
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = 0;
		item.pszText = (LPTSTR)Plane_PG_Names_Ext[SpinningStar::s_aiHOLZ_symmetries[i]];
		m_ctrlSymmLB2.InsertItem(&item);
	}

	// Now check the data
	if( NULL != m_pLZ1 )
	{
		GetDlgItem(IDC_RUN_TESTS)->EnableWindow();
		GetDlgItem(IDC_CRYSTAL_SYSTEM_CLASS_COMBO)->EnableWindow();
		OpenHideExtraDialog(true);
	}
	else
	{
		OpenHideExtraDialog(false);
	}

	UpdateData(FALSE);

	return TRUE;
}

void CSpinningStarStartDlg::PostNcDestroy()
{
//	StaticSingleton::instance().RemovePointer(this);
//	theStaticSingleton.RemovePointer(this);
	TRACE(_T("CSpinningStarStartDlg::PostNcDestroy() ... destroying\n"));
//	delete this;
}

void CSpinningStarStartDlg::OnOK()
{
	UpdateData(TRUE);
//	DestroyWindow();
}

void CSpinningStarStartDlg::OnClose() 
{
	Finalize();

	CDialog::OnClose();

//	DestroyWindow();
}

void CSpinningStarStartDlg::OnCancel() 
{
	Finalize();

	CDialog::OnCancel();

//	DestroyWindow();
}

void CSpinningStarStartDlg::OpenHideExtraDialog(bool bOpen)
{
	CRect rc, rc2, wndrc;
	CWnd *pWnd = GetDlgItem(IDC_STATIC_TEXT_SECOND_FILE);
	CWnd *pWnd2 = GetDlgItem(IDC_SYMMETRY_STATIC);
	int cx, cy, border;

	if( (NULL == pWnd) || (NULL == pWnd2) )
	{
		return;
	}

	GetWindowRect(wndrc);

	cx = wndrc.Width();
	cy = wndrc.Height();

	pWnd->GetWindowRect(rc);
	pWnd2->GetWindowRect(rc2);

	border = rc.left - wndrc.left;

	if( true == bOpen )
	{
		// Open
		cx = rc2.right - wndrc.left + border;
	}
	else
	{
		// Hide
		cx = rc.right - wndrc.left + border;
	}
	// reposition the window
	SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
}

SpinningStar::ZONE_AXIS CSpinningStarStartDlg::CBindex2ZA(int iIndex)
{
	SpinningStar::ZONE_AXIS nZA = SpinningStar::ZA_UNKNOWN;
	// unknown Crystal System
	if( XS_Unknown != m_nCrystalSystem )
	{
		// find the corresponding Mornirolli table
		const SpinningStar::Morniroli_table *pTables = SpinningStar::s_MorniroliTables;
		const SpinningStar::Morniroli_entry *pEntry;
		int i;
		while( pTables->pEntries )
		{
			// found the crystal system?
			if( pTables->nCrystalSystem == m_nCrystalSystem )
			{
				i = 1;
				pEntry = pTables->pEntries;
				while( SpinningStar::ZA_UNKNOWN != pEntry->nZA )
				{
					// found?
					if( i == iIndex )
					{
						nZA = pEntry->nZA;
						break;
					}
					i++;
					pEntry++;
				}
				break;
			}
			// next table entry
			pTables++;
		}
	}
	return nZA;
}

bool CSpinningStarStartDlg::BrowseForFile(CString &sFileName)
{
	CFileDialog dlg(TRUE, _T(""), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T(""), this);
	// TODO: finish
	if( IDOK == dlg.DoModal() )
	{
		sFileName = dlg.GetPathName();
	}
	return true;
}

void CSpinningStarStartDlg::OnBrowseZolz() 
{
//	String strName;

	if( false == BrowseForFile(m_sZolzFilePath) )
	{
		return;
	}

//	m_sZolzFilePath.GetDisplayName(strName, 50, true);
//	m_sZolzFile = CString(strName.c_str());
	m_sZolzFile = m_sZolzFilePath;

	UpdateData(FALSE);

	// enable Zone Axis text and ComboBox in case if the crystal system is known
/*
	GetDlgItem(IDC_STATIC_ZOLZ_ZA_TEXT)->EnableWindow(FALSE);//bEnable);
	GetDlgItem(IDC_ZONE_AXIS_COMBO_1)->EnableWindow(FALSE);//bEnable);

	GetDlgItem(IDC_CRYSTAL_SYSTEM_CLASS_COMBO)->EnableWindow();

	GetDlgItem(IDC_STATIC_TEXT_SECOND_FILE)->EnableWindow();
	GetDlgItem(IDC_FOLZ_FILE)->EnableWindow();
	GetDlgItem(IDC_BROWSE_FOLZ)->EnableWindow();
	GetDlgItem(IDC_RUN_TESTS)->EnableWindow();
*/

	OnSelchangeCrystalSystemClassCombo();

	OpenHideExtraDialog(true);
}

void CSpinningStarStartDlg::OnBrowseFolz()
{
//	String strName;

	if( false == BrowseForFile(m_sFolzFilePath) )
	{
		return;
	}

/*
	m_sFolzFilePath.GetDisplayName(strName, 50, true);
	m_sFolzFile = CString(strName.c_str());
*/
	m_sFolzFile = m_sFolzFilePath;

/*
	GetDlgItem(IDC_FOLZ_RADIO)->EnableWindow();
	GetDlgItem(IDC_FOLZ_CHOICE)->EnableWindow();
*/
	// Do not enable IDC_ZONE_AXIS_COMBO_2, because the FOLZ is the default choice

	OnSelchangeCrystalSystemClassCombo();

	UpdateData(FALSE);
}

void CSpinningStarStartDlg::ClearSymmLists()
{
	int i;
	LVITEM item;

	for(i = 0; i < MAX_ZOLZ_SYMMETRIES; i++)
	{
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = 1;
		item.pszText = (LPTSTR)_T("");
		m_ctrlSymmLB1.SetItem(&item);
	}
	for(i = 0; i < MAX_HOLZ_SYMMETRIES; i++)
	{
		item.mask = LVIF_TEXT;
		item.iItem = i;
		item.iSubItem = 1;
		item.pszText = (LPTSTR)_T("");
		m_ctrlSymmLB2.SetItem(&item);
	}
	m_ctrlSymmLB1.SetBestChoice(-1, (int)0);
	m_ctrlSymmLB2.SetBestChoice(-1, (int)0);
}

void CSpinningStarStartDlg::OnRunTests() 
{
	if( TRUE == m_sZolzFilePath.IsEmpty() )
	{
		return;
	}
	// none of ZA is ready
	m_abZAready[0] = false;
	m_abZAready[1] = false;

	ClearSymmLists();

	// Try ZOLZ first and ...
	if( true == TestSymmetries2(m_sZolzFilePath, ZERO_ORDER_LZ) )
	{
	}
	if( TRUE == m_sFolzFilePath.IsEmpty() )
	{
		// ... we are done
	}
	else
	{
		// ... FOLZ after
		if( true == TestSymmetries2(m_sFolzFilePath, FIRST_ORDER_LZ) )
		{
		}
	}
	UpdateData(TRUE);
}

bool CSpinningStarStartDlg::TestSymmetries2(const CString &path, LAUE_ZONE nZone)
{
	EldReflVecCit cit;
	int nFileNo = 0;
	LaueZone *pLZ;
	double params[3];

	// determine the file number
	if( ZERO_ORDER_LZ == nZone )
	{
		nFileNo = 0;
		pLZ = m_pLZ1;
		m_aThreadParams[nFileNo].pvRefls = &m_vRefls2;
		m_aThreadParams[nFileNo].pBasicCell = m_pCell1;
		m_aThreadParams[nFileNo].hListCtrl = m_ctrlSymmLB1.GetSafeHwnd();
		m_aThreadParams[nFileNo].nLaueZone = 0;
	}
	else if( FIRST_ORDER_LZ == nZone )
	{
		nFileNo = 1;
		pLZ = m_pLZ2;
		m_aThreadParams[nFileNo].pvRefls = &m_vRefls1;
		m_aThreadParams[nFileNo].pBasicCell = m_pCell2;
		m_aThreadParams[nFileNo].hListCtrl = m_ctrlSymmLB2.GetSafeHwnd();
		m_aThreadParams[nFileNo].nLaueZone = 1;
	}
	params[0] = pLZ->a_length;
	params[1] = pLZ->b_length;
	params[2] = pLZ->gamma;
	m_aThreadParams[nFileNo].pBasicCell->SetParameters(params,
		UnitCell::RADIANS, UnitCell::RECIPROCAL);
	for(cit = pLZ->m_vRefls.begin(); cit != pLZ->m_vRefls.end(); ++cit)
	{
		TReflection refl;
		ZeroMemory(&refl, sizeof(refl));
		refl.hkl.x = cit->h;
		refl.hkl.y = cit->k;
		refl.hkl.z = 0;
		if( 0 == m_nPatternMode )	// single pattern
		{
			refl.hkl.z = nZone;
		}
		refl.Fhkl = cit->ae * 256;
		m_aThreadParams[nFileNo].pvRefls->push_back(refl);
	}
	// terminate the previous thread if it was created
	if( NULL != m_ahThreads[nFileNo] )
	{
		::SuspendThread(m_ahThreads[nFileNo]);
		::TerminateThread(m_ahThreads[nFileNo], -1);
		m_ahThreads[nFileNo] = NULL;
	}
	// thread parameters
	m_aThreadParams[nFileNo].pThread = &m_ahThreads[nFileNo];
	m_aThreadParams[nFileNo].hMainWnd = this->GetSafeHwnd();
//	m_aThreadParams[nFileNo].nColPG = 0;
//	m_aThreadParams[nFileNo].nColRA = (int)nZone + 1;
	m_aThreadParams[nFileNo].bCheckFriedel =
		((0 == m_nPatternMode) && (1 == nZone)) ? false : true;
	// start the thread
	m_ahThreads[nFileNo] = ::CreateThread(NULL, 0, SpinningStar::TestSymmetries,
		&m_aThreadParams[nFileNo], CREATE_SUSPENDED, &m_adwThreadIDs[nFileNo]);
/*
	// we must set HOLZ l-index to 1
	if( (FIRST_ORDER_LZ == nZone) && (0 == m_nZAorFOLZ) )
	{
		TReflsVecIt it;
		for(it = m_aCSs[nFileNo].m_vRefls.begin(); it != m_aCSs[nFileNo].m_vRefls.end(); ++it)
		{
			it->hkl.z = 1;
		}
	}
*/
	if( NULL == m_ahThreads[nFileNo] )
	{
		SpinningStar::TestSymmetries(&m_aThreadParams[nFileNo]);
	}
	else
	{
		::ResumeThread(m_ahThreads[nFileNo]);
	}
	return true;
}

void CSpinningStarStartDlg::Finalize()
{
	for(int i = 0; i < 2; i++)
	{
		if( NULL != m_ahThreads[i] )
		{
			DWORD dwRes = ::SuspendThread(m_ahThreads[i]);
			if( FALSE == ::TerminateThread(m_ahThreads[i], -1) )
			{
				TRACE(_T("CSpinningStarStartDlg::Finalize() -> terminating alive thread (%d).\n"), i);
			}
			m_ahThreads[i] = NULL;
		}
//		if( NULL != m_aCSs[i].m_pCell )
//		{
//			UnitCell::CUnitCell::DestroyUnitCell(m_aCSs[i].m_pCell);
//			m_aCSs[i].m_pCell = NULL;
//		}
	}
	if( NULL != m_pCell1 )
	{
		UnitCell::CUnitCell::DestroyUnitCell(m_pCell1);
		m_pCell1 = NULL;
	}
	if( NULL != m_pCell2 )
	{
		UnitCell::CUnitCell::DestroyUnitCell(m_pCell2);
		m_pCell2 = NULL;
	}
}

// wParam -> the best choice
// lParam -> the column been analyzed (1==RA1 or 2==RA2)
LRESULT CSpinningStarStartDlg::OnFinishedRsymm(WPARAM wParam, LPARAM lParam)
{
//	m_ctrlSymmLB.SetBestChoice((int)wParam, (int)lParam - 1);
	int nBestPG_Guess = (int)wParam;
	int nLaueZone = (int)lParam;
	int nChoice = -1;
	CSymmListBox *apBoxes[] = {&m_ctrlSymmLB1, &m_ctrlSymmLB2};
	static const int *aiPtGrps[] = {SpinningStar::s_aiZOLZ_symmetries, SpinningStar::s_aiHOLZ_symmetries};
	static const int aiSizes[] = {MAX_ZOLZ_SYMMETRIES, MAX_HOLZ_SYMMETRIES};

	// some checks before
	if( (nBestPG_Guess < 0) || (nBestPG_Guess > MAX_HOLZ_SYMMETRIES) )
	{
		return -1;
	}
	if( (nLaueZone < 0) || (nLaueZone > 1) )
	{
		return -1;
	}
	// convert the plane group into the point group frist
//	nChoice = SpinningStar::s_aiPlane2PointGroup[nChoice];
	// get the position of the plane point group in the ListControl
	nChoice = find(aiPtGrps[nLaueZone], aiPtGrps[nLaueZone] + aiSizes[nLaueZone],
		nBestPG_Guess) - aiPtGrps[nLaueZone];
	// set the flag that we are done
	m_abZAready[nLaueZone] = true;
	// update the selection mark in the ListControl
	apBoxes[nLaueZone]->SetBestChoice(nChoice, (int)0);
	// update the list
	OnSelchangeSymmList();
	UpdateData(FALSE);
	return 0;
}
// wParam - the file number (1 or 2)
// lParam - the selected row == plane s.g.
LRESULT CSpinningStarStartDlg::OnBestChoiceChanged(WPARAM wParam, LPARAM lParam)
{
	OnSelchangeSymmList();

	return 0;
}

void CSpinningStarStartDlg::OnSelchangeSymmList()
{
	CString str;
	int nSel1, nSel2, nBestGuess1, nBestGuess2;
	bool bTest = true;

	nBestGuess1 = nBestGuess2 = -1;
	nSel1 = m_ctrlSymmLB1.GetBestChoice(0);
	nSel2 = m_ctrlSymmLB2.GetBestChoice(0);
//	nSel1 = m_ctrlSymmLB.GetBestChoice(0);
//	nSel2 = m_ctrlSymmLB.GetBestChoice(1);
/*
	if( nSel1 >= 0 )
	{
		m_sZolzChoice = SpaceGroup::GetAbsSgName(nSel1, SG_PlaneGroup);
		m_sZolzChoice += _T("\r\n(");
		m_sZolzChoice += Plane_PG_Names[PlaneGr_to_Plane_PG[nSel1+1]];
		m_sZolzChoice += _T(")");
	}
	if( nSel2 >= 0 )
	{
		m_sFolzChoice = SpaceGroup::GetAbsSgName(nSel2, SG_PlaneGroup);
		m_sFolzChoice += _T("\r\n(");
		m_sFolzChoice += Plane_PG_Names[PlaneGr_to_Plane_PG[nSel2+1]];
		m_sFolzChoice += _T(")");
	}
*/

//	nBestGuess1 = m_ctrlSymmLB.GetBestChoice(0);
	nBestGuess1 = m_ctrlSymmLB1.GetBestChoice(0);
	// if we have files and they have been analyzed
	if( FALSE == m_sZolzFilePath.IsEmpty() )
	{
		if( false == m_abZAready[0] )
		{
			// cancel test - the thread is not ready yet
			bTest = false;
		}
		else
		{
			if( FALSE == m_sFolzFilePath.IsEmpty() )
			{
				if( false == m_abZAready[1] )
				{
					// cancel test - the thread is not ready yet
					bTest = false;
				}
				else
				{
//					nBestGuess2 = m_ctrlSymmLB.GetBestChoice(1);
					nBestGuess2 = m_ctrlSymmLB2.GetBestChoice(0);
				}
			}
			else
			{
				nBestGuess2 = -1;
			}
		}
	}
	else
	{
		bTest = false;
	}
	// clear matches list
	m_vMatches.clear();
	// try only if allowed
	if( true == bTest )
	{
		SpinningStar::GuessPointGroup(nBestGuess1, nBestGuess2, m_nCrystalSystem,
			m_pCell1, m_pCell2, m_n1stZA, m_n2ndZA, m_sResultSymm, m_vMatches);
		// guess the partial symmetry symbol in case if both HOLZ and ZOLZ are ready
		if( m_abZAready[0] && m_abZAready[1] )
		{
			SpinningStar::GuessPartialSymbol(m_aThreadParams[0].pvRefls,
				m_aThreadParams[1].pvRefls, m_pCell1, m_pCell2, m_sResultPartSymb, m_vMatches);
		}
	}
	UpdateData(FALSE);
	::UpdateWindow(GetDlgItem(IDC_RESULT_TEXT)->GetSafeHwnd());
	::UpdateWindow(GetDlgItem(IDC_PARTIAL_SYMBOLS_TEXT)->GetSafeHwnd());
}

void CSpinningStarStartDlg::OnChooseFOLZorZA() 
{
	UpdateData();
/*
	if( 0 == m_nZAorFOLZ )
	{
		// 1. Disable 2nd Combo with Zone axes list
		m_ctrlZoneAxis2.EnableWindow(FALSE);
		// 2. Change the text of the bounding rectangle
		GetDlgItem(IDC_STATIC_TEXT_SECOND_FILE)->SetWindowText(_T("FOLZ (First Order Laue Zone)"));
	}
	else
	{
		// 1. Enable 2nd Combo with Zone axes list
		m_ctrlZoneAxis2.EnableWindow(TRUE);
		// 2. Change the text of the bounding rectangle
		GetDlgItem(IDC_STATIC_TEXT_SECOND_FILE)->SetWindowText(_T("Zone Axis of the 2nd pattern"));
	}
*/
	OnSelchangeCrystalSystemClassCombo();
	UpdateData(FALSE);
}

void CSpinningStarStartDlg::OnSelchangeZoneAxisCombo1() 
{
	// 1. Check that the ZA from the 2nd Combo is different
	int n1stZA = m_ctrlZoneAxis1.GetCurSel();
	int n2ndZA = m_ctrlZoneAxis2.GetCurSel();

	if( (n1stZA == n2ndZA) && (n2ndZA != 0) )
	{
		MessageBox(_T("You have the same Zone Axis for the 2nd pattern."));
		m_ctrlZoneAxis1.SetCurSel(m_n1stZA);
	}
	else
	{
		m_n1stZA = CBindex2ZA(n1stZA);
	}
	OnSelchangeSymmList();
}

void CSpinningStarStartDlg::OnSelchangeZoneAxisCombo2() 
{
	// 1. Check that the ZA from the 1st Combo is different
	int n1stZA = m_ctrlZoneAxis1.GetCurSel();
	int n2ndZA = m_ctrlZoneAxis2.GetCurSel();

	if( (n1stZA == n2ndZA) && (n1stZA != 0) )
	{
		MessageBox(_T("You have the same Zone Axis for the 1st pattern."));
		m_ctrlZoneAxis2.SetCurSel(m_n2ndZA);
	}
	else
	{
		m_n2ndZA = CBindex2ZA(n2ndZA);
	}
	OnSelchangeSymmList();
}

void CSpinningStarStartDlg::OnSelchangeCrystalSystemClassCombo() 
{
	BOOL bEnableZA1, bEnableZA2;
	int nCurCrSystem = m_ctrlCrystSystemClass.GetCurSel();

	// enable/disable some windows
	bEnableZA1 = TRUE;
	bEnableZA2 = TRUE;
	// Unknown symmetry class?
	if( 0 == nCurCrSystem )
	{
		bEnableZA1 = FALSE;
		bEnableZA2 = FALSE;
	}
	if( TRUE == m_sZolzFilePath.IsEmpty() )
	{
		bEnableZA1 = FALSE;
	}
//	GetDlgItem(IDC_STATIC_ZOLZ_ZA_TEXT)->EnableWindow(bEnableZA1);
	GetDlgItem(IDC_ZONE_AXIS_COMBO_1)->EnableWindow(bEnableZA1);
	// for the second file - enable Zone Axis if and only if we have the 2nd file and Crystal System
	if( TRUE == m_sFolzFilePath.IsEmpty() )
	{
		bEnableZA2 = FALSE;
	}
/*
	GetDlgItem(IDC_ZONE_AXIS_RADIO)->EnableWindow(bEnableZA2);
	// if we have a FOLZ as a choice - we cannot choose 2nd ZA
	if( 0 == m_nZAorFOLZ )
	{
		bEnableZA2 = FALSE;
	}
	GetDlgItem(IDC_ZONE_AXIS_COMBO_2)->EnableWindow(bEnableZA2);
*/
	// if the crystal system has been changed
	// and it is not UNKNOWN
	// fill in the list of possible zone axes
	if( m_nCrystalSystem != nCurCrSystem )
	{
		// remove all
		m_ctrlZoneAxis1.ResetContent();
		m_ctrlZoneAxis2.ResetContent();
		if( XS_Unknown != nCurCrSystem )
		{
			// find the corresponding Morniroli table
			const SpinningStar::Morniroli_table *pTables = SpinningStar::s_MorniroliTables;
			const SpinningStar::Morniroli_entry *pEntry;
			int i;
			while( pTables->pEntries )
			{
				// found the crystal system?
				if( pTables->nCrystalSystem == nCurCrSystem )
				{
					i = 1;
					pEntry = pTables->pEntries;
					if( TRUE == bEnableZA1 )
					{
						m_ctrlZoneAxis1.InsertString(0, _T("Unknown ZA"));
					}
					if( TRUE == bEnableZA2 )
					{
						m_ctrlZoneAxis2.InsertString(0, _T("Unknown ZA"));
					}
					while( SpinningStar::ZA_UNKNOWN != pEntry->nZA )
					{
						if( TRUE == bEnableZA1 )
						{
							m_ctrlZoneAxis1.InsertString(i, SpinningStar::pszZoneAxes[pEntry->nZA]);
						}
						if( TRUE == bEnableZA2 )
						{
							m_ctrlZoneAxis2.InsertString(i, SpinningStar::pszZoneAxes[pEntry->nZA]);
						}
						i++;
						pEntry++;
					}
					m_ctrlZoneAxis1.SetCurSel(0);
					m_ctrlZoneAxis2.SetCurSel(0);
					// finish
					break;
				}
				// next table entry
				pTables++;
			}
		}
	}

	m_nCrystalSystem = nCurCrSystem;

	OnSelchangeSymmList();
}

HBRUSH CSpinningStarStartDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	if( CTLCOLOR_STATIC == nCtlColor )
	{
		if( pWnd->GetSafeHwnd() == GetDlgItem(IDC_ZOLZ_CHOICE)->GetSafeHwnd() )
		{
			pDC->SetTextColor(RGB(255, 0, 0));
		}
		else if( pWnd->GetSafeHwnd() == GetDlgItem(IDC_FOLZ_CHOICE)->GetSafeHwnd() )
		{
			pDC->SetTextColor(RGB(0, 127, 0));
		}
	}
	
	// TODO: Return a different brush if the default is not desired
	return hbr;
}
