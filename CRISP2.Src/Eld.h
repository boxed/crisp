#pragma once

// ELD definitions
// used by ELD, PhIDO
#include "commondef.h"
#include "LinAlg.h"
#include "trace.h"
#include "image.h"

class ELD;
class OBJ;

// Added by Peter on [9/8/2005]
typedef std::vector<float>			VectorF;
typedef VectorF::iterator			VectorFIt;
typedef VectorF::const_iterator		VectorFCit;

typedef std::vector<double>			VectorD;
typedef VectorD::iterator			VectorDIt;
typedef VectorD::const_iterator		VectorDCit;

typedef std::vector<CVector2d>		Pt2Vector;
typedef Pt2Vector::iterator			Pt2VectorIt;
typedef Pt2Vector::const_iterator	Pt2VectorCit;

typedef std::vector<CVector3d>		Pt3Vector;
typedef Pt3Vector::iterator			Pt3VectorIt;
typedef Pt3Vector::const_iterator	Pt3VectorCit;

// Reduced cell data-types

typedef struct
{
	double cell[6];		// the reduced cell, angles are in radians
	double a_axis;		// the length of the a-axis
	double b_axis;		// the length of the b-axis
	CVector2d h_dir;	// the direction of h-axis
	CVector2d k_dir;	// the direction of k-axis
	CVector2d center;	// the center (usually refined)
	double a_angle;		// the angle where the a-axis was selected
	double b_angle;		// the angle where the b-axis was selected
	int nCount;			// number of same reduced cells met during the search
	CMatrix3x3 cb;		// changing the base matrix

} LATTICE_CELL;

typedef std::vector<LATTICE_CELL>	LatCellVec;
typedef LatCellVec::iterator		LatCellVecIt;
typedef LatCellVec::const_iterator	LatCellVecCit;

// end of addition by Peter [9/8/2005]
/*---------------------------------------------------------*/
typedef struct
{
	float	x;
	float	y;
	float	n;
	float	a;
	float	as;
	float	l;
	int		nOthers;	// shows how many other reflections give integer H & K
						// using h_dir & k_dir with this reflection

} ARITEM, * LPARITEM;

// ADDED BY PETER ON 3 AUG 2005
typedef std::vector<ARITEM>				EldAritemVec;
typedef EldAritemVec::iterator			EldAritemVecIt;
typedef EldAritemVec::const_iterator	EldAritemVecCit;
// END OF ADDITION

/*---------------------------------------------------------*/
typedef struct
{
	signed short	h;
	signed short	k;
	long/*short*/			i_amp;
	WORD			flags;
	long/*short*/			idummy;

	long/*short*/			num_2;		// No. of pixels with value > A/2
	long/*short*/			num_over;	// No. of pixels with value > over-saturation level
	float			*pix_table;	// pointer to the pixels table
	float			*integr_tbl;// pointer to the table of integrated values
	long/*short*/			tbl_size;
	double			noise;		// the background noise level
	double			circ_noise;		// the background noise level
	double			dEstA;	// the estimated intensity
#define	FLAG_UNDEFINED_CENTER			0x8000		// Undefined center
#define	FLAG_MISPLACED					0x4000		// Misplaced
#define	FLAG_FLAT_CENTER				0x2000		// Flat center
#define	FLAG_BAD_SHAPE					0x1000		// Bad shape
#define	FLAG_USER_DELETED 				0x0800		// Deleted by user
#define	FLAG_BELOW_BACKGROUND 			0x0400		// A-Bg<0
#define	FLAG_TOO_NOISY 					0x0200		// Too noisy
#define	FLAG_UNDEFINED_SHAPE_DERIVATIVE 0x0100		// Undefined shape derivative
#define	FLAG_ERC						0xF000		// I don't know what ERC is supposed to mean, I suspect the E is for "error" /Anders

	float	xl, yl	;	// position from latref
	float	xc, yc  ;	// position from centgrav
	float	xc0, yc0;	// predicted position after ubend
	float	bg	;		// background level
	float	a0	;		// amplitude of some kind? what is the difference of a0 and dEstA?
	float	ae	;		// amplitude, estimated
	float	anew;		// new integration procedure, added by Peter on 20 Maj 2003

	float	IntegrA;	// integrated intensity minus BKG
	float	Amp_bkg;	// AMPLITUDE minus BKG
	float	bkg	;		// BKG value
	float	circ_bkg;	// BKG value
	DWORD	*histo;		// pointer to the histogram if any

	float	sg	;
	float	dval;		// 10
	float	sr	;		// Refl radius at cutoff level	//11
	float	dummy[2];	// to get size = 64 bytes

} EREFL, * LPEREFL;

/*---------------------------------------------------------*/
// ADDED BY PETER ON 3 AUG 2005
typedef std::vector<EREFL>			EldReflVec;
typedef EldReflVec::iterator		EldReflVecIt;
typedef EldReflVec::const_iterator	EldReflVecCit;

#pragma pack(push, 1)

// all members are defined in ELD_HOLZ.cpp
class LaueZone
{
public:
//	LaueZone(int nZone, struct ELD_ *pEld);
	LaueZone(int nZone, ELD *pEld);
	~LaueZone();

//	void Init(int nZone, struct ELD_ *pEld);
	void Init(int nZone, ELD *pEld);
	void Deinit();
public:
	typedef enum
	{
		ZONE_UNINITIALIZED = 0,
		ZONE_MANUALLY_INDEXED = 1,
		ZONE_AUTO_INDEXED = 2,
		ZONE_INDEXED = 3,			// Just a mask for ZONE_MANUALLY_INDEXED and ZONE_AUTO_INDEXED
		ZONE_INT_EXTRACTED = 4,

	} LAUE_ZONE_FLAGS;

//protected:
//private:
	LPCSTR		m_pszImgName;
	int			m_nZoneNr;		// Laue zone number (0, 1, ...)
	DWORD		m_nFlags;		// flags, one of LAUE_ZONE_FLAGS for example
	double		m_dRadius;		// Laue zone limiting circle radius
	COLORREF	m_rgbColor;		// Zone color
	BOOL		bAuto;			// Auto lat ref
	EldReflVec	m_vRefls;		// zone reflections
	int			h[3];			// h index of first reflection
	int			k[3];			// k index of first reflection
	double		x[3];
	double		y[3];

//	CVector2 xy[3];				// coordinates of user-specified reflections
	HPEN		hPen;			// to paint

	// Lattice
	CVector2d	&h_dir;
	CVector2d	&k_dir;
	CVector2d	&center;

	double		m_adCell[6];	// real-space cell
	double		m_adCellR[6];	// reciprocal-space cell
	double		m_adRedCell[6];	// real-space reduced cell
	double		m_adConvCell[6];// real-space conventional cell
	CVector3d	m_vA;			// 3 real-space std::vector for the conventional cell
	CVector3d	m_vB;
	CVector3d	m_vC;
	CMatrix3x3	m_mChangeBase;	// the matrix for changing the base
	bool		m_bHaveCell3D;	// true if reduced and transformed
	char		m_sCellType[32];// triclinic, etc.
	char		m_sCentring[32];// P-primitive, etc.

	CVector2d	shift;			// the LZ shift
	double		m_dAvgRad;		// the average radius for the Laue Zone (to calculate c & c*)
	double		a_length, b_length, c_length;
	double		gamma;
	double		ral, rbl, rcl;	// Length in Angstroms
	double		rgamma;
	int			nLRefCnt;		// Number of the reflections used in lat ref

	int			nEstCount;		// Number of estimation passes
	int			nEstMax;		// Maximum number of estimations

	CVector2d GetPeakAbsXY(int H, int K)
	{
		return H*h_dir + K*k_dir + center + shift;
	}
	double GetPeakAbsX(int H, int K)
	{
		return H*h_dir.x + K*k_dir.x + shift.x + center.x;
	}
	double GetPeakAbsY(int H, int K)
	{
		return H*h_dir.y + K*k_dir.y + shift.y + center.y;
	}
	// Relative coordinates on the image
	CVector2d GetPeakRelXY(int H, int K)
	{
		return H*h_dir + K*k_dir + shift;
	}
	double GetPeakRelX(int H, int K)
	{
		return H*h_dir.x + K*k_dir.x + shift.x;
	}
	double GetPeakRelY(int H, int K)
	{
		return H*h_dir.y + K*k_dir.y + shift.y;
	}
};

#pragma pack(pop)

typedef std::vector<LaueZone*>		LaueZoneVec;
typedef LaueZoneVec::iterator		LaueZoneVecIt;
typedef LaueZoneVec::const_iterator LaueZoneVecCit;
// END OF ADDITION 3 AUG 2005

/*---------------------------------------------------------*/
class ProtocolOut
{
public:
	typedef enum
	{
		LATTICE_REFINEMENT = 1,
	} PROTOCOL_TYPE;

public:
	ProtocolOut();
// 	virtual void operator << (LPCSTR pszText) = 0;

	void Initialize(HWND hDialog);

	void SetHeader(LPCSTR pszHeader);
	void SetHeader(const std::string _string) { SetHeader(_string.c_str()); }
	void AddProtocolString(LPCSTR pszString);
	void AddProtocolString(LPCWSTR pszString);
	void AddProtocolString(const _tstd::string &_format);

	void ResetContent();
	int GetCount();
	void SetCurSel(int nCurSel);

protected:
	HWND m_hDialog;
	HWND m_hHeader;
	HWND m_hProtocolList;
	PROTOCOL_TYPE m_nType;
	std::string m_sHeader;
};

/*---------------------------------------------------------*/
class ELD
{
public:
	typedef std::vector<int>	Histo;
	typedef std::vector<Histo>	HistoArr;
	typedef HistoArr::iterator	HistoArrIt;
	typedef std::vector<DWORD>	DWVector;

public:
	ELD();
	~ELD();

	BOOL Initialize(OBJ* pObj);
	void Deinitialize();

	BOOL InternalCalibration();
	BOOL IntensitySummation();
	BOOL FitHisto(DWVector &vPlot, int nColLen, VectorD &vX, VectorD &vY, VectorD &vdParams);
	BOOL FitHistogram(VectorD &vX, VectorD &vdParams, Histo &histo, double &dScaleX);
	BOOL IsCloseToHisto(const VectorD &vX, const VectorD &vdParams,
		const Histo &histo, double &dChi2);
	int ExtractRefls(float rmin, float rmax, int nLaueZone);
	void RunSpinningStar();
	void DrawMark(float x, float y, int pn, int r);
	void DrawMark(float x, float y, HPEN pn, int r);
	void PaintELDDlg();
	int PeakCenter2(int r, float &xm, float &ym, float cd, float thr, int mode, double &amp, double &as);
	bool PeakCenter(int r, float &xm, float &ym, float cd, float thr, int mode, double &amp, double &as);
	void RecalculateUserRefls(LaueZone *pLZ, LPARITEM pRefls1 = NULL, int nSize = 0);

	void RecalculateRecCell(LaueZone *pLZ, BOOL bResetAllLaueZones);

	void MakeHisto();

	void Synth();

	// Protocol data
	void protocolLatAxies(int nrefl, int pass);//, ProtocolOut &output);
	void protocolLatRef();//ProtocolOut &output);
	BOOL ELD_RefineLattice(int nPass, double dErrPrec, double dLimitR,
		BOOL bCheckR, EldAritemVec *pvPeaks = NULL);
	BOOL ELD_FindClosestCenter(const CVector2d &mass_center, const CVector2d &h_dir,
		const CVector2d &k_dir, CVector2d &center);
	BOOL ELD_FindCenter(double &x0, double &y0);
	int ELD_DoPeakSearch(EldAritemVec &vRefls, int nWindowR, double dLimitRad, BOOL bCheckR);
	int ELD_DoPeakSearch2(EldAritemVec &vRefls, int nWindowR, double dLimitRad, BOOL bCheckR);
	BOOL ELD_AnalyzePeaks(const EldAritemVec &vPeaks, double dLimitR, BOOL bCheckR);
	BOOL ELD_CalculateUserLattice();
	void ELDSaveHKA( LPCSTR fname );

	void GetIndex3D(CVector3d &za, double h, double k, int nLZ, CVector3l &vOut);

	void UpdateFirstRefls(LPARAM lParam, int h, int k, WPARAM wParam);

	// HOLZ indexing and refinement
	BOOL DoAutoHOLZ();
	BOOL DoHOLZref(int nLaueZone);
	bool DoAutoHOLZrefinement(Pt2Vector &vPtObs, Pt2Vector &vPtCalc, VectorD &vParamsOut);
	void SetELDparamsFromHOLZRef(VectorD &vParams);

	// LATTICE REFINEMENT
	BOOL DoLatRef(int autoref);
	BOOL CalculateCell3D(LaueZone *pLZ);
	BOOL AnalyzeConvCell(LaueZone *pLZ, CVector2d &new_a, CVector2d &new_b);

	float CGauss(float s, float x);
	float XGauss(float s, float x);

	BOOL ProcessGoodRefls(EldReflVec &vGoodRefls, EldReflVec &vRefCurveRefls, std::vector<float> &vTables,
		std::vector<float> &vIntegrTables, std::vector<float> &vRefCurve);

	int GetReflInfo(LPEREFL lpR, BOOL pass1, BOOL bIntegrate);
	BOOL Calc2to3d(int &a, int &b, int &c);
	void FillReflList(int nLaueZone);
private:
//	BOOL ProcessStrongRefls(EldReflVec &vStrongRefls, std::vector<float> &vTables,
//		std::vector<float> &vIntegrTables, std::vector<float> &vRefCurve,
//		int arad, IMAGE &image, LPBYTE pMask, int nMaskSize);
	BOOL CalculateBandSum(LPEREFL lpR, int nClip, double &dSum);
	BOOL GetGoodRefls(EldReflVec &vGoodRefls, EldReflVec &vWeakRefls,
		EldReflVec &vStrNeighbRefls, double &nSigma, double dMaxVal);
//	BOOL EstimateTables(EldReflVec &vGoodRefls, std::vector<float> &vTables, int nSigma, LPVOID pData);
//	BOOL EstimateIntensity(LPEREFL lpR, LPEREFL lpRefR, const ELD::Histo &histo,
//		const VectorD &vX2, const VectorD &vdParams2);

public:
	OBJ*	m_pPar;
	OBJ*	m_pObj;

	ProtocolOut m_ProtocolOut;

	IMAGE	m_image;
	/*WORD*/double	m_mean;
	/*WORD*/double	m_min;
	/*WORD*/double	m_max;
	int		m_hkmax;
	int		m_hkmaxe;		// Maximum hk index for extraction
	BOOL	m_bSmartLR;	// Smart reflection coordinates
	LPBYTE	m_mask;
	std::vector<BYTE> m_vMask;

	int		m_nMaskW;
	int		m_nMaskH;

	typedef enum
	{
		ELD_INTEGRATE_SHAPE_FIT,
		ELD_INTEGRATE_COMMON_PROFILE,
		ELD_INTEGRATE_SIMPLE_SUMMATION,
	} ELD_INTEGRATION_MODE;

	ELD_INTEGRATION_MODE m_nIntegrationMode;

	LPFLOAT	m_as;
	LPFLOAT m_ds;
	LPFLOAT m_ws;
	LPFLOAT m_wn;
	LPFLOAT m_wa;

	int		m_nPeakWindow;		// for peak-search

	BYTE*	m_map;
	LPPOINT m_ppt;
	int		m_xms, m_yms;
	int		m_rms, m_rma;
	float	m_athr;
	float	m_athd;
	int		m_flags;
//#define	ELR_DONE	0x0001
//#define	ESR_DONE	0x0002
#define	MAP_EX		0x0010
#define	SH1_EX		0x0020
#define	SH1_EX		0x0020
#define	ELR_SYNTH	0x1000

	CVector2d	m_h_dir;		// ZOLZ axes and center
	CVector2d	m_k_dir;
	CVector2d	m_center;
	CVector2d	m_mass_center;
//-----------------------------------------------------
	LPEREFL	m_pCurRefl;
	int		m_r0;
	int		m_r1;
	float	m_sg;
	float	m_sgx;
	float	m_kf;
	float	m_gr1;
	float	m_wx0, m_wy0, m_wx1, m_wy1;
	float	m_zy;
	float	m_MeanSigma;
	float	m_UserSigma;
	float	m_ksg;
	float	m_ksg0;
	float	m_esdx, m_esdy;		// ESD of the LATREF results
	BOOL	m_bUseFixedSigma;
	BOOL	m_bProtocol;		// TRUE - protocol is ON
	int		m_last;			// Last updated string in protocol
	BOOL	m_bMarks;			// Preserve marks on image
	BOOL	m_bIndex;			// Draw index value
	BOOL	m_bNotListY;		// Do not list 'yellow' reflections
	int		m_arad;			// Average reflection radius
	int		m_uu[2],m_vv[2];	// 2D indices
	int		m_hh[2],m_kk[2],m_ll[2];//  3D indices
	int		m_uh, m_uk, m_ul;		//
	int		m_vh, m_vk, m_vl;		// Matrix for (u,v)->(h,k,l)
	BOOL	m_f2to3;			// Matrix is valid

	double	m_dvfrom, m_dvto;	// range for saving on d-value
	double	m_infrom, m_into;	// range for saving on intensities
	BOOL	m_bCentOnly;		// save only centered reflections
	float	m_cfoi;			// ratio curve fitting/integration
	float	m_meanampl;		// Mean amplitude observed

	int		m_n10Histo;			// the MAX value which has 10 or more pixels

	BOOL	m_bRefinedLattice;	// TRUE if lattice has been refined

	LaueZone *m_pZOLZ;			// ZOLZ is always present
	LaueZone *m_pCurLZ;			// pointer to current Laue Zone
	LaueZoneVec	m_vLaueZones;		// Laue zones

	BOOL	m_bDrawAxes;			// TRUE if lattice axes should be drawn

	HANDLE	m_handle;			// for SpinStar dialog call
	DWORD	m_id;				// .... same

	BOOL	IsCalibrated();
	// added by Peter on 4th Feb 2007
	// Protocol saves
#define  ELD_MAX_PASSES_LATREF	4
	struct PROTOCOL_REFL
	{
		PROTOCOL_REFL()
		{
			h = k = dval = 0;
		}
		float	h;
		float	k;
		float	dval;
		CVector2d pt2, pt2m;
	};

	struct PROTOCOL_PASS
	{
		PROTOCOL_PASS()
		{
			nRefls = 0;
		}
		CVector2d	vCenter;
		CVector2d	vAxisH;
		CVector2d	vAxisK;
		int			nRefls;
		std::vector<PROTOCOL_REFL> vRefls;
	};

	int m_nCurPass;
	PROTOCOL_PASS m_aPasses[ELD_MAX_PASSES_LATREF];
	// end of Peters addition on 4th Feb 2007

#define	ELD_MAX_LAUE_CIRCLES	2			// maximum number of Laue circles
	BOOL				m_bUseLaueCircles;	// TRUE if use Laue circles for indexing of EDP
	BOOL				m_bPaintLaueCirc;		// TRUE if paint, FALSE if not
	BOOL				m_bPaintHOLZaxes;		// TRUE if paint, FALSE if not
	std::vector<bool>	m_vbLaueCircles;		// shows if the corresponding circle was enabled
	int					m_nCurLaueZone;		// 0, 1, ... ELD_MAX_LAUE_CIRCLES-1
//	std::vector<double> vLaueCircles;		// list of Laue circle radii
//	std::vector<COLORREF> vLaueCircRGB;		// colors for Laue circles
//	std::vector<CVector2> vZOLZrefls;
//	ELD_MOUSE_MODE		nMouseMode;			// ELD_MOUSE_REFL or ELD_MOUSE_LAUE
	// END OF ADDITION ON 23 JULY 2005

};

typedef ELD * LPELD;

// ADDED BY PETER ON 1 AUG 2005
// Drawing functions
void XY2Sc( float &x, float &y, OBJ* O, float xc, float yc);
//int PeakCenter(OBJ* O, int r, float * xm, float * ym,
//			float cd, float thr, int mode, LPWORD amp, float &as);
//void Mark( OBJ* O, float x, float y, int pn, int r );
//void Mark( OBJ* O, float x, float y, HPEN pn, int r );
// void protocolLatAxies(OBJ* O, int nrefl, int pass);
// void protocolLatRef( OBJ* O );
//void RecalculateUserRefls(OBJ* lpObj, LaueZone *pLZ, LPARITEM pRefls1 = NULL, int nSize = 0);
//void RecalculateRecCell(OBJ* lpObj, LPELD lpEld, LaueZone *pLZ, BOOL bResetAllLaueZones);
// END OF ADDITION ON 1 AUG 2005

// Added by Peter on [5/7/2006]
BOOL ELD_ReadEldData(OBJ* pObj/*, vObjectsVec &vObjects*/);
// End of addition by Peter on [5/7/2006]

// Added by Peter on [7/4/2006]
// BOOL ELD_Findcenter(LPELD pEld, double &x0, double &y0);
// int ELD_DoPeakSearch(OBJ* pObj, EldAritemVec &vRefls, int nWindowR,
// 					 double dLimitRad, BOOL bCheckR);
// int ELD_DoPeakSearch2(OBJ* pObj, EldAritemVec &vRefls, int nWindowR,
// 					  double dLimitRad, BOOL bCheckR);
// BOOL ELD_AnalyzePeaks(OBJ* pObj, LPELD pEld, const EldAritemVec &vPeaks,
// 					  double dLimitR, BOOL bCheckR);
// BOOL ELD_RefineLattice(OBJ* pObj, LPELD pEld, int nPass, double dErrPrec,
// 					   double dLimitR, BOOL bCheckR, EldAritemVec *pvPeaks = NULL);
//BOOL ELD_CalculateUserLattice(OBJ* pObj, LPELD pEld);
//BOOL ELD_CalculateCell3D(OBJ* pObj, LPELD lpEld, LaueZone *pLZ);

// BOOL ELD_FindClosestcenter(const CVector2d &mass_center, const CVector2d &h_dir,
// 						   const CVector2d &k_dir, CVector2d &center);

void Buerger_ReduceCell(const LATTICE_CELL &pParams, LATTICE_CELL &pReduced);
void Buerger_ReduceCell(double *pdCell, double *pdRedCell, CMatrix3x3 &cb);
BOOL ConventionalCell(double *padCell, CMatrix3x3 &conv, char *pszCellType, char *pszCentring);

bool ELD_FftIndex(const EldAritemVec &vEldPeaks, LatCellVec &vRedCells, int nRotate);

// OLD ELD functions
BOOL ELD_AnalyzePeaks_old(OBJ* pObj, LPELD pEld, const EldAritemVec &vPeaks, int nPeaks,
					  EldAritemVec &vOutDist, int nClosestDist);
BOOL ELD_EstimateAB_old(LPELD pEld, EldAritemVec &vPeaks);
// end of addition by Peter [7/4/2006]
