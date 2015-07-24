/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.Lattice * by ML ***************************************************/
/****************************************************************************/

typedef signed	 char SGC;
#pragma pack(push, 1)

/****************************************************************************/
typedef struct	{		//
		SGC		h;		// H - index
		SGC		k;		// K - index
		SGC		l;		// h - index
		SGC		m;		// k - index
		DWORD	f;		// Flags
		PHASE	p;		// Phase
		double	a;		// Amplitude
		SGC		h0;		// Original H
		SGC		k0;		// Original K
		SGC		l0;		// Original h
		SGC		m0;		// Original k
		PHASE	p0;		// Original Phase
		double	a0;		// Original Amplitude
		double	x0;		// Original x
		double	y0;		// Original y
		int		qq;		// Amp/Noise
		double	d_val;	// d value
} REFL, * LPREFL;

#define	HK(R)	 (*(short*)(&(R)->h))
#define	LM(R)	 (*(short*)(&(R)->l))
#define	HKLM(R)	 (*(int*)(&(R)->h))
#define	HK0(R)	 (*(short*)(&(R)->h0))
#define	LM0(R)	 (*(short*)(&(R)->l0))
#define	HKLM0(R) (*(int*)(&(R)->h0))

#define RIQ		0x0000000F	// Q index (0-15)

#define RQUA	0x000000F0	// mask
#define	BCD		0x00000010	// |C - C0| > 1.4
#define	BCN		0x00000020	// Center not found (noice)
#define	BSN		0x00000040	// Ampl/Noise < 1
#define	BAMP	0x00000080	// Ampl < 0

#define	SABS	0x00000100	// Syst. absent reflection
#define	BPHU	0x00000200	// Phase diff. more than 45
#define	UADD	0x00000400	// Added by user
#define	UDEL	0x00000800	// Deleted by user

#define RSPC	0x00001000	// Special reflection
#define UNIC	0x00002000	// Reflection is unic
#define RIP1	0x00004000	// Phase sign has changed
#define RIP2	0x00008000	// Phase has added 180

#define	SYMT	0x00010000	// Has been symetrized


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#define	NUMSYMM	17+4	// 17 as usual, plus for which have two settings (pm, pg, cm, pmg)

#define	SYMM_P1		1
#define	SYMM_P2		2
#define	SYMM_PMa	3
#define	SYMM_PMb	4
#define	SYMM_PGa	5
#define	SYMM_PGb	6
#define	SYMM_CMa	7
#define	SYMM_CMb	8
#define	SYMM_PMM	9
#define	SYMM_PMGa	10
#define	SYMM_PMGb	11
#define	SYMM_PGG	12
#define	SYMM_CMM	13
#define	SYMM_P4		14
#define	SYMM_P4M	15
#define	SYMM_P4G	16
#define	SYMM_P3		17
#define	SYMM_P3M1	18
#define	SYMM_P31M	19
#define	SYMM_P6		20
#define	SYMM_P6M	21

struct SGINFO 
{
		double	Rr;		int		Nr;		// phase residual
		double	RFa;	int		NFa;	// Amplitude R value
		double	RFp;	int		NFp;	// Forbiden refls R value
		double	SF;
		PHASE	Bh, Bk;					// Origin refinement shift
		PHASE	Ch, Ck;					// Alternative origin shift
		PHASE	Sh, Sk;					// User abitrary shift
		double	al,  bl,  gamma;		// reciprocal lattice
		double	ral, rbl, rgamma;		// real lattice
		double	rcl, rdl, rgamma2;		// real supper lattice
		double	aw;		// a^x angle on the original image

		DWORD	flg;
#define	BAD_GAMMA	0x0001
#define	BAD_AXES	0x0002
#define	BAD_SYM		0x0004
#define	OR_DONE		0x0010
#define	RA_DONE		0x0020
#define	RR_DONE		0x0040
#define	RS_DONE		0x0080
#define	RF_DONE		0x0100
#define	OR_MAN		0x0800
#define	DM_NEG		0x1000
//#define	DM_REOR		0x2000
#define	DM_SH05		0x4000


		LPBYTE	rmap;		// residuals map
		int		rxs;		//
#define	RXS	64				// residuals map dimension

		LPBYTE	dmap;		// density map
		int		dxs;		//
#define	IXS	64				// density map size for OREF
#define	DXS	256				// density map size for DMAP

//		int		ds;
		double	p2a;   	double	scale;
		double	rot, shx, shy;	// dmap rotation nad shifts
		int		edge;		// Edge visible flag
		int		contmap;	// Contour map flag
		int		cmsteps;	// Number of steps in contour map
};
typedef SGINFO* LPSGINFO;


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
typedef struct {
	HWND	hFFTwnd;		// FFT object window
	OBJ*	F;				// FFT object
	FT2R	ft2;			// FFT

	BOOL	bDefRefl[4];	// User defined reflections flags
	POINTD	pDefRefl[4];	// User defined reflections coordinates
	int		H[4],K[4];		// User defined reflections indicies
	int		h[4],k[4];
	HWND	hwSpin;			// Spin control
	int		spinitem;		// item under editing (refl no)
	int		spinsubitem;	// subitem under editing (H,K,h,k)
	double	rad;			// Radius for refinement
	double	ax, ay;			// a axis
	double	bx, by;			// b axis
	double	ax0, ay0;		// a axis
	double	bx0, by0;		// b axis
	double	cx, cy;			// a' axis
	double	dx, dy;			// b' axis
	double	cx0, cy0;		// a' axis
	double	dx0, dy0;		// b' axis
	double	al, bl;			// a,b axies length
	double	cl, dl;			// a',b' axies length
	double	gamma;			// a^b
	double	cdgamma;		// c^d
	BOOL	bDone;			// Lattice has been refined
	int		num_dimensions;				// Number of dimensions
	UINT	LR_Mode;		// Mode flags

	UINT	Ex_Mode;		// Extraction Mode flags
		#define	EM_PHA_MODE		0x0001	// extract phases
		#define	EM_NOISE_MODE	0x0006	// extract noise
		#define	EM_MODE			0x0007	// mask for all extraction modes
		#define EM_AMPVSUM		0x0008	// Unic reflection amplitude is a vector sum
		#define EM_MAX			0x0010	// Unic reflection amplitude is a maximum amplitude
		#define EM_AVERAGE		0x0020	// Unic reflection amplitude is an average

	double	MaxAmp;			// Maximum amplitude
	int		nrefl;			// Number of relections
	LPREFL	rptr;			// pointer to reflections

	UINT	OR_Mode;		// Origin refinement mode
		#define	OR_WEIGHT	0x0001
		#define	OR_RESTR	0x0002
		#define	OR_RELAT	0x0004
//		#define	OR_MAPNS	0x0010
//		#define	OR_MAPCN	0x0020
//		#define	OR_MAPEX	0x0040
		#define	OR_SYNAMES	0x0100
		#define	OR_DMAPEX	0x1000

	LPSGINFO	SI;			// Info on each spg
	int			Spg;		// Space group number (1-p1,...)
	double		Thr;		// Threshold
	int			uu[2],vv[2];	// 2D indicies
	int			hh[2],kk[2],ll[2];//  3D indicies
	int			uh, uk, ul;		//
	int			vh, vk, vl;		// Matrix for (u,v)->(h,k,l)
	BOOL		f2to3;			// Matrix is valid

	HFONT		m_hFont;		// font reference, must not be destroyed in destructor
	int			m_nFontH;

} LATTICE, * LPLATTICE;
#define	GetLATTICE(O)	((LPLATTICE)(O->dummy))

#define	LR_AUTO		1
#define	LR_CENTAB	2
#define	LR_CENTab	4
#define	LR_DONE		8

#define	LR_SWAP		0x0010
#define	LR_NEGA		0x0020
#define	LR_NEGB		0x0040

#define	LR_CEN0		0x0100
#define	LR_CEN1		0x0200
#define	LR_CEXAB0	0x0400
#define	LR_CEXAB1	0x0800
#define	LR_CEXab0	0x4000
#define	LR_CEXab1	0x8000
/*-------------------------------*/
#define	ML_SWAPAB	1
#define	ML_NEGA		2
#define	ML_NEGB		4

#define	ML_CENTAB	8
#define	ML_SWAPab	0x10
#define	ML_NEGa		0x20
#define	ML_NEGb		0x40
#define	ML_CENTab	0x80
#define	ML_CENAB0	0x100
#define	ML_CENAB1	0x200
#define	ML_CENab0	0x400
#define	ML_CENab1	0x800

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
typedef struct	{		// size = 20
		int		h;
		int		k;
		PHASE	p;
		double	w;
} RESID, *LPRESID;

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
// latref.c
BOOL	lrCGPeakPos( FT2R& ft2, LPDOUBLE xc, LPDOUBLE yc, double r, LPDOUBLE shift, LPDOUBLE aver );
BOOL	lrMaxPeakPos( FT2R& ft2, LPDOUBLE xc, LPDOUBLE yc, double r, LPDOUBLE shift, LPDOUBLE aver );
double	lrAreaESD( FT2R& ft2, double xc, double yc, double r, double aver );
BOOL	lrCheckRefl( FT2R& ft2, LPDOUBLE xc, LPDOUBLE yc, double rmax );

int		rflLatticeRefine(LPLATTICE L, int go, int Mode);

BOOL	rflGetRefls(LPLATTICE L);

LPSTR	rflSymmName( int syn, BOOL type );

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
// DmapUtil.c
void	DMap_To_Img   ( LPSGINFO SI, LPVOID img, int imgw, int imgh);
void	DMap_To_Img256( LPSGINFO SI, LPVOID img, int imgw, int imgh);
void	RMap_To_Img	  ( LPLATTICE L, LPSGINFO SI, LPVOID img, int imgw, int imgh);
void	Get_CSym      ( LPLATTICE L, int w, int h, LPINT xc, LPINT yc);
int		Set_CSym	  ( LPLATTICE L, int w, int h, int xc, int yc, BOOL any);

void	Create_DMap	  ( LPLATTICE L, int flg);
void	Fract2Client  ( LPSGINFO SI, int imgw, int imgh, double af, double bf, LPLONG xc, LPLONG yc);
int		Get_DCenter	  (LPSGINFO SI, int imgw, int imgh,
						double * xc, double * yc, double * af, double * bf,
						double * dns0, double * dns1, int flg);

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
// symm.c
void	OriginRefine   ( LPLATTICE L );
BOOL	Asym           ( LPLATTICE L, int go);
void	ApplySymmetry  ( LPLATTICE L );
void	ApplyPhaseShift( LPLATTICE L );
void	CalcRFactor    ( LPLATTICE L );

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
// OREF.c
#define	DOR_EXTRACT	1
#define	DOR_TRYALL	2
#define	DOR_FULL	4
#define	DOR_SHORT	8

BOOL DoOriginRefine( HWND hDlg, OBJ* O, LPLATTICE L, UINT what );

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
#pragma pack(pop)
