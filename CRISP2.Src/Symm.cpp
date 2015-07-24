/****************************************************************************/
/* Copyright (c) 1998 SoftHard Technology, Ltd. *****************************/
/****************************************************************************/
/* CRISP2.symmetry * by ML **************************************************/
/****************************************************************************/

#include "StdAfx.h"

#include "commondef.h"
#include "resource.h"
#include "const.h"
#include "objects.h"
#include "shtools.h"
#include "globals.h"
#include "common.h"
#include "fft.h"
#include "ft32util.h"
#include "lattice.h"
#include "xutil.h"
// Added by Peter on 5 Sep 2001
#include "symm.h"

#pragma optimize ("",off)

// Moved to "Symm.h" by Peter on 5 Sep 2001
/*
typedef int MX2[2*3];

typedef struct
{
	LPSTR	name;		// Name like p1, p2,  pm, pg
	LPSTR	xname;		// Name like p1, p21, p12
	double	gamma;		// Gamma value: 90, 120, or 0
	BOOL	aeqb;		// a equals to b
	BOOL	Centric;	// Centric
	int		N;			// Number of symmetry matricies
	MX2		mx[12];		// Sym. matricies

} SYMMETRY, *LPSYMMETRY;
*/

/*static */SYMMETRY Symmetry[NUMSYMM+1] = {	// Where 0 and 1 are P1
// p1
{	"p1     ",	"p1     ",	0.,		FALSE, FALSE, 1,
	{ {1,0,0,  0,1,0} }
},

// p1
{	"p1     ",	"p1     ",	0.,		FALSE, FALSE, 1,
	{ {1,0,0,  0,1,0} }
},

// p2
{	"p2     ",	"p21    ",	0.,		FALSE, FALSE, 2,
	{ {1,0,0, 0,1,0},  {-1,0,0, 0,-1,0} }
},

// pm,  mTx
//{	"pm  m_x",	"p12    ",	90.,	FALSE, FALSE, 2,
{	"pm  m_x",	"p12    ",	90.,	FALSE, FALSE, 2,
	{ {1,0,0, 0,1,0},  {-1,0,0, 0,1,0} }
},
// pm,  mTy
//{	"pm  mƒy",	"p12    ",	90.,	FALSE, FALSE, 2,
{	"pm  m_y",	"p12    ",	90.,	FALSE, FALSE, 2,
	{ {1,0,0, 0,1,0},  {1,0,0, 0,-1,0} }
},

// pg,  gTx
// {	"pg  gƒx",	"p12¹   ",	90.,	FALSE, FALSE, 2,
{	"pg  g_x",	"p121   ",	90.,	FALSE, FALSE, 2,
	{ {1,0,0, 0,1,0},  {-1,0,0, 0,1,1} }
},
// pg,  gTy
// {	"pg  gƒy",	"p12¹   ",	90.,	FALSE, FALSE, 2,
{	"pg  g_y",	"p121   ",	90.,	FALSE, FALSE, 2,
	{ {1,0,0, 0,1,0},  {1,0,1, 0,-1,0} }
},

// cm,  mTx
// {	"cm  mƒx",	"c12    ",	90.,	FALSE, TRUE, 4,
{	"cm  m_x",	"c12    ",	90.,	FALSE, TRUE, 4,
	{ {1,0,0, 0,1,0},  {1,0,1, 0,1,1}, {-1,0,0, 0,1,0}, {-1,0,1, 0,1,1} }
},
// cm,  mTy
//{	"cm  mƒy",	"c12    ",	90.,	FALSE, TRUE, 4,
{	"cm  m_y",	"c12    ",	90.,	FALSE, TRUE, 4,
	{ {1,0,0, 0,1,0},  {1,0,1, 0,1,1}, {1,0,0, 0,-1,0}, {1,0,1, 0,-1,1} }
},

// pmm
{	"pmm    ",	"p222   ",	90.,	FALSE, FALSE, 4,
	{ {1,0,0, 0,1,0},  {-1,0,0, 0,-1,0}, {-1,0,0, 0,1,0}, {1,0,0, 0,-1,0} }
},

// Changed symmetry operations betw PMG (mTx) and PMG (mTy) by Peter on 24 Aug 2001
// pmg,  mTx
// {	"pmg mƒx",	"p22¹2  ",	90.,	FALSE, FALSE, 4,
{	"pmg m_x",	"p2212  ",	90.,	FALSE, FALSE, 4,
	{ {1,0,0, 0,1,0},  {1,0,1, 0,-1,0}, {-1,0,0, 0,-1,0}, {-1,0,1, 0,1,0} }
},

// pmg,  mTy
// {	"pmg mƒy",	"p22¹2  ",	90.,	FALSE, FALSE, 4,
{	"pmg m_y",	"p2212  ",	90.,	FALSE, FALSE, 4,
	{ {1,0,0, 0,1,0},  {-1,0,0, 0,1,1}, {-1,0,0, 0,-1,0}, {1,0,0, 0,-1,1} }
},

// pgg
// {	"pgg    ",	"p22¹2¹ ",	90.,	FALSE, FALSE, 4,
{	"pgg    ",	"p22121 ",	90.,	FALSE, FALSE, 4,
	{ {1,0,0, 0,1,0},  {-1,0,1, 0,1,1}, {-1,0,0, 0,-1,0}, {1,0,1, 0,-1,1} }
},

// cmm
{	"cmm    ",	"c222   ",	90.,	FALSE, TRUE, 8,
	{ {1,0,0, 0,1,0},  {1,0,1, 0,1,1}, {-1,0,0, 0,-1,0}, {-1,0,0, 0,1,0},
	  {-1,0,1, 0,-1,1},  {1,0,0, 0,-1,0}, {-1,0,1, 0,1,1}, {1,0,1, 0,-1,1}}
},

// p4
{	"p4     ",	"p4     ",	90.,	TRUE,  FALSE, 4,
	{ {1,0,0, 0,1,0},  {0,-1,0, 1,0,0}, {-1,0,0, 0,-1,0}, {0,1,0, -1,0,0} }
},

// p4m
{	"p4m    ",	"p422   ",	90.,	TRUE,  FALSE, 8,
	{ {1,0,0, 0,1,0},  {0,-1,0, 1,0,0}, {-1,0,0, 0,-1,0}, {0,1,0, -1,0,0},
	  {-1,0,0, 0,1,0},  {0,1,0, 1,0,0}, {1,0,0, 0,-1,0}, {0,-1,0, -1,0,0} }
},

// p4g
// {	"p4g    ",	"p42¹2  ",	90.,	TRUE,  FALSE, 8,
{	"p4g    ",	"p4212  ",	90.,	TRUE,  FALSE, 8,
	{ {1,0,0, 0,1,0},  {0,1,1, 1,0,1}, {-1,0,0, 0,-1,0}, {0,-1,0, 1,0,0},
	  {0,-1,1, -1,0,1},  {0,1,0, -1,0,0}, {-1,0,1, 0,1,1}, {1,0,1, 0,-1,1} }
},

// p3
{	"p3     ",	"p3     ",	120.,	TRUE,  FALSE, 3,
	{ {1,0,0, 0,1,0},  {0,-1,0, 1,-1,0}, {-1,1,0, -1,0,0} }
},

// p3m1
{	"p3m1   ",	"p312   ",	120.,	TRUE,  FALSE, 6,
	{ {1,0,0, 0,1,0},  {0,-1,0, 1,-1,0}, {-1,1,0, -1,0,0},
	  {0,-1,0, -1,0,0},  {1,0,0, 1,-1,0}, {-1,1,0, 0,1,0} }
},

// p31m
{	"p31m   ",	"p321   ",	120.,	TRUE,  FALSE, 6,
	{ {1,0,0, 0,1,0},  {0,-1,0, 1,-1,0}, {-1,1,0, -1,0,0},
	  {0,1,0, 1,0,0},  {-1,0,0, -1,1,0}, {1,-1,0, 0,-1,0} }
},

// p6
{	"p6     ",	"p6     ",	120.,	TRUE,  FALSE, 6,
	{ {1,0,0, 0,1,0},  {0,-1,0, 1,-1,0}, {-1,1,0, -1,0,0},
	  {-1,0,0, 0,-1,0},  {0,1,0, -1,1,0}, {1,-1,0, 1,0,0} }
},

// p6m
{	"p6m    ",	"p622   ",	120.,	TRUE,  FALSE, 12,
	{ {1,0,0, 0,1,0},  {0,-1,0, 1,-1,0}, {-1,1,0, -1,0,0},
	  {0,1,0, 1,0,0},  {-1,0,0, -1,1,0}, {1,-1,0, 0,-1,0},
	  {-1,0,0, 0,-1,0},  {0,1,0, -1,1,0}, {1,-1,0, 1,0,0},
	  {0,-1,0, -1,0,0},  {1,0,0, 1,-1,0}, {-1,1,0, 0,1,0} }
}
// ƒ  - is 'T' upside down
};

typedef struct
{
	int  M;      // Multiplicity
	int  N;      // Number of equivalent hkl to follow
	int  h[12];  // If hkl == 000 M = N = 1
	int  k[12];  // If hkl != 000 M = 2 * N
	int  TH[12]; // Phase shift relative to h[0], k[0], l[0]
				 //List of hkl does not contain friedel mates
} EQ_HK, * LPEQ_HK;
typedef EQ_HK const * LPCQ_HK;

#define STBF 12 // Seitz           Matrix Translation Base Factor

int		rflIsSysAbsent     (int syn, int h, int k, LPINT TH_Restriction);
int		rflBuildEq         (int syn, LPEQ_HK Eq_hk, int h, int k);
int		rflAreSymEquivalent(int syn, int h1, int k1, int h2, int k2);

/*******************************************************************************/
LPSTR rflSymmName( int syn, BOOL type )
{
	if (type) return Symmetry[syn].xname;
	else      return Symmetry[syn].name;
}
/*******************************************************************************/
int rflIsSysAbsent(int syn, int h, int k, LPINT TH_Restriction)
{
	int		i, mh, mk, hm, km;
	int		TH, THr, FlagMismatch;
	int		*pmx;
	LPSYMMETRY	sm = &Symmetry[syn];

	mh = -h; mk = -k;

	/* check list of symmetry operations
	 take care of lattice type and "centric" flag */

	THr = -1;
	if (TH_Restriction != NULL) *TH_Restriction = THr;
	FlagMismatch = 0;

	for (i=0; i < sm->N; i++ ) {

		pmx=sm->mx[i];

		hm = pmx[0] * h + pmx[3] * k;
		km = pmx[1] * h + pmx[4] * k;

		TH =  pmx[2] * h * 6 + pmx[5] * k * 6;
		TH %= STBF; if (TH < 0) TH += STBF;

		if (mh == hm && mk == km) {
			if (THr < 0) THr = TH;
			else if (THr != TH)
				FlagMismatch = 1; /* must be systematic absent */
												  /* will check later ...      */
		} else if ( h == hm &&  k == km ) {
			if (TH != 0) return  (i + 1);
		}
	}

	if (THr >= 0 && FlagMismatch) /* ... consistency check */
		MessageBox( MainhWnd, "rflIsSysAbsent_hkl error", "rfl", MB_ICONSTOP );

	if (TH_Restriction != NULL) {
/*		if (sm->Centric == -1) *TH_Restriction = 0;
		else   */                *TH_Restriction = THr;
	}

	return 0;
}
/*******************************************************************************/
int		rflBuildEq(int syn, LPEQ_HK Eq_hk, int h, int k)
{
	int		i, j, hm, km;
	EQ_HK	BufEq_hk;
	int		*pmx;
	LPSYMMETRY	sm = &Symmetry[syn];

	if (Eq_hk == NULL) Eq_hk = &BufEq_hk;

	Eq_hk->M = 1;
	Eq_hk->N = 1;
	Eq_hk->h[0] = h;
	Eq_hk->k[0] = k;
	Eq_hk->TH[0] = 0;

	if (! (h || k))	return Eq_hk->M; /* this is 00 */

	Eq_hk->M++;

  /* check list of symmetry operations */
  /* skip first = identity matrix */

	for (i=1; i < sm->N; i++ ) {
		pmx=sm->mx[i];

		hm = pmx[0] * h + pmx[3] * k;
		km = pmx[1] * h + pmx[4] * k;

		for (j=0; j<Eq_hk->N; j++) {
		  if (    hm == Eq_hk->h[j] &&  km == Eq_hk->k[j]
			  || -hm == Eq_hk->h[j] && -km == Eq_hk->k[j] )
			break;
		}

		if (j == Eq_hk->N) {
			if (Eq_hk->N >= 24) {
				//MessageBox( MainhWnd, "rflBuildEq_hkl error (1)", "rfl", MB_ICONSTOP );
				return 0;
			}

			Eq_hk->h[j] = hm;
			Eq_hk->k[j] = km;

			Eq_hk->TH[j] = ( pmx[2] * h * 6 + pmx[5] * k * 6 ) % STBF;
			if (Eq_hk->TH[j] < 0) Eq_hk->TH[j] += STBF;

			Eq_hk->M += 2;
			Eq_hk->N++;
		}
	}

	if (sm->N % Eq_hk->N) /* another error trap */ {
		MessageBox( MainhWnd, "rflBuildEq_hkl error (2)", "rfl", MB_ICONSTOP );
		return 0;
	}

	return Eq_hk->M;
}
/*******************************************************************************/
int		rflAreSymEquivalent(int syn, int h1, int k1, int h2, int k2)
{
int     i, mh2, mk2, hm, km;
int		*pmx;
LPSYMMETRY	sm = &Symmetry[syn];

	mh2 = -h2;
	mk2 = -k2;

	/* check list of symmetry operations */
	for (i = 0; i < sm->N; i++) {
		pmx=sm->mx[i];
		hm = pmx[0] * h1 + pmx[3] * k1;
		km = pmx[1] * h1 + pmx[4] * k1;
		if      ( h2 == hm &&  k2 == km ) return  (i + 1);
		else if (mh2 == hm && mk2 == km ) return -(i + 1);
	}
	return 0;
}
/*******************************************************************************/
int	_cdecl cmpHK( const void * av, const void * bv )
{
	LPREFL	a = (LPREFL)av;
	LPREFL	b = (LPREFL)bv;
	register int hh;

	hh = abs(a->h) - abs(b->h);	if (hh) return hh;
	hh = abs(a->k) - abs(b->k);	if (hh) return hh;
	hh = a->h - b->h;          	if (hh) return -hh;
	hh = a->k - b->k;          	if (hh) return -hh;

	hh = abs(a->h0) - abs(b->h0);	if (hh) return hh;
	hh = abs(a->k0) - abs(b->k0);	if (hh) return hh;
	hh = a->h0 - b->h0;          	if (hh) return -hh;
	hh = a->k0 - b->k0;          	if (hh) return -hh;
	return 0;
}
/*******************************************************************************/
int	_cdecl cmpAMP( const void * av, const void * bv )
{
	LPREFL	a = (LPREFL)av;
	LPREFL	b = (LPREFL)bv;
	double	dd;
	dd = a->a - b->a;
	if( FastAbs(dd) > 1e-10 )
	{
		if (dd<0) return 1;
		else	  return -1;
	}
	return 0;
}
/*******************************************************************************/
int	_cdecl cmpDHK( const void * av, const void * bv )
{
	LPREFL	a = (LPREFL)av;
	LPREFL	b = (LPREFL)bv;
	register int hh;
	double	dd;

	dd = a->d_val-b->d_val;
	if( FastAbs(dd)>1e-10 )
	{
		if (dd<0) return -1;
		else	  return 1;
	}

	hh = abs(a->h) - abs(b->h);	if (hh) return hh;
	hh = abs(a->k) - abs(b->k);	if (hh) return hh;

	hh = a->h - b->h;          	if (hh) return -hh;
	hh = a->k - b->k;          	if (hh) return -hh;
	return 0;
}
/*******************************************************************************/
int	_cdecl cmpPOSHK( const void * av, const void * bv )
{
	register int hh=0;
	LPREFL	a = (LPREFL)av;
	LPREFL	b = (LPREFL)bv;

	if (a->h>=0) hh++; if (a->k>=0) hh++;
	if (b->h>=0) hh--; if (b->k>=0) hh--;
	if (hh) return -hh;
	return cmpHK( a, b );
}
/*******************************************************************************/
static BOOL	rflIsFriedel( LPREFL R )
{
	return (R->h<0 || (R->h==0 && R->k<0));
}
/*******************************************************************************/
static BOOL	rflUnFriedel( LPREFL R )
{
	if (rflIsFriedel(R))
	{
		R->h = -R->h; R->k = -R->k;
		R->p = -R->p;
		return TRUE;
	}
	return FALSE;
}
/*******************************************************************************/
void	rflAsymUnit(int syn, LPREFL R, int nrefl )
{
EQ_HK	eq;
int		i,j;
const LPREFL	R0___=R;
REFL	tst[24];
const LPREFL	R0=R;
int		restr;

	if (R0___ == NULL)
	{
		int asd = 0;
	}

	for(i = 0; i < nrefl; i++, R++) {
		if (R0___ == NULL)
		{
			int asd = 0;
		}
		BitClr(R->f,(RIP1|RIP2|UNIC|SABS|RSPC));
		if (R0___ == NULL)
		{
			int asd = 0;
		}
		memset(tst,0,sizeof(tst));

		if (R0___ == NULL)
		{
			int asd = 0;
		}
		if (rflBuildEq(syn, &eq, R->h0, R->k0) == 0)
			continue; // rflBuildEq failed
		for(j = 0; j < eq.N; j++) {
			tst[j].h = eq.h[j];
			tst[j].k = eq.k[j];
			tst[j].p = D2P(P2D(R->p0)-eq.TH[j]*30.);
			if (eq.TH[j])  tst[j].f |= RIP2;
			if (rflUnFriedel( &tst[j] )) tst[j].f ^= RIP1;
		}
		if (R0___ == NULL)
		{
			int asd = 0;
		}
		if (eq.N >= 24)
		{
			int asd = 0;
		}
		qsort(&tst, eq.N, sizeof(REFL), cmpPOSHK );
		if (R0___ == NULL)
		{
			int asd = 0;
		}
		R->h =tst[0].h; R->k=tst[0].k;
		R->p=tst[0].p;	R->f|=tst[0].f;
		if (R->h0==R->h && R->k0==R->k) R->f |=  UNIC;
		if (R0___ == NULL)
		{
			int asd = 0;
		}
		if (rflIsSysAbsent(syn, R->h, R->k, &restr)) R->f |=  SABS;
		if (R0___ == NULL)
		{
			int asd = 0;
		}
		if (restr>=0) R->f |= RSPC;
		if (R0___ == NULL)
		{
			int asd = 0;
		}
	}
	if (0) {
		//FILE *ff=fopen("C:/rr.txt","w");
		R=R0;
		for(i=0;i<nrefl;i++,R++) {
			Trace(_FMT("%3d %3d %3d %3d %8.0f %8.0f %5.0f %5.0f %04X",
					R->h0,R->k0,R->h,R->k,R->a0,R->a,P2D(R->p0),P2D(R->p),R->f));
			/*if (Bit(R->f,SABS)) fprintf (ff, " SABS");
			if (Bit(R->f,UNIC)) fprintf (ff, " UNIC");
			if (Bit(R->f,RSPC)) fprintf (ff, " RSPC");
			if (B1it(R->f,RIP1)) fprintf (ff, " ----");
			if (Bit(R->f,RIP2)) fprintf (ff, " +180");
			fprintf (ff, "\n");*/
		}
	}
	if (R0___ == NULL)
	{
		int asd = 0;
	}
	if (nrefl != 0)
		qsort(R0___, nrefl, sizeof(REFL), cmpHK );
}
/*******************************************************************************/
void	Expand_Refls( LPLATTICE L )
{
	int		hk, i, p;
	LPREFL	R = L->rptr;
	LPREFL	R1;

	i = L->nrefl;
	while(i > 0)
	{
		hk = HK(R); R1 = R;
		while( hk == HK(R1))
			if(hk == HK0(R1)) break; else R1++;
		while( hk == HK(R))
		{
			if((hk == HK(R1)) && (R1->a0 > 1.) && (R->a0 < 1.))
			{
				R->a = R->a0 = R1->a0;
				p = R1->p;
				if(R->f & RIP1) p = -p;	if(R->f & RIP2) p += P_PI;//0x8000;
				R->p = R->p0 = p;
				R->f &= ~(RIQ | RQUA /*| UDEL | UADD*/);
			}
			R++; i--;
		}
	}
}
/*******************************************************************************/
BOOL Asym( LPLATTICE L, int tryall)
{
	int			f=0;
	double		aw,fontWidth,fontHeight,x, al,	bl, gamma, ral, rbl;
	// Added by Peter on [18/8/2005]
	double		dSin, dCos;
	// end of addition by Peter [18/8/2005]
	LPREFL		R = L->rptr;
	LPSGINFO	SI = L->SI+L->Spg;

	if(L->num_dimensions == 1) {
		L->bx = -L->ay; L->by = L->ax; L->gamma = (double)(M_PI/2.); L->bl = L->al;
	}
	al = L->al;	bl = L->bl;	gamma  = R2D(L->gamma);
	x = FastAbs((double)L->ft2.S / (L->ax*L->by - L->ay*L->bx));
	ral = x * L->bl;
	rbl = x * L->al;
	SI->rcl = x * L->cl;
	SI->rdl = x * L->dl;
	SI->rgamma2 = D2R(180.-R2D(L->cdgamma));
	SI->aw  = ATAN2(-L->bx * x, L->by * x);

	// added by Peter on 19 Oct 2001 - solves mirror while DMAP
	if( gamma < 0 )
		gamma = -gamma;
	else if( gamma > 180 )
		gamma = 360 - gamma;
	// finidh added by Peter on 19 Oct 2001

	if(L->Spg >= SYMM_P3) {
		if(FastAbs(gamma-60.) > 10.) f |= BAD_GAMMA;
		gamma	= 60.;
	}
	else if(L->Spg > SYMM_P2) {
		if(FastAbs(gamma-90.) > 10.) f |= BAD_GAMMA;
		gamma	= 90.;
	}
	if(L->Spg >= SYMM_P4) {
		if((al > 1.2*bl) || (bl > 1.2*al)) f |= BAD_AXES;
		al = bl = (al+bl)/2.;	ral = rbl = (ral+rbl)/2.;
	}

	SI->al = al; SI->bl = bl; SI->gamma = D2R(gamma);
	SI->ral = ral; SI->rbl = rbl;
	SI->rgamma = D2R(180.-gamma);

	// Added by Peter on [18/8/2005]
	aw = FastAtan2(L->ax, L->ay);
	FastSinCos(-aw, dSin, dCos);
	fontWidth = L->bx*dCos - L->by*dSin;
	fontHeight = L->bx*dSin + L->by*dCos;
	x  = FastAtan2(fontWidth, fontHeight);
	// end of addition by Peter [18/8/2005]
//	aw = ATAN2(L->ay, L->ax);
//	fontWidth = L->bx*cos(-aw) - L->by*sin(-aw);
//	fontHeight = L->bx*sin(-aw) + L->by*cos(-aw);
//	x  = ATAN2(fontHeight,fontWidth);
	if(x < 0)
		SI->rgamma *= -1;

	SI->flg &= ~(BAD_SYM | BAD_GAMMA | BAD_AXES);

	if( (L->LR_Mode & LR_CENTAB)!=0 &&
		(L->Spg != SYMM_P1) != 0 &&
		(L->Spg != SYMM_CMa) &&
		(L->Spg != SYMM_CMb) &&
		(L->Spg != SYMM_CMM)) f |= BAD_SYM;
	if(f) {
		SI->flg |= f | BAD_SYM;
		if(tryall) return FALSE;
	}

	rflAsymUnit( L->Spg, L->rptr, L->nrefl );

//	if(go & XRFL) Expand_Refls(L);
//	Expand_Refls(L);
	return TRUE;
}
/*******************************************************************************/
void	OriginRefine(LPLATTICE L)
{
LPRESID	S = NULL, S0 = NULL;
LPREFL	R1 = NULL, R2 = NULL, R = L->rptr;
LPBYTE	map = NULL;
WORD	d = 0;
int		mode = L->OR_Mode;
int		bh, bk, bh1, bk1, p, p2, i, j, f, n, ns=0;
double	t1, t, a, am, z, wgt = 0.001;
int		fa = BCD | BCN | BSN | BAMP | UDEL | SABS;
int		h1,h2,k1,k2,hk=111;
int		nrefl = L->nrefl;
LPSGINFO	SI = L->SI+L->Spg;

#define	MAXRESID	6000
#define RELWGT	1.		// it was 0.017
#define	ONE_32	0x4000000
#define ONE_16  0x8000000

//	if(SI->flg & OR_DONE) return;
	SI->flg &= ~(OR_DONE | RF_DONE);
	am = (double)L->Thr /* sometime ago it was in percent: ( * L->MaxAmp/100.) */;
	SI->Bh = SI->Bk = 0; map = SI->rmap;

	if ((S0=S=(LPRESID)calloc(MAXRESID, sizeof(RESID)))==NULL) return;

	for(i = nrefl; i>0; i--)
	{
		if(hk != HK(R))
		{
			R1 = R; hk = HK(R);
			if(mode & OR_RELAT)
			while(hk == HK(R1) )
			{
				f = R1->f; t1 = R1->a0;
				if((LM0(R1))||(f & fa)||(t1<am)) {R1++; continue;}
				h1 = R1->h0; k1 = R1->k0; p = R1->p;
				if(f & RIP1) {h1=-h1; k1=-k1;}
		if (L->Spg==SYMM_PMM || L->Spg==SYMM_P4G)  if(R1->x0 < 0) {h1=-h1; k1=-k1; p = -p;}
				R1++; R2 = R1;
				while( hk == HK(R2) )
				{
					t = R2->a0; f = R2->f;
					if((LM0(R2))||(t<am)||(f & fa)) goto sss;
					t = FastSqrt(t1*t)*RELWGT;
					h2 = R2->h0; k2 = R2->k0; p2 = R2->p;
					if(f & RIP1) {h2=-h2; k2=-k2;}
		if (L->Spg==SYMM_PMM || L->Spg==SYMM_P4G) if(R2->x0 < 0) {h2=-h2; k2=-k2; p2 = -p2;}
					if ((mode & 1)== 0) t = 1.;
					S->p = p - p2;
					S->h = (h1-h2)*ONE_32;	S->k = (k1-k2)*ONE_32;
					S->w = t;	wgt += t;	S++;	ns++;
					if(ns>=MAXRESID) goto xxx;
				sss: R2++;
				}
			}
		}
		f = R->f;
		if( (LM0(R)==0) && (mode & OR_RESTR) && ((f & fa) == 0) && (f & RSPC) )
		{
			if((t=R->a0) > am)
			{
				if ((mode & 1) == 0) t = 1.;
				p2 = R->p0*2;	//if(f & RIPT) p2 += P_PI;
				h2 = R->h0 * ONE_16; k2 = R->k0 * ONE_16;
				S->p = p2; S->h = h2; S->k = k2; S->w = t/2.;
				S++;	ns++;	wgt += t;
				if(ns>=MAXRESID) goto xxx;
			}
		}
		R++;
	}
xxx:
//-------------------------------------------------------
	bh = bk = 0;
	if(ns==0) goto xx4;
	z = 1e34; t = wgt*64.*65536.; t1=0.;
	// residuals map should be transposed (h- k|)
	for(i=0; i<64; i++)
	{
		S = S0;	map = SI->rmap+i;
		for(j=0; j<64; j++)
		{
			Residual(S, ns, &a);
			if(map)
			{
				d = (WORD)(a/t); if(d>255) d=255;
				*map = (BYTE)d ^ 255;
				map += 64;
			}
			if(a < z+t1) {z = a; bh = i; bk = j;}
		}
		for(n=0; n<ns; n++){ S->p += S->h; S++;}
	}
	S = S0;	if(bh>=32) bh -= 64; if(bk>=32) bk -=64;
	for(i=0; i<ns; i++)
	{
		S->p +=  S->h * (bh-1) + S->k*(bk-1);	// -1!!
		S->h /=16; S->k /=16; S++;
	}
	bh1 = bk1 = 0;

	for(i=-16; i<16; i++) {	// 16!!
		S = S0;
		for(j=-16; j<16; j++) {	// 16!!
			Residual(S, ns, &a);
			if(a < z+t1)
			 {z = a; bh1 = i; bk1 = j;}
		}
		for(n=0; n<ns; n++)
			{ S->p += S->h - S->k * 32; S++;}	// 32!!
	}
//-------------------------------------------------------
	bh = (1024<<16)*bh + (64<<16)*bh1;	// 64<<16
	bk = (1024<<16)*bk + (64<<16)*bk1;
xx4:
	SI->Bh = bh;
	SI->Bk = bk;
	SI->flg |= OR_DONE;
	free( S0 );
}

#if 0
void	OriginRefine(LPLATTICE L)
{
	LPRESID	S, S0;
	LPREFL	R1, R2, R = L->rptr;
	LPBYTE	map, srcmap;
	WORD	d;
	int		mode = L->OR_Mode;
	int		bh, bk, bh1, bk1, p, p2, i, j, f, n, ns=0, y_off;
	double	t1, t, a, am, z, wgt = 0.001;
	int		fa = BCD | BCN | BSN | BAMP | UDEL | SABS;
	int		h1,h2,k1,k2,hk=111;
	int		nrefl = L->nrefl;
	LPSGINFO	SI = L->SI+L->Spg;
	std::vector<BYTE> vMap(180*180);
	double dx, dy, _xcur, _ycur;

	// MAXRESID CHANGED BY PETER ON 15 APRIL 2005
//#define	MAXRESID	6000
#define	MAXRESID	32768
#define RELWGT	1.		// it was 0.017
#define	ONE_32	0x4000000
#define ONE_16  0x8000000

//	if(SI->flg & OR_DONE) return;
	SI->flg &= ~(OR_DONE | RF_DONE);
	am = (double)L->Thr /* sometime ago it was in percent: ( * L->MaxAmp/100.) */;
	SI->Bh = SI->Bk = 0; map = SI->rmap;

	if( NULL == (S0=S=(LPRESID)calloc(MAXRESID, sizeof(RESID))) )
		return;

	for(i = nrefl; i>0; i--)
	{
		if(hk != HK(R)) {
			R1 = R; hk = HK(R);
			if(mode & OR_RELAT)
			while(hk == HK(R1) ) {
				f = R1->f; t1 = R1->a0;
				if((LM0(R1))||(f & fa)||(t1<am)) {R1++; continue;}
				h1 = R1->h0; k1 = R1->k0; p = R1->p;
				if(f & RIP1) {h1=-h1; k1=-k1;}
		if (L->Spg==SYMM_PMM || L->Spg==SYMM_P4G)  if(R1->x0 < 0) {h1=-h1; k1=-k1; p = -p;}
				R1++; R2 = R1;
				while( hk == HK(R2) ) {
					t = R2->a0; f = R2->f;
					if((LM0(R2))||(t<am)||(f & fa)) goto sss;
					t = FastSqrt(t1*t)*RELWGT;
					h2 = R2->h0; k2 = R2->k0; p2 = R2->p;
					if(f & RIP1) {h2=-h2; k2=-k2;}
		if (L->Spg==SYMM_PMM || L->Spg==SYMM_P4G) if(R2->x0 < 0) {h2=-h2; k2=-k2; p2 = -p2;}
					if ((mode & 1)== 0) t = 1.;
					S->p = p - p2;
					S->h = (h1-h2)*ONE_32;	S->k = (k1-k2)*ONE_32;
					S->w = t;	wgt += t;	S++;	ns++;
					if(ns>=MAXRESID) goto xxx;
				sss: R2++;
				}
			}
		}
		f = R->f;
		if( (LM0(R)==0) && (mode & OR_RESTR) && ((f & fa) == 0) && (f & RSPC) ) {
			if((t=R->a0) > am) {
				if ((mode & 1) == 0) t = 1.;
				p2 = R->p0*2;	//if(f & RIPT) p2 += P_PI;
				h2 = R->h0 * ONE_16; k2 = R->k0 * ONE_16;
				S->p = p2; S->h = h2; S->k = k2; S->w = t/2.;
				S++;	ns++;	wgt += t;
				if(ns>=MAXRESID) goto xxx;
			}
		}
		R++;
	}
xxx:
//-------------------------------------------------------
	bh = bk = 0;
	if( 0 == ns )
		goto xx4;
	z  = 1e34;

	// PETER 15 APRIL 2005
	t  = wgt*64.*65536.;
	t1 = 0.;
	for(i = 0; i < 180; i++)
	{
		S = S0;
//		map = SI->rmap + i;
		map = vMap.begin() + i;
		for(j = 0; j < 180; j++)
		{
			Residual(S, ns, &a);
			if(map)
			{
				d = (WORD)(a/t);
				if(d>255) d = 255;
				*map = (BYTE)d ^ 255;
				map += 180;
			}
			if(a < z+t1)
			{
				z = a;
				bh = i;
				bk = j;
			}
		}
		for(n = 0; n < ns; n++, S++)
		{
			S->p += S->h;
		}
	}
	S = S0;
	if( bh >= 90)
		bh -= 180;
	if(bk >= 90)
		bk -=180;
	{
		FILE *pFile = fopen("d:\\residual180.txt", "wt");
		if( NULL != pFile )
		{
			map = vMap.begin();
			for(i=0; i<180; i++)
			{
				for(j=0; j<180; j++, map++)
				{
					fprintf(pFile, "%d\t", *map);
				}
				fprintf(pFile, "\n");
			}
			fclose(pFile);
		}
	}
	for(i=0; i<ns; i++)
	{
		S->p +=  S->h * (bh-1) + S->k*(bk-1);	// -1!!
		S->h /=16; S->k /=16; S++;
	}
	for(i=-16; i<16; i++) {	// 16!!
		S = S0;
		for(j=-16; j<16; j++) {	// 16!!
			Residual(S, ns, &a);
			if(a < z+t1)
			 {z = a; bh1 = i; bk1 = j;}
		}
		for(n=0; n<ns; n++)
			{ S->p += S->h - S->k * 32; S++;}	// 32!!
	}
	// convert the MAP from 180x180 into 64x64
	dy = dx = 180.0 / 64.0;
	_ycur = 0.0;
	map = SI->rmap;
	srcmap = vMap.begin();
	y_off = 0;
	for(i = 0; i < 64; i++, _ycur += dy)
	{
		_xcur = 0.0;
//		map = SI->rmap + i;
		for(j = 0; j < 64; j++, map++, _xcur += dx)
		{
			// bilinear interpolation???
			*map = srcmap[(int)floor(_xcur + 0.5) + y_off];
		}
		y_off = 180*(int)floor(_ycur + 0.5);
	}
//-------------------------------------------------------
	bh = (1024<<16)*bh + (64<<16)*bh1;	// 64<<16
	bk = (1024<<16)*bk + (64<<16)*bk1;
	// END PETER 15 APRIL 2005
/*
	t  = wgt*64.*65536.;
	t1 = 0.;
	// residuals map should be transposed (h- k|)
	for(i=0; i<64; i++) {
		S = S0;	map = SI->rmap+i;
		for(j=0; j<64; j++) {
			Residual(S, ns, &a);
			if(map) {
				d = (WORD)(a/t); if(d>255) d=255;
				*map = (BYTE)d ^ 255;
				map += 64;
			}
			if(a < z+t1) {z = a; bh = i; bk = j;}
		}
		for(n=0; n<ns; n++){ S->p += S->h; S++;}
	}
	S = S0;	if(bh>=32) bh -= 64; if(bk>=32) bk -=64;
	for(i=0; i<ns; i++) {
		S->p +=  S->h * (bh-1) + S->k*(bk-1);	// -1!!
		S->h /=16; S->k /=16; S++;
	}
*/
	bh1 = bk1 = 0;

#if 0//_DEBUG
	// PETER 13 APRIL 2005
	{
		FILE *pFile = fopen("d:\\residual.txt", "wt");
		if( NULL != pFile )
		{
			map = SI->rmap;
			for(i=0; i<64; i++)
			{
				for(j=0; j<64; j++, map++)
				{
					fprintf(pFile, "%d\t", *map);
				}
				fprintf(pFile, "\n");
			}
			fclose(pFile);
		}
	}
	// END OF PETER
#endif
/*
	for(i=-16; i<16; i++) {	// 16!!
		S = S0;
		for(j=-16; j<16; j++) {	// 16!!
			Residual(S, ns, &a);
			if(a < z+t1)
			 {z = a; bh1 = i; bk1 = j;}
		}
		for(n=0; n<ns; n++)
			{ S->p += S->h - S->k * 32; S++;}	// 32!!
	}
//-------------------------------------------------------
	bh = (1024<<16)*bh + (64<<16)*bh1;	// 64<<16
	bk = (1024<<16)*bk + (64<<16)*bk1;
*/
xx4:
	SI->Bh = bh;
	SI->Bk = bk;
	SI->flg |= OR_DONE;
	free( S0 );
}
#endif
/*******************************************************************************/
void	ApplyPhaseShift( LPLATTICE L )
{
int			i, sh, bh, bk;
LPREFL		R = L->rptr;
LPSGINFO	SI = L->SI+L->Spg;
double		d, sx, sy;

	bh = SI->Bh+SI->Ch;	bk = SI->Bk+SI->Ck;
	if(SI->flg & OR_MAN) { bh += SI->Sh; bk += SI->Sk; }
	d = L->ax*L->by - L->ay*L->bx;
	sx = (bh*L->by - bk*L->ay)/d;
	sy = (bk*L->ax - bh*L->bx)/d;
	for(i=0; i<L->nrefl; i++) {
//		sh	= (int)R->h0 * bh + (int)R->k0 * bk;
		sh = round(R->x0*sx + R->y0*sy);
		if(R->f & RIP1)	R->p -= sh;
		else	        R->p += sh;
		R++;
	}
}
/*******************************************************************************/
static int Calc_IQ( double sn )
{
	if(sn > 2.) return (int)(7./(sn-1.));
	if(sn > 1.) return(8);
	return(9);
}
/*******************************************************************************/
void	ApplySymmetry( LPLATTICE L )
{
	LPREFL	R1, R = L->rptr;
	double	as, rs, is, ns, a, p, qs, amax;
	int		q, f, f0, hk, i = L->nrefl;
	PHASE	pp;
	// Added by Peter on [18/8/2005]
	double dSin, dCos;
	// end of addition by Peter [18/8/2005]

	while(i > 0)
	{
		hk = HK(R);	R1 = R; as = rs = is = ns = qs = 1e-8; f0 = 0;
		amax = 0;
		while( hk == HK(R1) ) {
			if(LM0(R1)==0) {
				a = R1->a0; p = P2R(R1->p); f0 = R1->f;
// Added by Peter on [18/8/2005]
				FastSinCos(p, dSin, dCos);
				as += a;  rs += a*dCos; is += a*dSin;
// end of addition by Peter [18/8/2005]
//				as += a;  rs += a*cos(p); is += a*sin(p);
				ns += 1.; qs += (R1->qq)/10.;
				if(amax < a) amax=a;
			}
			R1++;
		}
		as /= ns; qs /= ns; q = Calc_IQ(qs); f = 0;
// Added by Peter on [18/8/2005]
		a = FastSqrt(rs*rs+is*is)/ns;	pp = R2P(FastAtan2(rs, is));
// end of addition by Peter [18/8/2005]
//		a = FastSqrt(rs*rs+is*is)/ns;	pp = R2P(ATAN2(is, rs));
		if(f0 & RSPC) {
//			if(f0 & RIPT) pp += P_PI2;
			switch((pp>>(13+16))&7) {
				case 0:
				case 7: pp = 0; break;
				case 1:
				case 6: pp = 0;    f |= BPHU;	break;
				case 2:
				case 5: pp = P_PI; f |= BPHU;	break;
				case 3:
				case 4: pp = P_PI; break;
			}
//			if(f0 & RIPT) pp -= P_PI2;
		}
		while( hk == HK(R) )
		{
			if(LM0(R) == 0)
			{
				if (L->Ex_Mode & EM_AMPVSUM) R->a = a; else R->a = as;
				if (L->Ex_Mode & EM_MAX) R->a = amax;
				R->p = pp;	R->f = (R->f & (~(RIQ|BPHU))) | f | q;
				if (ns>1) BitSet(R->f,SYMT);	// If it has been symmetrized
				else      BitClr(R->f,SYMT);
			}
			R++; i--;
		}
	}
}
/*******************************************************************************/
int	FindBin( LPDOUBLE bin, double a )
{
int     i;
	for(i=0;i<10;i++) {
		if(a<bin[i]) return i;
	}
	return 9;
}
/*******************************************************************************/
void	CalcRFactor( LPLATTICE L)
{
LPREFL	R = L->rptr;
int		mode = L->OR_Mode;
int		p, p0, bh, bk, i, f, ns=0, j;
double	t, am, ps = 0., pwg = 0.001, as = 0., awg = 0.001;
int		fa = BCD | BCN | BSN | BAMP | UDEL /*| SABS*/;
LPSGINFO SI = L->SI+L->Spg;
double	rma=-1e20, x,y,rad;
double	obin[10], fbin[10], rbin[10];
int		ocnt[10], fcnt[10];
double	rs,nrs;

	if(SI->flg & RF_DONE) return;
	SI->flg |= RF_DONE;

//	am = L->Thr /* * L->MaxAmp/100.*/;
	am = L->Thr;
	bh = SI->Bh+SI->Ch; bk = SI->Bk+SI->Ck;
	if(SI->flg & OR_MAN) { bh += SI->Sh; bk += SI->Sk; }

	R = L->rptr; rma = 10.;
	for(i = L->nrefl; i>0; i--,R++) if ((((f=R->f) & fa)==0) && ((t=R->a0) > am)){
		x = (L->ax*R->h0+L->bx*R->k0); x *= x;
		y = (L->ay*R->h0+L->by*R->k0); y *= y;
		rad = FastSqrt(x+y);
		rma = __max(rma,rad);
	}

	rad = rma*rma/10.;
	for(i=0;i<10;i++) rbin[i] = FastSqrt(rad*(i+1.));

	memset(obin,0, sizeof(obin)); memset(fbin,0, sizeof(fbin));
	memset(ocnt,0, sizeof(ocnt)); memset(fcnt,0, sizeof(fcnt));

	R = L->rptr;
	for(i = L->nrefl; i>0; i--)
	{
		if( (LM0(R)==0) && (((f=R->f) & fa)==0) && ((t=R->a0) > am) )
		{
			x = (L->ax*R->h0+L->bx*R->k0); x *= x;
			y = (L->ay*R->h0+L->by*R->k0); y *= y;
			rad = FastSqrt(x+y);
			j = FindBin( rbin, rad );
			if ((f & SABS) == 0)
			{
				if (R->f & SYMT) { as += FastAbs(R->a - t); awg += R->a; }
				if ((R->f & SYMT) || (R->f & RSPC)) {
					if ((mode & 1) == 0) t = 1.;
					p0 = R->p0 + (int)R->h0*bh + (int)R->k0*bk;
					p  = R->p;
					if(f & RIP1) p = -p;
					if(f & RIP2) p += P_PI;
					ps += FastAbs((double)(p-p0))*t; pwg += t;
					ns++;
				}
				obin[j] += R->a0;
				ocnt[j]++;
			} else {
				fbin[j] += R->a0;
				fcnt[j]++;
			}
		}
		R++;
	}

	for(i=0,rs=0,nrs=0;i<10;i++) if (fcnt[i] && ocnt[i]) {
		rbin[i] = (fbin[i]/fcnt[i])/(obin[i]/ocnt[i]);
		rs+=rbin[i]*(fcnt[i]+ocnt[i]);
		nrs+=fcnt[i]+ocnt[i];
	}
	SI->Nr = (int)nrs;
	if (nrs) SI->Rr = rs/nrs;
	else     SI->Rr = 0;

	SI->NFa = SI->NFp = ns;
	if (ns==0) {
		SI->RFa = 0.;
		SI->RFp = 0.;
	} else {
		SI->RFa = __min(100.*as/awg, 99.9);
		SI->RFp = __min(P2D((int)(ps/pwg)), 90.);
	}

	if(L->Spg<=SYMM_P2) SI->NFa = 0;
	if(L->Spg==SYMM_P1) SI->NFp = 0;
}
